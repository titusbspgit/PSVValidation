import json, os, zipfile
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

FINAL_ORDER = [
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

HEADER_FILL = PatternFill('solid', fgColor='FF4472C4')  # solid blue
HEADER_ALIGN = Alignment(horizontal='center', vertical='center', wrap_text=True)
DATA_ALIGN_TEXT = Alignment(horizontal='left', vertical='top', wrap_text=False)
DATA_ALIGN_CENTER = Alignment(horizontal='center', vertical='top', wrap_text=False)
THIN = Side(style='thin', color='FF000000')
BORDER_THIN = Border(top=THIN, bottom=THIN, left=THIN, right=THIN)


def number_cell(text: str) -> str:
    if text is None:
        return ''
    lines = str(text).splitlines()
    out = []
    n = 1
    for line in lines:
        if line.strip() == '':
            continue
        out.append(f"{n}. {line}")
        n += 1
    return "\n".join(out) if out else str(text)


def auto_width(ws):
    for c in range(1, ws.max_column + 1):
        header = ws.cell(row=1, column=c).value
        max_len = len(str(header)) if header is not None else 0
        for r in range(2, ws.max_row + 1):
            v = ws.cell(row=r, column=c).value
            s = '' if v is None else str(v)
            if s:
                for line in s.splitlines():
                    max_len = max(max_len, len(line))
        width = max(12, min(max_len + 2, 100))
        ws.column_dimensions[get_column_letter(c)].width = width


def apply_borders(ws):
    for r in range(1, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            ws.cell(row=r, column=c).border = BORDER_THIN


def set_row_heights(ws):
    base = 15
    for r in range(1, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            s = '' if v is None else str(v)
            lines = s.count('\n') + 1 if s else 1
            max_lines = max(max_lines, lines)
        ws.row_dimensions[r].height = min(409, base * max_lines)


def main():
    with open('tools/full_json_gpio.json', 'r', encoding='utf-8') as f:
        full = json.load(f)

    if not isinstance(full, list) or not full:
        raise SystemExit('Invalid FULL_JSON: not a non-empty array')
    obj = full[0]
    testcases = obj.get('testcases', [])
    if not isinstance(testcases, list) or not testcases:
        raise SystemExit('Invalid FULL_JSON: testcases missing/empty')

    # Build union of keys preserving first-seen order
    key_order = []
    seen = set()
    for tc in testcases:
        if not isinstance(tc, dict):
            raise SystemExit('Invalid testcase entry: must be object')
        for k in tc.keys():
            if k not in seen:
                seen.add(k)
                key_order.append(k)

    # Create workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Header
    for c, k in enumerate(key_order, start=1):
        cell = ws.cell(row=1, column=c, value=k)
        cell.font = Font(bold=True, color='FFFFFFFF')
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
    ws.freeze_panes = 'A2'

    # Rows
    for r, tc in enumerate(testcases, start=2):
        for c, k in enumerate(key_order, start=1):
            ws.cell(row=r, column=c, value=tc.get(k, ''))

    auto_width(ws)

    # Borders
    apply_borders(ws)

    # Meta sheet
    meta_ws = wb.create_sheet('Meta_data_sheet')
    for c, k in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=c, value=k).font = Font(bold=True)
    for r, tc in enumerate(testcases, start=2):
        for c, k in enumerate(META_COLS, start=1):
            meta_ws.cell(row=r, column=c, value=tc.get(k, ''))
    meta_ws.sheet_state = 'veryHidden'

    # Rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Rebuild TestPlan in-place with FINAL_ORDER and without META cols
    # Capture current data first
    current_headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    rows_data = []
    for r in range(2, ws.max_row + 1):
        row_dict = {current_headers[c - 1]: ws.cell(row=r, column=c).value for c in range(1, ws.max_column + 1)}
        rows_data.append(row_dict)

    # Clear sheet
    ws.delete_rows(1, ws.max_row)

    # Write new header
    for c, h in enumerate(FINAL_ORDER, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True, color='FFFFFFFF')
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL

    # Write rows in final order
    for r, row_dict in enumerate(rows_data, start=2):
        for c, h in enumerate(FINAL_ORDER, start=1):
            ws.cell(row=r, column=c, value=row_dict.get(h, ''))

    # Formatting for TestPlan
    # Wrap specific columns
    wrap_cols = {'Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'}
    for r in range(2, ws.max_row + 1):
        for c, h in enumerate(FINAL_ORDER, start=1):
            if h in wrap_cols:
                ws.cell(row=r, column=c).alignment = Alignment(horizontal='left', vertical='top', wrap_text=True)
            elif h == 'Index':
                ws.cell(row=r, column=c).alignment = DATA_ALIGN_CENTER
            else:
                ws.cell(row=r, column=c).alignment = DATA_ALIGN_TEXT

    auto_width(ws)
    set_row_heights(ws)

    # Borders again after rebuild
    apply_borders(ws)

    # Numbering inside cells for steps and criteria
    for r in range(2, ws.max_row + 1):
        for col_name in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
            idx = FINAL_ORDER.index(col_name) + 1
            old = ws.cell(row=r, column=idx).value
            ws.cell(row=r, column=idx, value=number_cell(old))

    # Data validation
    if 'Code Generation (Required / Not)' in FINAL_ORDER:
        cg_col = FINAL_ORDER.index('Code Generation (Required / Not)') + 1
        dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
        ws.add_data_validation(dv)
        dv.add(f"{get_column_letter(cg_col)}2:{get_column_letter(cg_col)}{ws.max_row}")

    # Safety visibility
    names = [s.title for s in wb.worksheets]
    if 'Data' in names:
        for s in wb.worksheets:
            if s.title == 'Data':
                wb.remove(s)
    names2 = [s.title for s in wb.worksheets]
    if set(names2) != {'TestPlan', 'Meta_data_sheet'}:
        raise SystemExit(f'Unexpected worksheets: {names2}')

    # Save with IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    out_dir = os.path.join('Test_Output', 'GPIO', 'TestPlan')
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f'GPIO_TestPlan_{ts}.xlsx')
    wb.save(out_path)

    # Validate ZIP/XLSX
    if not zipfile.is_zipfile(out_path):
        raise SystemExit('Saved file is not a valid XLSX ZIP')
    with zipfile.ZipFile(out_path, 'r') as z:
        required = {'[Content_Types].xml', 'xl/workbook.xml'}
        names = set(z.namelist())
        if not required.issubset(names):
            raise SystemExit('XLSX structure invalid')

    print(f'Generated: {out_path}')


if __name__ == '__main__':
    main()
