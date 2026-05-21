#!/usr/bin/env python3
# coding: utf-8

"""
Generate a REAL .xlsx Test Plan workbook from embedded JSON using openpyxl.
Sheets:
- TestPlan (visible) with required non-meta columns
- MetaData (veryHidden) with meta columns

Timestamp: IST (Asia/Kolkata), format YYYYMMDD_HHMMSS
Output dir (relative to repo root): from env OUTPUT_DIR or default 'Test_Output/GPIO/TestPlan'
"""

from __future__ import annotations

import os
import json
from datetime import datetime
from zoneinfo import ZoneInfo
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Font

# Embedded final aggregated JSON (preserve exactly)
JSON_DATA: List[Dict[str, Any]] = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO Register Read/Write and Reset Default Verification",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verify that GPIO GP0 pin registers (GPIO_8 to GPIO_27) power up to their specified default values (ignoring bit0) and that writable bits accept data patterns while non-writable bits retain default values based on defined read/write masks.",
    "Meta Test Description": "The test validates two aspects for a set of GPIO GP0 pin registers (indices 8 through 27) using arrays populated from GPIO definition/offset headers: (1) Default/reset values: For each address in addr_array, if readable and not marked to skip by skip_rst_array, the register is read and the value is masked with 0xFFFFFFFE (to ignore bit0) before comparison with default_value_array. Any mismatch increments def_fail_cnt. (2) Read/Write behavior: For each of six 32-bit data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000), the code writes (pattern & write_mask) to each address that is not skipped (skip_array==0) and has a non-zero write_mask, then reads back (read_val & read_mask) and compares against an expected value computed as: (pattern & read_mask & write_mask) | ((~write_mask) & read_mask & default_value). Any mismatch increments wr_fail_cnt. The overall test result is pass if both def_fail_cnt and wr_fail_cnt are zero; otherwise fail.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Default comparison intentionally ignores the least-significant bit due to input behavior; registers with zero read or write mask are skipped accordingly; soft-reset routine is present but disabled; current skip arrays indicate no registers are skipped.",
    "Test Steps / Procedure": "1) Read the default values of GPIO GP0 GPIO_8 to GPIO_27 where readable and not flagged for reset-skip, and confirm they match the documented defaults while ignoring bit0. 2) For each of the predefined data patterns, write to each GPIO GP0 GPIO_8 to GPIO_27 register where writing is allowed, constrained by the write mask. 3) Read back each register where readable and verify writable bits reflect the written pattern while non-writable bits remain at their reset defaults as constrained by the masks. 4) Report the test as pass only if no mismatches are detected in either the default-value or read/write checks; otherwise, report fail.",
    "Meta Test Steps / Procedure": "Initialization: def_fail_cnt=0; wr_fail_cnt=0. A) Default check loop: For i in [0..CNT-1]: addr=addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); if (data==default_value_array[i]) pass; else {def_fail_cnt++; log mismatch}. B) Read/Write check for each pattern in chk_val[] = {0xFFFFFFFF,0xAAAAAAAA,0x55555555,0xF5F5F5F5,0xA5A5A5A5,0xFFFF0000}: 1) Write phase: For i in [0..CNT-1]: addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; write_reg(addr, (pattern & write_mask_array[i])). 2) Read/verify phase: For i in [0..CNT-1]: addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xFFFFFFFF); exp_val = ((pattern & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) pass; else {wr_fail_cnt++; log mismatch}. C) Completion: if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).",
    "Impacted Registers": "GPIO GP0 GPIO_8, GPIO GP0 GPIO_9, GPIO GP0 GPIO_10, GPIO GP0 GPIO_11, GPIO GP0 GPIO_12, GPIO GP0 GPIO_13, GPIO GP0 GPIO_14, GPIO GP0 GPIO_15, GPIO GP0 GPIO_16, GPIO GP0 GPIO_17, GPIO GP0 GPIO_18, GPIO GP0 GPIO_19, GPIO GP0 GPIO_20, GPIO GP0 GPIO_21, GPIO GP0 GPIO_22, GPIO GP0 GPIO_23, GPIO GP0 GPIO_24, GPIO GP0 GPIO_25, GPIO GP0 GPIO_26, GPIO GP0 GPIO_27",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27",
    "Validation / Acceptance Criteria": "Pass if: (1) For all readable GPIO GP0 GPIO_8–GPIO_27 registers not marked for reset-skip, the reset value read (with bit0 ignored) matches the documented default; and (2) For each data pattern and for each register that is both writable and readable, the readback value equals the combination of written bits (within the write/read mask) and default values for non-writable bits. Any deviation constitutes a failure.",
    "Meta Validation / Acceptance Criteria": "Default check: (read_reg(addr) & 0xFFFFFFFE) == default_value_array[i] for all i where read_mask_array[i] != 0 and skip_rst_array[i] == 0. Read/Write check: For each pattern p and each i where skip_array[i]==0, write_mask_array[i]!=0, read_mask_array[i]!=0, let data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xFFFFFFFF); exp_val = ((p & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); require data_rd == exp_val. Overall: def_fail_cnt==0 and wr_fail_cnt==0.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>, #include <stdlib.h>, #include \"test_common.h\", #include \"test_define.c\", #include <gpio/gpio_def.h>, #include <gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49; #define SOFT_RST_REG_ADDRESS 0x00000000; #define SOFT_RST_REG_DATA 0x00000000",
    "Meta Arrays": "skip_array[20] = {all zeros}; skip_rst_array[20] = {all zeros}"
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "GPIO Interrupt – Negative Edge Trigger Enable and Handling",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verify that enabling negative-edge interrupts per GPIO pad generates an interrupt when a falling edge is applied, the group status reflects the event, and the interrupt and status can be cleared correctly.",
    "Meta Test Description": "The test enables the platform interrupt for the target GPIO instance (GPIO0 -> IRQ 87 or GPIO1 -> IRQ 88) via the system interrupt enable register. It configures each GP0 pad register starting at GPIO_8 (indexed by i=0..31 with 4-byte stride) by writing bitfields (bits 20, 18, 16 set). For each pad index i, it: (1) clears raw pad interrupt status for that bit in gpio_intr_raw_stclr1; (2) enables the corresponding bit in INTR1_INTR_EN1; (3) initializes int_pend=1; (4) programs a pad drive/data register at 0xA0243ffc to first drive all ones and then drive ~wr_val to create a falling edge on bit i; (5) waits (with timeout) for Default_IRQHandler to run and clear int_pend. On timeout, logs an error. In Default_IRQHandler, it reads the current pad register at GPIO_8 + i*4. It checks that bit0 is 0 and that bit1 is nonzero; then it reads INTR1_INTR_STS1 to confirm the set bit matches the expected local_wr (1<<i). It reprograms the pad register (bits 20 and 16 set), clears the raw pad interrupt for that bit via gpio_intr_raw_stclr1, and verifies INTR1_INTR_STS1 becomes 0. Finally, it clears the system RAW_STCR1 for the GPIO interrupt source and clears the corresponding GIC IRQ. The test accumulates errors (timeouts or mismatches) in test_err and finishes with pass (0) if none.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Requires platform interrupt controller to be enabled for the selected GPIO instance; uses address 0xA0243ffc to drive pad values for edge generation; operates on GP0 pad registers starting at GPIO_8; test depends on ISR execution within timeout.",
    "Test Steps / Procedure": "1) Enable the system interrupt for the target GPIO instance in INTR_EN1 and enable the corresponding GIC interrupt (IRQ 87 for GPIO0 or IRQ 88 for GPIO1). 2) For each pad from GPIO_8 to GPIO_39, configure the pad control register with the required bit settings for negative-edge interrupt operation. 3) For each pad bit i (0..31): a) Clear its raw interrupt in gpio_intr_raw_stclr1 and enable its bit in INTR1_INTR_EN1. b) Drive the pad data source at 0xA0243ffc to create a falling edge on bit i. c) Wait for the interrupt to be taken (ISR executed) within the timeout. 4) In the ISR handling, confirm the pad register reflects the expected event, group status INTR1_INTR_STS1 shows the bit set, then clear the pad interrupt in gpio_intr_raw_stclr1 and verify INTR1_INTR_STS1 returns to 0. 5) Clear the platform RAW_STCR1 and the corresponding GIC IRQ. 6) Pass if no timeouts or mismatches occur for all pads; otherwise fail.",
    "Meta Test Steps / Procedure": "Initialization: test_err=0. Conditionally enable GIC IRQ: if GPIO0 defined -> GIC_EnableIRQ(87); if GPIO1 defined -> GIC_EnableIRQ(88). Conditionally enable system interrupt: if GPIO0 defined -> write_reg(INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); if GPIO1 defined -> write_reg(INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR). Preload pad driver: write_reg(0xA0243ffc, 0xffffffff). Configure pads: for (i=0..31) { addr1 = GPIO_8 + (i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10); }. For each bit i=0..31: wr_val = 1<<i; write_reg(gpio_intr_raw_stclr1, wr_val); write_reg(INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { printf timeout error with GPIO index (i+8); test_err++; }. ISR (Default_IRQHandler): local_wr=1<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr = GPIO_8 + (i*4); rdata = read_reg(raddr); if ((rdata & 0x1) != 0) { test_err++; } if ((rdata & 0x2) != 0x0) { rdata_grp = read_reg(INTR1_INTR_STS1); if ((rdata_grp & local_wr) == 0) test_err++; raddr2 = GPIO_8 + (i*4); write_reg(raddr2, (1<<20)|(1<<16)); write_reg(gpio_intr_raw_stclr1, local_wr); rdata_grp = read_reg(INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; if GPIO0 defined { write_reg(RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); } if GPIO1 defined { write_reg(RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); } } else { test_err++; }. Completion: finish(test_err).",
    "Impacted Registers": "GPIO_8..GPIO_39, INTR1_INTR_EN1, INTR1_INTR_STS1, gpio_intr_raw_stclr1, INTR_EN1, RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8 (+ i*4 for i=0..31), MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Validation / Acceptance Criteria": "Pass if for every pad from GPIO_8 to GPIO_39: a negative-edge event causes the interrupt to assert and be serviced within the timeout; INTR1_INTR_STS1 shows the corresponding bit set; after clearing, INTR1_INTR_STS1 returns to 0; and no unexpected pad state is observed. Any timeout or mismatch results in fail.",
    "Meta Validation / Acceptance Criteria": "For each i=0..31: (1) ISR must execute before timeout (int_pend cleared). (2) In ISR: (rdata & 0x1) == 0; (rdata & 0x2) != 0; (read_reg(INTR1_INTR_STS1) & (1<<i)) != 0; after write_reg(gpio_intr_raw_stclr1, (1<<i)), read_reg(INTR1_INTR_STS1) == 0. System RAW_STCR1 and GIC IRQ must be cleared without errors. Overall pass if test_err==0.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>, #include <lss_sysreg.h>, #include \\\"test_define.c\\\", #include <test_common.h>, #include <gpio/gpio_def.h>, #include <gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "skip_array[20] = {all zeros}"
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "GPIO Interrupt – Positive Edge Trigger Enable and Handling",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validate that enabling positive-edge interrupts on GPIO pads triggers an interrupt on a rising edge, that the group interrupt status reflects the event, and that both per-pin raw status and system interrupt status can be cleared and re-enabled correctly.",
    "Meta Test Description": "The test enables the GIC IRQ for the selected GPIO instance (IRQ 87 if GPIO0, IRQ 88 if GPIO1). It enables the platform interrupt source for the selected GPIO instance by writing to INTR_EN1. It then configures each pad register from GPIO_8 through GPIO_39 (i=0..31, 4-byte stride) to enable positive-edge detection by writing 0x00020000 to (GPIO_8 + i*4). All GPIOs 8–39 are put into input mode by writing 0x000000FF to GPIO_IO_CTRL_GROUP1..4. Group interrupt output is enabled by writing 0xFFFFFFFF to gp0_intr2_intr_en1. For each pad index i (0..31): the stimulus register at 0xA0243ffc is first driven low (0x00000000), a wait flag (int_pend) is armed, and then it is driven high (0xFFFFFFFF) to generate a single rising edge. A timeout loop (count=2000, with waits) polls for the ISR to run and clear int_pend; on timeout, an error is logged and the loop breaks. After servicing, the driver is optionally driven low again to prepare for the next iteration. In Default_IRQHandler: int_pend is cleared; the group status INTR1_INTR_STS1 is read and gp0_intr2_intr_en1 is written with 0 to mask during service. If group status is nonzero, proceed; otherwise, increment error. The handler then clears per-pin raw interrupt status by writing 0x00010000 to each pad register (GPIO_8 + j*4, j=0..31). After a short wait, INTR1_INTR_STS1 is read again and must be 0; otherwise increment error. The system RAW_STCR1 bit corresponding to the selected GPIO instance is written to clear the system-level interrupt latch, and READBACK must show the bit cleared; otherwise increment error. Finally, gp0_intr2_intr_en1 is re-enabled with 0xFFFFFFFF, and the GIC IRQ is cleared. Test ends with finish(test_err).",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Requires enabling the appropriate platform interrupt in INTR_EN1 and the GIC IRQ for the GPIO instance. GPIO pads 8–39 must be configured in input mode via GPIO_IO_CTRL_GROUP1..4. A stimulus at 0xA0243ffc is used to generate rising edges. The group interrupt is masked during ISR service and re-enabled afterward.",
    "Test Steps / Procedure": "1) Enable the platform interrupt for the target GPIO instance in INTR_EN1 and enable its GIC IRQ. 2) Configure GPIO_8 through GPIO_39 for positive-edge detection. 3) Set GPIO_IO_CTRL_GROUP1..4 so GPIOs 8–39 operate as inputs. 4) Enable group interrupt output via gp0_intr2_intr_en1. 5) For each pad, generate a rising edge using the stimulus register at 0xA0243ffc and wait for the ISR to complete within the timeout. 6) In the ISR, confirm INTR1_INTR_STS1 indicates an interrupt, then clear per-pin raw status by programming each pad register and verify INTR1_INTR_STS1 returns to 0. 7) Clear the system RAW_STCR1 bit for the GPIO source and verify it is cleared. 8) Re-enable gp0_intr2_intr_en1 and clear the GIC IRQ. 9) Pass if all pads trigger and clear correctly without timeouts or mismatches; otherwise fail.",
    "Meta Test Steps / Procedure": "Initialization: test_err=0. Conditionally enable GIC: if GPIO0 then GIC_EnableIRQ(87); if GPIO1 then GIC_EnableIRQ(88). Enable platform interrupt: if GPIO0 then write_reg(INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); if GPIO1 then write_reg(INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR). Configure positive-edge on pads: for (i=0..31) write_reg(GPIO_8 + i*4, 0x00020000). Wait 10. Configure input mode: write_reg(GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(GPIO_IO_CTRL_GROUP4, 0x000000FF). Wait 10. Enable group: write_reg(gp0_intr2_intr_en1, 0xFFFFFFFF). For each i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while (int_pend==1 && --timeout>0) wait_on(10); if (timeout==0) { printf timeout with index i; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10). ISR (Default_IRQHandler): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(INTR1_INTR_STS1); write_reg(gp0_intr2_intr_en1, 0x00000000); if ((rdata_grp & 0xFFFFFFFF)==0) { printf error; test_err++; } For j=0..31: write_reg(GPIO_8 + j*4, 0x00010000); wait_on(2); rdata_grp=read_reg(INTR1_INTR_STS1); if (rdata_grp!=0x0) { printf clear failed; test_err++; } If GPIO0: write_reg(RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(RAW_STCR1); if (rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) { printf sysreg not cleared; test_err++; } If GPIO1: write_reg(RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(RAW_STCR1); if (rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) { printf sysreg not cleared; test_err++; } write_reg(gp0_intr2_intr_en1, 0xFFFFFFFF); Clear GIC IRQ (87 or 88). Completion: finish(test_err).",
    "Impacted Registers": "INTR_EN1, GPIO_8..GPIO_39, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, gp0_intr2_intr_en1, INTR1_INTR_STS1, RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8 (+ i*4 for i=0..31), MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Validation / Acceptance Criteria": "Pass if each pad from GPIO_8 to GPIO_39 produces an interrupt on a rising edge, INTR1_INTR_STS1 asserts during service and returns to 0 after clearing per-pin raw status, and the system RAW_STCR1 bit for the GPIO source is cleared as expected. Any timeout waiting for the ISR or any mismatch in status clearing results in failure.",
    "Meta Validation / Acceptance Criteria": "For each i=0..31: ISR must execute before timeout (int_pend cleared). On ISR entry: read_reg(INTR1_INTR_STS1) != 0; after clearing all per-pin raw statuses (write 0x00010000 to GPIO_8 + j*4, j=0..31), read_reg(INTR1_INTR_STS1) == 0. After writing RAW_STCR1 with the corresponding GPIO bit, a readback must show the bit cleared. Overall pass if test_err == 0.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <lss_sysreg.h>, #include <stdio.h>, #include <test_define.c>, #include <test_common.h>, #include <gpio/gpio_def.h>, #include <gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "skip_array[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}"
  }
]

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]

