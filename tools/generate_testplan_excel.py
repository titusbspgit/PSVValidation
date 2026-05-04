#!/usr/bin/env python3
import argparse
import json
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
import zipfile
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
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

HEADER_FILL = PatternFill("solid", fgColor="4F81BD")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True, help="Path to JSON file containing top-level object with key 'testcases'")
    ap.add_argument("--output-dir", required=True, help="Output directory inside repo for Excel file")
    ap.add_argument("--ip-name", required=True, help="IP name used for filename")
    return ap.parse_args()


def load_json_array(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, dict) or "testcases" not in data:
        raise ValueError("Input JSON must be an object with key 'testcases'")
    arr = data["testcases"]
    if not isinstance(arr, list) or not arr:
        raise ValueError("'testcases' must be a non-empty array")
    for i, rec in enumerate(arr):
        if not isinstance(rec, dict):
            raise ValueError(f"Record {i} is not an object")
    return arr


def union_schema(records):
    cols = []
    seen = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def write_data_sheet(ws, schema, records):
    # Header
    for c, key in enumerate(schema, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = HEADER_FILL
    ws.freeze_panes = "A2"

    # Rows
    for r, rec in enumerate(records, 2):
        for c, key in enumerate(schema, 1):
            val = rec.get(key, "")
            ws.cell(row=r, column=c, value=val)

    # Borders for all populated cells
    max_row = ws.max_row
    max_col = ws.max_column
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = THIN_BORDER

    # Basic alignments default
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).alignment = Alignment(vertical="top")

    # Approx column widths
    widths = [len(str(h)) for h in schema]
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            v = ws.cell(row=r, column=c).value
            l = len(str(v)) if v is not None else 0
            if l > widths[c-1]:
                widths[c-1] = l
    for i, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(i)].width = min(max(w + 2, 12), 80)


def create_meta_sheet(wb, records):
    ws = wb.create_sheet("Meta_data_sheet")
    # Header
    for c, key in enumerate(META_COLS, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = HEADER_FILL
    # Rows
    for r, rec in enumerate(records, 2):
        for c, key in enumerate(META_COLS, 1):
            ws.cell(row=r, column=c, value=rec.get(key, ""))
    # Very hidden
    ws.sheet_state = "veryHidden"
    # Borders
    max_row = ws.max_row
    max_col = ws.max_column
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = THIN_BORDER
    return ws


def renumber_multiline(text):
    if text is None:
        return ""
    parts = [ln.strip() for ln in str(text).splitlines()]
    parts = [p for p in parts if p]
    out = []
    for idx, p in enumerate(parts, 1):
        # strip leading bullets/numbers
        q = p
        # Remove common bullets like '-', '*', '•' and numeric prefixes like '1)', '1.'
        for ch in ["\u2022", "-", "*", "\t"]:
            if q.startswith(ch):
                q = q[len(ch):].strip()
        if q[:2].isdigit() or (len(q) > 1 and q[0].isdigit() and q[1] in ")."):
            # find first space after number and trim
            i = 0
            while i < len(q) and (q[i].isdigit() or q[i] in ") ."):
                i += 1
            q = q[i:].strip()
        out.append(f"{idx}. {q}")
    return "\n".join(out) if out else ""


def reorder_and_format_testplan(ws):
    # Build current header map
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    idx_map = {h: i+1 for i, h in enumerate(headers) if h is not None}

    # Build new data matrix with MAIN_ORDER
    keep_cols = [h for h in MAIN_ORDER if h in idx_map]
    rows = []
    for r in range(2, ws.max_row + 1):
        row = []
        for h in keep_cols:
            c = idx_map[h]
            row.append(ws.cell(row=r, column=c).value)
        rows.append(row)

    # Clear sheet and write back
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)

    # Write headers in new order
    for c, key in enumerate(keep_cols, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = HEADER_FILL
    ws.freeze_panes = "A2"

    # Numbering conversion for specific columns
    wrap_idx = {h: i for i, h in enumerate(keep_cols)}
    steps_col = wrap_idx.get("Test Steps / Procedure")
    val_col = wrap_idx.get("Validation / Acceptance Criteria")

    for r_idx, src in enumerate(rows, 2):
        row = list(src)
        if steps_col is not None:
            row[steps_col] = renumber_multiline(row[steps_col])
        if val_col is not None:
            row[val_col] = renumber_multiline(row[val_col])
        for c, val in enumerate(row, 1):
            ws.cell(row=r_idx, column=c, value=val)

    # Formatting: borders, alignment, wrap text for specified columns
    max_row = ws.max_row
    max_col = ws.max_column
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            cell.border = THIN_BORDER
            if r == 1:
                cell.alignment = Alignment(horizontal="center", vertical="center")
            else:
                # Default data alignment
                cell.alignment = Alignment(vertical="top", horizontal="left", wrapText=(ws.cell(row=1, column=c).value in WRAP_COLS))

    # Center or right for numeric/index columns
    if "Index" in keep_cols:
        c = keep_cols.index("Index") + 1
        for r in range(2, max_row + 1):
            ws.cell(row=r, column=c).alignment = Alignment(vertical="top", horizontal="center")

    # Column widths
    widths = [len(str(h)) for h in keep_cols]
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            v = ws.cell(row=r, column=c).value
            l = len(str(v)) if v is not None else 0
            if l > widths[c-1]:
                widths[c-1] = l
    for i, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(i)].width = min(max(w + 2, 12), 100)

    # Approx row heights for wrapped text columns
    wrap_cols_idx = [keep_cols.index(h) + 1 for h in WRAP_COLS if h in keep_cols]
    base_h = 15
    for r in range(2, max_row + 1):
        lines = 1
        for c in wrap_cols_idx:
            txt = ws.cell(row=r, column=c).value
            if txt:
                n = str(txt).count("\n") + 1
                if n > lines:
                    lines = n
        ws.row_dimensions[r].height = base_h * min(lines, 40)

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in keep_cols:
        c = keep_cols.index("Code Generation (Required / Not)") + 1
        col_letter = get_column_letter(c)
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
        ws.add_data_validation(dv)
        if max_row >= 2:
            dv.add(f"{col_letter}2:{col_letter}{max_row}")


def validate_xlsx(path):
    if not zipfile.is_zipfile(path):
        raise ValueError("Not a valid XLSX (ZIP) file")
    # Try open with openpyxl
    _ = load_workbook(path)


def main():
    args = parse_args()
    records = load_json_array(args.input)
    schema = union_schema(records)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    write_data_sheet(ws, schema, records)
    create_meta_sheet(wb, records)

    # Transform Data -> TestPlan on the same sheet
    reorder_and_format_testplan(ws)
    ws.title = "TestPlan"

    # Safety check: ensure only allowed sheets exist
    names = [s.title for s in wb.worksheets]
    if "Data" in names:
        # remove any residual Data sheet (shouldn't happen as we renamed the only sheet)
        idx = names.index("Data")
        wb.remove(wb.worksheets[idx])

    # Ensure only TestPlan and Meta_data_sheet
    names = [s.title for s in wb.worksheets]
    assert set(names) == {"TestPlan", "Meta_data_sheet"}, f"Unexpected sheets present: {names}"

    # Timestamp IST
    now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    stamp = now_ist.strftime("%Y%m%d_%H%M%S")
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{args.ip_name}_TestPlan_{stamp}.xlsx"

    wb.save(out_path)
    validate_xlsx(out_path)

    print(f"SAVED: {out_path}")

if __name__ == "__main__":
    main()
