import json
import os
from datetime import datetime, timezone, timedelta
from pathlib import Path

from openpyxl import Workbook
from openpyxl.styles import Font


def ist_now_string():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime("%YMMDD_%H%M%S").replace("YMM", datetime.now(ist).strftime("%Y%m"))


def ist_timestamp():
    # Corrected formatter: YYYYMMDD_HHMMSS in IST
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime("%Y%m%d_%H%M%S")


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


def flatten_value(v):
    if v is None:
        return ""
    if isinstance(v, list):
        return "; ".join(str(x) for x in v)
    if isinstance(v, dict):
        # Preserve deterministically
        return json.dumps(v, ensure_ascii=False, sort_keys=True)
    return str(v)


def build_rows(data, columns):
    rows = []
    for obj in data:
        row = [flatten_value(obj.get(col, "")) for col in columns]
        rows.append(row)
    return rows


def main():
    repo_root = Path(__file__).resolve().parents[1]
    data_path = repo_root / "data" / "testplan.json"

    if not data_path.exists():
        raise FileNotFoundError(f"Input JSON not found: {data_path}")

    with data_path.open("r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("json_data must be a non-empty array of objects")

    # Prepare workbook
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = "TestPlan"
    ws_meta = wb.create_sheet("MetaData")

    # Headers (bold)
    ws_plan.append(TESTPLAN_COLUMNS)
    ws_meta.append(METADATA_COLUMNS)
    for cell in ws_plan[1]:
        cell.font = Font(bold=True)
    for cell in ws_meta[1]:
        cell.font = Font(bold=True)

    # Freeze top row
    ws_plan.freeze_panes = "A2"
    ws_meta.freeze_panes = "A2"

    # Rows
    plan_rows = build_rows(data, TESTPLAN_COLUMNS)
    meta_rows = build_rows(data, METADATA_COLUMNS)
    for r in plan_rows:
        ws_plan.append(r)
    for r in meta_rows:
        ws_meta.append(r)

    # Very hide metadata sheet
    ws_meta.sheet_state = "veryHidden"

    # Output location
    output_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
    out_path = repo_root / output_dir
    out_path.mkdir(parents=True, exist_ok=True)

    # Filename with IST timestamp
    ts = ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    fullpath = out_path / filename

    # Save actual XLSX (openpyxl writes real binary .xlsx)
    wb.save(str(fullpath))

    # Write marker file with latest filename
    marker = out_path / "latest_excel_filename.txt"
    with marker.open("w", encoding="utf-8") as mf:
        mf.write(filename + "\n")

    print(f"Generated: {fullpath}")


if __name__ == "__main__":
    main()
