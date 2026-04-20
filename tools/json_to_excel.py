#!/usr/bin/env python3
import sys, os, json, datetime, re
from collections import OrderedDict
from pathlib import Path

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
    from openpyxl.utils import get_column_letter
except Exception as e:
    print(f"ERROR: openpyxl not available: {e}")
    sys.exit(2)

SEPARATOR = ' | '

def ist_now():
    # IST is UTC+05:30
    # Use timezone from environment if set by runner; otherwise compute offset manually
    try:
        # Prefer Python 3.9+ zoneinfo if available
        from zoneinfo import ZoneInfo
        tz = ZoneInfo('Asia/Kolkata')
        now = datetime.datetime.now(tz)
    except Exception:
        now = datetime.datetime.utcnow() + datetime.timedelta(hours=5, minutes=30)
    return now

def normalize_rows(obj):
    # If has 'testcases' array, build rows per testcase; else one row from whole object
    rows = []
    columns = []  # in first-seen order

    def add_col(col):
        if col not in columns:
            columns.append(col)

    def stringify_value(v):
        if isinstance(v, list):
            return SEPARATOR.join([stringify_value(x) for x in v])
        elif isinstance(v, dict):
            # Preserve order; no extra spaces
            return json.dumps(v, ensure_ascii=False, separators=(',', ':' ))
        elif v is None:
            return ''
        else:
            return str(v)

    # Build metadata columns if present
    meta = obj.get('metadata') if isinstance(obj, dict) else None
    features = obj.get('features') if isinstance(obj, dict) else None

    meta_cols = []
    meta_map = OrderedDict()
    if isinstance(meta, dict):
        for k in ['project','ip_name','repo','branch','subdirectory','generation_timestamp_ist','source_paths']:
            if k in meta:
                meta_cols.append(f'metadata.{k}')
                if k == 'source_paths' and isinstance(meta[k], list):
                    meta_map[f'metadata.{k}'] = SEPARATOR.join([str(x) for x in meta[k]])
                else:
                    meta_map[f'metadata.{k}'] = stringify_value(meta[k])
    if features is not None:
        meta_cols.append('features')
        meta_map['features'] = stringify_value(features)

    testcases = None
    if isinstance(obj, dict):
        tc = obj.get('testcases')
        if isinstance(tc, list):
            testcases = tc

    if testcases is None:
        # Single row from object
        row = OrderedDict()
        for c in meta_cols:
            add_col(c)
            row[c] = meta_map.get(c, '')
        # Flatten other top-level keys
        for k, v in obj.items() if isinstance(obj, dict) else []:
            if k in ('metadata','features'): 
                continue
            add_col(k)
            row[k] = stringify_value(v)
        rows.append(row)
        return columns, rows

    # Build union of testcase keys in first-seen order
    tc_keys = []
    seen = set()
    for t in testcases:
        if isinstance(t, dict):
            for k in t.keys():
                if k not in seen:
                    seen.add(k)
                    tc_keys.append(k)

    # Final columns: metadata.* then tc_keys
    for c in meta_cols:
        add_col(c)
    for k in tc_keys:
        add_col(k)

    # Build rows
    for t in testcases:
        row = OrderedDict()
        for c in meta_cols:
            row[c] = meta_map.get(c, '')
        if isinstance(t, dict):
            for k in tc_keys:
                v = t.get(k, '')
                row[k] = stringify_value(v)
        else:
            for k in tc_keys:
                row[k] = ''
        rows.append(row)

    return columns, rows

def write_excel(columns, rows, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'
    # Header
    ws.append(columns)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    # Rows
    for r in rows:
        ws.append([r.get(c, '') for c in columns])
    # Freeze top row
    ws.freeze_panes = 'A2'
    # Auto-fit like: set width based on max length up to a cap
    col_widths = [len(c) for c in columns]
    for row in rows:
        for idx, c in enumerate(columns):
            val = row.get(c, '')
            ln = len(val) if isinstance(val, str) else len(str(val))
            if ln > col_widths[idx]:
                col_widths[idx] = ln
    for idx, w in enumerate(col_widths, start=1):
        capped = min(max(10, w + 2), 80)
        col_letter = get_column_letter(idx)
        ws.column_dimensions[col_letter].width = capped
    wb.save(out_path)


def process_json_file(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        content = f.read().strip()
    if not content:
        raise ValueError('Empty JSON input')
    try:
        obj = json.loads(content)
    except json.JSONDecodeError as e:
        raise ValueError(f'Invalid JSON: {e}')

    columns, rows = normalize_rows(obj)
    if len(rows) == 0 or len(columns) == 0:
        raise ValueError('Unsupported or empty JSON structure after normalization')

    # Determine output filename using IST
    now = ist_now()
    ts = now.strftime('%Y%m%d_%H%M%S')
    # IP name from metadata if present
    ip_name = 'GPIO'
    try:
        m = obj.get('metadata', {})
        ip = m.get('ip_name')
        if isinstance(ip, str) and ip.strip():
            ip_name = ip.strip().upper()
    except Exception:
        pass

    out_dir = os.path.dirname(json_path)
    base_name = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(out_dir, base_name)
    # Avoid overwrite: if exists, add numeric suffix
    if os.path.exists(out_path):
        i = 1
        while True:
            candidate = os.path.join(out_dir, f"{ip_name}_TestPlan_{ts}_{i}.xlsx")
            if not os.path.exists(candidate):
                out_path = candidate
                break
            i += 1

    write_excel(columns, rows, out_path)
    return out_path, len(rows), len(columns)


def main():
    if len(sys.argv) < 2:
        print('Usage: json_to_excel.py <json-file-or-directory>')
        sys.exit(2)
    target = Path(sys.argv[1])
    outputs = []
    if target.is_dir():
        json_files = sorted([p for p in target.iterdir() if p.suffix.lower() == '.json'])
        if not json_files:
            print('No JSON files found in directory; nothing to do.')
            sys.exit(0)
        for p in json_files:
            out_path, nrows, ncols = process_json_file(str(p))
            print(f"Generated: {out_path} (rows={nrows}, cols={ncols})")
            outputs.append(out_path)
    else:
        out_path, nrows, ncols = process_json_file(str(target))
        print(f"Generated: {out_path} (rows={nrows}, cols={ncols})")
        outputs.append(out_path)

if __name__ == '__main__':
    main()
