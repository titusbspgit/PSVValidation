import os, re, sys, datetime, pytz, subprocess
from openpyxl import load_workbook
from openpyxl.styles import Alignment, Font, Border, Side
from openpyxl.utils import get_column_letter

INPUT_DIR = os.getenv("INPUT_DIR", "Test_Output/GPIO/TestPlan")
OUTPUT_DIR = os.getenv("OUTPUT_DIR", INPUT_DIR)
IP_NAME = os.getenv("IP_NAME", "IP")
TZ_NAME = os.getenv("TZ", "Asia/Kolkata")
COMMIT_MESSAGE = os.getenv("COMMIT_MESSAGE", "Add formatted test plan (IST)")

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLS = [
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
]

PATTERN = re.compile(rf"^{re.escape(IP_NAME)}_TestPlan_(\\d{{8}})_(\\d{{6}})(?:_IST)?\\.xlsx$")

def pick_latest_xlsx(path):
    cands = []
    for name in os.listdir(path):
        if not name.lower().endswith(".xlsx"): continue
        m = PATTERN.match(name)
        ts = None
        if m:
            d, t = m.group(1), m.group(2)
            try:
                ts = datetime.datetime.strptime(d + t, "%Y%m%d%H%M%S")
            except Exception:
                ts = None
        full = os.path.join(path, name)
        mtime = os.path.getmtime(full)
        cands.append((ts, mtime, name))
    if not cands:
        raise SystemExit("No .xlsx found in directory: " + path)
    # sort: first by ts (None last), then by mtime
    cands.sort(key=lambda x: ((x[0] is None), x[0] or datetime.datetime.min, x[1]))
    latest = cands[-1][2]
    return os.path.join(path, latest)


def headers_map(ws):
    hdr = {}
    for col in range(1, ws.max_column + 1):
        v = ws.cell(row=1, column=col).value
        if v is None:
            continue
        hdr[str(v)] = col
    return hdr


def copy_meta_sheet(wb, src_ws, hdr_map):
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # build meta header from available cols in required order
    meta_cols_present = [c for c in META_COLS if c in hdr_map]
    for j, col_name in enumerate(meta_cols_present, start=1):
        ws_meta.cell(row=1, column=j, value=col_name)
    # copy rows
    for r in range(2, src_ws.max_row + 1):
        for j, col_name in enumerate(meta_cols_present, start=1):
            src_c = src_ws.cell(row=r, column=hdr_map[col_name])
            ws_meta.cell(row=r, column=j, value=src_c.value)
    # very hidden
    ws_meta.sheet_state = 'veryHidden'
    return meta_cols_present


def rebuild_main_as_testplan(ws, hdr_map, meta_cols_present):
    # approved main columns that are present
    keep_cols = [c for c in MAIN_COLS if c in hdr_map]
    # snapshot data
    data = []
    for r in range(2, ws.max_row + 1):
        row_vals = [ws.cell(row=r, column=hdr_map[c]).value for c in keep_cols]
        data.append(row_vals)
    # clear sheet
    ws.delete_rows(1, ws.max_row)
    # write header
    for j, name in enumerate(keep_cols, start=1):
        ws.cell(row=1, column=j, value=name)
    # write data
    for i, row_vals in enumerate(data, start=2):
        for j, val in enumerate(row_vals, start=1):
            ws.cell(row=i, column=j, value=val)
    # remove any residual extra columns (best-effort)
    if ws.max_column > len(keep_cols):
        ws.delete_cols(len(keep_cols) + 1, ws.max_column - len(keep_cols))
    return keep_cols


def format_testplan(ws, keep_cols):
    # header formatting
    hdr_font = Font(bold=True)
    hdr_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for j in range(1, len(keep_cols) + 1):
        c = ws.cell(row=1, column=j)
        c.font = hdr_font
        c.alignment = hdr_align
    # data alignment
    text_left_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for i in range(1, ws.max_row + 1):
        for j in range(1, len(keep_cols) + 1):
            c = ws.cell(row=i, column=j)
            # wrap text for specific columns
            col_name = keep_cols[j-1]
            if col_name in text_left_cols:
                c.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            elif col_name == 'Index':
                c.alignment = Alignment(vertical='top', horizontal='center')
            else:
                c.alignment = Alignment(vertical='top', horizontal='left')
            c.border = border
    # basic auto-fit widths by content length
    for j in range(1, len(keep_cols) + 1):
        max_len = len(str(keep_cols[j-1]))
        for i in range(2, ws.max_row + 1):
            v = ws.cell(row=i, column=j).value
            if v is None:
                continue
            s = str(v)
            if len(s) > max_len:
                max_len = len(s)
        width = min(120, max_len + 2) * 0.9
        ws.column_dimensions[get_column_letter(j)].width = width
    # let row heights be default (Excel will expand on open if needed)


def main():
    in_path = pick_latest_xlsx(INPUT_DIR)
    wb = load_workbook(in_path)
    # pick first visible sheet
    main_ws = None
    for ws in wb.worksheets:
        if ws.sheet_state == 'visible':
            main_ws = ws
            break
    if main_ws is None:
        main_ws = wb.active
    hdr_map = headers_map(main_ws)
    meta_cols_present = copy_meta_sheet(wb, main_ws, hdr_map)
    # rename main to TestPlan
    main_ws.title = 'TestPlan'
    keep_cols = rebuild_main_as_testplan(main_ws, hdr_map, meta_cols_present)
    format_testplan(main_ws, keep_cols)
    # output file name in IST
    ist = pytz.timezone(TZ_NAME)
    now = datetime.datetime.now(ist)
    out_name = f"{IP_NAME}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, out_name)
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(out_path)
    # commit
    subprocess.run(["git", "config", "user.name", "github-actions[bot]"])  
    subprocess.run(["git", "config", "user.email", "github-actions[bot]@users.noreply.github.com"])  
    subprocess.run(["git", "add", out_path], check=False)
    subprocess.run(["git", "commit", "-m", COMMIT_MESSAGE], check=False)
    subprocess.run(["git", "push"], check=False)

if __name__ == "__main__":
    main()
