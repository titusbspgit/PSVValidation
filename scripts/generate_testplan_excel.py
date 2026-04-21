import json
import os
from datetime import datetime, timedelta, timezone
from pathlib import Path
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, Border, Side
from openpyxl.utils import get_column_letter

# Configuration
INPUT_JSON_PATH = os.environ.get('TP_INPUT_JSON', 'Test_Output/GPIO/TestPlan/input/TestPlan_GPIO.json')
OUTPUT_DIR = Path('Test_Output/GPIO/TestPlan')
IP_NAME = 'GPIO'

MAIN_COLUMNS = [
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

META_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]

WRAP_COLUMNS = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria'
}


def _join_val(v: Any) -> Any:
    if isinstance(v, list):
        return '\n'.join(str(x) if x is not None else '' for x in v)
    return v


def load_json(path: str) -> Dict[str, Any]:
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def build_schema_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.append(k)
    return seen


def create_workbook(rows: List[Dict[str, Any]]) -> Workbook:
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Schema order from first appearances
    schema = build_schema_order(rows)

    # Write header
    for c, key in enumerate(schema, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')

    # Write data rows
    for r, row in enumerate(rows, start=2):
        for c, key in enumerate(schema, start=1):
            val = _join_val(row.get(key, ''))
            ws.cell(row=r, column=c, value=val)

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Best-effort autofit columns
    for c, key in enumerate(schema, start=1):
        max_len = len(str(key))
        for r in range(2, len(rows) + 2):
            v = ws.cell(row=r, column=c).value
            if v is None:
                l = 0
            else:
                v_str = str(v)
                # consider wrapped lines
                l = max(len(line) for line in v_str.split('\n')) if '\n' in v_str else len(v_str)
            if l > max_len:
                max_len = l
        ws.column_dimensions[get_column_letter(c)].width = min(120, max(10, max_len + 2))

    return wb


def add_meta_sheet_and_hide(wb: Workbook, rows: List[Dict[str, Any]]):
    ws_meta = wb.create_sheet('Meta_data_sheet')
    # Header
    for c, key in enumerate(META_COLUMNS, start=1):
        ws_meta.cell(row=1, column=c, value=key)
    # Rows
    for r, row in enumerate(rows, start=2):
        for c, key in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=r, column=c, value=_join_val(row.get(key, '')))
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'


def prepare_testplan_sheet(wb: Workbook):
    ws = wb['Data']
    ws.title = 'TestPlan'

    # Build header map
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    header_to_idx = {h: i+1 for i, h in enumerate(headers)}

    # Remove META columns from TestPlan
    keep_headers = [h for h in headers if h not in META_COLUMNS]

    # Reorder to MAIN_COLUMNS first (in given order), then any remaining columns in their original order
    ordered = [h for h in MAIN_COLUMNS if h in keep_headers]
    ordered += [h for h in keep_headers if h not in ordered]

    # Create a new temporary sheet for reordered content
    ws_new = wb.create_sheet('TestPlan_tmp')

    # Header row
    for c, h in enumerate(ordered, start=1):
        cell = ws_new.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')

    # Data rows
    for r in range(2, ws.max_row + 1):
        for c, h in enumerate(ordered, start=1):
            src_col = header_to_idx.get(h)
            val = ws.cell(row=r, column=src_col).value if src_col else ''
            ws_new.cell(row=r, column=c, value=val)

    # Delete old TestPlan (Data) and rename tmp
    wb.remove(ws)
    ws_new.title = 'TestPlan'

    # Formatting rules
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Wrap and align
    headers_now = [ws_new.cell(row=1, column=c).value for c in range(1, ws_new.max_column + 1)]

    wrap_cols_idx = set()
    for c, h in enumerate(headers_now, start=1):
        if h in WRAP_COLUMNS:
            wrap_cols_idx.add(c)

    # Header formatting
    for c in range(1, ws_new.max_column + 1):
        cell = ws_new.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        cell.border = border

    # Data rows formatting
    for r in range(2, ws_new.max_row + 1):
        for c in range(1, ws_new.max_column + 1):
            cell = ws_new.cell(row=r, column=c)
            # Wrap where required
            if c in wrap_cols_idx:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            else:
                # Index numeric alignment heuristic
                header = headers_now[c-1]
                if header == 'Index':
                    cell.alignment = Alignment(vertical='top', horizontal='center')
                else:
                    cell.alignment = Alignment(vertical='top', horizontal='left')
            cell.border = border

    # Best-effort autofit columns
    for c in range(1, ws_new.max_column + 1):
        max_len = len(str(ws_new.cell(row=1, column=c).value or ''))
        for r in range(2, ws_new.max_row + 1):
            v = ws_new.cell(row=r, column=c).value
            if v is None:
                l = 0
            else:
                v_str = str(v)
                l = max(len(line) for line in v_str.split('\n')) if '\n' in v_str else len(v_str)
            if l > max_len:
                max_len = l
        ws_new.column_dimensions[get_column_letter(c)].width = min(120, max(10, max_len + 2))

    # Approximate row height with wrap
    base_height = 15
    for r in range(2, ws_new.max_row + 1):
        lines = 1
        for c in wrap_cols_idx:
            v = ws_new.cell(row=r, column=c).value
            if v:
                lines = max(lines, str(v).count('\n') + 1)
        ws_new.row_dimensions[r].height = base_height * lines


def compute_ist_filename(ip_name: str) -> str:
    now_ist = datetime.now(timezone.utc) + timedelta(hours=5, minutes=30)
    return f"{ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"


def main():
    data = load_json(INPUT_JSON_PATH)
    if not isinstance(data, dict) or 'testcases' not in data or not isinstance(data['testcases'], list) or len(data['testcases']) == 0:
        raise SystemExit('Invalid JSON: expected object with non-empty "testcases" array')

    rows = data['testcases']
    # Normalize values
    norm_rows = []
    for row in rows:
        norm = {}
        for k, v in row.items():
            norm[k] = _join_val(v)
        norm_rows.append(norm)

    wb = create_workbook(norm_rows)
    add_meta_sheet_and_hide(wb, norm_rows)
    prepare_testplan_sheet(wb)

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    filename = compute_ist_filename(IP_NAME)
    out_path = OUTPUT_DIR / filename
    wb.save(out_path)
    print(f"Saved: {out_path}")


if __name__ == '__main__':
    main()
