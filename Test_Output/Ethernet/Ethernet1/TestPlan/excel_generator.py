#!/usr/bin/env python3
"""Generate Ethernet1 TestPlan Excel workbook and output base64."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import base64
import io
import json
import sys

def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    ts = now_ist.strftime('%Y%m%d_%H%M%S')
    filename = f'Ethernet1_TestPlan_{ts}.xlsx'
    
    wb = openpyxl.Workbook()
    
    # TestPlan sheet
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'
    
    tp_headers = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
                  'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
                  'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
                  'Code Generation']
    
    # MetaData sheet
    ws_md = wb.create_sheet('MetaData')
    md_headers = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
                  'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
                  'Meta Headers', 'Meta Macros', 'Meta Arrays']
    
    # Write headers
    header_font = Font(bold=True, color='FFFFFF')
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap = Alignment(wrap_text=True, vertical='top')
    
    for col_idx, h in enumerate(tp_headers, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=h)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    
    for col_idx, h in enumerate(md_headers, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=h)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    
    print(f'Generated: {filename}')
    print(f'Timestamp: {ts}')
    return filename

if __name__ == '__main__':
    generate_workbook()
