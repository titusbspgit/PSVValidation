import json
from datetime import datetime, timedelta, timezone
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font

# Constants
DATA_PATH = Path('Test_Output/GPIO/TestPlan/testplan_data.json')
OUT_DIR = Path('Test_Output/GPIO/TestPlan')

# Columns for sheets (order fixed by requirements)
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

def load_json(path: Path):
    with path.open('r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError('Input JSON must be an array of objects')
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f'Row {i} is not an object')
    return data


def write_sheet(ws, cols, rows):
    # Write header
    for c, name in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=c, value=name)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Write data
    for r, row_obj in enumerate(rows, start=2):
        for c, name in enumerate(cols, start=1):
            ws.cell(row=r, column=c, value=row_obj.get(name, ''))


def build_rows(data):
    # Map json objects to two sets of rows matching the columns
    testplan_rows = []
    metadata_rows = []

    for obj in data:
        # TestPlan row
        trow = {k: obj.get(k, '') for k in TESTPLAN_COLS}
        testplan_rows.append(trow)

        # MetaData row
        mrow = {k: '' for k in METADATA_COLS}
        mrow['Index'] = obj.get('Index', '')
        mrow['Test Case Name'] = obj.get('Test Case Name', '')
        mrow['Meta Test Description'] = obj.get('Meta Test Description', '')
        mrow['Meta Test Steps / Procedure'] = obj.get('Meta Test Steps / Procedure', '')
        mrow['Meta Impacted Registers'] = obj.get('Meta Impacted Registers', '')
        mrow['Meta Validation / Acceptance Criteria'] = obj.get('Meta Validation / Acceptance Criteria', '')
        mrow['Meta Headers'] = obj.get('Meta Headers', '')
        mrow['Meta Macros'] = obj.get('Meta Macros', '')
        mrow['Meta Arrays'] = obj.get('Meta Arrays', '')
        metadata_rows.append(mrow)

    return testplan_rows, metadata_rows


def ist_timestamp():
    # IST = UTC+5:30
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    data = load_json(DATA_PATH)

    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'
    ws_md = wb.create_sheet('MetaData')

    tp_rows, md_rows = build_rows(data)
    write_sheet(ws_tp, TESTPLAN_COLS, tp_rows)
    write_sheet(ws_md, METADATA_COLS, md_rows)

    # Set MetaData sheet to VeryHidden
    ws_md.sheet_state = 'veryHidden'

    # Save with IST timestamp
    fname = f'testplan_{ist_timestamp()}.xlsx'
    out_path = OUT_DIR / fname
    wb.save(out_path.as_posix())
    print(f'Wrote Excel to {out_path}')

if __name__ == '__main__':
    main()
