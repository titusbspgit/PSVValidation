#!/usr/bin/env python3
import os, sys, json, datetime
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Columns per requirements
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

META_COLS = [
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


def load_json():
    # Prefer JSON from GitHub raw URL to avoid local path assumptions
    json_url = os.environ.get("JSON_URL", "").strip()
    fallback_path = os.environ.get("JSON_PATH", "Test_Output/GPIO/TestPlan/final_testplan.json")
    if json_url:
        try:
            req = Request(json_url, headers={"User-Agent": "excel-gen-action"})
            with urlopen(req) as resp:
                charset = resp.headers.get_content_charset() or "utf-8"
                raw = resp.read().decode(charset)
                data = json.loads(raw)
        except (URLError, HTTPError, json.JSONDecodeError) as e:
            print(f"ERROR: Failed to fetch/parse JSON from URL: {json_url}: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        try:
            with open(fallback_path, "r", encoding="utf-8") as f:
                data = json.load(f)
        except (OSError, json.JSONDecodeError) as e:
            print(f"ERROR: Failed to read/parse JSON from path: {fallback_path}: {e}", file=sys.stderr)
            sys.exit(1)

    if not isinstance(data, list):
        print("ERROR: json_data is not an array.", file=sys.stderr)
        sys.exit(1)
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            print(f"ERROR: Element at index {i} is not an object", file=sys.stderr)
            sys.exit(1)
    return data


def build_workbook(rows):
    wb = Workbook()
    # Remove default sheet to control order
    if wb.active:
        wb.remove(wb.active)

    ws = wb.create_sheet("TestPlan")
    ws_meta = wb.create_sheet("MetaData")

    bold = Font(bold=True)

    # Headers + freeze first row
    ws.append(TESTPLAN_COLS)
    for c in ws[1]:
        c.font = bold
    ws.freeze_panes = "A2"

    ws_meta.append(META_COLS)
    for c in ws_meta[1]:
        c.font = bold
    ws_meta.freeze_panes = "A2"

    # Rows preserving order and exact values
    for obj in rows:
        ws.append([obj.get(col, "") for col in TESTPLAN_COLS])
        ws_meta.append([obj.get(col, "") for col in META_COLS])

    # VeryHidden
    ws_meta.sheet_state = 'veryHidden'
    return wb


def main():
    rows = load_json()
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan").rstrip("/")
    os.makedirs(output_dir, exist_ok=True)

    # IST timestamped filename
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.datetime.now(tz=ist).strftime("%Y%m%d_%H%M%S")
    out_path = os.path.join(output_dir, f"testplan_{ts}.xlsx")

    wb = build_workbook(rows)
    wb.save(out_path)

    # Emit output path for GitHub Actions
    print(f"xlsx_path={out_path}")

if __name__ == "__main__":
    main()

# no-op trigger to retrigger workflow
