#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def normalize_rows(data):
    # Accept either top-level object with key 'TestCases' or a list of objects
    if isinstance(data, dict) and 'TestCases' in data and isinstance(data['TestCases'], list):
        rows = data['TestCases']
    elif isinstance(data, list):
        rows = data
    elif isinstance(data, dict):
        rows = [data]
    else:
        raise ValueError('Unsupported JSON structure for tabular conversion')

    # Determine column order preserving first-seen order across all rows
    columns = []
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError('All rows must be JSON objects (dicts)')
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                columns.append(k)
    return columns, rows


def write_excel(columns, rows, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Header
    ws.append(columns)
    bold = Font(bold=True)
    for c in range(1, len(columns) + 1):
        ws.cell(row=1, column=c).font = bold

    # Rows
    for row in rows:
        ws.append([row.get(col, '') for col in columns])

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Auto-fit columns by computing max string length
    for idx, col in enumerate(columns, start=1):
        max_len = len(str(col))
        for r in range(2, len(rows) + 2):
            val = ws.cell(row=r, column=idx).value
            s = '' if val is None else str(val)
            if len(s) > max_len:
                max_len = len(s)
        # Estimate width (characters)
        ws.column_dimensions[chr(64 + idx)].width = min(max(12, max_len + 2), 120)

    # Save
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)


def main():
    ap = argparse.ArgumentParser(description='Convert JSON to single-sheet Excel (IST time, deterministic).')
    ap.add_argument('--input', required=True, help='Path to input JSON file in repo')
    ap.add_argument('--ip-name', required=True, help='IP name for filename rule')
    ap.add_argument('--outdir', required=True, help='Output directory inside repo')
    args = ap.parse_args()

    data = load_json(args.input)
    columns, rows = normalize_rows(data)

    # Time in IST (Asia/Kolkata)
    if ZoneInfo is not None:
        now = datetime.now(ZoneInfo('Asia/Kolkata'))
    else:
        # Fallback: manual offset +05:30 (does not handle DST which IST does not have)
        from datetime import timedelta
        now = datetime.utcnow() + timedelta(hours=5, minutes=30)

    fname = f"{args.ip_name}_TestPlan_{now:%Y%m%d}_{now:%H%M%S}.xlsx"
    out_path = os.path.join(args.outdir, fname)

    write_excel(columns, rows, out_path)

    # Output resulting path for workflow logs
    print(out_path)


if __name__ == '__main__':
    main()
