import json
import os
import re
import sys
import io
from datetime import datetime
from zipfile import ZipFile
from pathlib import Path

try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Configuration from environment
IP_NAME = os.environ.get('IP_NAME', 'IP')
OUTPUT_DIR = os.environ.get('OUTPUT_DIR', 'Test_Output')
RAW_JSON = os.environ.get('JSON_PAYLOAD', '').strip()

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

META_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]

BLUE_FILL = PatternFill(start_color='FF0070C0', end_color='FF0070C0', fill_type='solid')
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))


def fail(msg):
    print(msg, file=sys.stderr)
    sys.exit(2)


def ist_now():
    # Asia/Kolkata (IST)
    if ZoneInfo is not None:
        return datetime.now(ZoneInfo('Asia/Kolkata'))
    # Fallback to manual offset +05:30
    return datetime.utcnow()  # Naming will still be deterministic by UTC if zoneinfo unavailable


def parse_json(raw):
    if not raw:
        fail('ERROR: Empty JSON input. Provide JSON via workflow input json_payload.')
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as e:
        fail(f'ERROR: Invalid JSON input: {e}')
    # Normalize to list of row dicts
    if isinstance(data, dict):
        # Keep insertion order of keys
        rows = list(data.values())
    elif isinstance(data, list):
        rows = data
    else:
        fail('ERROR: JSON root must be an array or object of rows')
    if not rows:
        fail('ERROR: JSON contains zero rows')
    # Ensure each row is a dict
    for i, r in enumerate(rows, 1):
        if not isinstance(r, dict):
            fail(f'ERROR: Row {i} is not an object')
    return rows


def union_keys_preserve_order(rows):
    seen = []
    s = set()
    for r in rows:
        for k in r.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def to_cell_value(v):
    # Preserve exact values. For list/dict, keep JSON string.
    if isinstance(v, (list, dict)):
        return json.dumps(v, ensure_ascii=False)
    return v


def normalize_numbering(text):
    if text is None:
        return ''
    s = str(text).replace('\r\n', '\n')
    lines = [ln for ln in s.split('\n') if ln.strip() != '']
    if not lines:
        return ''
    cleaned = []
    for ln in lines:
        # Remove bullets or existing numbering like "1)", "1.", "-", "•"
        ln2 = re.sub(r'^\s*(?:\d+[\.)]|[-•])\s*', '', ln.strip())
        cleaned.append(ln2)
    renum = [f"{i}. {item}" for i, item in enumerate(cleaned, 1)]
    return "\n".join(renum)


def autofit_columns(ws):
    # Compute approximate width per column based on content length
    max_len = {}
    for row in ws.iter_rows(values_only=True):
        for idx, val in enumerate(row, start=1):
            txt = '' if val is None else str(val)
            l = len(txt)
            max_len[idx] = max(l, max_len.get(idx, 0))
    for idx, ml in max_len.items():
        width = min(max(ml + 2, 10), 80)
        ws.column_dimensions[openpyxl.utils.get_column_letter(idx)].width = width


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def format_testplan(ws):
    # Header formatting
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        cell.fill = BLUE_FILL
    # Wrap certain columns
    wrap_cols = {
        'Test Description',
        'Remarks',
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria'
    }
    header_map = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            hdr = ws.cell(row=1, column=c).value
            if hdr in wrap_cols:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            else:
                # Left align text-ish, center/right numeric-like
                val = cell.value
                if isinstance(val, (int, float)):
                    cell.alignment = Alignment(vertical='top', horizontal='right')
                else:
                    # If looks like number string, center
                    if isinstance(val, str) and val.isdigit():
                        cell.alignment = Alignment(vertical='top', horizontal='center')
                    else:
                        cell.alignment = Alignment(vertical='top', horizontal='left')
    # Row heights based on wrapped text lines
    for r in range(2, ws.max_row + 1):
        # Count max lines across wrap columns
        max_lines = 1
        for col_name in wrap_cols:
            c = header_map.get(col_name)
            if c:
                val = ws.cell(row=r, column=c).value
                txt = '' if val is None else str(val)
                lines = txt.count('\n') + 1 if txt else 1
                if lines > max_lines:
                    max_lines = lines
        ws.row_dimensions[r].height = min(15 * max_lines, 300)  # Cap height

    apply_borders(ws)


