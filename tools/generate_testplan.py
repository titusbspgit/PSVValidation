import json
from datetime import datetime, timezone, timedelta
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font

# Config
INPUT_JSON = Path("data/final_testplan.json")
OUTPUT_DIR = Path("Test_Output/GPIO/TestPlan")
LAST_NAME_FILE = OUTPUT_DIR / ".last_generated_name"
LAST_TS_FILE = OUTPUT_DIR / ".last_generated_ts"

# Column definitions
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

def main():
    # Load JSON
    data = json.loads(INPUT_JSON.read_text(encoding="utf-8"))
    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    # Timestamp in IST for filename
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime("%YMMDD_%H%M%S").replace("MM", datetime.now(ist).strftime("%m"))
    # Build output paths
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    fname = f"testplan_{ts}.xlsx"
    fpath = OUTPUT_DIR / fname

    # Create workbook
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write TestPlan header
    ws.append(TESTPLAN_COLS)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Write TestPlan rows
    for row in data:
        ws.append([row.get(col, "") for col in TESTPLAN_COLS])

    # MetaData sheet
    ws2 = wb.create_sheet("MetaData")
    ws2.append(METADATA_COLS)
    for cell in ws2[1]:
        cell.font = Font(bold=True)
    ws2.freeze_panes = "A2"

    for row in data:
        ws2.append([row.get(col, "") for col in METADATA_COLS])

    # Very hide MetaData sheet
    ws2.sheet_state = "veryHidden"

    # Save workbook
    wb.save(fpath)

    # Write helper files for workflow commit message
    LAST_NAME_FILE.write_text(str(fpath.as_posix()), encoding="utf-8")
    LAST_TS_FILE.write_text(ts, encoding="utf-8")

if __name__ == "__main__":
    main()
