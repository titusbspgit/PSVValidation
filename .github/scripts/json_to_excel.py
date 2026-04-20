#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
import sys
from collections import OrderedDict
from datetime import datetime
from pathlib import Path

try:
    from zoneinfo import ZoneInfo  # Python 3.9+
except Exception:  # pragma: no cover
    ZoneInfo = None

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment
except ImportError as e:  # pragma: no cover
    print(f"ERROR: openpyxl not installed: {e}", file=sys.stderr)
    sys.exit(2)


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Convert JSON to Excel (.xlsx) with a single worksheet.")
    p.add_argument("--input", required=True, help="Path to input JSON file (array or object)")
    p.add_argument("--output-dir", required=True, help="Output directory to write the .xlsx file")
    p.add_argument("--ip-name", required=True, help="IP name for naming the output file")
    p.add_argument("--sheet-name", default="Data", help="Worksheet name (default: Data)")
    return p.parse_args()


def load_json(path: Path):
    with path.open("r", encoding="utf-8") as f:
        try:
            # Preserve key order deterministically
            return json.load(f, object_pairs_hook=OrderedDict)
        except json.JSONDecodeError as e:
            print(f"ERROR: Invalid JSON: {e}", file=sys.stderr)
            sys.exit(1)


def normalize(records):
    # records can be a dict (single object) or list of dicts
    if isinstance(records, dict):
        rows = [records]
    elif isinstance(records, list):
        rows = records
    else:
        print("ERROR: Unsupported JSON structure: expected object or array of objects", file=sys.stderr)
        sys.exit(1)
    if len(rows) == 0:
        print("ERROR: Empty JSON array", file=sys.stderr)
        sys.exit(1)

    # Build ordered union of keys based on first appearance
    columns = []
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            print("ERROR: Each array element must be a JSON object", file=sys.stderr)
            sys.exit(1)
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                columns.append(k)
    return columns, rows


def to_cell_value(v):
    # Preserve exact JSON values. Convert lists/dicts to compact JSON string.
    if isinstance(v, (dict, list)):
        return json.dumps(v, ensure_ascii=False, separators=(",", ":"))
    return v


def autosize_columns(ws):
    # Compute max length per column and set width
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                val = "" if cell.value is None else str(cell.value)
            except Exception:
                val = ""
            if len(val) > max_len:
                max_len = len(val)
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 120)


def build_workbook(columns, rows, sheet_name: str) -> Workbook:
    wb = Workbook()
    ws = wb.active
    ws.title = sheet_name

    # Header
    header_font = Font(bold=True)
    ws.append(columns)
    for cell in ws[1]:
        cell.font = header_font
        cell.alignment = Alignment(vertical="top", wrap_text=True)

    # Rows
    for r in rows:
        ws.append([to_cell_value(r.get(c, "")) for c in columns])
    ws.freeze_panes = "A2"

    # Basic auto-size
    autosize_columns(ws)
    return wb


def ist_timestamp() -> str:
    tz = ZoneInfo("Asia/Kolkata") if ZoneInfo else None
    now = datetime.now(tz) if tz else datetime.utcnow()
    if not tz:
        # Fallback: UTC used if ZoneInfo not available, but append _IST to follow naming rule
        pass
    return now.strftime("%Y%m%d_%H%M%S")


def main():
    args = parse_args()
    in_path = Path(args.input)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    data = load_json(in_path)
    columns, rows = normalize(data)

    wb = build_workbook(columns, rows, args.sheet_name)

    ts = ist_timestamp()
    out_name = f"{args.ip_name}_TestPlan_{ts}_IST.xlsx"
    out_path = out_dir / out_name

    wb.save(out_path)
    print(str(out_path))

if __name__ == "__main__":
    main()
