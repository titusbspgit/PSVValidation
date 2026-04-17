import json, os, sys
from openpyxl import Workbook
from openpyxl.styles import Font
from openpyxl.utils import get_column_letter

def load_rows(data):
    if isinstance(data, list):
        return data
    if isinstance(data, dict) and isinstance(data.get("TestCases"), list):
        return data["TestCases"]
    if isinstance(data, dict):
        return [data]
    print("Unsupported JSON structure", file=sys.stderr)
    sys.exit(2)

def build_header(rows):
    header, seen = [], set()
    for r in rows:
        if not isinstance(r, dict):
            print("Row is not an object", file=sys.stderr)
            sys.exit(2)
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                header.append(k)
    return header

def auto_fit(ws, header, nrows):
    for idx, h in enumerate(header, start=1):
        col = get_column_letter(idx)
        max_len = len(str(h))
        for r in range(2, nrows + 2):
            v = ws.cell(row=r, column=idx).value
            if v is None:
                l = 0
            else:
                s = str(v)
                l = min(len(s), 120)
            if l > max_len:
                max_len = l
        ws.column_dimensions[col].width = min(max_len + 2, 120)

def main():
    in_path = os.environ.get('INPUT_JSON_PATH', 'scripts/input.json')
    out_path_env = os.environ.get('OUTPUT_FILE_PATH', 'Test_Output/GPIO/TestPlan/GPIO_TestPlan_1.xlsx')
    out_name = os.environ.get('OUTPUT_FILE_NAME', '')
    if out_path_env.lower().endswith('.xlsx'):
        out_path = out_path_env
    else:
        name = out_name or 'output.xlsx'
        out_path = os.path.join(out_path_env, name)

    with open(in_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    rows = load_rows(data)
    if not rows:
        print('Empty JSON', file=sys.stderr)
        sys.exit(2)

    header = build_header(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    for c, h in enumerate(header, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)

    for r_idx, r in enumerate(rows, start=2):
        for c, h in enumerate(header, start=1):
            v = r.get(h, '')
            if isinstance(v, (list, dict)):
                v = json.dumps(v, ensure_ascii=False, separators=(',', ':'))
            ws.cell(row=r_idx, column=c, value=v)

    ws.freeze_panes = 'A2'
    auto_fit(ws, header, len(rows))

    out_dir = os.path.dirname(out_path)
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    wb.save(out_path)

if __name__ == '__main__':
    main()
