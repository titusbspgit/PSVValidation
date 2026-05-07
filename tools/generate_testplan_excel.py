#!/usr/bin/env python3
import argparse, json, os, sys, re, io, zipfile
from copy import deepcopy
from typing import List, Dict, Any

from openpyxl import Workbook, load_workbook
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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

ALLOWED_DV = ["Required", "Blank", "Not Required"]


def load_json_records(path: str) -> List[Dict[str, Any]]:
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if isinstance(data, dict):
        # Convert dict to list in deterministic order using Index if present
        items = list(data.values())
        try:
            items.sort(key=lambda x: x.get("Index", 0))
        except Exception:
            pass
        return items
    if isinstance(data, list):
        return data
    raise ValueError("json_data must be an array or object of rows")


def read_existing_rows(xlsx_path: str) -> List[Dict[str, Any]]:
    rows = []
    if not os.path.exists(xlsx_path):
        return rows
    try:
        wb = load_workbook(xlsx_path, data_only=True)
        if 'TestPlan' not in wb.sheetnames:
            return rows
        ws = wb['TestPlan']
        headers = [c.value for c in ws[1] if c.value is not None]
        for r in ws.iter_rows(min_row=2, values_only=True):
            if all(v is None for v in r):
                continue
            rec = {}
            for i, h in enumerate(headers):
                if i < len(r):
                    rec[h] = r[i]
                else:
                    rec[h] = ""
            rows.append(rec)
    except Exception:
        # If any issue reading, treat as no existing rows
        rows = []
    return rows


def normalize_schema(records: List[Dict[str, Any]]) -> List[str]:
    headers: List[str] = []
    seen = set()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                headers.append(k)
    return headers


def to_cell_value(v: Any) -> Any:
    if v is None:
        return ""
    if isinstance(v, (list, dict)):
        return json.dumps(v, ensure_ascii=False)
    return v


def ensure_numbered_block(text: str) -> str:
    if text is None:
        return ""
    # Split on newlines; strip bullets like '1)', '1.', '-', '*'
    lines = [l.strip() for l in str(text).splitlines()]
    out = []
    idx = 1
    for l in lines:
        if not l:
            continue
        l = re.sub(r"^\s*(\d+)[\.)]\s*", "", l)
        l = re.sub(r"^\s*[-*]\s*", "", l)
        out.append(f"{idx}. {l}")
        idx += 1
    return "\n".join(out)


def autosize_columns(ws):
    from openpyxl.utils import get_column_letter
    dims = {}
    for row in ws.iter_rows(values_only=True):
        for i, v in enumerate(row, start=1):
            s = str(v) if v is not None else ""
            l = len(s)
            dims[i] = max(dims.get(i, 0), l)
    for i, l in dims.items():
        width = max(10, min(60, int(l * 1.1) + 2))
        ws.column_dimensions[get_column_letter(i)].width = width


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    max_row = ws.max_row
    max_col = ws.max_column
    for r in ws.iter_rows(min_row=1, max_row=max_row, min_col=1, max_col=max_col):
        for cell in r:
            cell.border = border


def set_alignment(ws):
    # Header: bold, center both
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    header_fill = PatternFill("solid", fgColor="0070C0")
    for cell in ws[1]:
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill
    # Data rows alignment
    max_row = ws.max_row
    max_col = ws.max_column
    # Determine column indices
    hdrs = [c.value for c in ws[1]]
    idx_col = None
    for i, h in enumerate(hdrs, start=1):
        if h == "Index":
            idx_col = i
    for row in ws.iter_rows(min_row=2, max_row=max_row, min_col=1, max_col=max_col):
        for i, cell in enumerate(row, start=1):
            if i == idx_col:
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)


def wrap_text_for_columns(ws, columns: List[str]):
    hdrs = [c.value for c in ws[1]]
    cols_idx = [hdrs.index(c) + 1 for c in columns if c in hdrs]
    for row in ws.iter_rows(min_row=2):
        for i in cols_idx:
            row[i-1].alignment = Alignment(wrap_text=True, vertical="top")


