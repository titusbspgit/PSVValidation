import json
import os
from pathlib import Path
from datetime import datetime, timedelta, timezone
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter


def main():
    repo_root = Path(__file__).resolve().parents[1]
    json_path = repo_root / 'data' / 'testplans' / 'GPIO' / 'final_json.json'
    with json_path.open('r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list) or not data:
        raise SystemExit('final_json must be a non-empty list')

    first = data[0]
    testplan_headers = [k for k in first.keys() if not k.startswith('Meta ')]
    metadata_headers = [k for k in first.keys() if k.startswith('Meta ')]

    wb = Workbook()
    ws_tp = wb.active
    ws_tp.title = 'TestPlan'
    ws_md = wb.create_sheet('MetaData')

    header_font = Font(bold=True)
    header_fill = PatternFill(start_color='FFCCE5FF', end_color='FFCCE5FF', fill_type='solid')
    wrap = Alignment(wrap_text=True, vertical='top')

    ws_tp.append(testplan_headers)
    for c in range(1, len(testplan_headers) + 1):
        cell = ws_tp.cell(row=1, column=c)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    ws_md.append(metadata_headers)
    for c in range(1, len(metadata_headers) + 1):
        cell = ws_md.cell(row=1, column=c)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap

    for r in data:
        ws_tp.append([r.get(k, '') for k in testplan_headers])
        ws_md.append([r.get(k, '') for k in metadata_headers])

    for ws in (ws_tp, ws_md):
        for row_cells in ws.iter_rows():
            for cell in row_cells:
                cell.alignment = wrap
        for col_idx in range(1, ws.max_column + 1):
            max_len = 0
            for row_idx in range(1, ws.max_row + 1):
                v = ws.cell(row=row_idx, column=col_idx).value
                l = len(str(v)) if v is not None else 0
                if l > max_len:
                    max_len = l
            width = min(max(10, max_len + 2), 60)
            ws.column_dimensions[get_column_letter(col_idx)].width = width
        ws.freeze_panes = 'A2'

    ws_md.sheet_state = 'veryHidden'

    ip_name = os.getenv('IP_NAME', 'GPIO')
    ist = timezone(timedelta(hours=5, minutes=30))
    timestamp = datetime.now(ist).strftime('%Y%m%d_%H%M%S')
    filename = f"{ip_name}_TestPlan_{timestamp}.xlsx"

    out_dir = repo_root / 'Test_Output' / 'GPIO' / 'TestPlan'
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / filename

    wb.save(out_path)
    print(f"Saved: {out_path}")


if __name__ == '__main__':
    main()
