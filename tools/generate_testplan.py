#!/usr/bin/env python3
import json, sys, os
from datetime import datetime, timedelta, timezone
from argparse import ArgumentParser
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, Border, Side
from openpyxl.utils import get_column_letter

MAIN_COLUMNS = [
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

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

def to_cell_value(v):
    if v is None:
        return ""
    if isinstance(v, (list, tuple)):
        return "\n".join([str(x) if x is not None else "" for x in v])
    return v


def union_keys(objs):
    seen = []
    for o in objs:
        for k in o.keys():
            if k not in seen:
                seen.append(k)
    return seen


def autosize(ws):
    for col in ws.iter_cols(min_row=1, max_row=ws.max_row, max_col=ws.max_column):
        max_len = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            try:
                val = str(cell.value) if cell.value is not None else ""
            except Exception:
                val = ""
            if val:
                max_len = max(max_len, max((len(s) for s in val.split("\n")), default=0))
        ws.column_dimensions[col_letter].width = min(max(12, max_len + 2), 80)


def apply_borders(ws):
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def main():
    ap = ArgumentParser()
    ap.add_argument('--input', required=True)
    ap.add_argument('--output-dir', required=True)
    ap.add_argument('--ip-name', required=True)
    args = ap.parse_args()

    with open(args.input, 'r', encoding='utf-8') as f:
        data = json.load(f)

    tests = data.get('tests', [])
    if not isinstance(tests, list) or len(tests) == 0:
        print('No tests found in JSON', file=sys.stderr)
        sys.exit(2)

    # Create workbook and Data sheet
    wb = Workbook()
    ws_data = wb.active
    ws_data.title = 'Data'

    keys = union_keys(tests)
    # Header
    for c, k in enumerate(keys, start=1):
        ws_data.cell(row=1, column=c, value=k)
    # Rows
    for r, t in enumerate(tests, start=2):
        for c, k in enumerate(keys, start=1):
            ws_data.cell(row=r, column=c, value=to_cell_value(t.get(k, "")))

    # Basic formatting on Data
    for cell in ws_data[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    ws_data.freeze_panes = 'A2'
    autosize(ws_data)

    # Meta_data_sheet
    ws_meta = wb.create_sheet('Meta_data_sheet')
    # Headers for meta
    for c, k in enumerate(META_COLUMNS, start=1):
        ws_meta.cell(row=1, column=c, value=k)
    # Rows for meta
    for r, t in enumerate(tests, start=2):
        for c, k in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=r, column=c, value=to_cell_value(t.get(k, "")))
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'

    # Build TestPlan visible sheet with only MAIN_COLUMNS in specified order
    # Start by renaming Data -> TestPlanTemp, then create TestPlan
    ws_data.title = 'TestPlanTemp'
    ws = wb.create_sheet('TestPlan')

    # Header for TestPlan
    for c, k in enumerate(MAIN_COLUMNS, start=1):
        ws.cell(row=1, column=c, value=k)
    # Rows
    for r, t in enumerate(tests, start=2):
        for c, k in enumerate(MAIN_COLUMNS, start=1):
            ws.cell(row=r, column=c, value=to_cell_value(t.get(k, "")))

    # Formatting for TestPlan
    # Header formatting
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    ws.freeze_panes = 'A2'

    # Alignment and wrap for specific columns
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }
    # Determine column index types
    for col_idx in range(1, ws.max_column + 1):
        header = ws.cell(row=1, column=col_idx).value
        for row_idx in range(2, ws.max_row + 1):
            cell = ws.cell(row=row_idx, column=col_idx)
            if header == 'Index':
                cell.alignment = Alignment(horizontal='center', vertical='top', wrap_text=False)
            elif header in wrap_cols:
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=False)

    autosize(ws)
    apply_borders(ws)

    # Remove the temporary sheet
    wb.remove(wb['TestPlanTemp'])

    # Compute IST timestamp and file name
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    date_part = now_ist.strftime('%Y%m%d')
    time_part = now_ist.strftime('%H%M%S')
    ist_hms = now_ist.strftime('%Y-%m-%d %H:%M:%S')

    os.makedirs(args.output_dir, exist_ok=True)
    filename = f"{args.ip-name if False else args.ip_name}_TestPlan_{date_part}_{time_part}.xlsx"
    out_path = os.path.join(args.output_dir, filename)

    wb.save(out_path)

    # Write metadata for workflow consumption
    os.makedirs('tools', exist_ok=True)
    with open('tools/testplan_output_meta.json', 'w', encoding='utf-8') as mf:
        json.dump({
            'filename': filename,
            'ist_timestamp': ist_hms
        }, mf, ensure_ascii=False)

    print(f"Generated: {out_path}")

if __name__ == '__main__':
    main()
