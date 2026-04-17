from collections import OrderedDict
import json
import os
import sys
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter


def load_rows(data):
    # Accept array of objects, or root object with single array field, or single object
    if isinstance(data, list):
        return data
    if isinstance(data, OrderedDict) or isinstance(data, dict):
        if len(data) == 1:
            only_key = next(iter(data))
            val = data[only_key]
            if isinstance(val, list):
                return val
        return [data]
    raise ValueError("Unsupported JSON structure: must be array of objects or object.")


def build_headers(rows):
    headers, seen = [], set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("All rows must be objects with key/value pairs.")
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                headers.append(k)
    return headers


def autosize(ws, headers, rows):
    for cidx, h in enumerate(headers, start=1):
        max_len = len(str(h))
        for r in rows:
            v = r.get(h, "")
            max_len = max(max_len, len(str(v)))
        ws.column_dimensions[get_column_letter(cidx)].width = min(max_len + 2, 120)


def main():
    json_input = os.environ.get("JSON_INPUT", "").strip()
    if not json_input:
        print("ERROR: JSON_INPUT environment variable is empty.")
        sys.exit(1)
    try:
        data = json.loads(json_input, object_pairs_hook=OrderedDict)
    except Exception as e:
        print(f"ERROR: Invalid JSON: {e}")
        sys.exit(1)

    try:
        rows = load_rows(data)
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

    if not isinstance(rows, list) or len(rows) == 0:
        print("ERROR: Empty JSON array or unsupported structure.")
        sys.exit(1)

    headers = build_headers(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row, bold
    for cidx, name in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=cidx, value=name)
        cell.font = Font(bold=True)

    # Data rows
    for ridx, row in enumerate(rows, start=2):
        for cidx, h in enumerate(headers, start=1):
            val = row.get(h, "")
            ws.cell(row=ridx, column=cidx, value=val)

    # Formatting
    ws.freeze_panes = "A2"
    autosize(ws, headers, rows)

    out_path = os.environ.get("OUTPUT_XLSX_PATH", "output.xlsx")
    out_dir = os.path.dirname(out_path)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    wb.save(out_path)

    print(f"Generated: {out_path}\nRows: {len(rows)}\nColumns: {len(headers)}")


if __name__ == "__main__":
    main()
