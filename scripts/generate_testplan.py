import json, os, sys, zipfile
from copy import deepcopy
from datetime import datetime
from io import StringIO

from openpyxl import Workbook, load_workbook
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter

# Configuration (fixed per task input)
OUTPUT_PATH = os.path.join("Test_Output", "PCIE", "TestPlan")
OUTPUT_FILENAME = "PCIE_TestPlan_20260511_000000.xlsx"

# JSON data embedded deterministically
JSON_DATA = r'''REPLACE_JSON_HERE'''

# Column definitions
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

VALIDATION_COL = "Code Generation (Required / Not)"
VALIDATION_LIST = "Required,Blank,Not Required"

HEADER_FILL = PatternFill(start_color="FF4F81BD", end_color="FF4F81BD", fill_type="solid")
HEADER_FONT = Font(bold=True, color="FFFFFFFF")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
DATA_ALIGN_LEFT = Alignment(horizontal="left", vertical="top", wrap_text=False)
DATA_ALIGN_LEFT_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)
DATA_ALIGN_CENTER = Alignment(horizontal="center", vertical="top", wrap_text=False)
THIN_BORDER = Border(left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin"))


def fail(msg: str):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def parse_json(data_str: str):
    try:
        data = json.loads(data_str)
    except Exception as e:
        fail(f"Invalid JSON: {e}")
    if not isinstance(data, list) or len(data) == 0:
        fail("JSON must be a non-empty array of objects")
    # Validate all rows are dicts
    for i, row in enumerate(data, 1):
        if not isinstance(row, dict):
            fail(f"Row {i} is not an object")
    return data


def union_keys_preserve_order(rows):
    seen = []
    s = set()
    for row in rows:
        for k in row.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def normalize_rows(rows, all_keys):
    norm = []
    for r in rows:
        norm.append({k: r.get(k, "") if r.get(k, None) is not None else "" for k in all_keys})
    return norm


def renumber_multiline(text: str) -> str:
    if text is None:
        return ""
    # Normalize newlines and split
    lines = [ln.strip() for ln in str(text).replace("\r\n", "\n").replace("\r", "\n").split("\n")]
    lines = [ln for ln in lines if ln != ""]
    if not lines:
        return ""
    numbered = [f"{i+1}. {ln}" for i, ln in enumerate(lines)]
    return "\n".join(numbered)


def write_base_sheet(wb: Workbook, rows, all_keys):
    ws = wb.active
    ws.title = "Data"
    # Header
    for c, key in enumerate(all_keys, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, row in enumerate(rows, 2):
        for c, key in enumerate(all_keys, 1):
            val = row.get(key, "")
            ws.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    ws.freeze_panes = "A2"
    return ws


def autofit_columns(ws):
    # Estimate width by max string length
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), 1):
        max_len = 0
        for cell in col:
            val = "" if cell.value is None else str(cell.value)
            if len(val) > max_len:
                max_len = len(val)
        adj = max_len + 2
        ws.column_dimensions[get_column_letter(col_idx)].width = min(max(10, adj), 120)


def style_headers(ws):
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def make_meta_sheet(wb: Workbook, rows):
    meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for c, key in enumerate(META_COLS, 1):
        cell = meta.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, row in enumerate(rows, 2):
        for c, key in enumerate(META_COLS, 1):
            val = row.get(key, "")
            meta.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    # Very hidden
    meta.sheet_state = "veryHidden"


