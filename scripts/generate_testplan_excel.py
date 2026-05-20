#!/usr/bin/env python3
import json
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from openpyxl import Workbook
from openpyxl.styles import Font

# Configuration
DATA_PATH = "Test_Output/GPIO/TestPlan/testplan_data.json"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan"
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

def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a list of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each element must be an object; found {type(row)} at index {i}")
    return data


def build_workbook(rows):
    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_md = wb.create_sheet("MetaData")

    # Write headers
    ws_tp.append(TESTPLAN_COLUMNS)
    ws_md.append(METADATA_COLUMNS)

    # Bold headers and freeze first row
    for cell in ws_tp[1]:
        cell.font = Font(bold=True)
    for cell in ws_md[1]:
        cell.font = Font(bold=True)
    ws_tp.freeze_panes = "A2"
    ws_md.freeze_panes = "A2"

    # Append data rows preserving order and leaving blanks if missing
    for row in rows:
        ws_tp.append([row.get(col, "") for col in TESTPLAN_COLUMNS])
        ws_md.append([row.get(col, "") for col in METADATA_COLUMNS])

    # Make MetaData very hidden
    ws_md.sheet_state = "veryHidden"
    return wb


def ist_timestamp():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        return datetime.now(tz).strftime("%Y%m%d_%H%M%S")
    # Fallback to naive localtime if ZoneInfo unavailable (runner should have it)
    return datetime.now().strftime("%Y%m%d_%H%M%S")


def main():
    rows = load_json(DATA_PATH)
    wb = build_workbook(rows)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    ts = ist_timestamp()
    out_path = os.path.join(OUTPUT_DIR, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    print(f"Saved: {out_path}")


if __name__ == "__main__":
    main()
