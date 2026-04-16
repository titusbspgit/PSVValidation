import json, sys
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment
from openpyxl.utils import get_column_letter

def auto_width(ws, min_width=8, max_width=50):
    dims = {}
    for row in ws.iter_rows(values_only=True):
        for i, v in enumerate(row, 1):
            s = '' if v is None else str(v)
            dims[i] = max(dims.get(i, 0), len(s))
    for i, w in dims.items():
        ws.column_dimensions[get_column_letter(i)].width = max(min_width, min(max_width, w + 2))

def main(inp, outp):
    with open(inp, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or not data:
        raise SystemExit('ERROR: JSON must be a non-empty array of objects.')
    # Deterministic column order from first object's keys
    cols = list(data[0].keys())
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'
    header_font = Font(bold=True)
    for c, h in enumerate(cols, 1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = header_font
        cell.alignment = Alignment(horizontal='center')
    for r, obj in enumerate(data, 2):
        for c, k in enumerate(cols, 1):
            ws.cell(row=r, column=c, value=obj.get(k))
    ws.freeze_panes = 'A2'
    auto_width(ws)
    wb.save(outp)

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: python json_to_xlsx.py input.json output.xlsx')
        sys.exit(2)
    main(sys.argv[1], sys.argv[2])
