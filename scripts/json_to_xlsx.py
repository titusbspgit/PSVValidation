import os, sys, json
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

def parse_json_stream(s: str):
    try:
        x = json.loads(s)
        return x if isinstance(x, list) else [x]
    except Exception:
        chunks = [c for c in s.split("\n\n") if c.strip()]
        rows = []
        for c in chunks:
            x = json.loads(c)
            if isinstance(x, list):
                rows.extend(x)
            elif isinstance(x, dict):
                rows.append(x)
            else:
                raise ValueError("Unsupported JSON top-level type")
        return rows

def main():
    in_path = 'data/json_data.json'
    out_path = 'Test_Output/GPIO/TestPlan/GPIO_TestPlan_1.xlsx'
    with open(in_path, 'r', encoding='utf-8') as f:
        txt = f.read()
    rows = parse_json_stream(txt)
    if not rows:
        raise SystemExit('ERROR: Empty JSON')
    cols = []
    for r in rows:
        if not isinstance(r, dict):
            raise SystemExit('ERROR: Unsupported JSON structure (expected array of objects)')
        for k in r.keys():
            if k not in cols:
                cols.append(k)
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'
    for cidx, key in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=cidx, value=key)
        cell.font = Font(bold=True)
    for ridx, obj in enumerate(rows, start=2):
        for cidx, key in enumerate(cols, start=1):
            val = obj.get(key, '')
            if isinstance(val, (list, dict)):
                val = json.dumps(val, ensure_ascii=False, separators=(',', ':'), sort_keys=False)
            ws.cell(row=ridx, column=cidx, value=val)
    ws.freeze_panes = 'A2'
    for cidx, key in enumerate(cols, start=1):
        maxlen = len(str(key))
        for r in range(2, ws.max_row + 1):
            v = ws.cell(row=r, column=cidx).value
            l = len(str(v)) if v is not None else 0
            if l > maxlen:
                maxlen = l
        width = min(150, max(10, int(maxlen * 1.1)))
        ws.column_dimensions[get_column_letter(cidx)].width = width
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)
    print(f'ROWS={len(rows)}')
    print(f'COLS={len(cols)}')

if __name__ == '__main__':
    main()
