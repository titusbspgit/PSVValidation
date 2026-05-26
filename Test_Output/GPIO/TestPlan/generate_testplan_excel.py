import json
import sys
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Font
from zoneinfo import ZoneInfo
from datetime import datetime
from pathlib import Path

# Constants
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


def load_json(path: Path):
    with path.open('r', encoding='utf-8') as f:
        # Preserve key order just in case (though we map explicitly)
        return json.load(f, object_pairs_hook=OrderedDict)


def build_rows(data, cols):
    rows = []
    for item in data:
        row = [str(item.get(col, "")) if item.get(col, "") is not None else "" for col in cols]
        rows.append(row)
    return rows


def apply_header_style(ws):
    bold = Font(bold=True)
    for cell in ws[1]:
        cell.font = bold
    ws.freeze_panes = "A2"


def main():
    # Resolve input JSON path
    if len(sys.argv) > 1:
        json_path = Path(sys.argv[1])
    else:
        json_path = Path("Test_Output/GPIO/TestPlan/testplan_data.json")

    output_dir = json_path.parent
    output_dir.mkdir(parents=True, exist_ok=True)

    data = load_json(json_path)
    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    # Create workbook
    wb = Workbook()
    # Remove default sheet
    default_ws = wb.active
    wb.remove(default_ws)

    ws_tp = wb.create_sheet("TestPlan")
    ws_md = wb.create_sheet("MetaData")

    # Headers
    ws_tp.append(TESTPLAN_COLS)
    ws_md.append(METADATA_COLS)

    # Rows
    tp_rows = build_rows(data, TESTPLAN_COLS)
    md_rows = build_rows(data, METADATA_COLS)

    for r in tp_rows:
        ws_tp.append(r)
    for r in md_rows:
        ws_md.append(r)

    # Styles and visibility
    apply_header_style(ws_tp)
    apply_header_style(ws_md)
    ws_md.sheet_state = 'veryHidden'

    # Timestamp in IST
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(tz=ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"
    out_path = output_dir / filename

    # Save REAL .xlsx
    wb.save(out_path.as_posix())

    # Record the generated filename for the workflow commit step
    last_file = output_dir / ".last_excel"
    last_file.write_text(out_path.as_posix(), encoding='utf-8')
    print(f"Wrote Excel: {out_path}")


if __name__ == "__main__":
    main()
