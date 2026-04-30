#!/usr/bin/env python3
import argparse
import json
import os
import re
import zipfile
from datetime import datetime, timezone, timedelta
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

META_COLS = [
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

HEADER_FILL = PatternFill(start_color="FF4F81BD", end_color="FF4F81BD", fill_type="solid")
HEADER_FONT = Font(bold=True, color="FFFFFFFF")
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=False)
TOP_LEFT_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)
TOP_LEFT = Alignment(horizontal="left", vertical="top")
RIGHT_TOP = Alignment(horizontal="right", vertical="top")
THIN = Side(style="thin", color="FF000000")
BORDER_THIN = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)


def normalize_json_records(obj):
    if isinstance(obj, dict):
        # preserve insertion order of dict values (TC1, TC2, ...)
        records = list(obj.values())
    elif isinstance(obj, list):
        records = obj
    else:
        raise ValueError("JSON root must be an object or array")
    if not records:
        raise ValueError("Empty JSON records")
    # Determine union of keys preserving first-seen order
    seen = []
    for rec in records:
        if not isinstance(rec, dict):
            raise ValueError("Each record must be a JSON object")
        for k in rec.keys():
            if k not in seen:
                seen.append(k)
    return records, seen


def auto_width(value):
    if value is None:
        return 0
    s = str(value)
    # rough approximation factor
    return min(max(len(s) + 2, 10), 120)


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = BORDER_THIN


def renumber_multiline(text):
    if text is None:
        return ""
    lines = str(text).splitlines()
    cleaned = []
    for line in lines:
        t = line.strip()
        # remove leading numbering/bullets like '1) ', '1. ', '- ', '* ', '(1) '
        t = re.sub(r"^(?:\(?\s*\d+\)?[\).:]\s*|[-*]\s+)", "", t)
        cleaned.append(t)
    # filter out empty lines while preserving order
    cleaned = [l for l in cleaned if l != ""]
    return "\n".join(f"{i}. {l}" for i, l in enumerate(cleaned, start=1))


def set_column_widths(ws):
    for col_idx, col in enumerate(ws.iter_cols(min_col=1, max_col=ws.max_column), start=1):
        maxw = 10
        for cell in col:
            w = auto_width(cell.value)
            if w > maxw:
                maxw = w
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = maxw


def adjust_row_heights(ws):
    base = 15
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            lines = str(v).count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base * max_lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json-path", required=True)
    ap.add_argument("--ip-name", required=True)
    ap.add_argument("--output-dir", required=True)
    args = ap.parse_args()

    with open(args.json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    records, key_order = normalize_json_records(data)

    # Create workbook and rename default sheet to Data
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers according to first-seen key order
    for ci, key in enumerate(key_order, start=1):
        cell = ws.cell(row=1, column=ci, value=key)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL

    # Write data rows, preserving values exactly
    for ri, rec in enumerate(records, start=2):
        for ci, key in enumerate(key_order, start=1):
            ws.cell(row=ri, column=ci, value=rec.get(key, ""))

    # Freeze header
    ws.freeze_panes = "A2"

    # Basic widths
    set_column_widths(ws)

    # Create META sheet and copy META columns
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    # write headers
    for ci, key in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=ci, value=key).font = HEADER_FONT
        meta_ws.cell(row=1, column=ci).alignment = CENTER
        meta_ws.cell(row=1, column=ci).fill = HEADER_FILL
    # map from Data headers to column index
    header_to_idx = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}
    for ri in range(2, ws.max_row + 1):
        for ci, key in enumerate(META_COLS, start=1):
            val = ""
            if key in header_to_idx:
                val = ws.cell(row=ri, column=header_to_idx[key]).value
            meta_ws.cell(row=ri, column=ci, value=val)
    # Set very hidden
    meta_ws.sheet_state = 'veryHidden'

    # Now normalize Data -> TestPlan
    # Determine indices for MAIN columns existing in Data; keep only those in MAIN_ORDER and present
    present_main = [k for k in MAIN_ORDER if k in header_to_idx]
    # Build a new order for Data: only present_main, by exact MAIN_ORDER
    # Create a mapping from key to its column index in Data
    main_indices = [header_to_idx[k] for k in present_main]

    # Build a new 2D array for TestPlan sheet
    rows = []
    header_row = present_main
    rows.append(header_row)
    for ri in range(2, ws.max_row + 1):
        row_vals = []
        for idx in main_indices:
            row_vals.append(ws.cell(row=ri, column=idx).value)
        rows.append(row_vals)

    # Clear existing Data and write reordered content
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)

    for ci, key in enumerate(header_row, start=1):
        cell = ws.cell(row=1, column=ci, value=key)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL
    for ri, row in enumerate(rows[1:], start=2):
        for ci, v in enumerate(row, start=1):
            ws.cell(row=ri, column=ci, value=v)

    # Rename Data to TestPlan (must not create a new sheet)
    ws.title = "TestPlan"

    # Formatting: wrap text for specified columns
    wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
    col_name_by_idx = {i: ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)}

    for i in range(1, ws.max_column + 1):
        hdr = ws.cell(row=1, column=i)
        hdr.font = HEADER_FONT
        hdr.alignment = CENTER
        hdr.fill = HEADER_FILL

    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            name = col_name_by_idx[c]
            cell = ws.cell(row=r, column=c)
            if name in wrap_cols:
                cell.alignment = TOP_LEFT_WRAP
            elif name == "Index":
                cell.alignment = CENTER
            else:
                cell.alignment = TOP_LEFT

    # Numbering inside cells for steps and VAC
    def col_index_by_name(name):
        for c in range(1, ws.max_column + 1):
            if ws.cell(row=1, column=c).value == name:
                return c
        return None

    for target in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
        cidx = col_index_by_name(target)
        if cidx:
            for r in range(2, ws.max_row + 1):
                v = ws.cell(row=r, column=cidx).value
                ws.cell(row=r, column=cidx, value=renumber_multiline(v))

    # Borders
    apply_borders(ws)

    # Auto-fit columns and rows after wrapping
    set_column_widths(ws)
    adjust_row_heights(ws)

    # Data validation on Code Generation (Required / Not)
    cg_col = col_index_by_name("Code Generation (Required / Not)")
    if cg_col:
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        dv.add(f"{ws.cell(row=1, column=cg_col).column_letter}2:{ws.cell(row=1, column=cg_col).column_letter}{ws.max_row}")

    # After normalization, only sheets allowed: TestPlan (visible), Meta_data_sheet (veryHidden)
    # Remove any other sheets if exist
    for st in list(wb.sheetnames):
        if st not in ("TestPlan", "Meta_data_sheet"):
            std = wb[st]
            wb.remove(std)

    # Validate XLSX by saving then checking ZIP members
    ist = ZoneInfo("Asia/Kolkata")
    now_ist = datetime.now(ist)
    out_dir = args.output_dir
    os.makedirs(out_dir, exist_ok=True)
    out_name = f"{args.ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"
    out_path = os.path.join(out_dir, out_name)
    wb.save(out_path)

    # Check OOXML ZIP structure
    with zipfile.ZipFile(out_path, 'r') as zf:
        names = set(zf.namelist())
        required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
        missing = [r for r in required if r not in names]
        if missing:
            raise SystemExit(f"XLSX validation failed, missing: {missing}")

    print(out_path)

if __name__ == "__main__":
    main()
