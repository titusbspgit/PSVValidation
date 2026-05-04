import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import zipfile

# Embedded JSON array of rows (authoritative input)
JSON_DATA = [
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
    "Test Steps / Procedure": "Read default values from per-pin and group registers and compare against expected resets using read masks.||For each writable register, write test patterns masked by the write mask.||Read back values using read masks and compute expected values with non-writable bits preserved.||Report failures for any default-value mismatch or write-read mismatch and summarize result.",
    "Impacted Registers": "GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GP0_INTR2_INTR_EN1, GP0_INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
    "Validation / Acceptance Criteria": "Default values: For each address, (read_data & 0xFFFFFFFE) equals the expected default value.||Write-read check: For each writable and readable address, read_data equals ((write_data & read_mask & write_mask) | (~write_mask & read_mask & default_value)).||Overall result: Test passes only if both default-check and write-read mismatch counters are zero.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "program.c calls chk_rst_val() then chk_rd_wr(). chk_rst_val(): For i=0..CNT-1, addr=addr_array[i]; if skip_rst_array[i]==1 → continue; if read_mask_array[i]==0 → continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); if data==default_value_array[i] → PASS else def_fail_cnt++ and print failure. chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; For each pattern j: data_wr=chk_val[j]; Writing phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; else write_reg(addr, (data_wr & write_mask_array[i])). Reading phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; if read_mask_array[i]==0 → continue; else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xFFFFFFFF); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val → PASS else wr_fail_cnt++ and print failure. At end of test_case(), if def_fail_cnt>0 or wr_fail_cnt>0 finish(1) else finish(0).",
    "Hidden_Remarks": "//80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers\n//when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
    "Hidden_Test_Steps_Procedure": "1) Define arrays (addr_array[49], default_value_array[49], read_mask_array[49], write_mask_array[49], skip_array[49], skip_rst_array[49]) in test_define.c with register macros MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4, and associated mask/default macros. 2) chk_rst_val(): Loop i=0..48: addr=addr_array[i]; if skip_rst_array[i]==1 → continue; if read_mask_array[i]==0x00000000 → continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); compare data with default_value_array[i]; if equal → optional PASS print; else increment def_fail_cnt and print detailed mismatch including addr, expected, read_data, raw data. 3) chk_rd_wr(): Define chk_val[6]={0xFFFFFFFF,0xAAAAAAAA,0x55555555,0xF5F5F5F5,0xA5A5A5A5,0xFFFF0000}. For each j in 0..5: data_wr=chk_val[j]. 3a) Write phase: for i=0..48: addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; else write_reg(addr, (data_wr & write_mask_array[i])) with optional debug print. 3b) Read/verify phase: for i=0..48: addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; if read_mask_array[i]==0x00000000 → continue; else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xFFFFFFFF); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val → optional PASS print; else wr_fail_cnt++ and print mismatch including addr and values. 4) In test_case(): call chk_rst_val(); call chk_rd_wr(); if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). 5) soft_reset_chk() is compiled out (#ifdef 0) and not executed.",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "1) In chk_rst_val(): For each non-skipped, readable address, (read_reg(addr) & 0xFFFFFFFE) must equal default_value_array[i]; else def_fail_cnt++. 2) In chk_rd_wr(): For each non-skipped, writable and readable address, read_reg(addr) masked by read_mask must equal exp_val=((data_wr & read_mask & write_mask) | (~write_mask & read_mask & default_value)); else wr_fail_cnt++. 3) Final decision: finish(0) only if def_fail_cnt==0 and wr_fail_cnt==0; otherwise finish(1).",
    "Source_Link_Name": "gpio_reg_wr_rd_test",
    "Source_Link_URL": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/gpio_reg_wr_rd_test"
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "Negative edge interrupt enable",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies negative-edge interrupt generation and handling for GPIO pins 8–39 including per-pin and group status clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243FFC",
    "Memory End Offset": "0xA0243FFC",
    "Remarks": "A bounded timeout (5000 iterations) is used while waiting for the interrupt; comment notes it may be adjusted to the simulation time base.",
    "Test Steps / Procedure": "Enable the appropriate system interrupt in INTR_EN1 and GIC.||Configure GP0_GPIO_8 through GP0_GPIO_39 for input and negative-edge detection; clear raw.||For each pin, clear the group raw bit in GPIO_INTR_RAW_STCLR1 and enable the bit in GP0_INTR1_INTR_EN1; generate a falling edge and wait for IRQ.||In the ISR, verify input level is low and group status is set; clear per-pin raw and group raw; clear system RAW; re-enable if masked.",
    "Impacted Registers": "INTR_EN1, GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
    "Validation / Acceptance Criteria": "No timeout occurs while waiting for the interrupt after a falling edge.||After the edge, the input level reads low; group status shows the pin set.||After clearing per‑pin raw and group raw, the group status reads zero.||The system interrupt status is cleared successfully after service.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad bus high at 0xA0243FFC. Phase 1: For i=0..31, addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) to set doe=1 (input), neie=1, iclr=1. Phase 2: For each i: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); int_pend=1; write_reg(0xA0243FFC, 0xFFFFFFFF); wait; write_reg(0xA0243FFC, ~wr_val) to create falling edge; bounded wait loop up to 5000 iterations while (int_pend). On timeout → print error and test_err++. finish(test_err). Default_IRQHandler(): local_wr=(1<<i); int_pend=0; write_reg(0xA0243FFC, 0xFFFFFFFF) to restore; raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++; write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(87 or 88); } else { test_err++; }",
    "Hidden_Remarks": "// Bounded wait instead of infinite loop; comment: 'adjust to your sim time base if needed'",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC IRQ (87 if GPIO0, 88 if GPIO1). 2) Enable system interrupt source by writing MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR. 3) Initialize pad driver to all-high state by write_reg(0xA0243FFC, 0xFFFFFFFF). 4) Configure per-pin registers: for i=0..31, at MIZAR_GPIO_GP0_GPIO_8+(i*4), write (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (negative-edge enable), and iclr=1 (clear any raw). 5) For each i=0..31: (a) wr_val=1<<i. (b) Clear group raw: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val). (c) Enable only this pin in group enable: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). (d) Set int_pend=1; (e) Drive all-high then drive bit i low using writes to 0xA0243FFC to create a falling edge; (f) Wait with timeout up to 5000 iterations for int_pend to be cleared by ISR; on timeout, log error and increment test_err. 6) Default_IRQHandler actions: (a) local_wr=(1<<i); int_pend=0; (b) Restore all-high on 0xA0243FFC; (c) Read per-pin reg rdata from MIZAR_GPIO_GP0_GPIO_8+(i*4). (d) Verify DIN low: if (rdata & 0x1)!=0 then test_err++. (e) If (rdata & 0x2)!=0x0 then read group status rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++. (f) Clear per-pin raw by writing (1<<20)|(1<<16) to MIZAR_GPIO_GP0_GPIO_8+(i*4). (g) Clear group raw by write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr). (h) Verify group clear by reading MIZAR_GPIO_GP0_INTR1_INTR_STS1 and expecting 0x0; else test_err++. (i) Clear system raw status via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR) and clear GIC IRQ (87 or 88). Else (raw bit not set), test_err++.",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin, the interrupt must arrive before timeout (5000 loop iterations); otherwise an error is logged and test_err is incremented. 2) In ISR, DIN bit (bit0) of the per-pin register must read 0 after a falling edge; else test_err++. 3) Group status (GP0_INTR1_INTR_STS1) must have the bit set for the active pin when servicing; else test_err++. 4) After clearing per-pin raw (iclr) and group raw (GPIO_INTR_RAW_STCLR1), GP0_INTR1_INTR_STS1 must read 0x0; else test_err++. 5) System raw status in RAW_STCR1 must be cleared for the corresponding GPIO source; if not, error is logged and test_err++.",
    "Source_Link_Name": "test_gpio_negedge_intr_en",
    "Source_Link_URL": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_negedge_intr_en"
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Positive edge interrupt enable",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Verifies positive-edge interrupts across GPIO pins 8–39 with group masking, per-pin raw clear, and system status clear.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243FFC",
    "Memory End Offset": "0xA0243FFC",
    "Remarks": "Group interrupt is masked during service and re-enabled afterward. A bounded timeout (2000 iterations) is used while waiting for each interrupt.",
    "Test Steps / Procedure": "Enable the appropriate system interrupt in INTR_EN1 and GIC.||Enable positive-edge detection on GP0_GPIO_8..39; configure group IO control for inputs.||Enable all bits in group interrupt enable.||For each pin: drive low, arm wait, drive high to create rising edge; wait for IRQ.||In the ISR: mask the group, verify group STS, clear per-pin raw across pins, verify group STS clears, clear RAW_STCR1, and re-enable group and controller.",
    "Impacted Registers": "INTR_EN1, GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
    "Validation / Acceptance Criteria": "No timeout occurs while waiting after each rising edge.||Group status shows a set bit during service and reads zero after raw clears.||The system raw status is cleared successfully after service.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set peie (positive-edge enable). Sets input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, 0x000000FF). Enables all group interrupts via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For each i=0..31: write_reg(0xA0243FFC, 0x00000000) (low), wait, set int_pend=1, then write_reg(0xA0243FFC, 0xFFFFFFFF) to create a rising edge. Wait up to 2000 iterations for int_pend to clear; on timeout, print error and increment test_err and break. Optionally drive low again for next iteration. Default_IRQHandler(): wr_val=1<<i; int_pend=0; Read group status rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). Mask group during service: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000). If (rdata_grp & 0xFFFFFFFF)!=0 → success log; else print error and test_err++. Clear per-pin raw for all pins: for j=0..31, write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000); wait. Verify group clear: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if rdata_grp!=0 then error++. Clear system raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR); read back MIZAR_LSS_SYSREG_RAW_STCR1 and check bit cleared; else error++. Re-enable group: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Clear GIC IRQ (87 or 88).",
    "Hidden_Remarks": "Group interrupt is masked during service (write 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1) and re-enabled after clearing. Timeout loop uses 2000 iterations.",
    "Hidden_Test_Steps_Procedure": "1) Conditionally enable GIC (87 if GPIO0, 88 if GPIO1). 2) Enable system interrupt by writing MIZAR_LSS_SYSREG_INTR_EN1 with the appropriate GPIOx bit. 3) Configure positive-edge detection per pin: for i=0..31, write 0x00020000 to MIZAR_GPIO_GP0_GPIO_8+(i*4). 4) Configure IO mode to input by writing 0x000000FF to MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4. 5) Enable all group interrupts via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 6) For each i=0..31: drive 0x00000000 to 0xA0243FFC, wait, set int_pend=1, drive 0xFFFFFFFF to 0xA0243FFC to generate a rising edge; wait up to 2000 iterations for int_pend==0; on timeout, print error and increment test_err then break; drive low again and wait. 7) Default_IRQHandler: set wr_val=1<<i; int_pend=0; read rdata_grp from MIZAR_GPIO_GP0_INTR1_INTR_STS1; mask group by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1; if (rdata_grp & 0xFFFFFFFF)!=0 then success else error++; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8+(j*4) for j=0..31; wait. Read rdata_grp again; if rdata_grp==0x0 then success else error++; clear system raw status by writing MIZAR_LSS_SYSREG_RAW_STCR1 with the corresponding GPIOx bit; read back and ensure the bit is cleared; else error++; re-enable group via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); clear GIC IRQ.",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin, an interrupt must arrive before the 2000-iteration timeout; otherwise log an error and increment test_err. 2) During ISR, group interrupt status (GP0_INTR1_INTR_STS1) must be nonzero; else log error and increment test_err. 3) After writing per-pin raw clears and verifying, GP0_INTR1_INTR_STS1 must read 0x0; else error++. 4) After writing to RAW_STCR1, reading RAW_STCR1 must show the corresponding bit cleared; else error++.",
    "Source_Link_Name": "test_gpio_pedge_all_pads_en",
    "Source_Link_URL": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_pedge_all_pads_en"
  }
]

