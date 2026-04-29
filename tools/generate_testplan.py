#!/usr/bin/env python3
# coding: utf-8

"""
Deterministic fallback automation to generate a formatted Excel (.xlsx) TestPlan from embedded JSON input.

Strictly adheres to rules:
- Create staging sheet named 'Data' first, then reorganize the same sheet into 'TestPlan'
- Create 'Meta_data_sheet' with META columns and set very hidden
- Main sheet formatting, numbering, validation, borders, wrapping, sizing
- Enforce final sheets: only 'TestPlan' (visible) and 'Meta_data_sheet' (veryHidden)
- Save as Office Open XML .xlsx, then validate by ZIP structure and openpyxl load
- Print the generated output path as: 'OUTPUT_FILE: <path>' for workflow consumption

This script embeds the first JSON DETAILS mapping as provided and converts it to an array in order [TC1, TC2, TC3].
"""

import json
import os
import sys
import zipfile
from collections import OrderedDict
from datetime import datetime, timezone, timedelta

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# ---------- Embedded JSON (first JSON DETAILS object) ----------
JSON_INPUT_STR = r'''{ "TC1": { "Index": "1", "SS / Module": "GPIO", "Feature": "AHB 32-bit register interface", "Test Case Name": "gpio_reg_wr_rd_test", "Test Description": "Validates GPIO register default values and masked write/read behavior across a defined set of GPIO-related registers using address, read-mask, write-mask, and expected default-value arrays. Includes skip controls for specific registers and masks out bit[0] during default-value comparisons.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Comment: SKIPPING VRRW registers. Note: when reading default values the din value is becoming 1 automatically if we don't force any value, but if we force zero to din bit level sel becoming high, so that reading value not matched with expected value.", "Test Steps / Procedure": "Entry Point: test_case()\n1. Call chk_rst_val().\n 1.1 Loop initialization: i = 0.\n 1.2 Loop condition: for (i < CNT) where CNT = 49.\n 1.3 Body for each i:\n - Set addr = addr_array[i]. addr_array contains register macros: MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4.\n - If skip_rst_array[i] == 1: continue (skip this address for default read check).\n - If read_mask_array[i] == 0x00000000: continue (not readable).\n - Register access: READ from addr using read_reg(addr). Capture data_rd.\n - Compute data = (data_rd & 0xFFFFFFFE).\n - Compare: if (data == default_value_array[i]) then PASS for this address; else increment def_fail_cnt.\n 1.4 Loop update: i++ and repeat until i == CNT (exit condition true).\n2. Call chk_rd_wr().\n 2.1 Initialize chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}.\n 2.2 Outer loop over patterns: for j = 0 to 5.\n - Set data_wr = chk_val[j].\n - Phase: Write to all writable registers.\n a) Loop i = 0 to CNT-1:\n - addr = addr_array[i].\n - If skip_array[i] == 1: continue (skip this address for writes).\n - If write_mask_array[i] == 0x00000000: continue (not writable).\n - Register access: WRITE to addr value (data_wr & write_mask_array[i]) using write_reg(addr, (data_wr & write_mask_array[i]) ).\n - Phase: Read-back and verify.\n b) Loop i = 0 to CNT-1:\n - addr = addr_array[i].\n - If skip_array[i] == 1: continue (skip this address for read-back verification).\n - If write_mask_array[i] == 0x00000000: continue (write not supported, skip read-back).\n - If read_mask_array[i] == 0x00000000: continue (not readable, skip read-back).\n - Register access: READ from addr using read_reg(addr), then mask: data_rd = (read_reg(addr) & read_mask_array[i]).\n - Compute wr_n = (write_mask_array[i] ^ 0xFFFFFFFF).\n - Compute expected value: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) ).\n - Compare: if (data_rd == exp_val) then PASS for this address; else increment wr_fail_cnt.\n 2.3 Exit outer loop when j > 5 (after all 6 patterns complete).\n3. Final decision:\n - If (def_fail_cnt > 0) OR (wr_fail_cnt > 0): finish(1).\n - Else: finish(0).\nTiming:\n- No explicit delay/wait in this test path (wait_on usage is only inside a disabled function soft_reset_chk()).", "Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4", "Validation / Acceptance Criteria": "Default-value check: For each i where skip_rst_array[i] == 0 and read_mask_array[i] != 0, the expression (read_reg(addr_array[i]) & 0xFFFFFFFE) must equal default_value_array[i]. Write/read check: For each pattern j and address i where skip_array[i] == 0, write_mask_array[i] != 0, and read_mask_array[i] != 0, the masked read-back value (read_reg(addr_array[i]) & read_mask_array[i]) must equal exp_val computed as ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xFFFFFFFF) & read_mask_array[i] & default_value_array[i])). Test passes if def_fail_cnt == 0 and wr_fail_cnt == 0, resulting in finish(0); otherwise finish(1).", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test", "Hidden_Test_Description": "Validates GPIO register default values and masked write/read behavior across a defined set of GPIO-related registers using address, read-mask, write-mask, and expected default-value arrays. Includes skip controls for specific registers and masks out bit[0] during default-value comparisons.", "Hidden_Remarks": "Comment: SKIPPING VRRW registers. Note: when reading default values the din value is becoming 1 automatically if we don't force any value, but if we force zero to din bit level sel becoming high, so that reading value not matched with expected value.", "Hidden_Test_Steps_Procedure": "Entry Point: test_case()\n1. Call chk_rst_val().\n 1.1 Loop initialization: i = 0.\n 1.2 Loop condition: for (i < CNT) where CNT = 49.\n 1.3 Body for each i:\n - Set addr = addr_array[i]. addr_array contains register macros: MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4.\n - If skip_rst_array[i] == 1: continue (skip this address for default read check).\n - If read_mask_array[i] == 0x00000000: continue (not readable).\n - Register access: READ from addr using read_reg(addr). Capture data_rd.\n - Compute data = (data_rd & 0xFFFFFFFE).\n - Compare: if (data == default_value_array[i]) then PASS for this address; else increment def_fail_cnt.\n 1.4 Loop update: i++ and repeat until i == CNT (exit condition true).\n2. Call chk_rd_wr().\n 2.1 Initialize chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}.\n 2.2 Outer loop over patterns: for j = 0 to 5.\n - Set data_wr = chk_val[j].\n - Phase: Write to all writable registers.\n a) Loop i = 0 to CNT-1:\n - addr = addr_array[i].\n - If skip_array[i] == 1: continue (skip this address for writes).\n - If write_mask_array[i] == 0x00000000: continue (not writable).\n - Register access: WRITE to addr value (data_wr & write_mask_array[i]) using write_reg(addr, (data_wr & write_mask_array[i]) ).\n - Phase: Read-back and verify.\n b) Loop i = 0 to CNT-1:\n - addr = addr_array[i].\n - If skip_array[i] == 1: continue (skip this address for read-back verification).\n - If write_mask_array[i] == 0x00000000: continue (write not supported, skip read-back).\n - If read_mask_array[i] == 0x00000000: continue (not readable, skip read-back).\n - Register access: READ from addr using read_reg(addr), then mask: data_rd = (read_reg(addr) & read_mask_array[i]).\n - Compute wr_n = (write_mask_array[i] ^ 0xFFFFFFFF).\n - Compute expected value: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) ).\n - Compare: if (data_rd == exp_val) then PASS for this address; else increment wr_fail_cnt.\n 2.3 Exit outer loop when j > 5 (after all 6 patterns complete).\n3. Final decision:\n - If (def_fail_cnt > 0) OR (wr_fail_cnt > 0): finish(1).\n - Else: finish(0).\nTiming:\n- No explicit delay/wait in this test path (wait_on usage is only inside a disabled function soft_reset_chk()).", "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4", "Hidden_Validation_Acceptance_Criteria": "Default-value check: For each i where skip_rst_array[i] == 0 and read_mask_array[i] != 0, the expression (read_reg(addr_array[i]) & 0xFFFFFFFE) must equal default_value_array[i]. Write/read check: For each pattern j and address i where skip_array[i] == 0, write_mask_array[i] != 0, and read_mask_array[i] != 0, the masked read-back value (read_reg(addr_array[i]) & read_mask_array[i]) must equal exp_val computed as ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xFFFFFFFF) & read_mask_array[i] & default_value_array[i])). Test passes if def_fail_cnt == 0 and wr_fail_cnt == 0, resulting in finish(0); otherwise finish(1)." }, "TC2": { "Index": "2", "SS / Module": "GPIO", "Feature": "Interrupts based on positive edge or negative edge or level high or level low detection at GPIO input", "Test Case Name": "test_gpio_negedge_intr_en", "Test Description": "Configures GPIO[8..39] as inputs with negative-edge interrupt detection, drives pad states to generate a falling edge per GPIO, waits with timeout for an interrupt, and services the interrupt by validating per-pin input level, raw and group status, and clearing both GPIO and system-level interrupt status.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "Uses a bounded wait loop with a timeout to avoid infinite hang while waiting for the interrupt. The interrupt source (GPIO0 or GPIO1) is selected via compile-time macros and corresponding GIC IDs. A known pad driver state is established before and after the edge generation.", "Test Steps / Procedure": "Entry Point: test_case()\n1. Initialize test_err = 0.\n2. Conditional: If targeting GPIO0 instance, enable its GIC interrupt (ID 87). If targeting GPIO1 instance, enable its GIC interrupt (ID 88).\n3. Enable system-register interrupt output for the selected GPIO instance by writing to MIZAR_LSS_SYSREG_INTR_EN1 with the corresponding enable bit (LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Operation: WRITE.\n4. Drive the pad driver to a known state (all high). Operation: WRITE address 0xA0243ffc value 0xFFFFFFFF.\n5. Configure per-pin control registers for GPIO[8..39] as input and enable negative-edge detection, and clear latched raw status per pin.\n 5.1 Loop: for i = 0..31\n - Compute addr1 = (MIZAR_GPIO_GP0_GPIO_8 + i4).\n - Operation: WRITE addr1 with value ((1<<20) | (1<<18) | (1<<16)) to set input mode (doe=1), negedge enable (neie=1), and raw clear (iclr=1).\n - Timing: wait_on(10) after each write.\n6. For each pin, enable only that pin, generate a falling edge, and wait with timeout for the interrupt.\n 6.1 Loop: for i = 0..31\n - Set wr_val = (1u << i).\n - Pre-clear group raw status for this bit. Operation: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 with wr_val.\n - Enable only this bit at group enable. Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with wr_val.\n - Timing: wait_on(10).\n - Arm the interrupt wait flag: int_pend = 1.\n - Generate falling edge on the corresponding pad bit:\n a) Operation: WRITE 0xA0243ffc with 0xFFFFFFFF (ensure high).\n b) Timing: wait_on(30).\n c) Operation: WRITE 0xA0243ffc with bitwise NOT of wr_val to drive only this bit low (falling edge on bit i).\n - Bounded wait for ISR to clear int_pend:\n a) Initialize timeout = 5000.\n b) Loop: while (int_pend && timeout--) { wait_on(10); }\n c) If timeout reached zero: log timeout error and increment test_err.\n7. Finalize: finish(test_err).\nInterrupt Handler Entry: Default_IRQHandler()\n8. On interrupt, clear wait flag and restore pad driver to known state.\n - Compute local_wr = (1u << i) from current index i.\n - Set int_pend = 0.\n - Operation: WRITE 0xA0243ffc with 0xFFFFFFFF to return pads high.\n9. Read back per-pin control/status and validate input state for falling edge.\n - Compute raddr = (MIZAR_GPIO_GP0_GPIO_8 + i4).\n - Operation: READ raddr into rdata.\n - Check: if (rdata & 0x1) != 0 then increment test_err (DIN should be 0 after falling edge).\n10. Validate that per-pin raw interrupt latched and group status reflects the bit.\n - Condition: if ((rdata & 0x2) != 0x0) then proceed, else increment test_err.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp.\n - Check: if ((rdata_grp & local_wr) == 0) then increment test_err.\n11. Clear per-pin raw and group raw status, verify clear.\n - Operation: WRITE raddr with ((1<<20) | (1<<16)) to keep input mode and clear per-pin raw.\n - Operation: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 with local_wr.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp; if (rdata_grp != 0x0) then increment test_err.\n12. Clear system-level raw status and corresponding GIC interrupt for the active GPIO instance.\n - If GPIO0 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR; then GIC_ClearIRQ(87).\n - If GPIO1 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO1_INTR; then GIC_ClearIRQ(88).\nTiming Summary:\n- wait_on(10) per per-pin configuration and after enabling intr for each pin.\n- wait_on(30) between driving high and low to form the falling edge.\n- Bounded wait loop per pin with timeout initialized to 5000 and inner wait_on(10).", "Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1", "Validation / Acceptance Criteria": "For each pin (8..39): after generating a falling edge, an interrupt must occur before the timeout expires (int_pend cleared by ISR). In the interrupt service, the per-pin input bit must read low, the raw status must indicate the event, the corresponding group status bit must be set, and after clearing per-pin and group raw status, the group status must be zero. The system-level raw status must be cleared for the active instance. The test passes if no timeouts occur and no validation increments test_err; finish(test_err) completes with zero.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en", "Hidden_Test_Description": "Configures GPIO[8..39] as inputs with negative-edge interrupt detection, drives pad states to generate a falling edge per GPIO, waits with timeout for an interrupt, and services the interrupt by validating per-pin input level, raw and group status, and clearing both GPIO and system-level interrupt status.", "Hidden_Remarks": "Uses a bounded wait loop with a timeout to avoid infinite hang while waiting for the interrupt. The interrupt source (GPIO0 or GPIO1) is selected via compile-time macros and corresponding GIC IDs. A known pad driver state is established before and after the edge generation.", "Hidden_Test_Steps_Procedure": "Entry Point: test_case()\n1. Initialize test_err = 0.\n2. Conditional: If targeting GPIO0 instance, enable its GIC interrupt (ID 87). If targeting GPIO1 instance, enable its GIC interrupt (ID 88).\n3. Enable system-register interrupt output for the selected GPIO instance by writing to MIZAR_LSS_SYSREG_INTR_EN1 with the corresponding enable bit (LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Operation: WRITE.\n4. Drive the pad driver to a known state (all high). Operation: WRITE address 0xA0243ffc value 0xFFFFFFFF.\n5. Configure per-pin control registers for GPIO[8..39] as input and enable negative-edge detection, and clear latched raw status per pin.\n 5.1 Loop: for i = 0..31\n - Compute addr1 = (MIZAR_GPIO_GP0_GPIO_8 + i4).\n - Operation: WRITE addr1 with value ((1<<20) | (1<<18) | (1<<16)) to set input mode (doe=1), negedge enable (neie=1), and raw clear (iclr=1).\n - Timing: wait_on(10) after each write.\n6. For each pin, enable only that pin, generate a falling edge, and wait with timeout for the interrupt.\n 6.1 Loop: for i = 0..31\n - Set wr_val = (1u << i).\n - Pre-clear group raw status for this bit. Operation: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 with wr_val.\n - Enable only this bit at group enable. Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with wr_val.\n - Timing: wait_on(10).\n - Arm the interrupt wait flag: int_pend = 1.\n - Generate falling edge on the corresponding pad bit:\n a) Operation: WRITE 0xA0243ffc with 0xFFFFFFFF (ensure high).\n b) Timing: wait_on(30).\n c) Operation: WRITE 0xA0243ffc with bitwise NOT of wr_val to drive only this bit low (falling edge on bit i).\n - Bounded wait for ISR to clear int_pend:\n a) Initialize timeout = 5000.\n b) Loop: while (int_pend && timeout--) { wait_on(10); }\n c) If timeout reached zero: log timeout error and increment test_err.\n7. Finalize: finish(test_err).\nInterrupt Handler Entry: Default_IRQHandler()\n8. On interrupt, clear wait flag and restore pad driver to known state.\n - Compute local_wr = (1u << i) from current index i.\n - Set int_pend = 0.\n - Operation: WRITE 0xA0243ffc with 0xFFFFFFFF to return pads high.\n9. Read back per-pin control/status and validate input state for falling edge.\n - Compute raddr = (MIZAR_GPIO_GP0_GPIO_8 + i4).\n - Operation: READ raddr into rdata.\n - Check: if (rdata & 0x1) != 0 then increment test_err (DIN should be 0 after falling edge).\n10. Validate that per-pin raw interrupt latched and group status reflects the bit.\n - Condition: if ((rdata & 0x2) != 0x0) then proceed, else increment test_err.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp.\n - Check: if ((rdata_grp & local_wr) == 0) then increment test_err.\n11. Clear per-pin raw and group raw status, verify clear.\n - Operation: WRITE raddr with ((1<<20) | (1<<16)) to keep input mode and clear per-pin raw.\n - Operation: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 with local_wr.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp; if (rdata_grp != 0x0) then increment test_err.\n12. Clear system-level raw status and corresponding GIC interrupt for the active GPIO instance.\n - If GPIO0 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR; then GIC_ClearIRQ(87).\n - If GPIO1 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO1_INTR; then GIC_ClearIRQ(88).\nTiming Summary:\n- wait_on(10) per per-pin configuration and after enabling intr for each pin.\n- wait_on(30) between driving high and low to form the falling edge.\n- Bounded wait loop per pin with timeout initialized to 5000 and inner wait_on(10).", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "For each pin (8..39): after generating a falling edge, an interrupt must occur before the timeout expires (int_pend cleared by ISR). In the interrupt service, the per-pin input bit must read low, the raw status must indicate the event, the corresponding group status bit must be set, and after clearing per-pin and group raw status, the group status must be zero. The system-level raw status must be cleared for the active instance. The test passes if no timeouts occur and no validation increments test_err; finish(test_err) completes with zero." }, "TC3": { "Index": "3", "SS / Module": "GPIO", "Feature": "Interrupts based on positive edge or negative edge or level high or level low detection at GPIO input", "Test Case Name": "test_gpio_pedge_all_pads_en", "Test Description": "Enables positive-edge interrupt detection on GPIO[8..39], configures them as inputs, enables all group interrupts, then drives a single rising edge per pin and waits with timeout for the interrupt; the interrupt handler validates group status, clears per-pin raw status for all pins, verifies clear, and clears the system-level interrupt.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "Uses a volatile interrupt-pending flag to synchronize between the loop and the interrupt handler. Masks group interrupt during service and re-enables after clearing. Bounded wait loop prevents infinite hang.", "Test Steps / Procedure": "Entry Point: test_case()\n1. If targeting GPIO0 instance, enable its GIC interrupt (ID 87); if targeting GPIO1 instance, enable its GIC interrupt (ID 88).\n2. Enable system-register interrupt output for the selected instance by writing MIZAR_LSS_SYSREG_INTR_EN1 with the appropriate enable bit. Operation: WRITE.\n3. Configure GPIO[8..39] for positive-edge detection.\n - Loop i = 0..31: Operation: WRITE (MIZAR_GPIO_GP0_GPIO_8 + i4) with 0x00020000 to enable posedge interrupt per pin.\n4. Set groups to input mode using IO control group registers.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP1 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP2 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP3 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP4 with 0x000000FF.\n - Timing: wait_on(10) after configuration.\n5. Enable group interrupt for all pins.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0xFFFFFFFF.\n6. For each pin, generate a rising edge and wait for interrupt.\n - Loop i = 0..31:\n a) Prepare level low: Operation: WRITE 0xA0243ffc with 0x00000000; Timing: wait_on(10).\n b) Arm: set int_pend = 1.\n c) Generate rising edge: Operation: WRITE 0xA0243ffc with 0xFFFFFFFF.\n d) Bounded wait: timeout = 2000; while (int_pend == 1 and --timeout > 0) { wait_on(10); } If timeout == 0, log error, increment test_err, and break.\n e) Optional post-step: Operation: WRITE 0xA0243ffc with 0x00000000; Timing: wait_on(10).\n7. Finalize: finish(test_err).\nInterrupt Handler Entry: Default_IRQHandler()\n8. Compute wr_val = (1 << i); set int_pend = 0.\n9. Read group interrupt status.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp.\n10. Mask group during service.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0x00000000.\n11. Validate group interrupt occurred: if ((rdata_grp & 0xFFFFFFFF) == 0) then log error and increment test_err.\n12. Clear per-pin raw status for all pins.\n - Loop j = 0..31: Operation: WRITE (MIZAR_GPIO_GP0_GPIO_8 + j4) with 0x00010000 to write-one-to-clear per-pin raw.\n - Timing: wait_on(2).\n13. Verify group status cleared.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp; if (rdata_grp != 0x0) then log error and increment test_err.\n14. Clear system-level raw status and verify cleared.\n - If GPIO0 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR; then READ MIZAR_LSS_SYSREG_RAW_STCR1 into rdata; if (rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0 then increment test_err.\n - If GPIO1 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO1_INTR; then READ MIZAR_LSS_SYSREG_RAW_STCR1 into rdata; if (rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0 then increment test_err.\n15. Re-enable group interrupts for next iteration.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0xFFFFFFFF.\n16. Clear GIC interrupt for the selected instance.\n - If GPIO0: GIC_ClearIRQ(87). If GPIO1: GIC_ClearIRQ(88).\nTiming Summary:\n- wait_on(10) after configuration and between pad transitions; wait_on(2) after raw clear loop; bounded polling with timeout 2000 and inner wait_on(10).", "Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1", "Validation / Acceptance Criteria": "For each pin (8..39) a rising edge must result in an interrupt before the timeout expires. In the interrupt service, group status must indicate an active bit; after issuing per-pin raw clears across all pins and a brief wait, group status must read as zero. The system-level raw status must be cleared and verified as cleared. Group interrupt output is re-enabled after service. The test passes if no timeout occurs and no validation increments the error counter.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en", "Hidden_Test_Description": "Enables positive-edge interrupt detection on GPIO[8..39], configures them as inputs, enables all group interrupts, then drives a single rising edge per pin and waits with timeout for the interrupt; the interrupt handler validates group status, clears per-pin raw status for all pins, verifies clear, and clears the system-level interrupt.", "Hidden_Remarks": "Uses a volatile interrupt-pending flag to synchronize between the loop and the interrupt handler. Masks group interrupt during service and re-enables after clearing. Bounded wait loop prevents infinite hang.", "Hidden_Test_Steps_Procedure": "Entry Point: test_case()\n1. If targeting GPIO0 instance, enable its GIC interrupt (ID 87); if targeting GPIO1 instance, enable its GIC interrupt (ID 88).\n2. Enable system-register interrupt output for the selected instance by writing MIZAR_LSS_SYSREG_INTR_EN1 with the appropriate enable bit. Operation: WRITE.\n3. Configure GPIO[8..39] for positive-edge detection.\n - Loop i = 0..31: Operation: WRITE (MIZAR_GPIO_GP0_GPIO_8 + i4) with 0x00020000 to enable posedge interrupt per pin.\n4. Set groups to input mode using IO control group registers.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP1 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP2 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP3 with 0x000000FF.\n - Operation: WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP4 with 0x000000FF.\n - Timing: wait_on(10) after configuration.\n5. Enable group interrupt for all pins.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0xFFFFFFFF.\n6. For each pin, generate a rising edge and wait for interrupt.\n - Loop i = 0..31:\n a) Prepare level low: Operation: WRITE 0xA0243ffc with 0x00000000; Timing: wait_on(10).\n b) Arm: set int_pend = 1.\n c) Generate rising edge: Operation: WRITE 0xA0243ffc with 0xFFFFFFFF.\n d) Bounded wait: timeout = 2000; while (int_pend == 1 and --timeout > 0) { wait_on(10); } If timeout == 0, log error, increment test_err, and break.\n e) Optional post-step: Operation: WRITE 0xA0243ffc with 0x00000000; Timing: wait_on(10).\n7. Finalize: finish(test_err).\nInterrupt Handler Entry: Default_IRQHandler()\n8. Compute wr_val = (1 << i); set int_pend = 0.\n9. Read group interrupt status.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp.\n10. Mask group during service.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0x00000000.\n11. Validate group interrupt occurred: if ((rdata_grp & 0xFFFFFFFF) == 0) then log error and increment test_err.\n12. Clear per-pin raw status for all pins.\n - Loop j = 0..31: Operation: WRITE (MIZAR_GPIO_GP0_GPIO_8 + j4) with 0x00010000 to write-one-to-clear per-pin raw.\n - Timing: wait_on(2).\n13. Verify group status cleared.\n - Operation: READ MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp; if (rdata_grp != 0x0) then log error and increment test_err.\n14. Clear system-level raw status and verify cleared.\n - If GPIO0 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR; then READ MIZAR_LSS_SYSREG_RAW_STCR1 into rdata; if (rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0 then increment test_err.\n - If GPIO1 path: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO1_INTR; then READ MIZAR_LSS_SYSREG_RAW_STCR1 into rdata; if (rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0 then increment test_err.\n15. Re-enable group interrupts for next iteration.\n - Operation: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 with 0xFFFFFFFF.\n16. Clear GIC interrupt for the selected instance.\n - If GPIO0: GIC_ClearIRQ(87). If GPIO1: GIC_ClearIRQ(88).", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "For each pin (8..39) a rising edge must result in an interrupt before the timeout expires. In the interrupt service, group status must indicate an active bit; after issuing per-pin raw clears across all pins and a brief wait, group status must read as zero. The system-level raw status must be cleared and verified as cleared. Group interrupt output is re-enabled after service. The test passes if no timeout occurs and no validation increments the error counter." } }'''
# ----------------------------------------------------------------

