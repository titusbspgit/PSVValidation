import json
import argparse
from datetime import datetime, timezone, timedelta
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
from pathlib import Path
from openpyxl import Workbook
from openpyxl.utils import get_column_letter

COLUMNS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    "Hidden_Macro_Defines",
    "Hidden_Skip_Array_Definition",
]

def now_ist():
    if ZoneInfo:
        tz = ZoneInfo("Asia/Kolkata")
    else:
        tz = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz)


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"
    # header
    ws.append(COLUMNS)
    # rows
    for row in rows:
        r = []
        for k in COLUMNS:
            v = row.get(k, "NA")
            if k == "Index":
                # ensure int
                try:
                    v = int(v)
                except Exception:
                    v = 0
            else:
                if v is None:
                    v = "NA"
                v = str(v)
            r.append(v)
        ws.append(r)
    # basic column width
    for i, col in enumerate(COLUMNS, start=1):
        ws.column_dimensions[get_column_letter(i)].width = min(max(len(col) + 2, 18), 80)
    return wb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True)
    ap.add_argument('--outdir', required=True)
    ap.add_argument('--ipname', default='PCIE')
    args = ap.parse_args()

    with open(args.json, 'r', encoding='utf-8') as f:
        data = json.load(f)
        if not isinstance(data, list):
            raise SystemExit('json_data must be a list')

    wb = build_workbook(data)
    ts = now_ist().strftime('%Y%m%d_%H%M%S')
    outdir = Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)
    fname = f"{args.ipname}_TestPlan_{ts}.xlsx"
    outpath = outdir / fname
    wb.save(outpath)
    print(str(outpath))

if __name__ == '__main__':
    main()
