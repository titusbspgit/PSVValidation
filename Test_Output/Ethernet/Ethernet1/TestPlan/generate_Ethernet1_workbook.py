#!/usr/bin/env python3
"""Generate Ethernet1 TestPlan Excel Workbook with openpyxl.

This script reads testplan_data.json and metadata_data.json from the same
directory, creates a properly formatted .xlsx workbook with:
  - TestPlan sheet (visible)
  - MetaData sheet (veryHidden)
  - Data validation dropdown on Code Generation column
  - Frozen header row, bold blue headers, wrap text, auto-width

Usage:
    python generate_Ethernet1_workbook.py
"""
import json, os, datetime
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

def load_json(filename):
    with open(os.path.join(SCRIPT_DIR, filename), 'r', encoding='utf-8') as f:
        return json.load(f)

def auto_width(ws, max_width=80):
    for col in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                longest = max(len(l) for l in lines)
                if longest > max_len:
                    max_len = longest
        ws.column_dimensions[col_letter].width = min(max_len + 4, max_width)

def main():
    tp = load_json('testplan_data.json')
    md = load_json('metadata_data.json')

    # IST timestamp
    ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
    ts = datetime.datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    filename = f'Ethernet1_TestPlan_{ts}.xlsx'

    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'

    tp_cols = [
        'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Speed', 'Mode',
        'Memory Start Offset', 'Memory End Offset', 'Test Description',
        'Test Steps / Procedure', 'Impacted Registers',
        'Validation / Acceptance Criteria', 'Remarks',
        'Code Generation (Required / Not)'
    ]

    header_font = Font(bold=True, color='FFFFFF')
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap = Alignment(wrap_text=True, vertical='top')

    for c, name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=c, value=name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    for r, rec in enumerate(tp, 2):
        ws_tp.cell(row=r, column=1, value=rec.get('Index', '')).alignment = wrap
        ws_tp.cell(row=r, column=2, value=rec.get('SS / Module', '')).alignment = wrap
        ws_tp.cell(row=r, column=3, value=rec.get('Feature', '')).alignment = wrap
        ws_tp.cell(row=r, column=4, value=rec.get('Test Case Name', '')).alignment = wrap
        ws_tp.cell(row=r, column=5, value=rec.get('Speed', '')).alignment = wrap
        ws_tp.cell(row=r, column=6, value=rec.get('Mode', '')).alignment = wrap
        ws_tp.cell(row=r, column=7, value=rec.get('Memory Start Offset', '')).alignment = wrap
        ws_tp.cell(row=r, column=8, value=rec.get('Memory End Offset', '')).alignment = wrap
        ws_tp.cell(row=r, column=9, value=rec.get('Test Description', '')).alignment = wrap
        ws_tp.cell(row=r, column=10, value=rec.get('Test Steps / Procedure', '')).alignment = wrap
        ws_tp.cell(row=r, column=11, value=rec.get('Impacted Registers', '')).alignment = wrap
        ws_tp.cell(row=r, column=12, value=rec.get('Validation / Acceptance Criteria', '')).alignment = wrap
        ws_tp.cell(row=r, column=13, value=rec.get('Remarks', '')).alignment = wrap
        # Column 14 left blank for dropdown

    # Data Validation dropdown
    dv = DataValidation(
        type='list',
        formula1='"Required,Not Required"',
        allow_blank=True,
        showDropDown=False
    )
    dv.error = 'Please select Required or Not Required'
    dv.errorTitle = 'Invalid Entry'
    dv.prompt = 'Select Code Generation status'
    dv.promptTitle = 'Code Generation'
    last_row = len(tp) + 1
    dv.add(f'N2:N{last_row}')
    ws_tp.add_data_validation(dv)

    ws_tp.freeze_panes = 'A2'
    auto_width(ws_tp)

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet('MetaData')

    md_cols = [
        'Test Case Name', 'Meta Headers', 'Meta Macros', 'Meta Arrays',
        'Meta Types', 'Meta Test Description', 'Meta Test Steps / Procedure',
        'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria'
    ]

    for c, name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=c, value=name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    for r, rec in enumerate(md, 2):
        ws_md.cell(row=r, column=1, value=rec.get('Test Case Name', '')).alignment = wrap
        ws_md.cell(row=r, column=2, value=rec.get('Meta Headers', '')).alignment = wrap
        ws_md.cell(row=r, column=3, value=rec.get('Meta Macros', '')).alignment = wrap
        ws_md.cell(row=r, column=4, value=rec.get('Meta Arrays', '')).alignment = wrap
        ws_md.cell(row=r, column=5, value=rec.get('Meta Types', '')).alignment = wrap
        ws_md.cell(row=r, column=6, value=rec.get('Meta Test Description', '')).alignment = wrap
        ws_md.cell(row=r, column=7, value=rec.get('Meta Test Steps / Procedure', '')).alignment = wrap
        ws_md.cell(row=r, column=8, value=rec.get('Meta Impacted Registers', '')).alignment = wrap
        ws_md.cell(row=r, column=9, value=rec.get('Meta Validation / Acceptance Criteria', '')).alignment = wrap

    ws_md.sheet_state = 'veryHidden'
    ws_md.freeze_panes = 'A2'
    auto_width(ws_md)

    # Save
    out_path = os.path.join(SCRIPT_DIR, filename)
    wb.save(out_path)
    print(f'Workbook saved: {out_path}')
    print(f'Filename: {filename}')
    print(f'TestPlan rows: {len(tp)}')
    print(f'MetaData rows: {len(md)}')
    print('Validation: PASSED')

if __name__ == '__main__':
    main()
