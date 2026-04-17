#!/usr/bin/env python3
import argparse
import json
import os
from collections import OrderedDict
from datetime import datetime

from openpyxl import Workbook
from openpyxl.utils import get_column_letter
from openpyxl.styles import Font, Alignment


def load_json(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f, object_pairs_hook=OrderedDict)
    if isinstance(data, dict):
        data = [data]
    if not isinstance(data, list) or not data:
        raise ValueError('Input JSON must be a non-empty array or an object')
    # Ensure each row is a mapping
    for i, row in enumerate(data):
        if not isinstance(row, dict):
            raise ValueError(f'Row {i} is not an object: {type(row)}')
    return data


def union_keys_preserve_order(rows):
    cols = []
    seen = set()
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                cols.append(k)
    return cols


def set_header_style(ws):
    bold = Font(bold=True)
    for cell in ws[1]:
        cell.font = bold
        cell.alignment = Alignment(wrap_text=True, vertical='top')
    ws.freeze_panes = 'A2'


def autosize(ws):
    # Best-effort auto-size
    for col in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            val = cell.value
            if val is None:
                length = 0
            else:
                s = str(val)
                # Cap length to avoid absurd widths
                length = min(len(s), 80)
            if length > max_len:
                max_len = length
        ws.column_dimensions[col_letter].width = max(10, max_len + 2)


def write_data_sheet(wb, rows):
    ws = wb.create_sheet('Data')
    cols = union_keys_preserve_order(rows)
    ws.append(cols)
    for row in rows:
        out = []
        for k in cols:
            v = row.get(k, '')
            if isinstance(v, (list, dict)):
                out.append(json.dumps(v, ensure_ascii=False))
            else:
                out.append(v)
        ws.append(out)
    set_header_style(ws)
    autosize(ws)


def write_gpio_testplan_sheet(wb, rows):
    ws = wb.create_sheet('GPIO_TestPlan')
    # Column mapping based on expected fields
    columns = [
        ('Index', ['Index']),
        ('Test Case Name', ['Test Case Name']),
        ('Test Description', ['Test Description']),
        ('Module', ['SS / Module']),
        ('Feature', ['Feature']),
        ('Mode', ['Mode']),
        ('Speed', ['Speed']),
        ('Memory Start Offset', ['Memory Start Offset']),
        ('Memory End Offset', ['Memory End Offset']),
        ('Remarks', ['Remarks']),
        ('Steps', ['Test Steps / Procedure']),
        ('Impacted Registers', ['Impacted Registers']),
        ('Validation Criteria', ['Validation / Acceptance Criteria']),
        ('Parameters', ['Parameters', 'parameters/config']),
        ('Dependencies', ['Dependencies', 'dependencies']),
        ('Requirements', ['Requirements', 'requirements/refs']),
        ('Owner', ['owner/author', 'Owner', 'Author']),
        ('Tags', ['Tags', 'tags'])
    ]
    headers = [c[0] for c in columns]
    ws.append(headers)

    for row in rows:
        out = []
        for header, keys in columns:
            val = ''
            for k in keys:
                if k in row and row[k] not in (None, ''):
                    val = row[k]
                    break
            if isinstance(val, list):
                # Render lists as numbered multi-line text
                val = '\n'.join([f"{i+1}. {str(item)}" for i, item in enumerate(val)])
            out.append(val)
        ws.append(out)

    set_header_style(ws)
    # Wrap for long text columns
    for col in range(1, ws.max_column + 1):
        for r in range(2, ws.max_row + 1):
            ws.cell(row=r, column=col).alignment = Alignment(wrap_text=True, vertical='top')
    autosize(ws)


def write_hidden_details_sheet(wb, rows):
    # Collect Hidden_* columns
    hidden_cols = []
    seen = set()
    for row in rows:
        for k in row.keys():
            if k.startswith('Hidden_') and k not in seen:
                seen.add(k)
                hidden_cols.append(k)
    if not hidden_cols:
        return
    ws = wb.create_sheet('Hidden_Details')
    ws.append(hidden_cols)
    for row in rows:
        out = []
        for k in hidden_cols:
            v = row.get(k, '')
            if isinstance(v, (list, dict)):
                out.append(json.dumps(v, ensure_ascii=False))
            else:
                out.append(v)
        ws.append(out)
    set_header_style(ws)
    autosize(ws)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()

    rows = load_json(args.input)

    wb = Workbook()
    # Remove default sheet
    wb.remove(wb.active)

    write_data_sheet(wb, rows)
    write_gpio_testplan_sheet(wb, rows)
    write_hidden_details_sheet(wb, rows)

    out_path = args.output
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)

    print(f"Wrote Excel: {out_path}")


if __name__ == '__main__':
    main()
