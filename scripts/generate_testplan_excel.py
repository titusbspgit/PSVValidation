#!/usr/bin/env python3
import json
import os
import sys
import argparse
from collections import OrderedDict
from datetime import datetime, timezone, timedelta

try:
    from zoneinfo import ZoneInfo
    def get_ist_now():
        return datetime.now(ZoneInfo("Asia/Kolkata"))
except Exception:
    # Fallback if zoneinfo isn't available
    def get_ist_now():
        return datetime.utcnow().replace(tzinfo=timezone.utc) + timedelta(hours=5, minutes=30)

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment


def load_aggregated_json(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        return json.load(f, object_pairs_hook=OrderedDict)


essential_meta_keys = [
    ("IP_NAME", None),
    ("repo", None),
    ("branch", None),
    ("output_directory", None),
    ("IST timestamp", None),
    ("computed filename", None),
    ("item_count", None),
]


def build_workbook(data_list, ip_name, repo, branch, output_dir):
    if not isinstance(data_list, list) or not data_list:
        raise ValueError("Aggregated JSON must be a non-empty array")

    # Preserve header order exactly as in the first JSON object
    first = data_list[0]
    headers = list(first.keys())

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Styles
    header_font = Font(bold=True)
    header_fill = PatternFill(fill_type="solid", start_color="FFDCE6F1", end_color="FFDCE6F1")
    wrap = Alignment(wrap_text=True, vertical="top")

    # Write header
    for col_idx, key in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    # Write rows preserving order
    for row_idx, item in enumerate(data_list, start=2):
        for col_idx, key in enumerate(headers, start=1):
            val = item.get(key, "")
            ws.cell(row=row_idx, column=col_idx, value=val).alignment = wrap

    # Freeze first row
    ws.freeze_panes = "A2"

    # Reasonable column widths
    for col_idx, key in enumerate(headers, start=1):
        max_len = len(str(key))
        for row_idx in range(2, len(data_list) + 2):
            v = ws.cell(row=row_idx, column=col_idx).value
            if v is None:
                l = 0
            else:
                l = len(str(v))
            if l > max_len:
                max_len = l
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = min(max(12, max_len + 2), 80)

    # MetaData sheet
    meta = wb.create_sheet(title="MetaData")
    meta.sheet_state = "veryHidden"
    meta_headers = ["Key", "Value"]
    for i, h in enumerate(meta_headers, start=1):
        c = meta.cell(row=1, column=i, value=h)
        c.font = header_font
        c.fill = header_fill
        c.alignment = wrap

    ist_now = get_ist_now()
    ts = ist_now.strftime("%Y%m%d_%H%M%S")
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"

    meta_rows = [
        ("IP_NAME", ip_name),
        ("repo", repo),
        ("branch", branch),
        ("output_directory", output_dir if output_dir.endswith('/') else output_dir + '/'),
        ("IST timestamp", ist_now.strftime("%Y-%m-%d %H:%M:%S %Z")),
        ("computed filename", filename),
        ("item_count", str(len(data_list))),
    ]

    for r_idx, (k, v) in enumerate(meta_rows, start=2):
        meta.cell(row=r_idx, column=1, value=k).alignment = wrap
        meta.cell(row=r_idx, column=2, value=str(v)).alignment = wrap

    return wb, filename


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from aggregated JSON")
    parser.add_argument('--json', required=True, help='Path to aggregated JSON array file')
    parser.add_argument('--ip', required=True, help='IP name (used in filename and metadata)')
    parser.add_argument('--repo', required=True, help='owner/repo string')
    parser.add_argument('--branch', required=True, help='target branch name')
    parser.add_argument('--output-dir', required=True, help='Output directory (relative to repo root)')
    args = parser.parse_args()

    data_list = load_aggregated_json(args.json)
    wb, filename = build_workbook(data_list, args.ip, args.repo, args.branch, args.output_dir)

    out_dir = args.output_dir
    if not os.path.isdir(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    out_path = os.path.join(out_dir, filename)
    wb.save(out_path)

    # Persist the generated path for workflow consumption
    last_path_file = os.path.join(out_dir, 'last_generated.txt')
    with open(last_path_file, 'w', encoding='utf-8') as f:
        f.write(out_path)

    print(f"GENERATED_FILE_PATH={out_path}")


if __name__ == '__main__':
    main()
