#!/usr/bin/env python3
import os
import sys
import json
from pathlib import Path
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment

TESTPLAN_HEADERS = [
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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

METADATA_HEADERS = [
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

BLUE = "4472C4"
WHITE = "FFFFFF"


def style_header(ws):
    for col_idx, title in enumerate((TESTPLAN_HEADERS if ws.title == "TestPlan" else METADATA_HEADERS), start=1):
        c = ws.cell(row=1, column=col_idx, value=title)
        c.font = Font(bold=True, color=WHITE)
        c.fill = PatternFill("solid", fgColor=BLUE)
        c.alignment = Alignment(wrap_text=True, vertical="top")
    ws.freeze_panes = "A2"


def apply_filters(ws):
    # Apply auto filter across used range if any data exists
    if ws.max_row >= 1 and ws.max_column >= 1:
        ws.auto_filter.ref = f"A1:{chr(64+ws.max_column)}{ws.max_row}"


def style_data(ws):
    for r in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for c in r:
            c.alignment = Alignment(wrap_text=True, vertical="top")


def set_column_widths(ws):
    if ws.title == "TestPlan":
        widths = [8, 18, 28, 28, 70, 10, 12, 18, 18, 40, 80, 70, 70, 26]
    else:
        widths = [8, 28, 80, 80, 80, 80, 50, 40, 60]
    for i, w in enumerate(widths, start=1):
        col_letter = ws.cell(row=1, column=i).column_letter
        ws.column_dimensions[col_letter].width = w


def validate_json_array(s):
    try:
        data = json.loads(s)
    except Exception as e:
        print(f"ERROR: Failed to parse JSON: {e}", file=sys.stderr)
        sys.exit(2)
    if not isinstance(data, list):
        print("ERROR: json_data must be a JSON array", file=sys.stderr)
        sys.exit(2)
    return data


def build_workbook(data):
    wb = Workbook()
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Header rows
    style_header(ws1)
    style_header(ws2)

    # Map JSON rows to both sheets without mutation or reordering
    def gv(obj, key):
        v = obj.get(key, "")
        if v is None:
            return ""
        return v

    # Write TestPlan sheet
    for idx, obj in enumerate(data, start=2):
        row_vals = [
            gv(obj, "Index"),
            gv(obj, "SS / Module"),
            gv(obj, "Feature"),
            gv(obj, "Test Case Name"),
            gv(obj, "Test Description"),
            gv(obj, "Speed"),
            gv(obj, "Mode"),
            gv(obj, "Memory Start Offset"),
            gv(obj, "Memory End Offset"),
            gv(obj, "Remarks"),
            gv(obj, "Test Steps / Procedure"),
            gv(obj, "Impacted Registers"),
            gv(obj, "Validation / Acceptance Criteria"),
            gv(obj, "Code Generation (Required / Not)"),
        ]
        for c_idx, val in enumerate(row_vals, start=1):
            ws1.cell(row=idx, column=c_idx, value=val)

    # Write MetaData sheet
    for idx, obj in enumerate(data, start=2):
        row_vals = [
            gv(obj, "Index"),
            gv(obj, "Test Case Name"),
            gv(obj, "Meta Test Description"),
            gv(obj, "Meta Test Steps / Procedure"),
            gv(obj, "Meta Impacted Registers"),
            gv(obj, "Meta Validation / Acceptance Criteria"),
            gv(obj, "Meta Headers"),
            gv(obj, "Meta Macros"),
            gv(obj, "Meta Arrays"),
        ]
        for c_idx, val in enumerate(row_vals, start=1):
            ws2.cell(row=idx, column=c_idx, value=val)

    # Formatting
    apply_filters(ws1)
    apply_filters(ws2)
    style_data(ws1)
    style_data(ws2)
    set_column_widths(ws1)
    set_column_widths(ws2)

    # VeryHidden for MetaData
    ws2.sheet_state = "veryHidden"

    return wb


def get_ist_timestamp():
    if ZoneInfo is not None:
        tz = ZoneInfo("Asia/Kolkata")
        now = datetime.now(tz)
    else:
        # Fallback: manual offset +05:30
        now = datetime.utcnow()
        # naive adjustment
        from datetime import timedelta
        now = now + timedelta(hours=5, minutes=30)
    return now.strftime("%Y%m%d_%H%M%S")


def main():
    json_raw = os.environ.get("JSON_RAW", "")
    if not json_raw:
        print("ERROR: JSON_RAW environment variable is empty", file=sys.stderr)
        sys.exit(2)
    data = validate_json_array(json_raw)

    ip_name = os.environ.get("IP_NAME", "IP")
    out_dir = os.environ.get("OUTPUT_DIR", "Test_Output")

    ts = get_ist_timestamp()
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = Path(out_dir)
    out_path.mkdir(parents=True, exist_ok=True)
    full_path = out_path / filename

    wb = build_workbook(data)
    wb.save(str(full_path))

    # Reopen to validate
    try:
        wb2 = load_workbook(str(full_path))
        sheets = wb2.sheetnames
        assert "TestPlan" in sheets and "MetaData" in sheets
        md_state = wb2["MetaData"].sheet_state
        assert md_state == "veryHidden"
        # Validate row counts (header + rows)
        testplan_rows = wb2["TestPlan"].max_row - 1
        metadata_rows = wb2["MetaData"].max_row - 1
        assert testplan_rows == len(data)
        assert metadata_rows == len(data)
    except Exception as e:
        print(f"ERROR: Validation failed: {e}", file=sys.stderr)
        sys.exit(3)

    # Output path for GitHub Actions
    print(f"EXCEL_OUTPUT_PATH={full_path}")
    gh_out = os.environ.get("GITHUB_OUTPUT")
    if gh_out:
        with open(gh_out, "a", encoding="utf-8") as f:
            f.write(f"excel_output_path={full_path}\n")


if __name__ == "__main__":
    main()
