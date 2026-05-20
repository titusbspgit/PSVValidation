import json
import os
from datetime import datetime
from openpyxl import Workbook
from openpyxl.styles import Font
from pytz import timezone

# Configuration
JSON_PATH = os.environ.get("TESTPLAN_JSON_PATH", "Test_Output/GPIO/TestPlan/final_testplan.json")
OUTPUT_DIR = os.environ.get("TESTPLAN_OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
TESTPLAN_SHEET = "TestPlan"
METADATA_SHEET = "MetaData"

TESTPLAN_COLS = [
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

METADATA_COLS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

def validate_and_load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"json_data element at index {i} is not an object")
    return data

def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = TESTPLAN_SHEET

    # Create MetaData sheet second
    meta = wb.create_sheet(METADATA_SHEET)

    # Header fonts
    bold = Font(bold=True)

    # Write headers for TestPlan
    for col_idx, key in enumerate(TESTPLAN_COLS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = bold
    ws.freeze_panes = "A2"

    # Write headers for MetaData
    for col_idx, key in enumerate(METADATA_COLS, start=1):
        cell = meta.cell(row=1, column=col_idx, value=key)
        cell.font = bold
    meta.freeze_panes = "A2"

    # Fill rows, preserving order
    for r_idx, obj in enumerate(rows, start=2):
        # TestPlan sheet data
        for c_idx, key in enumerate(TESTPLAN_COLS, start=1):
            ws.cell(row=r_idx, column=c_idx, value=obj.get(key, ""))
        # MetaData sheet data
        for c_idx, key in enumerate(METADATA_COLS, start=1):
            meta.cell(row=r_idx, column=c_idx, value=obj.get(key, ""))

    # Very hide MetaData
    meta.sheet_state = "veryHidden"

    return wb

def ensure_dir(path):
    if not os.path.isdir(path):
        os.makedirs(path, exist_ok=True)


def main():
    rows = validate_and_load_json(JSON_PATH)
    wb = build_workbook(rows)

    # Build IST timestamped filename
    ist = timezone("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    fname = f"testplan_{ts}.xlsx"

    ensure_dir(OUTPUT_DIR)
    out_path = os.path.join(OUTPUT_DIR, fname)

    # Save real .xlsx
    wb.save(out_path)

    # Print output path for logs
    print(f"Generated Excel: {out_path}")

if __name__ == "__main__":
    main()
