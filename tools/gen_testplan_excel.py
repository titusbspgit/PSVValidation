#!/usr/bin/env python3
import json
import sys
import argparse
from datetime import datetime, timezone, timedelta
from pathlib import Path
import zipfile
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    "Hidden_Macro_Define",
    "Hidden_Skip_Array_Definition",
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

BLUE_FILL = PatternFill("solid", fgColor="FF4472C4")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"),
    top=Side(style="thin"), bottom=Side(style="thin")
)


def ist_now():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist)


def normalize_schema(rows):
    if not isinstance(rows, list) or len(rows) == 0:
        raise ValueError("json_data must be a non-empty array")
    # Determine union of keys preserving first-seen order
    seen = []
    for r in rows:
        if not isinstance(r, dict):
            raise ValueError("Each array element must be an object")
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    # Normalize each row to have all keys, filling missing with ''
    norm = []
    for r in rows:
        norm.append({k: r.get(k, "") for k in seen})
    return seen, norm


def write_data_sheet(wb, headers, rows):
    ws = wb.active
    ws.title = "Data"
    # Header
    ws.append(headers)
    # Rows
    for r in rows:
        ws.append([r.get(h, "") for h in headers])
    # Freeze header
    ws.freeze_panes = "A2"
    # Bold header
    for c in range(1, len(headers)+1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
    # Best-effort auto-fit columns by content length
    for col_idx, h in enumerate(headers, start=1):
        max_len = len(str(h))
        for row_idx in range(2, ws.max_row+1):
            v = ws.cell(row=row_idx, column=col_idx).value
            if v is None:
                continue
            ln = max((len(str(v)) for v in str(v).splitlines()), default=0)
            if ln > max_len:
                max_len = ln
        width = max(10, min(80, max_len + 2))
        ws.column_dimensions[get_column_letter(col_idx)].width = width
    return ws


def create_meta_sheet(wb, data_ws, headers):
    meta = wb.create_sheet("Meta_data_sheet")
    # Copy only META columns that actually exist in headers, preserving META_COLUMNS order
    meta_headers = [c for c in META_COLUMNS if c in headers]
    if meta_headers:
        meta.append(meta_headers)
        # Map header -> column index in data_ws
        col_map = {h: i+1 for i, h in enumerate(headers)}
        for r in range(2, data_ws.max_row+1):
            row_vals = []
            for h in meta_headers:
                col = col_map[h]
                row_vals.append(data_ws.cell(row=r, column=col).value)
            meta.append(row_vals)
    # Very hidden
    meta.sheet_state = 'veryHidden'
    return meta


def rebuild_testplan_on_same_sheet(data_ws, headers):
    # Build the new header set for main order. If a column is missing, include it with blanks.
    present = set(headers)
    new_headers = MAIN_ORDER.copy()
    # Rebuild all rows into a list of lists according to new_headers
    records = []
    for r in range(2, data_ws.max_row+1):
        row_map = {headers[c-1]: data_ws.cell(row=r, column=c).value for c in range(1, len(headers)+1)}
        records.append([row_map.get(h, "") for h in new_headers])
    # Clear existing sheet and rewrite
    for row in data_ws[1:data_ws.max_row]:
        for cell in row:
            cell.value = None
            cell._style = None
    data_ws.delete_rows(1, data_ws.max_row)
    data_ws.append(new_headers)
    for rec in records:
        data_ws.append(rec)

    # Rename to TestPlan
    data_ws.title = "TestPlan"

    # Numbering inside cells for two columns
    def renumber(val):
        if val is None:
            return val
        s = str(val)
        parts = s.splitlines()
        if len(parts) <= 1:
            return s
        out = []
        for i, line in enumerate(parts, start=1):
            ln = line.strip()
            # strip any existing numeric bullets like '1)', '1.', etc.
            import re
            ln = re.sub(r"^\s*\d+[\.)]\s*", "", ln)
            out.append(f"{i}. {ln}")
        return "\n".join(out)

    # Apply renumbering to the two columns if present
    hdr_to_col = {h: i+1 for i, h in enumerate(new_headers)}
    for target in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if target in hdr_to_col:
            cidx = hdr_to_col[target]
            for r in range(2, data_ws.max_row+1):
                v = data_ws.cell(row=r, column=cidx).value
                data_ws.cell(row=r, column=cidx).value = renumber(v)

    # Formatting
    # Header formatting
    for c in range(1, len(new_headers)+1):
        cell = data_ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
        cell.fill = BLUE_FILL

    # Data rows formatting
    # Column alignments: text left/top, index likely numeric centered
    for r in range(2, data_ws.max_row+1):
        for c in range(1, len(new_headers)+1):
            cell = data_ws.cell(row=r, column=c)
            # Default left/top
            align = Alignment(horizontal='left', vertical='top', wrap_text=False)
            if new_headers[c-1] == "Index":
                align = Alignment(horizontal='center', vertical='top', wrap_text=False)
            # Wrap for selected columns
            if new_headers[c-1] in [
                "Test Description",
                "Remarks",
                "Test Steps / Procedure",
                "Validation / Acceptance Criteria",
            ]:
                align = Alignment(horizontal='left', vertical='top', wrap_text=True)
            cell.alignment = align

    # Borders for all populated cells
    for r in range(1, data_ws.max_row+1):
        for c in range(1, len(new_headers)+1):
            data_ws.cell(row=r, column=c).border = THIN_BORDER

    # Best-effort auto-fit columns and approximate row heights
    for col_idx, h in enumerate(new_headers, start=1):
        max_len = len(str(h))
        for row_idx in range(2, data_ws.max_row+1):
            v = data_ws.cell(row=row_idx, column=col_idx).value
            if v is None:
                continue
            lines = str(v).splitlines()
            ln = max((len(s) for s in lines), default=0)
            if ln > max_len:
                max_len = ln
            # approximate row height by number of lines
            if len(lines) > 1:
                base = 15
                data_ws.row_dimensions[row_idx].height = base + (len(lines)-1) * 12
        width = max(10, min(80, max_len + 2))
        data_ws.column_dimensions[get_column_letter(col_idx)].width = width

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in hdr_to_col:
        cg_col = hdr_to_col["Code Generation (Required / Not)"]
        start = 2
        end = data_ws.max_row
        rng = f"{get_column_letter(cg_col)}{start}:{get_column_letter(cg_col)}{end}"
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        data_ws.add_data_validation(dv)
        dv.add(rng)

    return new_headers


def validate_xlsx(path: Path) -> bool:
    try:
        if not zipfile.is_zipfile(path):
            return False
        with zipfile.ZipFile(path, 'r') as zf:
            names = set(zf.namelist())
            required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
            return required.issubset(names)
    except Exception:
        return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--json', required=True)
    ap.add_argument('--output-dir', required=True)
    ap.add_argument('--ip-name', required=True)
    ap.add_argument('--commit-prefix', default="Add TestPlan")
    args = ap.parse_args()

    json_path = Path(args.json)
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # Read and validate JSON
    data = json.loads(json_path.read_text(encoding='utf-8'))
    headers, rows = normalize_schema(data)

    # Create workbook and Data sheet
    wb = Workbook()
    data_ws = write_data_sheet(wb, headers, rows)

    # Create META sheet (very hidden)
    create_meta_sheet(wb, data_ws, headers)

    # Normalize main sheet on same worksheet and format
    new_headers = rebuild_testplan_on_same_sheet(data_ws, headers)

    # Safety: Ensure no sheet named 'Data'
    for ws in list(wb.worksheets):
        if ws.title == 'Data':
            wb.remove(ws)

    # Save with IST timestamped name
    ts = ist_now().strftime('%Y%m%d_%H%M%S')
    fname = f"{args.ip_name}_TestPlan_{ts}.xlsx"
    fpath = out_dir / fname
    wb.save(fpath)

    # Validate OOXML zip structure
    if not validate_xlsx(fpath):
        print("ERROR: XLSX validation failed", file=sys.stderr)
        sys.exit(2)

    # Write commit message file for the calling step
    commit_msg = f"{args.commit_prefix} {ts}"
    (out_dir / '.last_commit_msg.txt').write_text(commit_msg, encoding='utf-8')
    # Also write last output filename helper (optional)
    (out_dir / '.last_output_name.txt').write_text(str(fpath), encoding='utf-8')

    # Print a brief summary for logs
    print(f"ROWS={len(rows)} COLUMNS={len(new_headers)} OUTPUT={fpath}")

if __name__ == '__main__':
    main()
