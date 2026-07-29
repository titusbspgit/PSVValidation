#!/usr/bin/env python3
import json
import os
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    from backports.zoneinfo import ZoneInfo  # type: ignore

from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment

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


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be an array of objects")
    for i, item in enumerate(data):
        if not isinstance(item, dict):
            raise ValueError(f"Each item must be an object; bad item at index {i}")
    return data


def apply_header_style(ws):
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill("solid", fgColor="1F4E78")  # dark blue
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = Alignment(wrap_text=True, vertical="top")
    ws.freeze_panes = "A2"


def apply_body_style_and_widths(ws):
    # Wrap text for all data cells and compute column widths
    max_width = {}
    for row in ws.iter_rows(min_row=1, values_only=False):
        for cell in row:
            # wrap and top-align
            cell.alignment = Alignment(wrap_text=True, vertical="top")
            val = cell.value
            s = str(val) if val is not None else ""
            l = len(s)
            col = cell.column
            if col not in max_width or l > max_width[col]:
                max_width[col] = l
    for col_idx, width in max_width.items():
        # Add padding and clamp to reasonable limits
        adj = min(max(width + 4, 12), 90)
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = adj


def write_sheet(ws, columns, rows):
    ws.append(columns)
    for r in rows:
        ws.append([r.get(col, "") for col in columns])
    apply_header_style(ws)
    apply_body_style_and_widths(ws)


def build_rows(data):
    # Ensure row order preserved and fields mapped exactly
    testplan_rows = []
    metadata_rows = []
    for obj in data:
        # Map for TestPlan
        testplan_rows.append({
            "Index": obj.get("Index", ""),
            "SS / Module": obj.get("SS / Module", ""),
            "Feature": obj.get("Feature", ""),
            "Test Case Name": obj.get("Test Case Name", ""),
            "Test Description": obj.get("Test Description", ""),
            "Speed": obj.get("Speed", ""),
            "Mode": obj.get("Mode", ""),
            "Memory Start Offset": obj.get("Memory Start Offset", ""),
            "Memory End Offset": obj.get("Memory End Offset", ""),
            "Remarks": obj.get("Remarks", ""),
            "Test Steps / Procedure": obj.get("Test Steps / Procedure", ""),
            "Impacted Registers": obj.get("Impacted Registers", ""),
            "Validation / Acceptance Criteria": obj.get("Validation / Acceptance Criteria", ""),
            "Code Generation (Required / Not)": obj.get("Code Generation (Required / Not)", ""),
        })
        # Map for MetaData
        metadata_rows.append({
            "Index": obj.get("Index", ""),
            "Test Case Name": obj.get("Test Case Name", ""),
            "Meta Test Description": obj.get("Meta Test Description", ""),
            "Meta Test Steps / Procedure": obj.get("Meta Test Steps / Procedure", ""),
            "Meta Impacted Registers": obj.get("Meta Impacted Registers", ""),
            "Meta Validation / Acceptance Criteria": obj.get("Meta Validation / Acceptance Criteria", ""),
            "Meta Headers": obj.get("Meta Headers", ""),
            "Meta Macros": obj.get("Meta Macros", ""),
            "Meta Arrays": obj.get("Meta Arrays", ""),
        })
    return testplan_rows, metadata_rows


def main():
    repo_root = Path(os.getenv("GITHUB_WORKSPACE", ".")).resolve()
    data_path = repo_root / "data" / "final_json.json"
    output_dir = repo_root / "Test_Output" / "GPIO" / "TestPlan"

    data = load_json(data_path)
    testplan_rows, metadata_rows = build_rows(data)

    wb = Workbook()
    # Create visible TestPlan first
    ws1 = wb.active
    ws1.title = "TestPlan"
    write_sheet(ws1, TESTPLAN_COLUMNS, testplan_rows)

    # Create MetaData and make it veryHidden
    ws2 = wb.create_sheet(title="MetaData")
    write_sheet(ws2, METADATA_COLUMNS, metadata_rows)
    ws2.sheet_state = "veryHidden"

    # Filename with IST timestamp and IP_NAME prefix per spec
    tz = ZoneInfo("Asia/Kolkata")
    now_ist = datetime.now(tz)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"GPIO_TestPlan_{ts}.xlsx"

    output_dir.mkdir(parents=True, exist_ok=True)
    out_path = output_dir / filename
    wb.save(out_path)

    # Persist path for subsequent steps if needed
    (repo_root / "generated_excel_path.txt").write_text(str(out_path.relative_to(repo_root)), encoding="utf-8")
    print(f"Wrote Excel to: {out_path}")


if __name__ == "__main__":
    main()
