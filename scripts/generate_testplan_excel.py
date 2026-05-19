import json
import os
from datetime import datetime, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font

# Constants
INPUT_JSON_PATH = os.path.join('scripts', 'testplan_input.json')
OUTPUT_DIR = os.path.join('Test_Output', 'PCIE', 'TestPlan')
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


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or not all(isinstance(x, dict) for x in data):
        raise ValueError('json_data must be a list of objects')
    return data


def build_workbook(rows):
    wb = Workbook()

    # Create sheets
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Write headers
    ws_plan.append(TESTPLAN_HEADERS)
    ws_meta.append(METADATA_HEADERS)

    bold_font = Font(bold=True)
    for cell in ws_plan[1]:
        cell.font = bold_font
    for cell in ws_meta[1]:
        cell.font = bold_font

    ws_plan.freeze_panes = 'A2'
    ws_meta.freeze_panes = 'A2'

    # Write rows preserving order
    for row in rows:
        ws_plan.append([row.get(h, '') for h in TESTPLAN_HEADERS])
        ws_meta.append([
            row.get('Index', ''),
            row.get('Test Case Name', ''),
            row.get('Meta Test Description', ''),
            row.get('Meta Test Steps / Procedure', ''),
            row.get('Meta Impacted Registers', ''),
            row.get('Meta Validation / Acceptance Criteria', ''),
            row.get('Meta Headers', ''),
            row.get('Meta Macros', ''),
            row.get('Meta Arrays', ''),
        ])

    # Set MetaData sheet to VeryHidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def get_ist_timestamp():
    # IST = UTC + 5:30
    ist_dt = datetime.utcnow() + timedelta(hours=5, minutes=30)
    return ist_dt.strftime('%Y%m%d_%H%M%S')


def unique_path(base_dir, base_name):
    path = os.path.join(base_dir, base_name)
    if not os.path.exists(path):
        return path
    stem, ext = os.path.splitext(base_name)
    i = 1
    while True:
        candidate = os.path.join(base_dir, f"{stem}_{i}{ext}")
        if not os.path.exists(candidate):
            return candidate
        i += 1


def main():
    rows = load_json(INPUT_JSON_PATH)
    wb = build_workbook(rows)

    ensure_dir(OUTPUT_DIR)
    ts = get_ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    out_path = unique_path(OUTPUT_DIR, filename)

    wb.save(out_path)

    # Write the path for the workflow to pick up
    with open('testplan_output_path.txt', 'w', encoding='utf-8') as f:
        f.write(out_path)


if __name__ == '__main__':
    main()
