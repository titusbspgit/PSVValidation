#!/usr/bin/env python3
import json
import sys
import os
import argparse
import datetime as dt
from zipfile import ZipFile
from io import BytesIO
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
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


def ist_now():
    # IST is UTC+5:30
    return dt.datetime.utcnow() + dt.timedelta(hours=5, minutes=30)


def normalize_records(raw):
    # raw can be array or object {TC1: {...}, ...}
    if isinstance(raw, dict):
        # preserve insertion order of keys in dict (Python 3.7+ preserves)
        records = list(raw.values())
    elif isinstance(raw, list):
        records = raw
    else:
        raise ValueError('json_data must be an array or an object of test cases')
    if not records:
        raise ValueError('json_data has no records')
    # Build ordered union of keys by first-seen order across all rows
    seen = []
    for rec in records:
        if not isinstance(rec, dict):
            raise ValueError('Each record must be an object')
        for k in rec.keys():
            if k not in seen:
                seen.append(k)
    return records, seen


def to_list(v):
    if v is None:
        return ''
    if isinstance(v, list):
        return ', '.join(str(x) for x in v)
    return v


def autosize_columns(ws):
    col_widths = {}
    for r in ws.iter_rows(values_only=True):
        for i, v in enumerate(r, start=1):
            s = '' if v is None else str(v)
            l = len(s)
            if l == 0:
                l = 1
            col_widths[i] = max(col_widths.get(i, 0), min(120, l + 2))
    for i, w in col_widths.items():
        ws.column_dimensions[ws.cell(row=1, column=i).column_letter].width = w


def apply_borders(ws):
    thin = Side(border_style='thin', color='000000')
    border = Border(top=thin, left=thin, right=thin, bottom=thin)
    max_row = ws.max_row
    max_col = ws.max_column
    for r in ws.iter_rows(min_row=1, max_row=max_row, min_col=1, max_col=max_col):
        for c in r:
            c.border = border


def format_header(ws):
    header_fill = PatternFill('solid', fgColor='4F81BD')  # blue
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=False)
        cell.fill = header_fill


def wrap_and_align(ws, wrap_cols, index_col_name='Index'):
    # Identify columns by name
    headers = [c.value for c in ws[1]]
    wrap_idx = [headers.index(n) + 1 for n in wrap_cols if n in headers]
    idx_index = headers.index(index_col_name) + 1 if index_col_name in headers else None

    for row in ws.iter_rows(min_row=2, max_row=ws.max_row):
        for cell in row:
            # default align
            cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=cell.column in wrap_idx)
            # index numeric center
            if idx_index and cell.column == idx_index:
                cell.alignment = Alignment(horizontal='center', vertical='top', wrap_text=False)


def std_numbering(text):
    if text is None:
        return ''
    s = str(text).strip()
    if not s:
        return ''
    # Split by common delimiters and normalize numbering to 1., 2., ... keeping single cell
    # First, convert existing patterns like '1) ' or '1 - ' to '1. '
    import re
    s = re.sub(r'(?m)^(\s*\d+)\)', r'\1.', s)
    s = re.sub(r'(?m)^(\s*\d+)\s*-\s*', r'\1. ', s)
    # If not already multiline numbers, attempt to split by '. ' or '; ' or '.\n'
    # Ensure each item starts with N.
    lines = []
    # Use existing line breaks if present
    if '\n' in s:
        parts = [p.strip() for p in s.split('\n') if p.strip()]
    else:
        # split by numbered markers or ';'
        parts = re.split(r'\s*\d+\.|;\s*', s)
        parts = [p.strip() for p in parts if p.strip()]
    for i, p in enumerate(parts, start=1):
        # Remove any leading numbering again
        p = re.sub(r'^\d+[\.)]\s*', '', p)
        lines.append(f"{i}. {p}")
    return '\n'.join(lines)


def enforce_numbering(ws):
    headers = [c.value for c in ws[1]]
    for col_name in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
        if col_name in headers:
            col_idx = headers.index(col_name) + 1
            for r in range(2, ws.max_row + 1):
                v = ws.cell(row=r, column=col_idx).value
                ws.cell(row=r, column=col_idx, value=std_numbering(v))


