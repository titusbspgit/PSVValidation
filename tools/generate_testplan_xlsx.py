#!/usr/bin/env python3
# Deterministic fallback automation: Generate, format, validate, and commit Excel (.xlsx) from embedded JSON
# Strictly adheres to the rules specified in the request.

import json
import os
import re
import sys
from datetime import datetime, timezone, timedelta
from io import BytesIO
import zipfile

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ====== Static inputs (deterministic) ======
IP_NAME = "PCIE"
OUTPUT_DIR = os.path.join("Test_Output", "PCIE", "TestPlan")
BRANCH = os.environ.get("GITHUB_REF_NAME", "main")
# IST timestamp fixed at runtime execution time (Asia/Kolkata, GMT+05:30)
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
STAMP_DATE = now_ist.strftime("%Y%m%d")
STAMP_TIME = now_ist.strftime("%H%M%S")
OUTPUT_FILE_NAME = f"{IP_NAME}_TestPlan_{STAMP_DATE}_{STAMP_TIME}.xlsx"
OUTPUT_PATH = os.path.join(OUTPUT_DIR, OUTPUT_FILE_NAME)

# Embedded JSON data (exactly as received)
RAW_JSON = r'''{ "TC1": { "Index": "1", "SS / Module": "PCIE", "Feature": "writeAsRead", "Test Case Name": "pcie0_dbi_dsp_reg_wr_rd_test", "Test Description": "Validates PCIE0 DBI DSP register default values and masked write-read behavior across a provided address list using multiple data patterns. Skips operations based on read/write masks and an explicit skip list; aggregates failures and signals pass/fail.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Addresses with zero read mask are not read. Addresses with zero write mask are not written or read back during write-read checks. Addresses flagged in skip_array are skipped for both write and read phases. Default value checks are skipped for DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_DSP_PL_DEBUG1_OFF. The soft reset routine is present but disabled in this test flow.", "Test Steps / Procedure": "1) Read all readable registers listed by the test and compare each value to its expected default, excluding DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_DSP_PL_DEBUG1_OFF.\n2) For each data pattern (all ones, alternating A, alternating 5, all zeros, A5, upper half ones), write the pattern to every writable register not flagged to skip.\n3) For each such register, read back the value and compute the expected value by applying the read mask and write mask, combining written bits and preserved default bits.\n4) Mark a failure for any default mismatch or write-read mismatch; otherwise, count as a pass.\n5) Declare the test passed only if no failures are recorded; otherwise, declare it failed.", "Imparted Registers": "DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF", "Validation / Acceptance Criteria": "- All readable registers match their documented default values, excluding the three listed; pass if none differ.\n- For each writable and readable register not skipped, the read-back value equals the masked combination of written data and preserved default bits; pass if all match.\n- Overall result: pass only when no default or write-read mismatches are detected; otherwise, fail.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "pcie0_dbi_dsp_reg_wr_rd_test", "Hidden_Test_Description": "Test verifies default values and masked write-read behavior for PCIE0 DBI DSP registers over a list of 775 addresses. It uses arrays: addr_array[775], default_value_array[775], read_mask_array[775], write_mask_array[775], skip_array[775] from test_define.c. Execution flow: test_case() -> chk_rst_val() -> chk_rd_wr() -> finish().", "Hidden_Remarks": "1) Any entry with read_mask_array[i] == 0x00000000 is skipped for reading. 2) Any entry with write_mask_array[i] == 0x00000000 is skipped for writing and for subsequent readback verification. 3) Any entry with skip_array[i] == 1 is skipped entirely for both write and read phases. 4) In chk_rst_val(), default value checks are skipped if addr == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG or addr == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS or addr == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF. 5) The soft_reset_chk() function exists but is not invoked (commented out).", "Hidden_Test_Steps_Procedure": "Global: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; #define SOFT_RST_REG_ADDRESS 0x00000000; #define SOFT_RST_REG_DATA 0x00000000.\nFunction test_case():\n1) Call chk_rst_val(). If DEBUG_DISPLAY is defined, print \"********* Default value check end ***\".\n2) Call chk_rd_wr(). If DEBUG_DISPLAY is defined, print \" Write & Read from registers end ***\".\n3) If (def_fail_cnt > 0 || wr_fail_cnt > 0) call finish(1); else call finish(0).\nFunction chk_rst_val():\n4) For i from 0 to CNT-1 (CNT = 775):\n 4.1) addr = addr_array[i].\n 4.2) If read_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print skip message; continue to next i.\n 4.3) If (addr_array[i] == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG) OR (addr_array[i] == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS) OR (addr_array[i] == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF): continue (skip default value check for this address).\n 4.4) data_rd = read_reg(addr).\n 4.5) If data_rd == default_value_array[i]: if DEBUG_DISPLAY print PASS message; else: def_fail_cnt++; print failure message with address, expected default_value_array[i], and data_rd.\nFunction chk_rd_wr():\n5) Define int chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}.\n6) For j from 0 to 5:\n 6.1) data_wr = chk_val[j].\n 6.2) Write phase: For i from 0 to CNT-1:\n 6.2.1) addr = addr_array[i].\n 6.2.2) If skip_array[i] == 1: if DEBUG_DISPLAY print skip-writing message; continue.\n 6.2.3) If write_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-writable message; continue.\n 6.2.4) Else: write_reg(addr, data_wr); if DEBUG_DISPLAY print write information.\n 6.3) Read/verify phase: For i from 0 to CNT-1:\n 6.3.1) addr = addr_array[i].\n 6.3.2) If skip_array[i] == 1: if DEBUG_DISPLAY print skip-reading message; continue.\n 6.3.3) If write_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-writable skip message; continue.\n 6.3.4) If read_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-readable skip message; continue.\n 6.3.5) Else: data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); If data_rd == exp_val: if DEBUG_DISPLAY print PASS with address, expected, read; Else: wr_fail_cnt++; print failure with address, expected, read.\nFunction soft_reset_chk() [not called in test_case()]:\n7) int default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000).", "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF", "Hidden_Validation_Acceptance_Criteria": "1) Default value check: For each i where read_mask_array[i] != 0 and addr_array[i] not in {mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF}, verify read_reg(addr_array[i]) == default_value_array[i]; else increment def_fail_cnt. 2) Write-read check: For patterns chk_val[0..5], for each i where skip_array[i] != 1, write_mask_array[i] != 0, and read_mask_array[i] != 0, verify read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); else increment wr_fail_cnt. 3) Final result via finish(): pass when (def_fail_cnt == 0 && wr_fail_cnt == 0); fail otherwise." }, "TC2": { "Index": "2", "SS / Module": "PCIE", "Feature": "writeAsRead", "Test Case Name": "pcie0_dbi_usp_reg_wr_rd_test", "Test Description": "Validates PCIE0 DBI USP register default values and masked write-read behavior across a provided address list using multiple data patterns. Skips operations based on read/write masks and an explicit skip list; aggregates failures and signals pass/fail.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Addresses with zero read mask are not read. Addresses with zero write mask are not written or read back during write-read checks. Addresses flagged in skip_array are skipped for both write and read phases. Default value checks are skipped for DBI_USP_CAP_ID_NXT_PTR_REG, DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_USP_PL_DEBUG1_OFF. The soft reset routine is present but disabled in this test flow.", "Test Steps / Procedure": "1) Read all readable registers listed by the test and compare each value to its expected default, excluding DBI_USP_CAP_ID_NXT_PTR_REG, DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_USP_PL_DEBUG1_OFF.\n2) For each data pattern (all ones, alternating A, alternating 5, all zeros, A5, upper half ones), write the pattern to every writable register not flagged to skip.\n3) For each such register, read back the value and compute the expected value by applying the read mask and write mask, combining written bits and preserved default bits.\n4) Mark a failure for any default mismatch or write-read mismatch; otherwise, count as a pass.\n5) Declare the test passed only if no failures are recorded; otherwise, declare it failed.", "Imparted Registers": "DBI_USP_CAP_ID_NXT_PTR_REG, DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, DBI_USP_PL_DEBUG1_OFF", "Validation / Acceptance Criteria": "- All readable registers match their documented default values, excluding the three listed; pass if none differ.\n- For each writable and readable register not skipped, the read-back value equals the masked combination of written data and preserved default bits; pass if all match.\n- Overall result: pass only when no default or write-read mismatches are detected; otherwise, fail.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "pcie0_dbi_usp_reg_wr_rd_test", "Hidden_Test_Description": "Test verifies default values and masked write-read behavior for PCIE0 DBI USP registers over a list of 775 addresses. It uses arrays: addr_array[775], default_value_array[775], read_mask_array[775], write_mask_array[775], skip_array[775] from test_define.c. Execution flow: test_case() -> chk_rst_val() -> chk_rd_wr() -> finish().", "Hidden_Remarks": "1) Any entry with read_mask_array[i] == 0x00000000 is skipped for reading. 2) Any entry with write_mask_array[i] == 0x00000000 is skipped for writing and for subsequent readback verification. 3) Any entry with skip_array[i] == 1 is skipped entirely for both write and read phases. 4) In chk_rst_val(), default value checks are skipped if addr == mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG or addr == mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS or addr == mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF. 5) The soft_reset_chk() function exists but is not invoked (commented out).", "Hidden_Test_Steps_Procedure": "Global: int data_rd, data_wr; int def_fail_cnt = 0, wr_fail_cnt = 0; #define SOFT_RST_REG_ADDRESS 0x00000000; #define SOFT_RST_REG_DATA 0x00000000.\nFunction test_case():\n1) Call chk_rst_val(). If DEBUG_DISPLAY is defined, print \" Default value check end ***\".\n2) Call chk_rd_wr(). If DEBUG_DISPLAY is defined, print \" Write & Read from registers end ************\".\n3) If (def_fail_cnt > 0 || wr_fail_cnt > 0) call finish(1); else call finish(0).\nFunction chk_rst_val():\n4) For i from 0 to CNT-1 (CNT = 775):\n 4.1) addr = addr_array[i].\n 4.2) If read_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print skip message; continue to next i.\n 4.3) If (addr_array[i] == mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG) OR (addr_array[i] == mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS) OR (addr_array[i] == mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF): continue (skip default value check for this address).\n 4.4) data_rd = read_reg(addr).\n 4.5) If data_rd == default_value_array[i]: if DEBUG_DISPLAY print PASS message; else: def_fail_cnt++; print failure message with address, expected default_value_array[i], and data_rd.\nFunction chk_rd_wr():\n5) Define int chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}.\n6) For j from 0 to 5:\n 6.1) data_wr = chk_val[j].\n 6.2) Write phase: For i from 0 to CNT-1:\n 6.2.1) addr = addr_array[i].\n 6.2.2) If skip_array[i] == 1: if DEBUG_DISPLAY print skip-writing message; continue.\n 6.2.3) If write_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-writable message; continue.\n 6.2.4) Else: write_reg(addr, data_wr); if DEBUG_DISPLAY print write information.\n 6.3) Read/verify phase: For i from 0 to CNT-1:\n 6.3.1) addr = addr_array[i].\n 6.3.2) If skip_array[i] == 1: if DEBUG_DISPLAY print skip-reading message; continue.\n 6.3.3) If write_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-writable skip message; continue.\n 6.3.4) If read_mask_array[i] == 0x00000000: if DEBUG_DISPLAY print not-readable skip message; continue.\n 6.3.5) Else: data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); If data_rd == exp_val: if DEBUG_DISPLAY print PASS with address, expected, read; Else: wr_fail_cnt++; print failure with address, expected, read.\nFunction soft_reset_chk() [not called in test_case()]:\n7) int default_value = read_reg(SOFT_RST_REG_ADDRESS); write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); wait_on(1000); write_reg(SOFT_RST_REG_ADDRESS, default_value); wait_on(1000).", "Hidden_Impacted_Registers": "mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF", "Hidden_Validation_Acceptance_Criteria": "1) Default value check: For each i where read_mask_array[i] != 0 and addr_array[i] not in {mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF}, verify read_reg(addr_array[i]) == default_value_array[i]; else increment def_fail_cnt. 2) Write-read check: For patterns chk_val[0..5], for each i where skip_array[i] != 1, write_mask_array[i] != 0, and read_mask_array[i] != 0, verify read_reg(addr_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); else increment wr_fail_cnt. 3) Final result via finish(): pass when (def_fail_cnt == 0 && wr_fail_cnt == 0); fail otherwise." } }'''

