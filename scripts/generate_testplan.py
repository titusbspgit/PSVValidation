#!/usr/bin/env python3
import json
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# Constants from task inputs
IP_NAME = "GPIO"
OUTPUT_DIR = Path("Test_Output/GPIO/TestPlan/")
INPUT_JSON_PATH = Path("data/testplan_input.json")
GENERATED_PATH_FILE = Path("data/generated_output_path.txt")

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


def load_json_array(path: Path):
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array")
    return data


def style_header(ws, last_col_letter: str):
    header_fill = PatternFill(fill_type="solid", fgColor="4472C4")  # blue
    header_font = Font(bold=True, color="FFFFFF")  # white
    wrap = Alignment(wrap_text=True, vertical="center")

    for cell in ws[1]:
        cell.fill = header_fill
        cell.font = header_font
        cell.alignment = wrap

    # Freeze first row
    ws.freeze_panes = "A2"

    # Filters
    ws.auto_filter.ref = f"A1:{last_col_letter}{ws.max_row}"


def style_cells(ws, max_col: int):
    wrap_top = Alignment(wrap_text=True, vertical="top")

    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=max_col):
        for cell in row:
            cell.alignment = wrap_top

    # Reasonable column widths
    widths = {
        1: 8,   # Index
        2: 14,  # SS / Module
        3: 28,  # Feature
        4: 28,  # Test Case Name
        5: 60,  # Test Description
        6: 10,  # Speed
        7: 10,  # Mode
        8: 18,  # Memory Start Offset
        9: 18,  # Memory End Offset
        10: 50, # Remarks
        11: 60, # Test Steps / Procedure
        12: 60, # Impacted Registers
        13: 60, # Validation / Acceptance Criteria
        14: 22, # Code Generation (Required / Not)
    }
    for col_idx in range(1, max_col + 1):
        ws.column_dimensions[get_column_letter(col_idx)].width = widths.get(col_idx, 16)


def write_sheet(ws, columns, rows):
    # Header
    ws.append(columns)
    # Data rows
    for obj in rows:
        row = [obj.get(col, "") for col in columns]
        ws.append(row)


def validate_workbook(xlsx_path: Path, input_count: int):
    wb = load_workbook(xlsx_path)
    if "TestPlan" not in wb.sheetnames or "MetaData" not in wb.sheetnames:
        raise ValueError("Required sheets missing")
    ws1 = wb["TestPlan"]
    ws2 = wb["MetaData"]
    # VeryHidden check (sheet_state should be 'veryHidden')
    if getattr(ws2, "sheet_state", "visible") != "veryHidden":
        raise ValueError("MetaData sheet is not VeryHidden")
    # Row count check (minus header)
    if (ws1.max_row - 1) != input_count or (ws2.max_row - 1) != input_count:
        raise ValueError("Row count mismatch vs input")
    # Header validation
    h1 = [c.value for c in ws1[1]]
    h2 = [c.value for c in ws2[1]]
    if h1 != TESTPLAN_COLUMNS:
        raise ValueError("TestPlan header mismatch")
    if h2 != METADATA_COLUMNS:
        raise ValueError("MetaData header mismatch")


def main():
    rows = load_json_array(INPUT_JSON_PATH)

    # Timestamp in Asia/Kolkata
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
    else:
        tz = None
    now = datetime.now(tz) if tz else datetime.utcnow()
    ts = now.strftime("%Y%m%d_%H%M%S")

    filename = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_path = OUTPUT_DIR / filename
    out_path.parent.mkdir(parents=True, exist_ok=True)

    wb = Workbook()
    # Create TestPlan sheet as active
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    # Create MetaData sheet
    ws_meta = wb.create_sheet("MetaData")

    write_sheet(ws_plan, TESTPLAN_COLUMNS, rows)
    write_sheet(ws_meta, METADATA_COLUMNS, rows)

    # VeryHidden metadata sheet
    ws_meta.sheet_state = "veryHidden"

    # Styling after data written
    style_header(ws_plan, get_column_letter(len(TESTPLAN_COLUMNS)))
    style_cells(ws_plan, len(TESTPLAN_COLUMNS))

    style_header(ws_meta, get_column_letter(len(METADATA_COLUMNS)))
    style_cells(ws_meta, len(METADATA_COLUMNS))

    # Save and validate by reopening
    wb.save(out_path)
    validate_workbook(out_path, len(rows))

    GENERATED_PATH_FILE.parent.mkdir(parents=True, exist_ok=True)
    GENERATED_PATH_FILE.write_text(str(out_path), encoding="utf-8")
    print(f"Generated: {out_path}")


if __name__ == "__main__":
    main()
