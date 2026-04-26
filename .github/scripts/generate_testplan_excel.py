#!/usr/bin/env python3
import json
import argparse
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    ZoneInfo = None
import os
from copy import deepcopy
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

JSON_DATA = r'''{ "ip": "GPIO", "summary": { "total_test_cases": 3, "processing_order": [ "gpio_reg_wr_rd_test", "test_gpio_negedge_intr_en", "test_gpio_pedge_all_pads_en" ], "folders": [ { "name": "gpio_reg_wr_rd_test", "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/gpio_reg_wr_rd_test" }, { "name": "test_gpio_negedge_intr_en", "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_negedge_intr_en" }, { "name": "test_gpio_pedge_all_pads_en", "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_pedge_all_pads_en" } ] }, "test_cases": [ { "Index": 1, "SS / Module": "GPIO", "Feature": "GPIO CSR reset defaults and R/W-masked behavior", "Test Case Name": "gpio_reg_wr_rd_test", "Test Description": "Validates that GPIO control/status registers hold expected reset values and support masked write/read behavior across the defined address list.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Certain registers are intentionally skipped for write/read and reset checks per skip arrays. Note: when reading default values, input data may float high unless externally driven; forcing the input low can alter level-select behavior and affect expected values.", "Test Steps / Procedure": [ "Entry point function executes and initializes internal failure counters to zero.", "Default-value phase: Iterate over the register list (count = 49). For each index, compute the target register from the fixed list.", "If the current register is marked to skip reset verification, continue to the next index without access.", "If the register read mask is zero, consider it not readable for default check and continue to the next index.", "READ operation: Access the targeted register (by name from the list) and mask the read value to exclude the least significant bit.", "Compare the masked read value against the expected reset value from the table; on mismatch, increment the default-failure counter and print details.", "Write/read phase: Prepare a sequence of six data patterns to exercise masked behavior.", "For each data pattern, iterate across the same register list (count = 49).", "If the current register is marked to skip write/read, continue to the next index.", "If the write mask for the current register is zero, treat it as not writable and continue.", "WRITE operation: Write the current pattern masked by the register’s write mask to the targeted register.", "After completing masked writes for the current data pattern, iterate again across the list to verify readback.", "If the current register is marked to skip write/read, continue.", "If the write mask is zero or the read mask is zero, skip the readback verification for this register.", "READ operation: Read the targeted register and apply the register’s read mask to the observed value.", "Compute the expected value as a combination of the written bits (within both read and write masks) and preserved reset bits (read mask AND inverse of write mask).", "Compare the observed masked read value against the expected value; on mismatch, increment the write/read-failure counter and print details.", "Completion: If either failure counter is non-zero, terminate with failure status; otherwise terminate with success status." ], "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GP0_INTR2_INTR_EN1, GPIO_GP0_INTR2_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GPIO_IO_CTRL_GROUP4, GPIO_GPIO_DOUT_GROUP1, GPIO_GPIO_DOUT_GROUP2, GPIO_GPIO_DOUT_GROUP3, GPIO_GPIO_DOUT_GROUP4, GPIO_GPIO_DIN_GROUP1, GPIO_GPIO_DIN_GROUP2, GPIO_GPIO_DIN_GROUP3, GPIO_GPIO_DIN_GROUP4", "Validation / Acceptance Criteria": [ "For each register included in reset verification, the masked read value equals the specified default for that register.", "For each register and each pattern included in write/read verification, the observed masked readback equals the expected combination of written bits within the write mask and preserved reset bits outside the write mask.", "Final pass condition: both the default-value failure counter and the write/read failure counter remain zero at test completion." ], "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test", "Hidden_Test_Description": "program.c runs test_case() which calls chk_rst_val() then chk_rd_wr(). chk_rst_val() loops i=0..CNT-1 over addr_array[]; if skip_rst_array[i]==1 continue; if read_mask_array[i]==0 continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); compare data==default_value_array[i]. chk_rd_wr() iterates j over 6 patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000); for each i=0..CNT-1: if skip_array[i]==1 continue; if write_mask_array[i]==0 continue; write_reg(addr,(data_wr & write_mask_array[i])); then readback phase: if skip_array[i]==1 or write_mask_array[i]==0 or read_mask_array[i]==0 continue; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare data_rd==exp_val. At end: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).", "Hidden_Remarks": "test_define.c: //80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers. const unsigned int skip_array[49]={...}; const unsigned int skip_rst_array[49]={...}; Note: //when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value", "Hidden_Test_Steps_Procedure": "Entry: test_case(); chk_rst_val(); // default check loop over addr_array (49 entries: includes MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4). For each i: if(skip_rst_array[i]==1) continue; if(read_mask_array[i]==0) continue; data_rd=read_reg(addr_array[i]); data=(data_rd & 0xfffffffe); if(data==default_value_array[i]) PASS else {def_fail_cnt++; print fail}. chk_rd_wr(): unsigned int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; For each j: data_wr=chk_val[j]; For each i: if(skip_array[i]==1) continue; if(write_mask_array[i]==0) continue; write_reg(addr_array[i], (data_wr & write_mask_array[i])); Readback loop For each i: if(skip_array[i]==1) continue; if(write_mask_array[i]==0) continue; if(read_mask_array[i]==0) continue; data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd==exp_val) PASS else {wr_fail_cnt++; print fail}. End: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0).", "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4", "Hidden_Validation_Acceptance_Criteria": "Default check: if((read_reg(addr_array[i]) & 0xfffffffe) != default_value_array[i]) def_fail_cnt++. R/W check: data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd != exp_val) wr_fail_cnt++; Final: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0)." }, { "Index": 2, "SS / Module": "GPIO", "Feature": "interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.", "Test Case Name": "test_gpio_negedge_intr_en", "Test Description": "Configures all targeted GPIOs as inputs with negative-edge interrupt enabled, generates a falling edge per pin, and validates raw and group interrupt behavior and clearing via system interrupt handling.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "System interrupt output enable and GIC line selection depend on the build-time selection of the GPIO instance. A bounded wait with a configurable timeout is used to avoid infinite loops; the comment indicates the timeout may be adjusted to the simulation time base.", "Test Steps / Procedure": [ "Entry point function executes and resets the error counter.", "Enable the platform interrupt controller line corresponding to the selected GPIO instance.", "Enable the system-register interrupt output line for the selected GPIO instance.", "Drive the external pad driver register to set all pads to logic high as an initial known state.", "Configuration loop (32 pins): For each of GPIO_8 through GPIO_39, WRITE to the per-pin control register to set input mode, enable negative-edge interrupt, and request clear of any latched raw status; include a short wait after each write.", "Per-pin test loop (32 pins): For current pin index, compute a one-hot bit mask.", "Pre-clear any prior raw status at the group raw-status-clear register using the one-hot mask (WRITE).", "Enable the group interrupt for only the current pin by writing the one-hot mask to the group enable register (WRITE), then wait briefly.", "Arm the ISR wait flag and generate a falling edge for the current pin by first driving the pad driver all high, waiting briefly, then driving low on only the current pin.", "Wait for the interrupt service using a bounded loop with timeout; the loop inserts periodic short waits between checks.", "If the timeout expires with no service, record an error for this pin and continue.", "On interrupt service: The handler returns the pad driver to all high. It READs the current per-pin register and checks that the input data bit reflects low after the falling edge.", "The handler validates that the per-pin raw status is asserted; it then READs the group masked status register and checks that the bit for the current pin is set.", "The handler clears the per-pin raw status using the per-pin control register (WRITE) while keeping input mode set, and also clears the corresponding group raw status bit (WRITE to the group raw-status-clear register).",
 "The handler verifies by READing the group masked status that all bits are cleared.", "The handler clears the corresponding system-register raw interrupt status bit and clears the interrupt controller pending state for the selected line." ], "Impacted Registers": "INTR_EN1, GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, RAW_STCR1", "Validation / Acceptance Criteria": [ "For each pin under test, an interrupt is serviced before the configured timeout expires.", "Within the service, the input data field for the serviced pin indicates a low value following the falling edge.", "During service, the per-pin raw status is active and the group masked status indicates the current pin.", "After clearing, the group masked status reads as zero.", "The system-register raw interrupt status bit for the selected GPIO instance is cleared after service.", "Final pass condition: the accumulated error counter remains zero at test completion." ], "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en", "Hidden_Test_Description": "program.c: test_case() sets test_err=0; ifdef GPIO0 GIC_EnableIRQ(87); ifdef GPIO1 GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR/…GPIO1_INTR). write_reg(0xA0243ffc, 0xffffffff). For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10). For i=0..31: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0){printf(\"ERROR: Timeout ...\"); test_err++;}. finish(test_err). ISR Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata=read_reg(raddr); if((rdata & 0x1) != 0) test_err++; if((rdata & 0x2) != 0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr) == 0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp != 0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }", "Hidden_Remarks": "Conditional IRQ line: GPIO0 uses GIC ID 87; GPIO1 uses GIC ID 88. Uses external address 0xA0243ffc to drive pads. Timeout variable comment: \"adjust to your sim time base if needed\".", "Hidden_Test_Steps_Procedure": "Entry: test_case(); GIC_EnableIRQ(87/88). write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). write_reg(0xA0243ffc, 0xffffffff). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10). For i=0..31: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0){printf(\"ERROR: Timeout ...\"); test_err++;}. finish(test_err). ISR Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata=read_reg(raddr); if((rdata & 0x1) != 0) test_err++; if((rdata & 0x2) != 0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr) == 0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp != 0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4", "Hidden_Validation_Acceptance_Criteria": "Timeout path: if(timeout==0) { printf(\"ERROR: Timeout waiting for GPIO%u negedge interrupt\", i+8); test_err++; }. ISR: if((rdata & 0x1)!=0) test_err++; if((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr)==0) test_err++; write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0) test_err++; clear sysreg raw and GIC. } else { test_err++; } finish(test_err)." }, { "Index": 3, "SS / Module": "GPIO", "Feature": "peie (Reset: 0x0, Type: R/W)", "Test Case Name": "test_gpio_pedge_all_pads_en", "Test Description": "Enables positive-edge interrupt on all targeted GPIOs in input mode, generates a rising edge sequentially on each pin, and validates group interrupt indication and proper clearing, including system-register status handling.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "The handler masks the group enable during service, clears per-pin raw status across all pins, and re-enables the group for subsequent iterations. A bounded timeout is used to avoid infinite waiting.", "Test Steps / Procedure": [ "Entry point function executes and enables the appropriate interrupt controller line based on the selected GPIO instance.", "Enable the system-register interrupt output for the selected GPIO instance.", "Configuration: For each of GPIO_8 through GPIO_39, WRITE to the per-pin control register to enable positive-edge interrupt.", "Set all targeted GPIOs to input mode using the four group IO control registers (WRITE).", "Enable group interrupt for all pins by writing an all-ones mask to the group enable register (WRITE).", "Per-pin sequence loop (32 pins): For each index, prepare a clean low level on the external pad driver register, insert a short wait, arm the ISR wait flag, and drive the pad driver high to generate a single rising edge.", "Wait for interrupt service using a bounded loop with periodic short waits; if the timeout expires, record an error and break out of the loop.", "After service for a pin, optionally return the external pad driver to low level in preparation for the next pin.", "In the handler: READ the group masked status register; temporarily mask the group enable by writing zero to the group enable register.", "If any group status bit is set, proceed; otherwise record an error.", "Clear per-pin raw status across all pins by writing the raw-status clear bit in each per-pin control register in a loop, then wait briefly.", "Verify that the group masked status is cleared by a subsequent READ; on non-zero value, record an error.", "Clear the system-register raw interrupt status for the selected GPIO instance and verify by READ that the specific bit is cleared.", "Re-enable the group interrupt for all pins and clear the interrupt controller pending state for the selected line." ], "Impacted Registers": "INTR_EN1, GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_GPIO_IO_CTRL_GROUP1, GPIO_GPIO_IO_CTRL_GROUP2, GPIO_GPIO_IO_CTRL_GROUP3, GPIO_GPIO_IO_CTRL_GROUP4, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, RAW_STCR1", "Validation / Acceptance Criteria": [ "For each pin, an interrupt is serviced before the configured timeout expires.", "During service, the group masked status indicates that an interrupt occurred.", "After clearing all per-pin raw statuses, the group masked status reads as zero.", "The system-register raw interrupt status bit for the selected GPIO instance is cleared and verified.", "Final pass condition: the accumulated error counter remains zero at test completion." ], "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en", "Hidden_Test_Description": "program.c: test_case(): ifdef GPIO0 GIC_EnableIRQ(87); ifdef GPIO1 GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR/…GPIO1_INTR). for(i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000) // peie=1. wait_on(10). write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,0xFF); ... GROUP2..4=0xFF // doe=1 input. wait_on(10). write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc,0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xFFFFFFFF); timeout=2000; while(int_pend && --timeout>0){wait_on(10);} if(timeout==0){printf(\"ERROR: Timeout...\"); test_err++; break;} write_reg(0xA0243ffc,0x00000000); wait_on(10). finish(test_err). Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1,0); if((rdata_grp & 0xffffffff)!=0) { /*success msg*/ } else { printf(\"ERROR: Group Interrupt not occured\"); test_err++; } for(j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + j*4, 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0){ printf(\"ERROR: Group Interrupt clear failed:%x\", rdata_grp); test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0) { printf(\"sysreg status not cleared\"); test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0) { printf(\"sysreg status not cleared\"); test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).", "Hidden_Remarks": "Uses volatile extern int_pend for ISR synchronization. Masks group enable during ISR (write zero), clears per-pin raw in a 32-iteration loop, then re-enables group with all ones. External pad driver address 0xA0243ffc is used to drive edges.", "Hidden_Test_Steps_Procedure": "Entry: test_case(); GIC_EnableIRQ(87/88). write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). for(i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); wait_on(10); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); wait_on(10); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); For i=0..31: write_reg(0xA0243ffc,0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xFFFFFFFF); int timeout=2000; while((int_pend==1) && (--timeout>0)) wait_on(10); if(timeout==0){ printf(\"ERROR: Timeout waiting for GPIO IRQ at i=%u\", i); test_err++; break; } write_reg(0xA0243ffc,0x00000000); wait_on(10); finish(test_err). ISR Default_IRQHandler(): rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if((rdata_grp & 0xffffffff)!=0) {/*ok*/} else { printf(\"ERROR: Group Interrupt not occured\"); test_err++; } for(j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0){ printf(\"ERROR : Group Interrupt clear failed\"); test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0) { printf(\"sysreg status not cleared\"); test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0) { printf(\"sysreg status not cleared\"); test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "Timeout path: if(timeout==0){ printf(\"ERROR: Timeout waiting for GPIO IRQ at i=%u\", i); test_err++; break; }. ISR: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0); if((rdata_grp & 0xffffffff)==0) { printf(\"ERROR: Group Interrupt not occured\"); test_err++; } for(j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0) test_err++; Clear sysreg raw via MIZAR_LSS_SYSREG_RAW_STCR1 and verify by read; if bit remains set, increment test_err. Re-enable MIZAR_GPIO_GP0_INTR1_INTR_EN1 to 0xFFFFFFFF. finish(test_err)." } ] }'''

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

