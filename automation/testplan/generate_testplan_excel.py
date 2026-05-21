import json, os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

JSON_PATH = "automation/testplan/final_testplan.json"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan"

TESTPLAN_HEADERS = [
    "Index","SS / Module","Feature","Test Case Name","Test Description",
    "Speed","Mode","Memory Start Offset","Memory End Offset",
    "Remarks","Test Steps / Procedure","Impacted Registers",
    "Validation / Acceptance Criteria","Code Generation (Required / Not)"
]

METADATA_HEADERS = [
    "Index","Test Case Name","Meta Test Description",
    "Meta Test Steps / Procedure","Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria","Meta Headers",
    "Meta Macros","Meta Arrays"
]

def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise SystemExit("json_data must be a list of objects")
    return data


def build_workbook(data):
    wb = Workbook()

    # Sheet 1: TestPlan
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws1.append(TESTPLAN_HEADERS)
    for cell in ws1[1]:
        cell.font = Font(bold=True)
    ws1.freeze_panes = "A2"

    for rec in data:
        row = [
            str(rec.get("Index", "")),
            rec.get("SS / Module", ""),
            rec.get("Feature", ""),
            rec.get("Test Case Name", ""),
            rec.get("Test Description", ""),
            rec.get("Speed", ""),
            rec.get("Mode", ""),
            rec.get("Memory Start Offset", ""),
            rec.get("Memory End Offset", ""),
            rec.get("Remarks", ""),
            rec.get("Test Steps / Procedure", ""),
            rec.get("Impacted Registers", ""),
            rec.get("Validation / Acceptance Criteria", ""),
            rec.get("Code Generation (Required / Not)", ""),
        ]
        ws1.append(row)

    # Sheet 2: MetaData (VERY HIDDEN)
    ws2 = wb.create_sheet("MetaData")
    ws2.append(METADATA_HEADERS)
    for cell in ws2[1]:
        cell.font = Font(bold=True)
    ws2.freeze_panes = "A2"

    for rec in data:
        row = [
            str(rec.get("Index", "")),
            rec.get("Test Case Name", ""),
            rec.get("Meta Test Description", ""),
            rec.get("Meta Test Steps / Procedure", ""),
            rec.get("Meta Impacted Registers", ""),
            rec.get("Meta Validation / Acceptance Criteria", ""),
            rec.get("Meta Headers", ""),
            rec.get("Meta Macros", ""),
            rec.get("Meta Arrays", ""),
        ]
        ws2.append(row)

    # Very hide MetaData sheet
    ws2.sheet_state = "veryHidden"

    return wb


def main():
    data = load_json(JSON_PATH)
    wb = build_workbook(data)

    # IST timestamp: YYYYMMDD_HHMMSS
    ts_ist = datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts_ist}.xlsx"

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, filename)
    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
