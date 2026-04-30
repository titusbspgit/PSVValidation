#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Deterministic fallback automation to generate GPIO TestPlan Excel (.xlsx)
- Consumes embedded JSON (from Stage1 TestPlan JSON payload)
- Produces Excel with exact formatting/visibility rules
- Validates XLSX structure
- Emits IST timestamp and output path to stdout for logs
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta
from copy import deepcopy
from zipfile import ZipFile

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl not available: {e}")
    sys.exit(1)

# ---- Configuration ----
IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
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

# ---- Embedded TestPlan JSON (authoritative) ----
TP_DICT = {
  "TC1": {"Index":"1","SS / Module":"GPIO","Feature":"Independent control register for each GPIO.","Test Case Name":"gpio_reg_wr_rd_test","Test Description":"This test checks default reset values and verifies write/read behavior of GPIO-related registers using defined masks and expected-value composition.","Speed":"NA","Mode":"NA","Memory Start Offset":"NA","Memory End Offset":"NA","Remarks":"The code masks out bit0 (0xfffffffe) when comparing default values. Comments note that reading default values may show DIN becoming 1 if not forced; forcing zero at DIN can change bit-level selection and cause mismatches.","Test Steps / Procedure":"1) Initialize and perform default value checks across GPIO registers using read masks. 2) Iterate over a set of write patterns and write to writable registers using their write masks. 3) Read back values with read masks and compute expected values combining written bits and default values for non-writable bits. 4) Count mismatches and report pass/fail based on zero failures.","Impacted Registers":"GP0_GPIO_8, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GPIO_INTR_RAW_STCLR1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4","Validation / Acceptance Criteria":"- Default values: Each readable register value (with bit0 ignored) must equal its documented reset value; any mismatch is a failure. - Write/read: For each register and pattern, the read value (masked) must equal (written bits within the write and read masks) OR (default bits where writes are not allowed); any mismatch is a failure. - Overall: Test passes only if both default and write/read failure counters are zero and finish(0) is called.","Code Generation (Required / Not)":"","Hidden_Test_Case_Name":"gpio_reg_wr_rd_test","Hidden_Test_Description":"Objective: Verify default reset values and masked write/read behavior for GPIO-related registers (per-pin control, group IO control/status, and interrupt control/status).","Hidden_Remarks":"1) During default value comparison, the code uses data = (data_rd & 0xfffffffe) to ignore bit0. 2) Comment in test_define.c: \"when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value\".","Hidden_Test_Steps_Procedure":"Entry Points: main/test harness calls test_case();\nA) test_case():\n1. Call chk_rst_val().\n2. Call chk_rd_wr().\n3. If (def_fail_cnt > 0 || wr_fail_cnt > 0) then finish(1) else finish(0).\n\nB) chk_rst_val(): Default value check\nLoop: for (i = 0; i < CNT; i++)\n  b1. addr = addr_array[i]. If (skip_rst_array[i] == 1) continue.\n  b2. If (read_mask_array[i] == 0x00000000) continue.\n  b3. READ: data_rd = read_reg(addr).\n  b4. Mask comparison value: data = (data_rd & 0xFFFFFFFE).\n  b5. If (data == default_value_array[i]) then PASS else { def_fail_cnt++; printf failure with addr, expected, read }.\n\nC) chk_rd_wr(): Write and readback check with patterns\nInitialize: chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}.\nOuter loop: for (j = 0; j < 6; j++)\n  c1. data_wr = chk_val[j].\n  c2. WRITE phase: for (i = 0; i < CNT; i++)\n      - addr = addr_array[i]. If (skip_array[i] == 1) continue.\n      - If (write_mask_array[i] == 0x00000000) continue.\n      - WRITE: write_reg(addr, (data_wr & write_mask_array[i])).\n  c3. READ/verify phase: for (i = 0; i < CNT; i++)\n      - addr = addr_array[i]. If (skip_array[i] == 1) continue.\n      - If (write_mask_array[i] == 0x00000000) continue.\n      - If (read_mask_array[i] == 0x00000000) continue.\n      - READ: data_rd = read_reg(addr) & read_mask_array[i].\n      - Compute wr_n = (write_mask_array[i] ^ 0xFFFFFFFF).\n      - Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n      - If (data_rd == exp_val) PASS else { wr_fail_cnt++; printf mismatch with addr, expected, read }.\n\nD) Registers accessed via addr_array[i] (READ/WRITE depending on masks and skips):\n  - Per-pin control: MIZAR_GPIO_GP0_GPIO_8 .. MIZAR_GPIO_GP0_GPIO_39\n  - Interrupt: MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1\n  - Group IO control: MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4\n  - Group data out: MIZAR_GPIO_GPIO_DOUT_GROUP1..4\n  - Group data in: MIZAR_GPIO_GPIO_DIN_GROUP1..4\n\nTiming: No explicit delays in this test other than functional loops.\nExit: Control returns to test_case() which invokes finish(status).","Hidden_Impacted_Registers":"MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4","Hidden_Validation_Acceptance_Criteria":"1) Default check: For each i where read_mask_array[i] != 0 and skip_rst_array[i] == 0: (read_reg(addr_array[i]) & 0xFFFFFFFE) == default_value_array[i]. 2) Write/read check: For each pattern and each i where write_mask_array[i] != 0, read_mask_array[i] != 0, skip_array[i] == 0: (read_reg(addr_array[i]) & read_mask_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). 3) Final: def_fail_cnt == 0 and wr_fail_cnt == 0, leading to finish(0)."},
  "TC2": {"Index":"2","SS / Module":"GPIO","Feature":"Interrupt based on negative edge detection at GPIO input","Test Case Name":"test_gpio_negedge_intr_en","Test Description":"This test verifies that a negative edge on each GPIO pad triggers an interrupt and that status and raw bits can be properly cleared.","Speed":"NA","Mode":"Interrupt","Memory Start Offset":"0xA0243ffc","Memory End Offset":"0xA0243ffc","Remarks":"A bounded timeout prevents infinite wait during interrupt polling. The wait is armed before generating the edge to avoid race conditions. GPIO0/GPIO1 compile-time defines select the IRQ line and sysreg bits.","Test Steps / Procedure":"1) Enable the interrupt controller for the selected GPIO instance. 2) Enable the system register interrupt output (INTR_EN1). 3) Initialize pad driver to high state via a test register. 4) Configure per-pin control for GPIO 8–39 to input mode and enable negative-edge detect. 5) For each pin, clear any raw status (GPIO_INTR_RAW_STCLR1) and enable that pin (GP0_INTR1_INTR_EN1). 6) For each pin, generate a falling edge on that pad and wait for the interrupt with a timeout. 7) In the handler, verify the per-pin DIN indicates low, confirm the masked group status (GP0_INTR1_INTR_STS1) for this pin, clear the per-pin raw status and the group raw bit (GPIO_INTR_RAW_STCLR1), then clear the system register raw status (RAW_STCR1) and the interrupt controller. 8) Repeat for all pins and report pass/fail.","Impacted Registers":"INTR_EN1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GPIO_INTR_RAW_STCLR1, RAW_STCR1, GP0_GPIO_8","Validation / Acceptance Criteria":"- For each pin, the interrupt must be observed within the configured timeout; a timeout indicates failure. - In the handler, the input state for the pin must read low after the falling edge. - The group status for the pin must be set and, after clearing, must read as zero. - The system register raw status must be cleared successfully. - The overall test passes if no errors are reported and the final status indicates success.","Code Generation (Required / Not)":"","Hidden_Test_Case_Name":"test_gpio_negedge_intr_en","Hidden_Test_Description":"Objective: Verify negative-edge interrupt enable and servicing for GPIO 8–39, including status set/clear at pin, group, and system levels.","Hidden_Remarks":"1) Uses a bounded wait loop (timeout initialized to 5000) while polling int_pend. 2) The wait (int_pend=1) is armed before generating the negative edge to avoid race conditions. 3) The pad driver is set via write_reg(0xA0243ffc, ...). 4) GPIO0/GPIO1 macros control which IRQ line (87/88) and sysreg bits are used.","Hidden_Test_Steps_Procedure":"Entry Point: test_case()\n1. test_err = 0.\n2. Ifdef GPIO0: GIC_EnableIRQ(87). Ifdef GPIO1: GIC_EnableIRQ(88).\n3. Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_INTR_EN1 = LSS_SYSREG_INTR_EN1_GPIO0_INTR. Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_INTR_EN1 = LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n4. WRITE 0xA0243ffc = 0xFFFFFFFF (drive all high).\n5. Configure per-pin control (GPIOs 8..39) loop i=0..31:\n   - addr1 = (MIZAR_GPIO_GP0_GPIO_8 + i*4).\n   - WRITE addr1 = ((1<<20) | (1<<18) | (1<<16))  // doe=1 (input), neie=1, iclr=1.\n   - wait_on(10).\n6. For each pin i=0..31:\n   a) wr_val = (1u << i).\n   b) WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 = wr_val (pre-clear any raw).\n   c) WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 = wr_val (enable only this bit).\n   d) wait_on(10).\n   e) int_pend = 1 (arm wait before edge).\n   f) WRITE 0xA0243ffc = 0xFFFFFFFF (set high), wait_on(30), then WRITE 0xA0243ffc = ~wr_val (drive only this pad low -> falling edge).\n   g) timeout = 5000; while(int_pend && timeout--) wait_on(10). If timeout==0: printf timeout error for (i+8); test_err++.\n7. finish(test_err).\n\nInterrupt Handler: Default_IRQHandler()\n1. local_wr = (1u << i).\n2. int_pend = 0.\n3. WRITE 0xA0243ffc = 0xFFFFFFFF (return pads to known high state).\n4. raddr = (MIZAR_GPIO_GP0_GPIO_8 + i*4); READ rdata = read_reg(raddr).\n5. Check DIN for falling edge: If ((rdata & 0x1) != 0) then test_err++.\n6. If ((rdata & 0x2) != 0x0) { // expected raw set\n   a) READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n   b) If ((rdata_grp & local_wr) == 0) then test_err++.\n   c) WRITE (MIZAR_GPIO_GP0_GPIO_8 + i*4) = ((1<<20) | (1<<16)) // clear per-pin raw while keeping doe=1.\n   d) WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 = local_wr // clear group raw.\n   e) READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); If (rdata_grp != 0x0) then test_err++.\n   f) Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO0_INTR; GIC_ClearIRQ(87).\n      Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO1_INTR; GIC_ClearIRQ(88).\n} else { test_err++; }\n\nTiming: wait_on(10/30) used during edge generation and polling; main-loop timeout initialized to 5000.\nExit: finish(test_err).","Hidden_Impacted_Registers":"MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR","Hidden_Validation_Acceptance_Criteria":"1) For each i (0..31), the interrupt must be observed before timeout elapses in the main loop (int_pend cleared by ISR). 2) In ISR: (rdata & 0x1) == 0 ensures DIN reads low after falling edge. 3) (read_reg(GP0_INTR1_INTR_STS1) & (1<<i)) != 0 must hold on entry, and becomes 0 after per-pin clear and group raw clear. 4) System raw status (RAW_STCR1 & GPIOx bit) is cleared after write. 5) test_err remains 0; finish(0)."},
  "TC3": {"Index":"3","SS / Module":"GPIO","Feature":"Interrupt based on positive edge detection at GPIO input","Test Case Name":"test_gpio_pedge_all_pads_en","Test Description":"This test enables positive-edge interrupts on all GPIO pads, generates rising edges, and validates group status and system status clear operations.","Speed":"NA","Mode":"Interrupt","Memory Start Offset":"0xA0243ffc","Memory End Offset":"0xA0243ffc","Remarks":"A bounded timeout is used during interrupt wait. Group masking is applied during ISR service. Input mode is configured via group IO control registers.","Test Steps / Procedure":"1) Enable the interrupt controller for the selected GPIO instance. 2) Enable the system register interrupt output (INTR_EN1). 3) Enable positive-edge detection for GPIO 8–39 via per-pin control registers. 4) Configure GPIOs 8–39 as inputs using group IO control registers (GPIO_IO_CTRL_GROUP1–4). 5) Enable all pins in the masked group enable register (GP0_INTR1_INTR_EN1). 6) For each pin, generate a rising edge and wait for the interrupt with a timeout. 7) In the handler, verify masked group status (GP0_INTR1_INTR_STS1), clear per-pin raw status, verify the group status clears to zero, clear the system register raw status (RAW_STCR1), and re-enable the group interrupt.","Impacted Registers":"INTR_EN1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, RAW_STCR1, GP0_GPIO_8","Validation / Acceptance Criteria":"- Each pin must trigger an interrupt on the rising edge within the timeout; a timeout indicates failure. - Group interrupt status must be set during service and read as zero after raw status is cleared. - The system register raw status must clear successfully on write. - The overall test passes if no errors are recorded and the final status indicates success.","Code Generation (Required / Not)":"","Hidden_Test_Case_Name":"test_gpio_pedge_all_pads_en","Hidden_Test_Description":"Objective: Verify positive-edge interrupt enable on all pads, ISR servicing, raw/group/system status clear, and re-enabling for subsequent iterations.","Hidden_Remarks":"1) Uses timeouts in the polling loop to avoid hangs. 2) Masks the group enable during ISR to avoid nested interrupts. 3) Input mode for pads 8–39 is set by group IO control writes (0x000000FF per group). 4) Pad driving for edges uses writes to 0xA0243ffc.","Hidden_Test_Steps_Procedure":"Entry Point: test_case()\n1. Ifdef GPIO0: GIC_EnableIRQ(87). Ifdef GPIO1: GIC_EnableIRQ(88).\n2. Enable sysreg interrupt output: Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_INTR_EN1 = LSS_SYSREG_INTR_EN1_GPIO0_INTR; Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_INTR_EN1 = LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n3. For i=0..31: WRITE (MIZAR_GPIO_GP0_GPIO_8 + i*4) = 0x00020000 // enable posedge (bit17=1).\n4. wait_on(10).\n5. Configure input mode via group IO control: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP1 = 0x000000FF; similarly GROUP2..GROUP4 = 0x000000FF.\n6. wait_on(10).\n7. Enable all masked group interrupts: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF.\n8. For i=0..31 (per pin):\n   a) WRITE 0xA0243ffc = 0x00000000 (prepare low), wait_on(10).\n   b) int_pend = 1 (arm before edge).\n   c) WRITE 0xA0243ffc = 0xFFFFFFFF (generate rising edge).\n   d) timeout = 2000; while (int_pend == 1 && --timeout > 0) wait_on(10).\n   e) If (timeout == 0) { printf timeout; test_err++; break; }\n   f) WRITE 0xA0243ffc = 0x00000000; wait_on(10) (prep for next iteration).\n9. finish(test_err).\n\nInterrupt Handler: Default_IRQHandler()\n1. wr_val = (1 << i); int_pend = 0.\n2. READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n3. WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0x00000000 (mask during service).\n4. If ((rdata_grp & 0xFFFFFFFF) != 0) { PASS group raised } else { printf error; test_err++; }.\n5. Clear per-pin raw for all pins: for j=0..31 WRITE (MIZAR_GPIO_GP0_GPIO_8 + j*4) = 0x00010000; wait_on(2).\n6. READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) PASS else { printf error; test_err++; }.\n7. Clear sysreg raw:\n   - Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO0_INTR; READ-back MIZAR_LSS_SYSREG_RAW_STCR1; ensure bit cleared.\n   - Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO1_INTR; READ-back MIZAR_LSS_SYSREG_RAW_STCR1; ensure bit cleared.\n8. Re-enable group interrupt: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF.\n9. Clear GIC IRQ: Ifdef GPIO0: GIC_ClearIRQ(87); Ifdef GPIO1: GIC_ClearIRQ(88).\n\nTiming: wait_on(10) in main loop per edge generation; ISR uses wait_on(2) after raw clears.\nExit: finish(test_err).","Hidden_Impacted_Registers":"MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4","Hidden_Validation_Acceptance_Criteria":"1) For each i, the interrupt must be observed before timeout elapses (int_pend cleared by ISR). 2) In ISR, masked group status (read_reg(GP0_INTR1_INTR_STS1)) must be non-zero on entry and become 0 after per-pin raw clear loop. 3) System raw status bit in RAW_STCR1 must read as cleared after write. 4) Overall pass is indicated when test_err remains 0 and finish(0) executes."}
}


