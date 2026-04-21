import os
import json
from datetime import datetime, timezone, timedelta
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, Border, Side, PatternFill
from openpyxl.utils import get_column_letter

# -----------------------------
# Input JSON (embedded exactly as provided)
# -----------------------------
json_data_str = r'''{
  "ip": "GPIO",
  "test_cases": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "Independent control register for each GPIO",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Checks default reset values for GPIO control and group registers and verifies masked write/read behavior across predefined patterns using per-register read/write masks.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "Default value comparison masks off the least significant bit of the read value before compare; certain registers are skipped for default and/or read/write phases as indicated by skip arrays.",
      "Test Steps / Procedure": [
        "Entry: Invoke the test entry point.",
        "Perform default value checks over the configured register list in index order, skipping those marked to be excluded for default verification; for each eligible register, read the value and compare the read value (with the least significant bit forced to zero) against its documented default.",
        "Iterate over six predefined data patterns; for each pattern, write masked data to each writable register in the list, skipping those marked to be excluded for write access.",
        "After each write pass for a given pattern, read each readable register in the list that is also writable; for each, compute the expected value using the intersection of read and write masks combined with preserved default bits where writes are masked off; compare the read value to the expected value.",
        "Aggregate failures from default value mismatches and write/read mismatches; report pass if no failures, otherwise fail."
      ],
      "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1, INTR2_INTR_EN1, INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": [
        "Default value check: For each register allowed for default checking, the read value with the least significant bit cleared must equal the documented default. Any mismatch constitutes a failure.",
        "Write/read check: For each writable and readable register and for each test pattern, the read value must equal the combination of written bits under the write and read masks plus preserved default bits where writes are masked off. Any mismatch constitutes a failure.",
        "Overall result: The test passes only if there are zero default value mismatches and zero write/read mismatches."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "Entry test_case() calls chk_rst_val() then chk_rd_wr(); chk_rst_val(): for i in [0..CNT-1], addr = addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); compare data == default_value_array[i], else def_fail_cnt++. chk_rd_wr(): patterns chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}; For each pattern, write phase: for each i, if skip_array[i]==1 continue; if (write_mask_array[i]==0) continue; else write_reg(addr_array[i], (data_wr & write_mask_array[i])). Read phase: for each i with write_mask_array[i]!=0 and read_mask_array[i]!=0, data_rd = (read_reg(addr_array[i]) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd != exp_val) wr_fail_cnt++. Finally, if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).",
      "Hidden_Remarks": "In chk_rst_val(), the least significant bit of the read value is masked off before comparing with defaults. test_define.c notes: when reading default values the DIN value may become 1 automatically if not forced; forcing zero affects level select and can cause mismatch, hence some registers are skipped (skip arrays).",
      "Hidden_Test_Steps_Procedure": [
        "A. Entry point: test_case()",
        "B. Call chk_rst_val()",
        "  - for (i = 0; i < CNT; i++):",
        "    - addr = addr_array[i] (macros include: MIZAR_GPIO_GP0_GPIO_8 ... MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4)",
        "    - if (skip_rst_array[i] == 1) continue;",
        "    - if (read_mask_array[i] == 0x00000000) continue;",
        "    - READ: data_rd = read_reg(addr);",
        "    - MODIFY (mask for compare): data = (data_rd & 0xfffffffe);",
        "    - if (data == default_value_array[i]) PASS else def_fail_cnt++ and log mismatch.",
        "C. Call chk_rd_wr()",
        "  - chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}",
        "  - for (j = 0; j < 6; j++):",
        "    - data_wr = chk_val[j];",
        "    - Write phase: for (i = 0; i < CNT; i++):",
        "      - addr = addr_array[i];",
        "      - if (skip_array[i] == 1) continue;",
        "      - if (write_mask_array[i] == 0x00000000) continue;",
        "      - WRITE: write_reg(addr, (data_wr & write_mask_array[i]));",
        "    - Read/verify phase: for (i = 0; i < CNT; i++):",
        "      - addr = addr_array[i];",
        "      - if (skip_array[i] == 1) continue;",
        "      - if (write_mask_array[i] == 0x00000000) continue;",
        "      - if (read_mask_array[i] == 0x00000000) continue;",
        "      - READ: data_rd = (read_reg(addr) & read_mask_array[i]);",
        "      - COMPUTE: wr_n = (write_mask_array[i] ^ 0xffffffff);",
        "      - COMPUTE: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]));",
        "      - if (data_rd == exp_val) PASS else wr_fail_cnt++ and log mismatch.",
        "D. Exit: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0)",
        "Timing: No explicit waits in this test.",
        "Notes: Skips controlled by skip_array[] and skip_rst_array[]."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": "chk_rst_val(): Pass when for every i where read_mask_array[i]!=0 and skip_rst_array[i]==0, (read_reg(addr_array[i]) & 0xfffffffe) == default_value_array[i]. chk_rd_wr(): For each pattern data_wr in chk_val[], for each i where write_mask_array[i]!=0 and read_mask_array[i]!=0 and skip_array[i]==0, after WRITE write_reg(addr_array[i], (data_wr & write_mask_array[i])), then READ data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); expected exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); require data_rd == exp_val. Final finish(0) only if def_fail_cnt==0 and wr_fail_cnt==0."
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "neie: Negative edge interrupt enable; 1 - Enable interrupt when falling edge (neg edge) is detected on gpio pin; 0 - Disable interrupt on neg-edge detection",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Configures all GPIO[8..39] as inputs with falling-edge interrupt enabled, generates a single falling edge per pin via pad control, and verifies per-pin raw and group interrupt behavior using an ISR and bounded wait timeouts.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "A bounded wait loop is used with a timeout of 5000 units; interrupt wait is armed before edge generation to avoid races; system-level interrupt enable and clear are performed via system registers.",
      "Test Steps / Procedure": [
        "Entry: Start the test entry point.",
        "Enable the platform interrupt line for the corresponding GPIO instance.",
        "Enable the system-level interrupt output for the selected GPIO instance.",
        "Drive the pad control register at 0xA0243ffc to drive all pads high to establish a known initial state.",
        "For each pin index from 0 to 31 corresponding to GPIO_8 through GPIO_39: configure the per-pin control register to input mode and enable falling-edge interrupt and raw-status clear; insert a short wait after each configuration.",
        "For each pin index from 0 to 31: clear the corresponding raw interrupt status bit in GPIO_INTR_RAW_STCLR1; enable only that bit in INTR1_INTR_EN1; insert a short wait; arm the wait flag; drive all pads high, wait, then drive the target bit low to create a falling edge.",
        "Enter a bounded wait loop with a timeout to wait for the interrupt service routine to clear the wait flag; if the timeout expires, record an error for that pin.",
        "ISR: Upon interrupt, immediately drive all pads high to restore the known state; read the current per-pin register (GPIO_8 + index stride) and confirm the input bit indicates low level after a falling edge.",
        "ISR: Verify the group interrupt status register INTR1_INTR_STS1 shows the bit for the current pin is set; if not, record an error.",
        "ISR: Clear the per-pin raw status by writing to the per-pin control register to set input mode and raw-status clear; also clear the corresponding bit in GPIO_INTR_RAW_STCLR1.",
        "ISR: Read INTR1_INTR_STS1 to confirm the group status is now cleared; clear the system-level raw status for the GPIO instance and clear the platform interrupt.",
        "Exit: Finish with the accumulated error count; pass only if no errors were recorded."
      ],
      "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1",
      "Validation / Acceptance Criteria": [
        "For each pin, an interrupt must be observed before the bounded wait timeout expires; a timeout constitutes a failure for that pin.",
        "Within the ISR for a falling-edge event, the per-pin input indicator must reflect a low level; otherwise record a failure.",
        "The group interrupt status must have the bit corresponding to the current pin set at entry to the ISR; after clearing per-pin raw status and the corresponding group raw clear, the group status must read as zero.",
        "Final result passes only if the accumulated error count is zero."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "test_case(): test_err=0; conditionally GIC_EnableIRQ(87/88). Enable system interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0|1}_INTR). write_reg(0xA0243ffc, 0xffffffff) to drive high. For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10). For i=0..31: wr_val=(1u<<i); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if(timeout==0){ printf timeout; test_err++; }. finish(test_err). Default_IRQHandler(): local_wr=(1u<<i); int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(raddr2,(1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR); GIC_ClearIRQ(87/88); } else { test_err++; }",
      "Hidden_Remarks": "Uses bounded wait loop with timeout=5000 for interrupt detection; arms int_pend before generating edge to avoid race; pad drive register at 0xA0243ffc used to create edges; system interrupt enable/clear via MIZAR_LSS_SYSREG_* registers.",
      "Hidden_Test_Steps_Procedure": [
        "A. Entry point: test_case()",
        "B. Setup:",
        "  - Optionally enable GIC IRQ (GIC_EnableIRQ(87) for GPIO0 or 88 for GPIO1).",
        "  - WRITE: MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR.",
        "  - WRITE: 0xA0243ffc = 0xffffffff (drive all high).",
        "C. Per-pin configuration loop (i=0..31):",
        "  - addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i*4).",
        "  - WRITE: addr1 = ((1<<20) | (1<<18) | (1<<16))  // doe=1, neie=1, iclr=1.",
        "  - wait_on(10).",
        "D. Test loop for each bit (i=0..31):",
        "  - wr_val = (1u<<i).",
        "  - WRITE: MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 = wr_val.",
        "  - WRITE: MIZAR_GPIO_GP0_INTR1_INTR_EN1 = wr_val.",
        "  - wait_on(10).",
        "  - int_pend = 1 (arm).",
        "  - WRITE: 0xA0243ffc = 0xffffffff (ensure high).",
        "  - wait_on(30).",
        "  - WRITE: 0xA0243ffc = ~wr_val (drive falling edge for bit i).",
        "  - timeout = 5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { log timeout; test_err++; }",
        "E. Exit main: finish(test_err).",
        "F. ISR: Default_IRQHandler()",
        "  - local_wr = (1u<<i); int_pend = 0.",
        "  - WRITE: 0xA0243ffc = 0xffffffff (restore high).",
        "  - raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); READ: rdata = read_reg(raddr).",
        "  - if ((rdata & 0x1) != 0) test_err++ (DIN should be 0 after negedge).",
        "  - if ((rdata & 0x2) != 0x0) {",
        "      READ: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);",
        "      if ((rdata_grp & local_wr) == 0) test_err++;",
        "      raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i*4);",
        "      WRITE: raddr2 = ((1<<20) | (1<<16))  // doe=1, iclr=1.",
        "      WRITE: MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 = local_wr;",
        "      READ: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++;",
        "      WRITE: MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR; GIC_ClearIRQ(87/88);",
        "    } else { test_err++; }",
        "Timing: Multiple wait_on calls (10 and 30 units) and ISR wait bounded by timeout=5000 with 10-unit polling intervals."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "Main loop: For each i, interrupt observed before timeout (int_pend cleared in ISR) else test_err++. ISR: Require ((rdata & 0x1)==0) (DIN low), require ((rdata & 0x2)!=0) indicating raw set, group status read from MIZAR_GPIO_GP0_INTR1_INTR_STS1 must have bit (1<<i) set, after clears (per-pin iclr and MIZAR_GPIO_GPIO_INTR_RAW_STCLR1) group status must be 0. Clear system raw and GIC. finish(test_err) with zero indicates PASS."
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "peie: Positive edge interrupt enable; 1 - Enable interrupt when rising edge (pos edge) is detected on gpio pin; 0 - Disable interrupt on pos-edge detection",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Enables positive edge detection on all GPIO[8..39], sets all pads to input mode, generates a rising edge per pad via pad control, and validates group interrupt behavior and system raw status clear in the ISR with timeouts to bound waiting.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "Interrupt wait is armed prior to generating the edge; a bounded wait loop with timeout=2000 units is used; per-pin raw status is cleared for all pins during ISR; group interrupt is masked during service then re-enabled.",
      "Test Steps / Procedure": [
        "Entry: Start the test entry point.",
        "Enable the platform interrupt line for the corresponding GPIO instance and enable system-level interrupt output for that instance.",
        "For each pin index from 0 to 31 corresponding to GPIO_8 through GPIO_39: enable positive-edge detection by programming the per-pin control register.",
        "Set all GPIO groups to input mode via GPIO_IO_CTRL_GROUP1 through GPIO_IO_CTRL_GROUP4.",
        "Enable all bits in the group interrupt enable register INTR1_INTR_EN1.",
        "For each pin index from 0 to 31: drive the pad control to low to establish baseline, arm the wait flag, then drive high to produce a rising edge; wait in a bounded loop for the ISR to signal completion; on timeout record an error; optionally drive low again to prepare for the next iteration.",
        "ISR: Read the group interrupt status register INTR1_INTR_STS1 and mask the group during service; verify a nonzero status.",
        "ISR: Clear per‑pin raw status for all pins by writing the raw clear field in each per‑pin control register; verify that the group status register reads zero after the clear.",
        "ISR: Clear the system raw status bit corresponding to the instance and verify the clear by reading back the system raw status register; re-enable the group interrupt and clear the platform interrupt.",
        "Exit: Finish with the accumulated error count; pass only if no errors were recorded."
      ],
      "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, INTR1_INTR_EN1, INTR1_INTR_STS1",
      "Validation / Acceptance Criteria": [
        "For each pin, an interrupt must be observed before the bounded wait timeout; a timeout constitutes a failure.",
        "In the ISR, the group interrupt status must be nonzero upon entry with positive-edge configuration, and must read zero after clearing per‑pin raw status for all pins.",
        "The system raw status bit corresponding to the selected GPIO instance must be cleared successfully, as verified by read-back.",
        "Final result passes only if the accumulated error count is zero."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "test_case(): Optionally GIC_EnableIRQ(87/88). Enable sysreg output: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO{0|1}_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) // peie=1. wait_on(10). Set input mode groups: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, 0x000000FF). wait_on(10). Enable all: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while(int_pend==1 && --timeout>0) wait_on(10); if(timeout==0){ printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if((rdata_grp & 0xffffffff) != 0) ok else { printf error; test_err++; }. For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2). rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp == 0x0) ok else { printf error; test_err++; }. Clear sysreg raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR) != 0) { printf error; test_err++; }. Re-enable: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).",
      "Hidden_Remarks": "Arms int_pend before generating the rising edge to avoid race; bounded wait timeout=2000 with 10-unit waits; group is masked during ISR and re-enabled after service; per-pin raw status is cleared for all pins in ISR.",
      "Hidden_Test_Steps_Procedure": [
        "A. Entry point: test_case()",
        "B. Setup:",
        "  - Optionally enable GIC IRQ (87/88).",
        "  - WRITE: MIZAR_LSS_SYSREG_INTR_EN1 = LSS_SYSREG_INTR_EN1_GPIO{0|1}_INTR.",
        "C. Configure per-pin posedge enable:",
        "  - For i=0..31: WRITE: MIZAR_GPIO_GP0_GPIO_8 + (i*4) = 0x00020000 (peie=1).",
        "  - wait_on(10).",
        "D. Set input mode via groups:",
        "  - WRITE: MIZAR_GPIO_GPIO_IO_CTRL_GROUP1 = 0x000000FF.",
        "  - WRITE: MIZAR_GPIO_GPIO_IO_CTRL_GROUP2 = 0x000000FF.",
        "  - WRITE: MIZAR_GPIO_GPIO_IO_CTRL_GROUP3 = 0x000000FF.",
        "  - WRITE: MIZAR_GPIO_GPIO_IO_CTRL_GROUP4 = 0x000000FF.",
        "  - wait_on(10).",
        "E. Enable interrupts:",
        "  - WRITE: MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF.",
        "F. For each pin i=0..31 (edge generation and wait):",
        "  - WRITE: 0xA0243ffc = 0x00000000 (drive low).",
        "  - wait_on(10).",
        "  - int_pend = 1.",
        "  - WRITE: 0xA0243ffc = 0xFFFFFFFF (rising edge).",
        "  - timeout = 2000; while (int_pend==1 && --timeout>0) wait_on(10); if (timeout==0) { log timeout; test_err++; break; }",
        "  - WRITE: 0xA0243ffc = 0x00000000; wait_on(10).",
        "G. Exit main: finish(test_err).",
        "H. ISR: Default_IRQHandler()",
        "  - wr_val = (1<<i); int_pend = 0.",
        "  - READ: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).",
        "  - WRITE: MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0x00000000 (mask during service).",
        "  - if ((rdata_grp & 0xffffffff) != 0) ok else { printf error; test_err++; }",
        "  - For j=0..31: WRITE: MIZAR_GPIO_GP0_GPIO_8 + (j*4) = 0x00010000 (iclr=1).",
        "  - wait_on(2).",
        "  - READ: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) ok else { printf error; test_err++; }",
        "  - WRITE: MIZAR_LSS_SYSREG_RAW_STCR1 = LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR.",
        "  - READ: rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO{0|1}_INTR) != 0) { printf error; test_err++; }",
        "  - WRITE: MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF.",
        "  - GIC_ClearIRQ(87/88).",
        "Timing: wait_on(10) during setup and per-iteration; ISR uses wait_on(2); bounded wait timeout=2000 with 10-unit intervals."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "Main loop: For each i, int_pend must be cleared by ISR before timeout (2000) else error++. ISR: On entry rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) must be non-zero; after clearing per-pin raw bits (writing 0x00010000 to each per-pin register), rdata_grp must read 0; system-level raw status clear verified by reading MIZAR_LSS_SYSREG_RAW_STCR1 and ensuring instance bit is 0; re-enable group and clear GIC. Test passes when test_err==0 at finish."
    }
  ]
}'''

