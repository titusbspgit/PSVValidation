#!/usr/bin/env python3
"""
Agent 7 - Excel Generation Script
IP_NAME: MIPI_CSI
Filename: MIPI_CSI_TestPlan_20261105_193000.xlsx
Generated for: titusbspgit/PSVValidation
Branch: main
Output: Test_Output/MIPI/TestPlan/

Run this script locally to generate the genuine .xlsx workbook and push to GitHub.
Requirements: pip install openpyxl requests

Usage:
  python generate_mipi_csi_testplan_20261105_193000.py
"""

import json
import os
import sys
import base64
import datetime

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ============================================================
# TESTPLAN_JSON (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Read/Write Verification",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Test Description": "This testcase verifies the register read/write functionality of 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test executes in two phases. Phase 1 (Default Value Check) reads all registers after reset and verifies that each register holds its expected default/reset value. Phase 2 (Write and Read Check) writes a test data pattern to each writable register, then reads back and compares the read data against an expected value that accounts for the register's read mask, write mask, and default value. Registers INTERRUPT_RAW and FLUSH are explicitly skipped during the write/read phase. Read-only registers are also skipped. The test reports overall pass or fail based on accumulated failure counters from both phases.",
    "Test Steps / Procedure": "Step 1: Initialize the test environment and set failure counters to zero.\nStep 2: Execute the Default Value Check phase - read each of the 10 MIPI CSI2 RB registers and compare the read value against the expected default/reset value for that register. Increment the default-fail counter for any mismatch.\nStep 3: Execute the Write and Read Check phase:\n  Step 3a: Iterate over all registers in the register list.\n  Step 3b: For each register, check if it is flagged in the skip list (INTERRUPT_RAW and FLUSH are skipped). If flagged, skip to the next register.\n  Step 3c: Check if the register is writable. If not writable, skip writing to that register.\n  Step 3d: For writable and non-skipped registers, write the test data pattern to the register.\n  Step 3e: After completing all writes, iterate over all registers again for the read-back phase.\n  Step 3f: For each register, skip if flagged in the skip list, if not writable, or if not readable.\n  Step 3g: For eligible registers, read the register value and compute the expected value using the write mask, read mask, and default value. The expected value accounts for writable bits reflecting the written data and non-writable bits retaining their default values.\n  Step 3h: Compare the read value against the computed expected value. Increment the write-fail counter for any mismatch.\nStep 4: Evaluate overall test result - if either the default-fail counter or the write-fail counter is greater than zero, report test FAIL. Otherwise, report test PASS.",
    "Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
    "Validation / Acceptance Criteria": "1. Default Value Check: Each register must return its expected default/reset value when read after reset. The default-fail counter must remain zero.\n2. Write/Read Check: For each writable and non-skipped register, the read-back value after writing the test pattern must match the computed expected value. The expected value is determined by combining the written data (masked by both the read mask and write mask) with the default value (masked by the read mask and the inverse of the write mask). The write-fail counter must remain zero.\n3. Skip Verification: Registers INTERRUPT_RAW and FLUSH must be correctly skipped during the write/read phase. Read-only registers must be skipped for writing.\n4. Overall Pass Criteria: Both the default-fail counter and the write-fail counter must be zero for the test to pass. Any non-zero counter results in test failure.",
    "Remarks": "The soft reset check function exists in the test but is currently disabled. Only 10 of the 60 array slots are populated with valid register entries. Registers INTERRUPT_RAW and FLUSH are explicitly excluded from write/read verification via the skip configuration."
  }
]

# ============================================================
# META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
  {
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Meta Headers": "File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\n\nFile: test_define.c\n#include<mipi_csi2_rb_reg.h>",
    "Meta Macros": "File: test_define.c\n#define CNT 60\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n\nConditional compilation directive used in program.c:\n#ifdef DEBUG_DISPLAY\n(DEBUG_DISPLAY is not defined in the supplied source; used as a conditional guard around printf statements.)",
    "Meta Arrays": "File: test_define.c\n\nconst unsigned long int addr_array[60]={\n    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,\n    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,\n    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,\n    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,\n    MIZAR_MIPI_CSI2_RB_REG_MEM,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,\n    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,\n    MIZAR_MIPI_CSI2_RB_REG_FLUSH\n};\n\nconst int default_value_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LINE_INFO_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LANE_CLK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_MEM_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FLUSH_DEFAULT_VAL\n};\n\nconst int read_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_READ_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_READ_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_READ_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_READ_MASK,\n    MIPI_CSI2_RB_REG_MEM_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_READ_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_READ_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_READ_MASK\n};\n\nconst int write_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_MEM_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_WRITE_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_WRITE_MASK\n};\n\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};",
    "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
    "Meta Test Description": "This testcase (mipi_csi2_rb_reg_wr_rd_test) for the MIPI_CSI IP performs register read/write verification on 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test has two main phases: (1) chk_rst_val() checks that all registers hold their expected default/reset values after reset, and (2) chk_rd_wr() writes a test pattern (data_wr) to each writable register (skipping registers flagged in skip_array or with zero write_mask), then reads back and compares against an expected value computed using read_mask, write_mask, and default_value arrays. Registers at index 5 (INTERRUPT_RAW) and 9 (FLUSH) are skipped via skip_array. A soft_reset_chk() function exists but is commented out. Pass/fail is determined by def_fail_cnt and wr_fail_cnt counters, calling finish(0) on success or finish(1) on failure.",
    "Meta Test Steps / Procedure": "SOURCE_FILE: test_define.c and program.c with full source content as extracted by Agent 5A.",
    "Meta Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
    "Meta Validation / Acceptance Criteria": "data_rd == exp_val where exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); def_fail_cnt == 0 and wr_fail_cnt == 0 for overall pass; finish(0) on success, finish(1) on failure."
  }
]

