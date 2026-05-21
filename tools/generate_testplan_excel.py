import json
import os
from datetime import datetime, timedelta, timezone
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

INPUT_JSON = os.path.join("tools", "final_testplan.json")
OUTPUT_DIR = os.path.join("Test_Output", "PCIE", "TestPlan")


def read_json_array(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("Input JSON must be an array of objects")
    return data


def build_workbook(rows):
    wb = Workbook()

    # TestPlan sheet
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws1.append(TESTPLAN_COLUMNS)
    for cell in ws1[1]:
        cell.font = Font(bold=True)
    ws1.freeze_panes = "A2"

    for r in rows:
        row_vals = [r.get(col, "") for col in TESTPLAN_COLUMNS]
        ws1.append(row_vals)

    # MetaData sheet (very hidden)
    ws2 = wb.create_sheet("MetaData")
    ws2.append(METADATA_COLUMNS)
    for cell in ws2[1]:
        cell.font = Font(bold=True)
    ws2.freeze_panes = "A2"

    for r in rows:
        row_vals = [r.get(col, "") for col in METADATA_COLUMNS]
        ws2.append(row_vals)

    # VERY HIDDEN MetaData
    ws2.sheet_state = "veryHidden"

    return wb


def ensure_dir(p):
    os.makedirs(p, exist_ok=True)


def ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime("%Y%m%d_%H%M%S")


def main():
    rows = read_json_array(INPUT_JSON)
    wb = build_wb_with_validation(rows=rows)


def build_wb_with_validation(rows):
    # Validate row objects
    for i, r in enumerate(rows, start=1):
        if not isinstance(r, dict):
            raise ValueError(f"Row {i} is not an object")
    wb = build_workbook(rows)
    ensure_dir(OUTPUT_DIR)
    out_path = os.path.join(OUTPUT_DIR, f"testplan_{ist_timestamp()}.xlsx")
    wb.save(out_path)
    print(f"Saved: {out_path}")
    return wb


if __name__ == "__main__":
    main()
