#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import re
from datetime import datetime, timedelta, timezone
from zipfile import ZipFile, is_zipfile

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded RAW JSON provided by user (do not modify)
RAW_JSON = r"""
{ "TC1": { "Index": "1", "SS / Module": "GPIO", "Feature": "Independent control register for each GPIO", "Test Case Name": "gpio_reg_wr_rd_test", "Test Description": "Validates default reset values and masked write/read behavior across GPIO per-pin and group registers using predefined masks and test patterns; reports pass only if all default and write-read checks succeed.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Certain registers are intentionally skipped for default or write-read checks based on skip arrays. Addresses with non-readable or non-writable masks are skipped accordingly. Input status bit behavior can read as high without external drive; comparisons ignore the least significant bit during default checks. Comments note specific groups being excluded to avoid VRRW and DIN-related mismatches.", "Test Steps / Procedure": "Entry points: a test routine followed by two internal phases: default checking and masked write-read verification. Default check: iterate through configured register addresses; skip entries flagged for reset-skip; if an address is not readable, skip; otherwise read the register and compare the value with the documented default while ignoring the least significant bit; count a failure on mismatch. Masked write-read check: for each of six predefined patterns, iterate through the address list; skip entries flagged for write-skip; if an address is not writable, skip; otherwise write the pattern masked by the write mask. Then iterate again to read back: skip entries flagged for write-skip; if an address is not writable or not readable, skip; otherwise read the register, apply the read mask, and compute the expected value by combining the writable bits from the pattern and the non-writable readable bits from the documented default; count a failure if the masked read value does not equal the expected value. Finalize: declare failure if any default or write-read mismatches were counted; otherwise declare success.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if: 1) For each non-skipped, readable address, the reset-time read value (with the least significant bit ignored) equals the documented default value; and 2) For each test pattern and each non-skipped address that is both writable and readable, the masked read value equals the combination of pattern bits at writable positions and default bits at non-writable readable positions. Any mismatch increments error counters and results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test", "Hidden_Test_Description": "Test verifies all GPIO register default values and write-read behavior using masks. Sequence: chk_rst_val() checks defaults for each addr_array[i] where skip_rst_array[i]==0 and read_mask_array[i]!=0, reading data_rd=read_reg(addr) then data=(data_rd & 0xfffffffe) and comparing to default_value_array[i]; logs and increments def_fail_cnt on mismatch. chk_rd_wr() uses chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}. For each pattern: write_reg(addr,(data_wr & write_mask_array[i])) for entries with skip_array[i]==0 and write_mask_array[i]!=0. Then read back for entries with skip_array[i]==0, write_mask_array[i]!=0, read_mask_array[i]!=0: data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare data_rd vs exp_val; increment wr_fail_cnt on mismatch. At end, finish(1) if def_fail_cnt>0||wr_fail_cnt>0 else finish(0).", "Hidden_Remarks": "skip_array and skip_rst_array explicitly exclude certain registers (e.g., group IO control and DOUT/DIN groups) and non-readable/non-writable cases. A comment explains DIN may read as 1 without forcing, and forcing zero affects bit-level selection, so default comparisons ignore bit0 and many DIN/group registers are skipped for default checks.", "Hidden_Test_Steps_Procedure": "Entry: int test_case(). 1) Call chk_rst_val(): for (i=0..CNT-1){ addr=addr_array[i]; if(skip_rst_array[i]==1) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr) [READ, macro: addr_array[i]]; data=(data_rd & 0xfffffffe); if(data==default_value_array[i]) PASS else {def_fail_cnt++; log failure}}. 2) Call chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}. For each j pattern: a) data_wr=chk_val[j]; b) Write phase: for (i=0..CNT-1){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; write_reg(addr, (data_wr & write_mask_array[i])) [WRITE, macro: addr_array[i], mask: write_mask_array[i]] }. c) Read/verify phase: for (i=0..CNT-1){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=(read_reg(addr) & read_mask_array[i]) [READ, macro: addr_array[i], mask: read_mask_array[i]]; wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd==exp_val) PASS else {wr_fail_cnt++; log failure}}. 3) Result: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). Timing: No delays used. Loops: i over 49 entries; j over 6 patterns. Branches: skip cases based on skip arrays and masks.", "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4", "Hidden_Validation_Acceptance_Criteria": "Defaults: For all i where skip_rst_array[i]==0 and read_mask_array[i]!=0, (read_reg(addr_array[i]) & 0xfffffffe) == default_value_array[i] => pass; else def_fail_cnt++. Writes: For each pattern and for all i where skip_array[i]==0, write_mask_array[i]!=0: write (data_wr & write_mask_array[i]) to addr_array[i]. Readback: For all i where skip_array[i]==0 and write_mask_array[i]!=0 and read_mask_array[i]!=0: data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); require data_rd==exp_val; else wr_fail_cnt++. Overall: finish(0) only if def_fail_cnt==0 and wr_fail_cnt==0; else finish(1)." }, "TC2": { "Index": "2", "SS / Module": "GPIO", "Feature": "Interrupts can be generated based on negative edge detection at GPIO input", "Test Case Name": "test_gpio_negedge_intr_en", "Test Description": "Configures GPIO inputs for negative-edge detection, triggers a falling edge per pin via a stimulus register, waits with a bounded timeout for the interrupt, and in the interrupt context verifies pin-level and group interrupt status, clears sources, and confirms system-level status is cleared.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "Interrupt line enable is instance-specific and controlled by compile-time selection. A stimulus register is used to create signal transitions; setup returns the driver to a known high state before and after handling. The wait is armed before edge generation to avoid race conditions. The wait loop is bounded with a timeout (5000 iterations) and includes short delays. Comments indicate the timeout may need adjustment based on simulation timing.", "Test Steps / Procedure": "Entry points: a test routine and an interrupt service routine. Test routine: enable the appropriate interrupt line for the selected instance and enable the corresponding system-level interrupt output. Initialize the stimulus output to drive the monitored inputs high. Configuration phase: for each of the 32 monitored inputs, program the per-pin control to enable input mode, negative-edge detection, and clear any latched raw status; insert a short wait after each configuration. Exercise phase: for each input index, pre-clear the corresponding group raw status bit, enable only that input’s interrupt at the group level, wait briefly, and arm the interrupt wait flag. Generate a single falling edge for that input by first driving all inputs high, waiting briefly, then pulling only the target input low via the stimulus register. Enter a bounded wait loop with a finite retry counter and short delays; if the wait flag does not clear before the counter expires, record a timeout error for that input. Finalize: report the aggregated error count. Interrupt service routine: clear the wait flag immediately, return the stimulus to a known high state, and read the per-pin control/status for the active input. Verify that the input status reflects a low level after the falling edge. If the raw indication is asserted at the pin, read the group interrupt status and confirm that the bit for the active input is set. Clear the per-pin raw status while keeping input mode enabled, clear the group raw status bit, and verify that the group status register reads as zero. Clear the system-level raw status for the selected instance and clear the platform interrupt controller state. If the raw indication is not asserted at the pin or if any verification fails, record an error.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if, for each exercised input: the interrupt is observed before the timeout; the input status indicates a low level after the falling edge; the group status shows the active bit set during service; after clearing, the group status reads zero; and the system-level status is cleared following the service. Any timeout or status mismatch results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en", "Hidden_Test_Description": "Negative-edge interrupt enable and validation for GPIO pins 8..39. Setup: optionally GIC_EnableIRQ(87/88) per instance; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drive all high: write_reg(0xA0243ffc, 0xffffffff). Configure per-pin: for i in 0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)) // doe=1, neie=1, iclr=1; wait_on(10). Loop per pin: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) [WRITE,W1C]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) [WRITE]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--){ wait_on(10);} if(timeout==0){ printf timeout; test_err++; }. finish(test_err).", "Hidden_Remarks": "Arming int_pend before generating the edge avoids a race. Timeout is 5000 iterations with 10-cycle waits; comment suggests adjusting based on simulation time base. The stimulus address 0xA0243ffc is used to toggle inputs; initial and post-ISR state is driven high.", "Hidden_Test_Steps_Procedure": "Entry: int test_case(). 1) Instance interrupt enable: Ifdef GPIO0: GIC_EnableIRQ(87); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO0_INTR]. Ifdef GPIO1: GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO1_INTR]. 2) Drive known state high: write_reg(0xA0243ffc, 0xffffffff) [WRITE, literal address]. 3) Configure per-pin for negedge + input + raw clear: for(i=0..31){ addr1=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)) [WRITE, bits: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16)]; wait_on(10)}. 4) For each pin i: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) [WRITE, W1C mask]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) [WRITE, enable bit i]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff) [WRITE]; wait_on(30); write_reg(0xA0243ffc, ~wr_val) [WRITE, make falling edge on bit i]; timeout=5000; while(int_pend && timeout--){ wait_on(10)}; if(timeout==0){ test_err++}. 5) finish(test_err). ISR: void Default_IRQHandler(). local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff) [WRITE]; raddr=MIZAR_GPIO_GP0_GPIO_8+(i4); rdata=read_reg(raddr) [READ]; if((rdata & 0x1)!=0){ test_err++} // DIN should be 0. if((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if((rdata_grp & local_wr)==0){ test_err++}; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(raddr2, (1u<<20)|(1u<<16)) [WRITE, clear iclr and keep doe]; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr) [WRITE, W1C]; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if(rdata_grp!=0x0){ test_err++}; Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR) [WRITE,W1C]; GIC_ClearIRQ(87); Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR) [WRITE,W1C]; GIC_ClearIRQ(88);} else { test_err++}. Timing: wait_on(10/30) delays; polling wait with timeout=5000. Loops: two for-loops over 32 pins; ISR executes per interrupt. Branches: per instance selection; ISR conditionals on DIN and raw bits.", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "Per pin i: main loop requires that int_pend becomes 0 before timeout after generating negedge; else test_err++. ISR requires: (rdata & 0x1)==0 (DIN low), (rdata & 0x2)!=0 (raw set); group status read (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must have bit i set; after write 1 to per-pin iclr and W1C to raw group stclr, group status must read 0; system-level raw clear write to MIZAR_LSS_SYSREG_RAW_STCR1 must clear status; finally platform IRQ is cleared. Overall pass if test_err==0." }, "TC3": { "Index": "3", "SS / Module": "GPIO", "Feature": "Interrupts can be generated based on positive edge detection at GPIO input", "Test Case Name": "test_gpio_pedge_all_pads_en", "Test Description": "Enables positive-edge detection on all monitored inputs, configures input mode, triggers a rising edge per pin using a stimulus register with a bounded wait, and in the interrupt context verifies group status, clears per-pin raw status across all pins, confirms group clear, and clears system-level status before re-enabling.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "An external stimulus register is used to drive signal transitions. The wait flag is armed before generating the edge to avoid race conditions. The polling wait uses a finite timeout (2000 iterations) with short delays. During service, group interrupts are masked, per-pin raw status is cleared for all pins, and system-level status is verified as cleared before re-enabling for the next iteration.", "Test Steps / Procedure": "Entry points: a test routine and an interrupt service routine. Test routine: enable the appropriate interrupt line for the selected instance and enable the corresponding system-level interrupt output. Configure per-pin control for all monitored inputs to enable positive-edge detection. Set input mode for the groups covering the monitored inputs. Enable all bits in the group interrupt enable register. For each input index, prepare a known low level via the stimulus register, wait briefly, arm the interrupt wait flag, and then drive a rising edge by setting all bits high. Enter a bounded wait loop with a finite retry counter and short delays; if the wait flag does not clear before the counter expires, record a timeout and stop further iterations. Optionally restore the stimulus to low for the next iteration. Finalize by reporting the aggregated error count. Interrupt service routine: compute the current bit position from the active index and clear the wait flag. Read the group interrupt status; temporarily mask the group interrupt enable while servicing. If any bit is set, treat group assertion as successful; otherwise, record an error. Clear per-pin raw status for all monitored inputs by writing the appropriate clear bit in each per-pin control register. Re-read the group status and require zero; on non-zero, record an error. Clear the system-level raw status for the selected instance and verify the status is cleared by reading back; record an error if not cleared. Re-enable the group interrupt for all bits and clear the platform interrupt controller state.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if the interrupt is observed for each exercised input before the timeout, the group status indicates an assertion during service, the group status reads as zero after per-pin raw clear, and the system-level status is verified cleared before re-enabling. Any timeout or status mismatch results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en", "Hidden_Test_Description": "Positive-edge interrupt test for GPIO pins 8..39. Setup: optionally GIC_EnableIRQ(87/88) per instance; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Enable posedge per-pin: for (i=0..31) write_reg(MIZAR_GPIO_GP0_GPIO_8+(i4), 0x00020000) [WRITE, bit17]. Set input mode groups: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); ... GROUP2..GROUP4 similarly; wait_on(10). Enable group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Exercise per-pin: for (i=0..31){ write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while((int_pend==1) && (--timeout>0)){ wait_on(10)}; if(timeout==0){ printf timeout; test_err++; break;} write_reg(0xA0243ffc, 0x00000000); wait_on(10);} finish(test_err).", "Hidden_Remarks": "int_pend is declared volatile to ensure ISR-visible updates. Timeouts of 2000 iterations with 10-cycle waits are used to avoid infinite waiting. The ISR masks the group enable register during service, clears all per-pin raw status bits, confirms the group is fully cleared, and then re-enables the group. The stimulus at 0xA0243ffc is used to toggle input levels.", "Hidden_Test_Steps_Procedure": "Entry: void test_case(). 1) Instance interrupt enable: Ifdef GPIO0: GIC_EnableIRQ(87); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO0_INTR]. Ifdef GPIO1: GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) [WRITE]. 2) Per-pin posedge config: for(i=0..31){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(i4), 0x00020000) [WRITE, bit17=1] }. 3) Set groups input mode: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); wait_on(10). 4) Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF) [WRITE]. 5) For each pin: write_reg(0xA0243ffc, 0x00000000) [WRITE, ensure low]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) [WRITE, rising edge]; timeout=2000; while((int_pend==1) && (--timeout>0)){ wait_on(10)}; if(timeout==0){ test_err++; break;} write_reg(0xA0243ffc, 0x00000000) [WRITE]; wait_on(10). 6) finish(test_err). ISR: void Default_IRQHandler(). wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) [WRITE, mask group]; if((rdata_grp & 0xffffffff)!=0){ success } else { printf error; test_err++}. Clear raws: for(j=0..31){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000) [WRITE, iclr bit16] }; wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if(rdata_grp==0x0){ success } else { printf error; test_err++ }. Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR) [WRITE,W1C]; rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1) [READ]; if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0){ printf error; test_err++ }. Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0){ printf error; test_err++ }. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF) [WRITE, re-enable]; GIC_ClearIRQ(87/88). Timing: wait_on(10/2) delays; polling wait with timeout=2000. Loops: per-pin loop (32) and ISR per event.", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "Main loop: int_pend must clear before timeout for each pin; else test_err++. ISR: group status must be non-zero on entry; after writing per-pin iclr across all pins, group status must be zero. System-level raw status must be cleared after W1C write and verified by readback. Re-enable group interrupts and clear platform IRQ at end. Overall pass if test_err==0." } }
"""

IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")

# Column definitions
META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

MAIN_COLUMNS = [
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

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

ALLOWED_VALIDATION_VALUES = ["Required", "Blank", "Not Required"]


def parse_and_normalize(raw_json: str):
    try:
        data_obj = json.loads(raw_json)
    except Exception as e:
        raise SystemExit(f"JSON parse error: {e}")

    if isinstance(data_obj, list):
        records = data_obj
    elif isinstance(data_obj, dict):
        # Deterministically convert dict of testcases to array by key-sorted order
        records = [data_obj[k] for k in sorted(data_obj.keys())]
    else:
        raise SystemExit("Invalid JSON type: expected array or object")

    if not records:
        raise SystemExit("JSON is empty")

    # Build union of keys in first-seen order, preserving internal object key order
    union_keys = []
    seen = set()
    for rec in records:
        if not isinstance(rec, dict):
            raise SystemExit("All entries must be JSON objects")
        for k in rec.keys():
            if k not in seen:
                seen.add(k)
                union_keys.append(k)

    return records, union_keys


def to_numbered_list(text: str) -> str:
    if text is None:
        return ""
    s = str(text).strip()
    if not s:
        return s

    # Normalize line breaks
    s = s.replace("\r\n", "\n").replace("\r", "\n")

    items = []
    if ";" in s:
        parts = [p.strip() for p in s.split(";")]
        items = [p for p in parts if p]
    else:
        # Split on sentence boundaries: period/exclamation/question mark followed by whitespace
        # Keep punctuation with the sentence
        parts = re.split(r"(?<=[.!?])\s+", s)
        parts = [p.strip() for p in parts]
        items = [p for p in parts if p]

    if not items:
        return s

    lines = [f"{i}. {itm}" for i, itm in enumerate(items, start=1)]
    return "\n".join(lines)


def set_header_style(cell):
    cell.font = Font(bold=True)
    cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell.fill = PatternFill(fill_type="solid", start_color="4F81BD", end_color="4F81BD")


def set_data_alignment(cell, col_name: str):
    if col_name == "Index":
        cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=True)
    else:
        cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=(col_name in WRAP_COLUMNS))