# Constants
IST = timezone(timedelta(hours=5, minutes=30))
OUTPUT_DIR = os.path.join('Test_Output', 'GPIO', 'TestPlan')
MAIN_COLUMNS = [
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
META_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria'
]
EXPECTED_TOTAL_COLS = 20

def parse_and_validate_json():
    try:
        data_map = json.loads(JSON_INPUT_STR, object_pairs_hook=OrderedDict)
    except Exception as e:
        print(f"ERROR: Invalid JSON input: {e}", file=sys.stderr)
        sys.exit(2)
    if not isinstance(data_map, (dict, OrderedDict)) or len(data_map) == 0:
        print("ERROR: JSON must be a non-empty object with TC entries", file=sys.stderr)
        sys.exit(2)
    # Convert to array rows preserving order TC1, TC2, ... by insertion order
    rows = []
    for k, v in data_map.items():
        if not isinstance(v, (dict, OrderedDict)):
            print(f"ERROR: Each TC entry must be an object; got {type(v)} for {k}", file=sys.stderr)
            sys.exit(2)
        rows.append(OrderedDict(v))
    if not rows:
        print("ERROR: No rows after parsing", file=sys.stderr)
        sys.exit(2)
    # Build union of keys preserving first-seen order across rows
    key_order = []
    seen = set()
    for row in rows:
        for key in row.keys():
            if key not in seen:
                seen.add(key)
                key_order.append(key)
    # Sanity: ensure our expected columns are present in union
    for col in MAIN_COLUMNS + META_COLUMNS:
        if col not in key_order:
            key_order.append(col)  # ensure presence; missing values will be blank
    if len(set(MAIN_COLUMNS + META_COLUMNS)) != EXPECTED_TOTAL_COLS:
        print("ERROR: Unexpected total columns count; expected 20 unique columns", file=sys.stderr)
        sys.exit(2)
    return rows, key_order