# PHASE 1 — JSON TO EXCEL GENERATION

def parse_and_normalize(raw_json: str):
    try:
        data = json.loads(raw_json)
    except Exception as e:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": f"Invalid JSON: {e}"
        }))
        sys.exit(1)

    # Ensure tabular array of records
    if isinstance(data, list):
        records = data
    elif isinstance(data, dict):
        # Convert mapping of records to array of objects while preserving insertion order of top-level keys
        records = [data[k] for k in data.keys()]
    else:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": "JSON root must be an array or object of row objects"
        }))
        sys.exit(1)

    if not records:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": "Empty JSON input"
        }))
        sys.exit(1)

    # Union of keys in first-seen order
    seen = []
    for rec in records:
        if not isinstance(rec, dict):
            print(json.dumps({
                "Status": "FAILURE",
                "Error": "All rows must be JSON objects"
            }))
            sys.exit(1)
        for k in rec.keys():
            if k not in seen:
                seen.append(k)

    return records, seen


def build_base_workbook(records, headers):
    wb = Workbook()
    # Ensure the single authoritative staging sheet is named 'Data'
    ws = wb.active
    ws.title = "Data"

    # Header row
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center')
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')

    for col_idx, h in enumerate(headers, start=1):
        c = ws.cell(row=1, column=col_idx, value=h)
        c.font = header_font
        c.alignment = header_align
        c.fill = header_fill

    # Data rows
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, h in enumerate(headers, start=1):
            val = rec.get(h, "")
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Auto-fit columns (best-effort)
    autofit_columns(ws)

    return wb