# Constants
MAIN_COLUMNS_ORDER = [
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

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

ALLOWED_DV_VALUES = ["Required", "Blank", "Not Required"]

# Utility: convert multi-item text into numbered lines (1., 2., ...)
def to_numbered_block(text: str) -> str:
    if not text:
        return ""
    # Prefer custom '||' delimiter; fallback to newline; fallback to '. ' only if it seems like a list
    items = None
    if "||" in text:
        items = [x.strip() for x in text.split("||") if x.strip()]
    elif "\n" in text:
        items = [x.strip() for x in text.splitlines() if x.strip()]
    elif ";" in text:
        items = [x.strip() for x in text.split(";") if x.strip()]
    else:
        # Single item
        items = [text.strip()]
    return "\n".join(f"{i+1}. {item}" for i, item in enumerate(items))

# Phase 1 — Validate JSON Input
if not isinstance(JSON_DATA, list) or len(JSON_DATA) == 0:
    raise SystemExit("ERROR: JSON input must be a non-empty array")

# Build union of keys preserving first-seen order
seen_keys = []
for row in JSON_DATA:
    if not isinstance(row, dict):
        raise SystemExit("ERROR: Each JSON record must be an object")
    for k in row.keys():
        if k not in seen_keys:
            seen_keys.append(k)

# Phase 1 — Generate base workbook with single 'Data' sheet
wb = Workbook()
ws = wb.active
ws.title = "Data"

# Write header
for col_idx, key in enumerate(seen_keys, start=1):
    cell = ws.cell(row=1, column=col_idx, value=key)
    cell.font = Font(bold=True)

# Write rows (fill missing keys with blanks)
for r_idx, row in enumerate(JSON_DATA, start=2):
    for c_idx, key in enumerate(seen_keys, start=1):
        ws.cell(row=r_idx, column=c_idx, value=row.get(key, ""))

# Freeze top row
ws.freeze_panes = "A2"

# Simple autofit approximation
for c_idx, key in enumerate(seen_keys, start=1):
    col_letter = get_column_letter(c_idx)
    max_len = len(str(key))
    for r in range(2, ws.max_row + 1):
        v = ws.cell(row=r, column=c_idx).value
        if v is None:
            continue
        max_len = max(max_len, len(str(v)))
    ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 80)

