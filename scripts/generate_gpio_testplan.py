import os, json, math, zipfile
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

IP_NAME = os.environ.get('IP_NAME', 'GPIO')
OUTPUT_DIR = os.environ.get('OUTPUT_DIR', 'Test_Output/GPIO/TestPlan')
JSON_DATA = os.environ.get('JSON_DATA', '').strip()

if not JSON_DATA:
    raise SystemExit('JSON_DATA environment variable is empty')

try:
    raw = json.loads(JSON_DATA)
except Exception as e:
    raise SystemExit(f'Invalid JSON input: {e}')

# Normalize to array of row dicts preserving order of keys and TCs
rows = []
if isinstance(raw, dict):
    for k in raw.keys():
        v = raw[k]
        if not isinstance(v, dict):
            raise SystemExit('Each TC entry must be an object')
        rows.append(v)
elif isinstance(raw, list):
    rows = raw
else:
    raise SystemExit('Top-level JSON must be an object or array')

if not rows:
    raise SystemExit('JSON array is empty')

# Build union of keys preserving first-seen order across rows
union_keys = []
seen = set()
for r in rows:
    for key in r.keys():
        if key not in seen:
            seen.add(key)
            union_keys.append(key)

# Create workbook and Data sheet
wb = Workbook()
ws_data = wb.active
ws_data.title = 'Data'

# Header style
header_font = Font(bold=True)
header_align = Alignment(horizontal='center', vertical='center')
header_fill = PatternFill(fill_type='solid', fgColor='FF0000FF')  # solid blue
thin = Side(border_style='thin', color='FF000000')
thin_border = Border(left=thin, right=thin, top=thin, bottom=thin)

# Write headers
for col_idx, key in enumerate(union_keys, start=1):
    c = ws_data.cell(row=1, column=col_idx, value=key)
    c.font = header_font
    c.alignment = header_align
    c.fill = header_fill
    c.border = thin_border

# Write data rows preserving values exactly (no mutation)
for r_idx, r in enumerate(rows, start=2):
    for c_idx, key in enumerate(union_keys, start=1):
        val = r.get(key, '')
        cell = ws_data.cell(row=r_idx, column=c_idx, value=val)
        cell.alignment = Alignment(vertical='top', horizontal='left', wrap_text=False)
        cell.border = thin_border

# Freeze top row
ws_data.freeze_panes = 'A2'

# Auto-fit column widths (approx)
for c_idx, key in enumerate(union_keys, start=1):
    max_len = len(str(key))
    for r_idx in range(2, 2 + len(rows)):
        v = ws_data.cell(row=r_idx, column=c_idx).value
        if v is None:
            v = ''
        s = str(v)
        l = max(len(line) for line in s.splitlines()) if s else 0
        max_len = max(max_len, l)
    ws_data.column_dimensions[ws_data.cell(row=1, column=c_idx).column_letter].width = min(120, max(10, max_len + 2))

# META columns
META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

# MAIN columns in final order
MAIN_COLS = [
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
    'Code Generation (Required / Not)',
]

# Create Meta_data_sheet and copy META columns AS-IS
ws_meta = wb.create_sheet('Meta_data_sheet')
for c_idx, key in enumerate(META_COLS, start=1):
    c = ws_meta.cell(row=1, column=c_idx, value=key)
    c.font = header_font
    c.alignment = header_align
    c.fill = header_fill
    c.border = thin_border

for r_idx, r in enumerate(rows, start=2):
    for c_idx, key in enumerate(META_COLS, start=1):
        v = r.get(key, '')
        cell = ws_meta.cell(row=r_idx, column=c_idx, value=v)
        cell.alignment = Alignment(vertical='top', horizontal='left', wrap_text=True)
        cell.border = thin_border

# Very hide META sheet
ws_meta.sheet_state = 'veryHidden'

# Create TestPlan sheet
ws_tp = wb.create_sheet('TestPlan')

# Header row for TestPlan
for c_idx, key in enumerate(MAIN_COLS, start=1):
    c = ws_tp.cell(row=1, column=c_idx, value=key)
    c.font = header_font
    c.alignment = header_align
    c.fill = header_fill
    c.border = thin_border

# Helper to number text within a single cell

def number_text(text: str) -> str:
    if text is None:
        return ''
    s = str(text).strip()
    if not s:
        return ''
    # Split deterministically on ';' if present, otherwise keep as single item to avoid mutating content
    if ';' in s:
        parts = [p.strip() for p in s.split(';') if p.strip()]
    else:
        parts = [s]
    lines = []
    for i, part in enumerate(parts, start=1):
        lines.append(f"{i}. {part}")
    return "\n".join(lines)

# Populate TestPlan rows
for r_idx, r in enumerate(rows, start=2):
    for c_idx, key in enumerate(MAIN_COLS, start=1):
        v = r.get(key, '')
        if key in ('Test Steps / Procedure', 'Validation / Acceptance Criteria'):
            v = number_text(v)
        cell = ws_tp.cell(row=r_idx, column=c_idx, value=v)
        # Alignment rules
        wrap = key in ('Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria')
        h_align = 'left'
        if key == 'Index':
            h_align = 'center'
        cell.alignment = Alignment(vertical='top', horizontal=h_align, wrap_text=wrap)
        cell.border = thin_border

# Freeze top row
ws_tp.freeze_panes = 'A2'

# Auto-fit column widths and row heights after wrapping
for c_idx, key in enumerate(MAIN_COLS, start=1):
    max_len = len(str(key))
    for r_idx in range(2, 2 + len(rows)):
        v = ws_tp.cell(row=r_idx, column=c_idx).value
        if v is None:
            v = ''
        s = str(v)
        # Consider longest line for width
        width_len = max((len(line) for line in s.splitlines()), default=0)
        max_len = max(max_len, width_len)
        # Row height estimation based on number of lines
        lines = max(1, s.count('\n') + 1)
        ws_tp.row_dimensions[r_idx].height = max(ws_tp.row_dimensions[r_idx].height or 15, 15 * lines)
    ws_tp.column_dimensions[ws_tp.cell(row=1, column=c_idx).column_letter].width = min(120, max(10, max_len + 2))

# Data validation for Code Generation (Required / Not) on data rows only
try:
    col_idx = MAIN_COLS.index('Code Generation (Required / Not)') + 1
    dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
    start_row = 2
    end_row = 1 + len(rows)
    col_letter = ws_tp.cell(row=1, column=col_idx).column_letter
    dv_range = f"{col_letter}{start_row}:{col_letter}{end_row}"
    dv.add(dv_range)
    ws_tp.add_data_validation(dv)
except ValueError:
    pass

# Save workbook with IST timestamp
os.makedirs(OUTPUT_DIR, exist_ok=True)
now_ist = datetime.now(ZoneInfo('Asia/Kolkata'))
file_ts = now_ist.strftime('%Y%m%d_%H%M%S')
file_name = f"{IP_NAME}_TestPlan_{file_ts}.xlsx"
file_path = os.path.join(OUTPUT_DIR, file_name)
wb.save(file_path)

# Validate as true XLSX (ZIP with xl/workbook.xml)
if not zipfile.is_zipfile(file_path):
    raise SystemExit('Generated file is not a valid ZIP-based XLSX')
with zipfile.ZipFile(file_path, 'r') as zf:
    names = set(zf.namelist())
    if 'xl/workbook.xml' not in names:
        raise SystemExit('Missing xl/workbook.xml in XLSX package')

print(f'Wrote: {file_path}')
