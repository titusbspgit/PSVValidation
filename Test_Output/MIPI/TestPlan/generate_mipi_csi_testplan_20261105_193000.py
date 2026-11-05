#!/usr/bin/env python3
"""
Agent 7 - Excel Workbook Generator & GitHub Publisher
IP_NAME: MIPI_CSI
Output: MIPI_CSI_TestPlan_20261105_193000.xlsx

This script generates a genuine .xlsx workbook with two sheets (TestPlan, MetaData),
validates it, and pushes it to GitHub.

Requirements: pip install openpyxl PyGithub
Usage: GITHUB_TOKEN=<token> python generate_mipi_csi_testplan_20261105_193000.py
"""

import json, os, sys, base64, io

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl required. Install: pip install openpyxl")
    sys.exit(1)

# ============================================================
# TESTPLAN_JSON (Agent 6B aggregated output - exact values)
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
# META_JSON (Agent 6A aggregated output - exact values)
# ============================================================
META_JSON = [
  {
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Meta Headers": "File: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"\n\nFile: test_define.c\n#include<mipi_csi2_rb_reg.h>",
    "Meta Macros": "File: test_define.c\n#define CNT 60\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS\t0x00000000\n#define SOFT_RST_REG_DATA\t0x00000000\n\nConditional compilation directive used in program.c:\n#ifdef DEBUG_DISPLAY\n(DEBUG_DISPLAY is not defined in the supplied source; used as a conditional guard around printf statements.)",
    "Meta Arrays": "File: test_define.c\n\nconst unsigned long int addr_array[60]={\n    MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,\n    MIZAR_MIPI_CSI2_RB_REG_LINE_INFO,\n    MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL,\n    MIZAR_MIPI_CSI2_RB_REG_LANE_CLK,\n    MIZAR_MIPI_CSI2_RB_REG_MEM,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK,\n    MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE,\n    MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,\n    MIZAR_MIPI_CSI2_RB_REG_FLUSH\n};\n\nconst int default_value_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LINE_INFO_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_LANE_CLK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_MEM_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_DEFAULT_VAL,\n    MIPI_CSI2_RB_REG_FLUSH_DEFAULT_VAL\n};\n\nconst int read_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_READ_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_READ_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_READ_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_READ_MASK,\n    MIPI_CSI2_RB_REG_MEM_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_READ_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_READ_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_READ_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_READ_MASK\n};\n\nconst int write_mask_array[60]={\n    MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LINE_INFO_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL_WRITE_MASK,\n    MIPI_CSI2_RB_REG_LANE_CLK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_MEM_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_RAW_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_MASK_WRITE_MASK,\n    MIPI_CSI2_RB_REG_INTERRUPT_ENABLE_WRITE_MASK,\n    MIPI_CSI2_RB_REG_CONTROL_DATA_WRITE_MASK,\n    MIPI_CSI2_RB_REG_FLUSH_WRITE_MASK\n};\n\nconst int skip_array[60]={0,0,0,0,0,1,0,0,0,1};",
    "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
    "Meta Function Inventory": "File: program.c\n\n--- Function Definition: test_case ---\nSignature: int test_case()\nReturn type: int\nParameters: none\nBody supplied: yes (complete)\nCalls in source order:\n  1. chk_rst_val()\n  2. printf(\"********* Default value check end ************\\n\") [inside #ifdef DEBUG_DISPLAY]\n  3. chk_rd_wr()\n  4. printf(\"********* Write & Read from registers end ************\\n\") [inside #ifdef DEBUG_DISPLAY]\n  5. finish(1) [inside if(def_fail_cnt > 0 || wr_fail_cnt > 0)]\n  6. finish(0) [inside else branch]\nCommented-out calls: soft_reset_chk(), printf(\"********* Soft reset end ************\\n\"), chk_rst_val()\n\n--- Function Definition: chk_rd_wr ---\nSignature: void chk_rd_wr() [inferred from supplied excerpt]\nReturn type: void (inferred)\nParameters: none\nBody supplied: partial\n\n--- Function: chk_rst_val ---\nCalled from: test_case()\nBody supplied: no.\n\n--- Function: soft_reset_chk ---\nReferenced in: test_case() (commented out)\nBody supplied: no.\n\n--- Function: write_reg ---\nCalled as: write_reg(addr, data_wr)\nBody supplied: no.\n\n--- Function: read_reg ---\nCalled as: read_reg(addr)\nBody supplied: no.\n\n--- Function: finish ---\nCalled as: finish(1), finish(0)\nBody supplied: no.",
    "Meta Test Description": "This testcase (mipi_csi2_rb_reg_wr_rd_test) for the MIPI_CSI IP performs register read/write verification on 10 MIPI CSI2 RB registers: VIRTUAL_CHANNEL, LINE_INFO, FIFO_THRESHOLD_VAL, LANE_CLK, MEM, INTERRUPT_RAW, INTERRUPT_MASK, INTERRUPT_ENABLE, CONTROL_DATA, and FLUSH. The test has two main phases: (1) chk_rst_val() checks that all registers hold their expected default/reset values after reset, and (2) chk_rd_wr() writes a test pattern (data_wr) to each writable register (skipping registers flagged in skip_array or with zero write_mask), then reads back and compares against an expected value computed using read_mask, write_mask, and default_value arrays. Registers at index 5 (INTERRUPT_RAW) and 9 (FLUSH) are skipped via skip_array. A soft_reset_chk() function exists but is commented out. Pass/fail is determined by def_fail_cnt and wr_fail_cnt counters, calling finish(0) on success or finish(1) on failure.",
    "Meta Test Steps / Procedure": "SOURCE_FILE: test_define.c and program.c with full source content as extracted by Agent 5A.",
    "Meta Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush",
    "Meta Validation / Acceptance Criteria": "data_rd == exp_val where exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); def_fail_cnt == 0 and wr_fail_cnt == 0 for overall pass; finish(0) on success, finish(1) on failure."
  }
]

