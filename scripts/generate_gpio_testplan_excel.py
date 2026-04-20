#!/usr/bin/env python3
import argparse
import json
import os
import sys
import datetime
from collections import OrderedDict

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment
except Exception as e:
    print(f"ERROR: openpyxl is required: {e}", file=sys.stderr)
    sys.exit(1)


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f, object_pairs_hook=OrderedDict)


def validate_json(data):
    if not isinstance(data, dict):
        raise ValueError('Top-level JSON must be an object')
    if 'testcases' not in data or not isinstance(data['testcases'], list):
        raise ValueError("JSON must contain 'testcases' as an array")
    for i, row in enumerate(data['testcases']):
        if not isinstance(row, dict):
            raise ValueError(f"Each testcase must be an object (row {i})")


def build_columns(rows):
    seen = set()
    cols = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def stringify(value):
    if value is None:
        return ''
    if isinstance(value, (list, tuple)):
        return '\n'.join([stringify(v) for v in value])
    if isinstance(value, dict):
        return json.dumps(value, ensure_ascii=False)
    return str(value)


def autosize_columns(ws, max_width=100):
    col_widths = {}
    for i, row in enumerate(ws.iter_rows(values_only=True), start=1):
        for j, cell in enumerate(row, start=1):
            val = '' if cell is None else str(cell)
            length = max(len(v) for v in val.split('\n')) if val else 0
            col_widths[j] = max(col_widths.get(j, 0), length)
    for j, width in col_widths.items():
        ws.column_dimensions[chr(64 + j)].width = min(max(width + 2, 10), max_width)


def write_excel(data, out_dir):
    rows = data['testcases']
    cols = build_columns(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = 'GPIO_TestPlan'

    # Header
    for c, key in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(wrap_text=True, vertical='top')

    # Rows
    for r, rowobj in enumerate(rows, start=2):
        for c, key in enumerate(cols, start=1):
            cell = ws.cell(row=r, column=c, value=stringify(rowobj.get(key, '')))
            cell.alignment = Alignment(wrap_text=True, vertical='top')

    ws.freeze_panes = 'A2'
    autosize_columns(ws)

    # Compute IST timestamp for filename (prefer metadata.generated_on)
    ts = (data.get('metadata') or {}).get('generated_on')
    try:
        dt = datetime.datetime.fromisoformat(str(ts).replace('Z', '+00:00'))
    except Exception:
        dt = datetime.datetime.now(datetime.timezone.utc)
    ist = datetime.timezone(datetime.timedelta(hours=5, minutes=30))
    ist_dt = dt.astimezone(ist)
    fname = f"GPIO_TestPlan_{ist_dt.strftime('%Y%m%d')}_{ist_dt.strftime('%H%M%S')}.xlsx"

    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)
    print(out_path)
    return out_path


def main():
    ap = argparse.ArgumentParser(description='Convert JSON test plan to Excel')
    ap.add_argument('--input', required=True, help='Path to JSON file')
    ap.add_argument('--output-dir', required=True, help='Directory to save Excel')
    args = ap.parse_args()

    data = load_json(args.input)
    validate_json(data)
    write_excel(data, args.output-dir)

if __name__ == '__main__':
    main()

# trigger: retrigger workflow