# Step 5 — Create META sheet and copy META columns AS-IS
meta_ws = wb.create_sheet("Meta_data_sheet")
# Header
for col_idx, key in enumerate(META_COLUMNS, start=1):
    meta_ws.cell(row=1, column=col_idx, value=key).font = Font(bold=True)
# Rows
for r_idx, row in enumerate(JSON_DATA, start=2):
    for c_idx, key in enumerate(META_COLUMNS, start=1):
        meta_ws.cell(row=r_idx, column=c_idx, value=row.get(key, ""))

# Add Change Log block
change_log_start = len(META_COLUMNS) + 3
meta_ws.cell(row=1, column=change_log_start, value="Change Log").font = Font(bold=True)
meta_ws.cell(row=2, column=change_log_start, value="Agent").font = Font(bold=True)
meta_ws.cell(row=2, column=change_log_start + 1, value="Ag-Emb-Mpsoc-Stage1 Agent")
meta_ws.cell(row=3, column=change_log_start, value="Timestamp (IST)").font = Font(bold=True)
now_ist = datetime.now(ZoneInfo("Asia/Kolkata")).strftime("%Y-%m-%d %H:%M:%S %Z")
meta_ws.cell(row=3, column=change_log_start + 1, value=now_ist)
meta_ws.cell(row=4, column=change_log_start, value="Source Subdirectory").font = Font(bold=True)
meta_ws.cell(row=4, column=change_log_start + 1, value="TestRepo/gpio")
meta_ws.cell(row=5, column=change_log_start, value="Repository").font = Font(bold=True)
meta_ws.cell(row=5, column=change_log_start + 1, value="https://github.com/titusbspgit/PSVValidation")

