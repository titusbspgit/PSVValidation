#!/usr/bin/env python3
import json
import os
import sys
from argparse import ArgumentParser
from datetime import datetime
from zoneinfo import ZoneInfo
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_ORDER = [
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

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}


def load_json(path: str) -> List[Dict[str, Any]]:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if isinstance(data, dict):
        data = [data]
    if not isinstance(data, list) or not data:
        raise ValueError("JSON must be a non-empty array or an object")
    # Normalize keys preserving first-seen order
    key_order: List[str] = []
    for row in data:
        if not isinstance(row, dict):
            raise ValueError("All rows must be objects")
        for k in row.keys():
            if k not in key_order:
                key_order.append(k)
    # Fill missing keys with blanks, preserve order
    norm = []
    for row in data:
        norm_row = {k: ("" if k not in row else row[k]) for k in key_order}
        norm.append(norm_row)
    return norm


def ensure_dirs(path: str):
    os.makedirs(path, exist_ok=True)


def strval(v: Any) -> str:
    if v is None:
        return ""
    return str(v)


def build_workbook(rows: List[Dict[str, Any]]):
    # Create workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    headers = list(rows[0].keys())

    # Write header
    for col, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=col, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")

    # Write data
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ""))

    # Freeze top row
    ws.freeze_panes = "A2"

    # Basic width fit (initial)
    autofit_columns(ws)

    # Create Meta sheet and copy META columns
    meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for c_idx, h in enumerate(META_COLUMNS, start=1):
        meta.cell(row=1, column=c_idx, value=h)
    # Rows
    for r_idx in range(2, ws.max_row + 1):
        for c_idx, h in enumerate(META_COLUMNS, start=1):
            try:
                src_col = headers.index(h) + 1
                meta.cell(row=r_idx, column=c_idx, value=ws.cell(row=r_idx, column=src_col).value)
            except ValueError:
                meta.cell(row=r_idx, column=c_idx, value="")

    # Hide Meta sheet (Very Hidden)
    meta.sheet_state = 'veryHidden'

    # Prepare TestPlan sheet
    ws.title = "TestPlan"

    # Remove META columns from TestPlan
    remaining_headers = [h for h in headers if h not in META_COLUMNS]

    # Reorder columns to MAIN_ORDER
    final_headers = [h for h in MAIN_ORDER if h in remaining_headers]
    # Append any extra columns that are not in MAIN_ORDER (preserve their order)
    for h in remaining_headers:
        if h not in final_headers:
            final_headers.append(h)

    # Build a mapping old_index -> new_index based on header order
    header_to_old_idx = {h: (headers.index(h) + 1) for h in headers}

    # Create a new grid with final order in place (within same sheet)
    # First, write the headers in row 1
    for c_idx, h in enumerate(final_headers, start=1):
        cell = ws.cell(row=1, column=c_idx, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")

    # Then populate the data rows
    for r_idx in range(2, ws.max_row + 1):
        for c_idx, h in enumerate(final_headers, start=1):
            old_col = header_to_old_idx.get(h)
            val = ws.cell(row=r_idx, column=old_col).value if old_col else ""
            ws.cell(row=r_idx, column=c_idx, value=val)

    # After reordering, delete any trailing unused columns beyond final_headers
    # Determine current max columns and delete extras from right to left
    current_max_col = ws.max_column
    for col in range(len(final_headers) + 1, current_max_col + 1):
        ws.delete_cols(len(final_headers) + 1)

    # Apply formatting on TestPlan only
    apply_testplan_format(ws, final_headers)

    return wb, final_headers


def autofit_columns(ws):
    # Estimate width based on max string length in each column
    for col in range(1, ws.max_column + 1):
        max_len = 0
        for row in range(1, ws.max_row + 1):
            v = ws.cell(row=row, column=col).value
            s = "" if v is None else str(v)
            # consider multi-line
            for line in s.split("\n"):
                if len(line) > max_len:
                    max_len = len(line)
        width = min(max(10, max_len + 2), 120)
        ws.column_dimensions[get_column_letter(col)].width = width


def estimate_row_heights(ws, headers):
    # For wrapped columns, estimate height from content and column width
    char_per_unit = 1.0  # approx: width units ~ chars
    header_index = {h: i + 1 for i, h in enumerate(headers)}
    wrap_cols = [header_index[h] for h in headers if h in WRAP_COLUMNS and h in header_index]
    for row in range(2, ws.max_row + 1):
        max_lines = 1
        for c in wrap_cols:
            v = ws.cell(row=row, column=c).value
            s = "" if v is None else str(v)
            col_letter = get_column_letter(c)
            col_width = ws.column_dimensions[col_letter].width or 10
            eff_width = max(10, col_width)
            total_lines = 0
            for line in s.split("\n"):
                units = max(1, int((len(line) / (eff_width / char_per_unit)) + 0.999))
                total_lines += units
            if total_lines > max_lines:
                max_lines = total_lines
        ws.row_dimensions[row].height = max(15, 15 * max_lines)


def apply_testplan_format(ws, headers):
    # Wrap text for selected columns
    header_to_idx = {h: i + 1 for i, h in enumerate(headers)}

    # Header formatting
    header_fill = PatternFill(fill_type="solid", fgColor="0070C0")
    for c_idx in range(1, len(headers) + 1):
        cell = ws.cell(row=1, column=c_idx)
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = header_fill

    # Data alignment and wrap
    for r in range(2, ws.max_row + 1):
        for c in range(1, len(headers) + 1):
            h = headers[c - 1]
            cell = ws.cell(row=r, column=c)
            if h in WRAP_COLUMNS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif h == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")

    # Borders
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, ws.max_row + 1):
        for c in range(1, len(headers) + 1):
            ws.cell(row=r, column=c).border = border

    # Data validation for Code Generation column
    code_col_name = "Code Generation (Required / Not)"
    if code_col_name in header_to_idx:
        col_idx = header_to_idx[code_col_name]
        col_letter = get_column_letter(col_idx)
        dv = DataValidation(type="list", formula1='"Required,Not Required,"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        if ws.max_row >= 2:
            dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Autofit columns and estimate row heights after wrapping
    autofit_columns(ws)
    estimate_row_heights(ws, headers)

    # Freeze top row
    ws.freeze_panes = "A2"


def main():
    ap = ArgumentParser()
    ap.add_argument('--json', required=True, help='Path to JSON input')
    ap.add_argument('--output-dir', required=True, help='Output directory inside repository')
    ap.add_argument('--ip-name', required=True, help='IP name used for filename rule')
    ap.add_argument('--source-path', required=True, help='Exact subdirectory source path to reference (unused in Excel but retained for traceability)')
    args = ap.parse_args()

    rows = load_json(args.json)

    wb, headers = build_workbook(rows)

    # Build filename with IST timestamp
    ist = datetime.now(ZoneInfo('Asia/Kolkata'))
    fname = f"{args.ip_name}_TestPlan_{ist.strftime('%Y%m%d_%H%M%S')}.xlsx"
    ensure_dirs(args.output_dir)
    out_path = os.path.join(args.output_dir, fname)

    wb.save(out_path)

    # Write path for GH Action to read
    ensure_dirs('tools')
    with open('tools/output_path.txt', 'w', encoding='utf-8') as f:
        f.write(out_path)

    print(f"Generated: {out_path}")


if __name__ == '__main__':
    main()
