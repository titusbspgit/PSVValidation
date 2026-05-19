import json
import os
import sys
from datetime import datetime, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font

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

REPO_ROOT = os.getcwd()
INPUT_PATH = os.path.join(REPO_ROOT, "scripts", "testplan_input.json")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", os.path.join("Test_Output", "PCIE", "TestPlan"))
OUTPUT_TRACK_FILE = os.path.join(REPO_ROOT, "scripts", ".testplan_output_path")


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each element must be an object; element {i} is {type(row)}")
    return data


def ist_timestamp():
    # IST = UTC + 05:30
    now_utc = datetime.utcnow()
    ist = now_utc + timedelta(hours=5, minutes=30)
    return ist.strftime("%Y%m%d_%H%M%S")


def write_sheet(ws, columns, rows):
    # Header
    for c_idx, header in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=c_idx, value=header)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Rows
    for r_idx, obj in enumerate(rows, start=2):
        for c_idx, key in enumerate(columns, start=1):
            val = obj.get(key, "")
            # Ensure we write plain text for non-None values
            ws.cell(row=r_idx, column=c_idx, value=val if val is not None else "")


def build_workbook(data):
    wb = Workbook()
    # Default sheet
    ws_testplan = wb.active
    ws_testplan.title = "TestPlan"
    ws_metadata = wb.create_sheet(title="MetaData")

    # Prepare row dicts (preserve order of input list)
    test_rows = data  # columns pick will handle missing keys
    meta_rows = data

    write_sheet(ws_testplan, TESTPLAN_COLS, test_rows)
    write_sheet(ws_metadata, METADATA_COLS, meta_rows)

    # VeryHidden MetaData
    ws_metadata.sheet_state = "veryHidden"

    return wb


def ensure_dir(path):
    os.makedirs(path, exist_ok=True)


def unique_output_path(base_dir):
    ts = ist_timestamp()
    base_name = f"testplan_{ts}.xlsx"
    path = os.path.join(base_dir, base_name)

    if not os.path.exists(path):
        return path

    # Collision guard (rare). Append incrementing suffix
    n = 1
    while True:
        alt = os.path.join(base_dir, f"testplan_{ts}_{n}.xlsx")
        if not os.path.exists(alt):
            return alt
        n += 1


def main():
    data = load_json(INPUT_PATH)
    wb = build_workbook(data)

    out_dir = os.path.join(REPO_ROOT, OUTPUT_DIR) if not os.path.isabs(OUTPUT_DIR) else OUTPUT_DIR
    ensure_dir(out_dir)

    out_path = unique_output_path(out_dir)
    wb.save(out_path)

    # Record relative path for the workflow commit step
    rel_path = os.path.relpath(out_path, REPO_ROOT)
    with open(OUTPUT_TRACK_FILE, "w", encoding="utf-8") as f:
        f.write(rel_path)

    print(f"Generated Excel: {rel_path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
