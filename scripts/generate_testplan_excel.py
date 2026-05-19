import json
import os
from datetime import datetime

try:
    from zoneinfo import ZoneInfo
    tz = ZoneInfo("Asia/Kolkata")
except Exception:
    tz = None

from openpyxl import Workbook
from openpyxl.styles import Font

INPUT_JSON = os.path.join('scripts', 'testplan_input.json')
OUTPUT_DIR = os.path.join('Test_Output', 'PCIE', 'TestPlan')
OUTPUT_MARKER = os.path.join('scripts', 'testplan_output_path.txt')

TESTPLAN_COLS = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]

METADATA_COLS = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

def ist_timestamp():
    if tz is not None:
        now = datetime.now(tz)
    else:
        # Fallback to localtime if zoneinfo not available
        now = datetime.now()
    return now.strftime('%Y%m%d_%H%M%S')

def main():
    with open(INPUT_JSON, 'r', encoding='utf-8') as f:
        data = json.load(f)
    assert isinstance(data, list) and len(data) > 0, 'json_data must be a non-empty array'

    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Headers
    ws_plan.append(TESTPLAN_COLS)
    ws_meta.append(METADATA_COLS)

    # Bold headers
    for cell in ws_plan[1]:
        cell.font = Font(bold=True)
    for cell in ws_meta[1]:
        cell.font = Font(bold=True)

    # Freeze first row
    ws_plan.freeze_panes = 'A2'
    ws_meta.freeze_panes = 'A2'

    # Rows
    for item in data:
        # Ensure item is dict
        if not isinstance(item, dict):
            raise ValueError('Each element in json_data must be an object (dict)')
        plan_row = [item.get(k, '') for k in TESTPLAN_COLS]
        meta_row = [item.get(k, '') for k in METADATA_COLS]
        ws_plan.append(plan_row)
        ws_meta.append(meta_row)

    # Set MetaData to VeryHidden
    ws_meta.sheet_state = 'veryHidden'

    # Ensure output dir
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    ts = ist_timestamp()
    base_name = f'testplan_{ts}.xlsx'
    out_path = os.path.join(OUTPUT_DIR, base_name)
    # Guarantee uniqueness if needed
    i = 2
    while os.path.exists(out_path):
        out_path = os.path.join(OUTPUT_DIR, f'testplan_{ts}_{i}.xlsx')
        i += 1

    # Save workbook
    wb.save(out_path)

    # Persist output path for the workflow commit step
    with open(OUTPUT_MARKER, 'w', encoding='utf-8') as mf:
        mf.write(out_path)

    print(f'Generated: {out_path}')

if __name__ == '__main__':
    main()
