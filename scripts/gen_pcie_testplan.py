#!/usr/bin/env python3
# Deterministic fallback Excel generator for PCIE TestPlan
# - Consumes embedded JSON (array of objects)
# - Produces a single OOXML .xlsx with strict formatting rules
# - Writes the final file to Test_Output/PCIE/TestPlan/<PCIE_TestPlan_YYYYMMDD_HHMMSS.xlsx>
# - Emits generated_path.txt and ist_timestamp.txt for the workflow to commit with the correct message

import json
import os
import re
import zipfile
from datetime import datetime

try:
    from zoneinfo import ZoneInfo
except Exception:
    # Fallback for very old Pythons (not expected on GH runners)
    from pytz import timezone as ZoneInfo  # type: ignore

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ------------------------ INPUT JSON (embedded) ------------------------
JSON_INPUT = r'''__JSON_PLACEHOLDER__'''
# ---------------------------------------------------------------------

# Configuration
IP_NAME = "PCIE"
OUTPUT_DIR = os.environ.get("EXPORT_DIR", os.path.join("Test_Output", IP_NAME, "TestPlan"))
TIMEZONE = ZoneInfo("Asia/Kolkata")

# Required column orders
META_COLS_CANONICAL = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    # Accept either singular or plural macro key; will pick what exists in data
    "Hidden_Macro_Define",
    "Hidden_Skip_Array_Definition",
]

MAIN_COLS_ORDER = [
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

# Helper: schema union preserving first-seen order
def build_schema_union(rows):
    seen = []
    seen_set = set()
    for obj in rows:
        for k in obj.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    return seen

# Helper: approximate autofit width from text length
# Use a cap to avoid excessive widths
COL_WIDTH_MIN = 10
COL_WIDTH_MAX = 80

def calc_width(text: str) -> int:
    if text is None:
        return COL_WIDTH_MIN
    s = str(text)
    # Tab/newlines increase perceived width slightly
    s = s.replace("\t", "    ")
    max_line = max((len(line) for line in s.splitlines()), default=0)
    return max(COL_WIDTH_MIN, min(COL_WIDTH_MAX, max_line + 2))

# Renumber content inside a single cell as "1. ...\n2. ..."
# Keep original order; strip any existing leading numbering/bullets
LEADING_NUM_RE = re.compile(r"^\s*([0-9]+[\).:-]?\s*|-\s*|•\s*|\*\s*)")

def renumber_cell(val: str) -> str:
    if val is None:
        return ""
    text = str(val).strip()
    if not text:
        return ""
    # Split on newlines; ignore empty lines
    raw_lines = [ln for ln in re.split(r"\r?\n", text) if ln.strip()]
    out_lines = []
    for idx, ln in enumerate(raw_lines, start=1):
        ln2 = LEADING_NUM_RE.sub("", ln.strip())
        out_lines.append(f"{idx}. {ln2}")
    return "\n".join(out_lines)

# Validate XLSX is a proper OOXML zip
REQUIRED_XLSX_ENTRIES = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}

def validate_xlsx(path: str) -> bool:
    try:
        with zipfile.ZipFile(path, "r") as zf:
            names = set(zf.namelist())
        return REQUIRED_XLSX_ENTRIES.issubset(names)
    except Exception:
        return False

# Load JSON
rows = json.loads(JSON_INPUT)
if not isinstance(rows, list) or len(rows) == 0:
    raise SystemExit("ERROR: JSON input is empty or not an array")

# Build full schema (for staging Data sheet)
schema = build_schema_union(rows)

# Create workbook and staging sheet "Data"
wb = Workbook()
ws = wb.active
ws.title = "Data"

# Write headers
for c, key in enumerate(schema, start=1):
    ws.cell(row=1, column=c, value=key)

# Write rows exactly preserving values
for r, obj in enumerate(rows, start=2):
    for c, key in enumerate(schema, start=1):
        val = obj.get(key, "")
        ws.cell(row=r, column=c, value=val)

# Base formatting on staging sheet
ws.freeze_panes = "A2"
header_font = Font(bold=True)
for c in range(1, len(schema) + 1):
    cell = ws.cell(row=1, column=c)
    cell.font = header_font

# Approximate auto-fit column widths
for c, key in enumerate(schema, start=1):
    maxw = calc_width(key)
    for r in range(2, len(rows) + 2):
        v = ws.cell(row=r, column=c).value
        maxw = max(maxw, calc_width(v))
    ws.column_dimensions[ws.cell(row=1, column=c).column_letter].width = maxw

# Create META sheet and copy existing META columns (AS-IS, raw)
meta_ws = wb.create_sheet("Meta_data_sheet")
# Determine which META keys exist in schema; handle singular/plural macro define
existing_meta_keys = []
for k in META_COLS_CANONICAL:
    if k in schema:
        existing_meta_keys.append(k)
# If singular not found but plural present, include plural
if "Hidden_Macro_Define" not in existing_meta_keys and "Hidden_Macro_Defines" in schema:
    existing_meta_keys.append("Hidden_Macro_Defines")

# Write META headers
for c, key in enumerate(existing_meta_keys, start=1):
    meta_ws.cell(row=1, column=c, value=key)