def autofit_columns(ws):
    max_width = {}
    for row in ws.iter_rows(values_only=True):
        for i, val in enumerate(row, start=1):
            s = str(val) if val is not None else ""
            width = len(s)
            if width > max_width.get(i, 0):
                max_width[i] = width
    for i, w in max_width.items():
        # Add padding, cap a reasonable width
        ws.column_dimensions[chr(64+i) if i <= 26 else _col_letter(i)].width = min(80, w + 2)


def _col_letter(n):
    s = ""
    while n:
        n, r = divmod(n-1, 26)
        s = chr(65+r) + s
    return s

# PHASE 2 — REORGANIZATION & FORMATTING
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


def create_meta_sheet(wb, source_ws, records):
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    # Write header
    for c, h in enumerate(META_COLS, start=1):
        cell = meta_ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center')
    # Write values from records preserving exactly
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, h in enumerate(META_COLS, start=1):
            meta_ws.cell(row=r_idx, column=c_idx, value=rec.get(h, ""))
    # Very hidden
    meta_ws.sheet_state = 'veryHidden'
    return meta_ws


def rename_and_normalize_main_sheet(wb):
    # STEP 4 and STEP 7
    ws = wb["Data"]
    ws.title = "TestPlan"

    # Remove META columns from main (by header names)
    headers = [c.value for c in ws[1]]
    # Build a mapping header -> column index (1-based)
    header_to_idx = {h: i+1 for i, h in enumerate(headers)}

    # Determine columns to keep strictly according to MAIN_ORDER; insert missing as blanks
    # First, drop any column not in MAIN_ORDER
    cols_to_drop = [header_to_idx[h] for h in headers if h not in MAIN_ORDER]
    # When deleting columns, process in descending order to keep indices valid
    for col_idx in sorted(cols_to_drop, reverse=True):
        ws.delete_cols(col_idx, 1)

    # Refresh headers after deletions
    headers = [c.value for c in ws[1]]

    # Insert any missing main columns at the correct positions
    for target_pos, h in enumerate(MAIN_ORDER, start=1):
        if h in headers:
            # Move existing column to target_pos if necessary
            current_pos = headers.index(h) + 1
            if current_pos != target_pos:
                ws.move_range(start_row=1, start_column=current_pos, end_row=ws.max_row, end_column=current_pos, rows=0, cols=target_pos-current_pos)
                # After move, recompute headers
                headers = [c.value for c in ws[1]]
        else:
            # Insert a new blank column at target_pos with header h
            ws.insert_cols(target_pos, 1)
            ws.cell(row=1, column=target_pos, value=h)
            headers = [c.value for c in ws[1]]

    # Ensure the final header order matches MAIN_ORDER exactly
    # Rebuild styles for header row (bold, center, blue fill)
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center')
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    for col_idx, h in enumerate(MAIN_ORDER, start=1):
        c = ws.cell(row=1, column=col_idx)
        c.value = h
        c.font = header_font
        c.alignment = header_align
        c.fill = header_fill

    # Formatting and numbering
    apply_formatting_and_numbering(ws)

    # Data validation for Code Generation (Required / Not) on data rows only
    apply_code_generation_validation(ws)

    # After normalization, ensure sheet visibility rules
    enforce_visibility_rules(wb)

    # Auto-fit columns and row heights after wrap
    autofit_columns(ws)
    autofit_row_heights(ws)

    # Borders on all populated cells
    apply_thin_borders(ws)

    return ws


