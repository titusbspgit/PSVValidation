import json
import os
import sys
from typing import List, Dict, Any

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font
except ImportError:
    print("ERROR: openpyxl is not installed.")
    sys.exit(1)


def load_json(path: str) -> Any:
    with open(path, 'r', encoding='utf-8') as f:
        return json.load(f)


def validate_and_normalize(data: Any) -> (List[str], List[Dict[str, Any]]):
    if data is None:
        raise ValueError("Empty JSON input")

    # If single object, convert to single-row list
    if isinstance(data, dict):
        rows = [data]
    elif isinstance(data, list):
        if len(data) == 0:
            raise ValueError("Empty JSON array")
        rows = data
    else:
        raise ValueError("Unsupported JSON structure: expected object or array of objects")

    # Ensure each row is a dict
    for i, r in enumerate(rows):
        if not isinstance(r, dict):
            raise ValueError(f"Unsupported row at index {i}: expected object, got {type(r).__name__}")

    # Build ordered union of keys based on first appearance across rows
    ordered_cols: List[str] = []
    seen = set()
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                ordered_cols.append(k)

    # Normalize rows (fill missing with blank string)
    normalized_rows: List[Dict[str, Any]] = []
    for r in rows:
        nr = {k: r.get(k, "") for k in ordered_cols}
        normalized_rows.append(nr)

    return ordered_cols, normalized_rows


def autosize(ws):
    # Compute max string lengths per column including header
    for col_idx, column_cells in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row), start=1):
        max_len = 0
        for cell in column_cells:
            v = cell.value
            if v is None:
                s = ""
            elif isinstance(v, (int, float)):
                s = str(v)
            elif isinstance(v, bool):
                s = "TRUE" if v else "FALSE"
            else:
                s = str(v)
            if len(s) > max_len:
                max_len = len(s)
        # Add a small padding and cap width to a reasonable size
        width = max(10, min(60, max_len + 2))
        col_letter = ws.cell(row=1, column=col_idx).column_letter
        ws.column_dimensions[col_letter].width = width


def write_excel(headers: List[str], rows: List[Dict[str, Any]], target_path: str):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row with bold font
    bold_font = Font(bold=True)
    for j, h in enumerate(headers, start=1):
        c = ws.cell(row=1, column=j, value=h)
        c.font = bold_font

    # Data rows
    for i, row in enumerate(rows, start=2):
        for j, h in enumerate(headers, start=1):
            ws.cell(row=i, column=j, value=row.get(h, ""))

    # Freeze top row
    ws.freeze_panes = "A2"

    # Auto-fit columns
    autosize(ws)

    # Ensure directory exists
    os.makedirs(os.path.dirname(target_path), exist_ok=True)

    wb.save(target_path)


def main():
    json_source = os.environ.get('JSON_SOURCE', 'data/source.json')
    xlsx_target = os.environ.get('XLSX_TARGET', 'TestRepo/gpio/GPIO_TestCase.xlsx')

    data = load_json(json_source)
    headers, rows = validate_and_normalize(data)
    write_excel(headers, rows, xlsx_target)
    print(f"Wrote Excel with {len(rows)} rows and {len(headers)} columns to {xlsx_target}")


if __name__ == '__main__':
    main()
