#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

def validate_input(data):
    if not isinstance(data, list):
        raise ValueError("Input JSON must be a list (array) of objects.")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Element at index {i} is not an object (dict).")
    return True


def write_sheet(ws, columns, rows):
    # Header
    bold_font = Font(bold=True)
    for c_idx, col_name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=c_idx, value=col_name)
        cell.font = bold_font
    # Freeze first row
    ws.freeze_panes = "A2"

    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, col_name in enumerate(columns, start=1):
            val = row.get(col_name, "")
            ws.cell(row=r_idx, column=c_idx, value=val)


def generate_excel(input_path: str, output_dir: str) -> str:
    with open(input_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Step 1: Validate
    validate_input(data)

    # Prepare workbook
    wb = Workbook()
    # Remove default sheet
    default = wb.active
    wb.remove(default)

    ws_testplan = wb.create_sheet("TestPlan")
    ws_metadata = wb.create_sheet("MetaData")

    # Very hidden MetaData
    ws_metadata.sheet_state = 'veryHidden'

    # Step 2: Split data (write columns; missing fields blank; preserve order)
    write_sheet(ws_testplan, TESTPLAN_COLUMNS, data)
    write_sheet(ws_metadata, METADATA_COLUMNS, data)

    # Step 4: Save file with IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    return out_path


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON.")
    parser.add_argument('--input', required=True, help='Path to input JSON file')
    parser.add_argument('--output-dir', required=True, help='Directory to place generated Excel')
    args = parser.parse_args()

    try:
        out_file = generate_excel(args.input, args.output_dir)
        print(f"OUTPUT_FILE={out_file}")
    except Exception as e:
        print(f"ERROR: {e}")
        raise


if __name__ == '__main__':
    main()
