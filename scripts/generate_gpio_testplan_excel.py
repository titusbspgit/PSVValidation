#!/usr/bin/env python3
import argparse
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timedelta, timezone
from copy import deepcopy

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_ORDER = [
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

BLUE_FILL = PatternFill(fill_type="solid", start_color="FF4472C4", end_color="FF4472C4")
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))


def parse_args():
    p = argparse.ArgumentParser(description="Generate GPIO Test Plan Excel from JSON with Stage1 rules")
    p.add_argument('--json', required=True, help='Path to JSON input (object with TC* or array)')
    p.add_argument('--ip', required=True, help='IP name (for naming)')
    p.add_argument('--outdir', required=True, help='Output directory inside repo')
    p.add_argument('--base-dir', required=True, help='Traceability base directory path (for commit message only)')
    return p.parse_args()


def ensure_json_array(data):
    # Accept either array or object (e.g., {"TC1": {...}, ...})
    if isinstance(data, list):
        arr = data
    elif isinstance(data, dict):
        # Preserve insertion order, but stably sort by Index if present
        items = list(data.items())
        # Build list of records
        arr = [deepcopy(v) for k, v in items]
        try:
            arr.sort(key=lambda d: int(str(d.get('Index', '0')).strip()))
        except Exception:
            pass
    else:
        raise ValueError('JSON must be an array or object mapping test IDs to objects')

    if not arr:
        raise ValueError('JSON is empty')
    return arr


def build_schema(records):
    # Union of keys preserving first-seen order across records
    cols = []
    seen = set()
    for rec in records:
        if not isinstance(rec, dict):
            raise ValueError('Each JSON record must be an object')
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def sanitize_value(v):
    return v if v is not None else ""


def compute_col_width(value):
    if value is None:
        return 0
    s = str(value)
    # approximate width: chars + padding; consider newlines
    max_line = max((len(line) for line in s.splitlines()), default=0)
    return min(max(10, max_line + 2), 120)


def renumber_multiline(text):
    if text is None:
        return ""
    lines = [ln.strip() for ln in str(text).splitlines() if ln.strip() != ""]
    out = []
    for i, ln in enumerate(lines, start=1):
        # strip any existing numeric prefix like '1)', '1.', '1 )', etc.
        ln2 = re.sub(r"^\s*\d+\s*[\.)]\s*", "", ln)
        out.append(f"{i}. {ln2}")
    return "\n".join(out)


def set_col_widths(ws, headers, rows):
    for ci, h in enumerate(headers, start=1):
        maxw = compute_col_width(h)
        for r in rows:
            v = r.get(h, "")
            maxw = max(maxw, compute_col_width(v))
        ws.column_dimensions[ws.cell(row=1, column=ci).column_letter].width = float(maxw)


def adjust_row_heights(ws, text_cols_idx):
    base = 15.0
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in text_cols_idx:
            v = ws.cell(row=r, column=c).value
            if v is not None:
                lines = str(v).count('\n') + 1
                if lines > max_lines:
                    max_lines = lines
        ws.row_dimensions[r].height = base * max_lines


def validate_xlsx(path):
    # Check ZIP structure
    if not zipfile.is_zipfile(path):
        return False
    try:
        with zipfile.ZipFile(path, 'r') as zf:
            names = set(zf.namelist())
            if '[Content_Types].xml' not in names:
                return False
        # Try loading via openpyxl
        _ = load_workbook(path)
        return True
    except Exception:
        return False


def ist_now():
    # IST = UTC+5:30
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(tz=ist)
    return now_ist


