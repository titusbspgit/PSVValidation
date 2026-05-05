#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import re
from datetime import datetime
from zoneinfo import ZoneInfo
from pathlib import Path
import zipfile

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded Test Plan JSON (macros already replaced)
JSON_PAYLOAD = {
  "metadata": {
    "ip_name": "GPIO",
    "source_repo": "titusbspgit/PSVValidation",
    "branch": "main",
    "subdirectory": "TestRepo/gpio"
  },
  "tests": {
    "TC1": {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "AHB 32-bit register interface.",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "The test checks default register values and verifies masked write/read behavior across GPIO pin and group registers using predefined masks and skip controls.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "During default reads, the input value can become high unless a value is driven; forcing zero causes a selection to become high, which can break expected reads.",
      "Test Steps / Procedure": "1) Verify default values for all targeted registers using read masks and skip controls.\n2) For each of six data patterns, write masked values to all writable and non-skipped registers.\n3) Read back masked values and compare against the computed expected value using read and write masks and default values.\n4) Report a failure if any default value or write/read comparison mismatches are observed.\n5) Report overall pass only if both default value checks and write/read checks have zero failures.",
      "Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": "- Each readable and non-skipped register’s default read value (after masking off the least significant bit) must equal its expected default value; otherwise the default-failure counter increments.\n- For each data pattern, each readable and writable non-skipped register’s readback (masked) must equal the expected value derived from the pattern, masks, and defaults; otherwise the write-failure counter increments.\n- Overall PASS if and only if both failure counters remain zero at the end; otherwise FAIL.",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "Default value and masked write/read verification across GPIO pin and group registers using arrays: addr_array[49], default_value_array[49], read_mask_array[49], write_mask_array[49], and skip controls. The test performs: chk_rst_val() then chk_rd_wr(), and finishes with 0 on success or 1 on any failure.",
      "Hidden_Remarks": "when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
      "Hidden_Test_Steps_Procedure": "1) Call chk_rst_val():\n   - Loop i=0..CNT-1 (CNT=49). Set addr=addr_array[i]. If skip_rst_array[i]==1, continue.\n   - If read_mask_array[i]==0x00000000, continue.\n   - Read data_rd=read_reg(addr). Compute data = (data_rd & 0xfffffffe).\n   - If data == default_value_array[i]: PASS (optional debug print). Else: def_fail_cnt++ and print mismatch message with addr, expected, read.\n2) Call chk_rd_wr():\n   - Initialize chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}.\n   - For each j=0..5: set data_wr = chk_val[j].\n     a) Write loop i=0..CNT-1: addr=addr_array[i]. If skip_array[i]==1 continue.\n        - If write_mask_array[i]==0x00000000 continue.\n        - Else write_reg(addr, (data_wr & write_mask_array[i])).\n     b) Read/compare loop i=0..CNT-1: addr=addr_array[i]. If skip_array[i]==1 continue.\n        - If write_mask_array[i]==0x00000000 continue.\n        - If read_mask_array[i]==0x00000000 continue.\n        - Read data_rd = (read_reg(addr) & read_mask_array[i]).\n        - Compute wr_n = (write_mask_array[i] ^ 0xffffffff).\n        - Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n        - If data_rd == exp_val: optional debug PASS print. Else: wr_fail_cnt++ and print mismatch.\n3) End condition:\n   - If def_fail_cnt > 0 or wr_fail_cnt > 0: finish(1). Else finish(0).",
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": "Default check: For each non-skipped and readable address, (read_reg(addr) & 0xfffffffe) must equal default_value_array[i]; otherwise def_fail_cnt++ and mismatch printed. Write/read check: For each pattern and each non-skipped readable/writable address, (read_reg(addr) & read_mask) must equal ((data_wr & read_mask & write_mask) | ((~write_mask) & read_mask & default)); otherwise wr_fail_cnt++ and mismatch printed. Final pass if def_fail_cnt==0 and wr_fail_cnt==0; else fail."
    },
    "TC2": {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "Programmable interrupt generation.",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "The test enables negative-edge interrupts for GPIOs 8–39, generates per-pin falling edges, and validates input state, raw/group interrupt status, and clear operations.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "A bounded timeout is used while waiting for the interrupt; the timeout value may be adjusted to the simulation time base.",
      "Test Steps / Procedure": "1) Enable the interrupt controller line for the selected GPIO instance.\n2) Enable the system interrupt output for the selected GPIO instance.\n3) For GPIOs 8–39, set input mode and enable negative-edge detection; clear any pending raw status per pin.\n4) For each pin, clear the raw status bit for the group, enable the corresponding interrupt, arm the wait, and generate a falling edge on that pin.\n5) Wait until the interrupt is observed or the timeout expires; report a timeout failure if it does.\n6) In the interrupt handler, verify the input reads low for the serviced pin, confirm the group status bit is set, clear per-pin and group raw status, and confirm the group status clears to zero.\n7) Clear the system’s raw status for the selected GPIO instance and the interrupt controller line.",
      "Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1",
      "Validation / Acceptance Criteria": "- For each pin, an interrupt must arrive before the timeout after the falling edge; otherwise the test records a timeout failure.\n- On service, the input for the serviced pin must be low; otherwise a failure is recorded.\n- The group masked status must indicate the serviced pin; otherwise a failure is recorded.\n- After clearing per-pin and group raw status, the group masked status must read zero; otherwise a failure is recorded.\n- Overall PASS if no failures are recorded; otherwise FAIL.",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "Negative-edge interrupt enable/verify for pins 8..39. test_case(): enable GIC IRQ (87/88), enable sysreg interrupt (MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO{0|1}_INTR). Drive pad high via write_reg(0xA0243ffc, 0xffffffff). Phase1: for i=0..31, addr1 = MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) to set doe=1 (input), neie=1, iclr=1; wait_on(10). Phase2: for each i, wr_val=1<<i; clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val). Enable per-pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10). Set int_pend=1, generate falling edge: write_reg(0xA0243ffc, 0xffffffff), wait_on(30), write_reg(0xA0243ffc, ~wr_val). Poll with bounded timeout (5000) on int_pend cleared by ISR; on timeout print error and increment test_err. finish(test_err).\nDefault_IRQHandler(): local_wr=1<<i; int_pend=0; drive high via write_reg(0xA0243ffc, 0xffffffff). Read raddr=MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata=read_reg(raddr). If ((rdata & 0x1)!=0) then test_err++. If ((rdata & 0x2)!=0x0) then: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; Clear per-pin raw and keep doe=1: write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, (1<<20)|(1<<16)). Clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr). Verify group clear: if (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)!=0x0) test_err++. Clear sysreg raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR). Clear GIC IRQ (87/88). Else (raw bit not set): test_err++.",
      "Hidden_Remarks": "Bounded wait instead of infinite loop; timeout may be adjusted to simulation time base.",
      "Hidden_Test_Steps_Procedure": "1) Conditional IRQ enable: ifdef GPIO0: GIC_EnableIRQ(87); ifdef GPIO1: GIC_EnableIRQ(88).\n2) Enable system register interrupt: ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n3) Initialize pad driver state: write_reg(0xA0243ffc, 0xffffffff).\n4) Configure per-pin input/negedge/clear: for i=0..31: addr1 = MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10).\n5) For each i=0..31:\n   a) wr_val = 1u<<i.\n   b) Clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val).\n   c) Enable per-pin group1 interrupt: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10).\n   d) Arm wait: int_pend=1.\n   e) Generate falling edge: write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val).\n   f) Timeout wait: timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0) { printf timeout error; test_err++; }\n6) finish(test_err).\nISR Default_IRQHandler():\n   - local_wr = 1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff).\n   - raddr = MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata = read_reg(raddr).\n   - If ((rdata & 0x1)!=0) test_err++.\n   - If ((rdata & 0x2)!=0x0) {\n       rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n       if ((rdata_grp & local_wr) == 0) test_err++;\n       write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, (1u<<20)|(1u<<16));\n       write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);\n       rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n       if (rdata_grp != 0x0) test_err++;\n       ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87);\n       ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88);\n     } else { test_err++; }",
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "Timeout check: If int_pend remains set until timeout expires in the per-pin loop, print error and increment test_err. ISR checks: (rdata & 0x1) must be 0 after negedge; if not, test_err++. If (rdata & 0x2)!=0 then rdata_grp must include bit local_wr; if not, test_err++. After clearing per-pin and group raw, read MIZAR_GPIO_GP0_INTR1_INTR_STS1 must be 0; else test_err++. If (rdata & 0x2)==0, test_err++. Final result via finish(test_err)."
    },
    "TC3": {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "Programmable interrupt generation.",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "The test enables rising-edge interrupts across GPIOs 8–39, triggers edges sequentially, and validates group status, clear operations, and system interrupt clearing.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "The interrupt flag is declared volatile to ensure the handler’s update is observed in the wait loop. A bounded timeout is used when waiting for each interrupt.",
      "Test Steps / Procedure": "1) Enable the interrupt controller line for the selected GPIO instance.\n2) Enable the system interrupt output for the selected GPIO instance.\n3) For GPIOs 8–39, enable rising-edge detection per pin.\n4) Put GPIOs 8–39 into input mode using the group IO control registers.\n5) Enable the group interrupt for all targeted pins.\n6) For each pin, drive the pad low, arm the wait, and drive the pad high to create a rising edge; wait until the interrupt is observed or the timeout expires.\n7) In the interrupt handler, read the group status, temporarily mask the group, clear per-pin raw status for all pins, verify the group status clears to zero, clear the system’s raw status for the selected instance, and re-enable the group interrupt.",
      "Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
      "Validation / Acceptance Criteria": "- For each pin, an interrupt must arrive before the timeout after the rising edge; otherwise a timeout failure is recorded.\n- On service, the group masked status must indicate that an interrupt occurred; otherwise a failure is recorded.\n- After clearing per-pin raw status for all pins, the group masked status must read zero; otherwise a failure is recorded.\n- After clearing the system raw status, the corresponding system status must not remain set; otherwise a failure is recorded.\n- Overall PASS if no failures are recorded; otherwise FAIL.",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "Rising-edge enable across pins 8..39 and sequential trigger/verify. test_case(): conditional GIC_EnableIRQ(87/88). Enable sysreg interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0|1}_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000) (peie=1). Then write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, 0x000000FF) to set doe=1 (input). Enable group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For each i: write_reg(0xA0243ffc, 0x00000000), wait_on(10), int_pend=1, write_reg(0xA0243ffc, 0xFFFFFFFF). Poll with timeout=2000 on int_pend; on timeout print error and increment test_err; prepare for next with low drive. finish(test_err).\nDefault_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000). If ((rdata_grp & 0xffffffff)!=0) optional success print else print error and test_err++. For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + j*4, 0x00010000) (iclr=1). Verify rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) == 0x0 else test_err++. Clear sysreg raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR); read back MIZAR_LSS_SYSREG_RAW_STCR1 and if bit remains set, test_err++. Re-enable group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Clear GIC IRQ (87/88).",
      "Hidden_Remarks": "extern volatile int int_pend ensures ISR/store is observed in the polling loop.",
      "Hidden_Test_Steps_Procedure": "1) Conditional IRQ enable: ifdef GPIO0: GIC_EnableIRQ(87); ifdef GPIO1: GIC_EnableIRQ(88).\n2) Enable system register interrupt: ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n3) Configure rising-edge per pin: for i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000) (peie=1).\n4) Input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). wait_on(10).\n5) Enable group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).\n6) For each i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF). Poll with timeout=2000 on int_pend; on timeout: print error and test_err++.\n7) In Default_IRQHandler():\n   - wr_val=1<<i; int_pend=0.\n   - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000).\n   - If ((rdata_grp & 0xffffffff)==0) { print error; test_err++; }\n   - For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + j*4, 0x00010000) (iclr=1). wait_on(2).\n   - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++.\n   - ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0) test_err++.\n   - ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0) test_err++.\n   - write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). ifdef GPIO0: GIC_ClearIRQ(87); ifdef GPIO1: GIC_ClearIRQ(88).",
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "In the main loop, an interrupt must arrive before the timeout after generating a rising edge; otherwise test_err++. In the ISR, rdata_grp must be non-zero; otherwise test_err++. After writing iclr=1 for all pins, rdata_grp must read as 0x0; otherwise test_err++. After clearing MIZAR_LSS_SYSREG_RAW_STCR1 for the selected instance, the corresponding bit must not remain set; otherwise test_err++. Final result is finish(test_err)."
    }
  }
}

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLS = [
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

BLUE_FILL = PatternFill(fill_type="solid", fgColor="FF4472C4")
HEADER_FONT = Font(bold=True, color="FFFFFFFF")
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=False)
WRAP_LEFT = Alignment(horizontal="left", vertical="top", wrap_text=True)
TOP_LEFT = Alignment(horizontal="left", vertical="top", wrap_text=False)
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side(style="thin")
)


