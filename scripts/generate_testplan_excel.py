#!/usr/bin/env python3
import argparse
import json
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Alignment, Border, Side, Font
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
    "Imparted Registers"  # placeholder replaced below
]
# Correct the column name to exact required value
MAIN_COLUMNS[-1] = "Impacted Registers"
MAIN_COLUMNS += [
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


def load_json(path: Path):
    with path.open('r', encoding='utf-8') as f:
        data = json.load(f)
    # Expect top-level object with key 'test_cases'
    if not isinstance(data, dict) or 'test_cases' not in data or not isinstance(data['test_cases'], list) or len(data['test_cases']) == 0:
        raise ValueError("Input JSON must be an object with a non-empty 'test_cases' array")
    return data


def normalize_rows(test_cases):
    # Preserve key order by first appearance across rows
    header = []
    rows = []
    for obj in test_cases:
        row = {}
        for k, v in obj.items():
            if k not in header:
                header.append(k)
            # Convert arrays to newline-separated strings for Excel cells
            if isinstance(v, list):
                row[k] = "\n".join(str(x) for x in v)
            else:
                row[k] = v
        rows.append(row)
    # Fill missing with blanks
    for r in rows:
        for k in header:
            if k not in r:
                r[k] = ""
    return header, rows


def autofit_columns(ws):
    # Approximate column width by max string length, capped and scaled
    for col_idx in range(1, ws.max_column + 1):
        col_letter = get_column_letter(col_idx)
        max_len = 0
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=col_idx, max_col=col_idx):
            cell = row[0]
            val = cell.value
            if val is None:
                length = 0
            else:
                s = str(val)
                # consider wrapped lines
                length = max(len(line) for line in s.split("\n")) if "\n" in s else len(s)
            if length > max_len:
                max_len = length
        # Scale length to Excel width units
        width = min(max(10, max_len + 2), 80)
        ws.column_dimensions[col_letter].width = width


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def adjust_row_heights(ws):
    # Approximate height with number of wrapped lines
    base_height = 15
    for r in range(2, ws.max_row + 1):  # skip header
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            val = ws.cell(row=r, column=c).value
            if isinstance(val, str):
                lines = val.count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines


def build_workbook(data, ip_name: str, out_dir: Path) -> Path:
    test_cases = data['test_cases']
    header, rows = normalize_rows(test_cases)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write header
    for idx, key in enumerate(header, start=1):
        ws.cell(row=1, column=idx, value=key)
    # Bold header and freeze
    for c in range(1, len(header) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    ws.freeze_panes = 'A2'

    # Write rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(header, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(key, ""))

    # Create meta sheet and move META columns
    meta_ws = wb.create_sheet('Meta_data_sheet')
    for c_idx, key in enumerate(META_COLUMNS, start=1):
        meta_ws.cell(row=1, column=c_idx, value=key)
        meta_ws.cell(row=1, column=c_idx).font = Font(bold=True)
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(META_COLUMNS, start=1):
            meta_ws.cell(row=r_idx, column=c_idx, value=row.get(key, ""))
    # Very hidden meta sheet
    meta_ws.sheet_state = 'veryHidden'

    # Rename Data to TestPlan
    ws.title = 'TestPlan'

    # Remove META columns from TestPlan
    header_to_index = {k: i + 1 for i, k in enumerate(header)}
    remove_indices = [header_to_index[k] for k in META_COLUMNS if k in header_to_index]
    for col_idx in sorted(remove_indices, reverse=True):
        ws.delete_cols(col_idx)
    # Rebuild header after deletions
    current_header = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]

    # Reorder to MAIN_COLUMNS exactly
    col_map = {name: (current_header.index(name) + 1) for name in current_header if name in MAIN_COLUMNS}
    # Ensure all required MAIN columns exist in current_header
    for name in MAIN_COLUMNS:
        if name not in current_header:
            # Create blank column for missing required column
            ws.insert_cols(ws.max_column + 1)
            ws.cell(row=1, column=ws.max_column, value=name)
            col_map[name] = ws.max_column
    # Build new data grid
    data_grid = []
    data_grid.append(MAIN_COLUMNS)
    for r in range(2, ws.max_row + 1):
        new_row = []
        for name in MAIN_COLUMNS:
            src_col = col_map.get(name)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            new_row.append(val)
        data_grid.append(new_row)

    # Clear and write back
    ws.delete_rows(1, ws.max_row)
    for r_idx, row in enumerate(data_grid, start=1):
        for c_idx, val in enumerate(row, start=1):
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Formatting for TestPlan only
    # Header styling
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    ws.freeze_panes = 'A2'

    # Data alignment and wrap for selected columns
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            hdr = ws.cell(row=1, column=c).value
            if hdr in WRAP_COLUMNS:
                ws.cell(row=r, column=c).alignment = Alignment(wrap_text=True, vertical='top')
            elif hdr == 'Index':
                ws.cell(row=r, column=c).alignment = Alignment(horizontal='center', vertical='top')
            else:
                ws.cell(row=r, column=c).alignment = Alignment(vertical='top')

    # Borders
    apply_borders(ws)

    # Autofit and row heights
    autofit_columns(ws)
    adjust_row_heights(ws)

    # Save to IST filename
    ist = ZoneInfo('Asia/Kolkata')
    ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    out_dir.mkdir(parents=True, exist_ok=True)
    filename = f"{ip_name}_TestPlan_{ts}.xlsx"
    out_path = out_dir / filename
    wb.save(out_path)

    print(f"WROTE {out_path}")
    return out_path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--input', required=True)
    ap.add_argument('--out-dir', required=True)
    ap.add_argument('--ip-name', required=True)
    args = ap.parse_args()

    data = load_json(Path(args.input))
    build_workbook(data, args.ip_name, Path(args.out_dir))


if __name__ == '__main__':
    main()