# ============================================================
# Configuration
# ============================================================
IP_NAME = "MIPI_CSI"
FILENAME = "MIPI_CSI_TestPlan_20261105_193000.xlsx"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
GITHUB_OWNER = "titusbspgit"
GITHUB_REPO = "PSVValidation"
GITHUB_BRANCH = "main"

# ============================================================
# TestPlan Sheet Columns
# ============================================================
TESTPLAN_COLUMNS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Test Description",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Remarks",
    "Code Generation (Required / Not)"
]

# ============================================================
# MetaData Sheet Columns
# ============================================================
METADATA_COLUMNS = [
    "Test Case Name",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
    "Meta Types",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria"
]

# ============================================================
# Formatting
# ============================================================
HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
HEADER_ALIGNMENT = Alignment(horizontal="center", vertical="center", wrap_text=True)
CELL_ALIGNMENT = Alignment(vertical="top", wrap_text=True)

def create_workbook():
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Write headers
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGNMENT

    # Write data rows
    for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
        ws_tp.cell(row=row_idx, column=1, value=tc.get("Index", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=2, value=tc.get("SS / Module", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=3, value=tc.get("Feature", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=4, value=tc.get("Test Case Name", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=5, value=tc.get("Speed", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=6, value=tc.get("Mode", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=7, value=tc.get("Memory Start Offset", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=8, value=tc.get("Memory End Offset", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=9, value=tc.get("Test Description", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=10, value=tc.get("Test Steps / Procedure", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=11, value=tc.get("Impacted Registers", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=12, value=tc.get("Validation / Acceptance Criteria", "")).alignment = CELL_ALIGNMENT
        ws_tp.cell(row=row_idx, column=13, value=tc.get("Remarks", "")).alignment = CELL_ALIGNMENT
        # Column 14: Code Generation - leave blank (user selects via dropdown)

    # Data Validation for Code Generation column
    dv = DataValidation(
        type="list",
        formula1='"Required,Not Required"',
        allow_blank=True,
        showDropDown=False
    )
    dv.error = "Please select Required or Not Required"
    dv.errorTitle = "Invalid Entry"
    dv.prompt = "Select Code Generation status"
    dv.promptTitle = "Code Generation"
    last_data_row = 1 + len(TESTPLAN_JSON)
    code_gen_col = get_column_letter(14)
    dv.add(f"{code_gen_col}2:{code_gen_col}{last_data_row}")
    ws_tp.add_data_validation(dv)

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns with max width
    MAX_WIDTH = 60
    for col_idx in range(1, len(TESTPLAN_COLUMNS) + 1):
        max_len = len(str(ws_tp.cell(row=1, column=col_idx).value or ""))
        for row_idx in range(2, last_data_row + 1):
            val = str(ws_tp.cell(row=row_idx, column=col_idx).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        adjusted = min(max_len + 2, MAX_WIDTH)
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet("MetaData")

    # Write headers
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGNMENT

    # Write data rows
    for row_idx, meta in enumerate(META_JSON, 2):
        ws_md.cell(row=row_idx, column=1, value=meta.get("Test Case Name", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=2, value=meta.get("Meta Headers", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=3, value=meta.get("Meta Macros", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=4, value=meta.get("Meta Arrays", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=5, value=meta.get("Meta Types", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=6, value=meta.get("Meta Test Description", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=7, value=meta.get("Meta Test Steps / Procedure", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=8, value=meta.get("Meta Impacted Registers", "")).alignment = CELL_ALIGNMENT
        ws_md.cell(row=row_idx, column=9, value=meta.get("Meta Validation / Acceptance Criteria", "")).alignment = CELL_ALIGNMENT

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Auto-size columns
    md_last_row = 1 + len(META_JSON)
    for col_idx in range(1, len(METADATA_COLUMNS) + 1):
        max_len = len(str(ws_md.cell(row=1, column=col_idx).value or ""))
        for row_idx in range(2, md_last_row + 1):
            val = str(ws_md.cell(row=row_idx, column=col_idx).value or "")
            lines = val.split("\n")
            for line in lines:
                if len(line) > max_len:
                    max_len = len(line)
        adjusted = min(max_len + 2, MAX_WIDTH)
        ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    return wb


def validate_workbook(filepath):
    from openpyxl import load_workbook
    wb = load_workbook(filepath)
    checks = {
        "file_exists": os.path.exists(filepath),
        "file_size_gt_0": os.path.getsize(filepath) > 0,
        "reopens_ok": True,
        "has_testplan": "TestPlan" in wb.sheetnames,
        "has_metadata": "MetaData" in wb.sheetnames,
        "testplan_rows": wb["TestPlan"].max_row - 1,
        "metadata_rows": wb["MetaData"].max_row - 1,
    }
    wb.close()
    return checks


def push_to_github(filepath, token):
    import requests
    with open(filepath, "rb") as f:
        content_bytes = f.read()
    content_b64 = base64.b64encode(content_bytes).decode("utf-8")

    api_url = f"https://api.github.com/repos/{GITHUB_OWNER}/{GITHUB_REPO}/contents/{OUTPUT_DIR}/{FILENAME}"

    # Check if file exists
    headers = {
        "Authorization": f"token {token}",
        "Accept": "application/vnd.github.v3+json"
    }
    resp = requests.get(api_url, headers=headers, params={"ref": GITHUB_BRANCH})
    sha = None
    if resp.status_code == 200:
        sha = resp.json().get("sha")

    payload = {
        "message": "Generated TestPlan Excel Workbook",
        "content": content_b64,
        "branch": GITHUB_BRANCH
    }
    if sha:
        payload["sha"] = sha

    resp = requests.put(api_url, headers=headers, json=payload)
    if resp.status_code in (200, 201):
        data = resp.json()
        return {
            "github_url": data["content"]["html_url"],
            "commit_sha": data["commit"]["sha"]
        }
    else:
        print(f"GitHub upload failed: {resp.status_code} {resp.text}")
        return None


def main():
    print(f"Generating workbook: {FILENAME}")
    wb = create_workbook()
    wb.save(FILENAME)
    print(f"Saved: {FILENAME}")

    checks = validate_workbook(FILENAME)
    print(f"Validation: {json.dumps(checks, indent=2)}")

    all_pass = all([
        checks["file_exists"],
        checks["file_size_gt_0"],
        checks["reopens_ok"],
        checks["has_testplan"],
        checks["has_metadata"],
        checks["testplan_rows"] == len(TESTPLAN_JSON),
        checks["metadata_rows"] == len(META_JSON),
    ])

    if not all_pass:
        result = {
            "status": "FAILURE",
            "output_file_path": "",
            "github_url": "",
            "filename": FILENAME,
            "rows_testplan": checks.get("testplan_rows", 0),
            "rows_metadata": checks.get("metadata_rows", 0),
            "commit_sha": "",
            "validation": "FAILED"
        }
        print(json.dumps(result, indent=2))
        return

    token = os.environ.get("GITHUB_TOKEN", "")
    if not token:
        print("No GITHUB_TOKEN set. Workbook generated locally but not pushed.")
        result = {
            "status": "IN_PROGRESS",
            "output_file_path": os.path.abspath(FILENAME),
            "github_url": "",
            "filename": FILENAME,
            "rows_testplan": checks["testplan_rows"],
            "rows_metadata": checks["metadata_rows"],
            "commit_sha": "",
            "validation": "PASSED"
        }
        print(json.dumps(result, indent=2))
        return

    gh = push_to_github(FILENAME, token)
    if gh:
        result = {
            "status": "SUCCESS",
            "output_file_path": f"{OUTPUT_DIR}/{FILENAME}",
            "github_url": gh["github_url"],
            "filename": FILENAME,
            "rows_testplan": checks["testplan_rows"],
            "rows_metadata": checks["metadata_rows"],
            "commit_sha": gh["commit_sha"],
            "validation": "PASSED"
        }
    else:
        result = {
            "status": "FAILURE",
            "output_file_path": os.path.abspath(FILENAME),
            "github_url": "",
            "filename": FILENAME,
            "rows_testplan": checks["testplan_rows"],
            "rows_metadata": checks["metadata_rows"],
            "commit_sha": "",
            "validation": "PASSED"
        }
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