def approximate_autofit(ws, header_row_idx=1):
    # Determine max string length per column, set width with factor
    max_len = {}
    for row in ws.iter_rows(values_only=True):
        for idx, val in enumerate(row, start=1):
            s = '' if val is None else str(val)
            if len(s) == 0:
                continue
            max_len[idx] = max(max_len.get(idx, 0), len(s))
    for col_idx, length in max_len.items():
        col_letter = ws.cell(row=header_row_idx, column=col_idx).column_letter
        ws.column_dimensions[col_letter].width = min(120, max(12, length + 2))


def apply_borders(ws):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def set_header_style(ws, header_row_idx=1):
    header_fill = PatternFill(fill_type='solid', fgColor='4472C4')
    header_font = Font(bold=True, color='FFFFFF')
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=False)
    for cell in ws[header_row_idx]:
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill


def wrap_and_align(ws):
    # Wrap text for specified columns
    wrap_cols = {
        'Test Description',
        'Remarks',
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria',
    }
    # Map headers to column indices
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    header_to_idx = {h: i + 1 for i, h in enumerate(headers)}
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            header = headers[c - 1]
            if header in wrap_cols:
                cell.alignment = Alignment(wrap_text=True, vertical='top', horizontal='left')
            elif header == 'Index':
                cell.alignment = Alignment(wrap_text=False, vertical='top', horizontal='center')
            else:
                # Default text left, vertical top
                cell.alignment = Alignment(wrap_text=False, vertical='top', horizontal='left')
    # Approximate row height based on content lines in wrap columns
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for col_name in wrap_cols:
            cidx = header_to_idx.get(col_name)
            if cidx is None:
                continue
            val = ws.cell(row=r, column=cidx).value
            s = '' if val is None else str(val)
            lines = s.count('\n') + 1 if s else 1
            max_lines = max(max_lines, lines)
        # Approx height: 14.4 points per line
        ws.row_dimensions[r].height = min(409, max(15, 14.4 * max_lines))


