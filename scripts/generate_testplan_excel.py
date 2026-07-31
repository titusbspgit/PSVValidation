#!/usr/bin/env python3
import argparse
import json
import os
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

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

BLUE_FILL = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")
WHITE_BOLD = Font(bold=True, color="FFFFFF")
WRAP_TOP = Alignment(wrap_text=True, vertical="top")


def compute_ist_timestamp():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        now = datetime.now(tz)
    else:
        # Fallback to naive localtime offset of IST (+05:30)
        from datetime import timezone, timedelta
        tz = timezone(timedelta(hours=5, minutes=30))
        now = datetime.now(tz)
    return now.strftime("%Y%m%d_%H%M%S")


def auto_size_columns(ws):
    # Determine max length per column and set width (bounded)
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            val = cell.value
            if val is None:
                continue
            l = len(str(val))
            if l > max_len:
                max_len = l
        # Heuristic: broader for text-heavy columns
        width = min(max(12, max_len + 2), 90)
        ws.column_dimensions[col[0].column_letter].width = width


def apply_header_style(ws):
    for cell in ws[1]:
        cell.font = WHITE_BOLD
        cell.fill = BLUE_FILL
        cell.alignment = WRAP_TOP


def apply_wrap_all(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.alignment = WRAP_TOP


def build_workbook(data, ip_name: str, output_dir: Path) -> Path:
    ts = compute_ist_timestamp()
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = output_dir / filename

    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet(title="MetaData")

    # Headers
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)

    # Rows
    for row in data:
        ws_plan.append([row.get(col, "") for col in TESTPLAN_COLUMNS])
        ws_meta.append([
            row.get("Index", ""),
            row.get("Test Case Name", ""),
            row.get("Meta Test Description", ""),
            row.get("Meta Test Steps / Procedure", ""),
            row.get("Meta Impacted Registers", ""),
            row.get("Meta Validation / Acceptance Criteria", ""),
            row.get("Meta Headers", ""),
            row.get("Meta Macros", ""),
            row.get("Meta Arrays", ""),
        ])

    # Extra generation metadata rows (after data) in MetaData sheet
    ws_meta.append([])
    ws_meta.append([
        "meta",
        "",
        "",
        "",
        "",
        "",
        f"Generation Timestamp (IST): {ts}",
        f"Source: titusbspgit/PSVValidation@main | Path: Test_Output/USB/TestPlan/ | IP_NAME: {ip_name}",
        f"Folder Count: 2 | Rows: {len(data)}",
    ])

    # Formatting
    apply_header_style(ws_plan)
    apply_header_style(ws_meta)
    apply_wrap_all(ws_plan)
    apply_wrap_all(ws_meta)

    # Freeze top row
    ws_plan.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Auto column widths
    auto_size_columns(ws_plan)
    auto_size_columns(ws_meta)

    # Very hide metadata sheet
    ws_meta.sheet_state = 'veryHidden'

    # Ensure directory and save
    out_path.parent.mkdir(parents=True, exist_ok=True)
    wb.save(out_path)

    return out_path, ts


def main():
    parser = argparse.ArgumentParser(description="Generate TestPlan Excel from JSON")
    parser.add_argument("--json", required=True, help="Path to input JSON array file")
    parser.add_argument("--ip-name", required=True, help="IP name for filename prefix")
    parser.add_argument("--output-dir", required=True, help="Output directory for Excel file")
    args = parser.parse_args()

    data_path = Path(args.json)
    output_dir = Path(args.output_dir)

    with data_path.open("r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    out_path, ts = build_workbook(data, args.ip_name, output_dir)

    # Emit outputs for GitHub Actions
    gh_output = os.environ.get("GITHUB_OUTPUT")
    if gh_output:
        with open(gh_output, "a", encoding="utf-8") as f:
            f.write(f"artifact_path={out_path.as_posix()}\n")
            f.write(f"timestamp={ts}\n")

    print(f"Generated: {out_path}")


if __name__ == "__main__":
    main()
