import argparse, json, os, sys, zipfile
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except ImportError:
    from pytz import timezone as ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

VALIDATION_COL = "Code Generation (Required / Not)"
VALIDATION_LIST = "Required,Blank,Not Required"


def load_json_records(path):
    with open(path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    # Normalize to list of dicts
    if isinstance(data, dict):
        items = []
        # Prefer numeric Index if present
        def key_fn(kv):
            k, v = kv
            idx = None
            if isinstance(v, dict) and "Index" in v:
                try:
                    idx = int(str(v["Index"]))
                except Exception:
                    idx = None
            if idx is None:
                # fallback to TC number in key (e.g., TC3)
                import re
                m = re.search(r"(\d+)$", str(k))
                if m:
                    return int(m.group(1))
                return sys.maxsize
            return idx
        for k, v in sorted(data.items(), key=key_fn):
            if isinstance(v, dict):
                items.append(v)
    elif isinstance(data, list):
        items = data
    else:
        raise ValueError("Top-level JSON must be object or array")
    if not items:
        raise ValueError("Empty JSON input after normalization")
    # Ensure all elements are dicts
    for r in items:
        if not isinstance(r, dict):
            raise ValueError("All records must be JSON objects")
    return items


def build_key_order(records):
    seen = []
    sset = set()
    for r in records:
        for k in r.keys():
            if k not in sset:
                sset.add(k)
                seen.append(k)
    return seen


def ensure_dir(p):
    os.makedirs(p, exist_ok=True)


def value_to_str(v):
    if isinstance(v, (list, dict)):
        return json.dumps(v, ensure_ascii=False)
    return v


def join_numbered(items):
    out_lines = []
    for i, it in enumerate(items, start=1):
        s = it if isinstance(it, str) else json.dumps(it, ensure_ascii=False)
        out_lines.append(f"{i}. {s}")
    return "\n".join(out_lines)


def autofit(ws):
    # Approximate autofit by character length
    dims = {}
    for row in ws.iter_rows(values_only=False):
        for cell in row:
            v = cell.value
            if v is None:
                l = 0
            else:
                s = str(v)
                l = max(len(line) for line in s.splitlines())
            dims[cell.column_letter] = max(dims.get(cell.column_letter, 0), l)
    for col, width in dims.items():
        ws.column_dimensions[col].width = min(max(width + 2, 12), 80)


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows():
        for cell in row:
            cell.border = border


def set_header_style(ws):
    header_fill = PatternFill("solid", fgColor="4472C4")
    for cell in ws[1]:
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = header_fill


def set_data_alignment(ws):
    max_row = ws.max_row
    max_col = ws.max_column
    headers = [ws.cell(row=1, column=i).value for i in range(1, max_col+1)]
    for r in range(2, max_row+1):
        for c in range(1, max_col+1):
            cell = ws.cell(row=r, column=c)
            h = headers[c-1]
            if h == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=False)
            elif h in WRAP_COLS:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=False)


def estimate_row_heights(ws):
    for row in ws.iter_rows(min_row=2):
        max_lines = 1
        for cell in row:
            if cell.value is None:
                continue
            s = str(cell.value)
            lines = s.count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[cell.row].height = min(15 * max_lines, 300)


def add_validation(ws):
    # Apply only to data rows for the specific column
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column+1)]
    try:
        col_idx = headers.index(VALIDATION_COL) + 1
    except ValueError:
        return
    max_row = ws.max_row
    dv = DataValidation(type="list", formula1=f'"{VALIDATION_LIST}"', allow_blank=True)
    ws.add_data_validation(dv)
    rng = f"{ws.cell(row=2, column=col_idx).coordinate}:{ws.cell(row=max_row, column=col_idx).coordinate}"
    dv.add(rng)


