#!/usr/bin/env python3
import os, json, io, sys, zipfile, datetime
from zoneinfo import ZoneInfo
from collections import OrderedDict
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

ALLOWED_DV = "Required,Blank,Not Required"


def fail(msg: str):
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def load_json_from_env() -> list:
    raw = os.environ.get("JSON_DATA", "").strip()
    if not raw:
        fail("JSON_DATA environment variable is empty")
    try:
        data = json.loads(raw)
    except Exception as e:
        fail(f"Invalid JSON input: {e}")
    if not isinstance(data, list) or len(data) == 0:
        fail("JSON must be a non-empty array of objects")
    for i, r in enumerate(data):
        if not isinstance(r, dict):
            fail(f"JSON array element at index {i} is not an object")
    return data


def ordered_union_keys(rows: list) -> list:
    seen = OrderedDict()
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def numberize_block(text: str) -> str:
    if text is None:
        return ""
    s = str(text)
    # Split by lines; filter blanks; renumber strictly (1., 2., ...)
    parts = [p.strip() for p in s.replace("\r", "").split("\n")]
    parts = [p for p in parts if p]
    if not parts:
        return ""
    return "\n".join(f"{i+1}. {parts[i]}" for i in range(len(parts)))


def apply_base_formatting(ws, max_col, max_row):
    # Header formatting
    header_font = Font(bold=True)
    header_fill = PatternFill(fill_type="solid", fgColor="4472C4")
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    thin = Side(style="thin")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Apply header styles and borders
    for col in range(1, max_col + 1):
        c = ws.cell(row=1, column=col)
        c.font = header_font
        c.fill = header_fill
        c.alignment = header_align
        c.border = border

    # Data row formatting and borders
    data_align_left_top = Alignment(horizontal="left", vertical="top", wrap_text=True)
    data_align_center_top = Alignment(horizontal="center", vertical="top", wrap_text=True)

    # Determine columns that should be centered (Index only, numbers optional)
    center_cols_by_title = {"Index"}
    title_by_col = {i: ws.cell(1, i).value for i in range(1, max_col + 1)}

    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            title = title_by_col[c]
            if title in center_cols_by_title:
                cell.alignment = data_align_center_top
            else:
                cell.alignment = data_align_left_top
            cell.border = border

    # Wrap text for specific columns
    for c in range(1, max_col + 1):
        title = title_by_col[c]
        if title in WRAP_COLS:
            for r in range(2, max_row + 1):
                ws.cell(r, c).alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)

    # Auto-fit columns (approximate with character count)
    for c in range(1, max_col + 1):
        col_letter = get_column_letter(c)
        max_len = 0
        for r in range(1, max_row + 1):
            v = ws.cell(r, c).value
            s = str(v) if v is not None else ""
            lines = s.split("\n")
            width = max(len(line) for line in lines) if lines else 0
            if width > max_len:
                max_len = width
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 80)

    # Auto-fit row heights (rough estimate: 15 per line)
    for r in range(1, max_row + 1):
        max_lines = 1
        for c in range(1, max_col + 1):
            v = ws.cell(r, c).value
            s = str(v) if v is not None else ""
            lines = s.count("\n") + 1 if s else 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = min(15 * max_lines, 300)


