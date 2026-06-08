#!/usr/bin/env python3
"""
GPIO TestPlan Excel Generator
=============================
Reads testplan_data.json and produces a real .xlsx workbook with:
  - Sheet 1: "TestPlan"  (visible)   - user-facing test plan columns
  - Sheet 2: "MetaData"  (VeryHidden) - machine-readable metadata columns

Formatting:
  - Header row: bold, blue background (#4472C4), white font
  - All cells: wrap text enabled
  - Column widths: auto-adjusted (capped at 60)
  - First row frozen in both sheets

Requires: openpyxl >= 3.0
Generation timestamp: 2026-06-08T19:37:03+05:30
"""

import json
import os
from datetime import datetime, timezone, timedelta

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.worksheet import Worksheet

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
JSON_PATH = os.path.join(SCRIPT_DIR, "testplan_data.json")
OUTPUT_DIR = SCRIPT_DIR  # .xlsx lands next to the script

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

HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
WRAP = Alignment(wrap_text=True, vertical="top")
MAX_COL_WIDTH = 60
MIN_COL_WIDTH = 12


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
def _auto_width(ws: Worksheet) -> None:
    """Set each column width to the max content length (capped)."""
    for col_cells in ws.columns:
        max_len = 0
        col_letter = col_cells[0].column_letter
        for cell in col_cells:
            try:
                val = str(cell.value) if cell.value is not None else ""
                # Use the longest single line for width estimation
                longest_line = max((len(line) for line in val.split("\n")), default=0)
                if longest_line > max_len:
                    max_len = longest_line
            except Exception:
                pass
        adjusted = min(max(max_len + 3, MIN_COL_WIDTH), MAX_COL_WIDTH)
        ws.column_dimensions[col_letter].width = adjusted


def _write_sheet(ws: Worksheet, columns: list[str], rows: list[dict]) -> None:
    """Write header + data rows with formatting."""
    # Header row
    for col_idx, header in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP

    # Data rows
    for row_idx, record in enumerate(rows, start=2):
        for col_idx, key in enumerate(columns, start=1):
            value = record.get(key, "")
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = WRAP

    # Freeze first row
    ws.freeze_panes = "A2"

    # Auto-adjust column widths
    _auto_width(ws)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
def main() -> None:
    # 1. Load JSON
    with open(JSON_PATH, "r", encoding="utf-8") as fh:
        data: list[dict] = json.load(fh)

    assert isinstance(data, list) and len(data) > 0, "JSON must be a non-empty array"
    print(f"Loaded {len(data)} test case(s) from {JSON_PATH}")

    # 2. Create workbook
    wb = Workbook()

    # --- TestPlan sheet (visible) ---
    ws_tp: Worksheet = wb.active  # type: ignore[assignment]
    ws_tp.title = "TestPlan"
    _write_sheet(ws_tp, TESTPLAN_COLUMNS, data)

    # --- MetaData sheet (VeryHidden) ---
    ws_md: Worksheet = wb.create_sheet(title="MetaData")
    _write_sheet(ws_md, METADATA_COLUMNS, data)
    ws_md.sheet_state = "veryHidden"  # VERY HIDDEN

    # 3. Generate filename with IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"
    filepath = os.path.join(OUTPUT_DIR, filename)

    # 4. Save
    wb.save(filepath)
    print(f"Excel workbook saved: {filepath}")
    print(f"  - Sheet 'TestPlan':  {len(data)} data rows, {len(TESTPLAN_COLUMNS)} columns")
    print(f"  - Sheet 'MetaData':  {len(data)} data rows, {len(METADATA_COLUMNS)} columns  [VeryHidden]")
    print(f"  - File size: {os.path.getsize(filepath):,} bytes")


if __name__ == "__main__":
    main()
