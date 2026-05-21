#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate a real .xlsx Test Plan from JSON with two sheets:
- TestPlan (visible)
- MetaData (veryHidden)

Usage:
  python scripts/generate_testplan_excel.py --json data/testplans/pcie_testplan.json --outdir Test_Output/PCIE/TestPlan [--timestamp-ist]

Notes:
- Preserves text exactly as provided in JSON.
- Maintains row order across both sheets.
- Header row is bold and first row is frozen.
- MetaData sheet is set to veryHidden.
"""
import argparse
import json
import os
from datetime import datetime, timedelta
from pathlib import Path

from openpyxl import Workbook
from openpyxl.styles import Font

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


def ist_timestamp(ts: datetime) -> str:
    # IST is UTC+05:30 and has no DST
    ist_dt = ts + timedelta(hours=5, minutes=30)
    return ist_dt.strftime("%Y%m%d_%H%M%S")


def build_workbook(rows: list) -> Workbook:
    wb = Workbook()

    # TestPlan sheet
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_tp.append(TESTPLAN_COLS)
    for cell in ws_tp[1]:
        cell.font = Font(bold=True)
    ws_tp.freeze_panes = "A2"

    # MetaData sheet
    ws_md = wb.create_sheet("MetaData")
    ws_md.append(METADATA_COLS)
    for cell in ws_md[1]:
        cell.font = Font(bold=True)
    ws_md.freeze_panes = "A2"

    # Append rows preserving order
    for obj in rows:
        # TestPlan row
        tp_row = [
            obj.get("Index", ""),
            obj.get("SS / Module", ""),
            obj.get("Feature", ""),
            obj.get("Test Case Name", ""),
            obj.get("Test Description", ""),
            obj.get("Speed", ""),
            obj.get("Mode", ""),
            obj.get("Memory Start Offset", ""),
            obj.get("Memory End Offset", ""),
            obj.get("Remarks", ""),
            obj.get("Test Steps / Procedure", ""),
            obj.get("Impacted Registers", ""),
            obj.get("Validation / Acceptance Criteria", ""),
            obj.get("Code Generation (Required / Not)", ""),
        ]
        ws_tp.append(tp_row)

        # MetaData row
        md_row = [
            obj.get("Index", ""),
            obj.get("Test Case Name", ""),
            obj.get("Meta Test Description", ""),
            obj.get("Meta Test Steps / Procedure", ""),
            obj.get("Meta Impacted Registers", ""),
            obj.get("Meta Validation / Acceptance Criteria", ""),
            obj.get("Meta Headers", ""),
            obj.get("Meta Macros", ""),
            obj.get("Meta Arrays", ""),
        ]
        ws_md.append(md_row)

    # Very hide MetaData sheet
    ws_md.sheet_state = "veryHidden"

    return wb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", required=True, help="Path to input JSON array file")
    ap.add_argument("--outdir", required=True, help="Directory to write the XLSX file to")
    ap.add_argument("--timestamp-ist", action="store_true", help="Use IST timestamp in filename (YYYYMMDD_HHMMSS)")
    args = ap.parse_args()

    # Read JSON
    with open(args.json, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit("json_data must be a non-empty array of objects")

    # Build workbook
    wb = build_workbook(data)

    # Filename timestamp
    now_utc = datetime.utcnow()
    ts = ist_timestamp(now_utc) if args.timestamp_ist else now_utc.strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    # Ensure outdir exists
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    # Save .xlsx (real binary Excel)
    out_path = outdir / filename
    wb.save(out_path.as_posix())

    print(str(out_path))


if __name__ == "__main__":
    main()
