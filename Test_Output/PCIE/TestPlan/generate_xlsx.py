#!/usr/bin/env python3
"""PCIE TestPlan XLSX Generator - Agent 7"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os
import base64

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"PCIE_TestPlan_{timestamp}.xlsx"

# Test case data
test_cases = json.loads('''PLACEHOLDER''')

# Create workbook
wb = openpyxl.Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_headers = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
              "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
              "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
              "Code Generation"]

# MetaData sheet
ws_md = wb.create_sheet("MetaData")
md_headers = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
              "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
              "Meta Headers", "Meta Macros", "Meta Arrays"]

# Formatting
header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for col_idx, header in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Populate data
for idx, tc in enumerate(test_cases, 1):
    reg_info = tc.get("Register_Name", "NA")
    macro_info = tc.get("Register_Macro", "NA")
    impacted = f"{reg_info} ({macro_info})" if reg_info != "NA" else macro_info

    tp_row = [
        idx,
        tc.get("Register_Group", ""),
        tc.get("Test_Type", ""),
        f"{tc.get('Test_Case_ID', '')} - {tc.get('Test_Name', '')}",
        tc.get("Test_Description", ""),
        "NA",
        tc.get("Access_Type", ""),
        tc.get("Register_Address_Offset", ""),
        "NA",
        tc.get("Spec_Reference", ""),
        tc.get("Test_Steps", ""),
        impacted,
        tc.get("Pass_Fail_Criteria", ""),
        "NA"
    ]
    for col_idx, value in enumerate(tp_row, 1):
        cell = ws_tp.cell(row=idx+1, column=col_idx, value=value)
        cell.alignment = wrap_alignment

    md_row = [
        idx,
        f"{tc.get('Test_Case_ID', '')} - {tc.get('Test_Name', '')}",
        tc.get("Test_Description", ""),
        tc.get("Test_Steps", ""),
        tc.get("Register_Name", ""),
        tc.get("Expected_Result", ""),
        tc.get("Block_Base_Address", ""),
        tc.get("Register_Macro", ""),
        f"{tc.get('Write_Mask', '')}; Default={tc.get('Default_Reset_Value', '')}; Pattern={tc.get('Test_Pattern', '')}"
    ]
    for col_idx, value in enumerate(md_row, 1):
        cell = ws_md.cell(row=idx+1, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Auto-size columns
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                max_length = max(max_length, len(str(cell.value)))
        adjusted_width = min(max_length + 2, 60)
        ws.column_dimensions[col_letter].width = adjusted_width

# Freeze first row
ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

# Very hidden MetaData
ws_md.sheet_state = "veryHidden"

# Save
filepath = f"/tmp/{filename}"
wb.save(filepath)

# Validate
file_size = os.path.getsize(filepath)
wb2 = openpyxl.load_workbook(filepath)
sheets = wb2.sheetnames
print(f"FILENAME={filename}")
print(f"FILEPATH={filepath}")
print(f"FILESIZE={file_size}")
print(f"SHEETS={sheets}")
print(f"ROWS_TP={ws_tp.max_row - 1}")
print(f"ROWS_MD={ws_md.max_row - 1}")
print("VALIDATION=PASSED")

# Output base64 for upload
with open(filepath, "rb") as f:
    b64 = base64.b64encode(f.read()).decode()
print(f"BASE64_LENGTH={len(b64)}")
