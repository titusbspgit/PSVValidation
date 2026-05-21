#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a REAL .xlsx Test Plan workbook from a JSON array.
- Sheet "TestPlan" (visible)
- Sheet "MetaData" (VeryHidden)
- Bold headers, freeze first row
- File name: testplan_<YYYYMMDD_HHMMSS>.xlsx in IST (Asia/Kolkata)

Usage:
  python scripts/generate_testplan_excel.py --input path/to/input.json --output-dir Test_Output/GPIO/TestCode --outpath-file tmp/outpath.txt

The script validates that the input JSON is an array of objects and preserves row order across sheets.
"""
import argparse
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font


def ist_now_timestamp():
    fmt = "%Y%m%d_%H%M%S"
    # Prefer system zoneinfo for Asia/Kolkata
    if ZoneInfo is not None:
        try:
            return datetime.now(ZoneInfo("Asia/Kolkata")).strftime(fmt)
        except Exception:
            pass
    # Fallback: fixed offset IST (UTC+05:30)
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime(fmt)


def validate_json_array(data):
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array of objects")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise ValueError(f"Each element must be an object. Found {type(item).__name__} at index {i}")


def build_workbook(rows):
    # Define exact column schemas
    testplan_columns = [
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

    metadata_columns = [
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

    wb = Workbook()

    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Write headers with bold font and freeze first row
    bold = Font(bold=True)

    for col_idx, name in enumerate(testplan_columns, start=1):
        c = ws_plan.cell(row=1, column=col_idx, value=name)
        c.font = bold
    ws_plan.freeze_panes = "A2"

    for col_idx, name in enumerate(metadata_columns, start=1):
        c = ws_meta.cell(row=1, column=col_idx, value=name)
        c.font = bold
    ws_meta.freeze_panes = "A2"

    # VeryHidden MetaData sheet
    ws_meta.sheet_state = "veryHidden"

    # Write data preserving order
    for row_idx, row in enumerate(rows, start=2):
        for col_idx, name in enumerate(testplan_columns, start=1):
            ws_plan.cell(row=row_idx, column=col_idx, value=row.get(name, ""))
        for col_idx, name in enumerate(metadata_columns, start=1):
            ws_meta.cell(row=row_idx, column=col_idx, value=row.get(name, ""))

    return wb


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON array")
    parser.add_argument("--input", required=True, help="Path to JSON file (array of objects)")
    parser.add_argument("--output-dir", required=True, help="Directory to write the .xlsx file into")
    parser.add_argument("--outpath-file", default="tmp/outpath.txt", help="File to write the absolute output .xlsx path into")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    try:
        validate_json_array(data)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(2)

    wb = build_workbook(data)

    ts = ist_now_timestamp()
    filename = f"testplan_{ts}.xlsx"

    os.makedirs(args.output_dir, exist_ok=True)
    outpath = os.path.join(args.output_dir, filename)

    # Save REAL .xlsx (openpyxl)
    wb.save(outpath)

    # Record path for CI to commit
    outdir = os.path.dirname(args.outpath_file)
    if outdir:
        os.makedirs(outdir, exist_ok=True)
    with open(args.outpath_file, "w", encoding="utf-8") as f:
        f.write(outpath)

    print(outpath)


if __name__ == "__main__":
    main()
