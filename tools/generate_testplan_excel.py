#!/usr/bin/env python3
import argparse, json, os, re
from datetime import datetime
from zoneinfo import ZoneInfo
from zipfile import ZipFile
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

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
    'Imparted Registers',  # typo-safe alias will be corrected to 'Impacted Registers' below if present
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]

WRAP_COLS = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria',
}

ALLOWED_DV = ['Required', 'Blank', 'Not Required']


def natural_tc_key(k: str):
    m = re.match(r'^(?:TC|tc)(\d+)$', k.strip())
    return (int(m.group(1)) if m else 10**9, k)


def load_json_records(path: str):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if isinstance(data, list):
        if not data:
            raise SystemExit('JSON array is empty')
        return data
    if isinstance(data, dict):
        # Deterministically convert dict of testcases to array ordered by TC1..TCN
        ordered = []
        for k in sorted(data.keys(), key=natural_tc_key):
            ordered.append(data[k])
        if not ordered:
            raise SystemExit('JSON object contains no test case entries')
        return ordered
    raise SystemExit('Unsupported JSON top-level type; expected array or object of test cases')


def union_keys_preserve_first_order(records):
    seen = []
    s = set()
    for rec in records:
        if not isinstance(rec, dict):
            raise SystemExit('Each JSON record must be an object')
        for k in rec.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def to_str(v):
    if v is None:
        return ''
    if isinstance(v, (int, float)):
        return str(v)
    if isinstance(v, list):
        # Preserve raw list; caller will format if needed
        return v
    return str(v)


def build_numbered_text(value):
    # Number list items inside a single wrapped cell
    if isinstance(value, list):
        items = [str(x).strip() for x in value if str(x).strip()]
    else:
        s = str(value)
        if '\n' in s:
            items = [line.strip() for line in s.split('\n') if line.strip()]
        else:
            # Single item; return as-is
            items = [s.strip()] if s.strip() else []
    if not items:
        return ''
    return '\n'.join(f"{i+1}. {items[i]}" for i in range(len(items)))


def auto_widths(ws):
    from openpyxl.utils import get_column_letter
    widths = {}
    for row in ws.iter_rows(values_only=True):
        for i, v in enumerate(row, start=1):
            s = ''
            if isinstance(v, list):
                s = '\n'.join(str(x) for x in v)
            elif v is not None:
                s = str(v)
            w = max((len(line) for line in s.split('\n')), default=0)
            widths[i] = max(widths.get(i, 0), min(80, max(10, w + 2)))
    for i, w in widths.items():
        ws.column_dimensions[get_column_letter(i)].width = w


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows():
        for c in row:
            c.border = border


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True)
    ap.add_argument('--outdir', required=True)
    ap.add_argument('--ip-name', required=True)
    args = ap.parse_args()

    records = load_json_records(args.json)
    all_keys = union_keys_preserve_first_order(records)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write header
    ws.append(all_keys)
    # Write rows preserving exact values (no mutation)
    for rec in records:
        row = [rec.get(k, '') for k in all_keys]
        ws.append(row)

    # Base formatting
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center')
    blue_fill = PatternFill('solid', fgColor='4472C4')

    for c in ws[1]:
        c.font = header_font
        c.alignment = header_align
        c.fill = blue_fill

    ws.freeze_panes = 'A2'

    # Create META sheet and copy meta columns AS-IS
    ws_meta = wb.create_sheet('Meta_data_sheet')
    meta_present = [c for c in META_COLS if c in all_keys]
    if meta_present:
        ws_meta.append(meta_present)
        for rec in records:
            ws_meta.append([rec.get(k, '') for k in meta_present])
    ws_meta.sheet_state = 'veryHidden'

    # Normalize main sheet: remove meta columns, enforce MAIN order, then append any extra non-meta columns in first-seen order
    main_cols = [c for c in MAIN_ORDER if c in all_keys]
    # Insert missing MAIN columns explicitly as blanks in visible sheet
    for c in MAIN_ORDER:
        if c not in main_cols:
            main_cols.append(c)
    extra_cols = [k for k in all_keys if k not in META_COLS and k not in MAIN_ORDER]
    final_cols = main_cols + extra_cols

    # Rebuild Data sheet content in-memory, then replace values
    data_rows = []
    for rec in records:
        row = []
        for col in final_cols:
            v = rec.get(col, '')
            # In-cell numbering for specific columns
            if col in ('Test Steps / Procedure', 'Validation / Acceptance Criteria'):
                v = build_numbered_text(v)
            row.append(v)
        data_rows.append(row)

    # Replace sheet with final_cols and data_rows
    ws.delete_rows(1, ws.max_row)
    ws.append(final_cols)
    for row in data_rows:
        ws.append(row)

    # Rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Text wrapping, alignments, borders
    wrap_cols_set = set(WRAP_COLS)
    col_index = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column+1)}

    for r in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for c in r:
            header = ws.cell(row=1, column=c.column).value
            if header in wrap_cols_set:
                c.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            else:
                c.alignment = Alignment(vertical='top')

    # Header formatting already applied; ensure alignment and fill preserved
    for c in ws[1]:
        c.font = header_font
        c.alignment = header_align
        c.fill = blue_fill

    auto_widths(ws)
    apply_borders(ws)

    # Data validation for Code Generation (Required / Not)
    if 'Code Generation (Required / Not)' in col_index:
        idx = col_index['Code Generation (Required / Not)']
        from openpyxl.utils import get_column_letter
        col_letter = get_column_letter(idx)
        dv = DataValidation(type='list', formula1='"' + ','.join(ALLOWED_DV) + '"', allow_blank=True, error='Select a value from the list')
        ws.add_data_validation(dv)
        dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: ensure no sheet named 'Data' remains
    if any(sh.title == 'Data' for sh in wb.worksheets):
        # Delete any residual 'Data' sheet
        for sh in list(wb.worksheets):
            if sh.title == 'Data':
                wb.remove(sh)

    # Save with IST timestamp and naming rule
    os.makedirs(args.outdir, exist_ok=True)
    now = datetime.now(ZoneInfo('Asia/Kolkata'))
    fname = f"{args.ip_name}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"
    outpath = os.path.join(args.outdir, fname)
    wb.save(outpath)

    # OOXML validation
    with ZipFile(outpath, 'r') as z:
        names = set(z.namelist())
        assert '[Content_Types].xml' in names
        assert 'xl/workbook.xml' in names
        assert any(n.startswith('xl/worksheets/') for n in names)

    print(outpath)

if __name__ == '__main__':
    main()
