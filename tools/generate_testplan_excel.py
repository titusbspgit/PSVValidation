#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Deterministic Stage1 fallback: Convert provided Test Plan JSON to Excel (.xlsx) with strict formatting rules
and commit-ready output. This script is intended to be run by GitHub Actions on main branch.

Rules implemented:
- Create Data sheet from JSON union-schema, preserving first-seen key order
- Create Meta_data_sheet with META columns; set sheet to Very Hidden
- Rename Data -> TestPlan; remove META columns; reorder columns to MAIN order
- Apply formatting only to TestPlan: header bold + blue fill, borders, wraps, alignments
- Add data validation drop-down on 'Code Generation (Required / Not)' column
- Compute IST timestamp and save to Test_Output/GPIO/TestPlan/GPIO_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx

No mutation of data values; arrays are newline-joined as-is.
"""

import json
import os
from datetime import datetime, timezone, timedelta
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ------------------------
# Embedded JSON input
# ------------------------
TEST_PLAN_JSON_STR = r'''{
  "ip": "GPIO",
  "source_repo": "titusbspgit/PSVValidation",
  "branch": "main",
  "source_subdirectory": "TestRepo/gpio",
  "test_cases": [
    {
      "Index": 1,
      "SS / Module": "GPIO",
      "Feature": "AHB 32-bit register interface",
      "Test Case Name": "gpio_reg_wr_rd_test/",
      "Test Description": "Verify default values and masked write/read behavior for GPIO per-pin and group registers via the programming interface.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "VRRW locations are skipped as encoded by the skip arrays. Reading default input status may not match expected unless inputs are driven; input forcing can alter selection behavior.",
      "Test Steps / Procedure": [
        "Start by checking default register values for each per-pin control from GPIO_GP0_GPIO_8 through GPIO_GP0_GPIO_39 and included group registers. For each address in the test list, skip if flagged by the reset-skip mask or if not readable; otherwise read the register using the read mask and compare against the expected default.",
        "Iterate over a fixed set of test data patterns. For each pattern, traverse all test registers in order; skip locations marked as non-writable; otherwise write the masked pattern to the register.",
        "After the write phase for a given pattern, traverse all test registers again; skip non-writable or non-readable locations; read the register value using the read mask.",
        "For each read value, derive the expected value by combining written bits on writable fields with the default value for non-writable fields, restricted by the read mask; compare read value with the expected result.",
        "Accumulate failures for any default mismatch or write/read mismatch; complete the test by reporting pass when no failures are recorded, otherwise fail."
      ],
      "Impacted Registers": "GPIO_GP0_GPIO_8, GPIO_GP0_GPIO_9, GPIO_GP0_GPIO_10, GPIO_GP0_GPIO_11, GPIO_GP0_GPIO_12, GPIO_GP0_GPIO_13, GPIO_GP0_GPIO_14, GPIO_GP0_GPIO_15, GPIO_GP0_GPIO_16, GPIO_GP0_GPIO_17, GPIO_GP0_GPIO_18, GPIO_GP0_GPIO_19, GPIO_GP0_GPIO_20, GPIO_GP0_GPIO_21, GPIO_GP0_GPIO_22, GPIO_GP0_GPIO_23, GPIO_GP0_GPIO_24, GPIO_GP0_GPIO_25, GPIO_GP0_GPIO_26, GPIO_GP0_GPIO_27, GPIO_GP0_GPIO_28, GPIO_GP0_GPIO_29, GPIO_GP0_GPIO_30, GPIO_GP0_GPIO_31, GPIO_GP0_GPIO_32, GPIO_GP0_GPIO_33, GPIO_GP0_GPIO_34, GPIO_GP0_GPIO_35, GPIO_GP0_GPIO_36, GPIO_GP0_GPIO_37, GPIO_GP0_GPIO_38, GPIO_GP0_GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GP0_INTR2_INTR_EN1, GPIO_GP0_INTR2_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GPIO_IO_CTRL_GROUP4, GPIO_GPIO_DOUT_GROUP1, GPIO_GPIO_DOUT_GROUP2, GPIO_GPIO_DOUT_GROUP3, GPIO_GPIO_DOUT_GROUP4, GPIO_GPIO_DIN_GROUP1, GPIO_GPIO_DIN_GROUP2, GPIO_GPIO_DIN_GROUP3, GPIO_GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": [
        "Default value check passes when each readable register’s masked read equals its masked default.",
        "Write/read check passes when, for each exercised register and data pattern, the masked readback equals the combination of written bits on writable fields and default bits on non-writable fields.",
        "Overall pass when no mismatches are detected across all iterations."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test/",
      "Hidden_Test_Description": "program.c performs two phases: chk_rst_val() to verify defaults and chk_rd_wr() to verify masked write/read using arrays from test_define.c (addr_array, default_value_array, read_mask_array, write_mask_array, skip_array, skip_rst_array). Fail counts (def_fail_cnt, wr_fail_cnt) accumulate and finish(0/1) indicates pass/fail.",
      "Hidden_Remarks": [
        "test_define.c comment: \"//80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers\"",
        "test_define.c comment: \"//when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value\""
      ],
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "Call chk_rst_val()",
        "Loop i=0..CNT-1: addr = addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); if (data == default_value_array[i]) PASS else { def_fail_cnt++; printf failure }",
        "Call chk_rd_wr()",
        "Set chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}",
        "For each pattern j=0..5: data_wr = chk_val[j]",
        "Write phase: loop i=0..CNT-1: addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; write_reg(addr, (data_wr & write_mask_array[i]))",
        "Read/compare phase: loop i=0..CNT-1: addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; if (read_mask_array[i]==0) continue; data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) PASS else { wr_fail_cnt++; printf failure }",
        "If (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0)"
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": [
        "chk_rst_val(): if ((read_reg(addr) & 0xfffffffe) == default_value_array[i]) pass; else def_fail_cnt++ and print mismatch",
        "chk_rd_wr(): data_rd = (read_reg(addr) & read_mask_array[i]); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); compare data_rd == exp_val; else wr_fail_cnt++ and print mismatch",
        "finish(0) if def_fail_cnt==0 and wr_fail_cnt==0; otherwise finish(1)"
      ]
    },
    {
      "Index": 2,
      "SS / Module": "GPIO",
      "Feature": "neie",
      "Test Case Name": "test_gpio_negedge_intr_en/",
      "Test Description": "Configure GPIO pins as inputs with falling-edge interrupt enabled, route the interrupt to the system controller, generate falling edges, and verify per-pin raw and group status, input level, and interrupt clearing.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "Interrupt wait is armed before generating the edge to avoid a race. A bounded wait with a timeout is used; the timeout value may require adjustment to the simulation time base. All pads are initially driven high to establish a known state.",
      "Test Steps / Procedure": [
        "Enable the platform interrupt for the selected instance at the interrupt controller.",
        "Enable routing of the GPIO interrupt at the system controller register.",
        "Drive the external pad control location to all ones to establish a high level on the pins.",
        "Configure per-pin control registers from GPIO_GP0_GPIO_8 through GPIO_GP0_GPIO_39 for input mode, enable falling-edge interrupt, and clear any latched raw status; include a small wait after each configuration.",
        "For each pin index from 0 to 31, first clear any pending raw group status for that bit at the raw status/clear register.",
        "Enable the interrupt for the current pin at the group enable register and wait briefly.",
        "Arm the wait flag prior to stimulus.",
        "Generate a falling edge for the selected bit by driving the external pad control high, wait briefly, then drive it low for that bit.",
        "Poll on the wait flag with a bounded retry and inter-iteration delay; if the flag remains set upon timeout, record an error and proceed.",
        "Upon interrupt service, return the pad drive to the high state, read the per-pin control register for the serviced index, and verify the input level is low after a falling edge.",
        "Verify the per-pin raw condition by checking the per-pin register and then the group masked status register; confirm the group bit is set for the serviced pin.",
        "Clear per-pin raw status at the per-pin control register and clear the corresponding group raw status; read the group status to confirm it is cleared.",
        "Clear the routed interrupt at the system controller register and clear the platform interrupt for the selected instance.",
        "After iterating all pins, complete the test and report pass if no errors were recorded."
      ],
      "Impacted Registers": "LSS_SYSREG_INTR_EN1, LSS_SYSREG_RAW_STCR1, GPIO_GP0_GPIO_8, GPIO_GP0_GPIO_9, GPIO_GP0_GPIO_10, GPIO_GP0_GPIO_11, GPIO_GP0_GPIO_12, GPIO_GP0_GPIO_13, GPIO_GP0_GPIO_14, GPIO_GP0_GPIO_15, GPIO_GP0_GPIO_16, GPIO_GP0_GPIO_17, GPIO_GP0_GPIO_18, GPIO_GP0_GPIO_19, GPIO_GP0_GPIO_20, GPIO_GP0_GPIO_21, GPIO_GP0_GPIO_22, GPIO_GP0_GPIO_23, GPIO_GP0_GPIO_24, GPIO_GP0_GPIO_25, GPIO_GP0_GPIO_26, GPIO_GP0_GPIO_27, GPIO_GP0_GPIO_28, GPIO_GP0_GPIO_29, GPIO_GP0_GPIO_30, GPIO_GP0_GPIO_31, GPIO_GP0_GPIO_32, GPIO_GP0_GPIO_33, GPIO_GP0_GPIO_34, GPIO_GP0_GPIO_35, GPIO_GP0_GPIO_36, GPIO_GP0_GPIO_37, GPIO_GP0_GPIO_38, GPIO_GP0_GPIO_39, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GPIO_INTR_RAW_STCLR1",
      "Validation / Acceptance Criteria": [
        "For each pin, the wait flag is cleared by the interrupt service within the timeout window.",
        "After service of a falling edge, the per-pin input level observed via the per-pin register indicates a low state.",
        "The group masked status reflects the serviced pin during the event and reads as cleared after the clear sequence.",
        "The system controller routed interrupt raw status reads as cleared after writing the clear register.",
        "Overall pass when no timeouts or status mismatches are observed."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en/",
      "Hidden_Test_Description": "program.c enables GIC (87/88) based on instance and routes interrupt via MIZAR_LSS_SYSREG_INTR_EN1. It sets external drive 0xA0243ffc=0xffffffff, then for i=0..31 configures MIZAR_GPIO_GP0_GPIO_8+(i*4) with doe=1, neie=1, iclr=1. For each i: clears MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 for bit, enables MIZAR_GPIO_GP0_INTR1_INTR_EN1 bit, arms int_pend=1, generates a 1->0 transition via 0xA0243ffc writes, waits for int_pend to clear with timeout. Default_IRQHandler() drives 0xffffffff, reads per-pin reg, checks DIN==0 and that per-pin raw/IRS indicates event, checks MIZAR_GPIO_GP0_INTR1_INTR_STS1 bit, clears per-pin iclr and group RAW_STCLR1, verifies group status cleared, clears MIZAR_LSS_SYSREG_RAW_STCR1 and GIC, else increments test_err. finish(test_err).",
      "Hidden_Remarks": [
        "Comment: \"// Drive all high initially (known state)\"",
        "Comment: \"// Arm the wait BEFORE generating the edge to avoid race\"",
        "Comment: \"// Bounded wait instead of infinite loop\"",
        "Comment: \"// adjust to your sim time base if needed\""
      ],
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "#ifdef GPIO0: GIC_EnableIRQ(87); #ifdef GPIO1: GIC_EnableIRQ(88);",
        "#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);",
        "#ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);",
        "write_reg(0xA0243ffc, 0xffffffff);",
        "for (i=0;i<32;i++): addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10);",
        "for (i=0;i<32;i++): wr_val = (1u<<i); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { printf timeout; test_err++; }",
        "finish(test_err)",
        "ISR Entry: Default_IRQHandler()",
        "local_wr = (1u<<i); int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata = read_reg(raddr);",
        "if ((rdata & 0x1) != 0) test_err++;",
        "if ((rdata & 0x2) != 0x0) { rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; #ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1",
      "Hidden_Validation_Acceptance_Criteria": [
        "Polling loop: while (int_pend && timeout--) wait_on(10); timeout==0 implies error",
        "In ISR: if ((rdata & 0x1) != 0) -> error (DIN should be 0 for negedge)",
        "In ISR: if ((rdata & 0x2) != 0x0) { rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr) == 0) error; } else error",
        "After clearing per-pin iclr and group RAW_STCLR1: if (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) != 0x0) error",
        "System controller clear: write MIZAR_LSS_SYSREG_RAW_STCR1 with instance bit and clear GIC line"
      ]
    },
    {
      "Index": 3,
      "SS / Module": "GPIO",
      "Feature": "peie",
      "Test Case Name": "test_gpio_pedge_all_pads_en/",
      "Test Description": "Enable rising-edge interrupt on all GPIO pins, configure pins as inputs, route the interrupt, generate rising edges, and validate group status behavior, interrupt clearing, and routed status.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "The wait flag is set before generating the stimulus to avoid missing the event. A bounded wait with a timeout is used in the polling loop. Group interrupt is masked during service and re-enabled afterward.",
      "Test Steps / Procedure": [
        "Enable the platform interrupt line for the selected instance at the interrupt controller.",
        "Enable routing of the GPIO interrupt at the system controller register.",
        "Configure per-pin control registers from GPIO_GP0_GPIO_8 through GPIO_GP0_GPIO_39 to enable rising-edge detection.",
        "Configure group input mode via the group IO control registers so the pins operate as inputs.",
        "Enable group interrupt for the pins at the group enable register.",
        "For each pin index from 0 to 31, ensure the external drive is low, arm the wait flag, then create a rising edge by driving the external location high.",
        "Poll the wait flag with a bounded timeout and inter-iteration delay; on timeout, record an error and stop iterating.",
        "After each event, optionally drive the external control low again and wait briefly.",
        "During interrupt service, read the group masked status register and temporarily mask the group enable to avoid re-entry.",
        "Verify the group status indicates that at least one pin triggered; report an error if not set.",
        "Clear per-pin raw status for all pins at their per-pin control registers using the write-one-to-clear field.",
        "Verify the group masked status register reads zero after the clear sequence.",
        "Clear the routed interrupt at the system controller register and verify the routed status bit is cleared by reading it back.",
        "Re-enable the group interrupt and clear the platform interrupt line for the selected instance."
      ],
      "Impacted Registers": "LSS_SYSREG_INTR_EN1, LSS_SYSREG_RAW_STCR1, GPIO_GP0_GPIO_8, GPIO_GP0_GPIO_9, GPIO_GP0_GPIO_10, GPIO_GP0_GPIO_11, GPIO_GP0_GPIO_12, GPIO_GP0_GPIO_13, GPIO_GP0_GPIO_14, GPIO_GP0_GPIO_15, GPIO_GP0_GPIO_16, GPIO_GP0_GPIO_17, GPIO_GP0_GPIO_18, GPIO_GP0_GPIO_19, GPIO_GP0_GPIO_20, GPIO_GP0_GPIO_21, GPIO_GP0_GPIO_22, GPIO_GP0_GPIO_23, GPIO_GP0_GPIO_24, GPIO_GP0_GPIO_25, GPIO_GP0_GPIO_26, GPIO_GP0_GPIO_27, GPIO_GP0_GPIO_28, GPIO_GP0_GPIO_29, GPIO_GP0_GPIO_30, GPIO_GP0_GPIO_31, GPIO_GP0_GPIO_32, GPIO_GP0_GPIO_33, GPIO_GP0_GPIO_34, GPIO_GP0_GPIO_35, GPIO_GP0_GPIO_36, GPIO_GP0_GPIO_37, GPIO_GP0_GPIO_38, GPIO_GP0_GPIO_39, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GPIO_IO_CTRL_GROUP4",
      "Validation / Acceptance Criteria": [
        "For each pin event, the wait flag clears within the timeout window.",
        "During service, the group masked status indicates an active interrupt and reads as cleared after the per-pin clear loop.",
        "After clearing the routed interrupt, the routed status bit is not set on readback.",
        "Test passes if no timeouts or status mismatches occur."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en/",
      "Hidden_Test_Description": "program.c enables GIC (87/88) and routes the interrupt via MIZAR_LSS_SYSREG_INTR_EN1. It writes MIZAR_GPIO_GP0_GPIO_8+(i*4) = 0x00020000 (peie) for i=0..31, sets MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4 = 0x000000FF (input), enables MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF, then loops i=0..31: write 0xA0243ffc=0x0, wait, int_pend=1, write 0xA0243ffc=0xFFFFFFFF; poll int_pend with timeout; drive low again. Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) success else error; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8+(j*4); verify group status cleared; clear MIZAR_LSS_SYSREG_RAW_STCR1 (GPIO0/1) and confirm readback bit clears; re-enable MIZAR_GPIO_GP0_INTR1_INTR_EN1; clear GIC.",
      "Hidden_Remarks": [
        "Comment: \"// enable posedge interrupt (bit17=1) per pin\"",
        "Comment: \"// Put GPIOs 8-39 in input mode (doe=1)\"",
        "Comment: \"// Wait with timeout to avoid infinite hangs\"",
        "Comment: \"// mask group during service\"",
        "Comment: \"// Re-enable group interrupt output for next iteration\""
      ],
      "Hidden_Test_Steps_Procedure": [
        "Entry: test_case()",
        "#ifdef GPIO0: GIC_EnableIRQ(87); #ifdef GPIO1: GIC_EnableIRQ(88);",
        "#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);",
        "#ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);",
        "for (i=0;i<32;i++): write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000);",
        "wait_on(10);",
        "write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);",
        "wait_on(10);",
        "write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);",
        "for (i=0;i<32;i++): write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) { printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10);",
        "finish(test_err)",
        "ISR Entry: Default_IRQHandler()",
        "wr_val=(1<<i); int_pend=0;",
        "rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & (0xffffffff)) != 0) { /* success */ } else { printf error; test_err++; }",
        "for (j=0;j<32;j++): write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2);",
        "rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { /* success */ } else { printf error; test_err++; }",
        "#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf not cleared; test_err++; } #endif",
        "#ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { printf not cleared; test_err++; } #endif",
        "write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); #ifdef GPIO0: GIC_ClearIRQ(87); #endif #ifdef GPIO1: GIC_ClearIRQ(88); #endif"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": [
        "Polling loop: while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) error",
        "Group status in ISR: if ((rdata_grp & (0xffffffff)) != 0) success else error",
        "After clearing per-pin iclr for all pins: if (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) == 0x0) success else error",
        "System controller clear verification: read MIZAR_LSS_SYSREG_RAW_STCR1 after write; if bit remains set, error",
        "Test passes if test_err remains 0 on finish()"
      ]
    }
  ]
}'''

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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

BLUE_FILL = PatternFill(fill_type="solid", start_color="4472C4", end_color="4472C4")
THIN_BORDER = Border(
    left=Side(style="thin"), right=Side(style="thin"), top=Side(style="thin"), bottom=Side("thin")
)


def newline_join(value: Any) -> Any:
    if isinstance(value, list):
        return "\n".join(str(x) for x in value)
    return value


def union_keys_order(rows: List[Dict[str, Any]]) -> List[str]:
    order: List[str] = []
    for row in rows:
        for k in row.keys():
            if k not in order:
                order.append(k)
    return order


def autosize_columns(ws, header_order: List[str]):
    # approximate width based on max length per column including header and all lines
    for col_idx, key in enumerate(header_order, start=1):
        max_len = len(str(key))
        for r in range(2, ws.max_row + 1):
            v = ws.cell(row=r, column=col_idx).value
            if v is None:
                continue
            s = str(v)
            for line in s.split("\n"):
                if len(line) > max_len:
                    max_len = len(line)
        # clamp for readability
        width = min(max(10, max_len + 2), 80)
        # compute Excel column letters properly
        n = col_idx
        letters = ""
        while n:
            n, rem = divmod(n - 1, 26)
            letters = chr(65 + rem) + letters
        ws.column_dimensions[letters].width = width


def apply_formatting(ws, header_order: List[str]):
    # Header styles
    for c in range(1, len(header_order) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = BLUE_FILL
        cell.border = THIN_BORDER

    # Data cell borders and alignment
    wrap_idx = {header_order.index(n) + 1 for n in header_order if n in WRAP_COLS}
    for r in range(2, ws.max_row + 1):
        for c in range(1, len(header_order) + 1):
            cell = ws.cell(row=r, column=c)
            # default: text left, vertical top
            horiz = "left"
            if header_order[c - 1] == "Index":
                horiz = "center"
            cell.alignment = Alignment(horizontal=horiz, vertical="top", wrap_text=(c in wrap_idx))
            cell.border = THIN_BORDER

    autosize_columns(ws, header_order)


def add_codegen_datavalidation(ws, last_row: int, col_letter: str):
    dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True)
    ws.add_data_validation(dv)
    dv.add(f"{col_letter}2:{col_letter}{last_row}")


def build_workbook(data: Dict[str, Any]) -> str:
    test_cases: List[Dict[str, Any]] = data.get("test_cases", [])
    if not isinstance(test_cases, list) or not test_cases:
        raise SystemExit("ERROR: test_cases is missing or empty in JSON input")

    # Normalize rows (newline-join arrays) without mutating source structure order
    norm_rows: List[Dict[str, Any]] = []
    for tc in test_cases:
        norm = {}
        for k, v in tc.items():
            norm[k] = newline_join(v)
        norm_rows.append(norm)

    # Determine union schema order based on first appearance
    schema_order = union_keys_order(norm_rows)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write Data sheet with union schema
    ws.append(schema_order)
    for row in norm_rows:
        ws.append([row.get(k, "") for k in schema_order])

    # Freeze header row
    ws.freeze_panes = "A2"

    # Create Meta_data_sheet with META_COLS
    meta_ws = wb.create_sheet("Meta_data_sheet")
    meta_ws.append(META_COLS)
    for row in norm_rows:
        meta_ws.append([row.get(k, "") for k in META_COLS])
    # Very hidden
    meta_ws.sheet_state = "veryHidden"

    # Prepare TestPlan main sheet from Data: remove META columns and enforce MAIN_COLS order
    # Rebuild the Data sheet in-place as TestPlan with only MAIN_COLS
    ws.delete_rows(1, ws.max_row)
    ws.append(MAIN_COLS)
    for row in norm_rows:
        ws.append([row.get(k, "") for k in MAIN_COLS])

    # Rename to TestPlan and apply formatting
    ws.title = "TestPlan"
    ws.freeze_panes = "A2"

    apply_formatting(ws, MAIN_COLS)

    # Add data validation to column N (14th) for "Code Generation (Required / Not)"
    add_codegen_datavalidation(ws, ws.max_row, "N")

    # Compute IST timestamp and save
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    out_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
    os.makedirs(out_dir, exist_ok=True)
    out_name = f"GPIO_TestPlan_{now_ist.strftime('%Y%m%d_%H%M%S')}.xlsx"
    out_path = os.path.join(out_dir, out_name)

    wb.save(out_path)

    print(out_path)
    return out_path


def main():
    data = json.loads(TEST_PLAN_JSON_STR)
    build_workbook(data)


if __name__ == "__main__":
    main()
