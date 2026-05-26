#!/usr/bin/env python3
import json
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Columns for the visible TestPlan sheet
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

# Columns for the VeryHidden MetaData sheet
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


def main():
    here = Path(__file__).resolve().parent
    data_path = here / "testplan_data.json"
    if not data_path.exists():
        raise SystemExit(f"Missing input JSON: {data_path}")

    # Load and validate JSON
    try:
        data = json.loads(data_path.read_text(encoding="utf-8"))
    except Exception as e:
        raise SystemExit(f"Failed to parse JSON: {e}")

    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    # Create workbook and sheets
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")
    # Set MetaData to VeryHidden
    ws_meta.sheet_state = "veryHidden"

    # Write headers (bold) and freeze first row
    bold = Font(bold=True)
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, start=1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = bold
    ws.freeze_panes = "A2"

    for col_idx, col_name in enumerate(METADATA_COLUMNS, start=1):
        cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
        cell.font = bold
    ws_meta.freeze_panes = "A2"

    # Fill rows preserving order
    for r, item in enumerate(data, start=2):
        if not isinstance(item, dict):
            raise SystemExit(f"Each item must be an object; got {type(item)} at row {r-1}")
        # TestPlan row
        for c, key in enumerate(TESTPLAN_COLUMNS, start=1):
            ws.cell(row=r, column=c, value=str(item.get(key, "")))
        # MetaData row
        # Map Index and Test Case Name again to maintain row association
        meta_values = [
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
        for c, val in enumerate(meta_values, start=1):
            ws_meta.cell(row=r, column=c, value=str(val))

    # Filename with IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    out_name = f"testplan_{ts}.xlsx"
    out_path = here / out_name

    wb.save(out_path)
    print(f"Generated: {out_path}")


if __name__ == "__main__":
    main()