def validate_ooxml(path):
    if not zipfile.is_zipfile(path):
        return False
    with zipfile.ZipFile(path, 'r') as z:
        needed = {"[Content_Types].xml", "xl/workbook.xml"}
        names = set(z.namelist())
        if not needed.issubset(names):
            return False
        ws_files = [n for n in names if n.startswith("xl/worksheets/sheet") and n.endswith(".xml")]
        return len(ws_files) >= 1


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--input', required=True)
    ap.add_argument('--output-dir', required=True)
    ap.add_argument('--ip-name', required=True)
    args = ap.parse_args()

    records = load_json_records(args.input)

    # Build union key order (not strictly needed since we later restrict to MAIN_COLS)
    _ = build_key_order(records)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Stage: write all keys in union order for staging
    # But we will directly create columns for MAIN+META union to preserve available data
    # Compose staging headers as union of MAIN_COLS + META_COLS preserving occurrence in records
    staging_keys = []
    for k in MAIN_COLS + META_COLS:
        staging_keys.append(k)
    # Write header
    for c, h in enumerate(staging_keys, start=1):
        ws.cell(row=1, column=c, value=h)
    # Write rows
    for r_idx, rec in enumerate(records, start=2):
        for c, h in enumerate(staging_keys, start=1):
            v = rec.get(h, "")
            ws.cell(row=r_idx, column=c, value=value_to_str(v))

    # Base formatting
    ws.freeze_panes = "A2"
    set_header_style(ws)

    # Create META sheet and copy columns AS-IS
    meta = wb.create_sheet(title="Meta_data_sheet")
    for c, h in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=c, value=h)
    for r_idx, rec in enumerate(records, start=2):
        for c, h in enumerate(META_COLS, start=1):
            v = rec.get(h, "")
            # Preserve raw content exactly; dump lists/dicts as JSON text
            meta.cell(row=r_idx, column=c, value=value_to_str(v))
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Normalize MAIN sheet on the SAME worksheet
    # Build a new table with only MAIN_COLS in specified order
    # Collect current values into memory to rebuild sheet
    rows = []
    for r in range(2, ws.max_row+1):
        rowd = {}
        for c in range(1, ws.max_column+1):
            h = ws.cell(row=1, column=c).value
            rowd[h] = ws.cell(row=r, column=c).value
        rows.append(rowd)

    # Clear ws
    for row in ws[ws.dimensions]:
        for cell in row:
            cell.value = None

    # Write MAIN headers
    for c, h in enumerate(MAIN_COLS, start=1):
        ws.cell(row=1, column=c, value=h)
    set_header_style(ws)

    # Write MAIN rows with numbering where required
    for r_idx, rowd in enumerate(rows, start=2):
        for c, h in enumerate(MAIN_COLS, start=1):
            v = rowd.get(h, "")
            if h in {"Test Steps / Procedure", "Validation / Acceptance Criteria"}:
                # If value is JSON array text, attempt to parse
                if isinstance(v, str):
                    try:
                        parsed = json.loads(v)
                    except Exception:
                        parsed = None
                else:
                    parsed = v
                if isinstance(parsed, list):
                    v = join_numbered(parsed)
            ws.cell(row=r_idx, column=c, value=v)

    # Rename Data -> TestPlan
    ws.title = "TestPlan"

    # Formatting
    autofit(ws)
    set_data_alignment(ws)
    estimate_row_heights(ws)
    apply_borders(ws)

    # Wrap specific columns
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column+1)]
    for c, h in enumerate(headers, start=1):
        if h in WRAP_COLS:
            for r in range(2, ws.max_row+1):
                ws.cell(row=r, column=c).alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)

    # Data validation on specific column only
    add_validation(ws)

    # Final sheet visibility enforcement: ensure no 'Data' sheet exists
    for sht in list(wb.sheetnames):
        if sht == 'Data':
            # If somehow exists, delete it
            del wb[sht]

    # Save
    tz = None
    try:
        tz = ZoneInfo('Asia/Kolkata')
    except Exception:
        pass
    now = datetime.now(tz) if tz else datetime.utcnow()
    ymd = now.strftime('%Y%m%d')
    hms = now.strftime('%H%M%S')
    out_name = f"{args.ip_name}_TestPlan_{ymd}_{hms}.xlsx"
    ensure_dir(args.output_dir)
    out_path = os.path.join(args.output_dir, out_name)
    wb.save(out_path)

    # OOXML validation
    if not validate_ooxml(out_path):
        print("XLSX validation failed", file=sys.stderr)
        sys.exit(2)

    # Emit path for workflow
    relpath = out_path
    with open('gen_path.txt', 'w', encoding='utf-8') as f:
        f.write(relpath)
    print(relpath)

if __name__ == '__main__':
    main()
