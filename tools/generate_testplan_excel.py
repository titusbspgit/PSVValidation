#!/usr/bin/env python3
import os
import json
from datetime import datetime
from zoneinfo import ZoneInfo
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
    "Imparted Registers" if False else "Impacted Registers",
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

HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")
HEADER_FONT = Font(bold=True, color="FFFFFF")
WRAP_TOP = Alignment(wrap_text=True, vertical="top")


def write_headers(ws, headers):
    ws.append(headers)
    for col_idx, _ in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=col_idx)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP_TOP
    ws.freeze_panes = "A2"


def set_column_widths(ws, widths):
    for col_letter, width in widths.items():
        ws.column_dimensions[col_letter].width = width


def ensure_array(json_text):
    try:
        data = json.loads(json_text)
    except Exception as e:
        raise SystemExit(f"Invalid JSON input: {e}")
    if not isinstance(data, list):
        raise SystemExit("json_data must be a JSON array")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise SystemExit(f"Each item must be an object; item {i} is {type(item)}")
    return data


def main():
    json_text = os.environ.get("JSON_DATA", "[]")
    data = ensure_array(json_text)

    ip_name = os.environ.get("IP_NAME", "IP")
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output")

    # Build workbook
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    write_headers(ws_plan, TESTPLAN_COLUMNS)
    write_headers(ws_meta, METADATA_COLUMNS)

    # Fill rows preserving order
    for item in data:
        plan_row = [
            item.get("Index", ""),
            item.get("SS / Module", ""),
            item.get("Feature", ""),
            item.get("Test Case Name", ""),
            item.get("Test Description", ""),
            item.get("Speed", ""),
            item.get("Mode", ""),
            item.get("Memory Start Offset", ""),
            item.get("Memory End Offset", ""),
            item.get("Remarks", ""),
            item.get("Test Steps / Procedure", ""),
            item.get("Impacted Registers", ""),
            item.get("Validation / Acceptance Criteria", ""),
            item.get("Code Generation (Required / Not)", ""),
        ]
        ws_plan.append(plan_row)

        meta_row = [
            item.get("Index", ""),
            item.get("Test Case Name", ""),
            item.get("Meta Test Description", ""),
            item.get("Meta Test Steps / Procedure", ""),
            item.get("Meta Impacted Registers", ""),
            item.get("Meta Validation / Acceptance Criteria", ""),
            item.get("Meta Headers", ""),
            item.get("Meta Macros", ""),
            item.get("Meta Arrays", ""),
        ]
        ws_meta.append(meta_row)

    # Add meta info rows after the aligned rows
    now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    ws_meta.append([""] * len(METADATA_COLUMNS))
    ws_meta.append([
        "",  # Index
        "Meta-Info",  # Test Case Name
        f"Generated IST: {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}",  # Meta Test Description
        f"IP_NAME={ip_name}; Source=repo:titusbspgit/PSVValidation branch:main",  # Meta Test Steps / Procedure
        "",  # Meta Impacted Registers
        "",  # Meta Validation / Acceptance Criteria
        "",  # Meta Headers
        "",  # Meta Macros
        "",  # Meta Arrays
    ])

    # Wrap text for all used cells
    for ws in (ws_plan, ws_meta):
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
            for cell in row:
                cell.alignment = WRAP_TOP

    # Reasonable widths for readability
    set_column_widths(ws_plan, {
        'A': 8, 'B': 16, 'C': 30, 'D': 28, 'E': 70, 'F': 10, 'G': 12,
        'H': 22, 'I': 22, 'J': 40, 'K': 90, 'L': 40, 'M': 60, 'N': 26
    })
    set_column_widths(ws_meta, {
        'A': 8, 'B': 30, 'C': 90, 'D': 90, 'E': 50, 'F': 60, 'G': 40, 'H': 30, 'I': 60
    })

    # Very hide the MetaData sheet
    ws_meta.sheet_state = 'veryHidden'

    # Filename with IST timestamp following <IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
    ts = now_ist.strftime('%Y%m%d_%H%M%S')
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"

    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)

    print(f"Generated Excel: {out_path}")

if __name__ == "__main__":
    main()
