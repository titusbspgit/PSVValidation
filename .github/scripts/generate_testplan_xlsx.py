#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timezone, timedelta
from copy import deepcopy

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl is required: {e}")
    sys.exit(2)

# Embedded JSON array input (Stage1 rows)
json_data = r"""
[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verifies default reset values and masked read/write behavior of GPIO and related group/interrupt registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "VRRW registers are skipped. When reading default values, DIN can read as 1 if not forced; forcing zero can set a selection high and cause mismatch.",
    "Test Steps / Procedure": "1) Read default values from gp0_gpio_8 to GP0_GPIO_39 and group/status registers and compare against expected resets using read masks. 2) For each writable register, write test patterns masked by the write mask. 3) Read back values using read masks and compute expected values based on write and default data with non-writable bits preserved. 4) Report failures for any default-value mismatch or write-read mismatch and summarize result.",
    "Impacted Registers": "gp0_gpio_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GP0_INTR2_INTR_EN1, GP0_INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
    "Validation / Acceptance Criteria": "1) Default values: For each address, (read_data & 0xFFFFFFFE) equals the expected default value; any mismatch is a failure. 2) Write-read check: For each writable and readable address, read_data equals ((write_data & read_mask & write_mask) | (~write_mask & read_mask & default_value)); any mismatch is a failure. 3) Overall result: Test passes only if both default-check and write-read mismatch counters are zero.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "program.c calls chk_rst_val() then chk_rd_wr(). chk_rst_val(): For i=0..CNT-1, addr=addr_array[i]; if skip_rst_array[i]==1 → continue; if read_mask_array[i]==0 → continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); if data==default_value_array[i] → PASS else def_fail_cnt++ and print failure. chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; For each pattern j: data_wr=chk_val[j]; Writing phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; else write_reg(addr, (data_wr & write_mask_array[i])). Reading phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; if read_mask_array[i]==0 → continue; else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xFFFFFFFF); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val → PASS else wr_fail_cnt++ and print failure. At end of test_case(), if def_fail_cnt>0 or wr_fail_cnt>0 finish(1) else finish(0).",
    "Hidden_Remarks": "//80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers //when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
    "Hidden_Test_Steps_Procedure": "1) Define arrays (addr_array[49], default_value_array[49], read_mask_array[49], write_mask_array[49], skip_array[49], skip_rst_array[49]) in test_define.c with register macros MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4, and associated mask/default macros. 2) chk_rst_val(): Loop i=0..48: addr=addr_array[i]; if skip_rst_array[i]==1 → continue; if read_mask_array[i]==0x00000000 → continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); compare data with default_value_array[i]; if equal → optional PASS print; else increment def_fail_cnt and print detailed mismatch including addr, expected, read_data, raw data. 3) chk_rd_wr(): Define chk_val[6]={0xFFFFFFFF,0xAAAAAAAA,0x55555555,0xF5F5F5F5,0xA5A5A5A5,0xFFFF0000}. For each j in 0..5: data_wr=chk_val[j]. 3a) Write phase: for i=0..48: addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; else write_reg(addr, (data_wr & write_mask_array[i])) with optional debug print. 3b) Read/verify phase: for i=0..48: addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; if read_mask_array[i]==0x00000000 → continue; else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xFFFFFFFF); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val → optional PASS print; else wr_fail_cnt++ and print mismatch including addr and values. 4) In test_case(): call chk_rst_val(); call chk_rd_wr(); if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). 5) soft_reset_chk() is compiled out (#ifdef 0) and not executed.",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "1) In chk_rst_val(): For each non-skipped, readable address, (read_reg(addr) & 0xFFFFFFFE) must equal default_value_array[i]; else def_fail_cnt++. 2) In chk_rd_wr(): For each non-skipped, writable and readable address, read_reg(addr) masked by read_mask must equal exp_val=((data_wr & read_mask & write_mask) | (~write_mask & read_mask & default_value)); else wr_fail_cnt++. 3) Final decision: finish(0) only if def_fail_cnt==0 and wr_fail_cnt==0; otherwise finish(1)."
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "neie: Negative edge interrupt enable",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies negative-edge interrupt generation and handling for GPIO pins 8–39 including per-pin and group status clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "A bounded timeout (5000 iterations) is used while waiting for the interrupt; comment notes it may be adjusted to the simulation time base.",
    "Test Steps / Procedure": "1) Enable the appropriate system interrupt in INTR_EN1. 2) Configure GP0_GPIO_8 through GP0_GPIO_39 for input and negative-edge detection. 3) For each pin, clear the group raw bit in GPIO_INTR_RAW_STCLR1 and enable the bit in GP0_INTR1_INTR_EN1. 4) Generate a falling edge on the selected pad and wait for the interrupt. 5) In the handler, verify input level is low, group status is set, clear per‑pin raw and group raw, then verify the group status is cleared. 6) Clear RAW_STCR1 and the interrupt controller and proceed to the next pin.",
    "Impacted Registers": "INTR_EN1, gp0_gpio_8, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
    "Validation / Acceptance Criteria": "1) No timeout occurs while waiting for the interrupt after a falling edge; otherwise the test fails. 2) After the edge, the input level reads low; a high level fails. 3) The group status shows the pin set, and after clearing per‑pin and group raw, the group status reads zero; any mismatch fails. 4) The system interrupt status is cleared successfully after service.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad bus high at 0xA0243FFC. Phase 1: For i=0..31, addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) to set doe=1 (input), neie=1, iclr=1. Phase 2: For each i: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); int_pend=1; write_reg(0xA0243FFC, 0xFFFFFFFF); wait; write_reg(0xA0243FFC, ~wr_val) to create falling edge; bounded wait loop up to 5000 iterations while (int_pend). On timeout → print error and test_err++. finish(test_err). Default_IRQHandler(): local_wr=(1<<i); int_pend=0; write_reg(0xA0243FFC, 0xFFFFFFFF) to restore; raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++; write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(87 or 88); } else { test_err++; }",
    "Hidden_Remarks": "// Bounded wait instead of infinite loop; comment: 'adjust to your sim time base if needed'",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC IRQ (87 if GPIO0, 88 if GPIO1). 2) Enable system interrupt source by writing MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR. 3) Initialize pad driver to all-high state by write_reg(0xA0243FFC, 0xFFFFFFFF). 4) Configure per-pin registers: for i=0..31, at MIZAR_GPIO_GP0_GPIO_8+(i*4), write (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (negative-edge enable), and iclr=1 (clear any raw). 5) For each i=0..31: (a) wr_val=1<<i. (b) Clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val). (c) Enable only this pin in group enable: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). (d) Set int_pend=1; (e) Drive all-high then drive bit i low using writes to 0xA0243FFC to create a falling edge; (f) Wait with timeout up to 5000 iterations for int_pend to be cleared by ISR; on timeout, log error and increment test_err. 6) Default_IRQHandler actions: (a) local_wr=(1<<i); int_pend=0; (b) Restore all-high on 0xA0243FFC; (c) Read per-pin reg rdata from MIZAR_GPIO_GP0_GPIO_8+(i*4). (d) Verify DIN low: if (rdata & 0x1)!=0 then test_err++. (e) If (rdata & 0x2)!=0x0 then read group status rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++. (f) Clear per-pin raw by writing (1<<20)|(1<<16) to MIZAR_GPIO_GP0_GPIO_8+(i*4). (g) Clear group raw by write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr). (h) Verify group clear by reading MIZAR_GPIO_GP0_INTR1_INTR_STS1 and expecting 0x0; else test_err++. (i) Clear system raw status via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR) and clear GIC IRQ (87 or 88). Else (raw bit not set), test_err++.",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin, the interrupt must arrive before timeout (5000 loop iterations); otherwise an error is logged and test_err is incremented. 2) In ISR, DIN bit (bit0) of the per-pin register must read 0 after a falling edge; else test_err++. 3) Group status (GP0_INTR1_INTR_STS1) must have the bit set for the active pin when servicing; else test_err++. 4) After clearing per-pin raw (iclr) and group raw (GPIO_INTR_RAW_STCLR1), GP0_INTR1_INTR_STS1 must read 0x0; else test_err++. 5) System raw status in RAW_STCR1 must be cleared for the corresponding GPIO source; if not, error is logged and test_err++."
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "peie: Positive edge interrupt enable",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Verifies positive-edge interrupts across GPIO pins 8–39 with group masking, per-pin raw clear, and system status clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Group interrupt is masked during service and re-enabled afterward. A bounded timeout (2000 iterations) is used while waiting for each interrupt.",
    "Test Steps / Procedure": "1) Enable the appropriate system interrupt in INTR_EN1. 2) Enable positive-edge detection for GP0_GPIO_8 through GP0_GPIO_39. 3) Set input mode using GPIO_IO_CTRL_GROUP1 to GPIO_IO_CTRL_GROUP4. 4) Enable all bits in GP0_INTR1_INTR_EN1. 5) For each pin, drive low, arm wait, then generate a rising edge on the pad. 6) In the handler, mask the group, verify group status is set, clear per‑pin raw for all pins, verify group status clears, clear RAW_STCR1, and re‑enable group and the interrupt controller.",
    "Impacted Registers": "INTR_EN1, gp0_gpio_8, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
    "Validation / Acceptance Criteria": "1) No timeout occurs while waiting after each rising edge; a timeout fails the test. 2) Group status shows a set bit during service and reads zero after raw clears; any mismatch fails. 3) The system raw status is cleared successfully after service.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set peie (positive-edge enable). Sets input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, 0x000000FF). Enables all group interrupts via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For each i=0..31: write_reg(0xA0243FFC, 0x00000000) (low), wait, set int_pend=1, then write_reg(0xA0243FFC, 0xFFFFFFFF) to create a rising edge. Wait up to 2000 iterations for int_pend to clear; on timeout, print error and increment test_err and break. Optionally drive low again for next iteration. Default_IRQHandler(): wr_val=1<<i; int_pend=0; Read group status rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). Mask group during service: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000). If (rdata_grp & 0xFFFFFFFF)!=0 → success log; else print error and test_err++. Clear per-pin raw for all pins: for j=0..31, write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000); wait. Verify group clear: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if rdata_grp!=0 then error++. Clear system raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR); read back MIZAR_LSS_SYSREG_RAW_STCR1 and check bit cleared; else error++. Re-enable group: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Clear GIC IRQ (87 or 88).",
    "Hidden_Remarks": "Group interrupt is masked during service (write 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1) and re-enabled after clearing. Timeout loop uses 2000 iterations.",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC (87 if GPIO0, 88 if GPIO1). 2) Enable system interrupt by writing MIZAR_LSS_SYSREG_INTR_EN1 with the appropriate GPIOx bit. 3) Configure positive-edge detection per pin: for i=0..31, write 0x00020000 to MIZAR_GPIO_GP0_GPIO_8+(i*4). 4) Configure IO mode to input by writing 0x000000FF to MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4. 5) Enable all group interrupts via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 6) For each i=0..31: drive 0x00000000 to 0xA0243FFC, wait, set int_pend=1, drive 0xFFFFFFFF to 0xA0243FFC to generate a rising edge; wait up to 2000 iterations for int_pend==0; on timeout, print error and increment test_err then break; drive low again and wait. 7) Default_IRQHandler: set wr_val=1<<i; int_pend=0; read rdata_grp from MIZAR_GPIO_GP0_INTR1_INTR_STS1; mask group by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1; if (rdata_grp & 0xFFFFFFFF)!=0 then success else error++; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8+(j*4) for j=0..31; wait. Read rdata_grp again; if rdata_grp==0x0 then success else error++; clear system raw status by writing MIZAR_LSS_SYSREG_RAW_STCR1 with the corresponding GPIOx bit; read back and ensure the bit is cleared; else error++; re-enable group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); clear GIC IRQ.",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin, an interrupt must arrive before the 2000-iteration timeout; otherwise log an error and increment test_err. 2) During ISR, group interrupt status (GP0_INTR1_INTR_STS1) must be nonzero; else log error and increment test_err. 3) After writing per-pin raw clears and verifying, GP0_INTR1_INTR_STS1 must read 0x0; else error++. 4) After writing to RAW_STCR1, reading RAW_STCR1 must show the corresponding bit cleared; else error++."
  }
]
"""

