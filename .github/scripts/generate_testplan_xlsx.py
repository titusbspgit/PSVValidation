#!/usr/bin/env python3
import argparse, json, re, sys, zipfile
from pathlib import Path
from typing import List, Dict, Any

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"Missing dependency: {e}", file=sys.stderr)
    sys.exit(2)

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

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

ALLOWED_VALIDATION_VALUES = "Required,Blank,Not Required"


def validate_json(records: Any) -> List[Dict[str, Any]]:
    if not isinstance(records, list):
        raise ValueError("JSON root must be an array")
    if len(records) == 0:
        raise ValueError("JSON array is empty")
    for i, rec in enumerate(records, 1):
        if not isinstance(rec, dict):
            raise ValueError(f"Item at index {i} is not an object")
    return records


def collect_headers_order(records: List[Dict[str, Any]]) -> List[str]:
    seen = []
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.append(k)
    return seen


def autosize_columns(ws):
    max_len = {}
    for row in ws.iter_rows(values_only=True):
        for idx, val in enumerate(row, 1):
            s = "" if val is None else str(val)
            # consider line breaks
            for part in s.splitlines() or [""]:
                max_len[idx] = max(max_len.get(idx, 0), len(part))
    for idx, length in max_len.items():
        # heuristic width: characters + padding; clamp
        width = min(max(length + 2, 12), 80)
        ws.column_dimensions[get_column_letter(idx)].width = width


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            if cell.value is not None and str(cell.value) != "":
                cell.border = border


def normalize_numbering(text: str) -> str:
    if text is None:
        return ""
    lines = [ln for ln in str(text).splitlines()]
    # filter empty-only whitespace lines
    items = [ln.strip() for ln in lines if ln.strip() != ""]
    out = []
    for i, ln in enumerate(items, 1):
        # remove leading bullets like '1)', '1.', '-', '*', etc.
        ln2 = re.sub(r"^\s*(?:[-*•\u2022\u25CF]|\d+[\.)])\s*", "", ln)
        out.append(f"{i}. {ln2}")
    return "\n".join(out) if out else ""


def set_header_style(ws):
    header_font = Font(bold=True)
    header_fill = PatternFill(fill_type="solid", start_color="4472C4", end_color="4472C4")
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align


def set_data_styles(ws, main_cols_index_map):
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            key = main_cols_index_map.get(c)
            # defaults
            align = Alignment(vertical="top", horizontal="left", wrap_text=(key in WRAP_COLUMNS))
            val = cell.value
            # numeric alignment for 'Index'
            if key == "Index":
                align = Alignment(vertical="top", horizontal="center", wrap_text=(key in WRAP_COLUMNS))
            elif isinstance(val, (int, float)):
                align = Alignment(vertical="top", horizontal="right", wrap_text=(key in WRAP_COLUMNS))
            cell.alignment = align


def rewrite_main_sheet_in_place(ws, records):
    # Build ordered main data rows
    rows = []
    for rec in records:
        row = [rec.get(col, "") for col in MAIN_COLUMNS]
        # numbering transforms on two columns inside visible main sheet only
        ts_idx = MAIN_COLUMNS.index("Test Steps / Procedure")
        va_idx = MAIN_COLUMNS.index("Validation / Acceptance Criteria")
        row[ts_idx] = normalize_numbering(row[ts_idx])
        row[va_idx] = normalize_numbering(row[va_idx])
        rows.append(row)

    # Clear and write
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)
    ws.append(MAIN_COLUMNS)
    for r in rows:
        ws.append(r)

    # Styles
    set_header_style(ws)
    ws.freeze_panes = "A2"
    # Wrap and alignment; build index map
    main_cols_index_map = {i + 1: k for i, k in enumerate(MAIN_COLUMNS)}
    set_data_styles(ws, main_cols_index_map)
    autosize_columns(ws)
    # approximate row height based on wrapped lines
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            text = str(cell.value) if cell.value is not None else ""
            lines = len(text.splitlines()) if cell.alignment and cell.alignment.wrap_text else 1
            max_lines = max(max_lines, lines)
        ws.row_dimensions[r].height = min(15 * max_lines, 300)
    apply_borders(ws)

    # Data validation for Code Generation column only
    code_col_index = MAIN_COLUMNS.index("Code Generation (Required / Not)") + 1
    dv = DataValidation(type="list", formula1=f'"{ALLOWED_VALIDATION_VALUES}"', allow_blank=True)
    ws.add_data_validation(dv)
    col_letter = get_column_letter(code_col_index)
    dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")


def build_meta_sheet(wb, records, all_headers):
    # Determine META columns from union of keys starting with 'Hidden_'; preserve first-seen order
    meta_cols = [h for h in all_headers if h.startswith("Hidden_")]
    ws_meta = wb.create_sheet("Meta_data_sheet")
    ws_meta.append(meta_cols)
    for rec in records:
        ws_meta.append([rec.get(k, "") for k in meta_cols])
    # Very hidden
    ws_meta.sheet_state = "veryHidden"
    autosize_columns(ws_meta)
    apply_borders(ws_meta)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", required=True, help="Path to input JSON array file")
    ap.add_argument("--outdir", required=True, help="Output directory for XLSX")
    ap.add_argument("--filename", required=True, help="Output XLSX file name")
    args = ap.parse_args()

    data = json.load(open(args.json, "r", encoding="utf-8"))
    records = validate_json(data)

    headers = collect_headers_order(records)

    # Create workbook and authoritative staging sheet 'Data'
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    ws.append(headers)
    for rec in records:
        ws.append([rec.get(h, "") for h in headers])

    # Base formatting on staging
    set_header_style(ws)
    ws.freeze_panes = "A2"
    autosize_columns(ws)
    apply_borders(ws)

    # META sheet
    build_meta_sheet(wb, records, headers)

    # STEP 7: Normalize MAIN sheet on same worksheet and then rename to TestPlan
    rewrite_main_sheet_in_place(ws, records)
    wb.active = wb.sheetnames.index("Data")
    ws.title = "TestPlan"

    # STEP 7B: Safety - ensure no sheet named 'Data' remains
    for s in list(wb.sheetnames):
        if s == "Data":
            del wb[s]

    # Enforce only allowed worksheets
    allowed = {"TestPlan", "Meta_data_sheet"}
    for name in list(wb.sheetnames):
        if name not in allowed:
            del wb[name]

    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    outpath = outdir / args.filename

    wb.save(outpath)

    # Validation: ZIP and load with openpyxl
    with zipfile.ZipFile(outpath, 'r') as zf:
        bad = zf.testzip()
        if bad is not None:
            raise RuntimeError(f"Corrupted XLSX (bad file in zip): {bad}")
    load_workbook(outpath)

    print(str(outpath))

if __name__ == "__main__":
    main()
