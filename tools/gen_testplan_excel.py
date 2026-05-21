#!/usr/bin/env python3
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font

# Configuration
OUTPUT_DIR = os.path.join('Test_Output', 'GPIO', 'TestPlan')
FILENAME_PREFIX = 'testplan_'
IST = ZoneInfo('Asia/Kolkata')

# Final aggregated Test Plan JSON (embedded as deterministic input)
JSON_TEXT = r'''[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO Register Default Value and Read/Write Integrity",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validate that GPIO gp0 data registers (gp0_gpio_8 to gp0_gpio_27) hold expected default reset values (bit[0] ignored) and support masked write/read integrity. For each target register, apply multiple data patterns to writable bits, read back using read masks, and compare against expected values derived from write masks and defaults.",
    "Meta Test Description": "program.c defines test_case() which executes two phases: (1) chk_rst_val() for default value verification and (2) chk_rd_wr() for write/read integrity checks. Final status is finish(0) if no failures, else finish(1).\n- chk_rst_val(): For i in [0..CNT-1], take addr = addr_array[i]. If skip_rst_array[i]==1, continue. If read_mask_array[i]==0x00000000, continue. Read data_rd = read_reg(addr); mask as data = (data_rd & 0xfffffffe) to ignore bit[0]. Compare data against default_value_array[i]. On mismatch, increment def_fail_cnt and print details.\n- chk_rd_wr(): Define patterns chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern (j=0..5), set data_wr = chk_val[j] and perform:\n  • Write loop: For each i, addr = addr_array[i]. If skip_array[i]==1, continue. If write_mask_array[i]==0x00000000, continue. Else write_reg(addr, (data_wr & write_mask_array[i])).\n  • Read/compare loop: For each i, if skip_array[i]==1 continue; if write_mask_array[i]==0x00000000 continue; if read_mask_array[i]==0x00000000 continue; else read data_rd = (read_reg(addr) & read_mask_array[i]). Compute wr_n = (write_mask_array[i] ^ 0xffffffff). Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). If data_rd != exp_val, increment wr_fail_cnt and print details; else print pass (under DEBUG_DISPLAY).\n- soft_reset_chk(): Present but compiled out (#ifdef 0). Would write/read SOFT_RST_REG_ADDRESS with delays via wait_on(1000).\nGlobals/counters: def_fail_cnt, wr_fail_cnt track failures across both phases.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Operates on gp0_gpio_8 through gp0_gpio_27 only; default-value check ignores bit[0] to avoid false mismatch; write/read operations respect per-register write/read masks to avoid modifying RO bits; skip lists exist but are currently all zero (no addresses skipped); the soft-reset routine is disabled; note potential array-count inconsistency (CNT=49 while arrays provide 20 entries), which could limit valid indices to available array elements.",
    "Test Steps / Procedure": "1) Initialize the test and perform a default reset value check across gp0_gpio_8..gp0_gpio_27 for addresses marked readable.\n2) For each readable register, read the value and compare its value (with bit[0] ignored) against the documented default value; record any mismatch.\n3) For each of the six data patterns (FFFFFFFFh, AAAAAAAAh, 55555555h, F5F5F5F5h, A5A5A5A5h, FFFF0000h), write the pattern to each target register using its write mask (skip non-writable or skipped registers).\n4) Read back each register using its read mask and calculate the expected value considering preserved default bits where writes are masked off; compare read-back to expected.\n5) Aggregate results across all registers and patterns; report PASS if no mismatches were detected; otherwise report FAIL.",
    "Meta Test Steps / Procedure": "- Entry: test_case(): call chk_rst_val(); call chk_rd_wr(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).\n- chk_rst_val():\n  • Loop i=0..(CNT-1): addr = addr_array[i]. If (skip_rst_array[i]==1) continue. If (read_mask_array[i]==0x00000000) continue.\n  • data_rd = read_reg(addr); data = (data_rd & 0xfffffffe).\n  • If (data == default_value_array[i]) PASS (optionally print under DEBUG_DISPLAY); else def_fail_cnt++ and print failure with addr, expected, read_data, raw_data.\n- chk_rd_wr():\n  • unsigned int chk_val[6] = { 0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}.\n  • For j=0..5: data_wr = chk_val[j].\n    - Write phase (i=0..CNT-1): addr = addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; else write_reg(addr, (data_wr & write_mask_array[i])).\n    - Read/verify phase (i=0..CNT-1): if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; else {\n        data_rd = (read_reg(addr) & read_mask_array[i]);\n        wr_n = (write_mask_array[i] ^ 0xffffffff);\n        exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]));\n        if (data_rd == exp_val) PASS (optional print); else { wr_fail_cnt++; print failure with addr, expected, read }.\n      }\n- soft_reset_chk(): Disabled via #ifdef 0; had sequence to save default, write SOFT_RST_REG_DATA, wait_on(1000), restore, wait_on(1000).",
    "Impacted Registers": "gp0_gpio_8, gp0_gpio_9, gp0_gpio_10, gp0_gpio_11, gp0_gpio_12, gp0_gpio_13, gp0_gpio_14, gp0_gpio_15, gp0_gpio_16, gp0_gpio_17, gp0_gpio_18, gp0_gpio_19, gp0_gpio_20, gp0_gpio_21, gp0_gpio_22, gp0_gpio_23, gp0_gpio_24, gp0_gpio_25, gp0_gpio_26, gp0_gpio_27",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27",
    "Validation / Acceptance Criteria": "PASS if: (a) for every targeted register, the masked default read (bit[0] ignored) equals the documented default value; and (b) for each data pattern and each applicable register, the masked read-back equals the expected value derived from the write mask (written bits) and default value (masked-off bits). FAIL if any mismatch is detected.",
    "Meta Validation / Acceptance Criteria": "- Default check: For all i with read_mask_array[i] != 0 and skip_rst_array[i] == 0, (read_reg(addr_array[i]) & 0xfffffffe) must equal default_value_array[i]. Any deviation increments def_fail_cnt.\n- Write/Read check: For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000} and each i with write_mask_array[i] != 0, read_mask_array[i] != 0, and skip_array[i] == 0, data_rd = (read_reg(addr_array[i]) & read_mask_array[i]) must equal exp_val where exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])). Any deviation increments wr_fail_cnt.\n- Final decision: If (def_fail_cnt > 0 || wr_fail_cnt > 0) → finish(1); else finish(0).",
    "Code Generation (Required / Not)": "NA",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include <gpio/gpio_def.h>\n#include <gpio/gpio_offset.h>",
    "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20] = { MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27 };\nconst unsigned int default_value_array[20] = { GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL, GPIO_GP0_GPIO_11_DEFAULT_VAL, GPIO_GP0_GPIO_12_DEFAULT_VAL, GPIO_GP0_GPIO_13_DEFAULT_VAL, GPIO_GP0_GPIO_14_DEFAULT_VAL, GPIO_GP0_GPIO_15_DEFAULT_VAL, GPIO_GP0_GPIO_16_DEFAULT_VAL, GPIO_GP0_GPIO_17_DEFAULT_VAL, GPIO_GP0_GPIO_18_DEFAULT_VAL, GPIO_GP0_GPIO_19_DEFAULT_VAL, GPIO_GP0_GPIO_20_DEFAULT_VAL, GPIO_GP0_GPIO_21_DEFAULT_VAL, GPIO_GP0_GPIO_22_DEFAULT_VAL, GPIO_GP0_GPIO_23_DEFAULT_VAL, GPIO_GP0_GPIO_24_DEFAULT_VAL, GPIO_GP0_GPIO_25_DEFAULT_VAL, GPIO_GP0_GPIO_26_DEFAULT_VAL, GPIO_GP0_GPIO_27_DEFAULT_VAL };\nconst unsigned int read_mask_array[20] = { GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK, GPIO_GP0_GPIO_11_READ_MASK, GPIO_GP0_GPIO_12_READ_MASK, GPIO_GP0_GPIO_13_READ_MASK, GPIO_GP0_GPIO_14_READ_MASK, GPIO_GP0_GPIO_15_READ_MASK, GPIO_GP0_GPIO_16_READ_MASK, GPIO_GP0_GPIO_17_READ_MASK, GPIO_GP0_GPIO_18_READ_MASK, GPIO_GP0_GPIO_19_READ_MASK, GPIO_GP0_GPIO_20_READ_MASK, GPIO_GP0_GPIO_21_READ_MASK, GPIO_GP0_GPIO_22_READ_MASK, GPIO_GP0_GPIO_23_READ_MASK, GPIO_GP0_GPIO_24_READ_MASK, GPIO_GP0_GPIO_25_READ_MASK, GPIO_GP0_GPIO_26_READ_MASK, GPIO_GP0_GPIO_27_READ_MASK };\nconst unsigned int write_mask_array[20] = { GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK, GPIO_GP0_GPIO_11_WRITE_MASK, GPIO_GP0_GPIO_12_WRITE_MASK, GPIO_GP0_GPIO_13_WRITE_MASK, GPIO_GP0_GPIO_14_WRITE_MASK, GPIO_GP0_GPIO_15_WRITE_MASK, GPIO_GP0_GPIO_16_WRITE_MASK, GPIO_GP0_GPIO_17_WRITE_MASK, GPIO_GP0_GPIO_18_WRITE_MASK, GPIO_GP0_GPIO_19_WRITE_MASK, GPIO_GP0_GPIO_20_WRITE_MASK, GPIO_GP0_GPIO_21_WRITE_MASK, GPIO_GP0_GPIO_22_WRITE_MASK, GPIO_GP0_GPIO_23_WRITE_MASK, GPIO_GP0_GPIO_24_WRITE_MASK, GPIO_GP0_GPIO_25_WRITE_MASK, GPIO_GP0_GPIO_26_WRITE_MASK, GPIO_GP0_GPIO_27_WRITE_MASK };\nconst unsigned int skip_array[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };\nconst unsigned int skip_rst_array[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };\n// Local array in chk_rd_wr\nunsigned int chk_val[6] = { 0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000 };"
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "GPIO Negative Edge Interrupt Enable and Handling",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Validate negative-edge interrupt generation and handling on GPIO Group 0 pins (GPIO_8 onwards). For each pin in the group, enable the corresponding interrupt, generate a controlled negative edge via the pad drive register, and verify that the interrupt is raised, latched in the group status, and properly cleared after servicing.",
    "Meta Test Description": "program.c configures interrupt routing and exercises negative-edge interrupts across 32 GPIO pads starting from the GPIO_8 register block. Flow: (1) Optionally enable GIC IRQ 87/88 depending on target instance (GPIO0/GPIO1). (2) Enable the system-level interrupt source via INTR_EN1 for the selected GPIO instance. (3) Write 0xFFFFFFFF to pad drive register at 0xA0243ffc to set all pads high. (4) For each i in [0..31]: compute per-pad register address addr1 = (GPIO_8 base + i*4); program per-pad configuration by setting bits (20,18,16), then clear any pending raw interrupt in gpio_intr_raw_stclr1; enable the per-pad interrupt mask via INTR1_INTR_EN1 for bit i; set int_pend=1; generate a negative edge by toggling pad drive: write 0xFFFFFFFF, wait, then write bitwise complement of (1<<i) at 0xA0243ffc to drop the specific pad; poll with timeout (up to ~5000 iterations with wait_on(10)) until int_pend is cleared by ISR; on timeout, flag error. (5) Default_IRQHandler is invoked on interrupt: it clears int_pend to 0, drives pads high again (0xA0243ffc=0xFFFFFFFF), reads the per-pad register (GPIO_8 + i*4) into rdata; if rdata bit[0] is 1 (pad level high) flag error (expect low after negedge). If rdata bit[1] (edge/status) is set, read INTR1_INTR_STS1 and require that the bit corresponding to the tested pad (local_wr = 1<<i) is set; otherwise flag error. Re-program per-pad configuration (set bits 20 and 16), clear raw status for the tested pad via gpio_intr_raw_stclr1, read INTR1_INTR_STS1 again and require it to be zero; if not, flag error. Finally, clear the system raw status in RAW_STCR1 for the selected instance and clear the corresponding GIC IRQ (87 or 88). If bit[1] was not set, flag error. (6) After iterating all pads, finish(test_err).",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Requires GIC interrupt line configured for the selected GPIO instance and system-level interrupt enable in INTR_EN1. Assumes pad drive control is available at 0xA0243ffc to generate signal transitions. The test iterates pins starting at the GPIO_8 register block over 32 consecutive pads. Timeouts indicate missing or unhandled interrupts. Ensure platform routing from GPIO to GIC is active.",
    "Test Steps / Procedure": "1) Enable the system-level interrupt source for the target GPIO instance using INTR_EN1 and enable the corresponding GIC IRQ line. 2) Initialize all GPIO pads in the GPIO_8 block range for negative-edge detection and input. 3) For each pad in the block: a) Clear its raw interrupt in gpio_intr_raw_stclr1, then enable the per-pad interrupt in INTR1_INTR_EN1. b) Generate a negative edge on the selected pad using the pad drive register at 0xA0243ffc. c) Wait for the ISR to trigger; if it does not arrive within the timeout, record a failure. 4) In the ISR, verify the pad level is low and the per-pad status indicates a negative edge. Confirm the group status in INTR1_INTR_STS1 has the corresponding bit set. 5) Acknowledge and clear the interrupt: reprogram pad config as needed, clear raw status in gpio_intr_raw_stclr1, confirm INTR1_INTR_STS1 is cleared, and clear system/GIC interrupt status (RAW_STCR1 and GIC). 6) Repeat for all pads and report PASS only if all checks succeed without timeouts or mismatches.",
    "Meta Test Steps / Procedure": "- Initialization: test_err=0. Conditionally GIC_EnableIRQ(87) for GPIO0 or GIC_EnableIRQ(88) for GPIO1. Conditionally write INTR_EN1 with the corresponding GPIO interrupt enable bit for the selected instance. Write 0xFFFFFFFF to 0xA0243ffc. - Pre-configure pads: For i=0..31, addr1=(GPIO_8 + i*4); write (1<<20)|(1<<18)|(1<<16) to addr1; wait_on(10). - Per-pad sequence for i=0..31: wr_val=(1u<<i); write gpio_intr_raw_stclr1=wr_val; write INTR1_INTR_EN1=wr_val; wait_on(10); set int_pend=1; write 0xA0243ffc=0xFFFFFFFF; wait_on(30); write 0xA0243ffc=(~wr_val); poll until (int_pend==0) with timeout=5000 and wait_on(10) each loop; on timeout==0, printf timeout error and increment test_err. - Default_IRQHandler: local_wr=(1u<<i); int_pend=0; write 0xA0243ffc=0xFFFFFFFF; raddr=(GPIO_8 + i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=(GPIO_8 + i*4); write raddr2=(1u<<20)|(1u<<16); write gpio_intr_raw_stclr1=local_wr; rdata_grp=read_reg(INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++; clear RAW_STCR1 for selected instance; GIC_ClearIRQ(87 or 88); } else { test_err++; }",
    "Impacted Registers": "GPIO_8, INTR1_INTR_EN1, INTR1_INTR_STS1, gpio_intr_raw_stclr1, INTR_EN1, RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Validation / Acceptance Criteria": "PASS if every tested pad generates a negative-edge interrupt when enabled, the corresponding bit in the group status (INTR1_INTR_STS1) sets, the pad level reads low upon entry to the ISR, and both raw and group statuses fully clear after acknowledgment (gpio_intr_raw_stclr1 and RAW_STCR1). Any timeout waiting for the ISR, incorrect pad level, missing group status bit, or uncleared status results in FAIL.",
    "Meta Validation / Acceptance Criteria": "- Timeout check: If ISR does not clear int_pend before timeout expires, record failure. - Pad level check: In ISR, (rdata & 0x1) must be 0; if nonzero, failure. - Edge/status check: In ISR, (rdata & 0x2) must be nonzero; if zero, failure. - Group status set: (read_reg(INTR1_INTR_STS1) & (1<<i)) must be nonzero; else failure. - Clear sequence: After write to gpio_intr_raw_stclr1 with (1<<i), read_reg(INTR1_INTR_STS1) must be 0; else failure. - System/GIC clear: RAW_STCR1 and GIC pending state must be cleared for the selected instance.",
    "Code Generation (Required / Not)": "NA",
    "Meta Headers": "#include <stdio.h>\n#include <lss_sysreg.h>\n#include \\\"test_define.c\\\"\n#include <test_common.h>\n#include <gpio/gpio_def.h>\n#include <gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\nconst int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\nconst int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\nconst int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,};\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}"
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "GPIO Positive Edge Interrupt Enable and Handling (All Pads)",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validate positive-edge interrupt generation and servicing across GPIO pads 8–39. Configure per-pad positive-edge detection, set pads as inputs, enable the group interrupt (INTR1_INTR_EN1) and system interrupt (INTR_EN1), then generate rising edges via the pad drive register (0xA0243ffc). Verify the group interrupt status (INTR1_INTR_STS1) sets on events, clear per-pin raw status, confirm group status clears, and ensure the system RAW status (RAW_STCR1) is cleared before re-enabling interrupts.",
    "Meta Test Description": "test_case():\n- If compiled for GPIO0, enable GIC IRQ 87; if compiled for GPIO1, enable GIC IRQ 88.\n- Initialize test_err = 0.\n- Enable the system-level GPIO interrupt source by writing to INTR_EN1 with the corresponding GPIOx enable bit depending on the instance.\n- For i = 0..31: program per-pad configuration at (GPIO_8 + i*4) with 0x00020000 to enable positive-edge interrupt (bit17=1).\n- wait_on(10).\n- Configure pads 8–39 as inputs (doe=1) by writing 0x000000FF to GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, and GPIO_IO_CTRL_GROUP4.\n- wait_on(10).\n- Enable group interrupt output by writing 0xFFFFFFFF to INTR1_INTR_EN1.\n- For i = 0..31:\n  • Drive pads low: write 0x00000000 to 0xA0243ffc and wait_on(10).\n  • Arm interrupt wait: set int_pend = 1.\n  • Generate one rising edge: write 0xFFFFFFFF to 0xA0243ffc.\n  • Poll with timeout=2000: while (int_pend==1) wait_on(10); if timeout expires, print error, increment test_err, break.\n  • Optionally drive low again (0xA0243ffc=0x00000000) and wait_on(10).\n- finish(test_err).\n\nDefault_IRQHandler():\n- Compute wr_val = (1 << i); set int_pend = 0.\n- Read group status rdata_grp = read(INTR1_INTR_STS1) and mask group during service by writing 0x00000000 to INTR1_INTR_EN1.\n- If (rdata_grp & 0xFFFFFFFF) != 0: log success; else log error and increment test_err.\n- Clear per-pin raw status for all pads by writing 0x00010000 to each per-pad register (GPIO_8 + j*4) for j=0..31; wait_on(2).\n- Verify group status cleared: read rdata_grp = read(INTR1_INTR_STS1); if rdata_grp != 0x0, log error and increment test_err; else log success.\n- Clear system RAW status in RAW_STCR1 with the bit corresponding to the GPIO instance; read back RAW_STCR1 and ensure the bit is cleared; if not, increment test_err.\n- Re-enable group interrupts by writing 0xFFFFFFFF to INTR1_INTR_EN1.\n- Clear the GIC IRQ (87 for GPIO0 or 88 for GPIO1).",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Requires functional interrupt routing from GPIO to the GIC via INTR_EN1; pad drive at 0xA0243ffc must be accessible to generate edges; all pads 8–39 are treated uniformly; group interrupt is masked during ISR to avoid re-entrancy; verification depends on INTR1_INTR_STS1 and RAW_STCR1 clearing behavior; timeouts indicate missing or stalled interrupt handling.",
    "Test Steps / Procedure": "1) Enable the GIC IRQ line for the target GPIO instance and enable the system-level GPIO interrupt in INTR_EN1. 2) Configure GPIO pads 8–39 for positive-edge detection using their per-pad registers (starting at GPIO_8) and set these pads to input using GPIO_IO_CTRL_GROUP1..4. 3) Enable the GPIO group interrupt via INTR1_INTR_EN1. 4) For each pad index (0–31 relative to GPIO_8), drive the pads low, arm the wait flag, then drive high to produce a single rising edge using the pad drive register at 0xA0243ffc. 5) Wait for the ISR to run; if it does not complete before timeout, record a failure and stop. 6) In the ISR, confirm the group interrupt status (INTR1_INTR_STS1) is set, clear per-pad raw status across all pads, and verify the group status clears. 7) Clear the system RAW status in RAW_STCR1 for the active instance and verify it is cleared; then re-enable the group interrupt and clear the GIC pending interrupt. 8) Repeat for all pads and declare PASS only if all checks succeed without timeouts or status-clear failures.",
    "Meta Test Steps / Procedure": "- Initialization: Optionally GIC_EnableIRQ(87/88) depending on instance; test_err=0; write INTR_EN1 with instance GPIO interrupt enable bit. - Pad configuration: For i=0..31, write (GPIO_8 + i*4)=0x00020000 (posedge enable); wait_on(10); write GPIO_IO_CTRL_GROUP1=0x000000FF; GPIO_IO_CTRL_GROUP2=0x000000FF; GPIO_IO_CTRL_GROUP3=0x000000FF; GPIO_IO_CTRL_GROUP4=0x000000FF; wait_on(10); write INTR1_INTR_EN1=0xFFFFFFFF. - Stimulation loop (i=0..31): write 0xA0243ffc=0x00000000; wait_on(10); int_pend=1; write 0xA0243ffc=0xFFFFFFFF; poll with timeout=2000 and wait_on(10) until int_pend==0; on timeout==0, printf error and test_err++; break; write 0xA0243ffc=0x00000000; wait_on(10). - Default_IRQHandler: wr_val=(1<<i); int_pend=0; rdata_grp=read(INTR1_INTR_STS1); write INTR1_INTR_EN1=0x00000000; if ((rdata_grp & 0xFFFFFFFF)==0) { printf error; test_err++; } For j=0..31: write (GPIO_8 + j*4)=0x00010000; wait_on(2); rdata_grp=read(INTR1_INTR_STS1); if (rdata_grp!=0) { printf error; test_err++; } Clear RAW_STCR1 (instance bit); rdata=read(RAW_STCR1); if (instance bit remains set) test_err++; write INTR1_INTR_EN1=0xFFFFFFFF; GIC_ClearIRQ(87/88).",
    "Impacted Registers": "GPIO_8 (per-pad registers 8–39), INTR1_INTR_EN1, INTR1_INTR_STS1, INTR_EN1, RAW_STCR1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
    "Validation / Acceptance Criteria": "PASS if each generated rising edge triggers the group interrupt (INTR1_INTR_STS1 reflects an active bit), the per-pin raw status clear returns the group status to zero, and the system RAW status (RAW_STCR1) is cleared before re-enabling interrupts; any timeout waiting for ISR completion, missing group status, or failure to clear group/system status results in FAIL.",
    "Meta Validation / Acceptance Criteria": "- Interrupt arrival: During stimulation loop, ISR must clear int_pend before timeout; otherwise record failure. - Group status set: In ISR, read(INTR1_INTR_STS1) must be nonzero; else failure. - Per-pin raw clear: After writing 0x00010000 to each per-pad register (GPIO_8 + j*4), read(INTR1_INTR_STS1) must be 0x0; else failure. - System RAW clear: After writing the instance bit to RAW_STCR1, read(RAW_STCR1) must show the bit cleared; else failure. - Final result: finish(test_err) where test_err==0 indicates PASS.",
    "Code Generation (Required / Not)": "NA",
    "Meta Headers": "#include <lss_sysreg.h>\n#include <stdio.h>\n#include <test_define.c>\n#include <test_common.h>\n#include <gpio/gpio_def.h>\n#include <gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\nconst unsigned int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\nconst unsigned int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\nconst unsigned int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,};\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}"
  }
]'''

