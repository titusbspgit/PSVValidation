#!/usr/bin/env python3
import argparse
import json
import os
import re
import sys
from datetime import datetime, timedelta, timezone
import zipfile

try:
    import openpyxl
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"[ERROR] openpyxl not available: {e}")
    sys.exit(1)

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

DV_COL = "Code Generation (Required / Not)"
DV_LIST = "Required,Blank,Not Required"

BLUE_FILL = PatternFill(fill_type="solid", fgColor="4472C4")
THIN_BORDER = Border(
    left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin')
)


def parse_args():
    p = argparse.ArgumentParser(description="Generate formatted TestPlan Excel from JSON")
    p.add_argument('--json', required=True, help='Path to input JSON array file')
    p.add_argument('--output-dir', required=True, help='Directory to save the Excel file')
    p.add_argument('--ip-name', required=True, help='IP name for filename prefix')
    return p.parse_args()


def load_json(json_path):
    if not os.path.exists(json_path):
        print(f"[ERROR] JSON file not found: {json_path}")
        sys.exit(2)
    try:
        with open(json_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"[ERROR] Failed to parse JSON: {e}")
        sys.exit(3)
    if not isinstance(data, list) or len(data) == 0:
        print("[ERROR] JSON must be a non-empty array")
        sys.exit(4)
    return data


def union_keys_preserve_order(rows):
    order = []
    seen = set()
    for r in rows:
        if not isinstance(r, dict):
            print("[ERROR] Each JSON element must be an object")
            sys.exit(5)
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                order.append(k)
    return order


def textlen(v):
    if v is None:
        return 0
    s = str(v)
    # heuristic width multiplier
    return min(max(len(s), 10), 120)


def normalize_numbering(text):
    if text is None:
        return ""
    s = str(text).strip()
    if not s:
        return s
    # split on occurrences of N) or N. at any position
    parts = re.split(r"\s*\d+[\)\.]\s*", s)
    parts = [p.strip() for p in parts if p.strip()]
    if len(parts) <= 1:
        # try newline or semicolon as separators
        if '\n' in s:
            parts = [ln.strip() for ln in s.splitlines() if ln.strip()]
        elif ';' in s:
            parts = [p.strip() for p in s.split(';') if p.strip()]
        else:
            return f"1. {s}"
    numbered = []
    for i, item in enumerate(parts, start=1):
        # Remove any leading numbering again, then add strict "i. "
        item = re.sub(r"^\s*\d+[\)\.]\s*", "", item)
        numbered.append(f"{i}. {item}")
    return "\n".join(numbered)


