#!/usr/bin/env python3
import argparse
import json
import os
import sys
import zipfile
from datetime import datetime
from zoneinfo import ZoneInfo

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded TEST_PLAN_JSON (dict with TC1, TC2, ...)
TEST_PLAN_JSON = {
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO Register Reset and Masked Read/Write Verification",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verifies GPIO register default values and masked read/write behavior across the address list; failures accumulated and reported via finish().",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Explicit skips noted: VRRW registers are skipped during tests. Note from source: when reading default values the DIN value may become 1 automatically if not forced; forcing zero to DIN causes bit-level select high, leading to mismatches with expected default values.",
    "Test Steps / Procedure": "- Entry point: test_case()\n- Invoke chk_rst_val()\n  - For i=0..(CNT-1):\n    - addr = addr_array[i] (macros listed in Hidden_Impacted_Registers)\n    - If skip_rst_array[i] == 1: continue\n    - If read_mask_array[i] == 0x00000000: continue\n    - data_rd = read_reg(addr)\n    - data = (data_rd & 0xFFFFFFFE)\n    - If data == default_value_array[i]: PASS for this address; else increment def_fail_cnt and print mismatch\n- Invoke chk_rd_wr()\n  - chk_val patterns = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}\n  - For each pattern j in 0..5:\n    - data_wr = chk_val[j]\n    - Write phase (i=0..CNT-1):\n      - addr = addr_array[i]\n      - If skip_array[i] == 1: continue\n      - If write_mask_array[i] == 0x00000000: continue\n      - write_reg(addr, (data_wr & write_mask_array[i]))\n    - Read/compare phase (i=0..CNT-1):\n      - addr = addr_array[i]\n      - If skip_array[i] == 1: continue\n      - If write_mask_array[i] == 0x00000000: continue\n      - If read_mask_array[i] == 0x00000000: continue\n      - data_rd = (read_reg(addr) & read_mask_array[i])\n      - wr_n = (write_mask_array[i] ^ 0xFFFFFFFF)\n      - exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]))\n      - If data_rd == exp_val: PASS for this address/pattern; else increment wr_fail_cnt and print mismatch\n- Completion:\n  - If (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0)",
    "Impacted Registers": "GP0_GPIO_8, GP0_GPIO_9, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GP0_INTR2_INTR_EN1, GP0_INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1",
    "Validation / Acceptance Criteria": "- Default values: For each listed register where readable and not skipped, the read value (with the least significant input bit masked off) must match the documented reset value; no increments to the default-failure counter occur.\n- Masked write/read: For each listed register where writable and readable, across all data patterns, the read-back value must equal the expected value formed by the intersection of the pattern and writable+readable mask, combined with default values for non-writable bits; no increments to the write-read failure counter occur.\n- Overall pass condition: Both failure counters remain zero and completion reports success.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Default value check and write & read check across GPIO registers listed in addr_array[] using read_mask_array[]/write_mask_array[]; def_fail_cnt and wr_fail_cnt incremented on mismatches; finish(0) if both zero else finish(1).",
    "Hidden_Remarks": "Comment: //80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers\nComment: //when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
    "Hidden_Test_Steps_Procedure": "int test_case(): calls chk_rst_val(); calls chk_rd_wr(); if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0).\nvoid chk_rst_val(): for (i=0..CNT-1) { addr=addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); if (data==default_value_array[i]) {pass} else {def_fail_cnt++; printf(...);} }\nvoid chk_rd_wr(): chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; for (j=0..5) { data_wr=chk_val[j]; // write phase for (i=0..CNT-1) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; write_reg(addr,(data_wr & write_mask_array[i])); } // read phase for (i=0..CNT-1) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; if (read_mask_array[i]==0) continue; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) pass else {wr_fail_cnt++; printf(...);} } }\naddr_array macros: MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4.",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Default check: (read_reg(addr) & 0xfffffffe) == default_value_array[i] for all i where readable and not skipped. Write-read check: For all patterns and addresses where writable & readable & not skipped, (read_reg(addr) & read_mask_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); Pass if def_fail_cnt==0 && wr_fail_cnt==0 and finish(0)."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "GPIO Negative-Edge Interrupt Handling",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Configures GPIO pins for input and negative-edge detection, enables group and system interrupts, generates a falling edge per pin, waits with timeout for ISR, verifies pin input level, raw status setting and clearing at both pin and group level, and system interrupt clearing; aggregates errors and reports via finish(test_err).",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Timeout-protected wait is used to avoid infinite hangs; the wait is armed before driving the falling edge to avoid race conditions.",
    "Test Steps / Procedure": "- Entry point: test_case()\n  - Optionally enable interrupt in the interrupt controller (platform IRQ 87 or 88, depending on instance selection).\n  - Enable the system-level interrupt output for the selected instance.\n  - Drive the external pad driver register to set all outputs high as a known state.\n  - Configure each per-pin register from index 8 through 39: enable input mode, enable negative-edge detection, and clear any latched raw status; include a small wait after each configuration.\n  - For each bit position 0..31 corresponding to pins 8..39:\n    - Pre-clear the corresponding group raw status bit.\n    - Enable the corresponding group interrupt bit for that pin only; wait briefly.\n    - Arm the wait flag prior to generating the edge.\n    - Generate a falling edge on the targeted pin by first driving all-high, waiting briefly, then driving that specific pin low.\n    - Enter a bounded wait loop that sleeps between polls and exits on either the ISR clearing the wait flag or on timeout.\n    - On timeout, log an error and increment the error counter.\n  - Complete by reporting the aggregated error count.\n- Interrupt handler: Default_IRQHandler()\n  - Capture the current pin index mask using the loop index at the time of interrupt.\n  - Clear the wait flag and restore the pad driver to a known all-high state.\n  - Read the per-pin register for the active pin; confirm the input bit reflects a low level following a falling edge; on mismatch, record an error.\n  - Check that the per-pin raw interrupt indicator implies a latched event; then read the group interrupt status and confirm the corresponding bit is set; record an error if not set.\n  - Clear the per-pin latched raw condition by writing the clear control while keeping input mode enabled; also clear the corresponding group raw bit.\n  - Verify that the group status register reads as cleared; record an error if not cleared.\n  - Clear the system-level raw status bit for the selected instance and acknowledge the interrupt in the interrupt controller.",
    "Impacted Registers": "LSS_SYSREG_INTR_EN1, GP0_GPIO_8, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, LSS_SYSREG_RAW_STCR1",
    "Validation / Acceptance Criteria": "- For each pin tested, the input bit in the per-pin register must read low after the generated falling edge.\n- For each interrupt event, the corresponding group interrupt status bit must be set and then cleared successfully after the per-pin clear and the group raw clear operations.\n- The system-level raw interrupt status must be cleared successfully for the selected instance.\n- No timeout must occur during the bounded wait for any pin; overall pass requires zero accumulated errors and a successful completion status.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Sets up GPIO 8..39 for input + negedge + iclr, enables LSS sysreg interrupt, toggles external pad at 0xA0243ffc to create falling edges per bit, waits with timeout for ISR (int_pend flag), ISR validates DIN==0, GP0_INTR1_INTR_STS1 bit set/cleared, clears MIZAR_LSS_SYSREG_RAW_STCR1, aggregates test_err, finish(test_err).",
    "Hidden_Remarks": "Bounded wait is used (timeout=5000); edge wait is armed before generating the edge to avoid race; pad driver restored to known state in ISR.",
    "Hidden_Test_Steps_Procedure": "int test_case():\n#ifdef GPIO0 GIC_EnableIRQ(87); #endif\n#ifdef GPIO1 GIC_EnableIRQ(88); #endif\n#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #endif\n#ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR); #endif\nwrite_reg(0xA0243ffc, 0xffffffff);\nfor (i=0;i<32;i++){ addr1 = MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10);} \nfor (i=0;i<32;i++){\n  wr_val = 1u<<i;\n  write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);\n  write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);\n  wait_on(10);\n  int_pend = 1;\n  write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val);\n  unsigned int timeout=5000; while (int_pend && timeout--) { wait_on(10);} if (timeout==0){ printf(\"ERROR timeout negedge\\n\"); test_err++; }\n}\nfinish(test_err);\n\nvoid Default_IRQHandler():\n  unsigned int local_wr = 1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff);\n  raddr = MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata=read_reg(raddr);\n  if ((rdata & 0x1) != 0) test_err++;\n  if ((rdata & 0x2) != 0x0){ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr) == 0) test_err++; raddr2 = MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Hidden_Validation_Acceptance_Criteria": "No timeout in while(int_pend && timeout--) for all i. In ISR: (rdata & 0x1)==0; (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) & (1u<<i))!=0 before clear; after clearing per-pin and MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)==0x0; system raw cleared via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIOx_INTR). Pass if test_err==0 and finish(0)."
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "GPIO Positive-Edge Interrupt Handling (All Pads Enabled)",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Enables positive-edge detection on all target pins, configures input mode via group I/O control, enables group interrupts, then generates a rising edge per pin by toggling an external pad driver. The ISR validates group interrupt assertion and clearing, clears system-level raw status, re-enables the group, and acknowledges the interrupt controller. Errors are counted and reported via finish(test_err).",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Interrupt wait is bounded with a timeout per pin to prevent hangs; group interrupt is masked during service and re-enabled after handling.",
    "Test Steps / Procedure": "- Entry point: test_case()\n  - Optionally enable interrupt in the interrupt controller (platform IRQ 87 or 88, depending on instance selection).\n  - Enable the system-level interrupt output for the selected instance.\n  - For each per-pin register from index 8 through 39, write configuration to enable positive-edge detection.\n  - Briefly wait, then set input mode for the entire pin range via group I/O control registers.\n  - Briefly wait, then enable all group interrupt bits.\n  - For each pin index 0..31 corresponding to pins 8..39:\n    - Drive the external pad low to establish a known state; wait briefly.\n    - Arm the wait flag prior to generating the edge, then drive the external pad high to create a single rising edge.\n    - Enter a bounded wait loop that sleeps between polls and exits on either ISR or timeout. On timeout, log error, increment the error counter, and break.\n    - Optionally drive low again and wait briefly to prepare for the next iteration.\n  - Report the aggregated error status.\n- Interrupt handler: Default_IRQHandler()\n  - Compute the active bit mask from the loop index and clear the wait flag.\n  - Read the group interrupt status; mask the group interrupt output during service.\n  - If any group status bit is set, proceed; otherwise, log an error.\n  - Clear per-pin raw status for all pins in the tested range; wait briefly.\n  - Re-read and confirm the group status is fully cleared; on failure, log an error.\n  - Clear the system-level raw status for the selected instance and verify the bit is cleared; on failure, log an error.\n  - Re-enable the group interrupt output and acknowledge the interrupt controller.",
    "Impacted Registers": "LSS_SYSREG_INTR_EN1, GP0_GPIO_8, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, LSS_SYSREG_RAW_STCR1",
    "Validation / Acceptance Criteria": "- Group interrupt must assert upon a generated rising edge and then clear successfully after the per-pin clear operations.\n- The system-level raw interrupt status must be cleared and read back as cleared for the selected instance.\n- No timeout must occur while waiting for interrupts across the tested pins; overall pass requires zero accumulated errors and a successful completion status.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Positive-edge enable on GPIO 8..39; set input mode via MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4; enable all bits in MIZAR_GPIO_GP0_INTR1_INTR_EN1; per-pin loop: drive 0xA0243ffc low, arm int_pend, drive 0xA0243ffc high; wait with timeout (2000) for ISR. ISR: read MIZAR_GPIO_GP0_INTR1_INTR_STS1, mask group, if nonzero OK else error; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8 + (j*4); verify group clear; clear MIZAR_LSS_SYSREG_RAW_STCR1 (GPIO0/1), verify readback; re-enable group; clear IRQ (87/88); finish(test_err).",
    "Hidden_Remarks": "Group interrupt is masked during service and re-enabled after clearing; timeout-based wait loop prevents infinite hang.",
    "Hidden_Test_Steps_Procedure": "void test_case():\n#ifdef GPIO0 GIC_EnableIRQ(87); #endif\n#ifdef GPIO1 GIC_EnableIRQ(88); #endif\n#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #endif\n#ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR); #endif\nfor (i=0;i<32;i++){ write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000); }\nwait_on(10);\nwrite_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);\nwait_on(10);\nwrite_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);\nfor (i=0;i<32;i++){\n  write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF);\n  int timeout=2000; while ((int_pend==1) && (--timeout>0)) { wait_on(10);} if (timeout==0){ printf(\"ERROR: Timeout i=%u\\n\", i); test_err++; break; }\n  write_reg(0xA0243ffc, 0x00000000); wait_on(10);\n}\nfinish(test_err);\n\nvoid Default_IRQHandler():\n  wr_val = 1<<i; int_pend=0;\n  rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);\n  if ((rdata_grp & 0xffffffff) != 0) { /* OK */ } else { printf(\"ERROR: Group Interrupt not occured\\n\"); test_err++; }\n  for (j=0;j<32;j++){ write_reg(MIZAR_GPIO_GP0_GPIO_8 + j*4, 0x00010000);} wait_on(2);\n  rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0){ printf(\"ERROR : Group Interrupt clear failed: %x\\n\", rdata_grp); test_err++; }\n#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0) test_err++; #endif\n#ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0) test_err++; #endif\n  write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);\n#ifdef GPIO0 GIC_ClearIRQ(87); #endif\n#ifdef GPIO1 GIC_ClearIRQ(88); #endif"
  }
}

