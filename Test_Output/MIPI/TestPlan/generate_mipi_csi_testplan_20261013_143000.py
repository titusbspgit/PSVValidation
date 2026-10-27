#!/usr/bin/env python3
"""
Agent 7: Excel Generation Script for MIPI_CSI TestPlan
Generated: 2026-10-13 14:30:00 IST
Filename: MIPI_CSI_TestPlan_20261013_143000.xlsx
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import json, os, base64, requests

# ============================================================
# INPUT DATA
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
# WORKBOOK GENERATION
# ============================================================

FILENAME = "MIPI_CSI_TestPlan_20261013_143000.xlsx"
OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_PATH = os.path.join(OUTPUT_DIR, FILENAME)

wb = openpyxl.Workbook()

# --- Header Style ---
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_align = Alignment(vertical='top', wrap_text=True)

# ============================================================
# TESTPLAN SHEET
# ============================================================
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Test Description",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Remarks",
    "Code Generation (Required / Not)"
]

# Write headers
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

# Write data rows
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
    # Column 14 (Code Generation) left blank intentionally

# Data Validation for Code Generation column
dv = DataValidation(
    type="list",
    formula1='"Required,Not Required"',
    allow_blank=True,
    showDropDown=False
)
dv.error = "Please select Required or Not Required"
dv.errorTitle = "Invalid Entry"
dv.prompt = "Select Required or Not Required"
dv.promptTitle = "Code Generation"
last_data_row = 1 + len(TESTPLAN_JSON)
dv.add(f"N2:N{last_data_row}")
ws_tp.add_data_validation(dv)

# Auto-size columns
col_widths = {
    1: 8, 2: 15, 3: 35, 4: 35, 5: 10, 6: 10,
    7: 20, 8: 20, 9: 60, 10: 60, 11: 60,
    12: 60, 13: 60, 14: 25
}
for col_num, width in col_widths.items():
    ws_tp.column_dimensions[get_column_letter(col_num)].width = width

# Freeze first row
ws_tp.freeze_panes = "A2"

# ============================================================
# METADATA SHEET
# ============================================================
ws_meta = wb.create_sheet(title="MetaData")

meta_columns = [
    "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
    "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
]

# Write headers
for col_idx, col_name in enumerate(meta_columns, 1):
    cell = ws_meta.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_align

# Write data rows
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

# Auto-size MetaData columns
meta_widths = {1: 35, 2: 50, 3: 60, 4: 50, 5: 30, 6: 60, 7: 60, 8: 60, 9: 60}
for col_num, width in meta_widths.items():
    ws_meta.column_dimensions[get_column_letter(col_num)].width = width

# Freeze first row
ws_meta.freeze_panes = "A2"

# Set MetaData sheet to veryHidden
ws_meta.sheet_state = 'veryHidden'

# ============================================================
# SAVE WORKBOOK
# ============================================================
wb.save(OUTPUT_PATH)
print(f"Workbook saved: {OUTPUT_PATH}")
print(f"File size: {os.path.getsize(OUTPUT_PATH)} bytes")

# ============================================================
# VALIDATION
# ============================================================
wb2 = openpyxl.load_workbook(OUTPUT_PATH)
assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
tp_rows = wb2["TestPlan"].max_row - 1
meta_rows = wb2["MetaData"].max_row - 1
assert tp_rows == len(TESTPLAN_JSON), f"TestPlan rows mismatch: {tp_rows} vs {len(TESTPLAN_JSON)}"
assert meta_rows == len(META_JSON), f"MetaData rows mismatch: {meta_rows} vs {len(META_JSON)}"
assert wb2["MetaData"].sheet_state == 'veryHidden', "MetaData not veryHidden"
print("Validation PASSED")
print(f"TestPlan rows: {tp_rows}")
print(f"MetaData rows: {meta_rows}")

# ============================================================
# GITHUB UPLOAD
# ============================================================
GITHUB_TOKEN = os.environ.get("GITHUB_TOKEN", "")
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
FILE_PATH = f"Test_Output/MIPI/TestPlan/{FILENAME}"

if GITHUB_TOKEN:
    with open(OUTPUT_PATH, "rb") as f:
        content_b64 = base64.b64encode(f.read()).decode()

    api_url = f"https://api.github.com/repos/{OWNER}/{REPO}/contents/{FILE_PATH}"
    headers = {
        "Authorization": f"token {GITHUB_TOKEN}",
        "Accept": "application/vnd.github.v3+json"
    }

    # Get existing file SHA
    resp = requests.get(api_url, headers=headers, params={"ref": BRANCH})
    sha = resp.json().get("sha", "") if resp.status_code == 200 else ""

    payload = {
        "message": "Generated TestPlan Excel Workbook",
        "content": content_b64,
        "branch": BRANCH
    }
    if sha:
        payload["sha"] = sha

    resp = requests.put(api_url, headers=headers, json=payload)
    if resp.status_code in (200, 201):
        result = resp.json()
        print(f"Upload SUCCESS")
        print(f"GitHub URL: {result['content']['html_url']}")
        print(f"Commit SHA: {result['commit']['sha']}")
    else:
        print(f"Upload FAILED: {resp.status_code} {resp.text}")
else:
    print("No GITHUB_TOKEN found. Run locally to upload.")
    print(f"Local file ready at: {OUTPUT_PATH}")