# ============================================================
# Column Definitions
# ============================================================
TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

METADATA_COLUMNS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Function Inventory", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria"
]

TESTPLAN_FIELD_MAP = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks"
]

# ============================================================
# Formatting Constants
# ============================================================
HDR_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HDR_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
HDR_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
CELL_ALIGN = Alignment(vertical="top", wrap_text=True)
MAX_COL_WIDTH = 65

FILENAME = "MIPI_CSI_TestPlan_20261105_193000.xlsx"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
REPO_OWNER = "titusbspgit"
REPO_NAME = "PSVValidation"
BRANCH = "main"


def auto_width(ws, num_cols, last_row):
    for c in range(1, num_cols + 1):
        mx = len(str(ws.cell(1, c).value or ""))
        for r in range(2, last_row + 1):
            v = str(ws.cell(r, c).value or "")
            for ln in v.split("\n"):
                mx = max(mx, len(ln))
        ws.column_dimensions[get_column_letter(c)].width = min(mx + 2, MAX_COL_WIDTH)


def build_workbook():
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws = wb.active
    ws.title = "TestPlan"
    for ci, cn in enumerate(TESTPLAN_COLUMNS, 1):
        c = ws.cell(1, ci, cn)
        c.font, c.fill, c.alignment = HDR_FONT, HDR_FILL, HDR_ALIGN

    for ri, tc in enumerate(TESTPLAN_JSON, 2):
        for ci, fld in enumerate(TESTPLAN_FIELD_MAP, 1):
            c = ws.cell(ri, ci, tc.get(fld, ""))
            c.alignment = CELL_ALIGN
        # Col 14 (Code Generation) left blank

    last = 1 + len(TESTPLAN_JSON)
    dv = DataValidation(type="list", formula1='"Required,Not Required"',
                        allow_blank=True, showDropDown=False)
    dv.error = "Select Required or Not Required"
    dv.errorTitle = "Invalid"
    col_letter = get_column_letter(14)
    dv.add(f"{col_letter}2:{col_letter}{last}")
    ws.add_data_validation(dv)
    ws.freeze_panes = "A2"
    auto_width(ws, len(TESTPLAN_COLUMNS), last)

    # ---- MetaData Sheet ----
    md = wb.create_sheet("MetaData")
    for ci, cn in enumerate(METADATA_COLUMNS, 1):
        c = md.cell(1, ci, cn)
        c.font, c.fill, c.alignment = HDR_FONT, HDR_FILL, HDR_ALIGN

    for ri, m in enumerate(META_JSON, 2):
        for ci, fld in enumerate(METADATA_COLUMNS, 1):
            c = md.cell(ri, ci, m.get(fld, ""))
            c.alignment = CELL_ALIGN

    md_last = 1 + len(META_JSON)
    md.freeze_panes = "A2"
    auto_width(md, len(METADATA_COLUMNS), md_last)
    md.sheet_state = "veryHidden"

    return wb


