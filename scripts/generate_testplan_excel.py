import json
import argparse
import os
from datetime import datetime
from zoneinfo import ZoneInfo
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

def write_sheet(ws, records, columns):
    ws.append(columns)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"
    for rec in records:
        row = [rec.get(col, "") for col in columns]
        ws.append(row)


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to input JSON file")
    parser.add_argument("--output-dir", required=True, help="Output directory for Excel file")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise ValueError("json_data must be an array of objects")

    # Create workbook and sheets
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    write_sheet(ws_plan, data, TESTPLAN_COLUMNS)

    ws_meta = wb.create_sheet("MetaData")
    write_sheet(ws_meta, data, METADATA_COLUMNS)
    ws_meta.sheet_state = "veryHidden"  # MUST be Very Hidden

    # Filename with IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    os.makedirs(args.output_dir, exist_ok=True)
    out_path = os.path.join(args.output_dir, filename)
    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