def build_workbook(data: List[Dict[str, Any]]) -> Workbook:
    wb = Workbook()
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Header rows
    ws1.append(TESTPLAN_COLUMNS)
    ws2.append(METADATA_COLUMNS)

    bold = Font(bold=True)
    for cell in ws1[1]:
        cell.font = bold
    for cell in ws2[1]:
        cell.font = bold

    # Data rows (preserve object order)
    for obj in data:
        ws1.append([obj.get(col, "") for col in TESTPLAN_COLUMNS])
        ws2.append([
            obj.get("Index", ""),
            obj.get("Test Case Name", ""),
            obj.get("Meta Test Description", ""),
            obj.get("Meta Test Steps / Procedure", ""),
            obj.get("Meta Impacted Registers", ""),
            obj.get("Meta Validation / Acceptance Criteria", ""),
            obj.get("Meta Headers", ""),
            obj.get("Meta Macros", ""),
            obj.get("Meta Arrays", ""),
        ])

    # Freeze header rows
    ws1.freeze_panes = "A2"
    ws2.freeze_panes = "A2"

    # Very hide the MetaData sheet
    ws2.sheet_state = 'veryHidden'

    return wb


def main() -> None:
    # Validate JSON
    if not isinstance(JSON_DATA, list) or not all(isinstance(x, dict) for x in JSON_DATA):
        raise SystemExit("json_data must be a list of objects")

    # Build workbook
    wb = build_workbook(JSON_DATA)

    # Filename with IST timestamp
    ist = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")

    out_dir = os.getenv("OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
    os.makedirs(out_dir, exist_ok=True)

    fname = f"testplan_{ts}.xlsx"
    fpath = os.path.join(out_dir, fname)

    wb.save(fpath)
    print(f"WROTE {fpath}")


if __name__ == "__main__":
    main()
