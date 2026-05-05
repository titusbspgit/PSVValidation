#!/usr/bin/env python3
import json
import sys
import re
import os
from datetime import datetime, timezone, timedelta
from zipfile import ZipFile
from io import BytesIO
from typing import List

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

def ist_now():
    return datetime.now(timezone(timedelta(hours=5, minutes=30)))

def normalize_json(data):
    if isinstance(data, dict):
        # Preserve insertion order
        rows = list(data.values())
    elif isinstance(data, list):
        rows = data
    else:
        raise ValueError("json_data must be an object or array")
    if not rows:
        raise ValueError("json_data is empty")
    # Build header preserving first-seen order
    header = []
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("Each JSON record must be an object")
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                header.append(k)
    return header, rows


def _len(value):
    if value is None:
        return 0
    s = str(value)
    return len(s)


def autofit(ws):
    # Approximate autofit by max string length per column
    for col in ws.columns:
        maxlen = 0
        col_letter = col[0].column_letter
        for cell in col:
            v = "" if cell.value is None else str(cell.value)
            for line in v.splitlines():
                maxlen = max(maxlen, len(line))
        width = min(max(10, maxlen + 2), 80)  # bound width
        ws.column_dimensions[col_letter].width = width


def thin_border():
    side = Side(style="thin", color="000000")
    return Border(left=side, right=side, top=side, bottom=side)


def style_header(ws):
    blue = PatternFill("solid", fgColor="4472C4")
    for cell in ws[1]:
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = blue


def style_data(ws, header_map):
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.border = thin_border()
            # Alignments
            key = header_map.get(cell.column)
            if key in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif key == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")


def ensure_wrapped_and_renumbered(text: str) -> str:
    if text is None:
        return ""
    # Split by lines; ignore empties at ends
    parts = [p for p in re.split(r"\r?\n", str(text))]
    # If only one line, still normalize markers like "1)" to "1."
    lines: List[str] = []
    for i, raw in enumerate(parts, start=1):
        s = raw.strip()
        if not s:
            continue
        # Remove existing numeric/bullet markers at start
        s = re.sub(r"^([0-9]+)\)\s*", "", s)
        s = re.sub(r"^[\-•\*]\s*", "", s)
        # Prefix with strict numbering
        lines.append(f"{len(lines)+1}. {s}")
    return "\n".join(lines) if lines else ""


def transform_macros(s: str) -> str:
    if not s:
        return s
    tokens = re.split(r"[,\s]+", s)
    out = []
    for t in tokens:
        if not t:
            continue
        if re.match(r"^0x[0-9A-Fa-f]+$", t):
            out.append(t)
        else:
            t2 = t
            if t2.startswith("MIZAR_"):
                t2 = t2[len("MIZAR_"):]
            t2 = t2.replace("_", " ")
            out.append(t2)
    # Collapse multiples and join
    res = ", ".join(out)
    # Append known bitfield notes if relevant
    notes = " (iclr bit16; peie bit17; neie bit18; doe bit20)"
    if notes.strip() not in res:
        res = res + notes
    return res