def number_multiline_items_in_cell(text: str) -> str:
    if text is None:
        return ''
    s = str(text)
    # Split on newline, trim whitespace lines, number non-empty lines
    lines = s.split('\n')
    out_lines = []
    idx = 1
    for ln in lines:
        t = ln.strip()
        if t == '':
            continue
        out_lines.append(f"{idx}. {t}")
        idx += 1
    return '\n'.join(out_lines) if out_lines else ''


def main():
    rows, key_order = parse_and_validate_json()

    # Phase 1 — Generate Base Excel Workbook with 'Data'
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write headers in first-seen key order
    for col_idx, key in enumerate(key_order, start=1):
        ws.cell(row=1, column=col_idx, value=key)
    # Write rows preserving exact values, missing as blank
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(key_order, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(key, ''))

    # Base formatting
    ws.freeze_panes = 'A2'
    set_header_style(ws, header_row_idx=1)
    approximate_autofit(ws, header_row_idx=1)

    # Phase 2 — Create META Sheet
    meta_ws = wb.create_sheet(title='Meta_data_sheet')
    # Write META headers and values in specified order
    for c_idx, key in enumerate(META_COLUMNS, start=1):
        meta_ws.cell(row=1, column=c_idx, value=key)
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, key in enumerate(META_COLUMNS, start=1):
            meta_ws.cell(row=r_idx, column=c_idx, value=row.get(key, ''))
    # Very hide META sheet
    meta_ws.sheet_state = 'veryHidden'

    # Phase 2 — Normalize MAIN Sheet on the same 'Data' sheet
    # Remove META columns and reorder remaining columns to MAIN order
    # Build a lookup from header to column index for current 'Data'
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    header_to_idx = {h: i + 1 for i, h in enumerate(headers)}

    # Create a snapshot of desired data according to MAIN_COLUMNS
    data_matrix = []
    data_matrix.append(MAIN_COLUMNS)  # header row
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for key in MAIN_COLUMNS:
            cidx = header_to_idx.get(key)
            val = ws.cell(row=r, column=cidx).value if cidx else ''
            row_vals.append(val if val is not None else '')
        data_matrix.append(row_vals)

    # Clear the 'Data' sheet completely and rewrite only MAIN columns
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)

    for r_idx, row_vals in enumerate(data_matrix, start=1):
        for c_idx, val in enumerate(row_vals, start=1):
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Rename 'Data' to 'TestPlan' (no new visible sheet must be created)
    ws.title = 'TestPlan'

    # STRICT formatting for TestPlan
    set_header_style(ws, header_row_idx=1)
    ws.freeze_panes = 'A2'

    # Number items inside specified multi-line columns
    header_map = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    for r in range(2, ws.max_row + 1):
        for col_name in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
            cidx = header_map.get(col_name)
            if cidx is None:
                continue
            cell = ws.cell(row=r, column=cidx)
            cell.value = number_multiline_items_in_cell(cell.value)

    # Wrap, align, borders, sizing
    wrap_and_align(ws)
    approximate_autofit(ws, header_row_idx=1)
    apply_borders(ws)

    # Data Validation for 'Code Generation (Required / Not)' ONLY on data rows
    codegen_col_idx = header_map.get('Code Generation (Required / Not)')
    if codegen_col_idx is not None and ws.max_row >= 2:
        col_letter = ws.cell(row=1, column=codegen_col_idx).column_letter
        dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=False, showDropDown=True)
        # Apply to data rows only
        dv_range = f"{col_letter}2:{col_letter}{ws.max_row}"
        dv.add(dv_range)
        ws.add_data_validation(dv)

    # Enforce final visibility: ensure only TestPlan (visible) and Meta_data_sheet (veryHidden)
    # Delete any sheet named 'Data' if still present
    for sht in list(wb.sheetnames):
        if sht == 'Data':
            del wb[sht]
    # Ensure only allowed sheets exist
    for sht in list(wb.sheetnames):
        if sht not in ('TestPlan', 'Meta_data_sheet'):
            # Remove any unexpected sheets
            del wb[sht]

    # Phase 3 — Save Final Excel File
    now_ist = datetime.now(IST)
    fname = f"GPIO_TestPlan_{now_ist.strftime('%Y%m%d_%H%M%S')}.xlsx"
    out_dir = OUTPUT_DIR
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)

    # Validate saved file as a true XLSX (ZIP with required parts)
    try:
        with zipfile.ZipFile(out_path, 'r') as zf:
            # Required parts
            required = {'[Content_Types].xml', 'xl/workbook.xml'}
            names = set(zf.namelist())
            missing = [p for p in required if p not in names]
            if missing:
                raise RuntimeError(f"Missing OOXML parts: {missing}")
        # Attempt to open with openpyxl
        _ = load_workbook(out_path, data_only=True)
    except Exception as e:
        print(f"ERROR: XLSX validation failed: {e}", file=sys.stderr)
        sys.exit(3)

    print(f"OUTPUT_FILE: {out_path}")


if __name__ == '__main__':
    main()
