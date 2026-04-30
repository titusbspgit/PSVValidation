#!/usr/bin/env python3
import json
import sys
import os
import argparse
import re
from datetime import datetime, timezone, timedelta
from zipfile import ZipFile
from io import BytesIO

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded Test Plan JSON (as provided by upstream agent)
TESTPLAN_JSON = r'''{
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO block provides an AHB interface to access configuration registers.Each register interface data width is 32-bit wide.",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verifies default reset values and masked write/read behavior across a defined set of GPIO registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "VRRW registers are explicitly skipped for write/read checks. During default read, DIN may read as 1 without forcing input; forcing DIN low can affect LISEL and expected values.",
    "Test Steps / Procedure": "1) Read the reset value of each per-pin and group GPIO register and compare against expected defaults. 2) For each data pattern, write masked values to each register and then read back with the read mask. 3) Compute the expected value from the pattern, write mask, read mask, and default values, and compare to the readback. 4) Repeat for all patterns and all registers. 5) Report pass if no mismatches are found; otherwise report fail.",
    "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, gp0_intr2, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1, INTR2_INTR_EN1, gp0_intr2, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
    "Validation / Acceptance Criteria": "- Default value check: For each address, after applying mask (0xfffffffe), the read value must equal the expected default. Any mismatch increments def_fail_cnt and fails the test.\n- Write/read check: For each pattern and address, readback (after applying read_mask) must equal ((pattern & read_mask & write_mask) | (~write_mask & read_mask & default)). Any mismatch increments wr_fail_cnt and fails the test.\n- Final result: finish(0) if def_fail_cnt == 0 and wr_fail_cnt == 0; otherwise finish(1).",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Verifies default reset values and masked write/read behavior across a defined set of GPIO registers.",
    "Hidden_Remarks": "VRRW registers are explicitly skipped for write/read checks. During default read, DIN may read as 1 without forcing input; forcing DIN low can affect LISEL and expected values.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1. Call chk_rst_val()\n   1.1 Loop i = 0..CNT-1 (CNT=49); addr = addr_array[i] (macros list below)\n       1.1.1 If skip_rst_array[i] == 1: continue (skip)\n       1.1.2 If read_mask_array[i] == 0x00000000: continue (not readable)\n       1.1.3 READ: data_rd = read_reg(addr)\n       1.1.4 MODIFY: data = (data_rd & 0xfffffffe)\n       1.1.5 COMPARE: if (data == default_value_array[i]) pass else { def_fail_cnt++ ; printf mismatch }\n2. Call chk_rd_wr()\n   2.1 Initialize chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}\n   2.2 Loop j = 0..5 (for each pattern)\n       2.2.1 data_wr = chk_val[j]\n       2.2.2 Phase WRITE: Loop i = 0..CNT-1; addr = addr_array[i]\n             2.2.2.1 If skip_array[i] == 1: continue\n             2.2.2.2 If write_mask_array[i] == 0x00000000: continue (not writable)\n             2.2.2.3 WRITE: write_reg(addr, (data_wr & write_mask_array[i]))\n       2.2.3 Phase READ/COMPARE: Loop i = 0..CNT-1; addr = addr_array[i]\n             2.2.3.1 If skip_array[i] == 1: continue\n             2.2.3.2 If write_mask_array[i] == 0x00000000: continue\n             2.2.3.3 If read_mask_array[i] == 0x00000000: continue\n             2.2.3.4 READ: data_rd = read_reg(addr) & read_mask_array[i]\n             2.2.3.5 COMPUTE: wr_n = (write_mask_array[i] ^ 0xffffffff)\n             2.2.3.6 COMPUTE: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]))\n             2.2.3.7 COMPARE: if (data_rd == exp_val) pass else { wr_fail_cnt++ ; printf mismatch }\n3. End of test_case()\n   3.1 If (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) else finish(0)\n\nAddress source (addr_array, default_value_array, read_mask_array, write_mask_array) include these macros used in reads/writes:\nMIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Default value: (read_reg(addr) & 0xfffffffe) must equal default_value_array[i], else def_fail_cnt++.\nWrite/read: data_rd = (read_reg(addr) & read_mask_array[i]) must equal exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); else wr_fail_cnt++.\nFinal: finish(0) if both def_fail_cnt and wr_fail_cnt are zero; else finish(1)."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "Independent GPIO control Register — neie",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Enables negative-edge interrupts per GPIO pin and verifies interrupt assertion, raw/group status, and clear behavior.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Wait is armed before edge generation to avoid race. ISR uses timeouts to prevent hangs; suggested to adjust to the simulation time base if needed. Conditional routing for GPIO0/ GPIO1 interrupts is controlled via build-time macros.",
    "Test Steps / Procedure": "1) Enable the interrupt controller for the GPIO instance. 2) Enable the system interrupt route for the selected GPIO instance. 3) Initialize the pad driver to a high level. 4) For each pin (8–39), configure input mode and enable negative-edge interrupt, then clear the pin raw status. 5) For each pin, clear the group raw bit, enable the corresponding interrupt bit, arm wait, and create a single falling edge on that pin. 6) Wait until the interrupt is taken or the timeout expires. 7) On interrupt, verify input is low, verify the group status bit is set for the active pin, clear the per-pin raw status and group raw bit, and verify the group status is cleared. 8) Clear the system raw status route and the interrupt controller request for the instance. 9) Report pass if all pins interrupt and status/clears behave as expected without timeouts.",
    "Impacted Registers": "GPIO_8, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1",
    "Validation / Acceptance Criteria": "- Timeout: If an interrupt does not arrive within the bounded wait, the test fails.\n- DIN after falling edge: The input value for the serviced pin must be low; otherwise the test fails.\n- Raw/group status: The per-pin raw status and the corresponding group status bit must be set on interrupt; if not, the test fails.\n- Clear verification: After clearing per-pin raw and group raw, the group status must read 0; otherwise the test fails.\n- System route clear: The system raw status for the GPIO instance must be cleared on write; any residual set bit indicates failure.\n- Final: finish(test_err) with test_err == 0 indicates pass; any nonzero indicates fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Enables negative-edge interrupts per GPIO pin and verifies interrupt assertion, raw/group status, and clear behavior.",
    "Hidden_Remarks": "Wait is armed before edge generation to avoid race. ISR uses timeouts to prevent hangs; suggested to adjust to the simulation time base if needed. Conditional routing for GPIO0/ GPIO1 interrupts is controlled via build-time macros.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1. test_err = 0\n2. Ifdef GPIO0: GIC_EnableIRQ(87)\n3. Ifdef GPIO1: GIC_EnableIRQ(88)\n4. Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR (enable sysreg route)\n5. Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR (enable sysreg route)\n6. WRITE 0xA0243ffc, 0xffffffff (drive all high)\n7. Phase 1 configuration loop: for (i=0..31)\n   7.1 addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4)\n   7.2 WRITE addr1, ((1u<<20) | (1u<<18) | (1u<<16))  // doe=1, neie=1, iclr=1\n   7.3 wait_on(10)\n8. Phase 2 per-pin test loop: for (i=0..31)\n   8.1 wr_val = 1u << i\n   8.2 WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val (pre-clear)\n   8.3 WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val (enable bit i)\n   8.4 wait_on(10)\n   8.5 int_pend = 1 (arm wait)\n   8.6 WRITE 0xA0243ffc, 0xffffffff; wait_on(30); WRITE 0xA0243ffc, ~wr_val (create falling edge on bit i)\n   8.7 timeout = 5000; while (int_pend && timeout--) wait_on(10)\n   8.8 If (timeout == 0): printf timeout, test_err++\n9. finish(test_err)\n\nInterrupt handler: Default_IRQHandler()\n10. local_wr = 1u << i; int_pend = 0\n11. WRITE 0xA0243ffc, 0xffffffff (return pad high)\n12. raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); READ rdata = read_reg(raddr)\n13. If ((rdata & 0x1) != 0): test_err++ (DIN must be 0 after negedge)\n14. If ((rdata & 0x2) != 0x0) then\n    14.1 READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n    14.2 If ((rdata_grp & local_wr) == 0): test_err++\n    14.3 raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); WRITE raddr2, ((1u<<20) | (1u<<16)) (doe=1, iclr=1)\n    14.4 WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr (group clear)\n    14.5 READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n    14.6 If (rdata_grp != 0x0): test_err++\n    14.7 Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR; GIC_ClearIRQ(87)\n    14.8 Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR; GIC_ClearIRQ(88)\n    else\n    14.9 test_err++ (raw bit not set unexpectedly)\n",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO0_INTR,LSS_SYSREG_INTR_EN1_GPIO1_INTR,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR,LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Hidden_Validation_Acceptance_Criteria": "Timeout on wait indicates failure. After negedge, (rdata & 0x1) must be 0 (DIN low). Raw status: (rdata & 0x2) must be nonzero and group status (read MIZAR_GPIO_GP0_INTR1_INTR_STS1) must have the corresponding bit set; otherwise failure. After clearing per-pin raw (iclr=1) and group raw (WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr), subsequent read of MIZAR_GPIO_GP0_INTR1_INTR_STS1 must be 0; otherwise failure. System route cleared via MIZAR_LSS_SYSREG_RAW_STCR1 should not read back the set bit; otherwise failure. Final pass if test_err==0; else fail."
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Independent GPIO control Register — peie",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Enables positive-edge interrupts on all GPIO pads and verifies interrupt assertion, group status, and clear behavior across all pins.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Timeouts bound the wait for interrupts. Group interrupt is masked during service and re-enabled afterward. Conditional routing for GPIO0/ GPIO1 is controlled via build-time macros.",
    "Test Steps / Procedure": "1) Enable the interrupt controller for the GPIO instance. 2) Enable the system interrupt route for the selected GPIO instance. 3) Enable positive-edge interrupts for each pin (8–39). 4) Configure all pins in input mode using group IO control. 5) Enable all per-pin interrupts in the group. 6) For each pin, drive low, arm wait, and create a single rising edge; wait until the interrupt is taken or the timeout expires. 7) In the handler, mask the group interrupt, verify group interrupt status is nonzero, clear per-pin raw status for all pins, and verify the group status clears to zero. 8) Clear the system raw status route and verify it is cleared. 9) Re-enable the group interrupt and clear the interrupt controller request.",
    "Impacted Registers": "GPIO_8, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, INTR1_INTR_EN1, INTR1_INTR_STS1",
    "Validation / Acceptance Criteria": "- Timeout: If an interrupt does not arrive within the bounded wait, the test fails.\n- Group status: The group interrupt status must be nonzero when an interrupt occurs; otherwise the test fails.\n- Clear verification: After clearing per-pin raw across all pins, the group status must read 0; otherwise the test fails.\n- System route clear: The system raw status for the GPIO instance must be cleared on write; any residual set bit indicates failure.\n- Final: finish(test_err) with test_err == 0 indicates pass; any nonzero indicates fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Enables positive-edge interrupts on all GPIO pads and verifies interrupt assertion, group status, and clear behavior across all pins.",
    "Hidden_Remarks": "Timeouts bound the wait for interrupts. Group interrupt is masked during service and re-enabled afterward. Conditional routing for GPIO0/ GPIO1 is controlled via build-time macros.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1. Ifdef GPIO0: GIC_EnableIRQ(87)\n2. Ifdef GPIO1: GIC_EnableIRQ(88)\n3. test_err = 0\n4. Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR\n5. Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR\n6. For i=0..31: WRITE (MIZAR_GPIO_GP0_GPIO_8 + i*4), 0x00020000 (enable peie per pin)\n7. wait_on(10)\n8. WRITE MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF; WRITE GROUP2, 0x000000FF; WRITE GROUP3, 0x000000FF; WRITE GROUP4, 0x000000FF (input mode)\n9. wait_on(10)\n10. WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF (enable all)\n11. For i=0..31:\n    11.1 WRITE 0xA0243ffc, 0x00000000 (drive low); wait_on(10)\n    11.2 int_pend = 1 (arm)\n    11.3 WRITE 0xA0243ffc, 0xFFFFFFFF (rising edge)\n    11.4 timeout = 2000; while (int_pend == 1 && --timeout > 0) wait_on(10)\n    11.5 If (timeout == 0) { printf timeout; test_err++; break; }\n    11.6 WRITE 0xA0243ffc, 0x00000000; wait_on(10)\n12. finish(test_err)\n\nInterrupt handler: Default_IRQHandler()\n13. wr_val = 1 << i; int_pend = 0\n14. READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n15. WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000 (mask during service)\n16. If ((rdata_grp & 0xffffffff) != 0) pass else { printf error; test_err++ }\n17. For j=0..31: WRITE (MIZAR_GPIO_GP0_GPIO_8 + j*4), 0x00010000 (iclr)\n18. wait_on(2)\n19. READ rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) pass else { printf error; test_err++ }\n20. Ifdef GPIO0: WRITE MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR; READ rdata; if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) test_err++\n21. Ifdef GPIO1: WRITE MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR; READ rdata; if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) test_err++\n22. WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF (re-enable)\n23. Ifdef GPIO0: GIC_ClearIRQ(87); Ifdef GPIO1: GIC_ClearIRQ(88)\n",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,LSS_SYSREG_INTR_EN1_GPIO0_INTR,LSS_SYSREG_INTR_EN1_GPIO1_INTR,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_LSS_SYSREG_RAW_STCR1,LSS_SYSREG_RAW_STCR1_GPIO0_INTR,LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Hidden_Validation_Acceptance_Criteria": "Timeout on wait indicates failure. Group interrupt status read from MIZAR_GPIO_GP0_INTR1_INTR_STS1 must be nonzero on interrupt; otherwise failure. After clearing per-pin raw across all pins (writing iclr), the group status must read 0; otherwise failure. System route cleared via MIZAR_LSS_SYSREG_RAW_STCR1 should not read back the set bit; otherwise failure. Final pass if test_err==0; else fail."
  }
}'''