def validate(path):
    ok = True
    checks = {}
    checks["file_exists"] = os.path.exists(path)
    checks["file_size"] = os.path.getsize(path) if checks["file_exists"] else 0
    checks["file_size_gt_0"] = checks["file_size"] > 0
    try:
        wb2 = load_workbook(path)
        checks["reopens"] = True
        checks["has_TestPlan"] = "TestPlan" in wb2.sheetnames
        checks["has_MetaData"] = "MetaData" in wb2.sheetnames
        checks["tp_rows"] = wb2["TestPlan"].max_row - 1
        checks["md_rows"] = wb2["MetaData"].max_row - 1
        checks["tp_match"] = checks["tp_rows"] == len(TESTPLAN_JSON)
        checks["md_match"] = checks["md_rows"] == len(META_JSON)
        # Check Code Generation column header
        checks["code_gen_col"] = wb2["TestPlan"].cell(1, 14).value == "Code Generation (Required / Not)"
        # Check Code Generation cells are blank
        all_blank = all(wb2["TestPlan"].cell(r, 14).value is None for r in range(2, 2 + len(TESTPLAN_JSON)))
        checks["code_gen_blank"] = all_blank
        wb2.close()
    except Exception as e:
        checks["reopens"] = False
        checks["error"] = str(e)
        ok = False
    ok = all([
        checks.get("file_exists"), checks.get("file_size_gt_0"),
        checks.get("reopens"), checks.get("has_TestPlan"),
        checks.get("has_MetaData"), checks.get("tp_match"),
        checks.get("md_match"), checks.get("code_gen_col"),
        checks.get("code_gen_blank")
    ])
    return ok, checks


def push_github(path, token):
    import requests
    with open(path, "rb") as f:
        raw = f.read()
    b64 = base64.b64encode(raw).decode()
    url = f"https://api.github.com/repos/{REPO_OWNER}/{REPO_NAME}/contents/{OUTPUT_DIR}/{FILENAME}"
    hdrs = {"Authorization": f"token {token}", "Accept": "application/vnd.github.v3+json"}
    r = requests.get(url, headers=hdrs, params={"ref": BRANCH})
    sha = r.json().get("sha") if r.status_code == 200 else None
    body = {"message": "Generated TestPlan Excel Workbook", "content": b64, "branch": BRANCH}
    if sha:
        body["sha"] = sha
    r = requests.put(url, headers=hdrs, json=body)
    if r.status_code in (200, 201):
        d = r.json()
        return d["content"]["html_url"], d["commit"]["sha"]
    print(f"Push failed: {r.status_code} {r.text}")
    return None, None


def main():
    wb = build_workbook()
    wb.save(FILENAME)
    print(f"Saved: {FILENAME} ({os.path.getsize(FILENAME)} bytes)")

    ok, checks = validate(FILENAME)
    print(f"Validation: {'PASSED' if ok else 'FAILED'}")
    for k, v in checks.items():
        print(f"  {k}: {v}")

    if not ok:
        print(json.dumps({"status":"FAILURE","validation":"FAILED","filename":FILENAME,
                          "rows_testplan":checks.get("tp_rows",0),
                          "rows_metadata":checks.get("md_rows",0)}, indent=2))
        sys.exit(1)

    token = os.environ.get("GITHUB_TOKEN", "")
    if not token:
        print("GITHUB_TOKEN not set. File generated locally only.")
        print(json.dumps({"status":"IN_PROGRESS","validation":"PASSED",
                          "filename":FILENAME,"output_file_path":os.path.abspath(FILENAME),
                          "rows_testplan":checks["tp_rows"],
                          "rows_metadata":checks["md_rows"]}, indent=2))
        sys.exit(0)

    gh_url, sha = push_github(FILENAME, token)
    if gh_url:
        print(json.dumps({"status":"SUCCESS","validation":"PASSED","filename":FILENAME,
                          "output_file_path":f"{OUTPUT_DIR}/{FILENAME}",
                          "github_url":gh_url,"commit_sha":sha,
                          "rows_testplan":checks["tp_rows"],
                          "rows_metadata":checks["md_rows"]}, indent=2))
    else:
        print(json.dumps({"status":"FAILURE","validation":"PASSED","filename":FILENAME,
                          "rows_testplan":checks["tp_rows"],
                          "rows_metadata":checks["md_rows"]}, indent=2))
        sys.exit(1)


if __name__ == "__main__":
    main()
