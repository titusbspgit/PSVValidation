#!/usr/bin/env python3
import json
import os
import sys
from datetime import datetime
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:
    ZoneInfo = None
from openpyxl import Workbook
from openpyxl.styles import Font

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
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

def _cell_value(v):
    if isinstance(v, (dict, list)):
        return json.dumps(v, ensure_ascii=False, separators=(",", ":"))
    return v

def json_to_excel(input_path: str, output_dir: str) -> str:
    with open(input_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each item must be an object. Offending index: {i}")

    wb = Workbook()
    # Remove default sheet and create explicit ones
    default_ws = wb.active
    wb.remove(default_ws)

    ws = wb.create_sheet(title="TestPlan")
    ws_meta = wb.create_sheet(title="MetaData")
    ws_meta.sheet_state = "veryHidden"

    # Headers (bold)
    ws.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)
    bold = Font(bold=True)
    for cell in ws[1]:
        cell.font = bold
    for cell in ws_meta[1]:
        cell.font = bold

    # Freeze first row
    ws.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Rows (preserve order, no data loss)
    for row in data:
        ws.append([_cell_value(row.get(col, "")) for col in TESTPLAN_COLUMNS])
        ws_meta.append([_cell_value(row.get(col, "")) for col in METADATA_COLUMNS])

    os.makedirs(output_dir, exist_ok=True)

    if ZoneInfo is not None:
        ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        # Fallback: approximate IST by UTC+5:30 if zoneinfo missing
        from datetime import timezone, timedelta
        ist_now = datetime.now(timezone(timedelta(hours=5, minutes=30)))

    timestamp = ist_now.strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{timestamp}.xlsx"
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)

    # Print the path so workflow can capture it
    print(out_path)
    return out_path


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to JSON file containing an array of objects")
    parser.add_argument("--output-dir", default="Test_Output", help="Directory to write the Excel file to")
    args = parser.parse_args()

    try:
        json_to_excel(args.input, args.output_dir)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
