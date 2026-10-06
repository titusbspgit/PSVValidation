#!/usr/bin/env python3
"""
Generate MIPI_CSI TestPlan Excel workbook.
Run this script to produce the XLSX file.
Requirements: openpyxl
Usage: python3 generate_and_push.py
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import base64

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)

wb = openpyxl.Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

tp_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# Header formatting
blue_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
white_font = Font(color='FFFFFF', bold=True)
wrap_alignment = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.fill = blue_fill
    cell.font = white_font
    cell.alignment = wrap_alignment

ws_tp.freeze_panes = 'A2'

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

md_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.fill = blue_fill
    cell.font = white_font
    cell.alignment = wrap_alignment

ws_md.freeze_panes = 'A2'
ws_md.sheet_state = 'veryHidden'

wb.save(output_path)
print(f'Generated: {output_path}')
print(f'File size: {os.path.getsize(output_path)} bytes')

# Encode to base64 for verification
with open(output_path, 'rb') as f:
    b64 = base64.b64encode(f.read()).decode()
print(f'Base64 length: {len(b64)}')