def main():
    data = load_json_from_env()

    # Build key union preserving first-seen order
    all_keys = ordered_union_keys(data)

    # Create workbook and staging sheet 'Data'
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers
    for col_idx, key in enumerate(all_keys, start=1):
        ws.cell(row=1, column=col_idx, value=key)

    # Write rows (preserve values exactly)
    for row_idx, rec in enumerate(data, start=2):
        for col_idx, key in enumerate(all_keys, start=1):
            ws.cell(row=row_idx, column=col_idx, value=rec.get(key, ""))

    # Base formatting on Data sheet before transformations
    apply_base_formatting(ws, len(all_keys), len(data) + 1)

    # Create Meta_data_sheet and copy META columns AS-IS
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    for col_idx, key in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=col_idx, value=key)
    for row_idx, rec in enumerate(data, start=2):
        for col_idx, key in enumerate(META_COLS, start=1):
            meta_ws.cell(row=row_idx, column=col_idx, value=rec.get(key, ""))
    # Very hidden
    meta_ws.sheet_state = "veryHidden"

    # STEP 7 — Normalize MAIN sheet on the same worksheet: rename Data -> TestPlan
    ws.title = "TestPlan"

    # Remove META columns and reorder to MAIN_ORDER on the same sheet
    # Build a mapping of current header -> column index
    current_headers = [ws.cell(1, c).value for c in range(1, ws.max_column + 1)]
    # Build the matrix of current data
    matrix = []
    for r in range(2, ws.max_row + 1):
        row_dict = {}
        for c, hdr in enumerate(current_headers, start=1):
            row_dict[hdr] = ws.cell(r, c).value
        matrix.append(row_dict)

    # Clear sheet content
    ws.delete_rows(1, ws.max_row)

    # Write MAIN_ORDER headers
    for col_idx, key in enumerate(MAIN_ORDER, start=1):
        ws.cell(row=1, column=col_idx, value=key)

    # Write MAIN_ORDER rows (filling blanks for missing keys)
    for row_idx, row_dict in enumerate(matrix, start=2):
        for col_idx, key in enumerate(MAIN_ORDER, start=1):
            val = row_dict.get(key, "")
            # Numbering enforcement for two columns on TestPlan only
            if key in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = numberize_block(val)
            ws.cell(row=row_idx, column=col_idx, value=val)

    # Apply strict formatting now to TestPlan
    apply_base_formatting(ws, len(MAIN_ORDER), len(matrix) + 1)

    # Data validation ONLY for 'Code Generation (Required / Not)' on data rows
    try:
        code_col_idx = MAIN_ORDER.index("Code Generation (Required / Not)") + 1
        code_col_letter = get_column_letter(code_col_idx)
        dv = DataValidation(type="list", formula1=f'"{ALLOWED_DV}"', allow_blank=True)
        dv.error = "Select a value from the list"
        dv.errorTitle = "Invalid Input"
        ws.add_data_validation(dv)
        if len(matrix) >= 1:
            dv.add(f"{code_col_letter}2:{code_col_letter}{len(matrix)+1}")
    except ValueError:
        pass  # Column missing, skip DV

    # Enforce final sheet visibility: only TestPlan (visible) and Meta_data_sheet (veryHidden)
    # Ensure no sheet named 'Data' remains
    for sh in list(wb.sheetnames):
        if sh == "Data":
            # delete if any
            del wb[sh]

    if set(wb.sheetnames) - {"TestPlan", "Meta_data_sheet"}:
        fail("Unexpected worksheets present after normalization")

    # Compute IST timestamp and final path
    ip_name = os.environ.get("IP_NAME", "GPIO").strip() or "GPIO"
    dest_dir = os.path.normpath(os.environ.get("DEST_DIR", "Test_Output/GPIO/TestPlan").strip() or "Test_Output/GPIO/TestPlan")
    tz = ZoneInfo("Asia/Kolkata")
    now = datetime.datetime.now(tz)
    stamp_date = now.strftime("%Y%m%d")
    stamp_time = now.strftime("%H%M%S")
    final_filename = f"{ip_name}_TestPlan_{stamp_date}_{stamp_time}.xlsx"
    final_path = os.path.join(dest_dir, final_filename)

    # Ensure directory exists
    os.makedirs(dest_dir, exist_ok=True)

    # Save workbook to file
    wb.save(final_path)

    # Validate as a real XLSX (ZIP with core parts)
    with zipfile.ZipFile(final_path, 'r') as zf:
        names = set(zf.namelist())
        core_parts = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
        missing = [p for p in core_parts if p not in names]
        if missing:
            fail(f"XLSX validation failed, missing parts: {missing}")

    # Output the path for subsequent commit step
    gh_out = os.environ.get("GITHUB_OUTPUT")
    if gh_out:
        with open(gh_out, "a", encoding="utf-8") as f:
            f.write(f"excel_path={final_path}\n")
    print(f"Generated: {final_path}")


if __name__ == "__main__":
    main()
