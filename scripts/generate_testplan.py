#!/usr/bin/env python3
import argparse
import base64
import io
import json
import math
import os
import re
import sys
import zipfile
from typing import List, Dict, Any

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

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

META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]

WRAP_COLS = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria'
}

HEADER_FILL_BLUE = PatternFill(fill_type='solid', fgColor='1F4E78')
THIN_BORDER = Border(
    left=Side(style='thin', color='000000'),
    right=Side(style='thin', color='000000'),
    top=Side(style='thin', color='000000'),
    bottom=Side(style='thin', color='000000')
)


def parse_args():
    p = argparse.ArgumentParser(description='Generate formatted TestPlan Excel from JSON (fallback automation)')
    p.add_argument('--ip-name', required=True)
    p.add_argument('--output-dir', required=True)
    p.add_argument('--date', required=True, help='IST date YYYYMMDD')
    p.add_argument('--time', required=True, help='IST time HHMMSS')
    p.add_argument('--json-b64', required=True)
    return p.parse_args()


def decode_json(b64_text: str) -> List[Dict[str, Any]]:
    try:
        raw = base64.b64decode(b64_text.encode('ascii'))
        data = json.loads(raw)
        if not isinstance(data, list) or not data:
            raise ValueError('JSON must be a non-empty array of objects')
        # Ensure each item is a dict
        for i, it in enumerate(data):
            if not isinstance(it, dict):
                raise ValueError(f'JSON array element at index {i} is not an object')
        return data
    except Exception as e:
        raise RuntimeError(f'Invalid JSON input: {e}')


def union_keys_preserve_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    s = set()
    for obj in rows:
        for k in obj.keys():  # dict preserves original key order
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def write_base_data_sheet(wb: Workbook, rows: List[Dict[str, Any]], cols: List[str]):
    ws = wb.active
    ws.title = 'Data'  # authoritative staging sheet
    # Header
    for cidx, col in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=cidx, value=col)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'
    # Data
    for ridx, obj in enumerate(rows, start=2):
        for cidx, col in enumerate(cols, start=1):
            val = obj.get(col, '')
            ws.cell(row=ridx, column=cidx, value=val)
    # Approx autofit widths
    autofit_columns(ws)


def autofit_columns(ws):
    max_col = ws.max_column
    max_row = ws.max_row
    for c in range(1, max_col + 1):
        letter = get_column_letter(c)
        max_len = 0
        for r in range(1, max_row + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                ln = 0
            else:
                s = str(v)
                ln = max(len(part) for part in s.split('\n')) if '\n' in s else len(s)
            if ln > max_len:
                max_len = ln
        width = min(100, max(10, int(max_len * 1.1) + 2))
        ws.column_dimensions[letter].width = width


def create_meta_sheet(wb: Workbook, rows: List[Dict[str, Any]]):
    ws = wb.create_sheet('Meta_data_sheet')
    # Header
    for cidx, col in enumerate(META_COLS, start=1):
        ws.cell(row=1, column=cidx, value=col).font = Font(bold=True)
    # Data
    for ridx, obj in enumerate(rows, start=2):
        for cidx, col in enumerate(META_COLS, start=1):
            ws.cell(row=ridx, column=cidx, value=obj.get(col, ''))
    # Very hidden
    ws.sheet_state = 'veryHidden'


def normalize_main_sheet(wb: Workbook):
    ws = wb['Data']
    # Rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Build a mapping from header to column index (before any deletion)
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    header_to_idx = {h: i for i, h in enumerate(headers, start=1)}

    # Determine columns to keep in final order
    keep_headers = [h for h in MAIN_ORDER]

    # Create a temporary in-memory table for the reordered data
    data_rows = []
    for r in range(2, ws.max_row + 1):
        row_dict = {}
        for h in keep_headers:
            cidx = header_to_idx.get(h)
            val = ws.cell(row=1, column=cidx).value if cidx else None
            if cidx:
                row_dict[h] = ws.cell(row=r, column=cidx).value
            else:
                row_dict[h] = ''
        data_rows.append(row_dict)

    # Clear sheet and write reordered headers + data
    for row in ws[1:ws.max_row]:
        for cell in row:
            cell.value = None
    ws.delete_cols(1, ws.max_column)

    for cidx, h in enumerate(keep_headers, start=1):
        cell = ws.cell(row=1, column=cidx, value=h)
        cell.font = Font(bold=True)

    for ridx, row_dict in enumerate(data_rows, start=2):
        for cidx, h in enumerate(keep_headers, start=1):
            ws.cell(row=ridx, column=cidx, value=row_dict.get(h, ''))


def apply_strict_formatting(ws):
    max_row = ws.max_row
    max_col = ws.max_column

    # Determine header style
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    data_top = Alignment(vertical='top')
    data_left = Alignment(horizontal='left', vertical='top', wrap_text=True)
    data_center = Alignment(horizontal='center', vertical='top', wrap_text=True)
    data_right = Alignment(horizontal='right', vertical='top', wrap_text=True)

    headers = [ws.cell(row=1, column=i).value for i in range(1, max_col + 1)]

    # Header styling
    for c in range(1, max_col + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = header_align
        cell.fill = HEADER_FILL_BLUE

    # Data row alignment and wrap rules
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            h = headers[c - 1]
            cell = ws.cell(row=r, column=c)
            # Wrap for specific columns
            if h in WRAP_COLS:
                cell.alignment = data_left
            else:
                if h == 'Index':
                    cell.alignment = data_center
                else:
                    # treat as text by default
                    cell.alignment = data_left

    # Add thin borders to all populated cells
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).border = THIN_BORDER

    # Autofit columns and adjust row heights after wrapping
    autofit_columns(ws)
    adjust_row_heights(ws, headers)


def adjust_row_heights(ws, headers: List[str]):
    # Estimate height based on wrapped text length per column width
    base_height = 15  # points per line approx
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            h = headers[c - 1]
            if h not in WRAP_COLS:
                continue
            val = ws.cell(row=r, column=c).value
            if val is None:
                continue
            text = str(val)
            width = ws.column_dimensions[get_column_letter(c)].width or 10
            # Rough chars per line approximation (Excel uses proportional fonts; heuristic only)
            chars_per_line = max(10, int(width))
            lines = 0
            for line in str(text).split('\n'):
                line = line.strip()
                if not line:
                    lines += 1
                    continue
                lines += max(1, math.ceil(len(line) / chars_per_line))
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines


def renumber_multiline(text: str) -> str:
    if text is None:
        return ''
    parts = re.split(r'\r?\n+', str(text))
    items = []
    for p in parts:
        s = p.strip()
        if not s:
            continue
        # Strip common bullets or existing numbering like "1)", "1.", "-", "*", "•"
        s = re.sub(r'^(?:\d+[\.)\-]*\s*|[\-\*•]\s*)', '', s)
        items.append(s)
    if not items:
        return ''
    return '\n'.join([f"{i}. {items[i-1]}" for i in range(1, len(items) + 1)])


def apply_numbering_in_cells(ws):
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    targets = {
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria'
    }
    target_indices = [i + 1 for i, h in enumerate(headers) if h in targets]
    for r in range(2, ws.max_row + 1):
        for c in target_indices:
            cell = ws.cell(row=r, column=c)
            cell.value = renumber_multiline(cell.value)


def apply_data_validation(ws):
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    try:
        col_idx = headers.index('Code Generation (Required / Not)') + 1
    except ValueError:
        return
    dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True)
    ws.add_data_validation(dv)
    data_range = f"{get_column_letter(col_idx)}2:{get_column_letter(col_idx)}{ws.max_row}"
    dv.add(data_range)


