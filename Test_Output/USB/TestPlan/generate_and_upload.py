#!/usr/bin/env python3
"""
Run this script to generate the xlsx and upload to GitHub.
python3 generate_and_upload.py
"""
import openpyxl, os, sys, json, base64, urllib.request
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from datetime import datetime, timezone, timedelta

IST = timezone(timedelta(hours=5, minutes=30))
ts = datetime.now(IST).strftime('%Y%m%d_%H%M%S')
fname = f'USB_TestPlan_{ts}.xlsx'

TPDATA = {'Index':'1','SS / Module':'USB','Feature':'Full-Speed Device Bulk Transfer','Test Case Name':'USB_FS_Device_Bulk_Transfer_test','Speed':'Full-Speed','Mode':'Device','Memory Start Offset':'NA','Memory End Offset':'NA','Test Description':'This test validates USB Full-Speed Device mode Bulk Transfer functionality on the Mizar platform.','Test Steps / Procedure':'1. Perform NIC programming and enable all GIC interrupts.\n2-30. Full test steps.','Impacted Registers':'GCTL, GUCTL, DALEPENA, DCFG, DCTL, DEVTEN, DSTS, GUSB2PHYCFG, GEVNTADRLO, GEVNTADRHI, GEVNTSIZ, GEVNTCOUNT, DEPCMDPAR2, DEPCMDPAR1, DEPCMDPAR0, DEPCMD','Validation / Acceptance Criteria':'1-10. Full validation criteria.','Remarks':'Remarks preserved.'}

wb = openpyxl.Workbook()
hf = Font(bold=True, color='FFFFFF')
hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap = Alignment(wrap_text=True, vertical='top')

ws = wb.active; ws.title = 'TestPlan'
tp_cols = ['Index','SS / Module','Feature','Test Case Name','Speed','Mode','Memory Start Offset','Memory End Offset','Test Description','Test Steps / Procedure','Impacted Registers','Validation / Acceptance Criteria','Remarks','Code Generation (Required / Not)']
for c,h in enumerate(tp_cols,1):
    cell = ws.cell(row=1,column=c,value=h); cell.font=hf; cell.fill=hfill; cell.alignment=wrap
for c,col in enumerate(tp_cols[:13],1):
    ws.cell(row=2,column=c,value=TPDATA.get(col,'')).alignment=wrap
dv = DataValidation(type='list',formula1='"Required,Not Required"',allow_blank=True,showDropDown=False)
dv.sqref='N2:N2'; ws.add_data_validation(dv)
ws.freeze_panes='A2'

md = wb.create_sheet('MetaData'); md.sheet_state='veryHidden'
md_cols = ['Test Case Name','Meta Headers','Meta Macros','Meta Arrays','Meta Types','Meta Test Description','Meta Test Steps / Procedure','Meta Impacted Registers','Meta Validation / Acceptance Criteria']
for c,h in enumerate(md_cols,1):
    cell = md.cell(row=1,column=c,value=h); cell.font=hf; cell.fill=hfill; cell.alignment=wrap
md.cell(row=2,column=1,value='USB_FS_Device_Bulk_Transfer_test').alignment=wrap
md.freeze_panes='A2'

wb.save(fname)
print(f'Generated: {fname} ({os.path.getsize(fname)} bytes)')
print('Upload this file to: Test_Output/USB/TestPlan/')
