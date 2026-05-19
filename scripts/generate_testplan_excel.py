import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Inputs
INPUT_JSON = os.environ.get("TESTPLAN_JSON", "scripts/testplan_input.json")
OUTPUT_DIR = os.environ.get("TESTPLAN_OUTPUT_DIR", "Test_Output/PCIE/TestPlan")

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

def load_json(path: str):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("json_data must be a non-empty array")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise ValueError(f"Each element must be an object, found {type(item)} at index {i}")
    return data


def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def bold_headers(ws):
    for cell in ws[1]:
        cell.font = Font(bold=True)


def write_sheet(ws, columns, rows):
    ws.append(columns)
    for obj in rows:
        row = [obj.get(col, "") for col in columns]
        ws.append(row)


def unique_filepath(base_dir: str, base_name: str) -> str:
    name, ext = os.path.splitext(base_name)
    candidate = os.path.join(base_dir, base_name)
    idx = 1
    while os.path.exists(candidate):
        candidate = os.path.join(base_dir, f"{name}_{idx:03d}{ext}")
        idx += 1
    return candidate


def main():
    data = load_json(INPUT_JSON)

    wb = Workbook()
    # Remove default sheet
    wb.remove(wb.active)

    ws_plan = wb.create_sheet("TestPlan")
    ws_meta = wb.create_sheet("MetaData")

    write_sheet(ws_plan, TESTPLAN_COLUMNS, data)
    write_sheet(ws_meta, METADATA_COLUMNS, data)

    # Formatting
    bold_headers(ws_plan)
    bold_headers(ws_meta)
    ws_plan.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Very hidden MetaData
    ws_meta.sheet_state = "veryHidden"

    # Timestamp in IST
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")

    ensure_dir(OUTPUT_DIR)
    filename = f"testplan_{ts}.xlsx"
    out_path = unique_filepath(OUTPUT_DIR, filename)

    wb.save(out_path)

    # Write output path for workflow commit step
    with open("scripts/testplan_output_path.txt", "w", encoding="utf-8") as f:
        f.write(out_path)

    print(f"Generated Excel at: {out_path}")


if __name__ == "__main__":
    main()
