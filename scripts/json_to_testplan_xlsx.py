#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import json
import os
import re
import sys
import zipfile
from datetime import datetime
from zoneinfo import ZoneInfo

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
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

HEADER_FILL = PatternFill(fill_type="solid", fgColor="1F4E79")  # Dark blue
HEADER_FONT = Font(bold=True, color="FFFFFF")
THIN_BORDER = Border(
    left=Side(style="thin"),
    right=Side(style="thin"),
    top=Side(style="thin"),
    bottom=Side(style="thin"),
)


def parse_args():
    p = argparse.ArgumentParser(description="Generate formatted TestPlan XLSX from JSON")
    p.add_argument("--json", dest="json_str", required=True, help="JSON array or object string")
    p.add_argument("--ip-name", dest="ip_name", required=False, default="GPIO")
    p.add_argument("--output-dir", dest="output_dir", required=True)
    return p.parse_args()


def load_json(json_str):
    try:
        data = json.loads(json_str)
    except json.JSONDecodeError as e:
        print(f"ERROR: Invalid JSON: {e}", file=sys.stderr)
        sys.exit(2)

    # Normalize to array of records. If dict (e.g., {TC1: {...}}), preserve insertion order of values
    if isinstance(data, dict):
        data_arr = list(data.values())
    elif isinstance(data, list):
        data_arr = data
    else:
        print("ERROR: JSON must be an array or an object of records", file=sys.stderr)
        sys.exit(2)

    if not data_arr:
        print("ERROR: JSON array is empty", file=sys.stderr)
        sys.exit(2)

    # Ensure each item is dict
    for i, rec in enumerate(data_arr):
        if not isinstance(rec, dict):
            print(f"ERROR: Element at index {i} is not an object", file=sys.stderr)
            sys.exit(2)

    return data_arr


def union_keys_preserve_order(records):
    seen = []
    s = set()
    for rec in records:
        for k in rec.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def value_to_cell(v):
    # Preserve values exactly; for lists/dicts store as JSON string
    if isinstance(v, (list, dict)):
        return json.dumps(v, ensure_ascii=False)
    return v


def strip_leading_bullets_and_numbers(line):
    # Remove leading bullets like '-', '*', '•', or leading numbers/letters with '.' or ')'
    # Example patterns: "1.", "2)", "- ", "• ", "a)"
    return re.sub(r"^([\-\u2022\*]+\s*|[0-9]+[\.)]\s*|[A-Za-z][\.)]\s*)", "", line).strip()


def renumber_block(text):
    if text is None:
        return ""
    # Normalize newlines
    t = str(text).replace("\r\n", "\n").replace("\r", "\n")
    lines = [ln for ln in (s.strip() for s in t.split("\n"))]
    # Keep non-empty lines; strip existing bullets/numbers
    items = [strip_leading_bullets_and_numbers(ln) for ln in lines if ln != ""]
    if not items:
        return ""
    return "\n".join(f"{i+1}. {items[i]}" for i in range(len(items)))


def autosize_columns(ws, max_width=80):
    # Compute max string length per column (approx)
    col_widths = {}
    for row in ws.iter_rows(values_only=True):
        for idx, v in enumerate(row, start=1):
            if v is None:
                l = 0
            else:
                s = str(v)
                # consider longest line in wrapped text
                l = max((len(part) for part in s.split("\n")), default=0)
            col_widths[idx] = max(col_widths.get(idx, 0), l)
    for idx, w in col_widths.items():
        adj = min(max(8, w + 2), max_width)
        ws.column_dimensions[get_column_letter(idx)].width = adj