def validate_input(payload: dict) -> list:
    if not isinstance(payload, dict) or "tests" not in payload:
        raise SystemExit("Invalid JSON payload: missing 'tests'")
    tests = payload["tests"]
    if not isinstance(tests, dict) or not tests:
        raise SystemExit("Invalid JSON payload: 'tests' must be a non-empty object")
    # Convert to ordered list of records (sorted by key to be deterministic)
    rows = []
    for key in sorted(tests.keys()):
        row = tests[key]
        if not isinstance(row, dict):
            raise SystemExit(f"Invalid test entry for {key}")
        rows.append(row)
    return rows


def union_keys_preserve_first_seen(rows: list) -> list:
    seen = []
    seen_set = set()
    for row in rows:
        for k in row.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    # Ensure MAIN_COLS exist and appear in specified order at the front of the TestPlan sheet later
    for k in MAIN_COLS:
        if k not in seen_set:
            seen.append(k)
            seen_set.add(k)
    return seen


def normalize_rows(rows: list, keys: list) -> list:
    norm = []
    for r in rows:
        norm.append({k: r.get(k, "") for k in keys})
    return norm


def number_lines(value: str) -> str:
    if value is None:
        return ""
    s = str(value)
    lines = [ln.strip() for ln in s.splitlines() if ln.strip()]
    out = []
    for i, ln in enumerate(lines, start=1):
        # Remove leading markers like '1)', '1.', '-', '*', '•'
        ln = re.sub(r"^\s*(?:\d+[\.)]|[-*•])\s*", "", ln)
        out.append(f"{i}. {ln}")
    return "\n".join(out) if out else s


