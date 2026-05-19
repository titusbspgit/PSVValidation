#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime, timezone
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from openpyxl import Workbook
from openpyxl.styles import Font


def build_parser():
    p = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    p.add_argument("--json-file", required=True)
    p.add_argument("--output-dir", required=True)
    p.add_argument("--source-owner", required=True)
    p.add_argument("--source-repo", required=True)
    p.add_argument("--source-branch", required=True)
    p.add_argument("--source-subdir", required=True)
    p.add_argument("--ip-name", required=True)
    return p


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("Input JSON must be a list (array) of objects")
    return data


def ensure_dir(d):
    os.makedirs(d, exist_ok=True)


def to_ist_timestamp():
    # Compute IST timestamp for filename (YYYYMMDD_HHMMSS)
    if ZoneInfo is not None:
        ist = ZoneInfo("Asia/Kolkata")
        now_ist = datetime.now(ist)
    else:
        # Fallback: add +5:30 offset to UTC
        now_utc = datetime.now(timezone.utc)
        now_ist = now_utc.astimezone(timezone.utc)
    return now_ist.strftime("%Y%m%d_%H%M%S")


def to_iso8601():
    # ISO8601 in UTC with Z
    return datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def apply_header_format(ws, header_row=1):
    bold = Font(bold=True)
    for cell in ws[header_row]:
        cell.font = bold


def write_testplan_sheet(wb, data):
    ws = wb.active
    ws.title = "TestPlan"
    headers = [
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
    ws.append(headers)
    for obj in data:
        row = [
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
        ws.append(row)
    apply_header_format(ws, 1)
    ws.freeze_panes = "A2"
    return ws


def write_metadata_sheet(wb, data, meta_info):
    ws = wb.create_sheet("MetaData")
    # Key/Value block
    ws.append(["Key", "Value"])
    apply_header_format(ws, 1)
    for k, v in meta_info.items():
        ws.append([k, v])
    ws.append([])  # blank separator
    # Per-test meta table
    meta_headers = [
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
    ws.append(meta_headers)
    apply_header_format(ws, ws.max_row)
    for obj in data:
        row = [
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
        ws.append(row)
    ws.freeze_panes = "A2"  # freeze first row of the sheet
    # Make VeryHidden
    ws.sheet_state = 'veryHidden'
    return ws


def main():
    args = build_parser().parse_args()
    data = load_json(args.json_file)
    if not data:
        raise ValueError("Input JSON array is empty")

    wb = Workbook()
    write_testplan_sheet(wb, data)

    ist_ts = to_ist_timestamp()
    iso_ts = to_iso8601()

    meta_info = {
        "source_owner": args.source_owner,
        "source_repo": args.source_repo,
        "source_branch": args.source_branch,
        "source_sub_directory": args.source_subdir,
        "ip_name": args.ip_name,
        "generated_timestamp": iso_ts,
        "total_items": str(len(data)),
    }

    write_metadata_sheet(wb, data, meta_info)

    ensure_dir(args.output_dir)
    out_path = os.path.join(args.output_dir, f"testplan_{ist_ts}.xlsx")
    wb.save(out_path)
    # Print path for CI logs
    print(out_path)


if __name__ == "__main__":
    main()