def main():
    if len(sys.argv) < 2:
        print("Usage: generate_testplan_excel.py <json_path>", file=sys.stderr)
        sys.exit(2)

    json_path = sys.argv[1]
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    header, rows = normalize_json(data)

    # Create workbook with single Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Ensure all keys present in each row
    for key in header:
        pass

    # Write header
    for col, key in enumerate(header, start=1):
        ws.cell(row=1, column=col, value=key)

    # Write rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(header, start=1):
            val = row.get(key, "")
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Base formatting
    style_header(ws)
    ws.freeze_panes = "A2"
    autofit(ws)

    # Create META sheet and copy meta cols
    meta = wb.create_sheet("Meta_data_sheet")
    # Copy headers
    for c_idx, key in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=c_idx, value=key)
    # Build a map from key to column index on Data
    key_to_col = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column+1)}
    for r in range(2, ws.max_row+1):
        for c_idx, key in enumerate(META_COLS, start=1):
            src_col = key_to_col.get(key)
            meta.cell(row=r, column=c_idx, value=ws.cell(row=r, column=src_col).value if src_col else "")
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Transform main sheet in place: rename and drop meta columns, reorder
    ws.title = "TestPlan"

    # Remove meta columns from TestPlan sheet
    # Gather columns to delete by index (from right to left)
    delete_cols = sorted([key_to_col[k] for k in META_COLS if k in key_to_col], reverse=True)
    for dc in delete_cols:
        ws.delete_cols(dc, 1)
    
    # Rebuild header index after deletions
    header_after = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column+1)]
    col_index = {name: idx for idx, name in enumerate(header_after)}

    # Ensure all MAIN_ORDER columns exist; if missing, append blank column with that header
    for name in MAIN_ORDER:
        if name not in col_index:
            ws.cell(row=1, column=ws.max_column+1, value=name)
            for r in range(2, ws.max_row+1):
                ws.cell(row=r, column=ws.max_column, value="")
            header_after.append(name)
            col_index[name] = len(header_after)-1

    # Reorder columns to MAIN_ORDER
    desired = MAIN_ORDER
    # Create a new temporary data array
    data_matrix = []
    for r in range(2, ws.max_row+1):
        row_vals = {}
        for i, name in enumerate(header_after):
            row_vals[name] = ws.cell(row=r, column=i+1).value
        data_matrix.append(row_vals)
    # Clear all columns
    ws.delete_cols(1, ws.max_column)
    # Write desired header
    for c, name in enumerate(desired, start=1):
        ws.cell(row=1, column=c, value=name)
    # Write back rows
    for r_idx, row in enumerate(data_matrix, start=2):
        for c, name in enumerate(desired, start=1):
            ws.cell(row=r_idx, column=c, value=row.get(name, ""))

    # Apply numbering and macro replacement ONLY on visible sheet
    header_map = {c: ws.cell(row=1, column=c).value for c in range(1, ws.max_column+1)}
    # Find indices
    col_name_to_idx = {v: k for k, v in header_map.items()}

    steps_col = col_name_to_idx.get("Test Steps / Procedure")
    val_col = col_name_to_idx.get("Validation / Acceptance Criteria")
    impacted_col = col_name_to_idx.get("Impacted Registers")

    # Load meta impacted values from meta sheet
    meta_impacted = {}
    # Build meta header map
    meta_hdr = {meta.cell(row=1, column=c).value: c for c in range(1, meta.max_column+1)}
    mi_col = meta_hdr.get("Hidden_Impacted_Registers")
    for r in range(2, meta.max_row+1):
        meta_impacted[r] = meta.cell(row=r, column=mi_col).value if mi_col else ""

    for r in range(2, ws.max_row+1):
        if steps_col:
            ws.cell(row=r, column=steps_col, value=ensure_wrapped_and_renumbered(ws.cell(row=r, column=steps_col).value))
        if val_col:
            ws.cell(row=r, column=val_col, value=ensure_wrapped_and_renumbered(ws.cell(row=r, column=val_col).value))
        if impacted_col:
            cur = ws.cell(row=r, column=impacted_col).value
            if not cur:
                raw = meta_impacted.get(r, "")
                ws.cell(row=r, column=impacted_col, value=transform_macros(raw))

    # Formatting
    style_header(ws)
    style_data(ws, header_map)
    autofit(ws)
    # Approx row height by counting lines for wrapped cols
    for r in range(2, ws.max_row+1):
        max_lines = 1
        for name in WRAP_COLS:
            cidx = col_name_to_idx.get(name)
            if cidx:
                v = ws.cell(row=r, column=cidx).value
                if v:
                    max_lines = max(max_lines, str(v).count("\n") + 1)
        ws.row_dimensions[r].height = min(15 * max_lines, 200)

    # Data validation on Code Generation (Required / Not)
    cg_col = col_name_to_idx.get("Code Generation (Required / Not)")
    if cg_col:
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
        start = 2
        end = ws.max_row
        col_letter = ws.cell(row=1, column=cg_col).column_letter
        dv.add(f"{col_letter}{start}:{col_letter}{end}")
        ws.add_data_validation(dv)

    # Safety check: only TestPlan and veryHidden Meta_data_sheet
    for s in wb.worksheets:
        if s.title == "Data":
            # Should not exist
            wb.remove(s)
    # Validate XLSX structure after save
    ist = ist_now()
    out_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
    os.makedirs(out_dir, exist_ok=True)
    fname = f"GPIO_TestPlan_{ist.strftime('%Y%m%d')}_{ist.strftime('%H%M%S')}\.xlsx"
    # Remove backslash escape if any
    fname = fname.replace('\\.', '.')
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)

    # Validate ZIP structure
    with ZipFile(out_path, 'r') as zf:
        if 'xl/workbook.xml' not in zf.namelist():
            raise RuntimeError('Invalid XLSX: workbook.xml missing')

    # Print relative path for GitHub Action to consume
    print(out_path)

if __name__ == "__main__":
    main()