def to_ist_now():
    if ZoneInfo:
        tz = ZoneInfo("Asia/Kolkata")
        return datetime.now(tz)
    # fallback: IST = UTC+5:30 approx
    return datetime.utcnow()


def load_json():
    try:
        obj = json.loads(JSON_DATA)
        if not isinstance(obj, dict) or "test_cases" not in obj or not isinstance(obj["test_cases"], list):
            raise ValueError("JSON must contain 'test_cases' array")
        return obj
    except Exception as e:
        raise SystemExit(f"Invalid JSON input: {e}")


def json_scalar(v):
    if isinstance(v, (str, int, float)) or v is None:
        return v
    # Preserve exactly as JSON string for arrays/objects/bools
    return json.dumps(v, ensure_ascii=False)


def union_columns(rows):
    seen = []
    seen_set = set()
    for r in rows:
        if not isinstance(r, dict):
            continue
        for k in r.keys():
            if k not in seen_set:
                seen.append(k)
                seen_set.add(k)
    return seen


def autofit_columns(ws):
    # Approximate width based on max string length
    for col_cells in ws.columns:
        max_len = 0
        col = col_cells[0].column_letter
        for c in col_cells:
            val = c.value
            if val is None:
                continue
            s = str(val)
            if len(s) > max_len:
                max_len = len(s)
        ws.column_dimensions[col].width = min(max(12, max_len + 2), 80)


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def build_workbook(data_rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Determine columns from union preserving order
    cols = union_columns(data_rows)

    # Header
    for j, key in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=j, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")

    # Rows
    for i, row in enumerate(data_rows, start=2):
        for j, key in enumerate(cols, start=1):
            val = json_scalar(row.get(key, ""))
            ws.cell(row=i, column=j, value=val)

    ws.freeze_panes = "A2"
    ws.auto_filter.ref = ws.dimensions
    autofit_columns(ws)

    # Create Meta_data_sheet
    meta = wb.create_sheet("Meta_data_sheet")
    # Headers in given order
    for j, key in enumerate(META_COLS, start=1):
        cell = meta.cell(row=1, column=j, value=key)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
    # Data rows
    for i, row in enumerate(data_rows, start=2):
        for j, key in enumerate(META_COLS, start=1):
            val = json_scalar(row.get(key, ""))
            meta.cell(row=i, column=j, value=val)
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Build TestPlan sheet from Data, keeping only MAIN_ORDER and reordering
    testplan = ws
    testplan.title = "TestPlan"

    # Map Data headers to column index
    header_index = { testplan.cell(row=1, column=c).value: c for c in range(1, testplan.max_column+1) }

    # Create a temporary 2D array for TestPlan with selected columns only
    out_cols = MAIN_ORDER
    out_grid = []
    out_grid.append(out_cols)
    for r in range(2, testplan.max_row+1):
        out_row = []
        for col_name in out_cols:
            src_col = header_index.get(col_name, None)
            val = ""
            if src_col is not None:
                val = testplan.cell(row=r, column=src_col).value
            out_row.append(val)
        out_grid.append(out_row)

    # Clear existing cells and rewrite
    for row in testplan[1:testplan.max_row]:
        for cell in row:
            cell.value = None
    # Write headers and data
    for i, row in enumerate(out_grid, start=1):
        for j, val in enumerate(row, start=1):
            testplan.cell(row=i, column=j, value=val)

    # Remove any extra columns beyond our defined ones
    while testplan.max_column > len(out_cols):
        testplan.delete_cols(idx=len(out_cols)+1)

    # Formatting rules
    header_fill = PatternFill(fill_type="solid", fgColor="4472C4")
    header_font = Font(bold=True, color="FFFFFF")
    header_alignment = Alignment(horizontal="center", vertical="center", wrap_text=False)

    for j in range(1, len(out_cols)+1):
        cell = testplan.cell(row=1, column=j)
        cell.font = header_font
        cell.alignment = header_alignment
        cell.fill = header_fill

    # Data alignment and wrap for specified columns
    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }

    # Determine column indices for wrap
    col_name_to_idx = { testplan.cell(row=1, column=j).value: j for j in range(1, len(out_cols)+1) }

    for r in range(2, testplan.max_row+1):
        for name, j in col_name_to_idx.items():
            cell = testplan.cell(row=r, column=j)
            if name == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=False)
            elif name in wrap_cols:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=False)

    # Data validation for Code Generation (Required / Not)
    cg_col = col_name_to_idx.get("Code Generation (Required / Not)")
    if cg_col:
        col_letter = testplan.cell(row=1, column=cg_col).column_letter
        dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
        dv.error = "Select from the list: Required, Not Required, or leave blank"
        testplan.add_data_validation(dv)
        dv.add(f"{col_letter}2:{col_letter}{testplan.max_row}")

    # Freeze top row and autofilter
    testplan.freeze_panes = "A2"
    testplan.auto_filter.ref = testplan.dimensions

    # Autofit columns and add borders
    autofit_columns(testplan)
    apply_borders(testplan)

    return wb


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo-root', default='.')
    ap.add_argument('--output-dir', required=True)
    ap.add_argument('--ip', required=True)
    ap.add_argument('--commit-message', default='Final formatted Excel generated from JSON input')
    args = ap.parse_args()

    obj = load_json()
    data_rows = obj.get('test_cases', [])

    wb = build_workbook(data_rows)

    ist_now = to_ist_now()
    fname = f"{args.ip}_TestPlan_{ist_now.strftime('%Y%m%d')}_{ist_now.strftime('%H%M%S')}.xlsx"
    out_dir = os.path.join(args.repo_root, args.output_dir)
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)

    # Commit if running in Actions
    try:
        os.system('git config user.email "github-actions[bot]@users.noreply.github.com"')
        os.system('git config user.name "github-actions[bot]"')
        os.system(f'git add "{out_path}"')
        os.system(f'git add "{args.output_dir}"')
        status = os.popen('git diff --cached --name-only').read().strip()
        if status:
            os.system(f'git commit -m "{args.commit_message}"')
            os.system('git push')
        else:
            print('No changes to commit')
    except Exception as e:
        print(f"Commit step failed: {e}")

if __name__ == '__main__':
    main()
