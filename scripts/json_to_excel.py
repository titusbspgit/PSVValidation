#!/usr/bin/env python3
import json
import os
import hashlib
from collections import OrderedDict
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

# Embedded JSON input (preserves key order deterministically)
JSON_INPUT = r'''
[
  {
    "id": 101,
    "name": "Anaya",
    "age": 28,
    "city": "Chennai",
    "active": true
  },
  {
    "id": 102,
    "name": "Rohan",
    "age": 34,
    "city": "Bengaluru",
    "active": false
  },
  {
    "id": 103,
    "name": "Meera",
    "age": 25,
    "city": "Mumbai",
    "active": true
  },
  {
    "id": 104,
    "name": "Arjun",
    "age": 41,
    "city": "Hyderabad",
    "active": true
  },
  {
    "id": 105,
    "name": "Kavya",
    "age": 30,
    "city": "Pune",
    "active": false
  }
]
'''.strip()

# Target Excel path (relative to repo root)
OUTPUT_XLSX_PATH = "TestRepo/gpio/TESTER_JSON.xlsx"
SHEET_NAME = "Data"

def main():
    # Load JSON preserving pair order
    records = json.loads(JSON_INPUT, object_pairs_hook=OrderedDict)

    # Normalize to list of dict rows
    if isinstance(records, dict):
        records = [records]
    if not isinstance(records, list) or not all(isinstance(r, dict) for r in records):
        raise SystemExit("Unsupported JSON structure: expected an object or an array of objects")

    if len(records) == 0:
        raise SystemExit("Empty JSON array is not allowed")

    # Build union of keys in first-seen order
    key_order = OrderedDict()
    for row in records:
        for k in row.keys():
            if k not in key_order:
                key_order[k] = None
    columns = list(key_order.keys())

    # Create workbook and sheet
    wb = Workbook()
    ws = wb.active
    ws.title = SHEET_NAME

    # Header row
    header_font = Font(bold=True)
    for col_idx, col_name in enumerate(columns, start=1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font

    # Data rows
    for row_idx, row in enumerate(records, start=2):
        for col_idx, col_name in enumerate(columns, start=1):
            value = row.get(col_name, None)
            ws.cell(row=row_idx, column=col_idx, value=value)

    # Freeze top row and add auto-filter
    ws.freeze_panes = "A2"
    last_col_letter = get_column_letter(len(columns))
    ws.auto_filter.ref = f"A1:{last_col_letter}{len(records) + 1}"

    # Auto-fit columns (approximate based on content width)
    col_widths = {i: max(3, len(str(columns[i - 1]))) for i in range(1, len(columns) + 1)}
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=len(columns)):
        for cell in row:
            text = "" if cell.value is None else str(cell.value)
            col_widths[cell.column] = max(col_widths[cell.column], len(text))
    for i in range(1, len(columns) + 1):
        ws.column_dimensions[get_column_letter(i)].width = col_widths[i] + 2  # padding

    # Ensure directory exists
    os.makedirs(os.path.dirname(OUTPUT_XLSX_PATH), exist_ok=True)

    # Save workbook
    wb.save(OUTPUT_XLSX_PATH)

    # Print SHA-256 for logging
    sha256 = hashlib.sha256()
    with open(OUTPUT_XLSX_PATH, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            sha256.update(chunk)
    print(f"Saved: {OUTPUT_XLSX_PATH}")
    print(f"SHA256: {sha256.hexdigest()}")

if __name__ == "__main__":
    main()

# noop trigger to run Generate JSON Excel workflow
