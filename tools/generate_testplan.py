#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import json
import os
import sys
from collections import OrderedDict
from datetime import datetime, timezone, timedelta
from io import BytesIO
import zipfile

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl import failed: {e}", file=sys.stderr)
    sys.exit(1)

META_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

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

WRAP_COLUMNS = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria',
}

NUMBERED_COLUMNS = {
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria',
}

REQUIRED_OOXML_PARTS = {
    '[Content_Types].xml',
    '_rels/.rels',
    'xl/workbook.xml'
}


def parse_args():
    p = argparse.ArgumentParser(description='Generate formatted TestPlan Excel from JSON.')
    p.add_argument('--json-file', required=True, help='Path to JSON input file')
    p.add_argument('--ip-name', required=True, help='IP name used in output filename rule')
    p.add_argument('--output-dir', required=True, help='Repository-relative output directory for final Excel')
    p.add_argument('--commit', action='store_true', help='If set, will git add/commit/push the generated XLSX')
    return p.parse_args()


def load_json_array(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    # Normalize: if top-level is dict/map, convert to array using values in first-seen key order
    if isinstance(data, dict):
        # Python 3.7+ preserves insertion order
        return list(data.values())
    if isinstance(data, list):
        return data
    raise ValueError('JSON root must be an object or array')


def build_schema(rows):
    # Union of keys in first-seen order across all rows
    seen = OrderedDict()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError('Each row must be an object')
        for k in row.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def value_to_str(v):
    # For Data sheet base representation only; join lists with newline
    if isinstance(v, list):
        return '\n'.join(str(x) if x is not None else '' for x in v)
    if v is None:
        return ''
    return str(v)


def autosize_columns(ws):
    # Approximate autosize based on max length in each column
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                val = '' if cell.value is None else str(cell.value)
            except Exception:
                val = ''
            if val:
                for line in val.split('\n'):
                    if len(line) > max_len:
                        max_len = len(line)
        width = min(max(10, max_len + 2), 80)
        ws.column_dimensions[col_letter].width = width


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def estimate_row_heights(ws):
    # Estimate row heights based on wrapped content: lines count
    base_height = 15
    for r in range(1, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            text = '' if cell.value is None else str(cell.value)
            lines = text.count('\n') + 1 if text else 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines


def generate_workbook(rows, ip_name, output_dir):
    if not rows:
        raise ValueError('JSON array is empty')

    schema = build_schema(rows)

    # Track original list values for numbering later
    list_tracker = []  # list of dicts: for each row, keys that had list values originally
    for row in rows:
        lt = {}
        for k, v in row.items():
            if isinstance(v, list):
                lt[k] = list(v)
        list_tracker.append(lt)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Header
    for col_idx, key in enumerate(schema, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Data rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(schema, start=1):
            val = row.get(key, '')
            ws.cell(row=r_idx, column=c_idx, value=value_to_str(val))

    autosize_columns(ws)

    # Create META sheet
    ws_meta = wb.create_sheet(title='Meta_data_sheet')
    # Write META headers
    for col_idx, key in enumerate(META_COLUMNS, start=1):
        cell = ws_meta.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True)
    # Write META rows from original rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(META_COLUMNS, start=1):
            v = row.get(key, '')
            # Raw, unnumbered; if list, join with newline
            if isinstance(v, list):
                v = '\n'.join(str(x) if x is not None else '' for x in v)
            ws_meta.cell(row=r_idx, column=c_idx, value='' if v is None else v)
    ws_meta.sheet_state = 'veryHidden'

    # Step 4/7: Rename 'Data' to 'TestPlan' (no new sheet created)
    ws.title = 'TestPlan'

    # Remove META columns and reorder to MAIN_COLUMNS on the same sheet
    # Build a mapping from column header to current index
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]

    # Build new data matrix with only MAIN_COLUMNS in exact order
    new_headers = MAIN_COLUMNS
    new_rows = []
    for r in range(2, ws.max_row + 1):
        row_vals = {}
        for c, h in enumerate(headers, start=1):
            row_vals[h] = ws.cell(row=r, column=c).value
        # Prepare ordered row by MAIN_COLUMNS, blank for missing
        ordered = [row_vals.get(col, '') for col in new_headers]
        new_rows.append(ordered)

    # Clear sheet and write back
    ws.delete_rows(1, ws.max_row)
    for c_idx, h in enumerate(new_headers, start=1):
        cell = ws.cell(row=1, column=c_idx, value=h)
        cell.font = Font(bold=True)
    for r_idx, ordered in enumerate(new_rows, start=2):
        for c_idx, v in enumerate(ordered, start=1):
            ws.cell(row=r_idx, column=c_idx, value=v)

    # Numbering logic for specific columns using original list values
    col_index_map = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    for r_idx, lt in enumerate(list_tracker, start=2):
        for col_name in NUMBERED_COLUMNS:
            c_idx = col_index_map.get(col_name)
            if not c_idx:
                continue
            orig_list = lt.get(col_name)
            if isinstance(orig_list, list):
                numbered = '\n'.join(f"{i+1}. {str(item) if item is not None else ''}" for i, item in enumerate(orig_list))
                ws.cell(row=r_idx, column=c_idx, value=numbered)

    # Formatting for TestPlan
    # Header styling
    header_fill = PatternFill(fill_type='solid', start_color='FF0070C0', end_color='FF0070C0')
    header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True, color='FFFFFFFF')
        cell.alignment = header_alignment
        cell.fill = header_fill

    # Data rows alignment and wrap
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            header = ws.cell(row=1, column=c).value
            if header in WRAP_COLUMNS:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            elif header == 'Index':
                cell.alignment = Alignment(vertical='top', horizontal='center')
            else:
                cell.alignment = Alignment(vertical='top', horizontal='left')

    autosize_columns(ws)
    estimate_row_heights(ws)
    apply_borders(ws)

    # Data Validation for 'Code Generation (Required / Not)' on data rows only
    code_col = col_index_map.get('Code Generation (Required / Not)')
    if code_col:
        from openpyxl.utils import get_column_letter
        col_letter = get_column_letter(code_col)
        if ws.max_row >= 2:
            dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
            dv.error = 'Select a value from the list.'
            dv.errorTitle = 'Invalid Input'
            ws.add_data_validation(dv)
            dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: only 'TestPlan' visible and 'Meta_data_sheet' veryHidden; ensure no 'Data' sheet
    if 'Data' in wb.sheetnames:
        # attempt to delete if exists
        try:
            del wb['Data']
        except Exception:
            raise RuntimeError('Validation failed: could not delete unexpected sheet named Data')

    # Enforce visibility states
    # TestPlan should be visible (default). Meta is veryHidden already.

    # Save to bytes buffer and validate OOXML parts
    bio = BytesIO()
    wb.save(bio)
    bio.seek(0)

    with zipfile.ZipFile(bio, 'r') as zf:
        names = set(zf.namelist())
        missing = [p for p in REQUIRED_OOXML_PARTS if p not in names]
        if missing:
            raise RuntimeError(f'OOXML validation failed; missing parts: {missing}')

    # Return the workbook bytes
    return bio.getvalue()


