#!/usr/bin/env python3
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment

# Columns per requirements
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
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]


def load_json(json_path: str):
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a JSON array")
    return data


def build_sheet(ws, columns, rows):
    # Write header
    for col_idx, name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=col_idx, value=name)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(wrap_text=True, vertical="top")
    # Write rows
    for r_idx, item in enumerate(rows, start=2):
        for c_idx, key in enumerate(columns, start=1):
            val = item.get(key, "")
            ws.cell(row=r_idx, column=c_idx, value=val)
    ws.freeze_panes = "A2"


def main():
    # Inputs via environment with sane defaults
    repo_root = Path(os.getenv("GITHUB_WORKSPACE", ".")).resolve()
    input_path = Path(os.getenv("TP_INPUT_JSON", repo_root / "TestOutput/PCIE/TestPlan/input/testplan_input.json")).resolve()
    output_dir = Path(os.getenv("TP_OUTPUT_DIR", repo_root / "TestOutput/PCIE/TestPlan")).resolve()

    data = load_json(str(input_path))

    # Prepare rows for each sheet mapping keys exactly
    testplan_rows = []
    metadata_rows = []
    for obj in data:
        # Ensure dict
        if not isinstance(obj, dict):
            raise ValueError("Each item in json_data must be an object")
        # Build dicts limited to required columns, preserving values
        t_row = {k: (obj.get(k, "") if obj.get(k) is not None else "") for k in TESTPLAN_COLUMNS}
        m_row = {k: (obj.get(k, "") if obj.get(k) is not None else "") for k in METADATA_COLUMNS}
        testplan_rows.append(t_row)
        metadata_rows.append(m_row)

    # Create workbook
    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    build_sheet(ws_tp, TESTPLAN_COLUMNS, testplan_rows)

    ws_meta = wb.create_sheet("MetaData")
    build_sheet(ws_meta, METADATA_COLUMNS, metadata_rows)

    # VERY HIDDEN metadata sheet
    ws_meta.sheet_state = "veryHidden"

    # Timestamp in IST
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")

    output_dir.mkdir(parents=True, exist_ok=True)
    out_path = output_dir / f"testplan_{ts}.xlsx"

    # Save real .xlsx
    wb.save(out_path)

    # Emit the output path for the workflow to pick up (optional)
    print(f"EXCEL_OUTPUT={out_path}")


if __name__ == "__main__":
    main()
