import json
import os
from datetime import datetime, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

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


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list):
        raise ValueError("json_data must be a list of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f"Each item must be an object (dict). Invalid at index {i}")
    return data


def style_header_row(ws, header_row=1):
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4F81BD", end_color="4F81BD", fill_type="solid")
    for cell in ws[header_row]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = Alignment(wrap_text=True, vertical="top")


def autofit_columns(ws):
    # Compute max length per column
    col_widths = {}
    for row in ws.iter_rows(values_only=True):
        for idx, value in enumerate(row, start=1):
            text = "" if value is None else str(value)
            col_widths[idx] = max(col_widths.get(idx, 0), len(text))
    for idx, width in col_widths.items():
        # Reasonable bounds for readability
        adj = min(120, max(15, width + 2))
        ws.column_dimensions[get_column_letter(idx)].width = adj


def write_sheet(ws, data, columns):
    # Header
    ws.append(columns)
    style_header_row(ws)
    # Rows
    for row in data:
        values = [row.get(col, "") for col in columns]
        ws.append(values)
    # Wrap text for all cells and top-align
    for r in ws.iter_rows():
        for c in r:
            c.alignment = Alignment(wrap_text=True, vertical="top")
    ws.freeze_panes = "A2"
    autofit_columns(ws)


def build_workbook(json_data):
    wb = Workbook()
    # TestPlan sheet (visible)
    ws1 = wb.active
    ws1.title = "TestPlan"
    write_sheet(ws1, json_data, TESTPLAN_COLUMNS)

    # MetaData sheet (very hidden)
    ws2 = wb.create_sheet("MetaData")
    write_sheet(ws2, json_data, METADATA_COLUMNS)
    ws2.sheet_state = "veryHidden"

    return wb


def ist_timestamp():
    # IST is UTC+05:30, no DST
    now_utc = datetime.utcnow()
    ist = now_utc + timedelta(hours=5, minutes=30)
    return ist.strftime("%Y%m%d_%H%M%S")


def main():
    data_path = os.path.join("data", "testplan_final.json")
    json_data = load_json(data_path)

    wb = build_workbook(json_data)

    out_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
    os.makedirs(out_dir, exist_ok=True)

    ts = ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    out_path = os.path.join(out_dir, filename)

    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