# Constants
IP_NAME = os.environ.get("IP_NAME", "GPIO")
OUTPUT_DIR = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan")
ALLOWED_DV = ["Required", "Blank", "Not Required"]
MAIN_ORDER = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description", "Speed", "Mode",
    "Memory Start Offset", "Memory End Offset", "Remarks", "Test Steps / Procedure",
    "Impacted Registers", "Validation / Acceptance Criteria", "Code Generation (Required / Not)"
]
META_COLS = [
    "Hidden_Test_Case_Name", "Hidden_Test_Description", "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure", "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria"
]


def now_ist():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist)


def json_validate_and_normalize(raw_json: str):
    try:
        data = json.loads(raw_json)
    except Exception as e:
        print(f"ERROR: Invalid JSON: {e}")
        sys.exit(1)
    if not isinstance(data, list) or len(data) == 0:
        print("ERROR: JSON must be a non-empty array")
        sys.exit(1)
    # Union of keys preserving first-seen order
    key_order = []
    seen = set()
    for row in data:
        if not isinstance(row, dict):
            print("ERROR: Each array element must be an object")
            sys.exit(1)
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                key_order.append(k)
    # Fill missing with blanks
    normalized = []
    for row in data:
        norm = {}
        for k in key_order:
            norm[k] = row.get(k, "")
        normalized.append(norm)
    return key_order, normalized