# Add Source Links section (preserved as clickable links)
links_start_row = 7
meta_ws.cell(row=links_start_row, column=change_log_start, value="Source Links").font = Font(bold=True)
row_ptr = links_start_row + 1
for row in JSON_DATA:
    name = row.get("Source_Link_Name", "")
    url = row.get("Source_Link_URL", "")
    if name and url:
        cell = meta_ws.cell(row=row_ptr, column=change_log_start, value=name)
        cell.hyperlink = url
        cell.style = "Hyperlink"
        row_ptr += 1

# Very Hidden meta sheet
meta_ws.sheet_state = 'veryHidden'

# Step 7 — Normalize MAIN sheet in place: rename and reorder/remove columns
ws.title = "TestPlan"

# Build map of current columns
current_headers = [ws.cell(1, c).value for c in range(1, ws.max_column + 1)]
# Indices of META columns to remove
meta_indices = [current_headers.index(k) + 1 for k in current_headers if k in META_COLUMNS or k in ("Source_Link_Name", "Source_Link_URL")]
# Remove META and Source Link columns from right to left
for idx in sorted(meta_indices, reverse=True):
    ws.delete_cols(idx, 1)

# Reorder remaining to MAIN_COLUMNS_ORDER
# After deletion, rebuild headers
headers_after = [ws.cell(1, c).value for c in range(1, ws.max_column + 1)]
# Create a new ordered list of columns by moving columns to match target order
target_order = [col for col in MAIN_COLUMNS_ORDER if col in headers_after]
# Append any remaining columns (if any) preserving order
for h in headers_after:
    if h not in target_order:
        target_order.append(h)

# Create a mapping from header to current index
header_to_index = {h: (headers_after.index(h) + 1) for h in headers_after}

# Create a new ordered set of columns by inserting temporary at end and copying values
# We'll build a list of rows representing the reordered table, then write back
rows_data = []
for r in range(2, ws.max_row + 1):
    row_dict = {}
    for h in headers_after:
        row_dict[h] = ws.cell(row=r, column=header_to_index[h]).value
    rows_data.append(row_dict)

# Clear existing sheet and write headers in target order
ws.delete_rows(1, ws.max_row)
for c_idx, h in enumerate(target_order, start=1):
    cell = ws.cell(row=1, column=c_idx, value=h)
    cell.font = Font(bold=True)

