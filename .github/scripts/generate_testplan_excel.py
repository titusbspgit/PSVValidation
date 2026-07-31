#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime

try:
    import pytz
except ImportError:
    pytz = None

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

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

HEADER_FILL = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")  # Blue
HEADER_FONT = Font(bold=True, color="FFFFFFFF")  # White
CELL_ALIGN = Alignment(wrap_text=True, vertical="top")


def autosize_columns(ws, max_width=80, min_width=12):
    dims = {}
    for row in ws.iter_rows(values_only=False):
        for cell in row:
            val = cell.value
            if val is None:
                length = 0
            else:
                s = str(val)
                # consider multi-line content
                length = max(len(line) for line in s.splitlines()) if s else 0
            dims[cell.column_letter] = max(dims.get(cell.column_letter, 0), length)
    for col, width in dims.items():
        final = max(min_width, min(max_width, width + 2))
        ws.column_dimensions[col].width = final


def write_sheet(ws, columns, rows):
    # Header
    ws.append(columns)
    for idx, col_name in enumerate(columns, start=1):
        c = ws.cell(row=1, column=idx)
        c.fill = HEADER_FILL
        c.font = HEADER_FONT
        c.alignment = CELL_ALIGN
    # Data rows
    for r in rows:
        ws.append([r.get(col, "") for col in columns])
    # Wrap text for all cells
    for row in ws.iter_rows():
        for cell in row:
            cell.alignment = CELL_ALIGN
    # Freeze first row
    ws.freeze_panes = "A2"
    # Auto-size columns
    autosize_columns(ws)


def build_rows(data, columns):
    rows = []
    for item in data:
        row = {col: ("" if item.get(col) is None else item.get(col)) for col in columns}
        rows.append(row)
    return rows


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--input", required=True, help="Path to input JSON file (array of objects)")
    parser.add_argument("--output-dir", required=True, help="Directory to write the Excel file")
    parser.add_argument("--ip-name", required=True, help="IP name for filename prefix")
    args = parser.parse_args()

    # Read JSON
    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    # Prepare workbook
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Build rows strictly following the provided columns
    testplan_rows = build_rows(data, TESTPLAN_COLUMNS)
    metadata_rows = build_rows(data, METADATA_COLUMNS)

    write_sheet(ws_plan, TESTPLAN_COLUMNS, testplan_rows)
    write_sheet(ws_meta, METADATA_COLUMNS, metadata_rows)

    # VERY HIDDEN metadata sheet
    ws_meta.sheet_state = "veryHidden"

    # IST timestamp
    if pytz is not None:
        tz = pytz.timezone("Asia/Kolkata")
        now = datetime.now(tz)
    else:
        # Fallback: manual offset +05:30 (does not handle DST as IST has none)
        from datetime import timedelta
        now = datetime.utcnow() + timedelta(hours=5, minutes=30)

    timestamp = now.strftime("%Y%m%d_%H%M%S")
    filename = f"{args.ip_name}_TestPlan_{timestamp}.xlsx"

    out_dir = args.output_dir
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, filename)

    # Save as real .xlsx
    wb.save(out_path)

    # Print absolute path for caller
    print(os.path.abspath(out_path))


if __name__ == "__main__":
    main()
