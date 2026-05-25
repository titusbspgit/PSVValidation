#!/usr/bin/env python3
import json
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Constants
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

DATA_PATH = Path("Test_Output/GPIO/TestPlan/testplan_data.json")
OUTPUT_DIR = Path("Test_Output/GPIO/TestPlan")


def load_json_array(path: Path):
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be an array of objects")
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            raise ValueError(f"Entry {i} is not an object")
    return data


def write_sheet(ws, columns, rows):
    # Header
    ws.append(columns)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Rows
    for obj in rows:
        ws.append([obj.get(col, "") for col in columns])


def build_workbook(data):
    wb = Workbook()
    ws_testplan = wb.active
    ws_testplan.title = "TestPlan"
    write_sheet(ws_testplan, TESTPLAN_COLUMNS, data)

    ws_meta = wb.create_sheet("MetaData")
    write_sheet(ws_meta, METADATA_COLUMNS, data)

    # Make MetaData VeryHidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def main():
    if not DATA_PATH.exists():
        raise FileNotFoundError(f"Missing input JSON: {DATA_PATH}")

    data = load_json_array(DATA_PATH)

    wb = build_workbook(data)

    # Timestamp in IST
    ist_now = datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y%m%d_%H%M%S")
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_file = OUTPUT_DIR / f"testplan_{ist_now}.xlsx"

    wb.save(out_file)
    print(f"Generated: {out_file}")


if __name__ == "__main__":
    main()