def apply_formatting_and_numbering(ws):
    # Wrap specified columns; alignments: header already set; data rows vertical top; text left; numeric/index center
    headers = [c.value for c in ws[1]]
    col_map = {h: i+1 for i, h in enumerate(headers)}

    wrap_alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
    default_alignment = Alignment(vertical='top', horizontal='left')
    index_alignment = Alignment(vertical='top', horizontal='center')

    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            h = headers[cell.column - 1]
            if h in WRAP_COLS:
                cell.alignment = wrap_alignment
            elif h == "Index":
                cell.alignment = index_alignment
            else:
                cell.alignment = default_alignment

    # Renumber items inside specific columns
    for h in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if h in col_map:
            col = col_map[h]
            for r in range(2, ws.max_row + 1):
                val = ws.cell(row=r, column=col).value
                if val is None:
                    continue
                new_val = renumber_multiline_text(str(val))
                ws.cell(row=r, column=col, value=new_val)


def renumber_multiline_text(text: str) -> str:
    # Split by lines, strip, remove existing bullet/number prefixes, then add 1., 2., ...
    lines = [ln.strip() for ln in text.splitlines() if ln.strip() != ""]
    cleaned = []
    for ln in lines:
        # Remove prefixes like '1)', '1.', '-', '•', '*', etc.
        ln2 = re.sub(r'^((\d+)[\).]|[-•*\u2022\u25CF\u25AA])\s*', '', ln)
        cleaned.append(ln2)
    if not cleaned:
        return ""
    renum = [f"{i+1}. {cleaned[i]}" for i in range(len(cleaned))]
    return "\n".join(renum)