TESTPLAN_COLUMNS = [
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

METADATA_COLUMNS = [
    'Index',
    'Test Case Name',
    'Meta Test Description',
    'Meta Test Steps / Procedure',
    'Meta Impacted Registers',
    'Meta Validation / Acceptance Criteria',
    'Meta Headers',
    'Meta Macros',
    'Meta Arrays'
]

def main():
    # Step 1: Validate JSON
    data = json.loads(JSON_TEXT)
    if not isinstance(data, list):
        raise SystemExit('json_data must be an array')

    # Step 2: Prepare workbook and sheets
    wb = Workbook()
    ws_plan = wb.active
    ws_plan.title = 'TestPlan'
    ws_meta = wb.create_sheet('MetaData')

    # Write headers with bold font and freeze top rows
    bold_font = Font(bold=True)
    ws_plan.append(TESTPLAN_COLUMNS)
    for cell in ws_plan[1]:
        cell.font = bold_font
    ws_plan.freeze_panes = 'A2'

    ws_meta.append(METADATA_COLUMNS)
    for cell in ws_meta[1]:
        cell.font = bold_font
    ws_meta.freeze_panes = 'A2'

    # Step 2: Split Data and write rows preserving order
    for obj in data:
        # TestPlan row
        plan_row = [obj.get(col, '') for col in TESTPLAN_COLUMNS]
        ws_plan.append(plan_row)
        # MetaData row
        meta_row = [obj.get(col, '') for col in METADATA_COLUMNS]
        ws_meta.append(meta_row)

    # Set MetaData sheet to veryHidden
    ws_meta.sheet_state = 'veryHidden'

    # Ensure output directory exists
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    # Step 4: Save file with IST timestamp
    ts = datetime.now(IST).strftime('%Y%m%d_%H%M%S')
    filename = f"{FILENAME_PREFIX}{ts}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, filename)
    wb.save(out_path)
    print(out_path)

if __name__ == '__main__':
    main()