def create_base_workbook(headers, rows):
    wb = Workbook()
    # Remove default sheet and create Data
    ws = wb.active
    ws.title = "Data"
    # Write headers
    for c, h in enumerate(headers, start=1):
        ws.cell(row=1, column=c, value=h)
    # Write rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(headers, start=1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ""))
    # Freeze header
    ws.freeze_panes = "A2"
    # Header style
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center")
    header_fill = PatternFill("solid", fgColor="FF4F81BD")
    for c in range(1, len(headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill
    # Approx autofit
    for c, h in enumerate(headers, start=1):
        max_len = max([len(str(h))] + [len(str(rows[r - 2].get(h, ""))) for r in range(2, len(rows) + 2)])
        ws.column_dimensions[chr(64 + c) if c <= 26 else f"A{chr(64 + c - 26)}"].width = min(80, max(12, int(max_len * 1.1)))
    return wb, ws


def create_meta_sheet(wb, rows):
    meta = wb.create_sheet("Meta_data_sheet")
    # Headers
    for c, h in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=c, value=h)
    # Rows
    for r_idx, row in enumerate(rows, start=2):
        for c_idx, h in enumerate(META_COLS, start=1):
            meta.cell(row=r_idx, column=c_idx, value=row.get(h, ""))
    # Very hidden
    meta.sheet_state = 'veryHidden'
    return meta


def normalize_main_sheet(ws, headers):
    # Remove hidden cols from Data (will be renamed to TestPlan)
    visible_headers = [h for h in headers if h not in META_COLS]
    # Reorder as per MAIN_ORDER
    # Include only those present in visible_headers, in specified order
    ordered = [h for h in MAIN_ORDER if h in visible_headers]
    # Add any extra visible headers (should not exist) to the end
    extra = [h for h in visible_headers if h not in ordered]
    final_headers = ordered + extra

    # Build a mapping from old col index to header
    header_row = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Create a new row list content for final order
    data_matrix = []
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for h in final_headers:
            c_old = header_row.get(h)
            val = ws.cell(row=r, column=c_old).value if c_old else ""
            row_vals.append(val)
        data_matrix.append(row_vals)

    # Clear sheet and write final headers and data
    ws.delete_rows(1, ws.max_row)
    for c, h in enumerate(final_headers, start=1):
        ws.cell(row=1, column=c, value=h)
    for r_idx, r_vals in enumerate(data_matrix, start=2):
        for c_idx, v in enumerate(r_vals, start=1):
            ws.cell(row=r_idx, column=c_idx, value=v)

    # Apply formatting to TestPlan
    header_font = Font(bold=True)
    header_align = Alignment(horizontal="center", vertical="center")
    header_fill = PatternFill("solid", fgColor="FF4F81BD")
    for c in range(1, len(final_headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.alignment = header_align
        cell.fill = header_fill

    # Text wrap for specified columns
    wrap_cols = {"Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"}
    thin = Side(style="thin", color="FF000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)

    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            header = ws.cell(row=1, column=c).value
            # Alignment
            if header == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=(header in wrap_cols))
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=(header in wrap_cols))
            cell.border = border

    # Autofit columns
    for c, h in enumerate(final_headers, start=1):
        col_letter = ws.cell(row=1, column=c).column_letter
        max_len = max([len(str(h))] + [len(str(ws.cell(row=r, column=c).value or "")) for r in range(2, ws.max_row + 1)])
        ws.column_dimensions[col_letter].width = min(120, max(14, int(max_len * 1.1)))

    # Row heights auto (approx): let Excel auto-calc

    # Numbering inside cells for Steps and Validation: unify to 1., 2., ... if patterns like 1) exist
    def renumber(text: str) -> str:
        if not isinstance(text, str) or not text.strip():
            return text
        t = re.sub(r"(\d+)\)\s*", r"\1. ", text)
        return t

    # Apply numbering
    head_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    for field in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if field in head_idx:
            c = head_idx[field]
            for r in range(2, ws.max_row + 1):
                val = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=renumber(val))

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in head_idx:
        c = head_idx["Code Generation (Required / Not)"]
        start_row = 2
        end_row = ws.max_row
        col_letter = ws.cell(row=1, column=c).column_letter
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True)
        ws.add_data_validation(dv)
        dv.add(f"{col_letter}{start_row}:{col_letter}{end_row}")

    # Freeze header
    ws.freeze_panes = "A2"

    return final_headers


