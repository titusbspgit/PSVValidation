import os
from datetime import datetime, timedelta, timezone
from openpyxl import load_workbook
from openpyxl.styles import Alignment, Font, Border, Side
from openpyxl.utils import get_column_letter

IP_NAME = "GPIO"
INPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")
OUTPUT_DIR = INPUT_DIR

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_ORDER = [
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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}


def find_latest_xlsx(path: str) -> str:
    candidates = [n for n in os.listdir(path) if n.lower().endswith(".xlsx")]
    if not candidates:
        raise FileNotFoundError("No .xlsx found in " + path)
    candidates.sort()
    return os.path.join(path, candidates[-1])


def get_headers(ws):
    return [(c.value if c.value is not None else "") for c in ws[1]]


def copy_meta_sheet(main_ws, wb):
    headers = get_headers(main_ws)
    header_to_col = {str(h): idx + 1 for idx, h in enumerate(headers)}
    meta_ws = wb.create_sheet("Meta_data_sheet")
    for j, h in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=j, value=h)
    max_row = main_ws.max_row
    for i in range(2, max_row + 1):
        for j, h in enumerate(META_COLS, start=1):
            col_idx = header_to_col.get(h)
            val = main_ws.cell(row=i, column=col_idx).value if col_idx else None
            meta_ws.cell(row=i, column=j, value=val)
    meta_ws.sheet_state = 'veryHidden'
    return meta_ws


def build_normalized_main(main_ws, wb):
    headers = get_headers(main_ws)
    header_to_col = {str(h): idx + 1 for idx, h in enumerate(headers)}
    tmp_ws = wb.create_sheet("_TMP_")
    for j, h in enumerate(MAIN_ORDER, start=1):
        tmp_ws.cell(row=1, column=j, value=h)
    max_row = main_ws.max_row
    for i in range(2, max_row + 1):
        for j, h in enumerate(MAIN_ORDER, start=1):
            col_idx = header_to_col.get(h)
            val = main_ws.cell(row=i, column=col_idx).value if col_idx else None
            tmp_ws.cell(row=i, column=j, value=val)
    wb.remove(main_ws)
    tmp_ws.title = "TestPlan"
    return tmp_ws


def apply_formatting(ws):
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=False)
    left_top = Alignment(horizontal="left", vertical="top", wrap_text=False)
    center_top = Alignment(horizontal="center", vertical="top", wrap_text=False)
    thin = Side(style="thin")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    headers = get_headers(ws)
    wrap_idx = {headers.index(h) + 1 for h in WRAP_COLS if h in headers}

    max_row = ws.max_row
    max_col = ws.max_column

    ws.freeze_panes = "A2"
    ws.auto_filter.ref = ws.dimensions

    for col in range(1, max_col + 1):
        c = ws.cell(row=1, column=col)
        c.font = header_font
        c.alignment = header_align
        c.border = border

    for r in range(2, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            if c in wrap_idx:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            else:
                cell.alignment = center_top if c == 1 else left_top
            cell.border = border

    for col in range(1, max_col + 1):
        letter = get_column_letter(col)
        max_len = 0
        for row in range(1, max_row + 1):
            v = ws.cell(row=row, column=col).value
            s = "" if v is None else str(v)
            ln = max((len(line) for line in s.splitlines()), default=0)
            max_len = max(max_len, ln)
        ws.column_dimensions[letter].width = min(max_len + 2, 80)


def main():
    src_path = find_latest_xlsx(INPUT_DIR)
    wb = load_workbook(src_path)

    # Prefer 'TestPlan' else first visible sheet
    main_ws = wb['TestPlan'] if 'TestPlan' in wb.sheetnames else next(
        (wb[name] for name in wb.sheetnames if wb[name].sheet_state == 'visible'), None)
    if main_ws is None:
        raise RuntimeError("No visible worksheet found")

    copy_meta_sheet(main_ws, wb)
    main_ws = build_normalized_main(main_ws, wb)

    for name in list(wb.sheetnames):
        if name not in ("TestPlan", "Meta_data_sheet"):
            wb.remove(wb[name])

    apply_formatting(main_ws)

    # Compute IST timestamp without ZoneInfo dependency
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    out_name = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, out_name)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(out_path)
    print(out_path)


if __name__ == "__main__":
    main()