def create_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Determine schema (union of keys preserving first-seen order)
    keys = union_keys_preserve_order(rows)

    # Header
    for col_idx, k in enumerate(keys, start=1):
        ws.cell(row=1, column=col_idx, value=k).font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Data rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, k in enumerate(keys, start=1):
            v = row.get(k, '')
            ws.cell(row=r_idx, column=c_idx, value=to_cell_value(v))

    # Basic autofit
    # openpyxl name needed below
    import openpyxl
    for col in range(1, ws.max_column + 1):
        length = 10
        for r in range(1, ws.max_row + 1):
            v = ws.cell(row=r, column=col).value
            l = len(str(v)) if v is not None else 0
            if l > length:
                length = l
        ws.column_dimensions[openpyxl.utils.get_column_letter(col)].width = min(max(length + 2, 12), 80)

    return wb


def copy_meta_sheet(wb, rows):
    meta = wb.create_sheet('Meta_data_sheet')
    # Header
    for c, k in enumerate(META_COLUMNS, 1):
        meta.cell(row=1, column=c, value=k).font = Font(bold=True)
    # Data
    for r_idx, row in enumerate(rows, start=2):
        for c, k in enumerate(META_COLUMNS, 1):
            meta.cell(row=r_idx, column=c, value=to_cell_value(row.get(k, '')))
    # Very hidden
    meta.sheet_state = 'veryHidden'


def reorder_and_format_testplan(wb):
    ws = wb['Data']
    ws.title = 'TestPlan'  # Must rename, not create new visible sheet

    # Build data in final order excluding META columns
    # Extract all rows into list of dicts using existing header
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    data_rows = []
    for r in range(2, ws.max_row + 1):
        obj = {}
        for c, h in enumerate(headers, 1):
            obj[h] = ws.cell(row=r, column=c).value
        data_rows.append(obj)

    # Clear sheet
    ws.delete_rows(1, ws.max_row)

    # Write headers in final order
    for c, h in enumerate(MAIN_COLUMNS, 1):
        ws.cell(row=1, column=c, value=h)

    # Write data rows following MAIN_COLUMNS and dropping META columns
    for r_idx, obj in enumerate(data_rows, start=2):
        for c, h in enumerate(MAIN_COLUMNS, 1):
            val = obj.get(h, '')
            ws.cell(row=r_idx, column=c, value=val)

    # Mandatory in-cell numbering for two columns
    hdr_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    for col_name in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
        c = hdr_to_col.get(col_name)
        if c:
            for r in range(2, ws.max_row + 1):
                val = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=normalize_numbering(val))

    # Data validation for Code Generation (Required / Not)
    code_col = hdr_to_col.get('Code Generation (Required / Not)')
    if code_col:
        start_cell = ws.cell(row=2, column=code_col).coordinate
        end_cell = ws.cell(row=ws.max_row, column=code_col).coordinate
        dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True)
        ws.add_data_validation(dv)
        dv.add(f"{start_cell}:{end_cell}")

    # Formatting (wraps, alignment, header styles, borders, row heights)
    format_testplan(ws)

    # Safety: ensure only TestPlan and Meta_data_sheet exist and no sheet named Data
    if 'Data' in wb.sheetnames:
        # Delete if somehow exists
        del wb['Data']
    allowed = {'TestPlan', 'Meta_data_sheet'}
    for name in list(wb.sheetnames):
        if name not in allowed:
            del wb[name]


def validate_xlsx_bytes(path):
    # Check OOXML ZIP parts
    with open(path, 'rb') as f:
        data = f.read()
    with ZipFile(io.BytesIO(data), 'r') as z:
        namelist = z.namelist()
        required = {'[Content_Types].xml', 'xl/workbook.xml'}
        missing = [p for p in required if p not in namelist]
        if missing:
            fail(f'ERROR: XLSX missing parts: {missing}')
    # Try to open with openpyxl
    load_workbook(path)


def main():
    rows = parse_json(RAW_JSON)

    # Phase 1: base workbook
    wb = create_workbook(rows)

    # Phase 2: meta + reorg on same main sheet
    copy_meta_sheet(wb, rows)
    reorder_and_format_testplan(wb)

    # Phase 3: save and validate
    now = ist_now()
    ts = now.strftime('%Y%m%d_%H%M%S')
    out_name = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_dir = Path(OUTPUT_DIR)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / out_name

    wb.save(str(out_path))
    validate_xlsx_bytes(str(out_path))

    # Output path for workflow
    gh_out = os.environ.get('GITHUB_OUTPUT')
    if gh_out:
        with open(gh_out, 'a', encoding='utf-8') as fh:
            fh.write(f"output_file_path={out_path.as_posix()}\n")
    print(f"Generated: {out_path.as_posix()}")


if __name__ == '__main__':
    main()
