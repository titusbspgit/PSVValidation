#!/usr/bin/env python3
import json
import argparse
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
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

HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")
HEADER_FONT = Font(bold=True, color="FFFFFF")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
DATA_ALIGN_LEFT_TOP = Alignment(horizontal="left", vertical="top", wrap_text=False)
DATA_ALIGN_CENTER_TOP = Alignment(horizontal="center", vertical="top", wrap_text=False)
DATA_ALIGN_LEFT_TOP_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)

THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def ordered_union_keys(rows):
    seen = OrderedDict()
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen[k] = None
    return list(seen.keys())


def autofit_columns(ws):
    # Approximate auto-fit based on max string length of cell values
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                val = cell.value
                if val is None:
                    ln = 0
                else:
                    s = str(val)
                    ln = max(len(line) for line in s.splitlines())
                if ln > max_len:
                    max_len = ln
            except Exception:
                pass
        width = min(120, max(10, int(max_len * 0.95) + 2))
        ws.column_dimensions[col_letter].width = width


def adjust_row_heights(ws):
    # Estimate row height based on line breaks in wrapped columns
    header_row = 1
    base_height = 15
    for row_idx in range(2, ws.max_row + 1):
        max_lines = 1
        for col_idx, header in enumerate(h for h in MAIN_COLUMNS):
            cell = ws.cell(row=row_idx, column=col_idx + 1)
            if header in WRAP_COLUMNS and isinstance(cell.value, str):
                lines = cell.value.count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        ws.row_dimensions[row_idx].height = base_height * max_lines


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def build_workbook(data_rows, output_path):
    wb = Workbook()
    ws_data = wb.active
    ws_data.title = "Data"

    # Build Data sheet headers from union of keys preserving order
    headers = ordered_union_keys(data_rows)
    ws_data.append(headers)
    # Write rows preserving values exactly
    for r in data_rows:
        ws_data.append([r.get(h, "") for h in headers])

    # Basic formatting on Data sheet
    for cell in ws_data[1]:
        cell.font = Font(bold=True)
        cell.alignment = HEADER_ALIGN
    ws_data.freeze_panes = "A2"
    autofit_columns(ws_data)

    # Meta_data_sheet
    ws_meta = wb.create_sheet("Meta_data_sheet")
    ws_meta.append(META_COLUMNS)
    for r in data_rows:
        ws_meta.append([r.get(h, "") for h in META_COLUMNS])
    # Very hidden
    ws_meta.sheet_state = "veryHidden"

    # Build TestPlan sheet from scratch to ensure exact column order and content
    ws_tp = wb.create_sheet("TestPlan")
    ws_tp.append(MAIN_COLUMNS)
    # Header styling
    for cell in ws_tp[1]:
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL

    # Append data rows
    for r in data_rows:
        row_vals = [r.get(h, "") for h in MAIN_COLUMNS]
        # Keep Index numeric if possible
        if isinstance(row_vals[0], str):
            try:
                row_vals[0] = int(row_vals[0])
            except Exception:
                pass
        ws_tp.append(row_vals)

    # Alignment per column
    for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=1, max_col=ws_tp.max_column):
        for cell in row:
            header = ws_tp.cell(row=1, column=cell.column).value
            if header in WRAP_COLUMNS:
                cell.alignment = DATA_ALIGN_LEFT_TOP_WRAP
            elif header == "Index":
                cell.alignment = DATA_ALIGN_CENTER_TOP
            else:
                # Text columns left/top
                cell.alignment = DATA_ALIGN_LEFT_TOP

    # Freeze and sizing
    ws_tp.freeze_panes = "A2"
    autofit_columns(ws_tp)
    adjust_row_heights(ws_tp)

    # Borders
    apply_borders(ws_tp)

    # Data validation for Code Generation (Required / Not)
    try:
        col_idx = MAIN_COLUMNS.index("Code Generation (Required / Not)") + 1
        col_letter = ws_tp.cell(row=1, column=col_idx).column_letter
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
        dv.error = "Select a value from the list: Required, Blank, Not Required"
        rng = f"{col_letter}2:{col_letter}{ws_tp.max_row}"
        dv.add(rng)
        ws_tp.add_data_validation(dv)
    except Exception:
        pass

    # Drop the Data sheet (since we have TestPlan now)
    wb.remove(ws_data)

    # Save
    wb.save(output_path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", required=True)
    ap.add_argument("--output-dir", required=True)
    ap.add_argument("--ip-name", required=True)
    ap.add_argument("--emit-outputs", default=None, help="Path to write GitHub Actions outputs")
    args = ap.parse_args()

    json_path = Path(args.json)
    if not json_path.exists():
        raise SystemExit(f"JSON file not found: {json_path}")
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    if not isinstance(data, dict) or "test_cases" not in data or not isinstance(data["test_cases"], list) or len(data["test_cases"]) == 0:
        raise SystemExit("Invalid JSON input: expected object with non-empty 'test_cases' array")

    rows = data["test_cases"]

    # Timestamp in IST for filename and commit message
    now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    ymd = now_ist.strftime("%Y%m%d")
    hms = now_ist.strftime("%H%M%S")
    filename = f"{args.ip_name}_TestPlan_{ymd}_{hms}.xlsx"

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / filename

    build_workbook(rows, str(out_path))

    ist_ts_human = now_ist.strftime("%Y-%m-%d %H:%M:%S IST")

    # Emit outputs for GitHub Actions
    if args.emit_outputs:
        with open(args.emit_outputs, "a", encoding="utf-8") as outf:
            outf.write(f"excel_path={out_path.as_posix()}\n")
            outf.write(f"excel_filename={filename}\n")
            outf.write(f"ist_timestamp={ist_ts_human}\n")

    print(f"Generated: {out_path.as_posix()}")
    print(f"IST timestamp: {ist_ts_human}")


if __name__ == "__main__":
    main()
