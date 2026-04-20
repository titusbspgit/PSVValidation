#!/usr/bin/env python3
import sys, os, json
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

def measure_width(s):
    if s is None:
        return 0
    if not isinstance(s, str):
        s = str(s)
    lines = s.splitlines() or [s]
    return max(len(line) for line in lines)

def main():
    if len(sys.argv) < 3:
        print("Usage: json_to_xlsx.py <input_json_path> <output_xlsx_path>", file=sys.stderr)
        sys.exit(2)
    in_path = sys.argv[1]
    out_path = sys.argv[2]

    with open(in_path, 'r', encoding='utf-8') as f:
        data = json.load(f, object_pairs_hook=OrderedDict)

    # Determine records
    if isinstance(data, list):
        records = data
    elif isinstance(data, dict):
        if 'test_cases' in data and isinstance(data['test_cases'], list):
            records = data['test_cases']
        else:
            records = [data]
    else:
        raise SystemExit("Unsupported JSON structure")

    if not records:
        raise SystemExit("Empty JSON array/object after extraction")

    # Build columns preserving first-seen order
    columns = []
    for rec in records:
        if not isinstance(rec, dict):
            raise SystemExit("All records must be JSON objects")
        for k in rec.keys():
            if k not in columns:
                columns.append(k)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header
    header_font = Font(bold=True)
    for c_idx, col_name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=c_idx, value=col_name)
        cell.font = header_font

    # Rows
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, col_name in enumerate(columns, start=1):
            val = rec.get(col_name, "")
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Auto-fit columns
    max_widths = [measure_width(col) for col in columns]
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, col_name in enumerate(columns, start=1):
            val = rec.get(col_name, "")
            w = measure_width(val)
            if w > max_widths[c_idx-1]:
                max_widths[c_idx-1] = w
    for c_idx, w in enumerate(max_widths, start=1):
        adj = min(100, w + 2)
        ws.column_dimensions[get_column_letter(c_idx)].width = adj

    os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
    wb.save(out_path)

if __name__ == "__main__":
    main()
