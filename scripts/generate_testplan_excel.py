#!/usr/bin/env python3
import json
import os
import sys
from datetime import datetime, timezone, timedelta
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


def ist_now_str():
    tz = None
    if ZoneInfo is not None:
        try:
            tz = ZoneInfo("Asia/Kolkata")
        except Exception:
            tz = None
    if tz is None:
        tz = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(tz)
    return now.strftime("%Y%m%d_%H%M%S")


def apply_header_style(ws):
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(fill_type="solid", fgColor="1F4E78")  # dark blue
    align = Alignment(wrap_text=True, vertical="center")
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = align


def enable_wrap_all(ws):
    align = Alignment(wrap_text=True, vertical="top")
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.alignment = align


def autosize_columns(ws, max_width=120, min_width=12):
    for col_cells in ws.columns:
        max_len = 0
        column = col_cells[0].column_letter
        for c in col_cells:
            try:
                v = "" if c.value is None else str(c.value)
                lines = v.splitlines() if isinstance(v, str) else [str(v)]
                for ln in lines:
                    if len(ln) > max_len:
                        max_len = len(ln)
            except Exception:
                pass
        width = max(min_width, min(max_width, int(max_len * 1.1)))
        ws.column_dimensions[column].width = width


def write_rows(ws, columns, rows):
    # Header
    ws.append(columns)
    # Data rows in the same order as provided
    for obj in rows:
        ws.append([obj.get(col, "") for col in columns])
    # Freeze first row
    ws.freeze_panes = "A2"


def main():
    repo_root = os.getcwd()
    data_path = os.path.join(repo_root, "data", "testplan.json")
    if not os.path.exists(data_path):
        print(f"ERROR: JSON not found at {data_path}")
        sys.exit(1)

    with open(data_path, "r", encoding="utf-8") as f:
        try:
            data = json.load(f)
        except Exception as e:
            print(f"ERROR: Failed to parse JSON: {e}")
            sys.exit(1)

    # STEP 1 — Validate JSON
    if not isinstance(data, list):
        print("ERROR: json_data must be an array of objects")
        sys.exit(1)
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            print(f"ERROR: Item at index {i} is not an object")
            sys.exit(1)

    # Prepare workbook
    wb = Workbook()
    ws_test = wb.active
    ws_test.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # STEP 2 — Split & write
    write_rows(ws_test, TESTPLAN_COLUMNS, data)
    write_rows(ws_meta, METADATA_COLUMNS, data)

    # STEP 3 — Formatting
    apply_header_style(ws_test)
    apply_header_style(ws_meta)
    enable_wrap_all(ws_test)
    enable_wrap_all(ws_meta)
    autosize_columns(ws_test)
    autosize_columns(ws_meta)

    # VERY HIDDEN metadata sheet
    ws_meta.sheet_state = "veryHidden"

    # STEP 4 — Save file with IST timestamp
    ip_name = os.environ.get("IP_NAME", "USB").strip() or "USB"
    out_dir = os.environ.get("OUTPUT_DIR", "Test_Output/USB/TestPlan").strip().rstrip("/")
    os.makedirs(out_dir, exist_ok=True)
    ts = ist_now_str()
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(out_dir, filename)

    wb.save(out_path)

    # Emit output path for the workflow
    print(f"OUTPUT_FILE={out_path.replace('\\\\', '/').replace('\\', '/')}")


if __name__ == "__main__":
    main()
