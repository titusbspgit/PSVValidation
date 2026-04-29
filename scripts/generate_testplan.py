#!/usr/bin/env python3
# coding: utf-8

import os
import sys
import json
import re
import io
import math
from datetime import datetime, timedelta, timezone
from zipfile import ZipFile, is_zipfile

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.utils import get_column_letter
    from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl not available: {e}", file=sys.stderr)
    sys.exit(2)

# IST timezone without external deps
IST = timezone(timedelta(hours=5, minutes=30))

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

HEADER_FILL = PatternFill("solid", fgColor="4472C4")  # blue
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def read_json_payload():
    # Prefer env JSON_PAYLOAD; fallback to file if provided as arg
    payload = os.environ.get("JSON_PAYLOAD", "").strip()
    if not payload and len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
        with io.open(sys.argv[1], "r", encoding="utf-8") as f:
            payload = f.read()
    if not payload:
        print("ERROR: No JSON payload provided", file=sys.stderr)
        sys.exit(1)
    try:
        data = json.loads(payload)
    except Exception as e:
        print(f"ERROR: Invalid JSON payload: {e}", file=sys.stderr)
        sys.exit(1)
    # Normalize to array of records, preserving order of top-level object values if object
    if isinstance(data, dict):
        records = list(data.values())
    elif isinstance(data, list):
        records = data
    else:
        print("ERROR: JSON root must be object or array", file=sys.stderr)
        sys.exit(1)
    if not records:
        print("ERROR: JSON array is empty", file=sys.stderr)
        sys.exit(1)
    # Ensure each element is an object/dict
    for i, r in enumerate(records):
        if not isinstance(r, dict):
            print(f"ERROR: Record at index {i} is not an object", file=sys.stderr)
            sys.exit(1)
    return records


def keys_union_preserve_order(records):
    seen = []
    seen_set = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    return seen


def to_cell_value(v):
    # Preserve exact values for lists/dicts via JSON string; otherwise pass through
    if isinstance(v, (list, dict)):
        return json.dumps(v, ensure_ascii=False)
    return v


def approx_autofit(ws):
    # Determine max length per column and set width; rough estimation
    col_max = {}
    for r in ws.iter_rows(values_only=True):
        for idx, val in enumerate(r, start=1):
            s = "" if val is None else str(val)
            l = len(s)
            if l == 0:
                continue
            col_max[idx] = max(col_max.get(idx, 0), l)
    for idx, m in col_max.items():
        width = min(120, max(10, m + 2))
        ws.column_dimensions[get_column_letter(idx)].width = width


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def numberize_block(text):
    if text is None:
        return ""
    s = str(text)
    lines = [ln for ln in s.splitlines() if ln.strip() != ""]
    if not lines:
        return s
    cleaned = []
    bullet_re = re.compile(r"^\s*(?:\d+[\.)]|[-*•])\s*")
    for ln in lines:
        cleaned.append(bullet_re.sub("", ln.strip()))
    numbered = [f"{i}. {ln}" for i, ln in enumerate(cleaned, 1)]
    return "\n".join(numbered)


