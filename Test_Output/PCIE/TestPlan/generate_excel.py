#!/usr/bin/env python3
"""Generate PCIE TestPlan Excel workbook from JSON data.

Usage:
    python generate_excel.py

Requires:
    pip install openpyxl

Reads PCIE_TestPlan_data.json from the same directory and produces
a timestamped .xlsx file with two sheets: Main and MetaData.
"""

import json
import os
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
except ImportError:
    print("ERROR: openpyxl is required. Install with: pip install openpyxl")
    raise SystemExit(1)


def load_data():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    json_path = os.path.join(script_dir, "PCIE_TestPlan_data.json")
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)


def create_workbook(data):
    wb = Workbook()

    # --- Styles ---
    header_font = Font(name="Calibri", bold=True, size=11, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell_alignment = Alignment(vertical="top", wrap_text=True)
    thin_border = Border(
        left=Side(style="thin"),
        right=Side(style="thin"),
        top=Side(style="thin"),
        bottom=Side(style="thin"),
    )

    # --- Main Sheet ---
    ws_main = wb.active
    ws_main.title = "Main"
    main_columns = [
        "Index", "SS / Module", "Test Case Name", "Feature",
        "Test Description", "Test Steps / Procedure",
        "Impacted Registers", "Validation / Acceptance Criteria", "Remarks"
    ]
    for col_idx, col_name in enumerate(main_columns, 1):
        cell = ws_main.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border

    for row_idx, entry in enumerate(data, 2):
        for col_idx, col_name in enumerate(main_columns, 1):
            cell = ws_main.cell(row=row_idx, column=col_idx, value=entry.get(col_name, "NA"))
            cell.alignment = cell_alignment
            cell.border = thin_border

    # Column widths
    main_widths = [8, 15, 35, 25, 60, 60, 40, 60, 50]
    for i, w in enumerate(main_widths, 1):
        ws_main.column_dimensions[ws_main.cell(row=1, column=i).column_letter].width = w

    # --- MetaData Sheet ---
    ws_meta = wb.create_sheet(title="MetaData")
    meta_columns = [
        "Index", "SS / Module", "Test Case Name", "Feature",
        "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers"
    ]
    for col_idx, col_name in enumerate(meta_columns, 1):
        cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border

    for row_idx, entry in enumerate(data, 2):
        for col_idx, col_name in enumerate(meta_columns, 1):
            cell = ws_meta.cell(row=row_idx, column=col_idx, value=entry.get(col_name, "NA"))
            cell.alignment = cell_alignment
            cell.border = thin_border

    meta_widths = [8, 15, 35, 25, 80, 80, 60]
    for i, w in enumerate(meta_widths, 1):
        ws_meta.column_dimensions[ws_meta.cell(row=1, column=i).column_letter].width = w

    return wb


def main():
    data = load_data()
    wb = create_workbook(data)

    ist = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(ist)
    timestamp = now.strftime("%Y%m%d_%H%M%S")

    script_dir = os.path.dirname(os.path.abspath(__file__))
    output_file = os.path.join(script_dir, f"PCIE_TestPlan_{timestamp}.xlsx")
    wb.save(output_file)
    print(f"Excel workbook generated: {output_file}")
    print(f"  - Main sheet: {len(data)} testcases")
    print(f"  - MetaData sheet: {len(data)} testcases")


if __name__ == "__main__":
    main()
