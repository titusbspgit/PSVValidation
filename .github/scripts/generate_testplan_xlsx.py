#!/usr/bin/env python3
import os
import sys
import json
import urllib.request
from datetime import datetime
from zoneinfo import ZoneInfo
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


def fetch_json(url: str):
    with urllib.request.urlopen(url) as resp:
        data = resp.read()
    payload = json.loads(data.decode("utf-8"))
    if not isinstance(payload, list):
        raise ValueError("json_data must be a JSON array")
    # Ensure each element is an object
    for i, row in enumerate(payload):
        if not isinstance(row, dict):
            raise ValueError(f"json_data element at index {i} is not an object")
    return payload


def bold_headers(ws, headers):
    ws.append(headers)
    bold = Font(bold=True)
    for col in range(1, len(headers) + 1):
        ws.cell(row=1, column=col).font = bold
    ws.freeze_panes = "A2"


def build_workbook(rows):
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Header rows with formatting
    bold_headers(ws_plan, TESTPLAN_HEADERS)
    bold_headers(ws_meta, METADATA_HEADERS)

    # Populate rows preserving order
    for row in rows:
        ws_plan.append([
            row.get("Index", ""),
            row.get("SS / Module", ""),
            row.get("Feature", ""),
            row.get("Test Case Name", ""),
            row.get("Test Description", ""),
            row.get("Speed", ""),
            row.get("Mode", ""),
            row.get("Memory Start Offset", ""),
            row.get("Memory End Offset", ""),
            row.get("Remarks", ""),
            row.get("Test Steps / Procedure", ""),
            row.get("Impacted Registers", ""),
            row.get("Validation / Acceptance Criteria", ""),
            row.get("Code Generation (Required / Not)", ""),
        ])

        ws_meta.append([
            row.get("Index", ""),
            row.get("Test Case Name", ""),
            row.get("Meta Test Description", ""),
            row.get("Meta Test Steps / Procedure", ""),
            row.get("Meta Impacted Registers", ""),
            row.get("Meta Validation / Acceptance Criteria", ""),
            row.get("Meta Headers", ""),
            row.get("Meta Macros", ""),
            row.get("Meta Arrays", ""),
        ])

    # Make MetaData very hidden
    ws_meta.sheet_state = "veryHidden"

    return wb


def main():
    json_url = os.environ.get("JSON_URL")
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output")
    if not json_url and len(sys.argv) > 1:
        json_url = sys.argv[1]
    if not json_url:
        print("ERROR: JSON_URL not provided", file=sys.stderr)
        sys.exit(2)

    rows = fetch_json(json_url)

    wb = build_workbook(rows)

    # IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    filename = f"testplan_{ts}.xlsx"

    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, filename)
    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