def write_and_maybe_commit(xlsx_bytes, ip_name, output_dir, do_commit):
    # Compute IST timestamp and filename rule
    ist = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(ist)
    fname = f"{ip_name}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"
    out_dir = output_dir
    out_path = os.path.join(out_dir, fname)

    os.makedirs(out_dir, exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(xlsx_bytes)

    # Re-validate on-disk as true XLSX zip
    try:
        with zipfile.ZipFile(out_path, 'r') as zf:
            names = set(zf.namelist())
            for req in REQUIRED_OOXML_PARTS:
                assert req in names
    except Exception as e:
        print(f"ERROR: Saved file validation failed: {e}", file=sys.stderr)
        sys.exit(2)

    if do_commit:
        # Commit only this file
        import subprocess
        try:
            subprocess.run(['git', 'config', 'user.name', 'github-actions[bot]'], check=True)
            subprocess.run(['git', 'config', 'user.email', '41898282+github-actions[bot]@users.noreply.github.com'], check=True)
            subprocess.run(['git', 'add', out_path], check=True)
            subprocess.run(['git', 'commit', '-m', 'Final formatted Excel generated from JSON input'], check=True)
            subprocess.run(['git', 'push'], check=True)
        except subprocess.CalledProcessError as e:
            print(f"ERROR: Git commit/push failed: {e}", file=sys.stderr)
            sys.exit(3)

    print(f"Generated: {out_path}")


def main():
    args = parse_args()
    rows = load_json_array(args.json_file)
    if not rows:
        print('ERROR: Input JSON is empty', file=sys.stderr)
        sys.exit(1)

    try:
        xbytes = generate_workbook(rows, args.ip_name, args.output_dir)
    except Exception as e:
        print(f"ERROR: Workbook generation/validation failed: {e}", file=sys.stderr)
        sys.exit(1)

    write_and_maybe_commit(xbytes, args.ip_name, args.output_dir, args.commit)


if __name__ == '__main__':
    main()
