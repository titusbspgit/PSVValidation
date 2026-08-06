import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

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
    "Impacted Registers",
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

def style_header(ws):
    header_font = Font(bold=True, color="FFFFFFFF")
    header_fill = PatternFill("solid", fgColor="4472C4")
    wrap = Alignment(wrap_text=True, vertical="top")
    for col, _ in enumerate(ws[1], start=1):
        cell = ws.cell(row=1, column=col)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap
    ws.freeze_panes = "A2"


def style_data(ws, max_col, max_row):
    wrap = Alignment(wrap_text=True, vertical="top")
    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            ws.cell(row=r, column=c).alignment = wrap


def set_column_widths(ws, widths):
    for i, w in enumerate(widths, start=1):
        ws.column_dimensions[get_column_letter(i)].width = w


def validate_workbook(path, expected_rows):
    wb2 = load_workbook(path)
    if "TestPlan" not in wb2.sheetnames or "MetaData" not in wb2.sheetnames:
        raise RuntimeError("Required worksheets missing")
    ws_tp = wb2["TestPlan"]
    ws_md = wb2["MetaData"]
    # Validate headers
    tp_headers = [c.value for c in ws_tp[1]]
    md_headers = [c.value for c in ws_md[1]]
    if tp_headers != TESTPLAN_HEADERS:
        raise RuntimeError("TestPlan headers mismatch")
    if md_headers != METADATA_HEADERS:
        raise RuntimeError("MetaData headers mismatch")
    # VeryHidden check
    if ws_md.sheet_state != "veryHidden":
        raise RuntimeError("MetaData sheet is not VeryHidden")
    # Row count (excluding header)
    tp_rows = ws_tp.max_row - 1 if ws_tp.max_row else 0
    md_rows = ws_md.max_row - 1 if ws_md.max_row else 0
    if tp_rows != expected_rows or md_rows != expected_rows:
        raise RuntimeError(f"Row count mismatch: tp={tp_rows}, md={md_rows}, expected={expected_rows}")


def main():
    # Inputs via env
    json_input_path = os.getenv("JSON_INPUT_PATH", "scripts/testplan_input.json")
    output_dir = os.getenv("OUTPUT_DIR", "Test_Output/GPIO/TestPlan/")
    ip_name = os.getenv("IP_NAME", "GPIO")
    tz_name = os.getenv("TIMEZONE", "Asia/Kolkata")
    branch = os.getenv("BRANCH", "main")
    commit_message = os.getenv("COMMIT_MESSAGE", "Added generated TestPlan Excel")

    # Load JSON
    with open(json_input_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise SystemExit("json_data must be a JSON array")

    # Timestamp in IST (or provided tz)
    now = datetime.now(ZoneInfo(tz_name))
    ts = now.strftime("%Y%m%d_%H%M%S")
    file_name = f"{ip_name}_TestPlan_{ts}.xlsx"

    # Create workbook
    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_md = wb.create_sheet("MetaData")

    # Write headers
    for col, h in enumerate(TESTPLAN_HEADERS, start=1):
        ws_tp.cell(row=1, column=col, value=h)
    for col, h in enumerate(METADATA_HEADERS, start=1):
        ws_md.cell(row=1, column=col, value=h)

    # Map rows preserving order and exact values
    for i, obj in enumerate(data, start=2):
        # TestPlan row
        ws_tp.cell(row=i, column=1, value=obj.get("Index", ""))
        ws_tp.cell(row=i, column=2, value=obj.get("SS / Module", ""))
        ws_tp.cell(row=i, column=3, value=obj.get("Feature", ""))
        ws_tp.cell(row=i, column=4, value=obj.get("Test Case Name", ""))
        ws_tp.cell(row=i, column=5, value=obj.get("Test Description", ""))
        ws_tp.cell(row=i, column=6, value=obj.get("Speed", ""))
        ws_tp.cell(row=i, column=7, value=obj.get("Mode", ""))
        ws_tp.cell(row=i, column=8, value=obj.get("Memory Start Offset", ""))
        ws_tp.cell(row=i, column=9, value=obj.get("Memory End Offset", ""))
        ws_tp.cell(row=i, column=10, value=obj.get("Remarks", ""))
        ws_tp.cell(row=i, column=11, value=obj.get("Test Steps / Procedure", ""))
        ws_tp.cell(row=i, column=12, value=obj.get("Impacted Registers", ""))
        ws_tp.cell(row=i, column=13, value=obj.get("Validation / Acceptance Criteria", ""))
        ws_tp.cell(row=i, column=14, value=obj.get("Code Generation (Required / Not)", ""))

        # MetaData row (aligned)
        ws_md.cell(row=i, column=1, value=obj.get("Index", ""))
        ws_md.cell(row=i, column=2, value=obj.get("Test Case Name", ""))
        ws_md.cell(row=i, column=3, value=obj.get("Meta Test Description", ""))
        ws_md.cell(row=i, column=4, value=obj.get("Meta Test Steps / Procedure", ""))
        ws_md.cell(row=i, column=5, value=obj.get("Meta Impacted Registers", ""))
        ws_md.cell(row=i, column=6, value=obj.get("Meta Validation / Acceptance Criteria", ""))
        ws_md.cell(row=i, column=7, value=obj.get("Meta Headers", ""))
        ws_md.cell(row=i, column=8, value=obj.get("Meta Macros", ""))
        ws_md.cell(row=i, column=9, value=obj.get("Meta Arrays", ""))

    # Styling and formatting
    style_header(ws_tp)
    style_header(ws_md)

    style_data(ws_tp, max_col=len(TESTPLAN_HEADERS), max_row=ws_tp.max_row)
    style_data(ws_md, max_col=len(METADATA_HEADERS), max_row=ws_md.max_row)

    # Set readable widths
    set_column_widths(ws_tp, [8, 18, 18, 24, 36, 10, 10, 20, 20, 18, 36, 24, 30, 24])
    set_column_widths(ws_md, [8, 24, 36, 36, 28, 30, 24, 20, 20])

    # Filters
    ws_tp.auto_filter.ref = f"A1:{get_column_letter(len(TESTPLAN_HEADERS))}{ws_tp.max_row}"
    ws_md.auto_filter.ref = f"A1:{get_column_letter(len(METADATA_HEADERS))}{ws_md.max_row}"

    # MetaData VeryHidden
    ws_md.sheet_state = "veryHidden"

    # Ensure output directory
    out_dir = Path(output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / file_name

    # Save and validate
    wb.save(out_path)
    validate_workbook(out_path, expected_rows=len(data))

    # Commit to repo
    # Ensure we are on the correct branch
    os.system(f"git checkout {branch}")
    os.system("git config user.name \"github-actions[bot]\"")
    os.system("git config user.email \"41898282+github-actions[bot]@users.noreply.github.com\"")
    os.system(f"git add {out_path.as_posix()}")
    os.system(f"git commit -m \"{commit_message}\"")
    rc = os.system("git push")
    if rc != 0:
        raise SystemExit("Failed to push generated workbook")

    print(f"WORKBOOK_PATH={out_path.as_posix()}")

if __name__ == "__main__":
    main()
