#!/usr/bin/env python3
import json, sys, os
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment
from openpyxl.utils import get_column_letter

def normalize_rows(rows, multiline_keys=None):
    if multiline_keys is None:
        multiline_keys = []
    # Preserve first-seen key order across all rows
    key_order = []
    for r in rows:
        for k in r.keys():
            if k not in key_order:
                key_order.append(k)
    norm = []
    for r in rows:
        row = OrderedDict()
        for k in key_order:
            v = r.get(k, "")
            if k in multiline_keys and isinstance(v, list):
                v = "\n".join(str(x) for x in v)
            elif isinstance(v, (list, dict)):
                # JSON-serialize exact value preserving order
                v = json.dumps(v, ensure_ascii=False, separators=(",", ":"))
            elif v is None:
                v = ""
            row[k] = v
        norm.append(row)
    return key_order, norm

def write_sheet(wb, title, key_order, rows):
    ws = wb.create_sheet(title)
    # Header
    ws.append(key_order)
    for c in range(1, len(key_order)+1):
        ws.cell(row=1, column=c).font = Font(bold=True)
    # Rows
    for r in rows:
        ws.append([r.get(k, "") for k in key_order])
    # Freeze top row
    ws.freeze_panes = "A2"
    # Wrap text and auto-fit (approximate) widths
    align_wrap = Alignment(wrap_text=True, vertical="top")
    max_width = [len(str(h)) for h in key_order]
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=len(key_order)):
        for idx, cell in enumerate(row):
            cell.alignment = align_wrap
            text = str(cell.value) if cell.value is not None else ""
            lw = max(len(line) for line in text.splitlines()) if "\n" in text else len(text)
            if lw > max_width[idx]:
                max_width[idx] = lw
    for i, w in enumerate(max_width, start=1):
        # heuristic: characters to excel column width
        ws.column_dimensions[get_column_letter(i)].width = min(max(10, w + 2), 80)


def main():
    if len(sys.argv) != 3:
        print("Usage: json_to_excel.py <input.json> <output.xlsx>")
        sys.exit(2)
    in_path, out_path = sys.argv[1], sys.argv[2]
    with open(in_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    wb = Workbook()
    # Remove default sheet
    wb.remove(wb.active)

    # Sheet 1: Tests_Table
    tt = data.get("tests_table", [])
    tt_keys, tt_rows = normalize_rows(tt, multiline_keys=[
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
        "Hidden_Test_Steps_Procedure",
        "Hidden_Validation_Acceptance_Criteria",
    ])
    write_sheet(wb, "Tests_Table", tt_keys, tt_rows)

    # Sheet 2: Tests_List
    tl = data.get("tests", [])
    tl_keys, tl_rows = normalize_rows(tl)
    write_sheet(wb, "Tests_List", tl_keys, tl_rows)

    # Ensure directory exists
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)
    print(f"Wrote {out_path}")

if __name__ == "__main__":
    main()
