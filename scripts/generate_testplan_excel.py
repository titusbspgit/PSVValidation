#!/usr/bin/env python3
import json
import os
import sys
from argparse import ArgumentParser
from datetime import datetime, timedelta, timezone
from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_HEADERS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

METADATA_HEADERS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("json_data must be a non-empty array")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each item must be an object. Bad item at index {i}")
    return data


def ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    return now_ist.strftime('%Y%m%d_%H%M%S')


def ensure_unique_path(base_dir, base_name):
    path = os.path.join(base_dir, base_name)
    if not os.path.exists(path):
        return path
    root, ext = os.path.splitext(path)
    n = 2
    while True:
        candidate = f"{root}_{n}{ext}"
        if not os.path.exists(candidate):
            return candidate
        n += 1


def write_sheet(ws, headers, rows):
    bold = Font(bold=True)
    ws.append(headers)
    for cell in ws[1]:
        cell.font = bold
    ws.freeze_panes = "A2"
    for row in rows:
        ws.append(row)


def to_rows(data, headers):
    rows = []
    for item in data:
        row = [item.get(h, "") for h in headers]
        rows.append(row)
    return rows


def main():
    p = ArgumentParser()
    p.add_argument('--input', required=True)
    p.add_argument('--outdir', required=True)
    p.add_argument('--ip-name', required=False, default="")
    args = p.parse_args()

    data = load_json(args.input)

    os.makedirs(args.outdir, exist_ok=True)

    # Build workbook
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')
    # Very hidden metadata sheet
    ws_meta.sheet_state = 'veryHidden'

    # Prepare rows preserving order
    testplan_rows = to_rows(data, TESTPLAN_HEADERS)
    metadata_rows = to_rows(data, METADATA_HEADERS)

    write_sheet(ws_plan, TESTPLAN_HEADERS, testplan_rows)
    write_sheet(ws_meta, METADATA_HEADERS, metadata_rows)

    # Filename using IST timestamp
    ts = ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    out_path = ensure_unique_path(args.outdir, filename)

    wb.save(out_path)

    # Record the output path for commit step
    with open('testplan_output_path.txt', 'w', encoding='utf-8') as f:
        f.write(out_path)

    print(f"Wrote Excel: {out_path}")

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