def reorder_to_main_inplace(ws, rows):
    # Clear sheet and write only MAIN_ORDER columns
    ws.delete_rows(1, ws.max_row)
    # Header
    for c, key in enumerate(MAIN_ORDER, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL
        cell.border = THIN_BORDER
    # Data
    for r_idx, src in enumerate(rows, 2):
        for c, key in enumerate(MAIN_ORDER, 1):
            val = src.get(key, "")
            ws.cell(row=r_idx, column=c, value=val).border = THIN_BORDER
    ws.freeze_panes = "A2"


def apply_wrapping_and_alignment(ws):
    # Determine column index map
    header = [cell.value for cell in ws[1]]
    colmap = {name: idx+1 for idx, name in enumerate(header)}
    max_row = ws.max_row

    for r in range(2, max_row+1):
        for name in header:
            c = colmap[name]
            cell = ws.cell(row=r, column=c)
            if name == "Index":
                cell.alignment = DATA_ALIGN_CENTER
            elif name in WRAP_COLS:
                cell.alignment = DATA_ALIGN_LEFT_WRAP
            else:
                cell.alignment = DATA_ALIGN_LEFT

    # Renumber required columns
    for name in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if name in colmap:
            c = colmap[name]
            for r in range(2, max_row+1):
                val = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=renumber_multiline(val))

    # Rough row height based on number of lines in wrapped columns
    for r in range(2, max_row+1):
        max_lines = 1
        for name in WRAP_COLS:
            if name in colmap:
                val = ws.cell(row=r, column=colmap[name]).value
                if val is None:
                    continue
                lines = str(val).count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        # Approximate 15 pts per line
        ws.row_dimensions[r].height = min(15 * max_lines + 2, 409)

    style_headers(ws)
    apply_borders(ws)
    autofit_columns(ws)


def apply_data_validation(ws):
    header = [cell.value for cell in ws[1]]
    if VALIDATION_COL not in header:
        return
    col_idx = header.index(VALIDATION_COL) + 1
    max_row = ws.max_row
    dv = DataValidation(type="list", formula1=f'"{VALIDATION_LIST}"', allow_blank=True, showErrorMessage=True)
    rng = f"{get_column_letter(col_idx)}2:{get_column_letter(col_idx)}{max_row}"
    dv.add(rng)
    ws.add_data_validation(dv)


def ensure_visibility_and_cleanup(wb: Workbook):
    # Rename Data to TestPlan (it must already be the active sheet)
    data_ws = wb[wb.sheetnames[0]]
    if data_ws.title != "TestPlan":
        data_ws.title = "TestPlan"
    # Ensure no sheet named Data remains
    if "Data" in wb.sheetnames:
        # If some other sheet named Data exists, delete it
        if wb["Data"].title == "Data":
            wb.remove(wb["Data"])
    # Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist
    allowed = {"TestPlan", "Meta_data_sheet"}
    for name in list(wb.sheetnames):
        if name not in allowed:
            # Do not delete Meta_data_sheet or TestPlan; others should not exist
            if name != "Meta_data_sheet" and name != "TestPlan":
                wb.remove(wb[name])


def main():
    rows_in = parse_json(JSON_DATA)
    # Compute union of keys preserving order
    all_keys = union_keys_preserve_order(rows_in)
    rows = normalize_rows(rows_in, all_keys)

    # Build workbook
    wb = Workbook()
    ws = write_base_sheet(wb, rows, all_keys)

    # Create Meta sheet (raw META columns)
    make_meta_sheet(wb, rows)

    # Rename staging sheet directly and reorder columns (in-place on same sheet)
    ws.title = "TestPlan"  # will be enforced again later
    reorder_to_main_inplace(ws, rows)

    # Apply wrapping, alignment, borders, sizing, and numbering
    apply_wrapping_and_alignment(ws)

    # Data validation on the specific column
    apply_data_validation(ws)

    # Final visibility enforcement
    ensure_visibility_and_cleanup(wb)

    # Save
    os.makedirs(OUTPUT_PATH, exist_ok=True)
    out_file = os.path.join(OUTPUT_PATH, OUTPUT_FILENAME)
    wb.save(out_file)

    # Validate XLSX as ZIP-based OOXML
    if not zipfile.is_zipfile(out_file):
        fail("Generated file is not a valid XLSX (zipfile check failed)")
    try:
        _ = load_workbook(out_file, read_only=True, data_only=True)
    except Exception as e:
        fail(f"Generated XLSX failed openpyxl validation: {e}")

    print(f"OK: Saved {out_file}")


if __name__ == "__main__":
    main()
