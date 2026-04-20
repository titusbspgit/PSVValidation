#!/usr/bin/env python3
import argparse
import json
import os
import re
import sys
from typing import Any, Dict, List

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except Exception as e:
    print(f"ERROR: openpyxl not available: {e}")
    sys.exit(2)


def load_json(path: str) -> Any:
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def derive_rows(data: Any) -> List[Dict[str, Any]]:
    # Supported shapes:
    # 1) list[dict]
    # 2) dict with key 'test_cases' as list[dict]
    # 3) dict -> single row
    if isinstance(data, list):
        if all(isinstance(x, dict) for x in data):
            return data
        else:
            raise ValueError('Unsupported JSON array contents; expected array of objects')
    if isinstance(data, dict):
        if 'test_cases' in data and isinstance(data['test_cases'], list) and all(isinstance(x, dict) for x in data['test_cases']):
            return data['test_cases']
        return [data]
    raise ValueError('Unsupported JSON structure; expected object or array of objects')


def union_keys(rows: List[Dict[str, Any]]) -> List[str]:
    seen = set()
    cols: List[str] = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def to_cell(v: Any) -> str:
    # Preserve exact JSON values: stringify nested structures deterministically
    if isinstance(v, (dict, list)):
        return json.dumps(v, ensure_ascii=False, separators=(",", ":"))
    if v is None:
        return ''
    return str(v)


def autosize(ws):
    # Simple auto-fit by content width
    for col_cells in ws.columns:
        length = 0
        col_letter = col_cells[0].column_letter
        for cell in col_cells:
            v = '' if cell.value is None else str(cell.value)
            if len(v) > length:
                length = len(v)
        width = min(max(10, length + 2), 100)
        ws.column_dimensions[col_letter].width = width


def build_workbook(rows: List[Dict[str, Any]], sheet_name: str = 'Data') -> Workbook:
    cols = union_keys(rows)
    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name
    # Header
    ws.append(cols)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'
    # Rows
    for r in rows:
        ws.append([to_cell(r.get(c, '')) for c in cols])
    autosize(ws)
    return wb


def default_output_from_input(input_path: str) -> str:
    # If input filename ends with _TestPlan_YYYYMMDD_HHMMSS_IST.json, mirror the same base with .xlsx
    bname = os.path.basename(input_path)
    m = re.match(r'^(.*_TestPlan_\d{8}_\d{6}_IST)\.json$', bname)
    if m:
        base = m.group(1)
        return os.path.join(os.path.dirname(input_path), base + '.xlsx')
    # else, just replace extension
    return os.path.splitext(input_path)[0] + '.xlsx'


def main():
    ap = argparse.ArgumentParser(description='Convert JSON to single-sheet Excel (.xlsx) with sheet name Data.')
    ap.add_argument('--input', required=True, help='Path to JSON file')
    ap.add_argument('--output', required=False, help='Path to output .xlsx file')
    ap.add_argument('--sheet-name', default='Data', help='Worksheet name (default: Data)')
    args = ap.parse_args()

    data = load_json(args.input)
    rows = derive_rows(data)
    if not rows:
        print('ERROR: Empty JSON array or no rows detected', file=sys.stderr)
        sys.exit(1)

    wb = build_workbook(rows, sheet_name=args.sheet_name)

    out_path = args.output or default_output_from_input(args.input)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)
    print(f'Wrote Excel: {out_path}')


if __name__ == '__main__':
    main()
