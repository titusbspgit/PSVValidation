#!/usr/bin/env python3
"""
Agent 7: Excel Generation Script for MIPI_CSI TestPlan
Timestamp: 2026-10-13 16:00:00 IST
Output: MIPI_CSI_TestPlan_20261013_160000.xlsx

INSTRUCTIONS:
  1. pip install openpyxl requests
  2. export GITHUB_TOKEN=<your_pat>
  3. python generate_mipi_csi_testplan_20261013_160000.py

This script generates a genuine .xlsx workbook with:
  - TestPlan sheet (visible, with data validation dropdown)
  - MetaData sheet (veryHidden)
Then uploads the binary to GitHub, replacing any placeholder.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import json, os, sys, base64

# ============================================================
# INPUT DATA - TESTPLAN_JSON (from Agent 6B)
# ============================================================
TESTPLAN_JSON = [
    {
        "Index": 1,
        "SS / Module": "MIPI_CSI",
        "Feature": "Register Read/Write Validation (RB Register Block)",
        "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This test performs register-level read/write validation on 60 MIPI CSI2 Receive Block (RB) registers. The test executes in two active phases: Phase 1 verifies that all 60 registers contain their expected default/reset values after power-on, skipping any registers that are not readable. Phase 2 writes six distinct test data patterns to each writable register, reads back the values, and compares the read data against expected values computed using the register read mask, write mask, and default value properties. Certain registers are excluded from the write/read phase via a skip list. A soft reset check function is defined but not actively invoked in the current test flow.",
        "Test Steps / Procedure": "Step 1: Execute the default value check routine. For each of the 60 RB registers, read the register value. If the register is non-readable (read mask is zero), skip it. Compare the read value against the expected default value for that register. If a mismatch is detected, increment the default-fail counter and log the failure.\n\nStep 2: Execute the write/read validation routine. Iterate over six test data patterns (all-ones, alternating 0xAA, alternating 0x55, all-zeros, checkerboard 0xA5, and upper-half 0xFFFF0000). For each pattern, write the pattern to each of the 60 registers, skipping registers that are in the skip list or have a zero write mask. Then read back each register, skipping non-writable and non-readable registers. Compute the expected read-back value using the write mask, read mask, and default value. Compare the actual read value against the computed expected value. If a mismatch is detected, increment the write-fail counter and log the failure.\n\nStep 3: Evaluate overall test result. If both the default-fail counter and write-fail counter are zero, the test passes. If either counter is greater than zero, the test fails.",
        "Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m1_addr_aw_ch0_data, dma_m1_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch0_data, dma_m2_addr_aw_ch0_Instruction, dma_m3_addr_aw_ch0_data, dma_m3_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_data, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_data, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_data, dma_m2_addr_ar_ch1_Instruction, dma_m3_addr_ar_ch1_data, dma_m3_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_data, dma_m0_addr_aw_ch1_Instruction, dma_m1_addr_aw_ch1_data, dma_m1_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch1_data, dma_m2_addr_aw_ch1_Instruction, dma_m3_addr_aw_ch1_data, dma_m3_addr_aw_ch1_Instruction, dphy_rx",
        "Validation / Acceptance Criteria": "1. All 60 readable registers must return their expected default values after reset; the default-fail counter must remain zero.\n2. For each of the six test data patterns, every writable and readable register (excluding those in the skip list) must return a read-back value that matches the computed expected value based on write mask, read mask, and default value; the write-fail counter must remain zero.\n3. The test passes (finish with success) if and only if both the default-fail counter and the write-fail counter are zero. Any non-zero counter results in test failure.\n4. Three registers at skip list indices 5, 9, and 10 are intentionally excluded from the write/read validation phase and are not evaluated for write-back correctness.",
        "Remarks": "The soft reset check function is defined in the test source but is commented out and not executed in the current test flow. Six test data patterns are used for comprehensive bit-level coverage: all-ones, alternating-AA, alternating-55, all-zeros, checkerboard-A5, and upper-half-ones. The expected read-back value computation accounts for read-only bits retaining their default values while writable bits reflect the written pattern. A total of 60 registers spanning the MIPI CSI2 RB register block are covered, including virtual channel configuration, interrupt control, FIFO threshold, DPHY configuration, DMA address mapping, and boot configuration registers."
    }
]

# ============================================================
# INPUT DATA - META_JSON (from Agent 6A)
# ============================================================
META_JSON = [
    {
        "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
        "Meta Headers": "SOURCE_FILE: program.c\n1. #include <stdio.h>\n2. #include <stdlib.h>\n3. #include \"test_define.c\"\n4. #include \"test_common.h\"\n\nSOURCE_FILE: test_define.c\n1. #include<mipi_csi2_rb_reg.h>",
        "Meta Macros": "SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA, CNT, DEBUG_DISPLAY, and 245 register macros (address, read_mask, write_mask, default_val) for 60 MIPI CSI2 RB registers. Total: 249 macros.",
        "Meta Arrays": "addr_array[60], read_mask_array[60], write_mask_array[60], default_value_array[60], skip_array[60], chk_val[6]",
        "Meta Types": "int, unsigned long int, const int",
        "Meta Test Description": "Register-level read/write validation on 60 MIPI CSI2 RB registers. Phase 1: Default value check. Phase 2: Write/Read with 6 test patterns. Phase 3: soft_reset_chk (commented out).",
        "Meta Test Steps / Procedure": "1. chk_rst_val() - Read all 60 registers, compare against default_value_array. 2. chk_rd_wr() - Write 6 patterns, read back, verify using mask computation. 3. test_case() evaluates def_fail_cnt and wr_fail_cnt, calls finish(0) on success or finish(1) on failure.",
        "Meta Impacted Registers": "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m1_addr_aw_ch0_data, dma_m1_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch0_data, dma_m2_addr_aw_ch0_Instruction, dma_m3_addr_aw_ch0_data, dma_m3_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_data, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_data, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_data, dma_m2_addr_ar_ch1_Instruction, dma_m3_addr_ar_ch1_data, dma_m3_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_data, dma_m0_addr_aw_ch1_Instruction, dma_m1_addr_aw_ch1_data, dma_m1_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch1_data, dma_m2_addr_aw_ch1_Instruction, dma_m3_addr_aw_ch1_data, dma_m3_addr_aw_ch1_Instruction, dphy_rx",
        "Meta Validation / Acceptance Criteria": "Pass: def_fail_cnt == 0 AND wr_fail_cnt == 0 -> finish(0). Fail: def_fail_cnt > 0 OR wr_fail_cnt > 0 -> finish(1)."
    }
]

# ============================================================
# CROSS-DATASET VALIDATION
# ============================================================
tp_names = {r["Test Case Name"] for r in TESTPLAN_JSON}
meta_names = {r["Test Case Name"] for r in META_JSON}
if not tp_names.issubset(meta_names):
    print("CROSS-DATASET VALIDATION FAILED")
    sys.exit(1)
print("Cross-dataset validation PASSED")

# ============================================================
# WORKBOOK GENERATION
# ============================================================
FILENAME = "MIPI_CSI_TestPlan_20261013_160000.xlsx"
OUTPUT_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), FILENAME)

wb = openpyxl.Workbook()

header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_align = Alignment(vertical='top', wrap_text=True)

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

for row_idx, item in enumerate(TESTPLAN_JSON, 2):
    ws_tp.cell(row=row_idx, column=1, value=item.get("Index", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=2, value=item.get("SS / Module", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=3, value=item.get("Feature", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=4, value=item.get("Test Case Name", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=5, value=item.get("Speed", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=6, value=item.get("Mode", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=7, value=item.get("Memory Start Offset", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=8, value=item.get("Memory End Offset", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=9, value=item.get("Test Description", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=10, value=item.get("Test Steps / Procedure", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=11, value=item.get("Impacted Registers", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=12, value=item.get("Validation / Acceptance Criteria", "")).alignment = cell_align
    ws_tp.cell(row=row_idx, column=13, value=item.get("Remarks", "")).alignment = cell_align
    # Column 14 left blank - user selects from dropdown

dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
dv.error = "Please select Required or Not Required"
dv.errorTitle = "Invalid Entry"
dv.prompt = "Select Required or Not Required"
dv.promptTitle = "Code Generation"
last_row = 1 + len(TESTPLAN_JSON)
dv.add(f"N2:N{last_row}")
ws_tp.add_data_validation(dv)

col_widths_tp = {1:8, 2:15, 3:35, 4:35, 5:10, 6:10, 7:20, 8:20, 9:60, 10:60, 11:60, 12:60, 13:60, 14:25}
for c, w in col_widths_tp.items():
    ws_tp.column_dimensions[get_column_letter(c)].width = w
ws_tp.freeze_panes = "A2"

# --- MetaData Sheet ---
ws_meta = wb.create_sheet(title="MetaData")

meta_columns = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

for col_idx, col_name in enumerate(meta_columns, 1):
    cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

for row_idx, item in enumerate(META_JSON, 2):
    ws_meta.cell(row=row_idx, column=1, value=item.get("Test Case Name", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=2, value=item.get("Meta Headers", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=3, value=item.get("Meta Macros", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=4, value=item.get("Meta Arrays", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=5, value=item.get("Meta Types", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=6, value=item.get("Meta Test Description", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=7, value=item.get("Meta Test Steps / Procedure", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=8, value=item.get("Meta Impacted Registers", "")).alignment = cell_align
    ws_meta.cell(row=row_idx, column=9, value=item.get("Meta Validation / Acceptance Criteria", "")).alignment = cell_align

meta_widths = {1:35, 2:50, 3:60, 4:50, 5:30, 6:60, 7:60, 8:60, 9:60}
for c, w in meta_widths.items():
    ws_meta.column_dimensions[get_column_letter(c)].width = w
ws_meta.freeze_panes = "A2"
ws_meta.sheet_state = 'veryHidden'

# ============================================================
# SAVE
# ============================================================
wb.save(OUTPUT_PATH)
file_size = os.path.getsize(OUTPUT_PATH)
print(f"Saved: {OUTPUT_PATH} ({file_size} bytes)")

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
wb2 = openpyxl.load_workbook(OUTPUT_PATH)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
assert wb2["TestPlan"].max_row - 1 == len(TESTPLAN_JSON)
assert wb2["MetaData"].max_row - 1 == len(META_JSON)
assert wb2["MetaData"].sheet_state == 'veryHidden'
assert file_size > 0
print("Post-save validation PASSED")

# ============================================================
# GITHUB UPLOAD
# ============================================================
try:
    import requests
    GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
    if not GITHUB_TOKEN:
        print("No GITHUB_TOKEN. Set env var and re-run to upload.")
        sys.exit(0)

    OWNER = "titusbspgit"
    REPO = "PSVValidation"
    BRANCH = "main"
    REMOTE_PATH = f"Test_Output/MIPI/TestPlan/{FILENAME}"
    API_URL = f"https://api.github.com/repos/{OWNER}/{REPO}/contents/{REMOTE_PATH}"
    HEADERS = {"Authorization": f"token {GITHUB_TOKEN}", "Accept": "application/vnd.github.v3+json"}

    with open(OUTPUT_PATH, "rb") as f:
        b64 = base64.b64encode(f.read()).decode()

    r = requests.get(API_URL, headers=HEADERS, params={"ref": BRANCH})
    sha = r.json().get("sha", "") if r.status_code == 200 else ""

    payload = {"message": "Generated TestPlan Excel Workbook", "content": b64, "branch": BRANCH}
    if sha:
        payload["sha"] = sha

    r = requests.put(API_URL, headers=HEADERS, json=payload)
    if r.status_code in (200, 201):
        data = r.json()
        print(f"GitHub URL: {data['content']['html_url']}")
        print(f"Commit SHA: {data['commit']['sha']}")
        print(f"File size on GitHub: {data['content']['size']} bytes")
    else:
        print(f"Upload failed: {r.status_code}")
        print(r.text)
except ImportError:
    print("requests not installed. pip install requests")
