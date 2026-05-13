import json, os, re, zipfile
from datetime import datetime, timedelta, timezone
from collections import OrderedDict
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Config
IP_NAME = "PCIE"
INPUT_JSON_PATH = Path("tools/testplan_payload.json")
OUTPUT_DIR = Path("Test_Output") / IP_NAME / "TestPlan"
LAST_OUTPUT_PATH_FILE = Path("tools/last_output_path.txt")

# Columns
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
# Allow both singular and plural macro define keys; include only those present
META_ORDER_CANDIDATES = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    "Hidden_Macro_Define",
    "Hidden_Macro_Defines",
    "Hidden_Skip_Array_Definition",
]

BLUE_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def load_json_array(path: Path):
    if not path.exists():
        raise SystemExit(f"JSON payload not found at {path}")
    with path.open("r", encoding="utf-8") as f:
        try:
            data = json.load(f, object_pairs_hook=OrderedDict)
        except json.JSONDecodeError as e:
            raise SystemExit(f"Invalid JSON: {e}")
    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit("JSON must be a non-empty array of objects")
    for i, rec in enumerate(data):
        if not isinstance(rec, dict):
            raise SystemExit(f"JSON element at index {i} is not an object")
    return data


def union_keys_preserve_order(records):
    seen = OrderedDict()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def write_base_sheet(wb: Workbook, records, schema_keys):
    ws = wb.active
    ws.title = "Data"
    # Header
    for col_idx, key in enumerate(schema_keys, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
    # Rows
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(schema_keys, start=1):
            val = rec.get(key, "")
            ws.cell(row=r, column=c, value=val)
    ws.freeze_panes = "A2"
    return ws


def create_meta_sheet(wb: Workbook, records, schema_keys):
    meta_keys = [k for k in META_ORDER_CANDIDATES if k in schema_keys]
    ws = wb.create_sheet("Meta_data_sheet")
    # Header
    for col_idx, key in enumerate(meta_keys, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL
    # Rows
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(meta_keys, start=1):
            ws.cell(row=r, column=c, value=rec.get(key, ""))
    # Very hidden
    ws.sheet_state = 'veryHidden'
    return ws


def renumber_items(text: str) -> str:
    if not isinstance(text, str) or not text.strip():
        return text or ""
    # First split by newlines; if single line, also split on numeric markers like 1) 2) 3) or 1. 2.
    raw = text.strip()
    parts = []
    if "\n" in raw:
        parts = [p.strip() for p in raw.splitlines() if p.strip()]
    else:
        # Split on patterns like '1) ', '2) ', '1. ', etc.
        tokens = re.split(r"\s*\d+\s*[\)\.]\s*", raw)
        parts = [p.strip() for p in tokens if p.strip()]
        # If splitting yielded only one part, try to split by ';' as a fallback
        if len(parts) <= 1 and ";" in raw:
            parts = [p.strip() for p in raw.split(";") if p.strip()]
    if not parts:
        return raw
    numbered = []
    for i, p in enumerate(parts, start=1):
        numbered.append(f"{i}. {p}")
    return "\n".join(numbered)


def rebuild_testplan_sheet_in_place(wb: Workbook, records, schema_keys):
    # Rename Data to TestPlan (must be same sheet)
    ws = wb["Data"]
    ws.title = "TestPlan"

    # Build final columns by taking MAIN_ORDER in order, include only those present
    final_cols = [k for k in MAIN_ORDER if k in schema_keys]

    # Rebuild sheet content
    # Clear all existing rows
    ws.delete_rows(1, ws.max_row)

    # Header
    for col_idx, key in enumerate(final_cols, start=1):
        cell = ws.cell(row=1, column=col_idx, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = BLUE_FILL

    # Data with numbering enforced in required columns
    for r, rec in enumerate(records, start=2):
        for c, key in enumerate(final_cols, start=1):
            val = rec.get(key, "")
            if key in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = renumber_items(val)
            ws.cell(row=r, column=c, value=val)

    # Formatting
    ws.freeze_panes = "A2"

    # Wrap specific columns
    wrap_cols = {k for k in ("Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria") if k in final_cols}
    # Build column index mapping
    col_index = {key: idx+1 for idx, key in enumerate(final_cols)}

    # Determine column widths by max string length
    maxlen = {key: len(key) for key in final_cols}
    for r in range(2, ws.max_row + 1):
        for key in final_cols:
            c = col_index[key]
            v = ws.cell(row=r, column=c).value
            s = str(v) if v is not None else ""
            if len(s) > maxlen[key]:
                maxlen[key] = len(s)

    for key in final_cols:
        c = col_index[key]
        width = min(max(10, maxlen[key] + 2), 100)
        ws.column_dimensions[ws.cell(row=1, column=c).column_letter].width = width

    # Apply styles row by row
    for r in range(1, ws.max_row + 1):
        for key in final_cols:
            c = col_index[key]
            cell = ws.cell(row=r, column=c)
            # Header row styles already applied; reinforce borders
            cell.border = THIN_BORDER
            if r == 1:
                continue
            # Data rows
            if key == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=(key in wrap_cols))
            else:
                align_h = "left"
                cell.alignment = Alignment(horizontal=align_h, vertical="top", wrap_text=(key in wrap_cols))

        # Approximate auto row height based on line breaks for wrapped cols
        if r > 1:
            lines = 1
            for key in wrap_cols:
                c = col_index[key]
                v = ws.cell(row=r, column=c).value
                if isinstance(v, str):
                    cnt = v.count("\n") + 1
                    if cnt > lines:
                        lines = cnt
            ws.row_dimensions[r].height = min(15 * lines, 300)

    # Data validation for Code Generation (Required / Not)
    cg_col_name = "Code Generation (Required / Not)"
    if cg_col_name in col_index and ws.max_row >= 2:
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        start_cell = ws.cell(row=2, column=col_index[cg_col_name]).coordinate
        end_cell = ws.cell(row=ws.max_row, column=col_index[cg_col_name]).coordinate
        dv.add(f"{start_cell}:{end_cell}")

    return ws


def validate_xlsx(path: Path):
    if not path.exists():
        return False
    if not zipfile.is_zipfile(path):
        return False
    with zipfile.ZipFile(path, 'r') as z:
        names = set(z.namelist())
        if "[Content_Types].xml" not in names:
            return False
        if "xl/workbook.xml" not in names:
            return False
    return True


def main():
    records = load_json_array(INPUT_JSON_PATH)
    schema_keys = union_keys_preserve_order(records)

    wb = Workbook()
    write_base_sheet(wb, records, schema_keys)
    create_meta_sheet(wb, records, schema_keys)
    rebuild_testplan_sheet_in_place(wb, records, schema_keys)

    # Safety: ensure only TestPlan and Meta_data_sheet exist; delete stray Data if any
    for title in list(wb.sheetnames):
        if title not in ("TestPlan", "Meta_data_sheet"):
            # Delete any unexpected sheets
            if title == "Data":
                std = wb[title]
                wb.remove(std)

    # Timestamp in IST
    IST = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(IST)
    filename = f"{IP_NAME}_TestPlan_{now:%Y%m%d}_{now:%H%M%S}.xlsx"
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    out_path = OUTPUT_DIR / filename

    wb.save(out_path)

    if not validate_xlsx(out_path):
        raise SystemExit("Generated file failed XLSX validation")

    # Persist output path for the workflow commit step
    LAST_OUTPUT_PATH_FILE.parent.mkdir(parents=True, exist_ok=True)
    LAST_OUTPUT_PATH_FILE.write_text(str(out_path), encoding="utf-8")

if __name__ == "__main__":
    main()