def enforce_final_sheets(wb: Workbook):
    # Must have only TestPlan (visible) and Meta_data_sheet (veryHidden)
    names = [ws.title for ws in wb.worksheets]
    if 'Data' in names:
        # Try to delete if any leftover named 'Data'
        for ws in list(wb.worksheets):
            if ws.title == 'Data':
                wb.remove(ws)
    # Recheck
    names = [ws.title for ws in wb.worksheets]
    if 'Data' in names:
        raise RuntimeError('Safety check failed: sheet named "Data" still exists')
    if 'TestPlan' not in names:
        raise RuntimeError('Safety check failed: sheet "TestPlan" missing')
    if 'Meta_data_sheet' not in names:
        raise RuntimeError('Safety check failed: sheet "Meta_data_sheet" missing')
    # Ensure Meta_data_sheet veryHidden
    meta = wb['Meta_data_sheet']
    meta.sheet_state = 'veryHidden'


def validate_xlsx_binary(path: str):
    if not zipfile.is_zipfile(path):
        raise RuntimeError('Output is not a ZIP-based XLSX file')
    with zipfile.ZipFile(path, 'r') as zf:
        if 'xl/workbook.xml' not in zf.namelist():
            raise RuntimeError('Missing core workbook entry in XLSX')
    # Try to open with openpyxl
    _ = load_workbook(filename=path, read_only=True)


def main():
    args = parse_args()
    rows = decode_json(args.json_b64)

    # PHASE 1 — Normalize schema and write Data
    cols = union_keys_preserve_order(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    write_base_data_sheet(wb, rows, cols)

    # PHASE 2 — Create META and normalize main sheet on the same original sheet
    create_meta_sheet(wb, rows)

    # Identify and lock main sheet (rename Data -> TestPlan and reorder on it)
    normalize_main_sheet(wb)

    # Strict formatting + numbering
    main_ws = wb['TestPlan']
    apply_numbering_in_cells(main_ws)
    apply_strict_formatting(main_ws)

    # Data validation after formatting
    apply_data_validation(main_ws)

    # Safety check for sheet visibility/state
    enforce_final_sheets(wb)

    # PHASE 3 — Save and validate
    file_name = f"{args.ip_name}_TestPlan_{args.date}_{args.time}.xlsx"
    out_dir = args.output_dir
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, file_name)

    wb.save(out_path)

    # Validate as true XLSX and openable
    validate_xlsx_binary(out_path)

    print(out_path)


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
