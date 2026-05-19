#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font

def build_workbook(data):
    # Define columns in exact order
    testplan_cols = [
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
    metadata_cols = [
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
    # Create sheets
    ws_main = wb.active
    ws_main.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Header formatting
    header_font = Font(bold=True)

    # Write headers
    for col_idx, h in enumerate(testplan_cols, start=1):
        c = ws_main.cell(row=1, column=col_idx, value=h)
        c.font = header_font
    for col_idx, h in enumerate(metadata_cols, start=1):
        c = ws_meta.cell(row=1, column=col_idx, value=h)
        c.font = header_font

    # Freeze top rows
    ws_main.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Write rows preserving order
    r = 2
    for item in data:
        # TestPlan row
        row_vals = [item.get(k, "") for k in testplan_cols]
        for col_idx, v in enumerate(row_vals, start=1):
            ws_main.cell(row=r, column=col_idx, value=v)
        # MetaData row
        meta_vals = [item.get(k, "") for k in metadata_cols]
        for col_idx, v in enumerate(meta_vals, start=1):
            ws_meta.cell(row=r, column=col_idx, value=v)
        r += 1

    # VeryHidden meta sheet
    ws_meta.sheet_state = "veryHidden"

    return wb


def ensure_unique_filename(outdir: Path, base_name: str) -> Path:
    p = outdir / base_name
    if not p.exists():
        return p
    stem, ext = os.path.splitext(base_name)
    i = 1
    while True:
        candidate = outdir / f"{stem}_{i}{ext}"
        if not candidate.exists():
            return candidate
        i += 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True, help="Path to JSON file with final aggregated Test Plan")
    ap.add_argument("--outdir", required=True, help="Output directory inside repo (e.g., Test_Output/PCIE/TestPlan)")
    ap.add_argument("--output-path-file", default="scripts/testplan_output_path.txt", help="File to write the generated Excel relative path to")
    args = ap.parse_args()

    # Load and validate JSON
    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit("json_data must be a non-empty array")
    for i, obj in enumerate(data):
        if not isinstance(obj, dict):
            raise SystemExit(f"Each json_data element must be an object; found {type(obj)} at index {i}")

    # Build workbook
    wb = build_workbook(data)

    # IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(tz=ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    # Ensure output directory exists
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    # Ensure unique filename
    out_path = ensure_unique_filename(outdir, filename)

    # Save workbook (REAL .xlsx)
    wb.save(out_path)

    # Write relative path for commit step
    rel_path = str(out_path)
    Path(args.output_path_file).parent.mkdir(parents=True, exist_ok=True)
    with open(args.output_path_file, "w", encoding="utf-8") as f:
        f.write(rel_path)
    print(rel_path)

if __name__ == "__main__":
    main()
