#!/usr/bin/env python3
import json
import os
import sys
import re
from datetime import datetime
from zoneinfo import ZoneInfo
from io import BytesIO
import zipfile
import subprocess
from typing import List, Dict, Any

from openpyxl import Workbook, load_workbook
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
]

MAIN_COLUMNS_ORDER = [
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

ALLOWED_DV = "Required,Blank, Not Required".replace(", ", ",")  # ensure no stray spaces except in label

BLUE_FILL = PatternFill(fill_type="solid", start_color="4472C4", end_color="4472C4")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def load_json_payload() -> List[Dict[str, Any]]:
    payload = os.getenv("JSON_PAYLOAD", "").strip()
    if not payload:
        # Support reading from stdin if provided
        if not sys.stdin.isatty():
            payload = sys.stdin.read().strip()
    if not payload:
        eprint("ERROR: JSON_PAYLOAD is empty")
        sys.exit(2)
    try:
        data = json.loads(payload)
    except Exception as e:
        eprint(f"ERROR: Invalid JSON payload: {e}")
        sys.exit(2)

    # Normalize: object with TC1/TC2 -> array
    if isinstance(data, dict) and all(k.startswith("TC") for k in data.keys()):
        # keep natural order of keys as provided
        normalized = [data[k] for k in data.keys()]
        return normalized
    if isinstance(data, dict):
        # If a single object, treat as one-row array
        return [data]
    if isinstance(data, list):
        if not data:
            eprint("ERROR: JSON array is empty")
            sys.exit(2)
        return data
    eprint("ERROR: Unsupported JSON structure")
    sys.exit(2)


def preserve_key_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.append(k)
    return seen


def load_register_map() -> Dict[str, str]:
    mapping_path = os.path.join("scripts", "register_map.json")
    try:
        with open(mapping_path, "r", encoding="utf-8") as f:
            mp = json.load(f)
            if isinstance(mp, dict):
                return {str(k): str(v) for k, v in mp.items()}
    except FileNotFoundError:
        eprint("WARNING: register_map.json not found; macro replacement will be a no-op")
    except Exception as e:
        eprint(f"WARNING: Failed to load register_map.json: {e}")
    return {}


def replace_macros_in_text(text: str, regmap: Dict[str, str]) -> str:
    if not regmap:
        return text
    # Replace longer keys first to avoid partial overlaps
    for k in sorted(regmap.keys(), key=len, reverse=True):
        v = regmap[k]
        text = text.replace(k, v)
    return text


def normalize_value(v: Any, regmap: Dict[str, str]) -> Any:
    # Preserve exact values; serialize lists/dicts as compact JSON strings
    if isinstance(v, str):
        return replace_macros_in_text(v, regmap)
    if isinstance(v, list) or isinstance(v, dict):
        s = json.dumps(v, separators=(",", ":"), ensure_ascii=False)
        return replace_macros_in_text(s, regmap)
    return v


def write_data_sheet(wb: Workbook, rows: List[Dict[str, Any]], headers: List[str]) -> None:
    ws = wb.active
    ws.title = "Data"

    # Header
    for c, h in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c, h in enumerate(headers, start=1):
            val = row.get(h, "")
            ws.cell(row=r_idx, column=c, value=val)
    # Freeze top row
    ws.freeze_panes = "A2"

    # Basic column width autofit by max text length capped
    for c, h in enumerate(headers, start=1):
        max_len = len(str(h))
        for r in range(2, len(rows) + 2):
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            l = len(str(v))
            if l > max_len:
                max_len = l
        adj_width = min(80, max(10, int(max_len * 0.9) + 2))
        ws.column_dimensions[get_column_letter(c)].width = adj_width

    # Apply thin borders to all populated cells
    for r in range(1, len(rows) + 2):
        for c in range(1, len(headers) + 1):
            ws.cell(row=r, column=c).border = THIN_BORDER


def create_meta_sheet(wb: Workbook, data_ws_name: str) -> None:
    ws_data = wb[data_ws_name]
    ws_meta = wb.create_sheet("Meta_data_sheet")
    # Header
    for c, h in enumerate(META_COLUMNS, start=1):
        cell = ws_meta.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Map headers from data sheet
    header_index = {ws_data.cell(row=1, column=i).value: i for i in range(1, ws_data.max_column + 1)}

    for r in range(2, ws_data.max_row + 1):
        for c, h in enumerate(META_COLUMNS, start=1):
            src_col = header_index.get(h)
            val = ws_data.cell(row=r, column=src_col).value if src_col else ""
            ws_meta.cell(row=r, column=c, value=val)

    # Autofit meta columns
    for c, h in enumerate(META_COLUMNS, start=1):
        max_len = len(str(h))
        for r in range(2, ws_meta.max_row + 1):
            v = ws_meta.cell(row=r, column=c).value
            if v is None:
                continue
            l = len(str(v))
            if l > max_len:
                max_len = l
        ws_meta.column_dimensions[get_column_letter(c)].width = min(100, max(10, int(max_len * 0.9) + 2))

    # Very hidden
    ws_meta.sheet_state = 'veryHidden'


def reorder_and_format_testplan(wb: Workbook) -> None:
    # Rename Data to TestPlan
    ws = wb["Data"]
    ws.title = "TestPlan"

    # Build mapping of existing headers
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column + 1)]
    header_index = {h: i for i, h in enumerate(headers, start=1)}

    # Create a new ordered set of columns as per MAIN_COLUMNS_ORDER from existing
    # Copy values into a temporary in-memory matrix
    data_matrix: List[List[Any]] = []
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for h in MAIN_COLUMNS_ORDER:
            c_idx = header_index.get(h)
            row_vals.append(ws.cell(row=r, column=c_idx).value if c_idx else "")
        data_matrix.append(row_vals)

    # Clear existing sheet content completely
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column if ws.max_column else 1)

    # Write new headers in the required order
    for c, h in enumerate(MAIN_COLUMNS_ORDER, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Write back data
    for r_idx, row_vals in enumerate(data_matrix, start=2):
        for c_idx, v in enumerate(row_vals, start=1):
            ws.cell(row=r_idx, column=c_idx, value=v)

    # Enable wrap text for specified columns
    wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
    col_letter_by_name = {MAIN_COLUMNS_ORDER[i-1]: get_column_letter(i) for i in range(1, len(MAIN_COLUMNS_ORDER)+1)}

    for r in range(2, ws.max_row + 1):
        for name in wrap_cols:
            c = MAIN_COLUMNS_ORDER.index(name) + 1
            cell = ws.cell(row=r, column=c)
            cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")

    # Number inside-cell lists for two columns
    def renumber_cell(text: Any) -> Any:
        if text is None:
            return text
        s = str(text)
        # Split by lines
        parts = [p for p in re.split(r"[\r\n]+", s) if p is not None and len(p.strip()) > 0]
        if not parts:
            return s
        out_lines = []
        idx = 1
        for p in parts:
            # Remove existing bullets or numbering like '14.1.', '1)', '-', '•'
            p2 = re.sub(r"^\s*(?:[-•\u2022]+|\d+(?:[\.)]|(?:\.\d+)*\.)\s*)", "", p.strip())
            out_lines.append(f"{idx}. {p2}")
            idx += 1
        return "\n".join(out_lines)

    for col_name in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        c = MAIN_COLUMNS_ORDER.index(col_name) + 1
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=c)
            cell.value = renumber_cell(cell.value)
            cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")

    # Autofit columns and approximate row heights
    for c in range(1, len(MAIN_COLUMNS_ORDER) + 1):
        header = ws.cell(row=1, column=c).value
        max_len = len(str(header)) if header is not None else 0
        for r in range(2, ws.max_row + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            lines = str(v).split("\n")
            for line in lines:
                max_len = max(max_len, len(line))
        ws.column_dimensions[get_column_letter(c)].width = min(100, max(10, int(max_len * 0.9) + 2))

    # Rough row height based on wrapped lines (approx 15 points per line)
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, len(MAIN_COLUMNS_ORDER) + 1):
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            max_lines = max(max_lines, len(str(v).split("\n")))
        ws.row_dimensions[r].height = min(300, max(15, max_lines * 15))

    # Header style reaffirm
    for c in range(1, len(MAIN_COLUMNS_ORDER) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Data rows style: vertical top, text left; numeric/index center
    index_col = MAIN_COLUMNS_ORDER.index("Index") + 1
    for r in range(2, ws.max_row + 1):
        for c in range(1, len(MAIN_COLUMNS_ORDER) + 1):
            cell = ws.cell(row=r, column=c)
            if c == index_col:
                cell.alignment = Alignment(vertical="top", horizontal="center", wrap_text=True)
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left", wrap_text=True)
            cell.border = THIN_BORDER

    # Data validation for Code Generation (Required / Not)
    code_col = MAIN_COLUMNS_ORDER.index("Code Generation (Required / Not)") + 1
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
    dv.error = 'Select a value from the list: Required, Blank, Not Required'
    dv.errorTitle = 'Invalid Selection'
    ws.add_data_validation(dv)
    if ws.max_row >= 2:
        dv.add(f"{get_column_letter(code_col)}2:{get_column_letter(code_col)}{ws.max_row}")


def enforce_sheet_visibility(wb: Workbook) -> None:
    # Only TestPlan (visible) and Meta_data_sheet (veryHidden)
    names = [ws.title for ws in wb.worksheets]
    if "TestPlan" not in names or "Meta_data_sheet" not in names:
        raise RuntimeError("Mandatory sheets missing after normalization")
    # Ensure no sheet named 'Data'
    if "Data" in names:
        idx = names.index("Data")
        ws = wb.worksheets[idx]
        wb.remove(ws)


def validate_xlsx_binary(buf: bytes) -> None:
    try:
        with zipfile.ZipFile(BytesIO(buf), 'r') as zf:
            if '[Content_Types].xml' not in zf.namelist():
                raise ValueError('Missing [Content_Types].xml')
            if 'xl/workbook.xml' not in zf.namelist():
                raise ValueError('Missing xl/workbook.xml')
    except Exception as e:
        raise RuntimeError(f"XLSX ZIP validation failed: {e}")
    # Re-open with openpyxl
    try:
        bio = BytesIO(buf)
        _ = load_workbook(bio)
    except Exception as e:
        raise RuntimeError(f"openpyxl load validation failed: {e}")


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--ip-name', default=os.getenv('IP_NAME', 'GPIO'))
    parser.add_argument('--output-dir', default=os.getenv('OUTPUT_DIR', 'Test_Output/GPIO/TestPlan'))
    parser.add_argument('--commit', default=os.getenv('COMMIT_CHANGES', 'true'))
    args = parser.parse_args()

    rows = load_json_payload()

    # Load register map and normalize values (macro replacement only; no mutation of data otherwise)
    regmap = load_register_map()
    norm_rows: List[Dict[str, Any]] = []
    for row in rows:
        norm_row = {}
        for k, v in row.items():
            norm_row[k] = normalize_value(v, regmap)
        norm_rows.append(norm_row)

    # Normalize schema order
    headers = preserve_key_order(norm_rows)

    # Create workbook and data sheet
    wb = Workbook()
    write_data_sheet(wb, norm_rows, headers)

    # Create meta sheet from Data
    create_meta_sheet(wb, "Data")

    # Reorganize Data -> TestPlan and format
    reorder_and_format_testplan(wb)

    # Enforce final sheet visibility and ensure no 'Data' sheet remains
    enforce_sheet_visibility(wb)

    # Prepare output path and save to bytes for validation
    ist = datetime.now(ZoneInfo('Asia/Kolkata'))
    ts = ist.strftime('%Y%m%d_%H%M%S')
    ip = args.ip_name
    out_dir = args.output_dir
    out_name = f"{ip}_TestPlan_{ts}.xlsx"
    final_path = os.path.join(out_dir, out_name)

    os.makedirs(out_dir, exist_ok=True)

    bio = BytesIO()
    wb.save(bio)
    buf = bio.getvalue()

    # Validate OOXML binary
    validate_xlsx_binary(buf)

    # Save to disk
    with open(final_path, 'wb') as f:
        f.write(buf)

    print(f"FINAL_OUTPUT_PATH={final_path}")

    # Commit only the XLSX if requested
    do_commit = str(args.commit).lower() in ("1", "true", "yes")
    if do_commit:
        try:
            subprocess.run(["git", "config", "user.name", "github-actions"], check=True)
            subprocess.run(["git", "config", "user.email", "github-actions@github.com"], check=True)
            subprocess.run(["git", "add", final_path], check=True)
            # Ensure only that file is staged
            subprocess.run(["git", "commit", "-m", "Final formatted Excel generated from JSON input"], check=True)
            subprocess.run(["git", "push"], check=True)
        except subprocess.CalledProcessError as e:
            eprint(f"ERROR: Git commit/push failed: {e}")
            sys.exit(3)


if __name__ == '__main__':
    main()