def apply_borders(ws, min_row, max_row, min_col, max_col):
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(min_row, max_row + 1):
        for c in range(min_col, max_col + 1):
            ws.cell(row=r, column=c).border = border


def autofit_columns(ws):
    # Estimate width based on max length of any line in the column
    for col_cells in ws.columns:
        max_len = 0
        col_letter = col_cells[0].column_letter
        for cell in col_cells:
            val = cell.value
            if val is None:
                continue
            s = str(val)
            for line in s.split("\n"):
                if len(line) > max_len:
                    max_len = len(line)
        # Add padding
        width = min(max_len + 2, 120)
        ws.column_dimensions[col_letter].width = width if width > 10 else 10


def autofit_row_heights(ws, wrap_col_indices, start_row: int, end_row: int):
    # Approximate row height by number of lines in wrapped columns
    base_height = 15
    for r in range(start_row, end_row + 1):
        max_lines = 1
        for c in wrap_col_indices:
            val = ws.cell(row=r, column=c).value
            if val is None:
                continue
            s = str(val)
            lines = s.count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines


def validate_xlsx(path: str) -> bool:
    if not os.path.isfile(path):
        return False
    if not is_zipfile(path):
        return False
    try:
        with ZipFile(path, 'r') as zf:
            required = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
            names = set(zf.namelist())
            return required.issubset(names)
    except Exception:
        return False


