#!/usr/bin/env python3
import os, json, re, zipfile
from datetime import datetime, timezone, timedelta
from pathlib import Path
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Constants per spec
META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLUMNS = [
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

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

HEADER_FILL = PatternFill("solid", fgColor="4F81BD")  # visual only blue
HEADER_FONT = Font(bold=True)
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
DATA_TEXT_ALIGN = Alignment(horizontal="left", vertical="top", wrap_text=True)
DATA_CENTER_ALIGN = Alignment(horizontal="center", vertical="top", wrap_text=True)
THIN_BORDER = Border(left=Side(style="thin", color="000000"),
                     right=Side(style="thin", color="000000"),
                     top=Side(style="thin", color="000000"),
                     bottom=Side(style="thin", color="000000"))


def parse_json_input(json_str):
    try:
        data = json.loads(json_str)
    except Exception as e:
        raise SystemExit(f"JSON parse failure: {e}")
    # Normalize to array of dicts
    if isinstance(data, dict):
        # Preserve first-seen order of values
        rows = list(data.values())
    elif isinstance(data, list):
        rows = data
    else:
        raise SystemExit("Invalid JSON: top-level must be object or array")
    if not rows:
        raise SystemExit("Invalid JSON: empty dataset")
    # Ensure all rows are dicts
    for i, r in enumerate(rows):
        if not isinstance(r, dict):
            raise SystemExit(f"Invalid JSON row at index {i}: not an object")
    return rows


def union_keys_preserve_first_seen(rows):
    seen = []
    sset = set()
    for r in rows:
        for k in r.keys():
            if k not in sset:
                sset.add(k)
                seen.append(k)
    return seen


def approximate_col_width(values):
    # Compute approximate Excel column width based on max string length
    max_len = 0
    for v in values:
        s = "" if v is None else str(v)
        # For wrapped text, consider the longest line
        for line in s.splitlines() or [s]:
            max_len = max(max_len, len(line))
    # Padding; cap to a reasonable width
    return min(max(10, max_len + 2), 80)


def split_numbered_items(text):
    if not text:
        return []
    s = str(text).strip()
    # If lines starting with '-' or '–', split by lines beginning with dash
    lines = [ln.strip() for ln in s.splitlines() if ln.strip()]
    items = []
    dashed = [ln for ln in lines if ln.lstrip().startswith(('-', '–'))]
    if dashed and len(dashed) == len(lines):
        for ln in lines:
            # remove leading dash and possible space
            items.append(ln.lstrip()[1:].strip())
        return items
    # Else, attempt split on patterns like '1) ... 2) ...'
    parts = re.split(r"\s*\d+\)\s*", s)
    parts = [p.strip() for p in parts if p.strip()]
    if parts:
        return parts
    # Fallback: split by semicolons
    parts = [p.strip() for p in re.split(r"\s*;\s*", s) if p.strip()]
    if parts:
        return parts
    # Else, return as single item
    return [s]


def enforce_incell_numbering(text):
    items = split_numbered_items(text)
    if not items:
        return ""
    numbered = [f"{i+1}. {itm}" for i, itm in enumerate(items)]
    return "\n".join(numbered)


def build_workbook(rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"  # authoritative staging sheet per spec

    headers = union_keys_preserve_first_seen(rows)

    # Write headers
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = HEADER_FONT
    ws.freeze_panes = "A2"

    # Write data
    for r_idx, rec in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=rec.get(h, ""))

    # Approximate auto-fit
    for c_idx, h in enumerate(headers, start=1):
        col_values = [h] + [rec.get(h, "") for rec in rows]
        ws.column_dimensions[ws.cell(1, c_idx).column_letter].width = approximate_col_width(col_values)

    # Create META sheet
    ws_meta = wb.create_sheet("Meta_data_sheet")
    for c, h in enumerate(META_COLUMNS, start=1):
        ws_meta.cell(row=1, column=c, value=h).font = HEADER_FONT
    for r_idx, rec in enumerate(rows, start=2):
        for c, h in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=r_idx, column=c, value=rec.get(h, ""))
    ws_meta.sheet_state = "veryHidden"

    # Normalize MAIN sheet on the same sheet (no new visible worksheet)
    ws_main = wb["Data"]
    ws_main.title = "TestPlan"

    # Extract records from current sheet to dicts keyed by header
    current_headers = [ws_main.cell(1, c).value for c in range(1, ws_main.max_column + 1)]
    records = []
    for r in range(2, ws_main.max_row + 1):
        rec = {}
        for c, h in enumerate(current_headers, start=1):
            rec[h] = ws_main.cell(r, c).value
        records.append(rec)

    # Clear sheet content (keep the sheet)
    ws_main.delete_rows(1, ws_main.max_row)

    # Write MAIN columns in specified order, dropping non-main columns
    for c, h in enumerate(MAIN_COLUMNS, start=1):
        cell = ws_main.cell(row=1, column=c, value=h)
        cell.font = HEADER_FONT
        cell.alignment = HEADER_ALIGN
        cell.fill = HEADER_FILL

    for r_idx, rec in enumerate(records, start=2):
        for c_idx, h in enumerate(MAIN_COLUMNS, start=1):
            val = rec.get(h, "")
            # Enforce numbering in specific columns on TestPlan only
            if h in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = enforce_incell_numbering(val)
            ws_main.cell(row=r_idx, column=c_idx, value=val)

    # Formatting: wrap text for specified columns, borders, alignments
    max_row = ws_main.max_row
    max_col = ws_main.max_column

    # Column index map
    col_index = {ws_main.cell(1, c).value: c for c in range(1, max_col + 1)}

    for c in range(1, max_col + 1):
        h = ws_main.cell(1, c).value
        # Approximate auto-fit widths on main sheet
        col_vals = [ws_main.cell(1, c).value]
        for r in range(2, max_row + 1):
            col_vals.append(ws_main.cell(r, c).value)
        ws_main.column_dimensions[ws_main.cell(1, c).column_letter].width = approximate_col_width(col_vals)

    # Apply styles and borders to all populated cells
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws_main.cell(row=r, column=c)
            if r == 1:
                cell.font = HEADER_FONT
                cell.alignment = HEADER_ALIGN
                cell.fill = HEADER_FILL
            else:
                header = ws_main.cell(1, c).value
                if header in WRAP_COLUMNS:
                    cell.alignment = DATA_TEXT_ALIGN
                elif header == "Index":
                    cell.alignment = DATA_CENTER_ALIGN
                else:
                    cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=False)
            cell.border = THIN_BORDER

    # Autofit row heights after wrapping (approximate based on newline count)
    base_height = 15
    for r in range(2, max_row + 1):
        lines = 1
        for h in WRAP_COLUMNS:
            c = col_index.get(h)
            if c:
                v = ws_main.cell(r, c).value
                if v is None:
                    continue
                s = str(v)
                # Consider existing newlines; if none, estimate by length/width
                n = s.count("\n") + 1
                lines = max(lines, n)
        ws_main.row_dimensions[r].height = base_height * lines + 2

    # Data validation only for Code Generation (Required / Not)
    code_col = col_index.get("Code Generation (Required / Not)")
    if code_col:
        start_row = 2
        end_row = max_row if max_row >= 2 else 2
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        addr = f"{ws_main.cell(1, code_col).column_letter}{start_row}:{ws_main.cell(1, code_col).column_letter}{end_row}"
        dv.add(addr)
        ws_main.add_data_validation(dv)

    # Safety check: only TestPlan (visible) and Meta_data_sheet (veryHidden) should exist
    if "Data" in [ws.title for ws in wb.worksheets]:
        # Attempt to delete if exists for any reason
        try:
            del wb["Data"]
        except Exception:
            raise SystemExit("Validation failure: Could not remove residual 'Data' sheet")

    names = [ws.title for ws in wb.worksheets]
    if sorted(names) != sorted(["TestPlan", "Meta_data_sheet"]):
        raise SystemExit(f"Validation failure: Unexpected sheets present: {names}")

    # Return workbook
    return wb


