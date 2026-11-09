#!/usr/bin/env python3
"""
Agent 7 - MIPI_DSI Excel TestPlan Generator
Generates: MIPI_DSI_TestPlan_20261110_020000.xlsx
Two sheets: TestPlan (visible), MetaData (veryHidden)
Code Generation column with Excel dropdown validation.

Usage:
  pip install openpyxl
  python generate_MIPI_DSI_TestPlan_20261110_v2.py

Optional: Set GITHUB_TOKEN env var to auto-upload to GitHub.
  pip install requests
  GITHUB_TOKEN=ghp_xxx python generate_MIPI_DSI_TestPlan_20261110_v2.py
"""
import json
import os
import sys

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.worksheet.datavalidation import DataValidation
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# ============================================================
# EMBEDDED DATA - TESTPLAN_JSON (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Register Read/Write Verification",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This test verifies register-level access for the MIPI DSI subsystem across 52 registers. It performs two main validation phases: (1) Default Value Check - reads all 52 registers after reset and compares each read value against its expected default/reset value, skipping registers that are not readable based on their read mask. Any mismatch is counted as a default value failure. (2) Write/Read-Back Check - writes six distinct test data patterns to all writable registers and reads back each register to verify correctness. Registers that are flagged to be skipped or have a zero write mask are excluded from the write/read-back phase. The expected read-back value accounts for writable and readable bit fields using the read mask, write mask, and default value of each register. Any mismatch is counted as a write/read failure. A soft reset check function is defined but is not invoked in the current test flow. The test reports overall pass if zero failures are detected across both phases, and overall fail otherwise.",
        "Test Steps / Procedure": "Step 1: Initialize failure counters for default value check and write/read check to zero.\nStep 2: Execute the Default Value Check phase - iterate over all 52 MIPI DSI subsystem registers. For each register, check if the register is readable (read mask is non-zero). If not readable, skip to the next register. If readable, read the register and compare the read value against the expected default value. If a mismatch is detected, increment the default value failure counter and log the failure.\nStep 3: Execute the Write/Read-Back Check phase - for each of the six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, 0xA5A5A5A5, 0xFFFF0000), perform the following: (a) Write sub-phase: iterate over all 52 registers. Skip registers flagged in the skip list. Skip read-only registers (zero write mask). For all other registers, write the current test pattern. (b) Read-back sub-phase: iterate over all 52 registers again. Skip registers flagged in the skip list or that are read-only. Read each register and compute the expected value considering writable bits (via write mask), readable bits (via read mask), and default values for non-writable bit positions. Compare the read value against the computed expected value. If a mismatch is detected, increment the write/read failure counter and log the failure.\nStep 4: Evaluate overall result - if either the default value failure counter or the write/read failure counter is greater than zero, report test FAIL. Otherwise, report test PASS.",
        "Impacted Registers": "data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0, vid_params_1, vid_params_2, vid_params_3, vid_params_4, vid_params_5, vid_params_6, vid_params_7, vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Validation / Acceptance Criteria": "1. Default Value Verification: For each of the 52 registers, the value read after reset must exactly match the specified default value for that register. Registers with a zero read mask are excluded from this check.\n2. Write/Read-Back Verification: For each of the six test data patterns written, the value read back from each writable register must match the expected value computed as: expected = (written_data AND read_mask AND write_mask) OR (inverted_write_mask AND read_mask AND default_value). Registers in the skip list or with a zero write mask are excluded.\n3. Overall Pass Criteria: The test passes (finish with success) only if zero default value mismatches and zero write/read-back mismatches are detected across all registers and all test patterns. Any non-zero failure count results in test failure.",
        "Remarks": "The test covers 52 MIPI DSI subsystem registers spanning data FIFO threshold, low power, DBI, interrupt, command FIFO, pixel data FIFO, DPI control, video parameters (0-8), FIFO flush, soft control reset, DMAC configuration, DPHY TX configuration, PHY lane, PLL configuration, and DSI host registers. Six test patterns are used to exercise bit toggling coverage. A soft reset check function exists in the test but is currently not invoked. Certain registers are flagged to be skipped during the write/read-back phase. Read-only registers are automatically excluded from write operations based on their write mask."
    }
]