def tc_dict_to_rows(tc_dict: dict) -> list:
    # Preserve TC1, TC2, ... order
    ordered_keys = sorted(tc_dict.keys(), key=lambda k: int(k[2:]) if k.startswith('TC') else k)
    rows = [deepcopy(tc_dict[k]) for k in ordered_keys]
    return rows


def build_schema(rows: list) -> list:
    # Union of keys preserving first-seen order across rows
    seen = []
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    return seen


def ensure_values(row: dict, headers: list) -> list:
    return [row.get(h, "") for h in headers]


def approx_autofit(ws):
    maxlen = {}
    for r in ws.iter_rows(values_only=True):
        for idx, val in enumerate(r, start=1):
            s = "" if val is None else str(val)
            maxlen[idx] = max(maxlen.get(idx, 0), len(s))
    for idx, m in maxlen.items():
        from openpyxl.utils import get_column_letter
        col = get_column_letter(idx)
        width = max(10, min(80, int(m * 1.2) + 2))
        ws.column_dimensions[col].width = width


def add_thin_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows():
        for cell in row:
            if cell.value is not None and str(cell.value) != "":
                cell.border = border


def normalize_numbering(text: str) -> str:
    if not text:
        return ""
    lines = [ln.strip() for ln in str(text).splitlines() if ln.strip()]
    if not lines:
        return str(text)
    out = []
    n = 1
    for ln in lines:
        for p in ["- ", "* ", "• "]:
            if ln.startswith(p):
                ln = ln[len(p):]
        if len(ln) > 2 and ln[0].isdigit() and (ln[1] in [')', '.'] or (ln[1].isdigit() and ln[2] in [')', '.'])):
            parts = ln.split(' ', 1)
            ln = parts[1] if len(parts) > 1 else ln
        out.append(f"{n}. {ln}")
        n += 1
    return "\n".join(out)


