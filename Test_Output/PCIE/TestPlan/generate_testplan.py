#!/usr/bin/env python3
"""
Auto-generated TestPlan Excel Generator for PCIE IP.
Run this script to produce PCIE_TestPlan_<timestamp>.xlsx
"""
import json, os, sys
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
ts = now_ist.strftime("%Y%m%d_%H%M%S")
FILENAME = f"PCIE_TestPlan_{ts}.xlsx"

json_data = [
  {
    "index": 1,
    "SS_Module": "PCIE",
    "Test_Case_Name": "pcie_reg_wr_rd_test",
    "Feature": "Register Read/Write Validation",
    "Test_Description": "This test validates the reset default values and read-write accessibility of PCIe controller registers across both PCIE0 and PCIE1 instances. It covers DBI DSP registers (MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG, FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, UTILITY_OFF), SII registers (Transmit Header2, Transmit Header3, PHY Control 23), PHY reset control registers, and PHY lane registers. The test first reads all registers and compares against expected default values of 0x0. Then it performs write-read-back verification using multiple data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555) with appropriate write masks applied for SII and PHY registers.",
    "Meta_Test_Description": "This testcase validates reset default values and read-write functionality of PCIe registers for both PCIE0 and PCIE1 controller instances.",
    "Test_Steps_Procedure": "1. Read all five PCIE0 DBI DSP registers and verify default 0x0.\n2. Read all five PCIE1 DBI DSP registers and verify default 0x0.\n3. Read all three PCIE0 SII registers and verify default 0x0.\n4. Read all three PCIE1 SII registers and verify default 0x0.\n5. Write PHY reset control 0x01203000.\n6. Read PHY lane registers and verify default 0x0.\n7. Write patterns and read-back verify.\n8. Report pass/fail.",
    "Meta_Test_Steps_Procedure": "Detailed meta steps...",
    "Validation_Acceptance_Criteria": "All registers must read back expected values.",
    "Impacted_Registers": "MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; FILTER_MASK_2_OFF; AXI_MSTR_MSG_ADDR_HIGH_OFF; UTILITY_OFF",
    "Meta_Impacted_Registers": "Full list of 24 register tokens",
    "Speed": "NA",
    "Mode": "NA",
    "Memory_Offset": "0x58; 0x60; 0x720; 0x8F4; 0xC80",
    "Remarks": "Test covers PCIE0 and PCIE1 instances.",
    "Headers_Include": "#include <stdlib.h>; #include <stdio.h>; #include <test_common.h>; #include <pcie.h>"
  }
]

wb = Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

tp_headers = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation"]
md_headers = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]

hdr_font = Font(bold=True, color="FFFFFF")
hdr_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap = Alignment(wrap_text=True, vertical="top")

for ci, h in enumerate(tp_headers, 1):
    c = ws_tp.cell(row=1, column=ci, value=h)
    c.font = hdr_font; c.fill = hdr_fill; c.alignment = wrap

for ci, h in enumerate(md_headers, 1):
    c = ws_md.cell(row=1, column=ci, value=h)
    c.font = hdr_font; c.fill = hdr_fill; c.alignment = wrap

for ri, row in enumerate(json_data, 2):
    ws_tp.cell(row=ri, column=1, value=row["index"]).alignment = wrap
    ws_tp.cell(row=ri, column=2, value=row["SS_Module"]).alignment = wrap
    ws_tp.cell(row=ri, column=3, value=row["Feature"]).alignment = wrap
    ws_tp.cell(row=ri, column=4, value=row["Test_Case_Name"]).alignment = wrap
    ws_tp.cell(row=ri, column=5, value=row["Test_Description"]).alignment = wrap
    ws_tp.cell(row=ri, column=6, value=row["Speed"]).alignment = wrap
    ws_tp.cell(row=ri, column=7, value=row["Mode"]).alignment = wrap
    ws_tp.cell(row=ri, column=8, value=row["Memory_Offset"]).alignment = wrap
    ws_tp.cell(row=ri, column=9, value="").alignment = wrap
    ws_tp.cell(row=ri, column=10, value=row["Remarks"]).alignment = wrap
    ws_tp.cell(row=ri, column=11, value=row["Test_Steps_Procedure"]).alignment = wrap
    ws_tp.cell(row=ri, column=12, value=row["Impacted_Registers"]).alignment = wrap
    ws_tp.cell(row=ri, column=13, value=row["Validation_Acceptance_Criteria"]).alignment = wrap
    ws_tp.cell(row=ri, column=14, value="").alignment = wrap

    ws_md.cell(row=ri, column=1, value=row["index"]).alignment = wrap
    ws_md.cell(row=ri, column=2, value=row["Test_Case_Name"]).alignment = wrap
    ws_md.cell(row=ri, column=3, value=row["Meta_Test_Description"]).alignment = wrap
    ws_md.cell(row=ri, column=4, value=row["Meta_Test_Steps_Procedure"]).alignment = wrap
    ws_md.cell(row=ri, column=5, value=row["Meta_Impacted_Registers"]).alignment = wrap
    ws_md.cell(row=ri, column=6, value=row["Validation_Acceptance_Criteria"]).alignment = wrap
    ws_md.cell(row=ri, column=7, value=row["Headers_Include"]).alignment = wrap
    ws_md.cell(row=ri, column=8, value="").alignment = wrap
    ws_md.cell(row=ri, column=9, value="").alignment = wrap

ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"
ws_md.sheet_state = "veryHidden"

for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        mx = 0
        for cell in col:
            if cell.value:
                mx = max(mx, min(len(str(cell.value)), 60))
        ws.column_dimensions[col[0].column_letter].width = max(mx + 2, 12)

script_dir = os.path.dirname(os.path.abspath(__file__))
out = os.path.join(script_dir, FILENAME)
wb.save(out)
print(f"Generated: {out}")
print(f"Filename: {FILENAME}")
