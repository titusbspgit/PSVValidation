#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime, timedelta, timezone

from openpyxl import Workbook
from openpyxl.styles import Font


def ist_now_str():
    # IST = UTC+5:30, no DST
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')


def build_workbook(data):
    # Columns as specified
    testplan_cols = [
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

    metadata_cols = [
        'Index',
        'Test Case Name',
        'Meta Test Description',
        'Meta Test Steps / Procedure',
        'Meta Impacted Registers',
        'Meta Validation / Acceptance Criteria',
        'Meta Headers',
        'Meta Macros',
        'Meta Arrays'
    ]

    wb = Workbook()
    ws = wb.active
    ws.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Write headers
    header_font = Font(bold=True)
    ws.append(testplan_cols)
    for cell in ws[1]:
        cell.font = header_font
    ws.freeze_panes = 'A2'

    ws_meta.append(metadata_cols)
    for cell in ws_meta[1]:
        cell.font = header_font
    ws_meta.freeze_panes = 'A2'

    # Write rows in order, preserving exact data
    def v(item, key):
        return item.get(key, '') if isinstance(item, dict) else ''

    for item in data:
        ws.append([v(item, c) for c in testplan_cols])
        ws_meta.append([v(item, c) for c in metadata_cols])

    # Make MetaData very hidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    parser = argparse.ArgumentParser(description='Generate Test Plan Excel from JSON')
    parser.add_argument('--json', required=True, help='Path to input JSON array file')
    parser.add_argument('--outdir', required=True, help='Output directory in repo')
    args = parser.parse_args()

    # Load JSON
    with open(args.json, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise SystemExit('Input JSON must be an array of objects')

    wb = build_workbook(data)

    ts = ist_now_str()
    filename = f'testplan_{ts}.xlsx'

    outdir = args.outdir
    os.makedirs(outdir, exist_ok=True)
    out_path = os.path.join(outdir, filename)

    wb.save(out_path)
    print(f'Wrote: {out_path}')


if __name__ == '__main__':
    main()
