#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except ImportError:
    # Fallback for very old Pythons (not expected on GH runners)
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_COLS = [
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

METADATA_COLS = [
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
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of objects")
    return data


def ist_now_str():
    if ZoneInfo is not None:
        tz = ZoneInfo('Asia/Kolkata')
        return datetime.now(tz).strftime('%Y%m%d_%H%M%S')
    # Fallback: manual offset +05:30 (approx; DST not applicable in IST)
    from datetime import timedelta, timezone
    tz = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz).strftime('%Y%m%d_%H%M%S')


def write_sheet(ws, headers, rows):
    # Header
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c, value=row.get(h, ""))


def build_workbook(records):
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Prepare rows
    plan_rows = []
    meta_rows = []

    for obj in records:
        # TestPlan
        plan_row = {h: obj.get(h, "") for h in TESTPLAN_COLS}
        plan_rows.append(plan_row)
        # MetaData
        meta_row = {h: obj.get(h, "") for h in METADATA_COLS}
        meta_rows.append(meta_row)

    write_sheet(ws_plan, TESTPLAN_COLS, plan_rows)
    write_sheet(ws_meta, METADATA_COLS, meta_rows)

    # Make MetaData very hidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    ap = argparse.ArgumentParser(description='Generate TestPlan Excel from JSON')
    ap.add_argument('--input', required=True, help='Path to testplan_data.json')
    ap.add_argument('--outdir', required=True, help='Output directory for Excel file')
    args = ap.parse_args()

    records = load_json(args.input)
    wb = build_workbook(records)

    os.makedirs(args.outdir, exist_ok=True)
    ts = ist_now_str()
    fname = f'testplan_{ts}.xlsx'
    out_path = os.path.join(args.outdir, fname)
    wb.save(out_path)
    print(out_path)

if __name__ == '__main__':
    main()