def create_base_workbook(rows, col_order):
    wb = openpyxl.Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header
    for j, key in enumerate(col_order, start=1):
        cell = ws.cell(row=1, column=j, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        cell.fill = BLUE_FILL

    # Rows
    for i, row in enumerate(rows, start=2):
        for j, key in enumerate(col_order, start=1):
            val = row.get(key, "")
            ws.cell(row=i, column=j, value=val)

    ws.freeze_panes = "A2"

    # Autosize columns (heuristic)
    for j, key in enumerate(col_order, start=1):
        maxlen = textlen(key)
        for i in range(2, len(rows) + 2):
            v = ws.cell(row=i, column=j).value
            maxlen = max(maxlen, textlen(v))
        ws.column_dimensions[openpyxl.utils.get_column_letter(j)].width = min(maxlen + 2, 80)

    # Borders and default data alignment
    for i in range(1, len(rows) + 2):
        for j in range(1, len(col_order) + 1):
            c = ws.cell(row=i, column=j)
            c.border = THIN_BORDER
            if i == 1:
                continue
            c.alignment = Alignment(vertical='top', horizontal='left', wrap_text=False)

    return wb, ws


def build_meta_sheet(wb, rows):
    meta = wb.create_sheet("Meta_data_sheet")
    # header
    for j, key in enumerate(META_COLS, start=1):
        c = meta.cell(row=1, column=j, value=key)
        c.font = Font(bold=True)
        c.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        c.fill = BLUE_FILL
    # rows
    for i, row in enumerate(rows, start=2):
        for j, key in enumerate(META_COLS, start=1):
            meta.cell(row=i, column=j, value=row.get(key, ""))
    # style
    for j, key in enumerate(META_COLS, start=1):
        maxlen = textlen(key)
        for i in range(2, len(rows) + 2):
            v = meta.cell(row=i, column=j).value
            maxlen = max(maxlen, textlen(v))
        meta.column_dimensions[openpyxl.utils.get_column_letter(j)].width = min(maxlen + 2, 80)
    for i in range(1, len(rows) + 2):
        for j in range(1, len(META_COLS) + 1):
            c = meta.cell(row=i, column=j)
            c.border = THIN_BORDER
            if i == 1:
                continue
            c.alignment = Alignment(vertical='top', horizontal='left', wrap_text=True)
    # very hidden
    meta.sheet_state = 'veryHidden'


def transform_to_testplan(wb, data_ws, rows):
    # Rename Data -> TestPlan
    data_ws.title = "TestPlan"

    # Build current column order
    existing_cols = [data_ws.cell(row=1, column=j).value for j in range(1, data_ws.max_column + 1)]

    # Remove META columns from visible sheet by reconstructing with MAIN_ORDER
    # Prepare data matrix according to MAIN_ORDER
    main_cols = MAIN_ORDER

    # Build a lookup from column name to index in existing sheet
    col_index = {name: idx+1 for idx, name in enumerate(existing_cols)}

    # Create a temp grid with header and data in main order
    grid = [main_cols]
    for r in rows:
        grid.append([r.get(k, "") for k in main_cols])

    # Clear the sheet and write grid
    data_ws.delete_rows(1, data_ws.max_row)
    data_ws.delete_cols(1, data_ws.max_column)

    for j, key in enumerate(grid[0], start=1):
        c = data_ws.cell(row=1, column=j, value=key)
        c.font = Font(bold=True)
        c.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        c.fill = BLUE_FILL

    for i in range(2, len(grid) + 1):
        for j in range(1, len(grid[0]) + 1):
            data_ws.cell(row=i, column=j, value=grid[i-1][j-1])

    # Numbering inside cells for two columns
    def apply_numbering_to_column(col_name):
        if col_name not in MAIN_ORDER:
            return
        col_idx = MAIN_ORDER.index(col_name) + 1
        for i in range(2, len(rows) + 2):
            v = data_ws.cell(row=i, column=col_idx).value
            data_ws.cell(row=i, column=col_idx, value=normalize_numbering(v))

    apply_numbering_to_column("Test Steps / Procedure")
    apply_numbering_to_column("Validation / Acceptance Criteria")

    # Wrap and alignment rules
    for j, key in enumerate(MAIN_ORDER, start=1):
        maxlen = textlen(key)
        for i in range(2, len(rows) + 2):
            cell = data_ws.cell(row=i, column=j)
            val = cell.value
            maxlen = max(maxlen, textlen(val))
            wrap = key in WRAP_COLS
            if i == 1:
                continue
            # default alignment
            halign = 'left'
            if key == 'Index':
                halign = 'center'
            cell.alignment = Alignment(vertical='top', horizontal=halign, wrap_text=wrap)
            cell.border = THIN_BORDER
        data_ws.column_dimensions[openpyxl.utils.get_column_letter(j)].width = min(maxlen + 2, 80)

    data_ws.freeze_panes = "A2"

    # Data validation for DV_COL
    if DV_COL in MAIN_ORDER:
        col_idx = MAIN_ORDER.index(DV_COL) + 1
        dv = DataValidation(type="list", formula1=f'"{DV_LIST}"', allow_blank=True)
        rng = f"{openpyxl.utils.get_column_letter(col_idx)}2:{openpyxl.utils.get_column_letter(col_idx)}{len(rows)+1}"
        dv.add(rng)
        data_ws.add_data_validation(dv)


def validate_xlsx(path):
    try:
        with zipfile.ZipFile(path, 'r') as zf:
            names = set(zf.namelist())
            required = {"[Content_Types].xml", "xl/workbook.xml"}
            return required.issubset(names)
    except Exception as e:
        print(f"[ERROR] XLSX validation exception: {e}")
        return False


def main():
    args = parse_args()
    rows = load_json(args.json)

    # PHASE 1: base workbook
    col_order = union_keys_preserve_order(rows)
    wb, data_ws = create_base_workbook(rows, col_order)

    # PHASE 2: meta + transform
    build_meta_sheet(wb, rows)
    transform_to_testplan(wb, data_ws, rows)

    # Safety: ensure no sheet named 'Data'
    if any(ws.title == 'Data' for ws in wb.worksheets):
        print("[ERROR] Sheet named 'Data' still exists after transformation")
        sys.exit(6)

    # Ensure only TestPlan and Meta_data_sheet exist
    allowed = {"TestPlan", "Meta_data_sheet"}
    actual = {ws.title for ws in wb.worksheets}
    if actual != allowed:
        # It's acceptable if the order differs, but not extra sheets
        if not actual.issubset(allowed):
            print(f"[ERROR] Unexpected sheets present: {actual}")
            sys.exit(7)

    # PHASE 3: save
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    fname = f"{args.ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"
    out_dir = args.output_dir
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)

    if not validate_xlsx(out_path):
        print("[ERROR] XLSX validation failed")
        sys.exit(8)

    print(f"Generated: {out_path}")

if __name__ == '__main__':
    main()