def autofit_columns(ws):
    max_width = {}
    for row in ws.iter_rows(values_only=False):
        for cell in row:
            if cell.value is None:
                val = ""
            else:
                val = str(cell.value)
            col = cell.column
            width = len(val)
            if width > max_width.get(col, 0):
                max_width[col] = width
    for col, width in max_width.items():
        letter = get_column_letter(col)
        # Cap and pad
        adjusted = min(100, max(10, width + 2))
        ws.column_dimensions[letter].width = adjusted


def apply_borders(ws):
    for row in ws.iter_rows():
        for cell in row:
            cell.border = THIN_BORDER


def main():
    rows = validate_input(JSON_PAYLOAD)
    keys_all = union_keys_preserve_first_seen(rows)
    norm_rows = normalize_rows(rows, keys_all)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"
    ws.freeze_panes = "A2"

    # Write headers
    for c, k in enumerate(keys_all, start=1):
        cell = ws.cell(row=1, column=c, value=k)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = BLUE_FILL

    # Write data rows exactly
    for r_idx, record in enumerate(norm_rows, start=2):
        for c, k in enumerate(keys_all, start=1):
            ws.cell(row=r_idx, column=c, value=record.get(k, ""))

    # Create Meta_data_sheet with META cols only
    ws_meta = wb.create_sheet(title="Meta_data_sheet")
    for c, k in enumerate(META_COLS, start=1):
        cell = ws_meta.cell(row=1, column=c, value=k)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = BLUE_FILL
    for r_idx, record in enumerate(norm_rows, start=2):
        for c, k in enumerate(META_COLS, start=1):
            ws_meta.cell(row=r_idx, column=c, value=record.get(k, ""))
    ws_meta.sheet_state = 'veryHidden'

    # Transform Data -> TestPlan in-place
    # Build a mapping of column indices for current Data sheet
    header_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Prepare final order values from existing data, with META removed
    final_headers = MAIN_COLS

    # Ensure Code Generation col exists; if missing, add blank during compose
    data_rows_final = []
    for r_idx in range(2, ws.max_row + 1):
        row_map = {h: "" for h in final_headers}
        for h in final_headers:
            if h in header_to_col:
                row_map[h] = ws.cell(row=r_idx, column=header_to_col[h]).value
        # Numbering rules for two columns
        for wrap_col in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
            row_map[wrap_col] = number_lines(row_map.get(wrap_col, ""))
        data_rows_final.append(row_map)

    # Clear existing Data sheet and rewrite in final order
    ws.title = "TestPlan"  # rename first to comply with rule
    ws.delete_rows(1, ws.max_row)

    # Write final headers
    for c, h in enumerate(final_headers, start=1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = HEADER_FONT
        cell.alignment = CENTER
        cell.fill = BLUE_FILL

    # Write final data
    for r_idx, rmap in enumerate(data_rows_final, start=2):
        for c, h in enumerate(final_headers, start=1):
            ws.cell(row=r_idx, column=c, value=rmap.get(h, ""))

    # Formatting: wrapping and alignment
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }
    for c, h in enumerate(final_headers, start=1):
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=c)
            if h in wrap_cols:
                cell.alignment = WRAP_LEFT
            elif h == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top")
            else:
                cell.alignment = TOP_LEFT

    # Header alignment already set; ensure freeze panes remains
    ws.freeze_panes = "A2"

    # Borders for all populated cells
    apply_borders(ws)

    # Autofit columns
    autofit_columns(ws)

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in final_headers:
        col_idx = final_headers.index("Code Generation (Required / Not)") + 1
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        start_row = 2
        end_row = ws.max_row
        col_letter = get_column_letter(col_idx)
        dv.add(f"{col_letter}{start_row}:{col_letter}{end_row}")
        ws.add_data_validation(dv)

    # Final sheet visibility check
    if any(name == "Data" for name in wb.sheetnames):
        # Attempt to delete any remaining Data sheet
        try:
            ws_data = wb["Data"]
            wb.remove(ws_data)
        except Exception:
            raise SystemExit("Validation failed: 'Data' sheet still present and could not be removed")

    # Timestamp and output path
    ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts_date = ist.strftime("%Y%m%d")
    ts_time = ist.strftime("%H%M%S")
    out_dir = Path("Test_Output/GPIO/TestPlan/")
    out_dir.mkdir(parents=True, exist_ok=True)
    filename = f"GPIO_TestPlan_{ts_date}_{ts_time}.xlsx"
    out_path = out_dir / filename

    # Save workbook
    wb.save(out_path.as_posix())

    # Validate as ZIP-based OOXML
    with zipfile.ZipFile(out_path.as_posix(), 'r') as zf:
        assert '[Content_Types].xml' in zf.namelist()
        assert 'xl/workbook.xml' in zf.namelist()
    # Validate open and structure
    _ = load_workbook(out_path.as_posix())

    print(f"CREATED: {out_path.as_posix()}")


if __name__ == "__main__":
    main()
