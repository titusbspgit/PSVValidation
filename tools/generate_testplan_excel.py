#!/usr/bin/env python3
import json
import argparse
import os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except ImportError:
    ZoneInfo = None

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

HEADER_FILL = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")
HEADER_FONT = Font(bold=True, color="FFFFFF")
WRAP_ALIGN = Alignment(wrap_text=True, vertical="top")

# Reasonable fixed widths for readability
TESTPLAN_WIDTHS = {
    "A": 8,   # Index
    "B": 14,  # SS / Module
    "C": 40,  # Feature
    "D": 34,  # Test Case Name
    "E": 70,  # Test Description
    "F": 10,  # Speed
    "G": 10,  # Mode
    "H": 20,  # Memory Start Offset
    "I": 20,  # Memory End Offset
    "J": 40,  # Remarks
    "K": 80,  # Test Steps / Procedure
    "L": 60,  # Impacted Registers
    "M": 80,  # Validation / Acceptance Criteria
    "N": 24,  # Code Generation (Required / Not)
}

METADATA_WIDTHS = {
    "A": 8,
    "B": 34,
    "C": 80,
    "D": 80,
    "E": 60,
    "F": 80,
    "G": 40,
    "H": 40,
    "I": 80,
}

def build_workbook(rows):
    wb = Workbook()
    # Remove default sheet
    default_ws = wb.active
    wb.remove(default_ws)

    ws_plan = wb.create_sheet(title="TestPlan")
    ws_meta = wb.create_sheet(title="MetaData")

    # Write headers
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)

    # Style headers
    for cell in ws_plan[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = WRAP_ALIGN
    for cell in ws_meta[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = WRAP_ALIGN

    # Write rows maintaining order
    for r in rows:
        plan_row = [str(r.get(col, "")) for col in TESTPLAN_COLUMNS]
        meta_row = [str(r.get(col, "")) for col in METADATA_COLUMNS]
        ws_plan.append(plan_row)
        ws_meta.append(meta_row)

    # Apply wrap-text to all cells and align top
    for ws in (ws_plan, ws_meta):
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
            for cell in row:
                cell.alignment = WRAP_ALIGN
        # Freeze header row
        ws.freeze_panes = "A2"

    # Set column widths
    for col_letter, width in TESTPLAN_WIDTHS.items():
        ws_plan.column_dimensions[col_letter].width = width
    for col_letter, width in METADATA_WIDTHS.items():
        ws_meta.column_dimensions[col_letter].width = width

    # Set MetaData sheet to VeryHidden
    ws_meta.sheet_state = "veryHidden"

    return wb


def get_ist_timestamp():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        now_ist = datetime.now(tz)
    else:
        # Fallback: manual offset +05:30
        now_ist = datetime.utcnow()
        # naive, but acceptable if zoneinfo unavailable on runner
        from datetime import timedelta
        now_ist = now_ist + timedelta(hours=5, minutes=30)
    return now_ist.strftime("%Y%m%d_%H%M%S")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--ip-name', required=True)
    ap.add_argument('--input', required=True)
    ap.add_argument('--output-dir', required=True)
    args = ap.parse_args()

    with open(args.input, 'r', encoding='utf-8') as f:
        data = json.load(f)

    if not isinstance(data, list):
        raise SystemExit("json_data must be a JSON array of objects")

    # Build workbook
    wb = build_workbook(data)

    # Ensure output dir exists
    out_dir = args.output_dir.rstrip('/').rstrip('\\')
    os.makedirs(out_dir, exist_ok=True)

    ts = get_ist_timestamp()
    filename = f"{args.ip_name}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(out_dir, filename)

    wb.save(out_path)

    # Write helper files for workflow commit step
    with open('tools/generated_file_path.txt', 'w', encoding='utf-8') as f:
        f.write(out_path)
    with open('tools/generated_filename.txt', 'w', encoding='utf-8') as f:
        f.write(filename)

    print(out_path)

if __name__ == '__main__':
    main()
