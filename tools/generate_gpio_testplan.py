#!/usr/bin/env python3
import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
IP_NAME = "GPIO"
NAMING_RULE = "<IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx"
COMMIT_CHANGES = True
SPEC_PATH = os.path.join("testplan_specs", "GPIO_final_json.json")


def load_final_json():
    with open(SPEC_PATH, 'r', encoding='utf-8') as f:
        text = f.read()
    data = json.loads(text)
    return text, data


def build_columns(records):
    # Start with keys from first record in order, then append any new keys from later records in encounter order
    cols = list(records[0].keys())
    for rec in records[1:]:
        for k in rec.keys():
            if k not in cols:
                cols.append(k)
    return cols


def apply_header_style(ws):
    header_fill = PatternFill(fill_type="solid", fgColor="FFD966")  # light yellow
    header_font = Font(bold=True)
    for cell in ws[1]:
        cell.fill = header_fill
        cell.font = header_font
        cell.alignment = Alignment(vertical="top", wrap_text=True)


def autosize_columns(ws, max_width=120):
    # Compute reasonable widths based on text length, capped
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            val = cell.value
            if val is None:
                continue
            s = str(val)
            if len(s) > max_len:
                max_len = len(s)
        width = min(max_width, max(10, max_len + 2))
        ws.column_dimensions[get_column_letter(col_idx)].width = width


def create_workbook(final_json_text, records):
    wb = Workbook()

    # TestPlan sheet
    ws = wb.active
    ws.title = 'TestPlan'

    cols = build_columns(records)
    ws.append(cols)

    # Preserve row order and write exact string values
    for rec in records:
        row = []
        for col in cols:
            val = rec.get(col, "")
            # Preserve exactly as provided (string values in JSON)
            row.append(val)
        ws.append(row)

    # Styling for TestPlan
    apply_header_style(ws)
    for r in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in r:
            cell.alignment = Alignment(wrap_text=True, vertical='top')
    ws.freeze_panes = 'A2'
    autosize_columns(ws)

    # MetaData sheet (very hidden)
    ws2 = wb.create_sheet('MetaData')
    ws2.append(["Key", "Value"])
    apply_header_style(ws2)

    # Compute IST timestamp for filename reference
    ist = timezone(timedelta(hours=5, minutes=30))
    ist_now = datetime.now(tz=ist)
    ts_str = ist_now.strftime('%Y%m%d_%H%M%S')

    meta_pairs = [
        ("owner", OWNER),
        ("repo", REPO),
        ("branch", BRANCH),
        ("output_directory", OUTPUT_DIR),
        ("IP_NAME", IP_NAME),
        ("output_file_naming_rule", NAMING_RULE),
        ("commit_changes", str(COMMIT_CHANGES).lower()),
        ("ist_timestamp", ts_str),
    ]
    for k, v in meta_pairs:
        ws2.append([k, v])

    # Preserve final_json exactly; chunk to avoid Excel 32,767 char per-cell limit
    CHUNK = 30000
    chunks = [final_json_text[i:i+CHUNK] for i in range(0, len(final_json_text), CHUNK)]
    ws2.append(["final_json_parts", str(len(chunks))])
    for idx, part in enumerate(chunks, start=1):
        ws2.append([f"final_json_part_{idx:03d}", part])

    for r in ws2.iter_rows(min_row=2, max_row=ws2.max_row, min_col=1, max_col=2):
        for cell in r:
            cell.alignment = Alignment(wrap_text=True, vertical='top')

    autosize_columns(ws2)
    ws2.freeze_panes = 'A2'
    ws2.sheet_state = 'veryHidden'

    return wb, ts_str


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    final_json_text, records = load_final_json()
    wb, ts_str = create_workbook(final_json_text, records)

    filename = f"{IP_NAME}_TestPlan_{ts_str}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, filename)
    wb.save(out_path)
    print(f"Generated: {out_path}")


if __name__ == '__main__':
    main()
