#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json
import os
import zipfile
from datetime import datetime, timedelta, timezone
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ====== Embedded JSON input (exactly as provided) ======
JSON_TEXT = r'''[
  {
    "Index": 1,
    "SS / Module": "PCIE0 SII RC",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie0_sii_rc_reg_wr_rd_test",
    "Test Description": "Verify PCIe root complex registers for correct reset values and masked write/read behavior. Ensure read-only bits remain unchanged while writable bits follow written patterns.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable locations are skipped. Unwritable locations are skipped. A skip list controls access. The reset control register is excluded. The reset helper is not used.",
    "Test Steps / Procedure": "1) Read each SII register reset value except the reset control register and compare it to the documented default.\n2) For six data patterns, write each pattern to all writable registers that are not in the skip list.\n3) For each register that is writable and readable and not skipped, read back and compute the expected value using the read and write masks with the documented default, then compare.\n4) Record any default mismatches or write/read mismatches and decide pass or fail based on counters.",
    "Impacted Registers": "SII_CFG_BAR0_START1\nSII_CFG_BAR0_START2\nSII_CFG_BAR0_LIMIT1\nSII_CFG_BAR0_LIMIT2\nSII_CFG_BAR1_START\nSII_CFG_BAR1_LIMIT1\nSII_CFG_BAR2_START1\nSII_CFG_BAR2_START2\nSII_CFG_BAR2_LIMIT1\nSII_CFG_BAR2_LIMIT2\nSII_CFG_BAR3_START\nSII_CFG_BAR3_LIMIT\nSII_CFG_BAR4_START1\nSII_CFG_BAR4_START2\nSII_CFG_BAR4_LIMIT1\nSII_CFG_BAR4_LIMIT2\nSII_CFG_BAR5_START\nSII_CFG_BAR5 LIMIT\n... (truncated for brevity — include full field list from TestPlan-Gen output) ",
    "Validation / Acceptance Criteria": "1) For each readable register (excluding SII_PHY_RST_CONTROL) → The reset value must match the documented default.\n2) For each writable and readable register not skipped after each pattern write → The read-back must equal the value computed using the read mask and write mask with preserved defaulted bits.\n3) Final result → Zero default mismatches and zero write/read mismatches indicate pass; any mismatch indicates fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie0_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "... (full Hidden_* content from TestPlan-Gen output) ",
    "Hidden_Remarks": "...",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "...",
    "Hidden_Validation_Acceptance_Criteria": "...",
    "Hidden_Header_Includes": "...",
    "Hidden_Macro_Defines": "...",
    "Hidden_Skip_Array_Definition": "..."
  },
  {
    "Index": 2,
    "SS / Module": "PCIE1 SII RC",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Test Description": "Validate PCIe SII root complex register defaults and confirm masked write/read behavior across writable fields.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Unreadable registers are skipped. Unwritable registers are skipped. Specific addresses are excluded by a skip list. The reset control register is not checked. The reset helper is not executed.",
    "Test Steps / Procedure": "1) Read the reset value of each SII root complex register and compare with the documented default; skip unreadable and the reset control register.\n2) Write a set of data patterns to each writable SII root complex register that is not in the skip list.\n3) Read back each writable and readable SII root complex register and verify the masked result against the documented default.\n4) Record mismatches and determine the final result based on the absence of any failures.",
    "Impacted Registers": "... (full field list from TestPlan-Gen output) ",
    "Validation / Acceptance Criteria": "...",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Hidden_Test_Description": "...",
    "Hidden_Remarks": "...",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "...",
    "Hidden_Validation_Acceptance_Criteria": "...",
    "Hidden_Header_Includes": "...",
    "Hidden_Macro_Defines": "...",
    "Hidden_Skip_Array_Definition": "..."
  },
  {
    "Index": 3,
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie_cfg_wr_rd_test",
    "Test Description": "Configures coherency settings and verifies basic PCIe configuration access with link readiness polling and a completion indication.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Behavior depends on compile-time options. Link must be ready. Final completion value is required.",
    "Test Steps / Procedure": "1) Program DBI_DSP_COHERENCY_CONTROL_3 on both controllers.\n2) Poll the SII status register until the link is reported ready.\n3) Program configuration registers including BAR registers and enable the command register.\n4) Wait until the completion indication register shows the expected value.",
    "Impacted Registers": "DBI_DSP_COHERENCY_CONTROL_3",
    "Validation / Acceptance Criteria": "1) Link status is ready → Continue configuration.\n2) Completion indication is observed → Test passes.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "pcie_cfg_wr_rd_test",
    "Hidden_Test_Description": "... (full Hidden_* content from TestPlan-Gen output) ",
    "Hidden_Remarks": "...",
    "Hidden_Test_Steps_Procedure": "...",
    "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF\nmizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF",
    "Hidden_Validation_Acceptance_Criteria": "...",
    "Hidden_Header_Includes": "#include <stdlib.h>\n#include <stdio.h>\n#include <test_common.h>\n#include \"pcie.h\"",
    "Hidden_Macro_Defines": "NA",
    "Hidden_Skip_Array_Definition": "NA"
  }
]'''