META_COLS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

MAIN_COLS_ORDER = [
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

ALL_EXPECTED_COLS = MAIN_COLS_ORDER + META_COLS


def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument('--output-dir', required=True)
    p.add_argument('--ip-name', required=True)
    p.add_argument('--ts', required=False, help='IST timestamp YYYYMMDD_HHMMSS (if omitted, computed at runtime)')
    return p.parse_args()


def json_to_rows():
    # Load JSON; it may be a dict of TCx objects, convert to array preserving lexical key order
    obj = json.loads(TESTPLAN_JSON)
    if isinstance(obj, list):
        rows = obj
    elif isinstance(obj, dict):
        rows = [obj[k] for k in sorted(obj.keys())]
    else:
        raise ValueError('Invalid JSON root; expected array or object of testcases')
    if not rows or not all(isinstance(r, dict) for r in rows):
        raise ValueError('JSON must yield non-empty array of objects')
    # Build union schema in first-seen order per ALL_EXPECTED_COLS precedence
    # Start with ALL_EXPECTED_COLS to enforce presence and order; then append any extras in first-seen order
    seen = []
    for key in ALL_EXPECTED_COLS:
        seen.append(key)
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    # Normalize rows (preserve values exactly; fill missing with '')
    norm_rows = []
    for r in rows:
        norm = {k: r.get(k, '') for k in seen}
        norm_rows.append(norm)
    return seen, norm_rows


def calc_col_width(value):
    s = str(value) if value is not None else ''
    maxlen = max((len(line) for line in s.splitlines()), default=0)
    # heuristic width; cap between 10 and 120
    return max(10, min(120, maxlen + 2))


def numberify(text):
    if not isinstance(text, str) or not text.strip():
        return text
    lines = [ln.strip() for ln in text.replace('\r\n', '\n').split('\n') if ln.strip()]
    out = []
    for i, ln in enumerate(lines, 1):
        # remove leading bullets or numbering like '1) ', '1. ', '- '
        ln = re.sub(r'^(\d+)[\)\.]\s*', '', ln)
        ln = re.sub(r'^[\-\*]\s*', '', ln)
        out.append(f"{i}. {ln}")
    return "\n".join(out)


def build_workbook(headers, rows, out_path):
    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'

    # Write headers
    for c, h in enumerate(headers, 1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Write data rows
    for r_idx, row in enumerate(rows, start=2):
        for c, h in enumerate(headers, 1):
            ws.cell(row=r_idx, column=c, value=row.get(h, ''))

    # Autofit columns (rough)
    for c, h in enumerate(headers, 1):
        width = calc_col_width(h)
        for r_idx in range(2, ws.max_row + 1):
            v = ws.cell(row=r_idx, column=c).value
            width = max(width, calc_col_width(v))
        ws.column_dimensions[get_column_letter(c)].width = width

    # Create Meta_data_sheet
    meta = wb.create_sheet('Meta_data_sheet')
    for c, h in enumerate(META_COLS, 1):
        meta.cell(row=1, column=c, value=h).font = Font(bold=True)
    for r_idx, row in enumerate(rows, start=2):
        for c, h in enumerate(META_COLS, 1):
            meta.cell(row=r_idx, column=c, value=row.get(h, ''))
    meta.sheet_state = 'veryHidden'

    # Normalize main sheet: rename and retain MAIN_COLS_ORDER only, in exact order
    ws.title = 'TestPlan'

    # Build reordered data for main sheet
    data_rows = []
    for r_idx in range(2, ws.max_row + 1):
        data = {}
        for h in MAIN_COLS_ORDER:
            # Find column index of h in headers (if present)
            try:
                c_idx = headers.index(h) + 1
                data[h] = ws.cell(row=r_idx, column=c_idx).value
            except ValueError:
                data[h] = ''
        data_rows.append(data)

    # Clear sheet and rewrite only main columns
    ws.delete_rows(1, ws.max_row)
    for c, h in enumerate(MAIN_COLS_ORDER, 1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    for r_idx, data in enumerate(data_rows, start=2):
        for c, h in enumerate(MAIN_COLS_ORDER, 1):
            ws.cell(row=r_idx, column=c, value=data.get(h, ''))

    # Apply numbering for specific columns in TestPlan sheet
    wrap_cols = ['Test Description', 'Remarks', 'Test Steps / Procedure', 'Validation / Acceptance Criteria']
    for r in range(2, ws.max_row + 1):
        for h in wrap_cols:
            if h in ['Test Steps / Procedure', 'Validation / Acceptance Criteria']:
                try:
                    c_idx = MAIN_COLS_ORDER.index(h) + 1
                except ValueError:
                    continue
                v = ws.cell(row=r, column=c_idx).value
                ws.cell(row=r, column=c_idx, value=numberify(v))

    # Formatting
    header_fill = PatternFill(start_color='FF0070C0', end_color='FF0070C0', fill_type='solid')
    center = Alignment(horizontal='center', vertical='center', wrap_text=True)
    left_wrap = Alignment(horizontal='left', vertical='top', wrap_text=True)
    left = Alignment(horizontal='left', vertical='top')
    thin = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))

    # Header row formatting
    for c in range(1, len(MAIN_COLS_ORDER) + 1):
        cell = ws.cell(row=1, column=c)
        cell.alignment = center
        cell.fill = header_fill
        cell.font = Font(bold=True)
        cell.border = thin

    # Data rows formatting and wrap
    for r in range(2, ws.max_row + 1):
        for c in range(1, len(MAIN_COLS_ORDER) + 1):
            cell = ws.cell(row=r, column=c)
            h = MAIN_COLS_ORDER[c-1]
            if h in wrap_cols:
                cell.alignment = left_wrap
            elif h == 'Index':
                cell.alignment = Alignment(horizontal='center', vertical='top')
            else:
                cell.alignment = left
            cell.border = thin

    # Approximate autofit columns after wrapping
    for c, h in enumerate(MAIN_COLS_ORDER, 1):
        width = calc_col_width(h)
        for r in range(2, ws.max_row + 1):
            width = max(width, calc_col_width(ws.cell(row=r, column=c).value))
        ws.column_dimensions[get_column_letter(c)].width = width

    # Approximate row heights based on line count for wrapped columns
    wrap_idx = [MAIN_COLS_ORDER.index(h)+1 for h in wrap_cols if h in MAIN_COLS_ORDER]
    base_height = 15
    for r in range(2, ws.max_row + 1):
        lines = 1
        for c in wrap_idx:
            v = ws.cell(row=r, column=c).value
            if isinstance(v, str):
                lines = max(lines, v.count('\n') + 1)
        ws.row_dimensions[r].height = base_height * lines

    # Data validation for Code Generation (Required / Not)
    try:
        code_col_idx = MAIN_COLS_ORDER.index('Code Generation (Required / Not)') + 1
        dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
        dv.error = 'Select one of: Required, Blank, Not Required'
        dv.errorTitle = 'Invalid selection'
        ws.add_data_validation(dv)
        if ws.max_row >= 2:
            dv.add(f"{get_column_letter(code_col_idx)}2:{get_column_letter(code_col_idx)}{ws.max_row}")
    except ValueError:
        pass

    # Safety check: only TestPlan and Meta_data_sheet should exist
    for sheet in list(wb.sheetnames):
        if sheet == 'Data':
            # Should not exist; remove if found
            std = wb[sheet]
            wb.remove(std)
    if set(wb.sheetnames) - set(['TestPlan', 'Meta_data_sheet']):
        # Other sheets should not exist; this is a hardening step (not expected here)
        for s in list(wb.sheetnames):
            if s not in ['TestPlan', 'Meta_data_sheet']:
                wb.remove(wb[s])

    # Save
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)

    # Validate as real XLSX (ZIP entries) and loadable
    with ZipFile(out_path, 'r') as zf:
        assert '[Content_Types].xml' in zf.namelist(), 'Missing [Content_Types].xml'
        assert 'xl/workbook.xml' in zf.namelist(), 'Missing xl/workbook.xml'
    _ = load_workbook(out_path)


if __name__ == '__main__':
    args = parse_args()
    headers, rows = json_to_rows()

    # Ensure all expected columns exist in headers
    for col in ALL_EXPECTED_COLS:
        if col not in headers:
            headers.append(col)
            for r in rows:
                if col not in r:
                    r[col] = ''

    # Determine IST timestamp
    ts = args.ts
    if not ts:
        ist = timezone(timedelta(hours=5, minutes=30))
        ts = datetime.now(ist).strftime('%Y%m%d_%H%M%S')

    filename = f"{args.ip_name}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(args.output_dir, filename)

    build_workbook(headers, rows, out_path)

    print(f"WROTE: {out_path}")
