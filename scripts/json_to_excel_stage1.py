#!/usr/bin/env python3
import argparse
import json
from datetime import datetime
from zoneinfo import ZoneInfo
import os
import sys
from openpyxl import Workbook
from openpyxl.styles import Alignment, Border, Side, Font, PatternFill
from openpyxl.worksheet.datavalidation import DataValidation

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

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

BLUE_HEADER = PatternFill(fill_type="solid", fgColor="1F4E78")
THIN_BORDER = Border(
    left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin')
)

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}


def load_records(path: str):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if isinstance(data, dict) and 'TEST_CASES' in data and isinstance(data['TEST_CASES'], list):
        rows = data['TEST_CASES']
    elif isinstance(data, list):
        rows = data
    elif isinstance(data, dict):
        rows = [data]
    else:
        raise SystemExit('Invalid JSON: unsupported top-level structure')
    if not rows:
        raise SystemExit('Invalid JSON: empty dataset')
    return rows


def normalize_schema(rows):
    # Union of keys preserving first-seen order
    keys = []
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            raise SystemExit('Invalid JSON: each row must be an object')
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                keys.append(k)
    return keys


def create_workbook(rows, keys):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'
    # Header
    for c, k in enumerate(keys, start=1):
        ws.cell(row=1, column=c, value=k)
    # Rows
    for r, row in enumerate(rows, start=2):
        for c, k in enumerate(keys, start=1):
            val = row.get(k, "")
            ws.cell(row=r, column=c, value=json.dumps(val, ensure_ascii=False) if isinstance(val, (list, dict)) else val)
    # Basic formatting
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    ws.freeze_panes = 'A2'
    # Autofit naive
    for i, k in enumerate(keys, start=1):
        max_len = max(len(str(ws.cell(row=r, column=i).value or '')) for r in range(1, ws.max_row+1))
        ws.column_dimensions[ws.cell(row=1, column=i).column_letter].width = min(max(10, max_len + 2), 80)
    return wb


def split_meta_and_main(wb, keys):
    ws_data = wb['Data']
    # Create Meta_data_sheet
    ws_meta = wb.create_sheet('Meta_data_sheet')
    # Copy META columns
    meta_keys = [k for k in keys if k in META_COLS]
    for c, k in enumerate(meta_keys, start=1):
        ws_meta.cell(row=1, column=c, value=k)
        for r in range(2, ws_data.max_row+1):
            ws_meta.cell(row=r, column=c, value=ws_data.cell(row=r, column=keys.index(k)+1).value)
    ws_meta.sheet_state = 'veryHidden'

    # Prepare TestPlan
    ws_data.title = 'TestPlan'
    # Determine kept MAIN columns in strict order
    main_keys = [k for k in MAIN_ORDER if k in keys]
    # Build a new grid for TestPlan with only main columns
    # Create a mapping from old col index to new index
    new_ws = wb.create_sheet('TMP_TestPlan')
    for c, k in enumerate(main_keys, start=1):
        new_ws.cell(row=1, column=c, value=k)
        old_c = keys.index(k) + 1
        for r in range(2, ws_data.max_row+1):
            new_ws.cell(row=r, column=c, value=ws_data.cell(row=r, column=old_c).value)
    # Remove old TestPlan and rename
    wb.remove(ws_data)
    new_ws.title = 'TestPlan'
    return wb, main_keys


def format_testplan(wb, main_keys):
    ws = wb['TestPlan']
    # Header styling
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
        cell.fill = BLUE_HEADER
        cell.border = THIN_BORDER
    # Data rows formatting
    for r in range(2, ws.max_row+1):
        for c in range(1, ws.max_column+1):
            cell = ws.cell(row=r, column=c)
            # Wrap for specific columns
            if ws.cell(row=1, column=c).value in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical='top')
            else:
                cell.alignment = Alignment(vertical='top')
            cell.border = THIN_BORDER
    # Autofit columns
    for c in range(1, ws.max_column+1):
        header = ws.cell(row=1, column=c).value or ''
        max_len = len(str(header))
        for r in range(2, ws.max_row+1):
            v = ws.cell(row=r, column=c).value
            l = len(str(v)) if v is not None else 0
            if l > max_len:
                max_len = l
        ws.column_dimensions[ws.cell(row=1, column=c).column_letter].width = min(max(12, max_len + 2), 100)
    # Freeze top row
    ws.freeze_panes = 'A2'
    # Data validation for Code Generation
    if "Code Generation (Required / Not)" in main_keys:
        col_idx = main_keys.index("Code Generation (Required / Not)") + 1
        last_row = ws.max_row if ws.max_row > 1 else 1000
        dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
        rng = f"{ws.cell(row=1, column=col_idx).column_letter}2:{ws.cell(row=1, column=col_idx).column_letter}{last_row}"
        dv.add(rng)
        ws.add_data_validation(dv)


def save_and_commit(wb, ip_name: str, output_dir: str):
    ist_now = datetime.now(ZoneInfo('Asia/Kolkata'))
    stamp_date = ist_now.strftime('%Y%m%d')
    stamp_time = ist_now.strftime('%H%M%S')
    filename = f"{ip_name}_TestPlan_{stamp_date}_{stamp_time}.xlsx"
    rel_dir = output_dir.strip('/')
    out_path = os.path.join(rel_dir, filename)
    os.makedirs(rel_dir, exist_ok=True)
    wb.save(out_path)
    # Commit and push
    os.system('git config user.name "github-actions[bot]"')
    os.system('git config user.email "41898282+github-actions[bot]@users.noreply.github.com"')
    os.system(f'git add "{out_path}"')
    rc = os.system('git commit -m "Final formatted Excel generated from JSON input"')
    if rc != 0:
        print('Nothing to commit or commit failed', file=sys.stderr)
    os.system('git push')
    print(f"::notice title=IST Timestamp::{ist_now.isoformat()}")
    print(f"::notice title=Excel Path::{out_path}")
    return filename, out_path, ist_now.isoformat()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--input', required=True)
    ap.add_argument('--ip-name', required=True)
    ap.add_argument('--output-dir', required=True)
    args = ap.parse_args()

    rows = load_records(args.input)
    keys = normalize_schema(rows)
    wb = create_workbook(rows, keys)
    wb, main_keys = split_meta_and_main(wb, keys)
    format_testplan(wb, main_keys)
    save_and_commit(wb, args.ip_name, args.output_dir)

if __name__ == '__main__':
    main()
