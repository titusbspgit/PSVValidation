#!/usr/bin/env python3
"""
Agent 7: Excel Generation - MIPI_CSI TestPlan Workbook
Generates genuine .xlsx with TestPlan + veryHidden MetaData sheets
"""
import json, os, sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
    from openpyxl.utils import get_column_letter
except ImportError:
    print('ERROR: pip install openpyxl'); sys.exit(1)

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

IP_NAME = "MIPI_CSI"
IST = timezone(timedelta(hours=5, minutes=30))
timestamp = datetime.now(IST).strftime("%Y%m%d_%H%M%S")
FILENAME = f"{IP_NAME}_TestPlan_{timestamp}.xlsx"
OUTPUT_DIR = "Test_Output/MIPI/TestPlan"
OUTPUT_PATH = os.path.join(OUTPUT_DIR, FILENAME)

TP_COLS = [
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

def validate():
    tp_names = {r["Test Case Name"] for r in TESTPLAN_JSON}
    meta_names = {r["Test Case Name"] for r in META_JSON}
    for n in tp_names:
        if n not in meta_names:
            print(f"FAIL: '{n}' missing from META"); return False
    return True

def generate():
    if not validate(): return None
    wb = Workbook()
    hf = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
    hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    ha = Alignment(horizontal="center", vertical="center", wrap_text=True)
    ca = Alignment(vertical="top", wrap_text=True)
    bdr = Border(left=Side(style="thin"), right=Side(style="thin"),
                 top=Side(style="thin"), bottom=Side(style="thin"))

    # === TestPlan Sheet ===
    ws = wb.active
    ws.title = "TestPlan"
    for ci, cn in enumerate(TP_COLS, 1):
        c = ws.cell(row=1, column=ci, value=cn)
        c.font = hf; c.fill = hfill; c.alignment = ha; c.border = bdr
    for ri, rec in enumerate(TESTPLAN_JSON, 2):
        for ci, cn in enumerate(TP_COLS, 1):
            v = "" if cn == "Code Generation (Required / Not)" else str(rec.get(cn, ""))
            c = ws.cell(row=ri, column=ci, value=v)
            c.alignment = ca; c.border = bdr
    cg_col = len(TP_COLS)
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=False)
    dv.error = "Select Required or Not Required"
    dv.errorTitle = "Invalid"
    cl = get_column_letter(cg_col)
    lr = 1 + len(TESTPLAN_JSON)
    dv.add(f"{cl}2:{cl}{lr}")
    ws.add_data_validation(dv)
    ws.freeze_panes = "A2"
    for ci in range(1, len(TP_COLS)+1):
        ml = len(str(TP_COLS[ci-1]))
        for row in ws.iter_rows(min_row=2, max_row=lr, min_col=ci, max_col=ci):
            for cell in row:
                if cell.value: ml = max(ml, min(len(str(cell.value)), 80))
        ws.column_dimensions[get_column_letter(ci)].width = min(ml+4, 60)

    # === MetaData Sheet ===
    wm = wb.create_sheet("MetaData")
    for ci, cn in enumerate(META_COLS, 1):
        c = wm.cell(row=1, column=ci, value=cn)
        c.font = hf; c.fill = hfill; c.alignment = ha; c.border = bdr
    for ri, rec in enumerate(META_JSON, 2):
        for ci, cn in enumerate(META_COLS, 1):
            v = str(rec.get(cn, ""))
            c = wm.cell(row=ri, column=ci, value=v)
            c.alignment = ca; c.border = bdr
    wm.freeze_panes = "A2"
    mlr = 1 + len(META_JSON)
    for ci in range(1, len(META_COLS)+1):
        ml = len(str(META_COLS[ci-1]))
        for row in wm.iter_rows(min_row=2, max_row=mlr, min_col=ci, max_col=ci):
            for cell in row:
                if cell.value: ml = max(ml, min(len(str(cell.value)), 80))
        wm.column_dimensions[get_column_letter(ci)].width = min(ml+4, 60)
    wm.sheet_state = "veryHidden"

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(OUTPUT_PATH)
    print(f"Saved: {OUTPUT_PATH}")

    # Validate
    if not os.path.exists(OUTPUT_PATH) or os.path.getsize(OUTPUT_PATH) == 0:
        print("FAIL: file missing or empty"); return None
    wb2 = load_workbook(OUTPUT_PATH)
    if "TestPlan" not in wb2.sheetnames or "MetaData" not in wb2.sheetnames:
        print("FAIL: sheets missing"); return None
    tp_r = wb2["TestPlan"].max_row - 1
    m_r = wb2["MetaData"].max_row - 1
    if tp_r != len(TESTPLAN_JSON) or m_r != len(META_JSON):
        print(f"FAIL: row count mismatch tp={tp_r} meta={m_r}"); return None
    print(f"PASSED: tp={tp_r} meta={m_r} size={os.path.getsize(OUTPUT_PATH)}")
    return {"filename": FILENAME, "path": OUTPUT_PATH, "rows_testplan": tp_r, "rows_metadata": m_r, "validation": "PASSED"}

if __name__ == "__main__":
    r = generate()
    print(json.dumps(r, indent=2) if r else json.dumps({"status":"FAILURE"}))
