#!/usr/bin/env python3
import json
import os
from pathlib import Path
from datetime import datetime, timedelta, timezone
from openpyxl import Workbook
from openpyxl.styles import Font

# Constants
OUTPUT_DIR = Path('Test_Output/GPIO/TestPlan')
JSON_PATH = OUTPUT_DIR / 'testplan_data.json'
TESTPLAN_SHEET = 'TestPlan'
METADATA_SHEET = 'MetaData'

TESTPLAN_HEADERS = [
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

METADATA_HEADERS = [
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

def load_json(path: Path):
    with path.open('r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError('json_data must be an array (list)')
    return data


def write_headers(ws, headers):
    bold = Font(bold=True)
    for col_idx, header in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = bold
    ws.freeze_panes = 'A2'


def append_rows(ws, headers, rows):
    for item in rows:
        values = [item.get(h, "") for h in headers]
        ws.append(values)


def build_workbook(data):
    wb = Workbook()
    # Remove default sheet
    default = wb.active
    wb.remove(default)

    ws_plan = wb.create_sheet(TESTPLAN_SHEET)
    ws_meta = wb.create_sheet(METADATA_SHEET)

    write_headers(ws_plan, TESTPLAN_HEADERS)
    write_headers(ws_meta, METADATA_HEADERS)

    # Map data for both sheets while preserving order
    plan_rows = []
    meta_rows = []
    for obj in data:
        plan_row = {h: obj.get(h, "") for h in TESTPLAN_HEADERS}
        meta_row = {h: obj.get(h, "") for h in METADATA_HEADERS}
        plan_rows.append(plan_row)
        meta_rows.append(meta_row)

    append_rows(ws_plan, TESTPLAN_HEADERS, plan_rows)
    append_rows(ws_meta, METADATA_HEADERS, meta_rows)

    # VeryHidden for metadata
    ws_meta.sheet_state = 'veryHidden'

    return wb


def ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')


def main():
    if not JSON_PATH.exists():
        raise FileNotFoundError(f'Missing JSON source at {JSON_PATH}')
    data = load_json(JSON_PATH)
    wb = build_workbook(data)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    filename = f'testplan_{ist_timestamp()}.xlsx'
    out_path = OUTPUT_DIR / filename
    wb.save(out_path)
    print(f'Wrote Excel to {out_path}')

if __name__ == '__main__':
    main()