def add_validation(ws):
    hdrs = [c.value for c in ws[1]]
    if "Code Generation (Required / Not)" not in hdrs:
        return
    col = hdrs.index("Code Generation (Required / Not)") + 1
    max_row = ws.max_row
    from openpyxl.utils import get_column_letter
    col_letter = get_column_letter(col)
    rng = f"{col_letter}2:{col_letter}{max_row}"
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
    ws.add_data_validation(dv)
    dv.add(rng)


def validate_xlsx_bytes(buf: bytes) -> bool:
    if not zipfile.is_zipfile(io.BytesIO(buf)):
        return False
    try:
        bio = io.BytesIO(buf)
        wb = load_workbook(bio)
        _ = wb.sheetnames
    except Exception:
        return False
    return True


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--input', required=True)
    ap.add_argument('--output', required=True)
    args = ap.parse_args()

    # Load incoming json
    incoming = load_json_records(args.input)
    if not incoming:
        print("ERROR: Empty json_data", file=sys.stderr)
        sys.exit(2)

    # If existing Excel exists, read and merge
    existing = read_existing_rows(args.output)
    merged: List[Dict[str, Any]] = []
    if existing:
        merged.extend(existing)
    merged.extend(incoming)

    # Normalize schema across merged
    headers = normalize_schema(merged)

    # Build workbook
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write header preserving header order
    ws.append(headers)
    ws.freeze_panes = 'A2'

    # Write rows preserving order
    for rec in merged:
        row = [to_cell_value(rec.get(h, "")) for h in headers]
        ws.append(row)

    # Create META sheet
    meta_ws = wb.create_sheet('Meta_data_sheet')
    meta_ws.append(META_COLS)
    for rec in merged:
        meta_ws.append([to_cell_value(rec.get(k, "")) for k in META_COLS])
    # Very hide meta sheet
    meta_ws.sheet_state = 'veryHidden'

    # Rename Data -> TestPlan and transform in place
    ws = wb['Data']
    ws.title = 'TestPlan'

    # Remove META columns from main and reorder to MAIN_COL_ORDER
    # Create a mapping of current headers to values per row, then rewrite sheet
    current_headers = [c.value for c in ws[1]]
    data_rows = []
    for r in ws.iter_rows(min_row=2, values_only=True):
        if all(v is None for v in r):
            continue
        d = {current_headers[i]: r[i] if i < len(r) else "" for i in range(len(current_headers))}
        data_rows.append(d)

    # Clear sheet
    ws.delete_rows(1, ws.max_row)

    # Final headers (MAIN order)
    final_headers = [h for h in MAIN_COL_ORDER]
    ws.append(final_headers)

    # Numbering function for steps and validation
    def numbered(val):
        return ensure_numbered_block(val)

    for d in data_rows:
        out = []
        for h in final_headers:
            val = d.get(h, "")
            if h in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = numbered(val)
            out.append(to_cell_value(val))
        ws.append(out)

    # Formatting
    set_alignment(ws)
    wrap_text_for_columns(ws, list(WRAP_COLS))
    autosize_columns(ws)
    apply_borders(ws)

    # Data validation on single column
    add_validation(ws)

    # Safety check: only TestPlan (visible) and Meta_data_sheet (veryHidden)
    if 'Data' in wb.sheetnames:
        # Delete if any stray sheet named Data exists
        del wb['Data']
    # Enforce visibility states
    if 'TestPlan' not in wb.sheetnames or 'Meta_data_sheet' not in wb.sheetnames:
        print("ERROR: Required sheets missing after normalization", file=sys.stderr)
        sys.exit(3)

    # Save to bytes then validate
    buf = io.BytesIO()
    wb.save(buf)
    bts = buf.getvalue()
    if not validate_xlsx_bytes(bts):
        print("ERROR: XLSX validation failed", file=sys.stderr)
        sys.exit(4)

    # Ensure directory exists
    out_path = args.output
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, 'wb') as f:
        f.write(bts)

    print(f"SUCCESS: Wrote Excel to {out_path}")

if __name__ == '__main__':
    main()
