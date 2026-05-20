#!/usr/bin/env python3
import os
import json
import requests
from datetime import datetime
import pytz
from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_HEADERS = [
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

METADATA_HEADERS = [
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


def fetch_json(json_url: str):
    resp = requests.get(json_url, timeout=60)
    resp.raise_for_status()
    try:
        data = resp.json()
    except Exception as e:
        raise SystemExit(f"ERROR: Failed to parse JSON from {json_url}: {e}")
    if not isinstance(data, list):
        raise SystemExit("ERROR: JSON root must be an array of row objects")
    return data


def build_workbook(rows):
    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_md = wb.create_sheet("MetaData")

    # VeryHidden MetaData sheet
    ws_md.sheet_state = 'veryHidden'

    # Write headers with bold font and freeze panes
    bold = Font(bold=True)

    # TestPlan headers
    for col, hdr in enumerate(TESTPLAN_HEADERS, start=1):
        c = ws_tp.cell(row=1, column=col, value=hdr)
        c.font = bold
    ws_tp.freeze_panes = 'A2'

    # MetaData headers
    for col, hdr in enumerate(METADATA_HEADERS, start=1):
        c = ws_md.cell(row=1, column=col, value=hdr)
        c.font = bold
    ws_md.freeze_panes = 'A2'

    # Write rows preserving order
    r_tp = 2
    r_md = 2
    for item in rows:
        # TestPlan row
        tp_vals = [item.get(h, "") for h in TESTPLAN_HEADERS]
        for col, val in enumerate(tp_vals, start=1):
            ws_tp.cell(row=r_tp, column=col, value=val)
        r_tp += 1

        # MetaData row
        md_vals = [item.get(h, "") for h in METADATA_HEADERS]
        for col, val in enumerate(md_vals, start=1):
            ws_md.cell(row=r_md, column=col, value=val)
        r_md += 1

    return wb


def main():
    json_url = os.environ.get("JSON_URL")
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
    if not json_url:
        raise SystemExit("ERROR: JSON_URL environment variable not set")

    rows = fetch_json(json_url)

    # Build workbook
    wb = build_workbook(rows)

    # IST timestamp
    ist = pytz.timezone("Asia/Kolkata")
    now_ist = datetime.now(ist)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")

    # Ensure output directory exists
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, f"testplan_{ts}.xlsx")

    # Save REAL .xlsx
    wb.save(out_path)
    print(f"Wrote: {out_path}")


if __name__ == "__main__":
    main()