for r_idx, row in enumerate(rows_data, start=2):
    for c_idx, h in enumerate(target_order, start=1):
        ws.cell(row=r_idx, column=c_idx, value=row.get(h, ""))

# Step 7A — FORMAT MAIN SHEET
wrap_cols = ["Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"]
blue_fill = PatternFill(start_color="FF4472C4", end_color="FF4472C4", fill_type="solid")
center = Alignment(horizontal="center", vertical="center", wrap_text=False)
wrap_align = Alignment(horizontal="left", vertical="top", wrap_text=True)
left_top = Alignment(horizontal="left", vertical="top", wrap_text=False)
right_top = Alignment(horizontal="right", vertical="top")
thin = Side(border_style="thin", color="FF000000")
border = Border(left=thin, right=thin, top=thin, bottom=thin)

# Header formatting
for c in range(1, ws.max_column + 1):
    cell = ws.cell(row=1, column=c)
    cell.font = Font(bold=True)
    cell.alignment = center
    cell.fill = blue_fill

# Numbering inside cells for specific columns
header_row = [ws.cell(1, c).value for c in range(1, ws.max_column + 1)]
col_index_map = {h: i+1 for i, h in enumerate(header_row)}
for r in range(2, ws.max_row + 1):
    # Test Steps / Procedure
    c = col_index_map.get("Test Steps / Procedure")
    if c:
        val = ws.cell(row=r, column=c).value
        ws.cell(row=r, column=c, value=to_numbered_block(val))
    # Validation / Acceptance Criteria
    c2 = col_index_map.get("Validation / Acceptance Criteria")
    if c2:
        val2 = ws.cell(row=r, column=c2).value
        ws.cell(row=r, column=c2, value=to_numbered_block(val2))

# Apply wrap alignment and borders
for r in range(1, ws.max_row + 1):
    for c in range(1, ws.max_column + 1):
        h = ws.cell(1, c).value
        cell = ws.cell(row=r, column=c)
        if r == 1:
            cell.alignment = center
        else:
            if h in wrap_cols:
                cell.alignment = wrap_align
            elif h == "Index":
                cell.alignment = center
            elif h in ("Speed", "Mode", "Memory Start Offset", "Memory End Offset"):
                cell.alignment = left_top
            else:
                cell.alignment = left_top
        cell.border = border

# Autofit columns (approx) and set row heights auto
for c in range(1, ws.max_column + 1):
    header = ws.cell(1, c).value or ""
    max_len = len(str(header))
    for r in range(2, ws.max_row + 1):
        v = ws.cell(r, c).value
        if v is None:
            continue
        max_len = max(max_len, len(str(v)))
    ws.column_dimensions[get_column_letter(c)].width = min(max(12, max_len * 0.9 + 4), 100)

# Freeze header row on TestPlan
ws.freeze_panes = "A2"

# Data validation ONLY for Code Generation (Required / Not)
code_col = col_index_map.get("Code Generation (Required / Not)")
if code_col:
    dv_list = ", ".join(ALLOWED_DV_VALUES)
    dv = DataValidation(type="list", formula1=f'"{dv_list}"', allow_blank=True)
    ws.add_data_validation(dv)
    dv_range = f"{get_column_letter(code_col)}2:{get_column_letter(code_col)}{ws.max_row}"
    dv.add(dv_range)

# Step 7B — Safety check: only TestPlan (visible) + Meta_data_sheet (Very Hidden)
# Ensure no sheet named 'Data'
for s in wb.sheetnames:
    if s == "Data":
        # If present, delete and fail-safe will be implicit by keeping only two sheets
        idx = wb.sheetnames.index(s)
        wb.remove(wb.worksheets[idx])
        raise SystemExit("ERROR: Safety check failed; 'Data' sheet still present after normalization.")

# Phase 3 — Save with IST timestamp and validate ZIP structure
ip_name = os.getenv("IP_NAME", "GPIO")
output_dir = os.getenv("OUTPUT_DIR", "Test_Output/GPIO/TestPlan/")
os.makedirs(output_dir, exist_ok=True)

now = datetime.now(ZoneInfo("Asia/Kolkata"))
file_name = f"{ip_name}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"
file_path = os.path.join(output_dir, file_name)

wb.save(file_path)

# Validate OOXML ZIP-based workbook
required_parts = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
with zipfile.ZipFile(file_path, 'r') as zf:
    names = set(zf.namelist())
    missing = [p for p in required_parts if p not in names]
    if missing:
        raise SystemExit(f"ERROR: XLSX validation failed; missing parts: {missing}")

print(f"SUCCESS: Generated {file_path}")