def autofit_row_heights(ws, start_row=2):
    # Approximate height based on wrapped lines for WRAP_COLUMNS only
    # Determine column indices for wrap columns
    headers = [c.value for c in ws[1]]
    wrap_col_indexes = {headers.index(h) + 1 for h in headers if h in WRAP_COLUMNS}
    base_height = 15
    for r in range(start_row, ws.max_row + 1):
        max_lines = 1
        for c in wrap_col_indexes:
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            s = str(v)
            lines = s.split("\n")
            max_lines = max(max_lines, len(lines))
        ws.row_dimensions[r].height = base_height * max_lines


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def generate_workbook(records, ip_name, output_dir):
    # Phase 1: Base workbook with Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header keys: union preserve first-seen
    headers = union_keys_preserve_order(records)
    # Write header
    for j, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=j, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Write rows
    for i, rec in enumerate(records, start=2):
        for j, h in enumerate(headers, start=1):
            ws.cell(row=i, column=j, value=value_to_cell(rec.get(h, "")))

    # Basic formatting: approximate auto-fit columns
    autosize_columns(ws)

    # Phase 2: Create Meta_data_sheet and copy META columns
    meta = wb.create_sheet(title="Meta_data_sheet")
    # Header
    for j, h in enumerate(META_COLUMNS, start=1):
        meta.cell(row=1, column=j, value=h).font = Font(bold=True)
    # Rows
    for i in range(2, ws.max_row + 1):
        for j, h in enumerate(META_COLUMNS, start=1):
            # Find source column index in Data
            try:
                src_idx = headers.index(h) + 1
                v = ws.cell(row=i, column=src_idx).value
            except ValueError:
                v = ""
            meta.cell(row=i, column=j, value=v)
    # Very hidden
    meta.sheet_state = "veryHidden"

    # Phase 2: Normalize main sheet in-place: rename Data to TestPlan
    ws.title = "TestPlan"

    # Rebuild TestPlan columns to MAIN_COLUMNS order, removing META
    # Build a mapping of current header -> column index
    current_headers = [c.value for c in ws[1]]
    # Create a new 2D array for TestPlan content
    rows_out = []
    rows_out.append(MAIN_COLUMNS[:])

    for r in range(2, ws.max_row + 1):
        row_vals = []
        for col_name in MAIN_COLUMNS:
            if col_name in current_headers:
                idx = current_headers.index(col_name) + 1
                v = ws.cell(row=r, column=idx).value
            else:
                v = ""
            row_vals.append(v)
        rows_out.append(row_vals)

    # Clear sheet and write back
    ws.delete_rows(1, ws.max_row)
    for i, row in enumerate(rows_out, start=1):
        for j, v in enumerate(row, start=1):
            ws.cell(row=i, column=j, value=v)

    # Strict formatting for TestPlan
    # Header formatting
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL

    # Data rows alignment and wrapping
    headers = [c.value for c in ws[1]]
    wrap_idx = {headers.index(h) + 1 for h in headers if h in WRAP_COLUMNS}
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            wrap = c in wrap_idx
            # default alignment: text left, vertical top
            halign = "left"
            if headers[c - 1] == "Index":
                halign = "center"
            cell.alignment = Alignment(horizontal=halign, vertical="top", wrap_text=wrap)

    # Numbering inside specific columns
    for target_col in [
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    ]:
        if target_col in headers:
            cidx = headers.index(target_col) + 1
            for r in range(2, ws.max_row + 1):
                cell = ws.cell(row=r, column=cidx)
                cell.value = renumber_block(cell.value)

    # Borders and sizing
    autosize_columns(ws)
    autofit_row_heights(ws, start_row=2)
    apply_borders(ws)

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in headers:
        cidx = headers.index("Code Generation (Required / Not)") + 1
        col_letter = get_column_letter(cidx)
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        dv.error = "Select a value from the list: Required, Blank, Not Required"
        dv.errorTitle = "Invalid value"
        dv.prompt = "Choose one of: Required, Blank, Not Required"
        dv.promptTitle = "Code Generation (Required / Not)"
        ws.add_data_validation(dv)
        # Apply only to data rows
        if ws.max_row >= 2:
            dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: only TestPlan (visible) and Meta_data_sheet (veryHidden) must exist
    # Ensure there is no sheet named 'Data'
    if "Data" in wb.sheetnames:
        # delete it if present (shouldn't be)
        for sh in wb.worksheets:
            if sh.title == "Data":
                wb.remove(sh)
                break

    # Final save path
    ist = ZoneInfo("Asia/Kolkata")
    now_ist = datetime.now(ist)
    fname = f"{ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"
    outdir = os.path.normpath(output_dir)
    os.makedirs(outdir, exist_ok=True)
    fpath = os.path.join(outdir, fname)
    wb.save(fpath)

    # Validate as true XLSX
    try:
        with zipfile.ZipFile(fpath, 'r') as zf:
            # Basic OOXML parts presence
            assert zf.getinfo('[Content_Types].xml')
            assert any(n.startswith('xl/') and n.endswith('workbook.xml') for n in zf.namelist())
        # Load roundtrip
        _ = load_workbook(fpath)
    except Exception as e:
        print(f"ERROR: XLSX validation failed: {e}", file=sys.stderr)
        sys.exit(3)

    print(f"Generated: {fpath}")
    return fpath


def main():
    args = parse_args()
    records = load_json(args.json_str)

    # Enforce that records are row objects; fill missing fields will be handled during write
    fpath = generate_workbook(records, args.ip_name, args.output_dir)
    # Emit a machine-readable line for the workflow to parse if needed
    print(f"::set-output name=xlsx_path::{fpath}")


if __name__ == "__main__":
    main()
