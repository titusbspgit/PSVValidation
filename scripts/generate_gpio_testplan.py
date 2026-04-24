import json
import os
from datetime import datetime
import pytz
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# Inputs and constants
REPO_ROOT = os.getcwd()
OUTPUT_DIR = os.path.join(REPO_ROOT, 'Test_Output', 'GPIO', 'TestPlan')
JSON_PATH = os.path.join(REPO_ROOT, 'scripts', 'json_input_gpio_testplan.json')
BLUE_FILL = PatternFill(fill_type='solid', fgColor='FF4F81BD')  # Solid blue header
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))

# Stage1 main and hidden columns
MAIN_COLUMNS = [
    'Index',
    'SS / Module',
    'Feature',
    'Test Case Name',
    'Test Description',
    'Speed',
    'Mode',
    'Memory Start Offset',
    'Memory End Offset',
    'Remarks',
    'Test Steps / Procedure',
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]
HIDDEN_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]

# Helpers

def to_cell_value(v):
    if isinstance(v, (dict, list)):
        return json.dumps(v, ensure_ascii=False)
    return v if v is not None else ''


def auto_width(ws):
    for col_idx in range(1, ws.max_column + 1):
        col_letter = get_column_letter(col_idx)
        max_len = 0
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            v = row[0].value
            s = str(v) if v is not None else ''
            if len(s) > max_len:
                max_len = len(s)
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 120)


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def set_row_heights(ws, wrap_cols_idx):
    base = 15
    for r in range(2, ws.max_row + 1):
        lines = 1
        for c in wrap_cols_idx:
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            s = str(v)
            n = s.count('\n') + 1
            if n > lines:
                lines = n
        ws.row_dimensions[r].height = base * min(lines, 30)  # cap to avoid extremes


# Load JSON
with open(JSON_PATH, 'r', encoding='utf-8') as f:
    data = json.load(f)

if not isinstance(data, dict) or 'TestCases' not in data or not isinstance(data['TestCases'], list):
    raise SystemExit('Invalid JSON: Expected object with TestCases array')

meta = data.get('META_DATA', {})
rows = data['TestCases']

# Build union of keys in order of first appearance
union_keys = []
seen = set()
for rec in rows:
    for k in rec.keys():
        if k not in seen:
            seen.add(k)
            union_keys.append(k)

# Create workbook and base Data sheet
wb = Workbook()
ws = wb.active
ws.title = 'Data'

# Headers
for c, h in enumerate(union_keys, start=1):
    cell = ws.cell(row=1, column=c, value=h)
    cell.font = Font(bold=True)

# Rows
for r_idx, rec in enumerate(rows, start=2):
    for c, h in enumerate(union_keys, start=1):
        ws.cell(row=r_idx, column=c, value=to_cell_value(rec.get(h, '')))

# Freeze top row
ws.freeze_panes = 'A2'

# Create Meta_data_sheet with META and Hidden columns
meta_ws = wb.create_sheet('Meta_data_sheet')
# Top-level META_DATA as key/value
meta_ws.append(['Field', 'Value'])
for k in meta.keys():
    meta_ws.append([k, to_cell_value(meta[k])])

# Add a blank row separator
meta_ws.append(['', ''])

# Hidden columns table header
meta_ws.append(HIDDEN_COLUMNS)
for rec in rows:
    meta_ws.append([to_cell_value(rec.get(h, '')) for h in HIDDEN_COLUMNS])

# Convert Data -> TestPlan by removing extra columns and ordering MAIN columns
# Map current headers to indices
header_map = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}

# Create a new sheet to ensure exact ordering
plan = wb.create_sheet('TestPlan')
# Write headers in final order
for c, h in enumerate(MAIN_COLUMNS, start=1):
    plan.cell(row=1, column=c, value=h)

# Copy rows
for r in range(2, ws.max_row + 1):
    for c, h in enumerate(MAIN_COLUMNS, start=1):
        src_col = header_map.get(h)
        val = ws.cell(row=r, column=src_col).value if src_col else ''
        plan.cell(row=r, column=c, value=val)

