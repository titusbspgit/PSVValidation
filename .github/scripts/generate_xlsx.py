#!/usr/bin/env python3
import os
import sys
import json
import base64
import zipfile
import re
from collections import OrderedDict
from typing import List, Dict, Any

from openpyxl import Workbook, load_workbook
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

MAIN_COLS = [
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

HEADER_FILL = PatternFill(fill_type="solid", start_color="4472C4", end_color="4472C4")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def fail(msg: str) -> None:
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def parse_json_from_env() -> List[OrderedDict]:
    b64 = os.environ.get("JSON_B64", "").strip()
    if not b64:
        fail("JSON_B64 environment variable is empty")
    try:
        raw = base64.b64decode(b64).decode("utf-8")
    except Exception as e:
        fail(f"Failed to decode JSON_B64: {e}")
    try:
        data = json.loads(raw, object_pairs_hook=OrderedDict)
    except Exception as e:
        fail(f"Invalid JSON input: {e}")
    if not isinstance(data, list) or len(data) == 0:
        fail("JSON must be a non-empty array of objects")
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            fail(f"Element at index {i} is not an object")
    return data


def build_schema_union(rows: List[OrderedDict]) -> List[str]:
    cols: List[str] = []
    seen = set()
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def write_data_sheet(ws, columns: List[str], rows: List[OrderedDict]):
    # Header
    for ci, h in enumerate(columns, start=1):
        c = ws.cell(row=1, column=ci, value=h)
        c.font = Font(bold=True, color="FFFFFF")
        c.alignment = Alignment(horizontal="center", vertical="center")
        c.fill = HEADER_FILL
        c.border = THIN_BORDER
    # Rows
    for ri, row in enumerate(rows, start=2):
        for ci, h in enumerate(columns, start=1):
            val = row.get(h, "")
            c = ws.cell(row=ri, column=ci, value=val)
            c.border = THIN_BORDER
    # Freeze header
    ws.freeze_panes = "A2"


def auto_fit_columns(ws):
    # Estimate width by maximum display length per column
    col_max = {}
    for row in ws.iter_rows(values_only=True):
        for idx, v in enumerate(row, start=1):
            s = "" if v is None else str(v)
            # for wrapped content, consider longest line
            s_len = max((len(line) for line in s.splitlines()), default=0)
            col_max[idx] = max(col_max.get(idx, 0), s_len)
    for idx, max_len in col_max.items():
        letter = get_column_letter(idx)
        # add padding, clamp to a reasonable max
        width = min(max_len + 2, 120)
        if width < 10:
            width = 10
        ws.column_dimensions[letter].width = width


def create_meta_sheet(wb: Workbook, rows: List[OrderedDict]):
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for ci, h in enumerate(META_COLS, start=1):
        c = ws_meta.cell(row=1, column=ci, value=h)
        c.font = Font(bold=True)
    # Data
    for ri, row in enumerate(rows, start=2):
        for ci, h in enumerate(META_COLS, start=1):
            ws_meta.cell(row=ri, column=ci, value=row.get(h, ""))
    # Very hidden
    ws_meta.sheet_state = "veryHidden"


def renumber_text(value: Any) -> Any:
    if value is None:
        return value
    if not isinstance(value, str):
        return value
    text = value.strip()
    if not text:
        return text
    # Split on newlines or semicolons
    parts = re.split(r"[\n;]+", text)
    items = []
    for p in parts:
        t = p.strip()
        if not t:
            continue
        # remove leading bullets/numbers like '1)', '1.', '-', '*', '•'
        t = re.sub(r"^([0-9]+[\.)]+\s*|[-*•]\s*)", "", t)
        items.append(t)
    if not items:
        return ""
    numbered = [f"{i}. {it}" for i, it in enumerate(items, start=1)]
    return "\n".join(numbered)


def normalize_to_main(ws, original_rows: List[OrderedDict]):
    # Build normalized rows mapping to MAIN_COLS in order, dropping anything else
    normalized: List[List[Any]] = []
    for row in original_rows:
        norm_row = []
        for h in MAIN_COLS:
            norm_row.append(row.get(h, ""))
        normalized.append(norm_row)

    # Clear existing content and write header + data back into the SAME worksheet
    ws.delete_rows(1, ws.max_row)
    for ci, h in enumerate(MAIN_COLS, start=1):
        c = ws.cell(row=1, column=ci, value=h)
        c.font = Font(bold=True, color="FFFFFF")
        c.alignment = Alignment(horizontal="center", vertical="center")
        c.fill = HEADER_FILL
        c.border = THIN_BORDER
    for ri, values in enumerate(normalized, start=2):
        for ci, v in enumerate(values, start=1):
            ws.cell(row=ri, column=ci, value=v).border = THIN_BORDER


def apply_main_formatting(ws):
    max_row = ws.max_row
    max_col = ws.max_column

    # Header alignment already set; ensure header border also set
    for ci in range(1, max_col + 1):
        c = ws.cell(row=1, column=ci)
        c.alignment = Alignment(horizontal="center", vertical="center")
        c.font = Font(bold=True, color="FFFFFF")
        c.fill = HEADER_FILL
        c.border = THIN_BORDER

    # Data rows: alignment and wrapping
    headers = [ws.cell(row=1, column=ci).value for ci in range(1, max_col + 1)]
    header_idx = {h: i + 1 for i, h in enumerate(headers)}

    for ri in range(2, max_row + 1):
        for ci in range(1, max_col + 1):
            h = headers[ci - 1]
            cell = ws.cell(row=ri, column=ci)
            # Borders on all populated cells (header also done)
            cell.border = THIN_BORDER
            # Vertical top for all data rows
            valign = "top"
            # Alignment rules
            if h == "Index":
                cell.alignment = Alignment(horizontal="center", vertical=valign, wrap_text=False)
            elif h in WRAP_COLS:
                cell.alignment = Alignment(horizontal="left", vertical=valign, wrap_text=True)
            else:
                # left align by default
                cell.alignment = Alignment(horizontal="left", vertical=valign, wrap_text=False)

    # Auto-fit columns and approximate row heights for wrapped text
    auto_fit_columns(ws)

    # Adjust row heights based on wrapped content line counts
    base_height = 15
    wrap_indexes = [header_idx[c] for c in WRAP_COLS if c in header_idx]
    for ri in range(2, max_row + 1):
        max_lines = 1
        for ci in wrap_indexes:
            v = ws.cell(row=ri, column=ci).value
            s = "" if v is None else str(v)
            lines = max(1, s.count("\n") + 1)
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[ri].height = base_height * max_lines

    # Freeze top row
    ws.freeze_panes = "A2"


def add_code_generation_validation(ws):
    # Apply ONLY to data rows, ONLY to Code Generation (Required / Not)
    headers = [ws.cell(row=1, column=ci).value for ci in range(1, ws.max_column + 1)]
    try:
        col_idx = headers.index("Code Generation (Required / Not)") + 1
    except ValueError:
        return  # column not present; nothing to do
    if ws.max_row <= 1:
        return
    start_row = 2
    end_row = ws.max_row
    col_letter = get_column_letter(col_idx)
    rng = f"{col_letter}{start_row}:{col_letter}{end_row}"
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
    dv.error = "Select a value from the list"
    dv.errorTitle = "Invalid Input"
    ws.add_data_validation(dv)
    dv.add(rng)


def enforce_final_sheets(wb: Workbook):
    names = [ws.title for ws in wb.worksheets]
    if "Data" in names:
        fail("Sheet named 'Data' exists after normalization")
    # Enforce only TestPlan and Meta_data_sheet exist
    allowed = {"TestPlan", "Meta_data_sheet"}
    if set(names) != allowed:
        # Attempt to delete any extra sheets if present (except allowed ones)
        for ws in list(wb.worksheets):
            if ws.title not in allowed:
                wb.remove(ws)
        # Re-evaluate
        names = [ws.title for ws in wb.worksheets]
        if set(names) != allowed:
            fail(f"Unexpected worksheets present: {names}")
    # Meta sheet must be veryHidden
    meta = wb["Meta_data_sheet"]
    if getattr(meta, "sheet_state", "visible") != "veryHidden":
        fail("Meta_data_sheet is not Very Hidden")


def validate_xlsx(path: str):
    if not os.path.isfile(path):
        fail(f"File not found after save: {path}")
    if not zipfile.is_zipfile(path):
        fail("Saved file is not a valid ZIP-based XLSX")
    # Try loading via openpyxl
    try:
        wb2 = load_workbook(path, read_only=True, data_only=True)
    except Exception as e:
        fail(f"openpyxl failed to load workbook: {e}")
    try:
        names = wb2.sheetnames
    finally:
        wb2.close()
    if "TestPlan" not in names or "Meta_data_sheet" not in names:
        fail("Workbook does not contain required sheets after reload")


def main():
    out_rel = os.environ.get("OUTPUT_FILE_REL", "").strip()
    if not out_rel:
        fail("OUTPUT_FILE_REL environment variable is empty")

    rows = parse_json_from_env()

    # PHASE 1: Create base workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    schema_cols = build_schema_union(rows)
    write_data_sheet(ws, schema_cols, rows)
    auto_fit_columns(ws)

    # PHASE 2: Create META sheet (Very Hidden)
    create_meta_sheet(wb, rows)

    # Rename Data -> TestPlan (no new visible sheet)
    ws.title = "TestPlan"

    # Normalize MAIN sheet: remove META, enforce MAIN order, drop extras
    normalize_to_main(ws, rows)

    # Numbering in specific cells on TestPlan only
    headers = [ws.cell(row=1, column=ci).value for ci in range(1, ws.max_column + 1)]
    idx_steps = headers.index("Test Steps / Procedure") + 1 if "Test Steps / Procedure" in headers else None
    idx_valac = headers.index("Validation / Acceptance Criteria") + 1 if "Validation / Acceptance Criteria" in headers else None
    for ri in range(2, ws.max_row + 1):
        if idx_steps:
            cell = ws.cell(row=ri, column=idx_steps)
            cell.value = renumber_text(cell.value)
        if idx_valac:
            cell = ws.cell(row=ri, column=idx_valac)
            cell.value = renumber_text(cell.value)

    # Apply strict formatting
    apply_main_formatting(ws)

    # Data validation
    add_code_generation_validation(ws)

    # Final sheet visibility enforcement
    enforce_final_sheets(wb)

    # Ensure output directory exists
    out_path = out_rel
    out_dir = os.path.dirname(out_path)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    # Save and validate
    wb.save(out_path)
    validate_xlsx(out_path)

    # Summary to stdout
    print(json.dumps({
        "status": "SUCCESS",
        "rows": len(rows),
        "columns": len(schema_cols),
        "path": out_rel
    }))


if __name__ == "__main__":
    main()