# Write META rows
for r, obj in enumerate(rows, start=2):
    for c, key in enumerate(existing_meta_keys, start=1):
        meta_ws.cell(row=r, column=c, value=obj.get(key, ""))

# Very hide META sheet
meta_ws.sheet_state = 'veryHidden'

# Normalize MAIN sheet in-place: rename Data -> TestPlan
ws.title = "TestPlan"

# Rebuild TestPlan content strictly with MAIN_COLS_ORDER (remove META columns)
# Construct table from input rows for visible columns only
visible_headers = MAIN_COLS_ORDER[:]
# Ensure any missing columns are still present (will be blanks)
# Clear current TestPlan sheet
for row in ws[1:ws.max_row]:
    for cell in row:
        cell.value = None

# Write visible headers
for c, key in enumerate(visible_headers, start=1):
    ws.cell(row=1, column=c, value=key)

# Write visible rows (with numbering for the two specific columns)
for r, obj in enumerate(rows, start=2):
    for c, key in enumerate(visible_headers, start=1):
        val = obj.get(key, "")
        if key in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
            val = renumber_cell(val)
        ws.cell(row=r, column=c, value=val)

# Strict formatting for TestPlan
blue_fill = PatternFill("solid", fgColor="B7DEE8")  # light blue for readability
hdr_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
cell_left = Alignment(horizontal="left", vertical="top", wrap_text=True)
cell_center = Alignment(horizontal="center", vertical="top", wrap_text=True)
thin_border = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))

# Header formatting
for c in range(1, len(visible_headers) + 1):
    cell = ws.cell(row=1, column=c)
    cell.font = Font(bold=True)
    cell.alignment = hdr_align
    cell.fill = blue_fill

# Column-specific wrapping and alignment
wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
for c, key in enumerate(visible_headers, start=1):
    # Auto-fit widths again for visible columns
    maxw = calc_width(key)
    for r in range(2, len(rows) + 2):
        v = ws.cell(row=r, column=c).value
        maxw = max(maxw, calc_width(v))
        # Borders
        ws.cell(row=r, column=c).border = thin_border
        # Alignment per column
        if key in wrap_cols:
            ws.cell(row=r, column=c).alignment = cell_left
        elif key == "Index":
            ws.cell(row=r, column=c).alignment = cell_center
        else:
            # Default text left/top
            ws.cell(row=r, column=c).alignment = cell_left
    ws.column_dimensions[ws.cell(row=1, column=c).column_letter].width = maxw

# Apply borders to header too
for c in range(1, len(visible_headers) + 1):
    ws.cell(row=1, column=c).border = thin_border

# Approximate row heights post-wrap: set a minimum height, scale with line breaks
BASE_ROW_HEIGHT = 15
for r in range(2, len(rows) + 2):
    # Estimate max lines across wrapped columns
    max_lines = 1
    for c, key in enumerate(visible_headers, start=1):
        if key in wrap_cols:
            val = ws.cell(row=r, column=c).value or ""
            lines = str(val).count("\n") + 1
            if lines > max_lines:
                max_lines = lines
    ws.row_dimensions[r].height = BASE_ROW_HEIGHT * max(1, min(max_lines, 10))

# Data validation for Code Generation (Required / Not)
if "Code Generation (Required / Not)" in visible_headers:
    col_idx = visible_headers.index("Code Generation (Required / Not)") + 1
    start_row = 2
    end_row = len(rows) + 1
    rng = f"{ws.cell(row=1, column=col_idx).column_letter}{start_row}:{ws.cell(row=1, column=col_idx).column_letter}{end_row}"
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
    dv.error = "Select only: Required, Blank, Not Required"
    dv.errorTitle = "Invalid choice"
    ws.add_data_validation(dv)
    dv.add(rng)

# Safety: ensure no sheet named 'Data' remains
if "Data" in [s.title for s in wb.worksheets]:
    # Attempt deletion
    try:
        del wb["Data"]
    except Exception as e:
        raise SystemExit(f"Validation error: Unable to delete 'Data' sheet: {e}")

# Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist
allowed = {"TestPlan", "Meta_data_sheet"}
existing = set(ws.title for ws in wb.worksheets)
existing.add(meta_ws.title)
for s in list(wb.sheetnames):
    if s not in allowed:
        # Should not occur, but be defensive
        try:
            del wb[s]
        except Exception:
            pass

# Compute IST timestamp and filename
now_ist = datetime.now(TIMEZONE)
filename = f"{IP_NAME}_TestPlan_{now_ist:%Y%m%d}_{now_ist:%H%M%S}.xlsx"
rel_path = os.path.join(OUTPUT_DIR, filename)
abs_dir = OUTPUT_DIR
os.makedirs(abs_dir, exist_ok=True)

# Save workbook
wb.save(rel_path)

# Validate OOXML zip structure
if not validate_xlsx(rel_path):
    raise SystemExit("ERROR: XLSX validation failed (not a proper OOXML workbook)")

# Emit helper files for the workflow
with open("generated_path.txt", "w", encoding="utf-8") as f:
    f.write(rel_path)
with open("ist_timestamp.txt", "w", encoding="utf-8") as f:
    f.write(now_ist.strftime("%Y-%m-%d %H:%M:%S"))

print(f"Generated: {rel_path}")