def validate_xlsx(path):
    # Validate as true XLSX (zip-based) and openable
    if not zipfile.is_zipfile(path):
        raise SystemExit("Validation failure: Not a ZIP-based XLSX")
    try:
        wb = load_workbook(path, read_only=True, data_only=True)
        names = [ws.title for ws in wb.worksheets]
        if sorted(names) != sorted(["TestPlan", "Meta_data_sheet"]):
            raise SystemExit(f"Validation failure after save: Unexpected sheets {names}")
        # Check veryHidden
        meta = wb["Meta_data_sheet"]
        if getattr(meta, 'sheet_state', None) != 'veryHidden':
            raise SystemExit("Validation failure: Meta_data_sheet is not Very Hidden")
        wb.close()
    except Exception as e:
        raise SystemExit(f"Validation failure: Cannot open workbook: {e}")


def main():
    json_str = os.environ.get("JSON_DATA")
    if not json_str:
        raise SystemExit("Missing JSON_DATA environment variable")

    rows = parse_json_input(json_str)

    # Build workbook with all required transformations
    wb = build_workbook(rows)

    # Output parameters
    ip_name = os.environ.get("IP_NAME", "GPIO").strip()
    out_dir = os.environ.get("OUTPUT_DIR", f"Test_Output/{ip_name}/TestPlan").strip()

    # IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    fname = f"{ip_name}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"

    out_path = Path(out_dir) / fname
    out_path.parent.mkdir(parents=True, exist_ok=True)

    # Save
    wb.save(out_path)

    # Validate saved file
    validate_xlsx(out_path)

    # Emit path for subsequent steps
    with open("final_path.txt", "w", encoding="utf-8") as f:
        f.write(str(out_path))
    print(str(out_path))

if __name__ == "__main__":
    main()