# Enable AutoFilter across the data range
plan.auto_filter.ref = f"A1:{get_column_letter(plan.max_column)}{plan.max_row}"

# Apply formatting to TestPlan only
# Header styling
for c in range(1, plan.max_column + 1):
    cell = plan.cell(row=1, column=c)
    cell.font = Font(bold=True)
    cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
    cell.fill = BLUE_FILL

# Data rows alignment
for r in range(2, plan.max_row + 1):
    for c in range(1, plan.max_column + 1):
        align = Alignment(vertical='top', horizontal='left', wrap_text=False)
        if c == 1:  # Index
            align = Alignment(vertical='top', horizontal='center', wrap_text=False)
        plan.cell(row=r, column=c).alignment = align

# Enable wrapping on specific columns
wrap_headers = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria',
}
wrap_cols_idx = []
for c in range(1, plan.max_column + 1):
    h = plan.cell(row=1, column=c).value
    if h in wrap_headers:
        wrap_cols_idx.append(c)
        for r in range(2, plan.max_row + 1):
            plan.cell(row=r, column=c).alignment = Alignment(vertical='top', horizontal='left', wrap_text=True)

# Borders
apply_borders(plan)

# Freeze top row
plan.freeze_panes = 'A2'

# Data validation for Code Generation (Required / Not)
try:
    code_col_idx = MAIN_COLUMNS.index('Code Generation (Required / Not)') + 1
    dv = DataValidation(type='list', formula1='"Required,Not Required"', allow_blank=True, showDropDown=True)
    rng = f"{get_column_letter(code_col_idx)}2:{get_column_letter(code_col_idx)}{plan.max_row}"
    dv.add(rng)
    plan.add_data_validation(dv)
except ValueError:
    pass

# Hyperlink Test Case Name to first Source_Artifacts URL if present
# Build a lookup of row index to first URL from original rows
source_urls = []
for rec in rows:
    urls = rec.get('Source_Artifacts') or []
    first = urls[0] if isinstance(urls, list) and len(urls) > 0 else None
    source_urls.append(first)

name_col_idx = MAIN_COLUMNS.index('Test Case Name') + 1
for i, url in enumerate(source_urls, start=2):
    if i <= plan.max_row and url:
        cell = plan.cell(row=i, column=name_col_idx)
        cell.hyperlink = url
        cell.font = Font(color='0000EE', underline='single')

# Auto-fit columns and adjust row heights
auto_width(plan)
set_row_heights(plan, wrap_cols_idx)

# Compute IST timestamp and filename
ist = pytz.timezone('Asia/Kolkata')
now_ist = datetime.now(ist)
stamp_date = now_ist.strftime('%Y%m%d')
stamp_time = now_ist.strftime('%H%M%S')
filename = f'GPIO_TestPlan_{stamp_date}_{stamp_time}.xlsx'

# Add IST timestamp to META sheet and then hide it
meta_ws.append(['Generated_Timestamp_IST', now_ist.strftime('%Y-%m-%d %H:%M:%S')])
meta_ws.sheet_state = 'veryHidden'

# Remove original Data sheet
wb.remove(ws)

# Ensure output dir
os.makedirs(OUTPUT_DIR, exist_ok=True)
output_path = os.path.join(OUTPUT_DIR, filename)

# Save workbook
wb.save(output_path)

# Persist generated file path and timestamp for subsequent commit step
with open(os.path.join(REPO_ROOT, 'scripts', '.last_generated_path.txt'), 'w', encoding='utf-8') as f:
    f.write(os.path.relpath(output_path, REPO_ROOT))
with open(os.path.join(REPO_ROOT, 'scripts', '.last_generated_ts_ist.txt'), 'w', encoding='utf-8') as f:
    f.write(now_ist.strftime('%Y-%m-%d %H:%M:%S'))

print(f'Generated: {output_path}')
