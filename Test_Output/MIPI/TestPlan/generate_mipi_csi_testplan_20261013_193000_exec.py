#!/usr/bin/env python3
"""
Agent 7: Excel Generation & GitHub Push Script
Generates MIPI_CSI_TestPlan_20261013_193000.xlsx and pushes to GitHub.
Run: pip install openpyxl requests && python generate_mipi_csi_testplan_20261013_193000_exec.py
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import os, sys, base64, requests, json

FILENAME = "MIPI_CSI_TestPlan_20261013_193000.xlsx"
GITHUB_REPO = "titusbspgit/PSVValidation"
GITHUB_BRANCH = "main"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"

TESTPLAN_JSON = [
  {
    "Index": 1,
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
    "Feature": "Register Read/Write Validation (RB Register Block)",
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

def generate_workbook():
    wb = openpyxl.Workbook()
    hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wa = Alignment(wrap_text=True, vertical='top')

    # === TestPlan Sheet ===
    ws = wb.active
    ws.title = "TestPlan"
    cols = ["Index","SS / Module","Feature","Test Case Name","Speed","Mode",
            "Memory Start Offset","Memory End Offset","Test Description",
            "Test Steps / Procedure","Impacted Registers",
            "Validation / Acceptance Criteria","Remarks",
            "Code Generation (Required / Not)"]
    for ci, cn in enumerate(cols, 1):
        c = ws.cell(row=1, column=ci, value=cn)
        c.font = hf; c.fill = hfill; c.alignment = wa
    for ri, tc in enumerate(TESTPLAN_JSON, 2):
        ws.cell(row=ri, column=1, value=tc["Index"]).alignment = wa
        ws.cell(row=ri, column=2, value=tc["SS / Module"]).alignment = wa
        ws.cell(row=ri, column=3, value=tc["Feature"]).alignment = wa
        ws.cell(row=ri, column=4, value=tc["Test Case Name"]).alignment = wa
        ws.cell(row=ri, column=5, value=tc["Speed"]).alignment = wa
        ws.cell(row=ri, column=6, value=tc["Mode"]).alignment = wa
        ws.cell(row=ri, column=7, value=tc["Memory Start Offset"]).alignment = wa
        ws.cell(row=ri, column=8, value=tc["Memory End Offset"]).alignment = wa
        ws.cell(row=ri, column=9, value=tc["Test Description"]).alignment = wa
        ws.cell(row=ri, column=10, value=tc["Test Steps / Procedure"]).alignment = wa
        ws.cell(row=ri, column=11, value=tc["Impacted Registers"]).alignment = wa
        ws.cell(row=ri, column=12, value=tc["Validation / Acceptance Criteria"]).alignment = wa
        ws.cell(row=ri, column=13, value=tc["Remarks"]).alignment = wa
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
    dv.error = "Select Required or Not Required"
    dv.errorTitle = "Invalid"
    dv.add(f"N2:N{1+len(TESTPLAN_JSON)}")
    ws.add_data_validation(dv)
    widths = [8,15,35,35,8,8,20,20,60,60,60,60,60,25]
    for i, w in enumerate(widths, 1):
        ws.column_dimensions[get_column_letter(i)].width = w
    ws.freeze_panes = "A2"

    # === MetaData Sheet ===
    wm = wb.create_sheet("MetaData")
    mcols = ["Test Case Name","Meta Headers","Meta Macros","Meta Arrays",
             "Meta Types","Meta Test Description","Meta Test Steps / Procedure",
             "Meta Impacted Registers","Meta Validation / Acceptance Criteria"]
    for ci, cn in enumerate(mcols, 1):
        c = wm.cell(row=1, column=ci, value=cn)
        c.font = hf; c.fill = hfill; c.alignment = wa
    for ri, m in enumerate(META_JSON, 2):
        wm.cell(row=ri, column=1, value=m["Test Case Name"]).alignment = wa
        wm.cell(row=ri, column=2, value=m["Meta Headers"]).alignment = wa
        wm.cell(row=ri, column=3, value=m["Meta Macros"]).alignment = wa
        wm.cell(row=ri, column=4, value=m["Meta Arrays"]).alignment = wa
        wm.cell(row=ri, column=5, value=m["Meta Types"]).alignment = wa
        wm.cell(row=ri, column=6, value=m["Meta Test Description"]).alignment = wa
        wm.cell(row=ri, column=7, value=m["Meta Test Steps / Procedure"]).alignment = wa
        wm.cell(row=ri, column=8, value=m["Meta Impacted Registers"]).alignment = wa
        wm.cell(row=ri, column=9, value=m["Meta Validation / Acceptance Criteria"]).alignment = wa
    mwidths = [35,50,60,60,30,60,60,60,60]
    for i, w in enumerate(mwidths, 1):
        wm.column_dimensions[get_column_letter(i)].width = w
    wm.freeze_panes = "A2"
    wm.sheet_state = 'veryHidden'

    wb.save(FILENAME)
    fsize = os.path.getsize(FILENAME)
    print(f"Saved: {FILENAME} ({fsize} bytes)")

    # Validate
    wb2 = openpyxl.load_workbook(FILENAME)
    assert "TestPlan" in wb2.sheetnames
    assert "MetaData" in wb2.sheetnames
    assert wb2["TestPlan"].max_row == 1 + len(TESTPLAN_JSON)
    assert wb2["MetaData"].max_row == 1 + len(META_JSON)
    assert wb2["MetaData"].sheet_state == 'veryHidden'
    print("Validation: PASSED")
    return FILENAME

def push_to_github(filepath, token):
    with open(filepath, 'rb') as f:
        content = base64.b64encode(f.read()).decode('utf-8')
    url = f"https://api.github.com/repos/{GITHUB_REPO}/contents/{OUTPUT_DIR}/{FILENAME}"
    headers = {"Authorization": f"token {token}", "Accept": "application/vnd.github.v3+json"}
    # Check if file exists
    r = requests.get(url, headers=headers, params={"ref": GITHUB_BRANCH})
    sha = r.json().get("sha") if r.status_code == 200 else None
    payload = {"message": "Generated TestPlan Excel Workbook", "content": content, "branch": GITHUB_BRANCH}
    if sha:
        payload["sha"] = sha
    r = requests.put(url, headers=headers, json=payload)
    if r.status_code in (200, 201):
        data = r.json()
        print(f"Pushed: {data['content']['html_url']}")
        print(f"Commit: {data['commit']['sha']}")
        return data
    else:
        print(f"Push failed: {r.status_code} {r.text}")
        return None

if __name__ == "__main__":
    generate_workbook()
    token = os.environ.get("GITHUB_TOKEN", "")
    if token:
        push_to_github(FILENAME, token)
    else:
        print("No GITHUB_TOKEN set. Run manually or set token.")
