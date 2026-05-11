#!/usr/bin/env python3
import argparse
import json
import os
import re
import zipfile
from copy import deepcopy
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

ALLOWED_DV = ["Required", "Blank", "Not Required"]


def parse_args():
    ap = argparse.ArgumentParser()
    ap.add_argument("--input", required=True)
    ap.add_argument("--output", required=True)
    return ap.parse_args()


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("JSON must be a non-empty array of objects")
    return data


def ordered_union_keys(rows):
    seen = set()
    order = []
    for obj in rows:
        for k in obj.keys():
            if k not in seen:
                seen.add(k)
                order.append(k)
    return order


def sanitize_numbering(text):
    if text is None:
        return ""
    s = str(text)
    lines = [ln.strip() for ln in s.splitlines() if ln.strip() != ""]
    out = []
    for i, ln in enumerate(lines, 1):
        # remove any existing numeric/bullet prefixes like '1)', '1.', '-', '*'
        ln2 = re.sub(r"^\s*(?:[-*•\u2022\u25CF]|\d+[\.)])\s*", "", ln)
        out.append(f"{i}. {ln2}")
    return "\n".join(out) if out else s


def autofit_columns(ws, header_row=1):
    max_len = {}
    for row in ws.iter_rows(values_only=True):
        for idx, val in enumerate(row, 1):
            if val is None:
                l = 0
            else:
                s = str(val)
                l = max((len(part) for part in s.split("\n")), default=0)
            max_len[idx] = max(max_len.get(idx, 0), l)
    for idx, l in max_len.items():
        # Approximate width: characters + padding
        ws.column_dimensions[get_column_letter(idx)].width = min(max(l + 4, 12), 100)


def adjust_row_heights(ws, start_row=2):
    for r in range(start_row, ws.max_row + 1):
        # Estimate by maximum number of lines among wrapped columns in this row
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            header = ws.cell(row=1, column=c).value
            val = ws.cell(row=r, column=c).value
            if header in WRAP_COLS and isinstance(val, str):
                lines = val.count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        # excel default height ~15pt per line
        ws.row_dimensions[r].height = 15 * max_lines


def style_sheet(ws):
    # Freeze header
    ws.freeze_panes = "A2"
    # Header style
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    header_fill = PatternFill(fill_type="solid", start_color="FF4F81BD", end_color="FF4F81BD")

    thin = Side(style="thin", color="000000")
    border_all = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Apply header style
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill
        cell.border = border_all

    # Data cells style
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            header = ws.cell(row=1, column=c).value
            if header in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif header == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")
            cell.border = border_all


def apply_data_validation(ws):
    # Find target column
    target = "Code Generation (Required / Not)"
    col_idx = None
    for c in range(1, ws.max_column + 1):
        if ws.cell(row=1, column=c).value == target:
            col_idx = c
            break
    if not col_idx:
        return
    start = 2
    end = ws.max_row
    if end < start:
        return
    rng = f"{get_column_letter(col_idx)}{start}:{get_column_letter(col_idx)}{end}"
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
    ws.add_data_validation(dv)
    dv.add(rng)


def make_workbook(rows, output_path):
    # Determine schema order
    schema = ordered_union_keys(rows)

    # Stage sheet: Data
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Headers
    for c, k in enumerate(schema, 1):
        ws.cell(row=1, column=c, value=k)

    # Rows
    for r, obj in enumerate(rows, 2):
        for c, k in enumerate(schema, 1):
            v = obj.get(k, "")
            ws.cell(row=r, column=c, value=v)

    # Base formatting on Data
    style_sheet(ws)
    autofit_columns(ws)
    adjust_row_heights(ws)

    # Create META sheet and copy columns
    meta = wb.create_sheet("Meta_data_sheet")
    # Copy META headers
    for c, k in enumerate(META_COLS, 1):
        meta.cell(row=1, column=c, value=k)
    # Copy META data from Data by header lookup
    header_to_index = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}
    for r in range(2, ws.max_row + 1):
        for c, k in enumerate(META_COLS, 1):
            src_col = header_to_index.get(k)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            meta.cell(row=r, column=c, value=val)
    # Very hidden
    meta.sheet_state = "veryHidden"

    # Now normalize main sheet on the SAME worksheet (rename and reorder)
    ws.title = "TestPlan"

    # Extract current data into list of dicts
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    data_rows = []
    for r in range(2, ws.max_row + 1):
        row_dict = {}
        for i, h in enumerate(headers, 1):
            row_dict[h] = ws.cell(row=r, column=i).value
        data_rows.append(row_dict)

    # Clear sheet content
    ws.delete_rows(1, ws.max_row)

    # Write MAIN headers in required order
    for c, k in enumerate(MAIN_ORDER, 1):
        ws.cell(row=1, column=c, value=k)

    # Write rows: map values; META columns are excluded here
    for r, obj in enumerate(data_rows, 2):
        for c, k in enumerate(MAIN_ORDER, 1):
            v = obj.get(k, "")
            # Normalize numbering in specific columns on TestPlan only
            if k in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                v = sanitize_numbering(v)
            ws.cell(row=r, column=c, value=v)

    # Re-apply styles on TestPlan
    style_sheet(ws)
    autofit_columns(ws)
    adjust_row_heights(ws)

    # Data validation only for Code Generation (Required / Not)
    apply_data_validation(ws)

    # Safety: ensure no sheet named 'Data' remains
    for s in list(wb.worksheets):
        if s.title == "Data":
            wb.remove(s)

    # Ensure only allowed sheets exist
    titles = {s.title for s in wb.worksheets}
    assert "TestPlan" in titles and "Meta_data_sheet" in titles

    # Save
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    wb.save(output_path)

    # Validate ZIP-based OOXML
    with zipfile.ZipFile(output_path, 'r') as zf:
        names = set(zf.namelist())
        assert '[Content_Types].xml' in names and 'xl/workbook.xml' in names


def main():
    args = parse_args()
    rows = load_json(args.input)
    make_workbook(rows, args.output)

if __name__ == "__main__":
    main()
