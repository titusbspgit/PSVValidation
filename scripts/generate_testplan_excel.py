#!/usr/bin/env python3
import argparse
import json
import sys
from pathlib import Path
from datetime import datetime
from zoneinfo import ZoneInfo
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

def load_json(path: Path):
    try:
        with path.open('r', encoding='utf-8') as f:
            data = json.load(f)
    except Exception as e:
        print(f"ERROR: Failed to load JSON from {path}: {e}", file=sys.stderr)
        sys.exit(2)
    if not isinstance(data, list) or len(data) == 0:
        print("ERROR: json_data must be a non-empty array", file=sys.stderr)
        sys.exit(2)
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            print(f"ERROR: Element at index {i} is not an object", file=sys.stderr)
            sys.exit(2)
    return data


def write_sheet(ws, columns, rows):
    # Header
    ws.append(columns)
    # Bold header and freeze
    for cell in ws[1]:
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"
    # Rows
    for obj in rows:
        row = [obj.get(col, "") for col in columns]
        ws.append(row)


def make_workbook(json_rows):
    wb = Workbook()
    # Default sheet becomes TestPlan
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    write_sheet(ws_plan, TESTPLAN_COLS, json_rows)
    # MetaData sheet
    ws_meta = wb.create_sheet("MetaData")
    write_sheet(ws_meta, METADATA_COLS, json_rows)
    # VeryHidden MetaData
    ws_meta.sheet_state = 'veryHidden'
    return wb


def ensure_unique_path(base_dir: Path, base_name: str) -> Path:
    p = base_dir / base_name
    if not p.exists():
        return p
    stem, ext = base_name.rsplit('.', 1)
    n = 2
    while True:
        cand = base_dir / f"{stem}__{n}.{ext}"
        if not cand.exists():
            return cand
        n += 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True, help='Path to input JSON file')
    ap.add_argument('--output-dir', required=True, help='Directory to place Excel file (repo-relative)')
    ap.add_argument('--ip-name', required=False, help='IP name (unused in structure, kept for traceability)')
    ap.add_argument('--out-flag-file', default='testplan_output_path.txt', help='Where to write the relative output path')
    args = ap.parse_args()

    json_path = Path(args.json)
    out_dir = Path(args.output_dir)

    rows = load_json(json_path)

    # Construct filename with IST timestamp
    ist = ZoneInfo('Asia/Kolkata')
    ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    filename = f"testplan_{ts}.xlsx"

    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = ensure_unique_path(out_dir, filename)

    wb = make_workbook(rows)
    wb.save(out_path)

    # Write relative path for the workflow commit step
    flag_path = Path(args.out_flag_file)
    flag_path.write_text(str(out_path), encoding='utf-8')
    print(f"Wrote Excel to {out_path}")

if __name__ == '__main__':
    main()