def main():
    args = parse_args()

    # Phase 1: Read & validate JSON
    try:
        with open(args.json, 'r', encoding='utf-8') as f:
            raw = json.load(f)
    except Exception as e:
        print(f"ERROR: Failed to read/parse JSON: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        records = ensure_json_array(raw)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)

    schema = build_schema(records)

    # Phase 1: Base workbook with 'Data' sheet
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Headers
    for ci, h in enumerate(schema, start=1):
        cell = ws.cell(row=1, column=ci, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
        cell.fill = BLUE_FILL

    # Data rows (preserve values exactly)
    for ri, rec in enumerate(records, start=2):
        for ci, h in enumerate(schema, start=1):
            ws.cell(row=ri, column=ci, value=sanitize_value(rec.get(h, "")))

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Approximate column widths
    set_col_widths(ws, schema, records)

    # Phase 2: Meta sheet (very hidden)
    meta = wb.create_sheet('Meta_data_sheet')
    for ci, h in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=ci, value=h).font = Font(bold=True)
    for ri, rec in enumerate(records, start=2):
        for ci, h in enumerate(META_COLS, start=1):
            meta.cell(row=ri, column=ci, value=sanitize_value(rec.get(h, "")))
    meta.sheet_state = 'veryHidden'

    # Phase 2: Normalize main sheet in place
    ws.title = 'TestPlan'  # rename Data -> TestPlan

    # Build map header -> col idx
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    name_to_idx = {h: i+1 for i, h in enumerate(headers)}

    # Delete META columns (descending order to avoid index shift)
    meta_indices = [name_to_idx[h] for h in META_COLS if h in name_to_idx]
    for col_idx in sorted(meta_indices, reverse=True):
        ws.delete_cols(col_idx)

    # Rebuild header mapping after deletions
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    # Compose final order strictly as MAIN_ORDER
    final_headers = [h for h in MAIN_ORDER if h in headers]

    # Create a snapshot of current rows by header
    cur_name_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Overwrite headers in required order
    for c, h in enumerate(final_headers, start=1):
        ws.cell(row=1, column=c, value=h)

    # Write rows in the new order
    for r in range(2, ws.max_row + 1):
        for c, h in enumerate(final_headers, start=1):
            src_col = cur_name_to_col.get(h)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            # Numbering inside cells for specified columns
            if h in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = renumber_multiline(val)
            ws.cell(row=r, column=c, value=val)

    # Delete any extra columns beyond final headers
    if ws.max_column > len(final_headers):
        ws.delete_cols(len(final_headers) + 1, ws.max_column - len(final_headers))

    # Re-style header row for TestPlan
    for c in range(1, len(final_headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
        cell.fill = BLUE_FILL

    # Apply wrap text and default alignments, borders
    header_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    text_cols_idx = [header_to_col[h] for h in WRAP_COLS if h in header_to_col]

    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            # Default alignments
            h_align = 'left'
            if ws.cell(row=1, column=c).value == 'Index':
                h_align = 'center'
            cell.alignment = Alignment(horizontal=h_align, vertical='top', wrap_text=(c in text_cols_idx))
            cell.border = THIN_BORDER

    # Autofit widths again based on final content
    # Build row dicts for width calc
    final_rows = []
    for r in range(2, ws.max_row + 1):
        row_dict = {}
        for c, h in enumerate(final_headers, start=1):
            row_dict[h] = ws.cell(row=r, column=c).value
        final_rows.append(row_dict)
    set_col_widths(ws, final_headers, final_rows)

    # Adjust row heights based on wrapped content
    adjust_row_heights(ws, text_cols_idx)

    # Data validation for Code Generation (Required / Not)
    if 'Code Generation (Required / Not)' in header_to_col:
        cg_col = header_to_col['Code Generation (Required / Not)']
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=False, showDropDown=True)
        ws.add_data_validation(dv)
        dv.add(f"{ws.cell(row=2, column=cg_col).coordinate}:{ws.cell(row=ws.max_row, column=cg_col).coordinate}")

    # Safety check: ensure 'Data' sheet does not exist and only allowed sheets remain
    for s in wb.sheetnames:
        if s == 'Data':
            # delete if somehow present
            del wb[s]
            break
    allowed = set(['TestPlan', 'Meta_data_sheet'])
    if set(wb.sheetnames) - allowed:
        # If any unexpected sheets are present, fail
        print("ERROR: Unexpected worksheets present after normalization", file=sys.stderr)
        sys.exit(2)

    # Phase 3: Save and validate
    now_ist = ist_now()
    ts_file = now_ist.strftime('%Y%m%d_%H%M%S')
    ts_human = now_ist.strftime('%Y-%m-%d %H:%M:%S')
    os.makedirs(args.outdir, exist_ok=True)
    out_name = f"{args.ip}_TestPlan_{ts_file}.xlsx"
    out_path = os.path.join(args.outdir, out_name)

    wb.save(out_path)

    if not validate_xlsx(out_path):
        print("ERROR: XLSX validation failed", file=sys.stderr)
        sys.exit(3)

    # Emit automation outputs for workflow
    with open('scripts/_automation_output.env', 'w', encoding='utf-8') as outf:
        outf.write(f"EXCEL_FILE={out_path}\n")
        outf.write(f"IST_TIMESTAMP={ts_human}\n")

    print(f"Wrote {out_path} (IST: {ts_human})")

if __name__ == '__main__':
    main()