# ============================================================
# EMBEDDED DATA - META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
    {
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Meta Headers": "File: test_define.c\n#include<mipi_dsi_subsys.h>\n\nFile: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include \"test_define.c\"\n#include \"test_common.h\"",
        "Meta Macros": "File: test_define.c\n#define CNT 52\n\nFile: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n\nConditional compilation macro referenced via #ifdef but definition not supplied in source:\nDEBUG_DISPLAY",
        "Meta Arrays": "File: test_define.c (file scope)\n\nconst unsigned long int addr_array[52]={MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,...,MIZAR_MIPI_DSI_SUBSYS_DSI_HOST};\nconst int default_value_array[52]={...};\nconst int read_mask_array[52]={...};\nconst int write_mask_array[52]={...};\nconst int skip_array[52]={0,0,0,0,1,0,0,0,0,0,...};\n\nFile: program.c (local to chk_rd_wr function)\nint chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};",
        "Meta Types": "TYPE_DEFINITIONS\nNA\n\nOTHER_DECLARATIONS\nFile: test_define.c (file scope)\nint data_rd,data_wr,rst_val;\n\nFile: program.c (file scope)\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;",
        "Meta Test Description": "This testcase performs register-level verification for the MIPI DSI subsystem across 52 registers. It consists of two main phases: (1) Default/reset value check (chk_rst_val) - iterates over all 52 registers via addr_array, reads each register using read_reg(), and compares against expected default values from default_value_array, skipping non-readable registers where read_mask_array is 0x00000000, incrementing def_fail_cnt on mismatch; (2) Write/read-back check (chk_rd_wr) - writes 6 test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000) to all writable registers and reads back to verify, skipping registers flagged in skip_array or with zero write_mask_array, computing expected value as exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) where wr_n = write_mask_array[i] ^ 0xffffffff, incrementing wr_fail_cnt on mismatch. A soft_reset_chk function is defined but commented out in test_case(). The test calls finish(1) on any failure (def_fail_cnt > 0 or wr_fail_cnt > 0) or finish(0) on success.",
        "Meta Test Steps / Procedure": "The full source code content from test_define.c and program.c as provided by Agent 5A.",
        "Meta Impacted Registers": "52 unique registers: data_fifo_threshold_val, low_pwr, dbite, dbi_fdiv, interrupt_raw, interrupt_mask, interrupt_enable, cmnd_fifo_threshold_val, pixdata_fifo_threshold_val, dpi_control, vid_params_0 through vid_params_8, fifo_flush, soft_ctl_rstn, dmac_boot, dmac_addr, dmac_irq_ns, dmac_mst_addr_wr_ch0_data, dmac_mst_addr_wr_ch1_data, dmac_mst_addr_rd_ch0_data, dmac_mst_addr_rd_ch1_data, dmac_mst_addr_rd_ch0_Instruction, dmac_mst_addr_rd_ch1_Instruction, dmac_mst_addr_wr_ch0_Instruction, dmac_mst_addr_wr_ch1_Instruction, dphy_tx_cfgclk, dphy_tx_hs, dphy_IO_continuity, phy_lane0, pll_m, pll_n, vco_cntrl, prop_cntrl, int_cntrl, gmp_cntrl, cpbias_cntrl, clksel, update, force_lock, pll_shadow_control, shadow_clear, gp_clk_en, lock, pll_cntrl_obs, dsi_host",
        "Meta Validation / Acceptance Criteria": "chk_rst_val: For each register, read value must match default_value_array[i]. chk_rd_wr: For each of 6 test patterns written, read-back must match exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Final: finish(0) if no failures, finish(1) otherwise."
    }
]

# ============================================================
# COLUMN DEFINITIONS
# ============================================================
TESTPLAN_COLS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

