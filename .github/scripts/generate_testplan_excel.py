#!/usr/bin/env python3
# Ethernet TestPlan Generator - Auto-triggered by GitHub Actions
import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")

# Load JSON data from file
script_dir = os.path.dirname(os.path.abspath(__file__))
repo_root = os.path.join(script_dir, '..', '..')
json_path = os.path.join(repo_root, 'Test_Output', 'Ethernet', 'Ethernet0', 'TestPlan', 'testplan_data.json')

with open(json_path, 'r') as f:
    json_data = json.load(f)

print(f"Loaded {len(json_data)} testcases from {json_path}")

# TestPlan sheet columns
testplan_columns = [
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
    "Code Generation"
]

# MetaData sheet columns
metadata_columns = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

# Create workbook
wb = Workbook()

# Create TestPlan sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"

# Create MetaData sheet
ws_md = wb.create_sheet("MetaData")

# Header formatting
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write TestPlan headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Populate TestPlan rows
for row_idx, tc in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = tc.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Populate MetaData rows
for row_idx, tc in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = tc.get(col_name, "")
        if value is None:
            value = ""
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Auto-size columns with max width cap
def auto_size_columns(ws, max_width=60):
    for col in ws.columns:
        max_length = 0
        column_letter = get_column_letter(col[0].column)
        for cell in col:
            try:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
            except:
                pass
        adjusted_width = min(max_length + 2, max_width)
        if adjusted_width < 12:
            adjusted_width = 12
        ws.column_dimensions[column_letter].width = adjusted_width

auto_size_columns(ws_tp)
auto_size_columns(ws_md)

# Freeze first row
ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save workbook
output_dir = os.path.join(repo_root, "Test_Output", "Ethernet", "Ethernet0", "TestPlan")
os.makedirs(output_dir, exist_ok=True)
filename = f"Ethernet_TestPlan_{timestamp_str}.xlsx"
filepath = os.path.join(output_dir, filename)
wb.save(filepath)

# Validate
assert os.path.exists(filepath), "File does not exist!"
assert os.path.getsize(filepath) > 0, "File is empty!"

# Re-open to validate
wb2 = load_workbook(filepath)
assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing!"
assert "MetaData" in wb2.sheetnames, "MetaData sheet missing!"

# Validate MetaData content
meta_fields = [
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

ws_md2 = wb2["MetaData"]
for row_idx, tc in enumerate(json_data, 2):
    for field in meta_fields:
        col_idx = metadata_columns.index(field) + 1
        cell_value = ws_md2.cell(row=row_idx, column=col_idx).value
        expected = tc.get(field, "")
        if expected is None:
            expected = ""
        assert cell_value == expected, f"Validation FAILED for row {row_idx}, field '{field}': cell value does not match json_data"

print(f"SUCCESS: {filepath}")
print(f"Filename: {filename}")
print(f"Rows TestPlan: {len(json_data)}")
print(f"Rows MetaData: {len(json_data)}")
print("Validation: PASSED")
