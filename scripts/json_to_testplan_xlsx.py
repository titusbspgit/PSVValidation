#!/usr/bin/env python3
import argparse
import json
import os
import sys
from datetime import datetime
from zoneinfo import ZoneInfo
import zipfile

from openpyxl import Workbook, load_workbook
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

THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)

BLUE_FILL = PatternFill(start_color="FFCCE5FF", end_color="FFCCE5FF", fill_type="solid")


def parse_args():
    ap = argparse.ArgumentParser(description="Generate GPIO TestPlan Excel from JSON")
    ap.add_argument("--json", required=True, help="Path to JSON array input")
    ap.add_argument("--outdir", required=True, help="Output directory inside repo")
    ap.add_argument("--ip-name", required=True, help="IP name for filename rule")
    ap.add_argument("--gha-output", required=False, help="Path to GITHUB_OUTPUT file to write outputs")
    return ap.parse_args()


def validate_json_array(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("JSON must be a non-empty array of objects")
    for i, rec in enumerate(data):
        if not isinstance(rec, dict):
            raise ValueError(f"JSON element {i} is not an object")
    return data


def normalize_schema(records):
    # Preserve first-seen key order across the array
    seen = []
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.append(k)
    # Ensure META and MAIN columns exist (create blanks if missing)
    for col in META_COLUMNS + MAIN_COLUMNS:
        if col not in seen:
            seen.append(col)
    # Build rows with blanks for missing
    norm = []
    for rec in records:
        row = {}
        for k in seen:
            row[k] = rec.get(k, "")
        norm.append(row)
    return seen, norm


def auto_width(ws):
    # Compute approximate column widths
    for col_cells in ws.columns:
        max_len = 0
        col_letter = col_cells[0].column_letter
        for cell in col_cells:
            try:
                v = "" if cell.value is None else str(cell.value)
            except Exception:
                v = ""
            max_len = max(max_len, len(v))
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 80)


def numberize_multiline(text):
    if text is None:
        return ""
    s = str(text).replace("\r\n", "\n").replace("\r", "\n").strip()
    if not s:
        return s
    # Split on explicit newlines; if none, return as-is
    parts = [p.strip() for p in s.split("\n") if p.strip()]
    if len(parts) <= 1:
        return s
    numbered = [f"{i+1}. {p}" for i, p in enumerate(parts)]
    return "\n".join(numbered)