META_COLS = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

# ============================================================
# STYLES
# ============================================================
HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
WRAP_ALIGN = Alignment(wrap_text=True, vertical="top")

FILENAME = "MIPI_DSI_TestPlan_20261110_020000.xlsx"

# ============================================================
# HELPER FUNCTIONS
# ============================================================
def style_header(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP_ALIGN
    ws.freeze_panes = "A2"

def auto_width(ws, max_w=80):
    for col in ws.columns:
        mx = 0
        for cell in col:
            if cell.value:
                lines = str(cell.value).split("\n")
                mx = max(mx, max(len(l) for l in lines))
        adj = min(mx + 4, max_w)
        ws.column_dimensions[col[0].column_letter].width = adj

# ============================================================
# CROSS-DATASET VALIDATION
# ============================================================
tp_names = {tc["Test Case Name"] for tc in TESTPLAN_JSON}
meta_names = {mc["Test Case Name"] for mc in META_JSON}
missing = tp_names - meta_names
if missing:
    print(f"CROSS-DATASET VALIDATION FAILED: {missing} in TESTPLAN but not in META")
    sys.exit(1)
print("Cross-dataset validation: PASSED")

# ============================================================
# BUILD WORKBOOK
# ============================================================
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"
style_header(ws_tp, TESTPLAN_COLS)

for row_idx, tc in enumerate(TESTPLAN_JSON, 2):
    for col_idx, col_name in enumerate(TESTPLAN_COLS, 1):
        if col_name == "Code Generation (Required / Not)":
            val = ""
        else:
            val = tc.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = WRAP_ALIGN

# Add data validation dropdown for Code Generation column
dv = DataValidation(
    type="list",
    formula1='"Required,Not Required"',
    allow_blank=True,
    showDropDown=False
)
dv.error = "Please select Required or Not Required"
dv.errorTitle = "Invalid Entry"
code_gen_col_idx = len(TESTPLAN_COLS)
for r in range(2, len(TESTPLAN_JSON) + 2):
    dv.add(ws_tp.cell(row=r, column=code_gen_col_idx))
ws_tp.add_data_validation(dv)

auto_width(ws_tp)

# --- MetaData Sheet ---
ws_meta = wb.create_sheet("MetaData")
style_header(ws_meta, META_COLS)

for row_idx, mc in enumerate(META_JSON, 2):
    for col_idx, col_name in enumerate(META_COLS, 1):
        val = mc.get(col_name, "")
        cell = ws_meta.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = WRAP_ALIGN

auto_width(ws_meta)
ws_meta.sheet_state = "veryHidden"

# ============================================================
# SAVE WORKBOOK
# ============================================================
wb.save(FILENAME)
file_size = os.path.getsize(FILENAME)
print(f"Workbook saved: {FILENAME}")
print(f"File size: {file_size} bytes")

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
validation_passed = True
errors = []

# 1. File exists
if not os.path.exists(FILENAME):
    errors.append("File does not exist")
    validation_passed = False

# 2. File size > 0
if file_size <= 0:
    errors.append("File size is 0")
    validation_passed = False

# 3. Reopen workbook
try:
    wb_check = load_workbook(FILENAME)
except Exception as e:
    errors.append(f"Cannot reopen workbook: {e}")
    validation_passed = False
    wb_check = None

if wb_check:
    # 4. TestPlan sheet exists
    if "TestPlan" not in wb_check.sheetnames:
        errors.append("TestPlan sheet missing")
        validation_passed = False

    # 5. MetaData sheet exists
    if "MetaData" not in wb_check.sheetnames:
        errors.append("MetaData sheet missing")
        validation_passed = False

    # 6. TestPlan row count
    ws_tp_check = wb_check["TestPlan"]
    tp_data_rows = ws_tp_check.max_row - 1
    if tp_data_rows != len(TESTPLAN_JSON):
        errors.append(f"TestPlan rows: expected {len(TESTPLAN_JSON)}, got {tp_data_rows}")
        validation_passed = False

    # 7. MetaData row count
    ws_meta_check = wb_check["MetaData"]
    meta_data_rows = ws_meta_check.max_row - 1
    if meta_data_rows != len(META_JSON):
        errors.append(f"MetaData rows: expected {len(META_JSON)}, got {meta_data_rows}")
        validation_passed = False

    # 8. Code Generation column exists
    tp_headers = [ws_tp_check.cell(row=1, column=c).value for c in range(1, ws_tp_check.max_column + 1)]
    if "Code Generation (Required / Not)" not in tp_headers:
        errors.append("Code Generation column missing")
        validation_passed = False

    # 9. Data validation exists
    if len(ws_tp_check.data_validations.dataValidation) == 0:
        errors.append("No data validation found")
        validation_passed = False

    # 10. Code Generation cells are blank
    cg_col = tp_headers.index("Code Generation (Required / Not)") + 1 if "Code Generation (Required / Not)" in tp_headers else None
    if cg_col:
        for r in range(2, ws_tp_check.max_row + 1):
            v = ws_tp_check.cell(row=r, column=cg_col).value
            if v is not None and v != "":
                errors.append(f"Code Generation cell row {r} not blank: {v}")
                validation_passed = False

    wb_check.close()

if validation_passed:
    print("Post-save validation: PASSED")
else:
    print(f"Post-save validation: FAILED - {errors}")

# ============================================================
# UPLOAD TO GITHUB (optional)
# ============================================================
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
github_url = ""
commit_sha = ""

if GITHUB_TOKEN and validation_passed:
    try:
        import base64
        import requests

        with open(FILENAME, "rb") as f:
            content_b64 = base64.b64encode(f.read()).decode()

        api_url = f"https://api.github.com/repos/titusbspgit/PSVValidation/contents/Test_Output/MIPI/TestPlan/{FILENAME}"
        headers = {
            "Authorization": f"token {GITHUB_TOKEN}",
            "Accept": "application/vnd.github.v3+json"
        }

        # Check if file exists to get SHA
        r = requests.get(api_url, headers=headers)
        sha = r.json().get("sha") if r.status_code == 200 else None

        payload = {
            "message": "Generated TestPlan Excel Workbook",
            "content": content_b64,
            "branch": "main"
        }
        if sha:
            payload["sha"] = sha

        r = requests.put(api_url, headers=headers, json=payload)
        if r.status_code in (200, 201):
            data = r.json()
            github_url = data["content"]["html_url"]
            commit_sha = data["commit"]["sha"]
            print(f"GitHub upload: SUCCESS")
            print(f"GitHub URL: {github_url}")
            print(f"Commit SHA: {commit_sha}")
        else:
            print(f"GitHub upload: FAILED ({r.status_code})")
            print(r.text)
    except ImportError:
        print("requests not installed. Skipping GitHub upload.")
elif not GITHUB_TOKEN:
    print("GITHUB_TOKEN not set. Skipping GitHub upload.")
    print("To upload: GITHUB_TOKEN=ghp_xxx python generate_MIPI_DSI_TestPlan_20261110_v2.py")

# ============================================================
# FINAL STATUS
# ============================================================
status = {
    "status": "SUCCESS" if validation_passed and (github_url or not GITHUB_TOKEN) else ("IN_PROGRESS" if validation_passed else "FAILURE"),
    "output_file_path": f"Test_Output/MIPI/TestPlan/{FILENAME}",
    "github_url": github_url if github_url else f"https://github.com/titusbspgit/PSVValidation/blob/main/Test_Output/MIPI/TestPlan/{FILENAME}",
    "filename": FILENAME,
    "rows_testplan": len(TESTPLAN_JSON),
    "rows_metadata": len(META_JSON),
    "commit_sha": commit_sha,
    "validation": "PASSED" if validation_passed else "FAILED"
}

print("\n" + json.dumps(status, indent=2))
