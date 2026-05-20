#!/usr/bin/env python3
import json
import os
from urllib.request import urlopen, Request
from datetime import datetime, timezone, timedelta
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


def ist_timestamp():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime("%Y%m%d_%H%M%S")


def fetch_json(json_url: str):
    req = Request(json_url, headers={"User-Agent": "xlsx-generator/1.0"})
    with urlopen(req, timeout=60) as resp:
        data = resp.read().decode("utf-8")
        return json.loads(data)


def write_sheet(ws, headers, rows):
    # headers
    bold = Font(bold=True)
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = bold
    ws.freeze_panes = "A2"

    # rows
    r = 2
    for row in rows:
        for c, key in enumerate(headers, start=1):
            val = row.get(key, "")
            ws.cell(row=r, column=c, value=val)
        r += 1


def build_workbook(data):
    # Ensure list of dicts
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array")

    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Prepare two aligned views preserving row order
    plan_rows = []
    meta_rows = []
    for item in data:
        if not isinstance(item, dict):
            raise ValueError("Each JSON array element must be an object (row)")
        plan_row = {k: item.get(k, "") for k in TESTPLAN_COLS}
        meta_row = {k: item.get(k, "") for k in METADATA_COLS}
        plan_rows.append(plan_row)
        meta_rows.append(meta_row)

    write_sheet(ws_plan, TESTPLAN_COLS, plan_rows)
    write_sheet(ws_meta, METADATA_COLS, meta_rows)

    # VeryHidden MetaData
    ws_meta.sheet_state = "veryHidden"

    return wb


def main():
    json_url = os.environ.get("JSON_URL") or (len(os.sys.argv) > 1 and os.sys.argv[1])
    output_dir = os.environ.get("OUTPUT_DIR") or (len(os.sys.argv) > 2 and os.sys.argv[2]) or "Test_Output/GPIO/TestPlan"
    if not json_url:
        raise SystemExit("JSON_URL must be provided via env or argv[1]")

    data = fetch_json(json_url)
    wb = build_workbook(data)

    ts = ist_timestamp()
    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, f"testplan_{ts}.xlsx")
    wb.save(out_path)
    print(f"Saved Excel: {out_path}")


if __name__ == "__main__":
    main()
