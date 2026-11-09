#!/usr/bin/env python3
"""Agent 7 - MIPI_DSI TestPlan Excel Generator
Generates MIPI_DSI_TestPlan_20261109_180000.xlsx with TestPlan and MetaData sheets.
Run: pip install openpyxl && python generate_MIPI_DSI_TestPlan_20261109_180000.py
Then push the generated .xlsx to GitHub.
"""
import json, os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TP_JSON = os.path.join(SCRIPT_DIR, 'MIPI_DSI_TestPlan_20261109_180000_testplan.json')
META_JSON = os.path.join(SCRIPT_DIR, 'MIPI_DSI_TestPlan_20261109_180000_metadata.json')
OUT_XLSX = os.path.join(SCRIPT_DIR, 'MIPI_DSI_TestPlan_20261109_180000.xlsx')

TP_COLS = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Speed', 'Mode',
    'Memory Start Offset', 'Memory End Offset', 'Test Description',
    'Test Steps / Procedure', 'Impacted Registers',
    'Validation / Acceptance Criteria', 'Remarks',
    'Code Generation (Required / Not)'
]

META_COLS = [
    'Test Case Name', 'Meta Headers', 'Meta Macros', 'Meta Arrays',
    'Meta Types', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria'
]

HDR_FONT = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
HDR_FILL = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
WRAP = Alignment(wrap_text=True, vertical='top')

def auto_width(ws, max_w=60):
    for col in ws.columns:
        mx = 0
        letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                mx = max(mx, max(len(l) for l in lines))
        ws.column_dimensions[letter].width = min(max(mx + 2, 12), max_w)

def build():
    with open(TP_JSON, 'r', encoding='utf-8') as f:
        tp_data = json.load(f)
    with open(META_JSON, 'r', encoding='utf-8') as f:
        meta_data = json.load(f)

    wb = Workbook()

    # --- TestPlan sheet ---
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'
    ws_tp.freeze_panes = 'A2'
    for ci, h in enumerate(TP_COLS, 1):
        c = ws_tp.cell(row=1, column=ci, value=h)
        c.font = HDR_FONT
        c.fill = HDR_FILL
        c.alignment = WRAP

    for ri, rec in enumerate(tp_data, 2):
        for ci, h in enumerate(TP_COLS, 1):
            if h == 'Code Generation (Required / Not)':
                ws_tp.cell(row=ri, column=ci).alignment = WRAP
            else:
                val = rec.get(h, '')
                c = ws_tp.cell(row=ri, column=ci, value=val)
                c.alignment = WRAP

    dv = DataValidation(
        type='list', formula1='"Required,Not Required"',
        allow_blank=True, showDropDown=False
    )
    dv.error = 'Please select Required or Not Required'
    dv.errorTitle = 'Invalid Entry'
    dv.prompt = 'Select Required or Not Required'
    dv.promptTitle = 'Code Generation'
    code_col = len(TP_COLS)
    code_letter = get_column_letter(code_col)
    last_row = len(tp_data) + 1
    dv.sqref = f'{code_letter}2:{code_letter}{last_row}'
    ws_tp.add_data_validation(dv)
    auto_width(ws_tp)

    # --- MetaData sheet ---
    ws_meta = wb.create_sheet('MetaData')
    ws_meta.sheet_state = 'veryHidden'
    ws_meta.freeze_panes = 'A2'
    for ci, h in enumerate(META_COLS, 1):
        c = ws_meta.cell(row=1, column=ci, value=h)
        c.font = HDR_FONT
        c.fill = HDR_FILL
        c.alignment = WRAP

    for ri, rec in enumerate(meta_data, 2):
        for ci, h in enumerate(META_COLS, 1):
            val = rec.get(h, '')
            c = ws_meta.cell(row=ri, column=ci, value=val)
            c.alignment = WRAP
    auto_width(ws_meta)

    wb.save(OUT_XLSX)
    print(f'Workbook saved: {OUT_XLSX}')
    print(f'TestPlan rows: {len(tp_data)}')
    print(f'MetaData rows: {len(meta_data)}')
    sz = os.path.getsize(OUT_XLSX)
    print(f'File size: {sz} bytes')
    assert sz > 0, 'File size is 0!'
    # Reopen to validate
    from openpyxl import load_workbook
    vwb = load_workbook(OUT_XLSX)
    assert 'TestPlan' in vwb.sheetnames
    assert 'MetaData' in vwb.sheetnames
    tp_rows = vwb['TestPlan'].max_row - 1
    meta_rows = vwb['MetaData'].max_row - 1
    assert tp_rows == len(tp_data), f'TestPlan row mismatch: {tp_rows} vs {len(tp_data)}'
    assert meta_rows == len(meta_data), f'MetaData row mismatch: {meta_rows} vs {len(meta_data)}'
    print('Validation PASSED')
    return OUT_XLSX

if __name__ == '__main__':
    build()