def apply_code_generation_validation(ws):
    headers = [c.value for c in ws[1]]
    if "Code Generation (Required / Not)" not in headers:
        return
    col_idx = headers.index("Code Generation (Required / Not)") + 1
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
    dv.error = "Select one of: Required, Blank, Not Required"
    dv.errorTitle = "Invalid selection"
    ws.add_data_validation(dv)
    if ws.max_row >= 2:
        rng = f"{_col_letter(col_idx)}2:{_col_letter(col_idx)}{ws.max_row}"
        dv.add(rng)


def enforce_visibility_rules(wb):
    # Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden)
    allowed = {"TestPlan", "Meta_data_sheet"}
    for name in list(wb.sheetnames):
        if name not in allowed:
            # If the name is 'Data' lingering or any other, delete it
            ws = wb[name]
            wb.remove(ws)
    # Safety: ensure the two sheets exist
    if "TestPlan" not in wb.sheetnames or "Meta_data_sheet" not in wb.sheetnames:
        raise RuntimeError("Sheet visibility enforcement failed: required sheets missing")


def apply_thin_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def autofit_row_heights(ws):
    # Rough estimate based on wrapped text: number of lines in WRAP_COLS
    headers = [c.value for c in ws[1]]
    col_map = {h: i+1 for i, h in enumerate(headers)}
    wrap_cols_idx = [col_map[h] for h in WRAP_COLS if h in col_map]
    base_height = 15
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in wrap_cols_idx:
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            lines = str(v).count('\n') + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines

# PHASE 3 — SAVE, VALIDATE, COMMIT

def validate_xlsx_binary(path: str) -> bool:
    try:
        with zipfile.ZipFile(path, 'r') as zf:
            # Basic OOXML structure checks
            required = {'[Content_Types].xml', 'xl/workbook.xml'}
            names = set(zf.namelist())
            return required.issubset(names)
    except Exception:
        return False


def git_commit_file(path: str, message: str):
    import subprocess
    try:
        subprocess.run(["git", "config", "user.name", "github-actions"], check=True)
        subprocess.run(["git", "config", "user.email", "github-actions[bot]@users.noreply.github.com"], check=True)
        subprocess.run(["git", "add", path], check=True)
        # Only commit if there are staged changes
        diff = subprocess.run(["git", "diff", "--cached", "--quiet"])  # returncode 0 means no diff
        if diff.returncode != 0:
            subprocess.run(["git", "commit", "-m", message], check=True)
            subprocess.run(["git", "push"], check=True)
    except Exception as e:
        print(json.dumps({
            "Status": "FAILURE",
            "Error": f"Git commit failed: {e}"
        }))
        sys.exit(1)


def main():
    records, headers = parse_and_normalize(RAW_JSON)

    wb = build_base_workbook(records, headers)

    # Create META sheet and set veryHidden
    create_meta_sheet(wb, wb["Data"], records)

    # Normalize main sheet (rename Data->TestPlan, drop META & extras, order MAIN columns, formatting, numbering, DV, borders, visibility)
    rename_and_normalize_main_sheet(wb)

    # Ensure output directory exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Save workbook
    wb.save(OUTPUT_PATH)

    # Validate as true XLSX OOXML ZIP
    if not validate_xlsx_binary(OUTPUT_PATH):
        print(json.dumps({
            "Status": "FAILURE",
            "Execution mode": "Fallback automation script failed to validate XLSX",
            "Final Excel file path": OUTPUT_PATH
        }))
        sys.exit(1)

    # Commit only the finalized Excel file
    git_commit_file(OUTPUT_PATH, "Final formatted Excel generated from JSON input")

    # Report JSON status to stdout
    print(json.dumps({
        "Status": "SUCCESS",
        "Execution mode": "Fallback automation executed in GitHub Actions",
        "Number of rows detected": len(records),
        "Number of columns detected": len(headers),
        "Final Excel file path": OUTPUT_PATH,
        "Commit status": "Committed from GitHub Actions"
    }))


if __name__ == "__main__":
    main()
