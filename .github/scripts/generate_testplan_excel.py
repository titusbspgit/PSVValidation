#!/usr/bin/env python3
import json
import argparse
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from typing import List, Dict, Any
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

MAIN_COLUMNS = [
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

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")  # Excel blue
THIN = Side(style="thin", color="000000")
ALL_BORDERS = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)


def normalize_value(val: Any) -> Any:
    if isinstance(val, list):
        # Join list values with newline without altering inner values
        return "\n".join(str(x) for x in val)
    return val


def make_rows(testcases: List[Dict[str, Any]]):
    main_rows = []
    meta_rows = []
    for tc in testcases:
        main_row = {}
        meta_row = {}
        # Fill main columns
        for col in MAIN_COLUMNS:
            v = tc.get(col, "")
            main_row[col] = normalize_value(v)
        # Fill meta columns
        for mcol in META_COLUMNS:
            mv = tc.get(mcol, "")
            meta_row[mcol] = normalize_value(mv)
        main_rows.append(main_row)
        meta_rows.append(meta_row)
    return main_rows, meta_rows


def autofit_columns(ws):
    # Approximate auto-fit by measuring max string length per column
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                val = cell.value
                if val is None:
                    continue
                l = len(str(val))
                if l > max_len:
                    max_len = l
            except Exception:
                pass
        # Add padding; limit width to a reasonable max
        width = min(max_len + 4, 100)
        ws.column_dimensions[col_letter].width = width


def apply_header_format(ws):
    # Header is first row
    for cell in ws[1]:
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL
        cell.border = ALL_BORDERS


def apply_data_format(ws):
    # Data rows formatting
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            header = ws.cell(row=1, column=cell.column).value
            if header in WRAP_COLUMNS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif header == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")
            cell.border = ALL_BORDERS


def add_codegen_dropdown(ws):
    # Find the Code Generation (Required / Not) column
    headers = [c.value for c in ws[1]]
    try:
        idx = headers.index("Code Generation (Required / Not)") + 1
    except ValueError:
        return
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True, errorTitle="Invalid", error="Select from dropdown: Required or Not Required, or leave blank")
    rng = f"{ws.cell(row=2, column=idx).coordinate}:{ws.cell(row=ws.max_row, column=idx).coordinate}"
    dv.add(rng)
    ws.add_data_validation(dv)


def write_sheet(ws, headers: List[str], rows: List[Dict[str, Any]]):
    ws.append(headers)
    for r in rows:
        ws.append([r.get(h, "") for h in headers])
    ws.freeze_panes = "A2"
    ws.auto_filter.ref = ws.dimensions
    apply_header_format(ws)
    apply_data_format(ws)
    autofit_columns(ws)


def create_workbook(data: Dict[str, Any], ip: str):
    # Extract testcases
    testcases = []
    if isinstance(data, dict) and isinstance(data.get("TestCases"), list):
        testcases = data["TestCases"]
    elif isinstance(data, list):
        testcases = data
    elif isinstance(data, dict):
        testcases = [data]
    else:
        raise ValueError("Unsupported JSON format for test plan")

    main_rows, meta_rows = make_rows(testcases)

    wb = Workbook()
    ws_data = wb.active
    ws_data.title = "Data"  # temporary; will rename to TestPlan later

    write_sheet(ws_data, MAIN_COLUMNS, main_rows)

    # Meta sheet
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # For meta, avoid styling except basic header to ensure readability; will set veryHidden later
    ws_meta.append(META_COLUMNS)
    for r in meta_rows:
        ws_meta.append([r.get(h, "") for h in META_COLUMNS])
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'

    # Rename Data -> TestPlan and ensure dropdown on CodeGen column
    ws_data.title = "TestPlan"
    add_codegen_dropdown(ws_data)

    return wb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json-file", required=True, help="Path to JSON test plan input")
    ap.add_argument("--ip", required=True, help="IP name for filename prefix")
    ap.add_argument("--outdir", required=True, help="Output directory inside repo")
    args = ap.parse_args()

    with open(args.json_file, "r", encoding="utf-8") as f:
        data = json.load(f)

    wb = create_workbook(data, args.ip)

    ist_now = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts = ist_now.strftime("%Y%m%d_%H%M%S")
    filename = f"{args.ip}_TestPlan_{ts}.xlsx"

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    outfile = outdir / filename

    wb.save(str(outfile))
    print(f"WROTE:{outfile}")

if __name__ == "__main__":
    main()
