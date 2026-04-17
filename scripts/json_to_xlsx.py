#!/usr/bin/env python3
import json
import os
import sys
from collections import OrderedDict

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except Exception as e:
    print(f"openpyxl not available: {e}")
    sys.exit(1)

INPUT_JSON = os.environ.get("INPUT_JSON_PATH", "data/gpio_testcases.json")
OUTPUT_XLSX = os.environ.get("OUTPUT_XLSX_PATH", "Test_Output/GPIO/TestPlan/GPIO_TestPlan_1.xlsx")
SHEET_NAME = os.environ.get("SHEET_NAME", "GPIO_TestCases")


def load_json(path):
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)
    if isinstance(data, dict):
        data = [data]
    if not isinstance(data, list) or len(data) == 0:
        raise ValueError("JSON must be a non-empty array or an object")
    return data


def build_headers(rows):
    seen = OrderedDict()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("All elements must be JSON objects")
        for k in row.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def autosize_columns(ws, max_width=100):
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            v = cell.value
            l = len(str(v)) if v is not None else 0
            if l > max_len:
                max_len = l
        adjusted = min(max_len + 2, max_width)
        ws.column_dimensions[col_letter].width = adjusted


def write_xlsx(rows, headers, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Header
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)

    ws.freeze_panes = "A2"

    # Rows
    for r, row in enumerate(rows, start=2):
        for c, h in enumerate(headers, start=1):
            ws.cell(row=r, column=c, value=row.get(h, None))

    autosize_columns(ws)

    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)


def main():
    rows = load_json(INPUT_JSON)
    headers = build_headers(rows)
    write_xlsx(rows, headers, OUTPUT_XLSX)
    print(f"Wrote {len(rows)} rows and {len(headers)} columns to {OUTPUT_XLSX}")


if __name__ == "__main__":
    main()
