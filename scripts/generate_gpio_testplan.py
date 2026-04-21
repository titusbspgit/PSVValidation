#!/usr/bin/env python3
import json, sys, os
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, Border, Side
from openpyxl.utils import get_column_letter

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

WRAP_COLUMNS = set([
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
])


def to_cell_value(v):
    if v is None:
        return ""
    if isinstance(v, list):
        return "\n".join(str(x) for x in v)
    return str(v)


def autosize_columns(ws):
    for col in range(1, ws.max_column + 1):
        col_letter = get_column_letter(col)
        max_len = 0
        for row in range(1, ws.max_row + 1):
            val = ws.cell(row=row, column=col).value
            if val is None:
                continue
            val = str(val)
            if "\n" in val:
                width = max(len(line) for line in val.split("\n"))
            else:
                width = len(val)
            if width > max_len:
                max_len = width
        ws.column_dimensions[col_letter].width = min(max_len + 4, 120)


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def main():
    if len(sys.argv) < 4:
        print("Usage: generate_gpio_testplan.py <input_json_path> <output_dir> <ip_name>")
        sys.exit(1)

    input_json_path = sys.argv[1]
    output_dir = sys.argv[2]
    ip_name = sys.argv[3]

    # Load JSON
    with open(input_json_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Determine rows
    if isinstance(data, dict) and 'test_cases' in data and isinstance(data['test_cases'], list):
        rows = data['test_cases']
    elif isinstance(data, list):
        rows = data
    elif isinstance(data, dict):
        rows = [data]
    else:
        print("Unsupported JSON structure")
        sys.exit(2)

    # Build union of keys preserving first-seen order from rows
    seen = []
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)

    # Create workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write header from seen keys
    for c, key in enumerate(seen, start=1):
        ws.cell(row=1, column=c, value=key)

    # Write rows
    for r_idx, r in enumerate(rows, start=2):
        for c, key in enumerate(seen, start=1):
            ws.cell(row=r_idx, column=c, value=to_cell_value(r.get(key, "")))

    # Basic formatting on Data
    header_font = Font(bold=True)
    center = Alignment(horizontal="center", vertical="center")
    top = Alignment(vertical="top")

    for c in range(1, ws.max_column + 1):
        ws.cell(row=1, column=c).font = header_font
        ws.cell(row=1, column=c).alignment = center

    ws.freeze_panes = "A2"
    autosize_columns(ws)

    # Create META sheet (no formatting per rules)
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # Write META headers
    for c, key in enumerate(META_COLUMNS, start=1):
        ws_meta.cell(row=1, column=c, value=key)

    # Map from Data headers to column index
    header_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Copy META columns from Data to META sheet
    for r_idx in range(2, ws.max_row + 1):
        for c, key in enumerate(META_COLUMNS, start=1):
            src_col = header_to_col.get(key)
            val = ws.cell(row=r_idx, column=src_col).value if src_col else ""
            ws_meta.cell(row=r_idx, column=c, value=val)

    # Very hidden the meta sheet
    ws_meta.sheet_state = "veryHidden"

    # Build final TestPlan sheet with only MAIN_COLUMNS in exact order
    ws_final = wb.create_sheet("TestPlan")
    for c, key in enumerate(MAIN_COLUMNS, start=1):
        ws_final.cell(row=1, column=c, value=key)
        ws_final.cell(row=1, column=c).font = header_font
        ws_final.cell(row=1, column=c).alignment = center

    for r_idx in range(2, ws.max_row + 1):
        dest_r = r_idx - 1  # since Data has header row
        for c, key in enumerate(MAIN_COLUMNS, start=1):
            src_col = header_to_col.get(key)
            val = ws.cell(row=r_idx, column=src_col).value if src_col else ""
            ws_final.cell(row=dest_r, column=c, value=val)

    # Delete original Data sheet
    wb.remove(ws)

    # Formatting for TestPlan
    header_index = {ws_final.cell(row=1, column=c).value: c for c in range(1, ws_final.max_column + 1)}

    # Wrap text in specified columns
    for key in WRAP_COLUMNS:
        c = header_index.get(key)
        if c:
            for r in range(2, ws_final.max_row + 1):
                ws_final.cell(row=r, column=c).alignment = Alignment(wrap_text=True, vertical="top")

    # Index centered; other cells top-aligned by default
    idx_col = header_index.get("Index")
    if idx_col:
        for r in range(2, ws_final.max_row + 1):
            ws_final.cell(row=r, column=idx_col).alignment = Alignment(horizontal="center", vertical="top")

    # Header formatting already applied; ensure freeze top row
    ws_final.freeze_panes = "A2"

    # Default top alignment for data rows
    for r in range(2, ws_final.max_row + 1):
        for c in range(1, ws_final.max_column + 1):
            cell = ws_final.cell(row=r, column=c)
            if not cell.alignment or (cell.alignment.horizontal is None and cell.alignment.vertical is None):
                cell.alignment = top

    # Autosize and borders
    autosize_columns(ws_final)
    apply_borders(ws_final)

    # Compute IST filename
    if ZoneInfo:
        now_ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    else:
        now_ist = datetime.utcnow()
    fname = f"{ip_name}_TestPlan_{now_ist.strftime('%Y%m%d_%H%M%S')}.xlsx"

    os.makedirs(output_dir, exist_ok=True)
    out_path = os.path.join(output_dir, fname)
    wb.save(out_path)
    print(out_path)

if __name__ == "__main__":
    main()
