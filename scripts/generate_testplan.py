#!/usr/bin/env python3
import json
import os
from datetime import datetime, timezone, timedelta
from pathlib import Path
from typing import List, Dict

from openpyxl import Workbook
from openpyxl.styles import PatternFill, Font, Alignment

# Configuration (can be overridden by environment variables)
INPUT_JSON_PATH = os.getenv('INPUT_JSON_PATH', '.github/testplan_input.json')
OUTPUT_DIR = os.getenv('OUTPUT_DIR', 'Test_Output/USB/TestPlan/')
IP_NAME = os.getenv('IP_NAME', 'USB')

# Columns specification
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

def validate_json(data: List[Dict]):
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError('json_data must be a non-empty array')
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f'Each item must be an object (row {i})')


def ist_now_str():
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    return now_ist.strftime('%Y%m%d_%H%M%S')


def apply_formatting(ws, header_row_idx=1, is_metadata=False):
    # Styles
    header_fill = PatternFill(fill_type='solid', fgColor='4472C4')  # Blue
    header_font = Font(bold=True, color='FFFFFFFF')  # White, bold
    wrap = Alignment(wrap_text=True, vertical='top')

    # Header styling
    for cell in ws[header_row_idx]:
        cell.fill = header_fill
        cell.font = header_font
        cell.alignment = wrap

    # Wrap for all cells
    for row in ws.iter_rows(min_row=header_row_idx+1):
        for cell in row:
            cell.alignment = wrap

    # Freeze first row
    ws.freeze_panes = 'A2'

    # Column widths (reasonable defaults)
    if not is_metadata:
        widths = [8, 14, 28, 26, 70, 10, 10, 18, 18, 10, 70, 40, 60, 24]
    else:
        widths = [8, 26, 70, 80, 50, 60, 40, 16, 26]

    for i, w in enumerate(widths, start=1):
        col_letter = ws.cell(row=1, column=i).column_letter
        ws.column_dimensions[col_letter].width = w


def build_workbook(rows: List[Dict]) -> Workbook:
    wb = Workbook()

    # Create sheets
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Write headers
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)

    # Write data preserving order
    for row in rows:
        plan_values = [row.get(col, '') for col in TESTPLAN_COLUMNS]
        meta_values = [row.get(col, '') for col in METADATA_COLUMNS]
        ws_plan.append(plan_values)
        ws_meta.append(meta_values)

    # Apply formatting
    apply_formatting(ws_plan, header_row_idx=1, is_metadata=False)
    apply_formatting(ws_meta, header_row_idx=1, is_metadata=True)

    # Set MetaData very hidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    # Load JSON
    with open(INPUT_JSON_PATH, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Validate
    validate_json(data)

    # Build workbook
    wb = build_workbook(data)

    # Prepare output path
    ts = ist_now_str()
    filename = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_dir = Path(OUTPUT_DIR)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / filename

    # Save real .xlsx
    wb.save(out_path)

    print(str(out_path))


if __name__ == '__main__':
    main()
