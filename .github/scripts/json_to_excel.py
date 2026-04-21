#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter


def load_json(path: str):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def validate_structure(data: dict):
    if not isinstance(data, dict):
        raise ValueError('JSON root must be an object')
    if 'metadata' not in data or 'test_cases' not in data:
        raise ValueError('JSON must contain "metadata" and "test_cases"')
    if not isinstance(data['test_cases'], list) or len(data['test_cases']) == 0:
        raise ValueError('"test_cases" must be a non-empty array')


def union_keys_preserve_order(records):
    seen = []
    in_seen = set()
    for rec in records:
        if not isinstance(rec, dict):
            raise ValueError('Each test case must be an object')
        for k in rec.keys():
            if k not in in_seen:
                in_seen.add(k)
                seen.append(k)
    return seen


def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def json_value_to_cell(val):
    # Preserve exact value; arrays/objects become JSON strings
    if isinstance(val, (dict, list)):
        return json.dumps(val, ensure_ascii=False)
    return val


def build_workbook(metadata, records, meta_columns, record_columns):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    headers = meta_columns + record_columns
    ws.append(headers)

    # Bold header
    for cell in ws[1]:
        cell.font = Font(bold=True)

    # Rows
    for rec in records:
        row = []
        for c in meta_columns:
            row.append(metadata.get(c, ''))
        for c in record_columns:
            row.append(json_value_to_cell(rec.get(c, '')))
        ws.append(row)

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Auto-fit columns based on max length (basic heuristic)
    for idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            try:
                val = '' if cell.value is None else str(cell.value)
            except Exception:
                val = ''
            if len(val) > max_len:
                max_len = len(val)
        # Set width with bounds
        width = min(max(12, max_len + 2), 80)
        ws.column_dimensions[get_column_letter(idx)].width = width

    return wb


def compute_filename(ip_name: str, tz_name: str):
    tz = ZoneInfo(tz_name)
    now = datetime.now(tz)
    ymd = now.strftime('%Y%m%d')
    hms = now.strftime('%H%M%S')
    return f"{ip_name}_TestPlan_{ymd}_{hms}.xlsx"


def main():
    parser = argparse.ArgumentParser(description='Convert Test Plan JSON to single-sheet Excel')
    parser.add_argument('--input', required=True, help='Path to input JSON file')
    parser.add_argument('--output-dir', required=True, help='Directory to write the Excel file into')
    parser.add_argument('--ip-name', required=True, help='IP name for filename rule')
    parser.add_argument('--timezone', default='Asia/Kolkata', help='IANA timezone for IST timestamp in filename')
    args = parser.parse_args()

    data = load_json(args.input)
    validate_structure(data)

    metadata = data.get('metadata', {})
    test_cases = data.get('test_cases', [])

    # Metadata columns first and in this exact order
    meta_columns = ['ip', 'repository', 'branch', 'subdirectory', 'generated_timestamp_IST']
    record_columns = union_keys_preserve_order(test_cases)

    wb = build_workbook(metadata, test_cases, meta_columns, record_columns)

    ensure_dir(args.output_dir)
    filename = compute_filename(args.ip-name if hasattr(args, 'ip-name') else args.ip_name, args.timezone)
    # Correct attribute: argparse uses underscore, not hyphen
    filename = compute_filename(args.ip_name, args.timezone)
    out_path = os.path.join(args.output_dir, filename)
    wb.save(out_path)

    print(f"WROTE {out_path}")

if __name__ == '__main__':
    main()
