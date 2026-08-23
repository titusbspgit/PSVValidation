#!/usr/bin/env python3
import json
import os
from datetime import datetime, timezone, timedelta

try:
    import openpyxl
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
except ImportError:
    import subprocess
    subprocess.check_call(['pip', 'install', 'openpyxl'])
    import openpyxl
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')

script_dir = os.path.dirname(os.path.abspath(__file__))
json_path = os.path.join(script_dir, 'PCIE_TestPlan_data.json')

with open(json_path, 'r') as f:
    data = json.load(f)

wb = openpyxl.Workbook()

# --- Main Sheet ---
ws_main = wb.active
ws_main.title = 'Main'

main_headers = [
    'Index', 'SS / Module', 'Test Case Name', 'Feature',
    'Test Description', 'Test Steps / Procedure',
    'Impacted Registers', 'Validation / Acceptance Criteria', 'Remarks'
]

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_alignment = Alignment(vertical='top', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)

for col_idx, header in enumerate(main_headers, 1):
    cell = ws_main.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

for row_idx, item in enumerate(data, 2):
    values = [
        item.get('Index', 'NA'),
        item.get('SS / Module', 'NA'),
        item.get('Test Case Name', 'NA'),
        item.get('Feature', 'NA'),
        item.get('Test Description', 'NA'),
        item.get('Test Steps / Procedure', 'NA'),
        item.get('Impacted Registers', 'NA'),
        item.get('Validation / Acceptance Criteria', 'NA'),
        item.get('Remarks', 'NA')
    ]
    for col_idx, value in enumerate(values, 1):
        cell = ws_main.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Column widths for Main
main_widths = [8, 15, 30, 25, 60, 60, 40, 60, 40]
for col_idx, width in enumerate(main_widths, 1):
    ws_main.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = width

# --- MetaData Sheet ---
ws_meta = wb.create_sheet('MetaData')

meta_headers = [
    'Index', 'SS / Module', 'Test Case Name', 'Feature',
    'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers'
]

meta_fill = PatternFill(start_color='ED7D31', end_color='ED7D31', fill_type='solid')

for col_idx, header in enumerate(meta_headers, 1):
    cell = ws_meta.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = meta_fill
    cell.alignment = header_alignment
    cell.border = thin_border

for row_idx, item in enumerate(data, 2):
    values = [
        item.get('Index', 'NA'),
        item.get('SS / Module', 'NA'),
        item.get('Test Case Name', 'NA'),
        item.get('Feature', 'NA'),
        item.get('Meta Test Description', 'NA'),
        item.get('Meta Test Steps / Procedure', 'NA'),
        item.get('Meta Impacted Registers', 'NA')
    ]
    for col_idx, value in enumerate(values, 1):
        cell = ws_meta.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Column widths for MetaData
meta_widths = [8, 15, 30, 25, 80, 80, 60]
for col_idx, width in enumerate(meta_widths, 1):
    ws_meta.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = width

output_filename = f'PCIE_TestPlan_{timestamp}.xlsx'
output_path = os.path.join(script_dir, output_filename)
wb.save(output_path)
print(f'Generated: {output_filename}')
print(f'Path: {output_path}')
