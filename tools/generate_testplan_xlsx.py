#!/usr/bin/env python3
import argparse
import json
from datetime import datetime, timezone, timedelta
from pathlib import Path
import zipfile

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

META_COLS_CANONICAL = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    # Support both singular/plural key variants seen in inputs
    "Hidden_Macro_Defines",
    "Hidden_Macro_Define",
    "Hidden_Skip_Array_Definition",
]

MAIN_COL_ORDER = [
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

ALLOWED_VALIDATION_VALUES = ["Required", "Blank", "Not Required"]


def parse_args():
    ap = argparse.ArgumentParser(description="Generate formatted Excel testplan from JSON input")
    ap.add_argument("--input", required=True, help="Path to JSON array input")
    ap.add_argument("--output-dir", required=True, help="Destination directory inside repo")
    ap.add_argument("--ip-name", required=True, help="IP name for filename rule")
    return ap.parse_args()


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit("JSON must be a non-empty array")
    return data


def build_schema(records):
    seen = []
    seen_set = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    return seen


def ensure_ist_now():
    try:
        # Fixed IST offset (UTC+5:30)
        ist = timezone(timedelta(hours=5, minutes=30))
        return datetime.now(tz=ist)
    except Exception:
        return datetime.utcnow() + timedelta(hours=5, minutes=30)


def auto_widths(ws):
    widths = {}
    for row in ws.iter_rows(values_only=True):
        for i, value in enumerate(row, start=1):
            s = "" if value is None else str(value)
            widths[i] = max(widths.get(i, 0), len(s))
    for i, w in widths.items():
        # heuristic: approx char width; cap to reasonable max
        ws.column_dimensions[ws.cell(row=1, column=i).column_letter].width = min(max(10, w + 2), 80)


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows():
        for cell in row:
            if cell.value is not None or cell.row == 1:
                cell.border = border


def ensure_numbering(text):
    if text is None:
        return ""
    s = str(text)
    # Split by newline preserving non-empty logical lines
    lines = s.splitlines()
    # If already starts with numeric prefixes, re-number to 1.,2.,...
    out = []
    n = 1
    for ln in lines:
        ln_stripped = ln.strip()
        if ln_stripped == "":
            out.append(ln)
            continue
        out.append(f"{n}. {ln_stripped.lstrip('0123456789.) ')}")
        n += 1
    return "\n".join(out)


def main():
    args = parse_args()
    records = load_json(args.input)
    schema = build_schema(records)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers
    for c, key in enumerate(schema, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = PatternFill("solid", fgColor="FF0070C0")

    ws.freeze_panes = "A2"

    # Write rows preserving values exactly initially
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(schema, start=1):
            ws.cell(row=r, column=c, value=rec.get(key, ""))

    auto_widths(ws)

    # Create META sheet
    meta = wb.create_sheet("Meta_data_sheet")
    meta_headers = [k for k in META_COLS_CANONICAL if k in schema]
    # Header row
    for c, key in enumerate(meta_headers, start=1):
        cell = meta.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
    # Values
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(meta_headers, start=1):
            meta.cell(row=r, column=c, value=rec.get(key, ""))
    # Very hidden
    meta.sheet_state = "veryHidden"

    # Normalize main sheet in-place: remove META columns, reorder to MAIN order
    # Build map of column index by header
    header_to_index = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}

    # Columns to retain for main sheet in specified order (if present)
    retain_cols = [k for k in MAIN_COL_ORDER if k in header_to_index]

    # Build a new ordered list of columns for main sheet
    # Copy data to a temporary 2D list in that order
    tmp = [retain_cols]
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for key in retain_cols:
            col_idx = header_to_index[key]
            row_vals.append(ws.cell(row=r, column=col_idx).value)
        tmp.append(row_vals)

    # Clear sheet and write back in final order
    ws.delete_rows(1, ws.max_row)
    for c, key in enumerate(retain_cols, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = PatternFill("solid", fgColor="FF0070C0")
    for r, row_vals in enumerate(tmp[1:], start=2):
        for c, val in enumerate(row_vals, start=1):
            ws.cell(row=r, column=c, value=val)

    # Rename Data -> TestPlan
    ws.title = "TestPlan"

    # Wrap text for specified columns
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }

    # Apply numbering inside cells for two specific columns
    numbering_cols = {"Test Steps / Procedure", "Validation / Acceptance Criteria"}
    header_row = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    for col_name in wrap_cols:
        if col_name in header_row:
            col_idx = header_row[col_name]
            for r in range(2, ws.max_row + 1):
                cell = ws.cell(row=r, column=col_idx)
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal=("left" if col_name != "Index" else "center"))
                if col_name in numbering_cols:
                    cell.value = ensure_numbering(cell.value)

    # Align other data rows
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            if ws.cell(row=1, column=c).value == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            elif isinstance(cell.value, (int, float)):
                cell.alignment = Alignment(vertical="top", horizontal="right")
            else:
                # default text left
                cell.alignment = Alignment(vertical="top", horizontal="left")

    auto_widths(ws)

    # Borders
    apply_borders(ws)

    # Data validation for Code Generation column
    if "Code Generation (Required / Not)" in header_row:
        col_idx = header_row["Code Generation (Required / Not)"]
        dv_list = ",".join(ALLOWED_VALIDATION_VALUES)
        dv = DataValidation(type="list", formula1=f'"{dv_list}"', allow_blank=True)
        dv.error = "Select one of: Required, Blank, Not Required"
        dv.errorTitle = "Invalid Selection"
        ws.add_data_validation(dv)
        # Apply to data rows only
        start_cell = ws.cell(row=2, column=col_idx).coordinate
        end_cell = ws.cell(row=ws.max_row, column=col_idx).coordinate
        dv.add(f"{start_cell}:{end_cell}")

    # Safety check: only TestPlan and Meta_data_sheet should exist
    names = [s.title for s in wb.worksheets]
    # Ensure no sheet named "Data"
    if "Data" in names:
        # delete it
        for s in wb.worksheets:
            if s.title == "Data":
                wb.remove(s)
                break

    # Final save with IST filename rule
    now_ist = ensure_ist_now()
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    fname = f"{args.ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"
    out_path = out_dir / fname
    wb.save(out_path)

    # Validate as real XLSX (ZIP + core parts)
    with zipfile.ZipFile(out_path, 'r') as zf:
        must_have = {"[Content_Types].xml", "xl/workbook.xml"}
        names_set = set(zf.namelist())
        if not must_have.issubset(names_set):
            raise SystemExit("XLSX validation failed: core parts missing")

    print(str(out_path))

if __name__ == "__main__":
    main()
