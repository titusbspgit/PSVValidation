#!/usr/bin/env python3
"""Run: pip install openpyxl && python3 gen_xlsx.py"""
import os, sys, base64
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
ts = datetime.now(IST).strftime("%Y%m%d_%H%M%S")
fn = f"MIPI_TestPlan_{ts}.xlsx"
fp = os.path.join(os.path.dirname(os.path.abspath(__file__)), fn)

wb = Workbook()
# TestPlan sheet
ws1 = wb.active
ws1.title = "TestPlan"
tp_cols = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
ws1.append(tp_cols)

# MetaData sheet
ws2 = wb.create_sheet("MetaData")
md_cols = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]
ws2.append(md_cols)

# Data rows - see full data in calling context
# This script is a template; actual data populated by the workflow
hdr_font = Font(bold=True, color="FFFFFF")
hdr_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

for ws in [ws1, ws2]:
    for cell in ws[1]:
        cell.font = hdr_font
        cell.fill = hdr_fill
        cell.alignment = wrap
    ws.freeze_panes = "A2"

wb.save(fp)
print(f"GENERATED: {fn}")
print(f"PATH: {fp}")
print(f"SIZE: {os.path.getsize(fp)} bytes")

with open(fp, "rb") as f:
    b64 = base64.b64encode(f.read()).decode()
print(f"BASE64_LENGTH: {len(b64)}")
