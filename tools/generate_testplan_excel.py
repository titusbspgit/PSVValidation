import json, os, sys, re, math, zipfile, io, datetime
from pathlib import Path
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Configuration from environment
INPUT_JSON_PATH = os.getenv('INPUT_JSON_PATH', 'Test_Input/PCIE/pcie_testplan_input.json')
OUTPUT_DIR = os.getenv('OUTPUT_DIR', 'Test_Output/PCIE/TestPlan')
IP_NAME = os.getenv('IP_NAME', 'PCIE')

# Column definitions
META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]
MAIN_ORDER = [
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
WRAP_COLS = set([
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria'
])

ALLOWED_CODE_GEN = ['Required', 'Blank', 'Not Required']


def fail(msg: str):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def load_json_records(path: str):
    if not os.path.exists(path):
        fail(f"JSON input not found at {path}")
    with open(path, 'r', encoding='utf-8') as f:
        raw = f.read().strip()
        if not raw:
            fail('JSON input is empty')
        try:
            data = json.loads(raw)
        except Exception as e:
            fail(f'Invalid JSON: {e}')
    # Normalize: top-level is array; if dict like {TC1:{...}, TC2:{...}}, convert to list of inner objects preserving order
    if isinstance(data, dict):
        records = list(data.values())
    elif isinstance(data, list):
        records = data
    else:
        fail('Top-level JSON must be an object or array')
    if not records:
        fail('No records found after normalization')
    # Ensure each record is dict
    for i, r in enumerate(records):
        if not isinstance(r, dict):
            fail(f'Record {i} is not an object')
    return records


def union_keys_in_order(records):
    seen = []
    seen_set = set()
    for r in records:
        for k in r.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    return seen


def approximate_autofit(ws):
    # Compute max string length per column and set width
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            v = '' if cell.value is None else str(cell.value)
            if len(v) > max_len:
                max_len = len(v)
        # Approx: each char ~ 1 unit; add padding
        width = min(max(10, max_len + 2), 120)
        ws.column_dimensions[get_column_letter(col_idx)].width = width


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def compute_row_height_for_text(text: str):
    if text is None:
        return None
    s = str(text)
    # Estimate lines by newlines and width ~ 35 chars per line
    explicit_lines = s.count('\n') + 1
    est_lines = max(explicit_lines, math.ceil(len(s) / 35) if s else 1)
    # Base height ~ 15 points per line
    return 15 * est_lines


def normalize_numbering(raw: str) -> str:
    if raw is None:
        return ''
    # Split into logical items by newline
    parts = [p for p in str(raw).split('\n') if p.strip()]
    normalized = []
    for idx, p in enumerate(parts, start=1):
        # Remove any leading bullets or numbering (e.g., 1), 1., -, •)
        m = re.match(r"^\s*(?:\d+[\).\-\s]*|[-*•]\s*)?(.*\S)\s*$", p)
        core = m.group(1) if m else p.strip()
        normalized.append(f"{idx}. {core}")
    return '\n'.join(normalized)


def validate_xlsx(path: str) -> bool:
    try:
        with zipfile.ZipFile(path, 'r') as z:
            names = set(z.namelist())
            required = {'[Content_Types].xml', 'xl/workbook.xml'}
            return required.issubset(names)
    except Exception:
        return False


def main():
    records = load_json_records(INPUT_JSON_PATH)

    # Build union of keys preserving first-seen order
    all_keys = union_keys_in_order(records)

    # Create workbook and the single authoritative staging sheet named Data
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write header
    for c_idx, key in enumerate(all_keys, start=1):
        ws.cell(row=1, column=c_idx, value=key)

    # Write rows exactly
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, key in enumerate(all_keys, start=1):
            val = rec.get(key, '')
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Base formatting: Bold header, freeze top row, approximate auto-fit
    header_font = Font(bold=True)
    for cell in ws[1]:
        cell.font = header_font
    ws.freeze_panes = 'A2'
    approximate_autofit(ws)

    # Create META sheet and copy META columns AS-IS, preserve order
    ws_meta = wb.create_sheet('Meta_data_sheet')
    for c_idx, key in enumerate(META_COLS, start=1):
        ws_meta.cell(row=1, column=c_idx, value=key)
    for r_idx in range(2, ws.max_row + 1):
        for c_idx, key in enumerate(META_COLS, start=1):
            # Find column index in Data if present
            try:
                src_col = all_keys.index(key) + 1
                val = ws.cell(row=r_idx, column=src_col).value
            except ValueError:
                val = ''
            ws_meta.cell(row=r_idx, column=c_idx, value=val)
    ws_meta.sheet_state = 'veryHidden'

    # Now normalize Data sheet to MAIN columns and order, and remove META columns
    # Rebuild header to MAIN_ORDER
    ws.delete_rows(1, ws.max_row)  # clear sheet content (keeping same sheet)
    for c_idx, key in enumerate(MAIN_ORDER, start=1):
        ws.cell(row=1, column=c_idx, value=key)
    # Map records into MAIN_ORDER values
    for r_idx, rec in enumerate(records, start=2):
        row_vals = []
        for key in MAIN_ORDER:
            row_vals.append(rec.get(key, ''))
        for c_idx, v in enumerate(row_vals, start=1):
            ws.cell(row=r_idx, column=c_idx, value=v)

    # Rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Strict formatting on TestPlan
    # Header style
    blue_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        cell.fill = blue_fill

    # Data row alignments and text wrapping
    # Determine column indices
    header_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            header = ws.cell(row=1, column=c).value
            val = ws.cell(row=r, column=c).value
            if header in WRAP_COLS:
                # Normalize numbering for specific columns
                if header in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
                    ws.cell(row=r, column=c, value=normalize_numbering(val))
                ws.cell(row=r, column=c).alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
                # Approximate row height based on the longest wrapped column value in this row
            else:
                # Alignment: text left, numeric/index center/right
                if header == 'Index':
                    ws.cell(row=r, column=c).alignment = Alignment(vertical='top', horizontal='center')
                else:
                    ws.cell(row=r, column=c).alignment = Alignment(vertical='top', horizontal='left')
        # After setting all cells for the row, compute row height
        # Use max of wrapped columns heights
        max_h = 15
        for colname in WRAP_COLS:
            if colname in header_idx:
                v = ws.cell(row=r, column=header_idx[colname]).value
                h = compute_row_height_for_text(v)
                if h and h > max_h:
                    max_h = h
        ws.row_dimensions[r].height = max_h

    # Autofit columns after wrapping
    approximate_autofit(ws)

    # Thin borders for all populated cells
    apply_borders(ws)

    # Data validation only for 'Code Generation (Required / Not)'
    if 'Code Generation (Required / Not)' in header_idx:
        col = header_idx['Code Generation (Required / Not)']
        col_letter = get_column_letter(col)
        dv = DataValidation(type='list', formula1='"' + ','.join(ALLOWED_CODE_GEN) + '"', allow_blank=True, showErrorMessage=True)
        dv.error = 'Select a value from the list: ' + ', '.join(ALLOWED_CODE_GEN)
        ws.add_data_validation(dv)
        # Apply only to data rows
        if ws.max_row >= 2:
            dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: ensure no sheet named 'Data'
    if 'Data' in [s.title for s in wb.worksheets]:
        # Attempt delete; if cannot, fail
        try:
            del wb['Data']
        except Exception:
            fail("Validation failed: Worksheet named 'Data' still exists and could not be deleted")

    # Prepare output path and filename based on IST
    ist = ZoneInfo('Asia/Kolkata')
    now_ist = datetime.datetime.now(tz=ist)
    ts = now_ist.strftime('%Y%m%d_%H%M%S')
    out_dir = Path(OUTPUT_DIR)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{IP_NAME}_TestPlan_{ts}.xlsx"

    # Save workbook
    wb.save(out_path.as_posix())

    # Validate XLSX as OOXML zip
    if not validate_xlsx(out_path.as_posix()):
        fail('XLSX validation failed: not a valid OOXML ZIP')

    print(f"OK: Generated {out_path}")


if __name__ == '__main__':
    main()
