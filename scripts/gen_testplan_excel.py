import json, os, sys, zipfile
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import List

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Constants
INPUT_JSON = 'scripts/input/GPIO_TestPlan.json'
OUTPUT_DIR = Path('Test_Output/GPIO/TestPlan')
META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
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

BORDER_THIN = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))
HEADER_FILL = PatternFill('solid', fgColor='BDD7EE')  # light blue
HEADER_FONT = Font(bold=True)
CENTER = Alignment(horizontal='center', vertical='center', wrap_text=False)
LEFT_WRAP = Alignment(horizontal='left', vertical='top', wrap_text=True)
LEFT = Alignment(horizontal='left', vertical='top', wrap_text=False)
RIGHT = Alignment(horizontal='right', vertical='top', wrap_text=False)
TOP = Alignment(vertical='top')


def fail(msg: str):
    print(msg)
    sys.exit(1)


def load_json_array(path: str) -> List[dict]:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        fail('JSON must be a non-empty array of objects')
    for i, rec in enumerate(data):
        if not isinstance(rec, dict):
            fail(f'JSON record at index {i} is not an object')
    return data


def first_seen_union_keys(rows: List[dict]) -> List[str]:
    seen = []
    sset = set()
    for rec in rows:
        for k in rec.keys():
            if k not in sset:
                sset.add(k)
                seen.append(k)
    return seen


def normalize_numbering(text: str) -> str:
    if not text:
        return text
    lines = [ln.strip() for ln in str(text).split('\n')]
    out = []
    idx = 1
    for ln in lines:
        if not ln:
            continue
        # remove leading bullets/numbers
        l = ln
        while l and (l[0] in '*-•·•' or l[:2].isdigit() or (len(l) > 1 and l[1] in ').')):
            # strip common patterns like '1) ', '1. ', '- '
            if l[:2].isdigit():
                # unlikely as two digits at start; break
                break
            if l[0] in '*-•·':
                l = l[1:].lstrip()
            elif len(l) > 1 and l[1] in ').':
                l = l[2:].lstrip()
            else:
                break
        out.append(f"{idx}. {l}")
        idx += 1
    return '\n'.join(out)


def autosize_columns(ws):
    maxlen = {}
    for row in ws.iter_rows(values_only=True):
        for i, val in enumerate(row, start=1):
            s = '' if val is None else str(val)
            l = len(s)
            maxlen[i] = max(maxlen.get(i, 0), l)
    for col_idx, l in maxlen.items():
        width = min(max(12, l + 2), 80)
        ws.column_dimensions[chr(64 + col_idx) if col_idx <= 26 else None]
    # openpyxl doesn't auto map >Z; compute column letters properly
    from openpyxl.utils import get_column_letter
    for col_idx, l in maxlen.items():
        width = min(max(12, l + 2), 80)
        ws.column_dimensions[get_column_letter(col_idx)].width = width


def apply_borders(ws):
    for row in ws.iter_rows():
        for cell in row:
            cell.border = BORDER_THIN


def main():
    rows = load_json_array(INPUT_JSON)
    # Build union of keys preserving first-seen order
    headers = first_seen_union_keys(rows)

    # Phase 1: Base workbook
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write headers
    ws.append(headers)
    # Write data rows, preserving exact values; fill missing as blank
    for rec in rows:
        row_vals = [rec.get(h, '') for h in headers]
        ws.append(row_vals)

    # Freeze header
    ws.freeze_panes = 'A2'

    # Basic header formatting
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL

    # Phase 2: Meta sheet
    meta = wb.create_sheet('Meta_data_sheet')
    meta.append(META_COLS)
    for rec in rows:
        meta.append([rec.get(c, '') for c in META_COLS])
    meta.sheet_state = 'veryHidden'

    # Transform main sheet in place: rename and reorder/remove
    ws.title = 'TestPlan'

    # Build in-place reordered data for MAIN_ORDER
    # Create a snapshot of current data rows (excluding header)
    data_rows = list(ws.iter_rows(min_row=2, values_only=True))
    # Map header to index
    h2i = {h: i for i, h in enumerate(headers)}

    # Rebuild sheet: clear and write MAIN_ORDER header
    ws.delete_rows(1, ws.max_row)
    ws.append(MAIN_ORDER)

    # Write rows in new order with transformations
    for vals in data_rows:
        rec = {h: (vals[h2i[h]] if h in h2i else '') for h in headers}
        # Numbering for specific fields
        td = rec.get('Test Description', '')
        rem = rec.get('Remarks', '')
        steps = rec.get('Test Steps / Procedure', '')
        vac = rec.get('Validation / Acceptance Criteria', '')
        rec['Test Description'] = td
        rec['Remarks'] = rem
        rec['Test Steps / Procedure'] = normalize_numbering(steps)
        rec['Validation / Acceptance Criteria'] = normalize_numbering(vac)

        ws.append([rec.get(k, '') for k in MAIN_ORDER])

    # Apply formatting
    # Header again after rebuild
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL
    # Data rows alignments
    idx_col = MAIN_ORDER.index('Index') + 1
    cg_col = MAIN_ORDER.index('Code Generation (Required / Not)') + 1
    td_col = MAIN_ORDER.index('Test Description') + 1
    rem_col = MAIN_ORDER.index('Remarks') + 1
    steps_col = MAIN_ORDER.index('Test Steps / Procedure') + 1
    vac_col = MAIN_ORDER.index('Validation / Acceptance Criteria') + 1

    for r in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for c in r:
            c.alignment = LEFT
        # Index center
        r[idx_col - 1].alignment = CENTER
        # Wrap select columns
        r[td_col - 1].alignment = LEFT_WRAP
        r[rem_col - 1].alignment = LEFT_WRAP
        r[steps_col - 1].alignment = LEFT_WRAP
        r[vac_col - 1].alignment = LEFT_WRAP

    autosize_columns(ws)
    apply_borders(ws)

    # Data validation for Code Generation (Required / Not)
    dv = DataValidation(type='list', formula1='"Required,Blank, Not Required"', allow_blank=True)
    ws.add_data_validation(dv)
    dv.add(f"{ws.cell(row=2, column=cg_col).coordinate}:{ws.cell(row=ws.max_row, column=cg_col).coordinate}")

    # Ensure no sheet named 'Data'
    if 'Data' in wb.sheetnames:
        # Attempt delete; if cannot, fail
        try:
            del wb['Data']
        except Exception:
            fail("Data sheet still exists and cannot be deleted")

    # Build IST timestamp and filename
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    date_part = now_ist.strftime('%Y%m%d')
    time_part = now_ist.strftime('%H%M%S')
    ist_human = now_ist.strftime('%Y-%m-%d %H:%M:%S')

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / f"GPIO_TestPlan_{date_part}_{time_part}.xlsx"

    wb.save(out_path.as_posix())

    # Validate OOXML by checking minimal parts
    with zipfile.ZipFile(out_path.as_posix(), 'r') as z:
        required = ['[Content_Types].xml','_rels/.rels','xl/workbook.xml']
        for r in required:
            if r not in z.namelist():
                fail(f'Missing OOXML part: {r}')

    # Expose outputs to GH Actions
    ghout = os.environ.get('GITHUB_OUTPUT')
    if ghout:
        with open(ghout, 'a', encoding='utf-8') as f:
            print(f"output_file={out_path.as_posix()}", file=f)
            print(f"ist_stamp={ist_human}", file=f)

    print(f"Generated: {out_path.as_posix()} (IST {ist_human})")

if __name__ == '__main__':
    main()
