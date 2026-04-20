#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime

try:
    # Python 3.9+: zoneinfo available
    from zoneinfo import ZoneInfo
    tz_ist = ZoneInfo('Asia/Kolkata')
except Exception:
    # Fallback: fixed offset +05:30 without DST handling
    from datetime import timedelta, tzinfo
    class ISTFixed(tzinfo):
        def utcoffset(self, dt): return timedelta(hours=5, minutes=30)
        def tzname(self, dt): return 'IST'
        def dst(self, dt): return timedelta(0)
    tz_ist = ISTFixed()

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment


def parse_args():
    p = argparse.ArgumentParser(description='Convert JSON to single-sheet Excel with deterministic schema and IST-based filename rule.')
    p.add_argument('--input', required=True, help='Path to input JSON file')
    p.add_argument('--output-dir', required=True, help='Directory to write the Excel file into')
    p.add_argument('--ip-name', required=True, help='IP name placeholder for naming rule')
    p.add_argument('--sheet-name', default='GPIO_TestPlan', help='Worksheet name')
    p.add_argument('--naming-rule', required=True, help='Filename rule, e.g., <IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx')
    return p.parse_args()


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def build_schema(rows):
    # Preserve key order based on first appearance across rows
    columns = []
    seen = set()
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                columns.append(k)
    return columns


def stringify(value):
    if value is None:
        return ''
    if isinstance(value, list):
        return '\n'.join([stringify(v) for v in value])
    if isinstance(value, dict):
        return json.dumps(value, ensure_ascii=False)
    return str(value)


def write_excel(rows, columns, sheet_name, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name

    # Header
    header_font = Font(bold=True)
    ws.append(columns)
    for col_idx in range(1, len(columns) + 1):
        cell = ws.cell(row=1, column=col_idx)
        cell.font = header_font

    # Rows
    for r in rows:
        ws.append([stringify(r.get(col, '')) for col in columns])

    # Formatting: freeze header, wrap text, basic width
    ws.freeze_panes = 'A2'
    wrap = Alignment(wrap_text=True, vertical='top')
    max_len = {i: len(str(columns[i-1])) for i in range(1, len(columns)+1)}
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=len(columns)):
        for cell in row:
            cell.alignment = wrap
            text = str(cell.value) if cell.value is not None else ''
            l = max(len(line) for line in text.split('\n')) if text else 0
            if l > max_len[cell.column]:
                max_len[cell.column] = l
    for i in range(1, len(columns)+1):
        # Approximate width: characters + padding, clamp to [15, 120]
        width = max(15, min(120, max_len[i] + 2))
        ws.column_dimensions[chr(64+i) if i <= 26 else ('A' + chr(64 + (i-26)))].width = width

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)


def compute_ist_filename(meta, ip_name, naming_rule):
    # Determine base timestamp in UTC
    gen_on = None
    if isinstance(meta, dict):
        gen_on = meta.get('generated_on')
    if gen_on:
        try:
            # Expect ISO8601 with Z
            dt_utc = datetime.fromisoformat(gen_on.replace('Z', '+00:00'))
        except Exception:
            dt_utc = datetime.utcnow().replace(tzinfo=ZoneInfo('UTC') if 'ZoneInfo' in globals() else None)
    else:
        dt_utc = datetime.utcnow().replace(tzinfo=ZoneInfo('UTC') if 'ZoneInfo' in globals() else None)

    # Convert to IST
    try:
        dt_ist = dt_utc.astimezone(tz_ist)
    except Exception:
        dt_ist = dt_utc  # fallback without tz conversion

    ymd = dt_ist.strftime('%Y%m%d')
    hms = dt_ist.strftime('%H%M%S')
    ist_human = dt_ist.strftime('%Y-%m-%d %H:%M:%S IST')

    fname = naming_rule
    fname = fname.replace('<IP_NAME>', ip_name)
    fname = fname.replace('<YYYYMMDD>', ymd)
    fname = fname.replace('<HHMMSS>', hms)

    return fname, ist_human


def main():
    args = parse_args()
    data = load_json(args.input)

    # Validate structure
    if not isinstance(data, dict) or 'testcases' not in data or not isinstance(data['testcases'], list) or len(data['testcases']) == 0:
        raise SystemExit('Invalid or empty JSON: expected object with non-empty "testcases" array')

    rows = data['testcases']
    columns = build_schema(rows)

    # Compute filename from metadata.generated_on using IST
    meta = data.get('metadata', {})
    filename, ist_human = compute_ist_filename(meta, args.ip_name, args.naming_rule)

    out_path = os.path.join(args.output_dir, filename)
    write_excel(rows, columns, args.sheet_name, out_path)

    # Expose outputs for GitHub Actions
    gha_out = os.getenv('GITHUB_OUTPUT')
    if gha_out:
        with open(gha_out, 'a', encoding='utf-8') as f:
            f.write(f"generated_filename={filename}\n")
            f.write(f"commit_message=Add GPIO Test Plan Excel autogenerated on {ist_human}\n")

    # Also print summary
    print(f"Wrote: {out_path}")

if __name__ == '__main__':
    main()
