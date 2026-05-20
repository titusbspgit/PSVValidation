import json
import os
from pathlib import Path
from datetime import datetime

try:
    # Python 3.9+ with zoneinfo
    from zoneinfo import ZoneInfo
    tz = ZoneInfo('Asia/Kolkata')
except Exception:
    tz = None

from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
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

def now_ist_str():
    dt = datetime.now(tz) if tz else datetime.utcnow()
    if tz is None:
        # Fallback: manually add 5:30 offset to approximate IST if zoneinfo unavailable
        from datetime import timedelta
        dt += timedelta(hours=5, minutes=30)
    return dt.strftime('%Y%m%d_%H%M%S')

def to_cell(value):
    # Preserve exact strings; convert non-str scalars; join lists as JSON-like strings
    if value is None:
        return ''
    if isinstance(value, (int, float)):
        return value
    if isinstance(value, list) or isinstance(value, dict):
        try:
            return json.dumps(value, ensure_ascii=False)
        except Exception:
            return str(value)
    return str(value)

def build_rows(data, columns):
    rows = []
    for obj in data:
        row = [to_cell(obj.get(col, '')) for col in columns]
        rows.append(row)
    return rows

def main():
    output_dir = os.environ.get('OUTPUT_DIR', 'Test_Output/GPIO/TestPlan')
    data_path = os.environ.get('DATA_PATH', 'data/testplan.json')

    # Load JSON
    with open(data_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Validate
    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit('json_data must be a non-empty JSON array')

    # Prepare workbook
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Headers
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)

    bold = Font(bold=True)
    for cell in ws_plan[1]:
        cell.font = bold
    for cell in ws_meta[1]:
        cell.font = bold

    # Freeze first row
    ws_plan.freeze_panes = 'A2'
    ws_meta.freeze_panes = 'A2'

    # Data rows
    plan_rows = build_rows(data, TESTPLAN_COLUMNS)
    meta_rows = build_rows(data, METADATA_COLUMNS)

    for r in plan_rows:
        ws_plan.append(r)
    for r in meta_rows:
        ws_meta.append(r)

    # Very hidden metadata sheet
    ws_meta.sheet_state = 'veryHidden'

    # Autosize (basic)
    for ws in (ws_plan, ws_meta):
        for col in ws.columns:
            max_len = 0
            col_letter = col[0].column_letter
            for cell in col:
                try:
                    v = str(cell.value) if cell.value is not None else ''
                except Exception:
                    v = ''
                max_len = max(max_len, len(v))
            ws.column_dimensions[col_letter].width = min(max(12, max_len + 2), 80)

    # Save
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    ts = now_ist_str()
    filename = f'testplan_{ts}.xlsx'
    out_path = Path(output_dir) / filename
    wb.save(out_path)

    # Optionally record last file name
    with open(Path(output_dir) / 'latest_excel_filename.txt', 'w', encoding='utf-8') as f:
        f.write(filename + '\n')

    print(str(out_path))

if __name__ == '__main__':
    main()