META_COLUMNS = [
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
    "Imparted Registers" if False else "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

BLUE_FILL = PatternFill(start_color="FF4F81BD", end_color="FF4F81BD", fill_type="solid")
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument('--ist-ts', required=True, help='IST timestamp in format YYYY-MM-DD HH:MM:SS IST')
    p.add_argument('--output-dir', required=True)
    p.add_argument('--ip-name', required=True)
    p.add_argument('--file-stamp', required=True, help='YYYYMMDD_HHMMSS derived from IST')
    return p.parse_args()


def ensure_array_from_tc_dict(tc_dict):
    # Convert dict of TCs (TC1, TC2, ...) into array
    ordered_keys = []
    rows = []
    for tc_key in sorted(tc_dict.keys(), key=lambda x: int(''.join(filter(str.isdigit, x)) or 1)):
        rec = tc_dict[tc_key]
        rows.append(rec)
        for k in rec.keys():
            if k not in ordered_keys:
                ordered_keys.append(k)
    return rows, ordered_keys


def normalize_rows(rows, ordered_keys):
    norm = []
    for r in rows:
        nr = {}
        for k in ordered_keys:
            nr[k] = r.get(k, "")
        norm.append(nr)
    return norm


def write_base_data_sheet(wb, rows, ordered_keys):
    ws = wb.active
    ws.title = 'Data'
    # Header
    ws.append(ordered_keys)
    # Rows
    for r in rows:
        ws.append([r[k] for k in ordered_keys])
    # Base formatting
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for c in range(1, len(ordered_keys) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = BLUE_FILL
    ws.freeze_panes = 'A2'
    # Autofilter
    ws.auto_filter.ref = ws.dimensions
    # Autofit columns (approx)
    for col_idx, key in enumerate(ordered_keys, start=1):
        max_len = len(str(key))
        for row_idx in range(2, len(rows) + 2):
            v = ws.cell(row=row_idx, column=col_idx).value
            if v is None:
                continue
            l = max(len(str(v)) for v in str(v).split('\n'))
            if l > max_len:
                max_len = l
        ws.column_dimensions[get_column_letter(col_idx)].width = min(120, max(12, int(max_len * 1.1)))
    # Borders for all populated cells
    for r in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in r:
            cell.border = THIN_BORDER
    return ws


def copy_meta_sheet(wb, data_ws, rows, ordered_keys):
    ws_meta = wb.create_sheet('Meta_data_sheet')
    # Header
    ws_meta.append(META_COLUMNS)
    # Build per-row meta in defined order
    for r in rows:
        ws_meta.append([r.get(k, "") for k in META_COLUMNS])
    # Style header
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for c in range(1, len(META_COLUMNS) + 1):
        cell = ws_meta.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = BLUE_FILL
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'
    return ws_meta


def number_items(text: str) -> str:
    if not isinstance(text, str):
        return text
    # Split on newlines; treat lines starting with '-' or ' -' or spaces as individual items
    raw_lines = [ln.strip() for ln in text.replace('\r', '').split('\n')]
    items = []
    for ln in raw_lines:
        if not ln:
            continue
        if ln.startswith('- '):
            ln = ln[2:].strip()
        items.append(ln)
    if not items:
        return text
    return '\n'.join(f"{i+1}. {itm}" for i, itm in enumerate(items))


def transform_to_testplan_in_place(wb, rows):
    ws = wb['Data']
    # Build main-ordered rows without META columns
    main_headers = MAIN_ORDER[:]
    main_data = []
    for r in rows:
        md = {}
        for k in main_headers:
            md[k] = r.get(k, "")
        # Numbering transformations for two columns
        md['Test Steps / Procedure'] = number_items(md.get('Test Steps / Procedure', ''))
        md['Validation / Acceptance Criteria'] = number_items(md.get('Validation / Acceptance Criteria', ''))
        main_data.append(md)
    # Clear and rewrite into same sheet
    ws.delete_rows(1, ws.max_row)
    ws.append(main_headers)
    for r in main_data:
        ws.append([r[k] for k in main_headers])
    # Rename sheet now
    ws.title = 'TestPlan'
    # Formatting strict
    header_font = Font(bold=True)
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=True)
    for c in range(1, len(main_headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = BLUE_FILL
    ws.freeze_panes = 'A2'
    ws.auto_filter.ref = ws.dimensions

    # Wrap text for specific columns
    wrap_cols = {'Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria'}
    col_index_map = {h: idx+1 for idx, h in enumerate(main_headers)}

    for row_idx in range(2, ws.max_row+1):
        for col_idx in range(1, ws.max_column+1):
            cell = ws.cell(row=row_idx, column=col_idx)
            # Alignments
            key = main_headers[col_idx-1]
            if key in wrap_cols:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            elif key == 'Index':
                cell.alignment = Alignment(vertical='top', horizontal='center')
            else:
                cell.alignment = Alignment(vertical='top', horizontal='left')
            cell.border = THIN_BORDER

    # Header row borders
    for cell in ws[1]:
        cell.border = THIN_BORDER

    # Approximate autofit and row height based on content lines
    for col_idx, key in enumerate(main_headers, start=1):
        max_len = len(str(key))
        for row_idx in range(2, ws.max_row+1):
            v = ws.cell(row=row_idx, column=col_idx).value
            if v is None:
                continue
            l = max(len(str(v)) for v in str(v).split('\n'))
            if l > max_len:
                max_len = l
        ws.column_dimensions[get_column_letter(col_idx)].width = min(120, max(12, int(max_len * 1.1)))

    base_height = 15
    for row_idx in range(2, ws.max_row+1):
        # Estimate number of lines across wrapped columns
        lines = 1
        for key in wrap_cols:
            col = col_index_map[key]
            v = ws.cell(row=row_idx, column=col).value
            if v:
                lines = max(lines, len(str(v).split('\n')))
        ws.row_dimensions[row_idx].height = base_height * min(10, lines)

    # Data validation on Code Generation (Required / Not)
    if 'Code Generation (Required / Not)' in col_index_map:
        cg_col = col_index_map['Code Generation (Required / Not)']
        dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        start_cell = f"{get_column_letter(cg_col)}2"
        end_cell = f"{get_column_letter(cg_col)}{ws.max_row}"
        dv.add(f"{start_cell}:{end_cell}")
        ws.add_data_validation(dv)

    # Safety: ensure no sheet named 'Data'
    if 'Data' in wb.sheetnames:
        # If exists (shouldn't), delete it
        if wb['Data'] != ws:
            wb.remove(wb['Data'])
    return ws


def validate_xlsx(path):
    # Check it is a zip and has required parts
    with zipfile.ZipFile(path, 'r') as zf:
        _ = zf.infolist()
        assert '[Content_Types].xml' in zf.namelist(), 'Missing [Content_Types].xml'
        assert 'xl/workbook.xml' in zf.namelist(), 'Missing xl/workbook.xml'
    # Try to load with openpyxl
    _ = load_workbook(path, data_only=True)


def main():
    args = parse_args()
    ist_ts_str = args.ist_ts  # e.g., 2026-04-29 18:45:12 IST
    file_stamp = args.file_stamp  # e.g., 20260429_184512

    # Build rows and schema
    rows, ordered_keys = ensure_array_from_tc_dict(TEST_PLAN_JSON)
    rows = normalize_rows(rows, ordered_keys)

    # Workbook
    wb = Workbook()
    # Set core properties timestamps
    try:
        # Parse IST timestamp to datetime
        dt = datetime.strptime(ist_ts_str.replace(' IST',''), '%Y-%m-%d %H:%M:%S').replace(tzinfo=ZoneInfo('Asia/Kolkata'))
        wb.properties.created = dt
        wb.properties.modified = dt
        wb.properties.title = f"{args.ip_name} TestPlan"
    except Exception:
        pass

    data_ws = write_base_data_sheet(wb, rows, ordered_keys)
    copy_meta_sheet(wb, data_ws, rows, ordered_keys)
    transform_to_testplan_in_place(wb, rows)

    # Ensure only TestPlan and Meta_data_sheet exist; Meta very hidden
    allowed = set(['TestPlan', 'Meta_data_sheet'])
    for sn in list(wb.sheetnames):
        if sn not in allowed:
            del wb[sn]
    # Final path and save
    out_dir = args.output_dir
    os.makedirs(out_dir, exist_ok=True)
    file_name = f"{args.ip_name}_TestPlan_{file_stamp}.xlsx"
    out_path = os.path.join(out_dir, file_name)
    wb.save(out_path)
    # Validate
    validate_xlsx(out_path)
    print(f"WROTE: {out_path}")

if __name__ == '__main__':
    main()
