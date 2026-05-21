#!/usr/bin/env python3
import json, sys, argparse, os
from datetime import datetime, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font

def to_ist_timestamp():
    utc_now = datetime.utcnow()
    ist = utc_now + timedelta(hours=5, minutes=30)
    return ist.strftime('%Y%m%d_%H%M%S')

def write_sheet(ws, headers, rows):
    ws.append(headers)
    for cell in ws[1]:
        cell.font = Font(bold=True)
    for r in rows:
        ws.append([r.get(h, "") for h in headers])
    ws.freeze_panes = 'A2'

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True)
    ap.add_argument('--outdir', required=True)
    args = ap.parse_args()

    with open(args.json, 'r', encoding='utf-8') as f:
        data = json.load(f)
    if not isinstance(data, list):
        print('ERROR: json_data is not a list', file=sys.stderr)
        sys.exit(2)

    testplan_cols = [
        "Index","SS / Module","Feature","Test Case Name","Test Description",
        "Speed","Mode","Memory Start Offset","Memory End Offset","Remarks",
        "Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria",
        "Code Generation (Required / Not)"
    ]
    metadata_cols = [
        "Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure",
        "Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers",
        "Meta Macros","Meta Arrays"
    ]

    testplan_rows = []
    metadata_rows = []
    for obj in data:
        if not isinstance(obj, dict):
            print('ERROR: array element is not object', file=sys.stderr)
            sys.exit(3)
        testplan_rows.append(obj)
        meta_obj = {
            "Index": obj.get("Index",""),
            "Test Case Name": obj.get("Test Case Name",""),
            "Meta Test Description": obj.get("Meta Test Description",""),
            "Meta Test Steps / Procedure": obj.get("Meta Test Steps / Procedure",""),
            "Meta Impacted Registers": obj.get("Meta Impacted Registers",""),
            "Meta Validation / Acceptance Criteria": obj.get("Meta Validation / Acceptance Criteria",""),
            "Meta Headers": obj.get("Meta Headers",""),
            "Meta Macros": obj.get("Meta Macros",""),
            "Meta Arrays": obj.get("Meta Arrays",""),
        }
        metadata_rows.append(meta_obj)

    os.makedirs(args.outdir, exist_ok=True)
    wb = Workbook()
    ws_main = wb.active
    ws_main.title = 'TestPlan'
    write_sheet(ws_main, testplan_cols, testplan_rows)

    ws_meta = wb.create_sheet('MetaData')
    write_sheet(ws_meta, metadata_cols, metadata_rows)
    ws_meta.sheet_state = 'veryHidden'

    ts = to_ist_timestamp()
    filename = f"testplan_{ts}.xlsx"
    outpath = os.path.join(args.outdir, filename)
    wb.save(outpath)
    with open(os.path.join(args.outdir, '.last_output_path.txt'), 'w', encoding='utf-8') as sf:
        sf.write(outpath + '\n')
    print(outpath)

if __name__ == '__main__':
    main()