# -----------------------------
# Constants per Stage-1 rules
# -----------------------------
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

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
IP_NAME = "GPIO"

# -----------------------------
# Helpers
# -----------------------------

def ensure_list_to_multiline(v: Any) -> Any:
    if isinstance(v, list):
        return "\n".join(str(x) for x in v)
    return v


def union_keys_preserve_order(rows: List[Dict[str, Any]]) -> List[str]:
    seen = []
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    return seen


def autosize_columns(ws):
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            val = cell.value
            if val is None:
                length = 0
            else:
                s = str(val)
                # consider wrapped lines
                length = max((len(line) for line in s.splitlines()), default=0)
            if length > max_len:
                max_len = length
        # heuristic: width ~ chars * 1.2 + padding
        adjusted = min(120, max(10, int(max_len * 1.2) + 2))
        ws.column_dimensions[get_column_letter(col_idx)].width = adjusted


def autofit_row_heights(ws):
    base_height = 15
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row):
        max_lines = 1
        for cell in row:
            if cell.value is None:
                continue
            s = str(cell.value)
            lines = s.count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[cell.row].height = base_height * max_lines


def apply_borders(ws):
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def create_workbook(data_rows: List[Dict[str, Any]]):
    # Normalize arrays to newline strings
    norm_rows = []
    for r in data_rows:
        norm = {k: ensure_list_to_multiline(v) for k, v in r.items()}
        norm_rows.append(norm)

    headers = union_keys_preserve_order(norm_rows)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header
    for c_idx, h in enumerate(headers, start=1):
        ws.cell(row=1, column=c_idx, value=h)
    # Rows
    for r_idx, row in enumerate(norm_rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ""))

    # Basic formatting
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center")
    for cell in ws[1]:
        cell.font = header_font
        cell.alignment = header_align

    ws.freeze_panes = "A2"
    autosize_columns(ws)

    # Create Meta sheet
    meta = wb.create_sheet("Meta_data_sheet")
    # Write meta headers
    for c_idx, h in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=c_idx, value=h)
        meta.cell(row=1, column=c_idx).font = header_font
        meta.cell(row=1, column=c_idx).alignment = header_align
    # Transfer values per row in same order as data rows
    for r_idx in range(2, ws.max_row + 1):
        # Build a map of header->value from Data row
        row_map = {headers[c - 1]: ws.cell(row=r_idx, column=c).value for c in range(1, len(headers) + 1)}
        for c_idx, h in enumerate(META_COLS, start=1):
            meta.cell(row=r_idx - 1 + 1, column=c_idx, value=row_map.get(h, ""))

    # Hide Meta sheet (veryHidden)
    meta.sheet_state = "veryHidden"

    # Prepare TestPlan sheet: remove META cols and keep only MAIN_ORDER
    # Rename Data -> TestPlan
    ws.title = "TestPlan"

    # Build header index map for current headers
    header_to_col = {headers[i]: i + 1 for i in range(len(headers))}

    # Determine columns to keep (MAIN_ORDER only)
    keep_headers = [h for h in MAIN_ORDER if h in header_to_col]

    # Rebuild TestPlan sheet with only MAIN_ORDER columns
    # Create a new sheet to ensure clean ordering, then replace
    tp = wb.create_sheet("_tmp_tp")
    # Write headers
    for c_idx, h in enumerate(keep_headers, start=1):
        tp.cell(row=1, column=c_idx, value=h)
    # Rows
    for r_idx in range(2, ws.max_row + 1):
        row_map = {headers[c - 1]: ws.cell(row=r_idx, column=c).value for c in range(1, len(headers) + 1)}
        for c_idx, h in enumerate(keep_headers, start=1):
            tp.cell(row=r_idx - 1 + 1, column=c_idx, value=row_map.get(h, ""))

    # Delete old TestPlan and rename tmp
    wb.remove(ws)
    tp.title = "TestPlan"
    ws = tp

    # Strict formatting for TestPlan
    # Header style
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=False)
    header_fill = PatternFill("solid", fgColor="DDDDDD")
    for cell in ws[1]:
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill

    # Data alignment
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            col_name = ws.cell(row=1, column=cell.column).value
            if col_name in WRAP_COLS:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
            elif col_name == "Index":
                cell.alignment = Alignment(vertical="top", horizontal="center")
            else:
                cell.alignment = Alignment(vertical="top", horizontal="left")

    # Freeze top row and autosize & row heights
    ws.freeze_panes = "A2"
    autosize_columns(ws)
    autofit_row_heights(ws)

    # Borders
    apply_borders(ws)

    return wb, len(norm_rows), len(keep_headers)


def main():
    data = json.loads(json_data_str)
    if not isinstance(data, dict) or "test_cases" not in data or not isinstance(data["test_cases"], list) or len(data["test_cases"]) == 0:
        raise SystemExit("Invalid or empty JSON: expected top-level object with non-empty 'test_cases' array")

    rows = data["test_cases"]

    wb, nrows, ncols = create_workbook(rows)

    # IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")

    filename = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, filename)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(out_path)

    print(json.dumps({
        "status": "SUCCESS",
        "rows": nrows,
        "columns": ncols,
        "output_file": out_path,
        "timestamp_ist": ts
    }))


if __name__ == "__main__":
    main()
