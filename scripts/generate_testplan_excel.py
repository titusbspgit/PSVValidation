import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Inputs (fixed for this workflow)
OUTPUT_DIR = os.environ.get('TESTPLAN_OUTPUT_DIR', 'Test_Output/PCIE/TestPlan')
INPUT_JSON_PATH = os.environ.get('TESTPLAN_INPUT_JSON', 'scripts/testplan_input.json')
IP_NAME = os.environ.get('TESTPLAN_IP_NAME', 'PCIE')

TESTPLAN_COLS = [
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

METADATA_COLS = [
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

def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError('json_data must be an array')
    for i, obj in enumerate(data):
        if not isinstance(obj, dict):
            raise ValueError(f'Each item must be an object; found {type(obj)} at index {i}')
    return data


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def ist_timestamp():
    ist = ZoneInfo('Asia/Kolkata')
    return datetime.now(tz=ist).strftime('%Y%m%d_%H%M%S')


def unique_filepath(base_dir, base_name):
    name = base_name
    root, ext = os.path.splitext(base_name)
    n = 1
    full = os.path.join(base_dir, name)
    while os.path.exists(full):
        name = f"{root}_{n}{ext}"
        full = os.path.join(base_dir, name)
        n += 1
    return full


def write_sheet(ws, headers, rows):
    # Header
    ws.append(headers)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'
    # Data rows
    for row in rows:
        ws.append(row)


def build_workbook(json_rows):
    wb = Workbook()
    # Default sheet becomes TestPlan
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Prepare rows for each sheet
    plan_rows = []
    meta_rows = []
    for obj in json_rows:
        plan_rows.append([obj.get(col, '') for col in TESTPLAN_COLS])
        meta_rows.append([
            obj.get('Index', ''),
            obj.get('Test Case Name', ''),
            obj.get('Meta Test Description', ''),
            obj.get('Meta Test Steps / Procedure', ''),
            obj.get('Meta Impacted Registers', ''),
            obj.get('Meta Validation / Acceptance Criteria', ''),
            obj.get('Meta Headers', ''),
            obj.get('Meta Macros', ''),
            obj.get('Meta Arrays', ''),
        ])

    write_sheet(ws_plan, TESTPLAN_COLS, plan_rows)
    write_sheet(ws_meta, METADATA_COLS, meta_rows)

    # Set MetaData sheet to very hidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    data = load_json(INPUT_JSON_PATH)
    ensure_dir(OUTPUT_DIR)

    ts = ist_timestamp()
    filename = f'testplan_{ts}.xlsx'
    out_path = unique_filepath(OUTPUT_DIR, filename)

    wb = build_workbook(data)
    wb.save(out_path)

    # Write output path for the commit step
    with open('testplan_output_path.txt', 'w', encoding='utf-8') as f:
        f.write(out_path)

    print(f'Generated: {out_path}')

if __name__ == '__main__':
    main()
