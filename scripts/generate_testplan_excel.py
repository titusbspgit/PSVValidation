#!/usr/bin/env python3
import json
import sys
import os
from datetime import datetime, timedelta, timezone
from typing import List, Dict, Any

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except Exception as e:
    print(f"ERROR: openpyxl is required. Install with: pip install openpyxl\nDetails: {e}", file=sys.stderr)
    sys.exit(1)

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
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

def load_json(path: str) -> List[Dict[str, Any]]:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be an array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each element in json_data must be an object. Found type at index {i}: {type(row)}")
    return data


def write_sheet(ws, columns: List[str], rows: List[Dict[str, Any]]):
    # Header
    for c_idx, col_name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=c_idx, value=col_name)
        cell.font = Font(bold=True)
    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, col_name in enumerate(columns, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(col_name, ""))
    # Freeze header
    ws.freeze_panes = "A2"


def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def build_row(mapping: List[str], src: Dict[str, Any]) -> Dict[str, Any]:
    return {k: src.get(k, "") for k in mapping}


def ist_now_timestamp() -> str:
    # IST is UTC+05:30
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(tz=ist)
    return now_ist.strftime("%Y%m%d_%H%M%S")


def main():
    import argparse
    p = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    p.add_argument("--input", required=True, help="Path to input JSON array file")
    p.add_argument("--output-dir", required=True, help="Directory to place generated Excel")
    args = p.parse_args()

    data = load_json(args.input)

    # Prepare workbook
    wb = Workbook()

    # TestPlan sheet (visible)
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    tp_rows = [build_row(TESTPLAN_COLUMNS, r) for r in data]
    write_sheet(ws_tp, TESTPLAN_COLUMNS, tp_rows)

    # MetaData sheet (VERY HIDDEN)
    ws_md = wb.create_sheet("MetaData")
    md_rows = [build_row(METADATA_COLUMNS, r) for r in data]
    write_sheet(ws_md, METADATA_COLUMNS, md_rows)
    ws_md.sheet_state = 'veryHidden'

    ensure_dir(args.output_dir)
    ts = ist_now_timestamp()
    out_path = os.path.join(args.output_dir, f"testplan_{ts}.xlsx")
    wb.save(out_path)

    # Print the output path for downstream steps
    print(out_path)

if __name__ == "__main__":
    main()