def apply_validation(ws):
    headers = [c.value for c in ws[1]]
    if 'Code Generation (Required / Not)' not in headers:
        return
    col_idx = headers.index('Code Generation (Required / Not)') + 1
    max_row = ws.max_row
    col_letter = ws.cell(row=1, column=col_idx).column_letter
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
    ws.add_data_validation(dv)
    dv.add(f"{col_letter}2:{col_letter}{max_row}")


def build_workbook(records, key_order):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'
    # Write headers in key_order
    ws.append(key_order)
    # Rows
    for rec in records:
        row = []
        for k in key_order:
            v = rec.get(k, '')
            v = to_list(v)
            row.append(v)
        ws.append(row)
    # Base formatting
    ws.freeze_panes = 'A2'
    format_header(ws)
    autosize_columns(ws)
    apply_borders(ws)

    # Create META sheet and copy META cols
    meta = wb.create_sheet('Meta_data_sheet')
    meta.append(META_COLS)
    # map headers to index for reading from Data sheet
    headers = {v: i+1 for i, v in enumerate([c.value for c in ws[1]])}
    for r in range(2, ws.max_row + 1):
        meta_row = []
        for k in META_COLS:
            if k in headers:
                meta_row.append(ws.cell(row=r, column=headers[k]).value)
            else:
                meta_row.append('')
        meta.append(meta_row)
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Normalize main sheet: remove META columns and reorder to MAIN_ORDER
    # First, build a list of current headers excluding META
    current_headers = [c.value for c in ws[1]]
    remaining = [h for h in current_headers if h not in META_COLS]
    # Reorder columns to MAIN_ORDER using only those present
    final_headers = [h for h in MAIN_ORDER if h in remaining]
    # Build a mapping from header to column index
    hdr_to_idx = {h: current_headers.index(h) + 1 for h in current_headers}

    # Create a new table in memory for the reordered columns
    data_rows = []
    for r in range(2, ws.max_row + 1):
        data_rows.append({h: ws.cell(row=r, column=hdr_to_idx[h]).value if h in hdr_to_idx else '' for h in final_headers})

    # Clear sheet and write final headers and data
    ws.delete_rows(1, ws.max_row)
    ws.append(final_headers)
    for row_dict in data_rows:
        ws.append([row_dict.get(h, '') for h in final_headers])

    # Rename sheet to TestPlan
    ws.title = 'TestPlan'

    # Formatting for TestPlan
    format_header(ws)
    wrap_and_align(ws, wrap_cols=['Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'])
    autosize_columns(ws)
    apply_borders(ws)
    enforce_numbering(ws)
    apply_validation(ws)

    # Safety: ensure there is no sheet named 'Data'
    if any(s.title == 'Data' for s in wb.worksheets):
        # attempt to delete; if not possible, raise
        try:
            del wb['Data']
        except Exception:
            raise RuntimeError("Sheet named 'Data' still exists; normalization failed")

    return wb


def validate_xlsx_bytes(xbytes: bytes):
    bio = BytesIO(xbytes)
    with ZipFile(bio) as zf:
        must_have = ['[Content_Types].xml', 'xl/workbook.xml']
        for m in must_have:
            if m not in zf.namelist():
                raise ValueError(f'Missing required part: {m}')


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True, help='Path to JSON input (array or object)')
    ap.add_argument('--ip', required=True, help='IP_NAME (for filename rule)')
    ap.add_argument('--outdir', required=True, help='Output directory relative to repo root')
    ap.add_argument('--branch', default='main')
    args = ap.parse_args()

    with open(args.json, 'r', encoding='utf-8') as f:
        raw = json.load(f)

    records, key_order = normalize_records(raw)

    # Build workbook
    wb = build_workbook(records, key_order)

    # Compose filename based on IST timestamp
    now = ist_now()
    datestr = now.strftime('%Y%m%d')
    timestr = now.strftime('%H%M%S')
    fname = f"{args.ip}_TestPlan_{datestr}_{timestr}.xlsx"
    outdir = args.outdir.rstrip('/')
    os.makedirs(outdir, exist_ok=True)
    fpath = os.path.join(outdir, fname)

    # Save to bytes and validate
    bio = BytesIO()
    wb.save(bio)
    xbytes = bio.getvalue()
    validate_xlsx_bytes(xbytes)

    with open(fpath, 'wb') as wf:
        wf.write(xbytes)

    # Emit shell-friendly outputs for the workflow step to consume
    print(f"OUTFILE={fpath}")
    print(f"TIMESTAMP_IST={datestr} {timestr}")


if __name__ == '__main__':
    main()