def build_workbook(records):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers (all keys, preserved order)
    keys, norm_rows = normalize_schema(records)

    for j, key in enumerate(keys, start=1):
        c = ws.cell(row=1, column=j, value=key)
        c.font = Font(bold=True)
        c.alignment = Alignment(horizontal="center", vertical="center")
        c.fill = BLUE_FILL
        c.border = THIN_BORDER

    # Write data rows
    for i, row in enumerate(norm_rows, start=2):
        for j, key in enumerate(keys, start=1):
            val = row.get(key, "")
            ws.cell(row=i, column=j, value=val)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Base autofit columns
    auto_width(ws)

    # Create META sheet
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for j, key in enumerate(META_COLUMNS, start=1):
        c = ws_meta.cell(row=1, column=j, value=key)
        c.font = Font(bold=True)
        c.alignment = Alignment(horizontal="center", vertical="center")
        c.fill = BLUE_FILL
        c.border = THIN_BORDER
    # Data
    for i, row in enumerate(norm_rows, start=2):
        for j, key in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=i, column=j, value=row.get(key, ""))
    auto_width(ws_meta)
    # Very Hidden
    ws_meta.sheet_state = "veryHidden"

    # Normalize main sheet (in-place on "Data")
    # Build a mapping from header to column index
    header_to_idx = {ws.cell(row=1, column=j).value: j for j in range(1, ws.max_column + 1)}

    # Determine column order for TestPlan (ensure blanks if missing)
    # If a column missing, append a new blank column with that header
    current_last_col = ws.max_column
    for col_name in MAIN_COLUMNS:
        if col_name not in header_to_idx:
            current_last_col += 1
            ws.cell(row=1, column=current_last_col, value=col_name)
            header_to_idx[col_name] = current_last_col
            for i in range(2, ws.max_row + 1):
                ws.cell(row=i, column=current_last_col, value="")

    # Remove META columns from main sheet (set values to blank, but keep structure by reordering copy)
    # Create a new ordered matrix for TestPlan
    rows_matrix = []
    # Header row in final order
    rows_matrix.append(MAIN_COLUMNS)
    for i in range(2, ws.max_row + 1):
        final_row = []
        for col_name in MAIN_COLUMNS:
            col_idx = header_to_idx[col_name]
            final_row.append(ws.cell(row=i, column=col_idx).value)
        rows_matrix.append(final_row)

    # Clear and rewrite ws
    ws.delete_rows(1, ws.max_row)
    for r, row_vals in enumerate(rows_matrix, start=1):
        for c, v in enumerate(row_vals, start=1):
            ws.cell(row=r, column=c, value=v)
    # Rename to TestPlan
    ws.title = "TestPlan"

    # Apply formatting on TestPlan
    max_row = ws.max_row
    max_col = ws.max_column

    # Header formatting
    for j in range(1, max_col + 1):
        cell = ws.cell(row=1, column=j)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Data alignment, wrapping, borders
    wrap_headers = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }

    # Numbering enforcement for two columns on TestPlan only (not on META)
    # Find column indices
    header_idx = {ws.cell(row=1, column=j).value: j for j in range(1, max_col + 1)}
    idx_steps = header_idx.get("Test Steps / Procedure")
    idx_val = header_idx.get("Validation / Acceptance Criteria")

    for i in range(2, max_row + 1):
        for j in range(1, max_col + 1):
            cell = ws.cell(row=i, column=j)
            header = ws.cell(row=1, column=j).value
            if header in wrap_headers:
                # For numbering-required columns, transform content
                if j == idx_steps or j == idx_val:
                    cell.value = numberize_multiline(cell.value)
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            else:
                # Numeric-ish columns center/right, others left
                if header in ("Index",):
                    cell.alignment = Alignment(vertical="top", horizontal="center")
                else:
                    cell.alignment = Alignment(vertical="top", horizontal="left")
            cell.border = THIN_BORDER

    # Autofit columns again after wrapping
    auto_width(ws)

    # Data validation ONLY on Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in header_idx:
        col = header_idx["Code Generation (Required / Not)"]
        col_letter = ws.cell(row=1, column=col).column_letter
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        dv.error = "Select a value from the list"
        dv.errorTitle = "Invalid Input"
        ws.add_data_validation(dv)
        data_range = f"{col_letter}2:{col_letter}{max_row}"
        dv.add(data_range)

    # Safety: ensure no sheet named Data remains
    if any(s.title == "Data" for s in wb.worksheets):
        # Delete any worksheet named Data
        for s in list(wb.worksheets):
            if s.title == "Data":
                wb.remove(s)

    # Return workbook
    return wb


def validate_final_xlsx(path):
    # Must be a valid ZIP-based OOXML
    if not zipfile.is_zipfile(path):
        return False, "Not a ZIP-based OOXML file"
    try:
        wb = load_workbook(path, data_only=True, read_only=True)
        names = [ws.title for ws in wb.worksheets]
        states = {ws.title: ws.sheet_state for ws in wb.worksheets}
        if set(names) != {"TestPlan", "Meta_data_sheet"}:
            return False, f"Unexpected sheets: {names}"
        if states.get("TestPlan", "visible") != "visible":
            return False, "TestPlan sheet not visible"
        if states.get("Meta_data_sheet") != "veryHidden":
            return False, "Meta_data_sheet not Very Hidden"
        return True, "OK"
    except Exception as e:
        return False, f"load_workbook failed: {e}"


def main():
    args = parse_args()
    records = validate_json_array(args.json)

    wb = build_workbook(records)

    # Filename rule with IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    now = datetime.now(ist)
    fname = f"{args.ip_name}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"
    outdir = args.outdir.rstrip("/")
    os.makedirs(outdir, exist_ok=True)
    out_path = os.path.join(outdir, fname)

    wb.save(out_path)

    ok, msg = validate_final_xlsx(out_path)
    if not ok:
        print(f"Validation failed: {msg}", file=sys.stderr)
        sys.exit(2)

    # Write path for downstream steps
    with open(".xlsx_path.txt", "w", encoding="utf-8") as f:
        f.write(out_path)

    print(f"Generated: {out_path}")

if __name__ == "__main__":
    main()