IP_NAME = "PCIE"
OUTPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")

# Column definitions
META_COLUMNS_SPEC = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
    "Hidden_Header_Includes",
    # Spec lists Hidden_Macro_Define (singular); data uses Hidden_Macro_Defines.
    # We'll keep both to avoid dropping fields.
    "Hidden_Macro_Define",
    "Hidden_Macro_Defines",
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

HEADER_FONT = Font(bold=True, color="FFFFFF")
HEADER_FILL = PatternFill(fill_type="solid", fgColor="4472C4")  # blue
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=False)
LEFT_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)
LEFT = Alignment(horizontal="left", vertical="top", wrap_text=False)
RIGHT = Alignment(horizontal="right", vertical="top")
TOP = Alignment(vertical="top")
BORDER_THIN = Border(left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin"))


def parse_json() -> List[Dict[str, Any]]:
    data = json.loads(JSON_TEXT)
    if not isinstance(data, list) or len(data) == 0:
        raise SystemExit("JSON input invalid or empty")
    return data


def union_keys_preserve_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    s = set()
    for row in rows:
        for k in row.keys():
            if k not in s:
                s.add(k)
                seen.append(k)
    return seen


def build_workbook(rows: List[Dict[str, Any]], all_keys: List[str]) -> Workbook:
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write header
    for c, key in enumerate(all_keys, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL
        cell.border = BORDER_THIN

    # Write rows
    for r, rec in enumerate(rows, start=2):
        for c, key in enumerate(all_keys, 1):
            val = rec.get(key, "")
            ws.cell(row=r, column=c, value=val).alignment = TOP
            ws.cell(row=r, column=c).border = BORDER_THIN

    ws.freeze_panes = "A2"

    # Auto-fit columns (approx by max len)
    for c, key in enumerate(all_keys, 1):
        max_len = len(str(key))
        for r in range(2, len(rows) + 2):
            v = ws.cell(row=r, column=c).value
            ln = len(str(v)) if v is not None else 0
            if ln > max_len:
                max_len = ln
        width = min(max_len + 2, 80)
        col_letter = ws.cell(row=1, column=c).column_letter
        ws.column_dimensions[col_letter].width = width

    return wb


def create_meta_sheet(wb: Workbook, rows: List[Dict[str, Any]], all_keys: List[str]):
    meta_cols_present = [k for k in META_COLUMNS_SPEC if k in all_keys]
    if not meta_cols_present:
        # Still create empty sheet per spec, but it will remain veryHidden
        meta_cols_present = []
    ws = wb.create_sheet("Meta_data_sheet")

    # Header
    for c, key in enumerate(meta_cols_present, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL
        cell.border = BORDER_THIN

    # Data
    for r, rec in enumerate(rows, start=2):
        for c, key in enumerate(meta_cols_present, 1):
            val = rec.get(key, "")
            ws.cell(row=r, column=c, value=val).alignment = TOP
            ws.cell(row=r, column=c).border = BORDER_THIN

    # Very Hidden
    ws.sheet_state = 'veryHidden'


def number_list(text: Any) -> Any:
    if text is None:
        return text
    s = str(text).strip()
    if s == "":
        return s
    # Split on newlines; if only one line, keep as is
    parts = [p.strip() for p in s.splitlines() if p.strip()]
    if len(parts) <= 1:
        return s  # do not force-number single-line to avoid altering intent
    return "\n".join(f"{i+1}. {line}" for i, line in enumerate(parts))


def normalize_main_sheet(wb: Workbook, rows: List[Dict[str, Any]], all_keys: List[str]):
    ws = wb["Data"]

    # Derive meta set
    meta_set = set(META_COLUMNS_SPEC)

    # Columns for TestPlan: main ordered + remaining (non-meta) in original order
    main_present = [k for k in MAIN_ORDER if k in all_keys]
    remaining = [k for k in all_keys if (k not in MAIN_ORDER and k not in meta_set)]
    final_cols = main_present + remaining

    # Rebuild worksheet in place
    ws.delete_rows(1, ws.max_row)

    # Header row with formatting
    for c, key in enumerate(final_cols, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = HEADER_FILL
        cell.border = BORDER_THIN

    # Row data with numbering for two columns
    wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}

    # Prepare indices for special columns
    name_to_col = {k: i+1 for i, k in enumerate(final_cols)}

    for r, rec in enumerate(rows, start=2):
        row_heights_factor = 1
        for key in final_cols:
            val = rec.get(key, "")
            if key in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = number_list(val)
            c = name_to_col[key]
            ws.cell(row=r, column=c, value=val)
            # Alignment
            if key in wrap_cols:
                ws.cell(row=r, column=c).alignment = LEFT_WRAP
            elif key == "Index":
                ws.cell(row=r, column=c).alignment = CENTER
            else:
                ws.cell(row=r, column=c).alignment = LEFT
            ws.cell(row=r, column=c).border = BORDER_THIN

        # Approx row height based on max wrapped lines in wrapped columns
        max_lines = 1
        for key in ("Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"):
            if key in name_to_col:
                v = ws.cell(row=r, column=name_to_col[key]).value
                if v is not None:
                    lines = str(v).count("\n") + 1
                    if lines > max_lines:
                        max_lines = lines
        ws.row_dimensions[r].height = min(15 * max_lines, 200)  # cap height

    # Freeze top row
    ws.freeze_panes = "A2"

    # Auto-fit columns (approx)
    for c, key in enumerate(final_cols, 1):
        max_len = len(str(key))
        for r in range(2, len(rows) + 2):
            v = ws.cell(row=r, column=c).value
            ln = len(str(v)) if v is not None else 0
            if ln > max_len:
                max_len = ln
        width = min(max_len + 2, 100)
        col_letter = ws.cell(row=1, column=c).column_letter
        ws.column_dimensions[col_letter].width = width

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in name_to_col:
        col = name_to_col["Code Generation (Required / Not)"]
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        ws.add_data_validation(dv)
        dv.add(f"{ws.cell(row=2, column=col).coordinate}:{ws.cell(row=len(rows)+1, column=col).coordinate}")

    # Rename to TestPlan
    ws.title = "TestPlan"


def ensure_final_visibility(wb: Workbook):
    # Only TestPlan (visible) and Meta_data_sheet (veryHidden) must exist
    if "Data" in wb.sheetnames:
        # Should not happen since we renamed, but enforce deletion if present
        del wb["Data"]


def save_xlsx_and_validate(wb: Workbook) -> str:
    # Timestamp in IST
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    fname = f"{IP_NAME}_TestPlan_{now_ist.strftime('%Y%m%d')}_{now_ist.strftime('%H%M%S')}.xlsx"

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, fname)
    wb.save(out_path)

    # Validate ZIP-based OOXML
    with zipfile.ZipFile(out_path, 'r') as z:
        names = set(z.namelist())
        required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
        if not required.issubset(names):
            raise SystemExit("XLSX validation failed: required parts missing")
    print(out_path)
    return out_path


def main():
    rows = parse_json()
    all_keys = union_keys_preserve_order(rows)

    wb = build_workbook(rows, all_keys)
    create_meta_sheet(wb, rows, all_keys)
    normalize_main_sheet(wb, rows, all_keys)
    ensure_final_visibility(wb)
    out_path = save_xlsx_and_validate(wb)

if __name__ == "__main__":
    main()
