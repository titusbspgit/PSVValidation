#!/usr/bin/env python3
import json
import argparse
from pathlib import Path
from datetime import datetime, timezone
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from openpyxl import Workbook
from openpyxl.styles import Font

TESTPLAN_COLS = [
    "Index","SS / Module","Feature","Test Case Name","Test Description",
    "Speed","Mode","Memory Start Offset","Memory End Offset",
    "Remarks","Test Steps / Procedure","Impacted Registers",
    "Validation / Acceptance Criteria","Code Generation (Required / Not)"
]

METADATA_COLS = [
    "Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure",
    "Meta Impacted Registers","Meta Validation / Acceptance Criteria",
    "Meta Headers","Meta Macros","Meta Arrays"
]

def ist_timestamp():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        now = datetime.now(tz)
    else:
        # Fallback: compute IST from UTC offset +5:30
        now = datetime.now(timezone.utc)  # UTC
        from datetime import timedelta
        now = now + timedelta(hours=5, minutes=30)
    return now.strftime("%Y%m%d_%H%M%S"), now


def flatten_row(obj, cols):
    return [obj.get(k, "") for k in cols]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True, help="Path to JSON array file")
    ap.add_argument("--output-dir", required=True, help="Directory to write .xlsx into")
    ap.add_argument("--owner", required=True)
    ap.add_argument("--repo", required=True)
    ap.add_argument("--branch", required=True)
    args = ap.parse_args()

    in_path = Path(args.input)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    data = json.loads(in_path.read_text(encoding="utf-8"))
    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    ts_str, ist_dt = ist_timestamp()
    # Also compute UTC timestamp string for metadata
    utc_now = datetime.now(timezone.utc)

    wb = Workbook()
    # Rename default sheet to TestPlan
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Header
    ws_tp.append(TESTPLAN_COLS)
    for cell in ws_tp[1]:
        cell.font = Font(bold=True)
    ws_tp.freeze_panes = "A2"

    # Rows
    for obj in data:
        ws_tp.append(flatten_row(obj, TESTPLAN_COLS))

    # MetaData sheet (very hidden)
    ws_md = wb.create_sheet("MetaData")
    # Generation metadata section (key/value)
    gen_meta = [
        ("GeneratedAtIST", ist_dt.strftime("%Y-%m-%d %H:%M:%S %Z")),
        ("GeneratedAtUTC", utc_now.strftime("%Y-%m-%d %H:%M:%S %Z")),
        ("SourceOwner", args.owner),
        ("SourceRepo", args.repo),
        ("SourceBranch", args.branch),
        ("OutputDirectory", str(out_dir)),
        ("ItemCount", str(len(data)))
    ]
    ws_md.append(["Key","Value"])  # header for metadata block
    for cell in ws_md[1]:
        cell.font = Font(bold=True)
    for k, v in gen_meta:
        ws_md.append([k, v])
    ws_md.append([""])  # blank line

    # Now the per-test meta table
    ws_md.append(METADATA_COLS)
    for cell in ws_md[ws_md.max_row]:
        cell.font = Font(bold=True)
    start_row = ws_md.max_row
    for obj in data:
        ws_md.append(flatten_row(obj, METADATA_COLS))
    ws_md.freeze_panes = f"A{start_row+1}"

    # Mark very hidden
    ws_md.sheet_state = 'veryHidden'

    # Save
    out_file = out_dir / f"testplan_{ts_str}.xlsx"
    wb.save(out_file)
    print(f"Wrote: {out_file}")

if __name__ == "__main__":
    main()
