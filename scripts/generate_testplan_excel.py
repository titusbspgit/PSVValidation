import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font


def main():
    # STEP 1 — Validate JSON
    json_path = os.path.join('data', 'testplan.json')
    if not os.path.exists(json_path):
        raise FileNotFoundError(f"Input JSON not found at {json_path}")

    with open(json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if not isinstance(data, list) or not all(isinstance(x, dict) for x in data):
        raise ValueError('json_data must be a non-empty list of objects')

    # STEP 2 — Split Data (define exact schemas)
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

    meta_cols = [
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

    # STEP 3 — Direct Excel Generation using openpyxl
    wb = Workbook()

    # Sheet 1: TestPlan
    ws1 = wb.active
    ws1.title = 'TestPlan'
    ws1.append(testplan_cols)

    def normalize_value(v):
        if isinstance(v, list):
            return '; '.join([str(i) for i in v])
        if isinstance(v, dict):
            return json.dumps(v, ensure_ascii=False)
        return v if v is not None else ''

    for row in data:
        ws1.append([normalize_value(row.get(col, '')) for col in testplan_cols])

    bold = Font(bold=True)
    for cell in ws1[1]:
        cell.font = bold
    ws1.freeze_panes = 'A2'

    # Sheet 2: MetaData (VERY HIDDEN)
    ws2 = wb.create_sheet('MetaData')
    ws2.append(meta_cols)
    for row in data:
        ws2.append([normalize_value(row.get(col, '')) for col in meta_cols])
    for cell in ws2[1]:
        cell.font = bold
    ws2.freeze_panes = 'A2'
    ws2.sheet_state = 'veryHidden'  # VERY HIDDEN

    # STEP 4 — Save File (IST timestamp)
    ist_now = datetime.now(ZoneInfo('Asia/Kolkata'))
    ts = ist_now.strftime('%Y%m%d_%H%M%S')
    out_dir = os.path.join('Test_Output', 'GPIO', 'TestPlan')
    os.makedirs(out_dir, exist_ok=True)
    filename = f'testplan_{ts}.xlsx'
    out_path = os.path.join(out_dir, filename)

    wb.save(out_path)  # Real .xlsx

    # Write helper file with latest filename
    with open(os.path.join(out_dir, 'latest_excel_filename.txt'), 'w', encoding='utf-8') as f:
        f.write(filename + '\n')


if __name__ == '__main__':
    main()