def set_meta_very_hidden(wb):
    if 'Meta_data_sheet' in wb.sheetnames:
        wb['Meta_data_sheet'].sheet_state = 'veryHidden'


def main():
    rows = tc_dict_to_rows(TP_DICT)
    if not rows:
        print("ERROR: No rows in input JSON")
        sys.exit(2)

    headers = build_schema(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    ws.append(headers)
    for r in rows:
        ws.append(ensure_values(r, headers))

    header_font = Font(bold=True)
    for c in range(1, len(headers) + 1):
        ws.cell(row=1, column=c).font = header_font
    ws.freeze_panes = 'A2'

    approx_autofit(ws)

    ws_meta = wb.create_sheet('Meta_data_sheet')
    ws_meta.append(META_COLS)
    idx_map = {h: i+1 for i, h in enumerate(headers)}
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for h in META_COLS:
            if h in idx_map:
                row_vals.append(ws.cell(row=r, column=idx_map[h]).value)
            else:
                row_vals.append("")
        ws_meta.append(row_vals)

    set_meta_very_hidden(wb)

    ws.title = 'TestPlan'

    current_headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    data_dicts = []
    for r in range(2, ws.max_row + 1):
        d = {}
        for c, h in enumerate(current_headers, start=1):
            d[h] = ws.cell(row=r, column=c).value
        data_dicts.append(d)

    new_headers = MAIN_ORDER
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)
    ws.append(new_headers)
    for d in data_dicts:
        row_vals = [d.get(h, "") for h in new_headers]
        ts_idx = new_headers.index("Test Steps / Procedure")
        vac_idx = new_headers.index("Validation / Acceptance Criteria")
        row_vals[ts_idx] = normalize_numbering(row_vals[ts_idx])
        row_vals[vac_idx] = normalize_numbering(row_vals[vac_idx])
        ws.append(row_vals)

    blue_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    center = Alignment(horizontal='center', vertical='center', wrap_text=False)
    for c in range(1, len(new_headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = center
        cell.fill = blue_fill

    wrap_cols = {
        new_headers.index("Test Description") + 1,
        new_headers.index("Remarks") + 1,
        new_headers.index("Test Steps / Procedure") + 1,
        new_headers.index("Validation / Acceptance Criteria") + 1,
    }
    for r in range(2, ws.max_row + 1):
        for c in range(1, len(new_headers) + 1):
            if c in wrap_cols:
                ws.cell(row=r, column=c).alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            else:
                if new_headers[c-1] == 'Index':
                    ws.cell(row=r, column=c).alignment = Alignment(horizontal='center', vertical='top')
                else:
                    ws.cell(row=r, column=c).alignment = Alignment(horizontal='left', vertical='top')

    approx_autofit(ws)

    base_height = 15
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in wrap_cols:
            txt = ws.cell(row=r, column=c).value or ""
            lines = str(txt).count('\n') + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines

    add_thin_borders(ws)

    if "Code Generation (Required / Not)" in new_headers:
        from openpyxl.utils import get_column_letter
        col_idx = new_headers.index("Code Generation (Required / Not)") + 1
        last_row = ws.max_row
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        dv.ranges.append(f"{get_column_letter(col_idx)}2:{get_column_letter(col_idx)}{last_row}")
        ws.add_data_validation(dv)

    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    ts_fname = now_ist.strftime('%Y%m%d_%H%M%S')
    ts_human = now_ist.strftime('%Y-%m-%d %H:%M:%S')
    file_name = f"{IP_NAME}_TestPlan_{ts_fname}.xlsx"
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, file_name)

    wb.save(out_path)

    required_parts = {'[Content_Types].xml', 'xl/workbook.xml'}
    try:
        with ZipFile(out_path, 'r') as zf:
            names = set(zf.namelist())
            missing = [p for p in required_parts if p not in names]
            if missing:
                print(f"ERROR: XLSX validation failed, missing parts: {missing}")
                sys.exit(3)
    except Exception as e:
        print(f"ERROR: XLSX validation failed to open ZIP: {e}")
        sys.exit(4)

    print("XLSX_GENERATION_SUCCESS")
    print(f"OUTPUT_FILE={out_path}")
    print(f"IST_TIMESTAMP={ts_human}")

if __name__ == '__main__':
    main()