def main():
    records, union_keys = parse_and_normalize(RAW_JSON)

    # Create workbook and Data sheet
    wb = Workbook()
    ws_data = wb.active
    ws_data.title = "Data"

    # Write headers preserving union_keys order
    for idx, key in enumerate(union_keys, start=1):
        cell = ws_data.cell(row=1, column=idx, value=key)
        set_header_style(cell)

    # Write data rows
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, key in enumerate(union_keys, start=1):
            val = rec.get(key, "")
            ws_data.cell(row=r_idx, column=c_idx, value=val)

    # Freeze top row and initial autofit
    ws_data.freeze_panes = "A2"
    autofit_columns(ws_data)

    # Create Meta_data_sheet and copy META columns AS-IS in defined order
    ws_meta = wb.create_sheet(title="Meta_data_sheet")
    for idx, key in enumerate(META_COLUMNS, start=1):
        cell = ws_meta.cell(row=1, column=idx, value=key)
        set_header_style(cell)
    for r_idx, rec in enumerate(records, start=2):
        for c_idx, key in enumerate(META_COLUMNS, start=1):
            ws_meta.cell(row=r_idx, column=c_idx, value=rec.get(key, ""))

    # Very hide meta sheet
    ws_meta.sheet_state = "veryHidden"

    # Prepare TestPlan sheet by renaming Data -> TestPlan
    ws_data.title = "TestPlan"
    ws = ws_data  # main working sheet now

    # Remove META columns and ensure ONLY MAIN columns remain in final order
    # Build a mapping from current headers to their column indices
    current_headers = [ws.cell(row=1, column=i + 1).value for i in range(ws.max_column)]

    # Create a new ordered set of columns to keep (MAIN_COLUMNS)
    # Build an index map from header to source column
    header_to_src_col = {h: (current_headers.index(h) + 1) for h in current_headers if h in current_headers}

    # Build a temporary matrix for TestPlan with only MAIN columns in final order
    temp_rows = []
    temp_rows.append(MAIN_COLUMNS[:])  # header row
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for key in MAIN_COLUMNS:
            if key in current_headers:
                src_col = current_headers.index(key) + 1
                row_vals.append(ws.cell(row=r, column=src_col).value)
            else:
                row_vals.append("")
        temp_rows.append(row_vals)

    # Clear existing sheet and write back only MAIN columns
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)
    for r_idx, row in enumerate(temp_rows, start=1):
        for c_idx, val in enumerate(row, start=1):
            ws.cell(row=r_idx, column=c_idx, value=val)

    # Apply numbering inside cells for specified columns (in TestPlan only)
    header_row = [ws.cell(row=1, column=i + 1).value for i in range(ws.max_column)]
    col_index_map = {h: (i + 1) for i, h in enumerate(header_row)}
    for key in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if key in col_index_map:
            c = col_index_map[key]
            for r in range(2, ws.max_row + 1):
                original = ws.cell(row=r, column=c).value
                ws.cell(row=r, column=c, value=to_numbered_list(original))

    # Strict formatting
    # Header styles already set previously, but re-apply to ensure consistency
    for c in range(1, ws.max_column + 1):
        set_header_style(ws.cell(row=1, column=c))

    # Data alignment, wrap text, borders
    wrap_col_indices = []
    for c in range(1, ws.max_column + 1):
        col_name = ws.cell(row=1, column=c).value
        if col_name in WRAP_COLUMNS:
            wrap_col_indices.append(c)
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            col_name = ws.cell(row=1, column=c).value
            set_data_alignment(ws.cell(row=r, column=c), col_name)

    # Borders for all populated cells
    apply_borders(ws, 1, ws.max_row, 1, ws.max_column)

    # Freeze top row
    ws.freeze_panes = "A2"

    # Autofit columns and row heights
    autofit_columns(ws)
    autofit_row_heights(ws, wrap_col_indices, 2, ws.max_row)

    # Data validation ONLY for Code Generation (Required / Not) column in data rows
    if "Code Generation (Required / Not)" in col_index_map and ws.max_row >= 2:
        c = col_index_map["Code Generation (Required / Not)"]
        col_letter = ws.cell(row=1, column=c).column_letter
        dv = DataValidation(type="list", formula1='"' + ", ".join(ALLOWED_VALIDATION_VALUES) + '"', allow_blank=True)
        dv.error = "Select a value from the list"
        dv.prompt = "Allowed: " + ", ".join(ALLOWED_VALIDATION_VALUES)
        ws.add_data_validation(dv)
        dv.add(f"{col_letter}2:{col_letter}{ws.max_row}")

    # Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist
    for sheet_name in list(wb.sheetnames):
        if sheet_name not in {"TestPlan", "Meta_data_sheet"}:
            ws_extra = wb[sheet_name]
            wb.remove(ws_extra)

    # Save with IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    ts = datetime.now(ist).strftime("%Y%m%d_%H%M%S")
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    out_name = f"{IP_NAME}_TestPlan_{ts}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, out_name)
    wb.save(out_path)

    # Validate saved XLSX structure
    if not validate_xlsx(out_path):
        raise SystemExit(f"XLSX validation failed for {out_path}")

    print(out_path)


if __name__ == "__main__":
    main()