def build_workbook(records):
    # Determine schema order
    all_keys = keys_union_preserve_order(records)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header
    for c, k in enumerate(all_keys, start=1):
        cell = ws.cell(row=1, column=c, value=k)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Rows
    for r_idx, rec in enumerate(records, start=2):
        for c, k in enumerate(all_keys, start=1):
            v = to_cell_value(rec.get(k, ""))
            ws.cell(row=r_idx, column=c, value=v)

    approx_autofit(ws)

    # Create META sheet and copy meta columns AS-IS
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    for c, k in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=c, value=k).font = Font(bold=True)
    for r_idx, rec in enumerate(records, start=2):
        for c, k in enumerate(META_COLS, start=1):
            meta_ws.cell(row=r_idx, column=c, value=to_cell_value(rec.get(k, "")))
    meta_ws.sheet_state = "veryHidden"

    # STEP 7 — Normalize MAIN Sheet directly on same sheet
    ws.title = "TestPlan"  # rename Data -> TestPlan

    # Rebuild content in-place with MAIN_COLS order and without META cols
    new_rows = []
    new_rows.append(MAIN_COLS[:])
    for rec in records:
        row_vals = []
        for k in MAIN_COLS:
            row_vals.append(rec.get(k, ""))
        new_rows.append(row_vals)

    # Clear existing cells and write new_rows
    ws.delete_rows(1, ws.max_row)
    for r_idx, row in enumerate(new_rows, start=1):
        for c_idx, val in enumerate(row, start=1):
            # Numbering transformations for specific columns on data rows only
            header = new_rows[0][c_idx - 1]
            out_val = val
            if r_idx > 1 and header in {"Test Steps / Procedure", "Validation / Acceptance Criteria"}:
                out_val = numberize_block(val)
            # Preserve list/dict for other columns
            out_val = to_cell_value(out_val)
            ws.cell(row=r_idx, column=c_idx, value=out_val)

    # Formatting
    header_row = 1
    for c_idx in range(1, len(MAIN_COLS) + 1):
        cell = ws.cell(row=header_row, column=c_idx)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL

    # Data rows formatting
    for r in range(2, ws.max_row + 1):
        for c_idx in range(1, ws.max_column + 1):
            hdr = ws.cell(row=1, column=c_idx).value
            cell = ws.cell(row=r, column=c_idx)
            # Wrap for specific columns
            if hdr in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            else:
                # Default: vertical top; left align; center numeric/index column
                if hdr == "Index":
                    cell.alignment = Alignment(vertical="top", horizontal="center")
                else:
                    cell.alignment = Alignment(vertical="top", horizontal="left")

    approx_autofit(ws)

    # Adjust row heights roughly for wrapped text (estimate by line breaks and length / width)
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c_idx in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c_idx).value
            if v is None:
                continue
            s = str(v)
            lines = s.count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = min(300, 15 * max_lines)

    apply_borders(ws)

    # Data validation for Code Generation (Required / Not) on data rows only
    try:
        code_col_index = MAIN_COLS.index("Code Generation (Required / Not)") + 1
        start_row = 2
        end_row = ws.max_row
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        rng = f"{get_column_letter(code_col_index)}{start_row}:{get_column_letter(code_col_index)}{end_row}"
        dv.add(rng)
        ws.add_data_validation(dv)
    except ValueError:
        print("ERROR: 'Code Generation (Required / Not)' column not found", file=sys.stderr)
        sys.exit(1)

    # Safety: Only TestPlan (visible) and Meta_data_sheet (veryHidden)
    names = [s.title for s in wb.worksheets]
    if "Data" in names:
        # try to delete if somehow exists
        for s in wb.worksheets:
            if s.title == "Data":
                wb.remove(s)
                break
        if "Data" in [s.title for s in wb.worksheets]:
            print("ERROR: Data sheet still exists after deletion attempt", file=sys.stderr)
            sys.exit(1)

    return wb


def validate_xlsx(path):
    # Validate as OOXML zip and re-open with openpyxl
    if not is_zipfile(path):
        return False, "File is not a valid ZIP"
    try:
        with ZipFile(path, 'r') as z:
            req = {'[Content_Types].xml', 'xl/workbook.xml'}
            names = set(z.namelist())
            if not req.issubset(names):
                return False, "Missing OOXML core parts"
    except Exception as e:
        return False, f"ZIP open failed: {e}"
    try:
        load_workbook(path)
    except Exception as e:
        return False, f"openpyxl load failed: {e}"
    return True, "OK"


def main():
    records = read_json_payload()

    # Build workbook
    wb = build_workbook(records)

    # Determine output
    ip_name = os.environ.get("IP_NAME", "IP")
    out_dir = os.environ.get("OUTPUT_DIR", "Test_Output/%s/TestPlan" % ip_name)
    os.makedirs(out_dir, exist_ok=True)

    now = datetime.now(IST)
    ts = now.strftime("%Y%m%d_%H%M%S")
    out_name = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(out_dir, out_name)

    # Save
    wb.save(out_path)

    ok, msg = validate_xlsx(out_path)
    if not ok:
        print(f"ERROR: XLSX validation failed: {msg}", file=sys.stderr)
        sys.exit(1)

    print(f"Saved: {out_path}")

    # Commit only the finalized Excel file
    commit_msg = "Final formatted Excel generated from JSON input"
    os.system('git config user.name "github-actions[bot]"')
    os.system('git config user.email "41898282+github-actions[bot]@users.noreply.github.com"')
    os.system(f'git add "{out_path}"')
    rc = os.system(f'git commit -m "{commit_msg}"')
    if rc != 0:
        print("WARNING: Nothing to commit or commit failed.")
    rc = os.system('git push')
    if rc != 0:
        print("ERROR: git push failed", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
