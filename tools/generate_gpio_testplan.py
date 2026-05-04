#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import re
import zipfile
from datetime import datetime, timedelta, timezone
from collections import OrderedDict

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ------------------ Input JSON (embedded, deterministic) ------------------
RAW_JSON = {
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface.",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validates GPIO register default values and read/write behavior across the defined address set using masks.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "VRRW registers are skipped per skip_array. Certain DIN defaults can read high unless forced; default-read for some groups skipped per skip_rst_array to avoid mismatches.",
    "Test Steps / Procedure": "1) Initialize default-value check over the configured register list and skip entries marked for reset-skip. 2) For each readable register, read and mask the value and compare to the expected default. 3) For each test pattern, write masked data to writable registers while skipping entries per configuration. 4) Read back masked values and compute the expected result considering write and read masks and preserved default bits. 5) Accumulate any mismatches and report pass when no mismatches are observed.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "1) Default values: For each readable register not skipped, the masked read must equal the documented default; any mismatch fails the test. 2) Write/read checks: For each pattern and register not skipped, the masked readback must equal the expected value derived from the write mask, read mask, and default value; any mismatch fails. 3) Overall result: The test passes only if there are zero default mismatches and zero write/read mismatches.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Performs two phases: (1) Default value check for each address in addr_array using read_mask_array and skip_rst_array; read data is masked with 0xFFFFFFFE then compared to default_value_array. (2) Read/write verification using six patterns {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each address not in skip_array and with nonzero write mask, writes (data_wr & write_mask_array[i]); then reads back data_rd=(read_reg(addr) & read_mask_array[i]) and computes expected value exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Compares data_rd to exp_val. Tracks def_fail_cnt and wr_fail_cnt. Finishes with finish(0) if both counts are zero, else finish(1).",
    "Hidden_Remarks": "Comment notes: // SKIPPING VRRW registers; skip_array used to skip such registers. // when reading default values the din value is becoming 1 automatically if we don't force any value, but if we force zero to din bit level sel becoming high, so that reading value not matched with expected value — thus skip_rst_array marks group DIN-related registers to avoid false mismatches.",
    "Hidden_Test_Steps_Procedure": "1) Call chk_rst_val(): For i=0..CNT-1, set addr=addr_array[i]; if skip_rst_array[i]==1 continue; if read_mask_array[i]==0 continue; read data_rd=read_reg(addr); set data=(data_rd & 0xFFFFFFFE); if (data==default_value_array[i]) pass else increment def_fail_cnt and log error. 2) Call chk_rd_wr(): For j over chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}, set data_wr=chk_val[j]. 2a) Write phase: For i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 continue; if write_mask_array[i]==0 continue; write_reg(addr,(data_wr & write_mask_array[i])). 2b) Read/compare phase: For i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 continue; if write_mask_array[i]==0 continue; if read_mask_array[i]==0 continue; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) pass else increment wr_fail_cnt and log error. 3) If (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0).",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Default phase: data=(read_reg(addr)&0xFFFFFFFE) must equal default_value_array[i] for all i not skipped and with readable mask. Write/read phase: data_rd=(read_reg(addr)&read_mask_array[i]) must equal exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])) for all i not skipped and with nonzero read/write masks. Test passes only if def_fail_cnt==0 and wr_fail_cnt==0 leading to finish(0); any nonzero count leads to finish(1)."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "neie",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies negative-edge interrupt enable per GPIO pin and correct raw/masked interrupt behavior with clearing and DIN state.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "The wait is armed before generating the falling edge to avoid race conditions. A timeout of 5000 iterations bounds the wait to prevent infinite hang; comment notes it may need adjustment based on simulation timing.",
    "Test Steps / Procedure": "1) Enable the platform interrupt for the selected GPIO instance. 2) Enable the system-register interrupt routing for the selected GPIO instance. 3) Drive the pad driver high to establish a known state. 4) For each GPIO pin, configure input mode, enable falling-edge detection, and clear the per-pin raw flag. 5) For each pin, clear the corresponding group raw status, enable only that pin’s interrupt, arm the wait, and generate a falling edge on that pin. 6) Wait for the interrupt with a bounded timeout; on timeout, record an error and continue. 7) At completion, report pass if no errors were recorded.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "1) Each interrupt must arrive before the timeout after a falling edge is generated; a timeout constitutes a failure. 2) In the handler, the input value for the serviced pin must be low; otherwise, it is a failure. 3) The masked group status must show the serviced pin set before clearing, and must read as zero after clearing; any deviation is a failure. 4) The test passes only if all pins meet these checks and no errors are recorded.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Negative-edge interrupt test for GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88 based on GPIO0/GPIO1). Enables system-register interrupt routing via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad controller at 0xA0243ffc to 0xFFFFFFFF. For i=0..31: writes per-pin register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (falling-edge enable), and iclr=1 (clear per-pin raw). For each i: wr_val=(1<<i); clears group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enables only this pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); arms wait (int_pend=1); creates falling edge by writing 0xFFFFFFFF then ~wr_val to 0xA0243ffc; waits with timeout (5000) looping on int_pend with wait_on(10). On timeout, prints error and increments test_err. finish(test_err). Default_IRQHandler: local_wr=(1<<i); sets int_pend=0; writes 0xFFFFFFFF to 0xA0243ffc to return to known state; reads per-pin register raddr=(MIZAR_GPIO_GP0_GPIO_8 + i*4) into rdata; if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) then read group masked status MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp and if ((rdata_grp & local_wr) == 0) test_err++; clear per-pin raw/doe via write_reg(raddr2, (1<<20)|(1<<16)); clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); verify MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else test_err++; clear system-register raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, appropriate bit) and call GIC_ClearIRQ(87/88); else (if raw bit not set) test_err++.",
    "Hidden_Remarks": "Arms the wait (int_pend=1) before generating the falling edge to avoid interrupt race. Uses a bounded timeout of 5000 iterations to prevent infinite waiting; comment suggests the timeout may be tuned for the simulation time base.",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1). 2) Enable system-register interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) or write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) depending on instance. 3) write_reg(0xA0243ffc, 0xFFFFFFFF) to drive all pads high. 4) For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10). 5) For i=0..31: wr_val=(1<<i); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); set int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); wait_on(30); write_reg(0xA0243ffc, ~wr_val); start timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { printf error; test_err++; }. 6) finish(test_err). ISR Default_IRQHandler(): local_wr=(1<<i); int_pend=0; write_reg(0xA0243ffc, 0xFFFFFFFF); raddr=MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata=read_reg(raddr); if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(raddr2, (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin: the interrupt must occur before the loop timeout; in the ISR, DIN bit0 must be 0 (after falling edge). The masked group status (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must indicate the serviced pin (bit local_wr set) and must read 0x0 after clearing both per-pin raw and group raw. Any timeout, unexpected DIN value, missing group bit, or uncleared status increments test_err; test passes only if test_err==0 at finish."
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "peie",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Checks rising-edge interrupt enable across all GPIO pins, proper masking during service, raw clear, and system-register status clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Uses a bounded timeout of 2000 iterations to prevent infinite wait during interrupt polling. The pending flag is made volatile to ensure visibility across the handler and main loop.",
    "Test Steps / Procedure": "1) Enable the platform interrupt for the selected GPIO instance. 2) Enable the system-register interrupt routing for the selected GPIO instance. 3) For each pin, enable rising-edge detection. 4) Configure the IO control groups for input mode. 5) Enable the group interrupt for all pins. 6) For each pin, generate a rising edge and wait for the interrupt with a bounded timeout; on timeout, record an error. 7) In the handler, mask the group, clear per-pin raw for all pins, verify the group status clears to zero, clear the system-register status, re-enable the group, and clear the platform IRQ. 8) Report pass if no errors were recorded.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "1) For each generated rising edge, an interrupt must be observed before the timeout; otherwise, it is a failure. 2) Group masked status must be nonzero on entry to the handler and must be zero after clearing per-pin raw; any mismatch is a failure. 3) System-register interrupt status must be cleared successfully after servicing; if not cleared, it is a failure. 4) The test passes only if all checks succeed with zero accumulated errors.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Rising-edge interrupt enable test for all GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88). Enables system-register interrupt routing: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to set peie=1 (bit17). wait_on(10). Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10). Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) to create a rising edge; poll with timeout=2000 on int_pend with wait_on(10); on timeout print error, increment test_err, and break. After ISR return, write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask; if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to clear per-pin raw (iclr=1); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } Clear system-register raw: #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).",
    "Hidden_Remarks": "Uses a bounded timeout of 2000 iterations in the main loop to avoid infinite waits. The int_pend variable is declared volatile to ensure the handler update is observed by the polling loop.",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1). 2) Enable system-register interrupt routing to GPIO by writing MIZAR_LSS_SYSREG_INTR_EN1 with the appropriate GPIO interrupt enable mask. 3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to enable peie (bit17=1). 4) wait_on(10). 5) Configure IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). 6) wait_on(10). 7) Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 8) For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); poll while (int_pend==1 && --timeout>0) with wait_on(10); on timeout print error, test_err++, break; then write_reg(0xA0243ffc, 0x00000000); wait_on(10). 9) finish(test_err). ISR Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xFFFFFFFF) == 0) { print error; test_err++; } For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) to set iclr=1; wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { print error; test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { print error; test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { print error; test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin, an interrupt must be observed before the main-loop timeout after producing a rising edge; otherwise, test_err increments. In the ISR: rdata_grp (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must be non-zero on entry; after writing 0x00010000 to all per-pin registers and waiting, it must read 0x0; failure increments test_err. System-register raw status must be cleared and verified by reading back MIZAR_LSS_SYSREG_RAW_STCR1; any bit remaining set increments test_err. Test passes only if test_err==0 at finish."
  }
}

# Configuration
OUTPUT_DIR = os.path.join('Test_Output', 'GPIO', 'TestPlan')
MAIN_SHEET_NAME = 'TestPlan'
DATA_SHEET_NAME = 'Data'
META_SHEET_NAME = 'Meta_data_sheet'

META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

MAIN_ORDER = [
    'Index',
    'SS / Module',
    'Feature',
    'Test Case Name',
    'Test Description',
    'Speed',
    'Mode',
    'Memory Start Offset',
    'Memory End Offset',
    'Remarks',
    'Test Steps / Procedure',
    'Impacted Registers',
    'Validation / Acceptance Criteria',
    'Code Generation (Required / Not)'
]

WRAP_COLS = {
    'Test Description',
    'Remarks',
    'Test Steps / Procedure',
    'Validation / Acceptance Criteria'
}

ALLOWED_CG = ['Required', 'Blank', 'Not Required']

# ------------------ Utilities ------------------

def to_ist_now():
    return datetime.now(timezone(timedelta(hours=5, minutes=30)))


def build_rows_from_json(raw: dict):
    # Convert dict of TC entries to ordered list sorted by numeric Index
    items = []
    for k, v in raw.items():
        items.append(v)
    items.sort(key=lambda x: int(x.get('Index', '0')))

    # Schema union preserving first-seen key order
    seen_keys = []
    for rec in items:
        for key in rec.keys():
            if key not in seen_keys:
                seen_keys.append(key)
    return items, seen_keys


def autosize_columns(ws):
    from openpyxl.utils import get_column_letter
    dim = {}
    for row in ws.iter_rows(values_only=True):
        for i, v in enumerate(row, start=1):
            val = '' if v is None else str(v)
            dim[i] = max(dim.get(i, 0), len(val))
    for i, w in dim.items():
        ws.column_dimensions[get_column_letter(i)].width = min(max(w + 2, 12), 80)


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def normalize_numbering(text: str) -> str:
    if text is None:
        return ''
    s = str(text).strip()
    if not s:
        return ''
    # Prefer newline splitting; if none, split on patterns like 1) 2) or 1. 2.
    if '\n' in s:
        parts = [p.strip() for p in s.split('\n') if p.strip()]
    else:
        # Split by occurrences of digits followed by ) or . while preserving words
        parts = []
        tokens = re.split(r'(?:^|\s)(\d+)[\)\.]\s*', s)
        # tokens like ['', '1', 'text1 ', '2', 'text2', ...]
        tmp = []
        i = 0
        while i < len(tokens):
            if i + 1 < len(tokens) and tokens[i] == '':
                # start
                i += 1
                num = tokens[i]
                i += 1
                seg = tokens[i] if i < len(tokens) else ''
                parts.append(seg.strip())
                i += 1
            else:
                # fallback: treat whole string as one item
                parts = [s]
                break
    # Re-number as 1., 2., 3., joined with newlines
    out_lines = []
    for idx, seg in enumerate(parts, start=1):
        if not seg:
            continue
        out_lines.append(f"{idx}. {seg}")
    return '\n'.join(out_lines)


def create_workbook(items, schema_keys):
    wb = Workbook()
    ws = wb.active
    ws.title = DATA_SHEET_NAME

    # Header style
    header_font = Font(bold=True, color='FFFFFFFF')
    header_fill = PatternFill('solid', fgColor='4F81BD')
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)

    # Write headers as per schema_keys
    for c, key in enumerate(schema_keys, start=1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align

    # Write data rows
    for r, rec in enumerate(items, start=2):
        for c, key in enumerate(schema_keys, start=1):
            ws.cell(row=r, column=c, value=rec.get(key, ''))

    # Freeze top row
    ws.freeze_panes = 'A2'

    # Basic alignment for data rows
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.alignment = Alignment(vertical='top', horizontal='left', wrap_text=False)

    autosize_columns(ws)

    # Create META sheet and copy META cols AS-IS
    meta_ws = wb.create_sheet(META_SHEET_NAME)

    # Write META headers
    for c, key in enumerate(META_COLS, start=1):
        cell = meta_ws.cell(row=1, column=c, value=key)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align

    # Fill META values per row (aligned with Data order)
    for r, rec in enumerate(items, start=2):
        for c, key in enumerate(META_COLS, start=1):
            meta_ws.cell(row=r, column=c, value=rec.get(key, ''))

    # Very hidden meta sheet
    meta_ws.sheet_state = 'veryHidden'

    # Rename Data -> TestPlan and normalize columns (remove META, reorder MAIN)
    ws.title = MAIN_SHEET_NAME

    # Build a map of header to column index
    headers = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column+1)]
    header_to_idx = {h: i for i, h in enumerate(headers, start=1)}

    # Determine visible main columns in order (must exist in headers)
    main_cols_idx = [header_to_idx.get(h) for h in MAIN_ORDER if h in header_to_idx]

    # Rebuild the sheet with only MAIN_ORDER columns
    new_data = []
    # Headers first
    new_data.append([h for h in MAIN_ORDER])
    # Rows
    for r in range(2, ws.max_row+1):
        row_vals = []
        for idx in main_cols_idx:
            row_vals.append(ws.cell(row=r, column=idx).value)
        new_data.append(row_vals)

    # Clear and rewrite
    ws.delete_rows(1, ws.max_row)
    for r, row_vals in enumerate(new_data, start=1):
        for c, v in enumerate(row_vals, start=1):
            cell = ws.cell(row=r, column=c, value=v)
            if r == 1:
                cell.font = header_font
                cell.fill = header_fill
                cell.alignment = header_align

    # Wrap text for specified columns; alignments
    headers2 = [ws.cell(row=1, column=i).value for i in range(1, ws.max_column+1)]
    colname_to_idx = {h: i for i, h in enumerate(headers2, start=1)}

    for r in range(2, ws.max_row+1):
        for name, idx in colname_to_idx.items():
            cell = ws.cell(row=r, column=idx)
            wrap = name in WRAP_COLS
            halign = 'left'
            if name == 'Index':
                halign = 'center'
            cell.alignment = Alignment(wrap_text=wrap, vertical='top', horizontal=halign)

    autosize_columns(ws)

    # Enforce numbering for steps/criteria using META raw values
    meta_headers = [meta_ws.cell(row=1, column=i).value for i in range(1, meta_ws.max_column+1)]
    meta_col_idx = {h: i for i, h in enumerate(meta_headers, start=1)}

    tp_idx = colname_to_idx.get('Test Steps / Procedure')
    va_idx = colname_to_idx.get('Validation / Acceptance Criteria')

    for r in range(2, ws.max_row+1):
        # r aligns with same row in META (since both started at 2)
        raw_steps = meta_ws.cell(row=r, column=meta_col_idx['Hidden_Test_Steps_Procedure']).value if 'Hidden_Test_Steps_Procedure' in meta_col_idx else ''
        raw_val = meta_ws.cell(row=r, column=meta_col_idx['Hidden_Validation_Acceptance_Criteria']).value if 'Hidden_Validation_Acceptance_Criteria' in meta_col_idx else ''
        if tp_idx:
            ws.cell(row=r, column=tp_idx, value=normalize_numbering(raw_steps))
        if va_idx:
            ws.cell(row=r, column=va_idx, value=normalize_numbering(raw_val))

    # Row heights based on wrapped content lines
    wrap_cols_indices = {colname_to_idx[n] for n in WRAP_COLS if n in colname_to_idx}
    for r in range(2, ws.max_row+1):
        max_lines = 1
        for c in wrap_cols_indices:
            txt = ws.cell(row=r, column=c).value or ''
            lines = str(txt).count('\n') + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = min(15 * max_lines, 200)

    # Borders for all cells
    apply_borders(ws)

    # Freeze top row (again, after rebuild)
    ws.freeze_panes = 'A2'

    # Data validation on Code Generation (Required / Not)
    cg_col = colname_to_idx.get('Code Generation (Required / Not)')
    if cg_col:
        from openpyxl.utils import get_column_letter
        col_letter = get_column_letter(cg_col)
        dv = DataValidation(type='list', formula1='"' + ','.join(ALLOWED_CG) + '"', allow_blank=True, showErrorMessage=True)
        ws.add_data_validation(dv)
        dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Safety check: ensure no sheet named 'Data'
    if DATA_SHEET_NAME in wb.sheetnames:
        # Attempt to delete
        del wb[DATA_SHEET_NAME]
        if DATA_SHEET_NAME in wb.sheetnames:
            raise RuntimeError("Validation failed: residual 'Data' sheet present after normalization")

    # Return workbook
    return wb


def validate_xlsx(path: str) -> bool:
    if not zipfile.is_zipfile(path):
        return False
    with zipfile.ZipFile(path, 'r') as z:
        names = set(z.namelist())
        required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
        return required.issubset(names)


def main():
    # Phase 1: JSON validation and normalization
    if not isinstance(RAW_JSON, dict) or not RAW_JSON:
        raise SystemExit("Invalid or empty JSON input")

    items, schema = build_rows_from_json(RAW_JSON)

    # Phase 2: Build workbook with strict formatting rules
    wb = create_workbook(items, schema)

    # Phase 3: Save and validate
    ist = to_ist_now()
    fname = f"GPIO_TestPlan_{ist.strftime('%Y%m%d')}_{ist.strftime('%H%M%S')}.xlsx"
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_path = os.path.join(OUTPUT_DIR, fname)
    wb.save(out_path)

    ok = validate_xlsx(out_path)
    if not ok:
        raise SystemExit("XLSX validation failed")

    # Print minimal key=value outputs for GitHub Actions parser
    print(f"GENERATED_FILENAME={fname}")
    print(f"IST_TIMESTAMP={ist.strftime('%Y-%m-%d %H:%M:%S')}")


if __name__ == '__main__':
    main()
