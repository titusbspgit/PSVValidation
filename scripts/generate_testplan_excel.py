#!/usr/bin/env python3
import os, sys, json
from datetime import datetime, timedelta

# Try to use IANA tz; fall back to fixed +05:30 if unavailable
try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:
    ZoneInfo = None

from openpyxl import Workbook
from openpyxl.styles import Font


def get_ist_timestamp():
    try:
        if ZoneInfo is not None:
            tz = ZoneInfo("Asia/Kolkata")
            now = datetime.now(tz)
        else:
            raise Exception("ZoneInfo not available")
    except Exception:
        # Fallback: UTC + 5:30 without DST
        now = datetime.utcnow() + timedelta(hours=5, minutes=30)
    return now.strftime("%Y%m%d_%H%M%S")


TESTPLAN_HEADERS = [
    "Index","SS / Module","Feature","Test Case Name","Test Description",
    "Speed","Mode","Memory Start Offset","Memory End Offset","Remarks",
    "Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria",
    "Code Generation (Required / Not)"
]

METADATA_HEADERS = [
    "Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure",
    "Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers",
    "Meta Macros","Meta Arrays"
]


def validate_json(data):
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Element at index {i} is not an object/dict")


def write_sheet(ws, headers, rows):
    # Header
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"
    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            val = row.get(h, "")
            if val is None:
                val = ""
            ws.cell(row=r_idx, column=c_idx, value=str(val))


def main():
    # Read JSON payload from env or file path argument
    json_payload = os.environ.get("JSON_PAYLOAD", None)
    # Allow first arg to be a file path if provided (and not the --outdir switch)
    if not json_payload and len(sys.argv) > 1 and sys.argv[1] != "--outdir":
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            json_payload = f.read()

    # Parse args
    outdir = "Test_Output/GPIO/TestCode"
    if "--outdir" in sys.argv:
        i = sys.argv.index("--outdir")
        if i + 1 < len(sys.argv):
            outdir = sys.argv[i + 1]

    if not json_payload:
        print("ERROR: No JSON payload provided. Set JSON_PAYLOAD env var or pass a file path.", file=sys.stderr)
        sys.exit(1)

    try:
        data = json.loads(json_payload)
    except Exception as e:
        print(f"ERROR: Failed to parse JSON: {e}", file=sys.stderr)
        sys.exit(1)

    try:
        validate_json(data)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)

    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    write_sheet(ws_plan, TESTPLAN_HEADERS, data)
    write_sheet(ws_meta, METADATA_HEADERS, data)

    # Very hidden MetaData
    ws_meta.sheet_state = "veryHidden"

    # Ensure output directory exists
    os.makedirs(outdir, exist_ok=True)

    # Filename with IST timestamp
    ts = get_ist_timestamp()
    outfile = os.path.join(outdir, f"testplan_{ts}.xlsx")

    wb.save(outfile)
    print(outfile)


if __name__ == "__main__":
    main()
