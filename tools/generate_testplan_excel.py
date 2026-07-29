#!/usr/bin/env python3
import argparse
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

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

HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")  # Blue
HEADER_FONT = Font(bold=True, color="FFFFFF")  # White text
CELL_ALIGN = Alignment(wrap_text=True, vertical="top")


def build_workbook(rows, ip_name: str, owner: str, repo: str, branch: str, output_dir: str):
    wb = Workbook()
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Write headers
    ws1.append(TESTPLAN_COLS)
    ws2.append(METADATA_COLS)

    for cell in ws1[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = CELL_ALIGN
    for cell in ws2[1]:
        cell.fill = HEADER_FILL
        cell.font = HEADER_FONT
        cell.alignment = CELL_ALIGN

    # IST timestamp for filename and metadata row
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(tz=ist)
    ts_str = ts.strftime("%Y%m%d_%H%M%S")

    # Optional top metadata row in MetaData sheet capturing workbook info
    meta_info = {
        "generated_ist": ts.isoformat(),
        "ip_name": ip_name,
        "source_repo": f"https://github.com/{owner}/{repo}",
        "branch": branch,
        "output_directory": output_dir,
        "row_count": len(rows),
    }
    meta_row = {
        "Index": "META",
        "Test Case Name": "WORKBOOK_INFO",
        "Meta Test Description": "",
        "Meta Test Steps / Procedure": "",
        "Meta Impacted Registers": "",
        "Meta Validation / Acceptance Criteria": "",
        "Meta Headers": json.dumps(meta_info, separators=(",", ":")),
        "Meta Macros": "",
        "Meta Arrays": "",
    }

    # Append the meta info row first
    ws2.append([meta_row.get(col, "") for col in METADATA_COLS])

    # Data rows preserving input order
    for obj in rows:
        # TestPlan sheet values
        ws1.append([obj.get(col, "") for col in TESTPLAN_COLS])
        # MetaData sheet values
        ws2.append([obj.get(col, "") for col in METADATA_COLS])

    # Freeze first row
    ws1.freeze_panes = "A2"
    ws2.freeze_panes = "A2"

    # Wrap text for all cells and compute column widths
    def autosize(ws):
        maxlen = {}
        for row in ws.iter_rows():
            for cell in row:
                cell.alignment = CELL_ALIGN
                v = cell.value
                if v is None:
                    l = 0
                else:
                    s = str(v)
                    l = max(len(line) for line in s.splitlines()) if s else 0
                col = cell.column_letter
                maxlen[col] = max(maxlen.get(col, 0), l)
        for col, l in maxlen.items():
            width = min(max(10, l + 2), 60)
            ws.column_dimensions[col].width = width

    autosize(ws1)
    autosize(ws2)

    # Very hide the MetaData sheet
    ws2.sheet_state = 'veryHidden'

    return wb, ts_str


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--input', required=True, help='Path to JSON file (array of test cases)')
    p.add_argument('--ip-name', required=True)
    p.add_argument('--owner', required=True)
    p.add_argument('--repo', required=True)
    p.add_argument('--branch', required=True)
    p.add_argument('--output-dir', required=True)
    args = p.parse_args()

    # Load and validate JSON
    with open(args.input, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise SystemExit('json_data must be a JSON array')

    wb, ts_str = build_workbook(
        data,
        ip_name=args.ip_name,
        owner=args.owner,
        repo=args.repo,
        branch=args.branch,
        output_dir=args.output_dir,
    )

    # File naming rule: <IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
    filename = f"{args.ip_name}_TestPlan_{ts_str}.xlsx"

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / filename

    wb.save(out_path)
    print(f"Saved Excel: {out_path}")

if __name__ == '__main__':
    main()