def finalize_and_save(wb):
    # Safety check: Only TestPlan (visible) and Meta_data_sheet (veryHidden)
    # Rename Data -> TestPlan
    if wb.active.title == "Data":
        wb.active.title = "TestPlan"
    # Ensure no sheet named Data remains
    for sh in list(wb.sheetnames):
        if sh == "Data":
            ws_del = wb[sh]
            wb.remove(ws_del)
    # Validate allowed
    names = wb.sheetnames
    if "TestPlan" not in names or "Meta_data_sheet" not in names or len(names) != 2:
        print(f"ERROR: Sheet visibility/allowed set invalid: {names}")
        sys.exit(2)

    ts = now_ist().strftime("%Y%m%d_%H%M%S")
    out_dir = OUTPUT_DIR
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"{IP_NAME}_TestPlan_{ts}.xlsx")
    wb.save(out_path)

    # Validate as true XLSX (ZIP with expected parts)
    try:
        with zipfile.ZipFile(out_path, 'r') as zf:
            if not any(name == "[Content_Types].xml" for name in zf.namelist()):
                raise RuntimeError("Not a valid Office Open XML package")
    except Exception as e:
        print(f"ERROR: XLSX validation failed: {e}")
        sys.exit(2)

    print(out_path)
    return out_path


def main():
    headers, rows = json_validate_and_normalize(json_data)
    wb, ws = create_base_workbook(headers, rows)
    create_meta_sheet(wb, rows)
    # Normalize main sheet in place (currently named Data)
    normalize_main_sheet(ws, headers)
    # Rename Data -> TestPlan and finalize
    saved = finalize_and_save(wb)
    print(f"Saved: {saved}")

if __name__ == "__main__":
    main()
