#!/usr/bin/env python3
import os
import json
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill

# Inputs (embedded for deterministic fallback)
IP_NAME = "GPIO"
output_directory = os.path.join("Test_Output", "GPIO", "TestPlan")
final_json = []  # exact content must be preserved in MetaData

# Timezone: IST (Asia/Kolkata)
now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
ts_date = now_ist.strftime("%Y%m%d")
ts_time = now_ist.strftime("%H%M%S")
filename = f"{IP_NAME}_TestPlan_{ts_date}_{ts_time}.xlsx"

# Ensure output directory exists
os.makedirs(output_directory, exist_ok=True)
output_path = os.path.join(output_directory, filename)

# Create workbook
wb = Workbook()
ws = wb.active
ws.title = "TestPlan"

# Define standard Test Plan headers
headers = [
    "TestID",
    "TestName",
    "Objective",
    "Preconditions",
    "Steps",
    "ExpectedResult",
    "PassCriteria",
    "Priority",
    "Owner",
    "Notes",
]

# Styling
header_font = Font(bold=True)
header_fill = PatternFill(start_color="FFD9E1F2", end_color="FFD9E1F2", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

# Write headers
for col_idx, h in enumerate(headers, start=1):
    cell = ws.cell(row=1, column=col_idx, value=h)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Freeze first row
ws.freeze_panes = "A2"

# Reasonable column widths
widths = {
    "TestID": 12,
    "TestName": 24,
    "Objective": 36,
    "Preconditions": 36,
    "Steps": 48,
    "ExpectedResult": 36,
    "PassCriteria": 24,
    "Priority": 12,
    "Owner": 18,
    "Notes": 36,
}
for col_idx, h in enumerate(headers, start=1):
    ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = widths.get(h, 18)

# No data rows to add since final_json is empty [] in this run.

# MetaData sheet with exact JSON string, very hidden
meta = wb.create_sheet("MetaData")
meta.cell(row=1, column=1, value=json.dumps(final_json, separators=(",", ":")))  # ensures exact '[]'
meta.sheet_state = "veryHidden"

# Save workbook
wb.save(output_path)
print(f"Generated: {output_path}")
