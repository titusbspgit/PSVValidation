#!/usr/bin/env python3
import argparse, json, os, sys, re
from collections import OrderedDict
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
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
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]

NUMBER_WRAP_COLS = [
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria'
]

BLUE_FILL = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))


def parse_args():
    ap = argparse.ArgumentParser(description='Generate formatted TestPlan Excel from JSON array')
    ap.add_argument('--json', required=True, help='Path to JSON array file')
    ap.add_argument('--output-path', required=True, help='Repo-relative output directory')
    ap.add_argument('--output-name', required=True, help='Output Excel filename (.xlsx)')
    return ap.parse_args()


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError('JSON input must be a non-empty array of objects')
    # Validate all entries are dict-like
    for i, rec in enumerate(data):
        if not isinstance(rec, dict):
            raise ValueError(f'JSON element at index {i} is not an object')
    return data


def union_keys_in_order(records):
    seen = OrderedDict()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def best_effort_autofit(ws):
    col_widths = {}
    for r in ws.iter_rows(values_only=True):
        for idx, val in enumerate(r, start=1):
            s = '' if val is None else str(val)
            col_widths[idx] = max(col_widths.get(idx, 0), len(s))
    for idx, w in col_widths.items():
        # heuristic width cap
        ws.column_dimensions[get_column_letter(idx)].width = min(max(w + 2, 12), 100)


def write_staging_data_sheet(wb, records, headers):
    ws = wb.active
    ws.title = 'Data'
    # Header
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'
    # Rows
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=rec.get(h, ''))
    best_effort_autofit(ws)
    return ws


def copy_meta_sheet(wb, records, headers):
    ws = wb.create_sheet('Meta_data_sheet')
    meta_headers = [h for h in META_COLS]
    # Write headers
    for c, h in enumerate(meta_headers, start=1):
        ws.cell(row=1, column=c, value=h).font = Font(bold=True)
    # Map from main headers to column index
    header_index = {h: i for i, h in enumerate(headers)}
    for r, rec in enumerate(records, start=2):
        for c, h in enumerate(meta_headers, start=1):
            ws.cell(row=r, column=c, value=rec.get(h, ''))
    # Very hidden
    ws.sheet_state = 'veryHidden'
    best_effort_autofit(ws)
    return ws


def reorder_and_format_testplan(ws, headers):
    # Determine final order: MAIN_ORDER present + any remaining headers (excluding META) preserving first-seen
    meta_set = set(META_COLS)
    main_present = [h for h in MAIN_ORDER if h in headers]
    extras = [h for h in headers if h not in meta_set and h not in main_present]
    final_headers = main_present + extras

    # Build column data snapshot
    data = []
    max_row = ws.max_row
    for r in range(2, max_row + 1):
        row_dict = {}
        for c, h in enumerate(headers, start=1):
            row_dict[h] = ws.cell(row=r, column=c).value
        data.append(row_dict)

    # Clear sheet and write final headers
    ws.delete_rows(1, ws.max_row)
    for c, h in enumerate(final_headers, start=1):
        ws.cell(row=1, column=c, value=h)
    # Rows back
    for r_idx, rec in enumerate(data, start=2):
        for c_idx, h in enumerate(final_headers, start=1):
            val = rec.get(h, '')
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Apply formatting
    # Header row style
    hdr_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for c in range(1, len(final_headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = hdr_align
        cell.fill = BLUE_FILL

    # Wrap in specific columns
    wrap_cols = set(NUMBER_WRAP_COLS + ['Test Description', 'Remarks'])
    # Determine column indices
    col_idx = {h: i+1 for i, h in enumerate(final_headers)}

    # Numbering inside required columns on data rows
    def normalize_numbering(text):
        if text is None:
            return ''
        s = str(text).strip()
        if not s:
            return ''
        # Split on newlines; if single line, keep as one item
        parts = [p.strip() for p in re.split(r'\r?\n', s) if p.strip()]
        if len(parts) <= 1:
            return f"1. {s}"
        out = []
        for i, item in enumerate(parts, start=1):
            # Strip leading bullets/numbers
            item = re.sub(r'^(?:[-*•\u2022\u25CF\u25E6]+\s*|\d+[\.)]\s*)', '', item)
            out.append(f"{i}. {item}")
        return "\n".join(out)

    max_row = ws.max_row
    max_col = ws.max_column
    for r in range(2, max_row + 1):
        for h in NUMBER_WRAP_COLS:
            if h in col_idx:
                c = col_idx[h]
                v = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=normalize_numbering(v))

    # Alignment and borders
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            h = final_headers[c-1]
            # Set wrap for select columns
            if h in wrap_cols or h in NUMBER_WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            else:
                cell.alignment = Alignment(vertical='top', horizontal='left')
            cell.border = THIN_BORDER
    # Index column numeric/center if present
    if 'Index' in col_idx:
        c = col_idx['Index']
        for r in range(2, max_row + 1):
            ws.cell(row=r, column=c).alignment = Alignment(vertical='top', horizontal='center')

    # Data validation for Code Generation (Required / Not)
    if 'Code Generation (Required / Not)' in col_idx and max_row >= 2:
        dv = DataValidation(type='list', formula1='"Required,Blank, Not Required"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        col_letter = get_column_letter(col_idx['Code Generation (Required / Not)'])
        dv.ranges.append(f"{col_letter}2:{col_letter}{max_row}")

    best_effort_autofit(ws)
    # Adjust row heights for wrapping
    for r in range(2, max_row + 1):
        ws.row_dimensions[r].height = None

    return final_headers


def main():
    args = parse_args()
    records = load_json(args.json)
    headers = union_keys_in_order(records)

    wb = Workbook()
    ws_data = write_staging_data_sheet(wb, records, headers)
    ws_meta = copy_meta_sheet(wb, records, headers)

    # Rename Data → TestPlan and normalize in-place
    ws_data.title = 'TestPlan'
    final_headers = reorder_and_format_testplan(ws_data, headers)

    # Safety: ensure only TestPlan (visible) and Meta_data_sheet (Very Hidden)
    for name in list(wb.sheetnames):
        if name not in ('TestPlan', 'Meta_data_sheet'):
            # unexpected extra sheets -> delete
            ws = wb[name]
            wb.remove(ws)

    if 'Data' in wb.sheetnames:
        # Delete if somehow remains
        wb.remove(wb['Data'])

    # Save
    out_dir = args.output_path
    out_name = args.output_name
    if not out_name.lower().endswith('.xlsx'):
        print('Output name must end with .xlsx', file=sys.stderr)
        sys.exit(1)
    os.makedirs(out_dir, exist_ok=True)
    out_file = os.path.join(out_dir, out_name)
    wb.save(out_file)

    # Validate OOXML ZIP + load
    import zipfile
    if not zipfile.is_zipfile(out_file):
        print('Generated file is not a valid OOXML ZIP workbook', file=sys.stderr)
        sys.exit(7)
    try:
        load_workbook(out_file, data_only=True)
    except Exception as e:
        print('Openpyxl could not load the generated workbook:', e, file=sys.stderr)
        sys.exit(8)

    # Print summary
    print('ROWS:', len(records))
    print('COLS:', len(final_headers))

if __name__ == '__main__':
    main()
