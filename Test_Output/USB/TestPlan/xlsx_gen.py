#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, sys, base64

IST = timezone(timedelta(hours=5, minutes=30))
now = datetime.now(IST)
ts = now.strftime("%Y%m%d_%H%M%S")
fn = f"USB_TestPlan_{ts}.xlsx"
fp = f"/tmp/{fn}"

wb = openpyxl.Workbook()

# TestPlan sheet
ws1 = wb.active
ws1.title = "TestPlan"

tp_headers = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]

for c, h in enumerate(tp_headers, 1):
    cell = ws1.cell(row=1, column=c, value=h)
    cell.font = Font(bold=True, color="FFFFFF")
    cell.fill = PatternFill("solid", fgColor="4472C4")
    cell.alignment = Alignment(wrap_text=True, vertical="top")

ws1.freeze_panes = "A2"

# MetaData sheet
ws2 = wb.create_sheet("MetaData")
md_headers = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

for c, h in enumerate(md_headers, 1):
    cell = ws2.cell(row=1, column=c, value=h)
    cell.font = Font(bold=True, color="FFFFFF")
    cell.fill = PatternFill("solid", fgColor="4472C4")
    cell.alignment = Alignment(wrap_text=True, vertical="top")

ws2.freeze_panes = "A2"
ws2.sheet_state = "veryHidden"

wb.save(fp)
print(f"SAVED: {fp}")
print(f"SIZE: {os.path.getsize(fp)}")

# Verify
wb2 = openpyxl.load_workbook(fp)
print(f"SHEETS: {wb2.sheetnames}")
wb2.close()

# Output base64
with open(fp, "rb") as f:
    b64 = base64.b64encode(f.read()).decode()
print(f"B64LEN: {len(b64)}")
print(f"FILENAME: {fn}")
