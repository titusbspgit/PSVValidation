#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import re
import sys
import zipfile
from datetime import datetime, timedelta, timezone
from io import BytesIO

from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation

# Configuration (can be overridden by environment variables in the workflow)
IP_NAME = os.getenv('IP_NAME', 'GPIO')
OUTPUT_DIR = os.getenv('OUTPUT_DIR', 'Test_Output/GPIO/TestPlan')
COMMIT_CHANGES = os.getenv('COMMIT_CHANGES', 'true').lower() == 'true'

# Embedded JSON payload (as provided to the agent)
JSON_PAYLOAD = r'''{ "TC1": { "Index": "1", "SS / Module": "GPIO", "Feature": "Independent control register for each GPIO", "Test Case Name": "gpio_reg_wr_rd_test", "Test Description": "Validates default reset values and masked write/read behavior across GPIO per-pin and group registers using predefined masks and test patterns; reports pass only if all default and write-read checks succeed.", "Speed": "NA", "Mode": "NA", "Memory Start Offset": "NA", "Memory End Offset": "NA", "Remarks": "Certain registers are intentionally skipped for default or write-read checks based on skip arrays. Addresses with non-readable or non-writable masks are skipped accordingly. Input status bit behavior can read as high without external drive; comparisons ignore the least significant bit during default checks. Comments note specific groups being excluded to avoid VRRW and DIN-related mismatches.", "Test Steps / Procedure": "Entry points: a test routine followed by two internal phases: default checking and masked write-read verification. Default check: iterate through configured register addresses; skip entries flagged for reset-skip; if an address is not readable, skip; otherwise read the register and compare the value with the documented default while ignoring the least significant bit; count a failure on mismatch. Masked write-read check: for each of six predefined patterns, iterate through the address list; skip entries flagged for write-skip; if an address is not writable, skip; otherwise write the pattern masked by the write mask. Then iterate again to read back: skip entries flagged for write-skip; if an address is not writable or not readable, skip; otherwise read the register, apply the read mask, and compute the expected value by combining the writable bits from the pattern and the non-writable readable bits from the documented default; count a failure if the masked read value does not equal the expected value. Finalize: declare failure if any default or write-read mismatches were counted; otherwise declare success.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if: 1) For each non-skipped, readable address, the reset-time read value (with the least significant bit ignored) equals the documented default value; and 2) For each test pattern and each non-skipped address that is both writable and readable, the masked read value equals the combination of pattern bits at writable positions and default bits at non-writable readable positions. Any mismatch increments error counters and results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test", "Hidden_Test_Description": "Test verifies all GPIO register default values and write-read behavior using masks. Sequence: chk_rst_val() checks defaults for each addr_array[i] where skip_rst_array[i]==0 and read_mask_array[i]!=0, reading data_rd=read_reg(addr) then data=(data_rd & 0xfffffffe) and comparing to default_value_array[i]; logs and increments def_fail_cnt on mismatch. chk_rd_wr() uses chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}. For each pattern: write_reg(addr,(data_wr & write_mask_array[i])) for entries with skip_array[i]==0 and write_mask_array[i]!=0. Then read back for entries with skip_array[i]==0, write_mask_array[i]!=0, read_mask_array[i]!=0: data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare data_rd vs exp_val; increment wr_fail_cnt on mismatch. At end, finish(1) if def_fail_cnt>0||wr_fail_cnt>0 else finish(0).", "Hidden_Remarks": "skip_array and skip_rst_array explicitly exclude certain registers (e.g., group IO control and DOUT/DIN groups) and non-readable/non-writable cases. A comment explains DIN may read as 1 without forcing, and forcing zero affects bit-level selection, so default comparisons ignore bit0 and many DIN/group registers are skipped for default checks.", "Hidden_Test_Steps_Procedure": "Entry: int test_case(). 1) Call chk_rst_val(): for (i=0..CNT-1){ addr=addr_array[i]; if(skip_rst_array[i]==1) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr) [READ, macro: addr_array[i]]; data=(data_rd & 0xfffffffe); if(data==default_value_array[i]) PASS else {def_fail_cnt++; log failure}}. 2) Call chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}. For each j pattern: a) data_wr=chk_val[j]; b) Write phase: for (i=0..CNT-1){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; write_reg(addr, (data_wr & write_mask_array[i])) [WRITE, macro: addr_array[i], mask: write_mask_array[i]] }. c) Read/verify phase: for (i=0..CNT-1){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=(read_reg(addr) & read_mask_array[i]) [READ, macro: addr_array[i], mask: read_mask_array[i]]; wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd==exp_val) PASS else {wr_fail_cnt++; log failure}}. 3) Result: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). Timing: No delays used. Loops: i over 49 entries; j over 6 patterns. Branches: skip cases based on skip arrays and masks.", "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4", "Hidden_Validation_Acceptance_Criteria": "Defaults: For all i where skip_rst_array[i]==0 and read_mask_array[i]!=0, (read_reg(addr_array[i]) & 0xfffffffe) == default_value_array[i] => pass; else def_fail_cnt++. Writes: For each pattern and for all i where skip_array[i]==0, write_mask_array[i]!=0: write (data_wr & write_mask_array[i]) to addr_array[i]. Readback: For all i where skip_array[i]==0 and write_mask_array[i]!=0 and read_mask_array[i]!=0: data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); require data_rd==exp_val; else wr_fail_cnt++. Overall: finish(0) only if def_fail_cnt==0 and wr_fail_cnt==0; else finish(1)." }, "TC2": { "Index": "2", "SS / Module": "GPIO", "Feature": "Interrupts can be generated based on negative edge detection at GPIO input", "Test Case Name": "test_gpio_negedge_intr_en", "Test Description": "Configures GPIO inputs for negative-edge detection, triggers a falling edge per pin via a stimulus register, waits with a bounded timeout for the interrupt, and in the interrupt context verifies pin-level and group interrupt status, clears sources, and confirms system-level status is cleared.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "Interrupt line enable is instance-specific and controlled by compile-time selection. A stimulus register is used to create signal transitions; setup returns the driver to a known high state before and after handling. The wait is armed before edge generation to avoid race conditions. The wait loop is bounded with a timeout (5000 iterations) and includes short delays. Comments indicate the timeout may need adjustment based on simulation timing.", "Test Steps / Procedure": "Entry points: a test routine and an interrupt service routine. Test routine: enable the appropriate interrupt line for the selected instance and enable the corresponding system-level interrupt output. Initialize the stimulus output to drive the monitored inputs high. Configuration phase: for each of the 32 monitored inputs, program the per-pin control to enable input mode, negative-edge detection, and clear any latched raw status; insert a short wait after each configuration. Exercise phase: for each input index, pre-clear the corresponding group raw status bit, enable only that input’s interrupt at the group level, wait briefly, and arm the interrupt wait flag. Generate a single falling edge for that input by first driving all inputs high, waiting briefly, then pulling only the target input low via the stimulus register. Enter a bounded wait loop with a finite retry counter and short delays; if the wait flag does not clear before the counter expires, record a timeout error for that input. Finalize: report the aggregated error count. Interrupt service routine: clear the wait flag immediately, return the stimulus to a known high state, and read the per-pin control/status for the active input. Verify that the input status reflects a low level after the falling edge. If the raw indication is asserted at the pin, read the group interrupt status and confirm that the bit for the active input is set. Clear the per-pin raw status while keeping input mode enabled, clear the group raw status bit, and verify that the group status register reads as zero. Clear the system-level raw status for the selected instance and clear the platform interrupt controller state. If the raw indication is not asserted at the pin or if any verification fails, record an error.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if, for each exercised input: the interrupt is observed before the timeout; the input status indicates a low level after the falling edge; the group status shows the active bit set during service; after clearing, the group status reads zero; and the system-level status is cleared following the service. Any timeout or status mismatch results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en", "Hidden_Test_Description": "Negative-edge interrupt enable and validation for GPIO pins 8..39. Setup: optionally GIC_EnableIRQ(87/88) per instance; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drive all high: write_reg(0xA0243ffc, 0xffffffff). Configure per-pin: for i in 0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)) // doe=1, neie=1, iclr=1; wait_on(10). Loop per pin: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) [WRITE,W1C]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) [WRITE]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--){ wait_on(10);} if(timeout==0){ printf timeout; test_err++; }. finish(test_err).", "Hidden_Remarks": "Arming int_pend before generating the edge avoids a race. Timeout is 5000 iterations with 10-cycle waits; comment suggests adjusting based on simulation time base. The stimulus address 0xA0243ffc is used to toggle inputs; initial and post-ISR state is driven high.", "Hidden_Test_Steps_Procedure": "Entry: int test_case(). 1) Instance interrupt enable: Ifdef GPIO0: GIC_EnableIRQ(87); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO0_INTR]. Ifdef GPIO1: GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO1_INTR]. 2) Drive known state high: write_reg(0xA0243ffc, 0xffffffff) [WRITE, literal address]. 3) Configure per-pin for negedge + input + raw clear: for(i=0..31){ addr1=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)) [WRITE, bits: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16)]; wait_on(10)}. 4) For each pin i: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) [WRITE, W1C mask]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) [WRITE, enable bit i]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff) [WRITE]; wait_on(30); write_reg(0xA0243ffc, ~wr_val) [WRITE, make falling edge on bit i]; timeout=5000; while(int_pend && timeout--){ wait_on(10)}; if(timeout==0){ test_err++}. 5) finish(test_err). ISR: void Default_IRQHandler(). local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff) [WRITE]; raddr=MIZAR_GPIO_GP0_GPIO_8+(i4); rdata=read_reg(raddr) [READ]; if((rdata & 0x1)!=0){ test_err++} // DIN should be 0. if((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if((rdata_grp & local_wr)==0){ test_err++}; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i4); write_reg(raddr2, (1u<<20)|(1u<<16)) [WRITE, clear iclr and keep doe]; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr) [WRITE, W1C]; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if(rdata_grp!=0x0){ test_err++}; Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR) [WRITE,W1C]; GIC_ClearIRQ(87); Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR) [WRITE,W1C]; GIC_ClearIRQ(88);} else { test_err++}. Timing: wait_on(10/30) delays; polling wait with timeout=5000. Loops: two for-loops over 32 pins; ISR executes per interrupt. Branches: per instance selection; ISR conditionals on DIN and raw bits.", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "Per pin i: main loop requires that int_pend becomes 0 before timeout after generating negedge; else test_err++. ISR requires: (rdata & 0x1)==0 (DIN low), (rdata & 0x2)!=0 (raw set); group status read (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must have bit i set; after write 1 to per-pin iclr and W1C to raw group stclr, group status must read 0; system-level raw clear write to MIZAR_LSS_SYSREG_RAW_STCR1 must clear status; finally platform IRQ is cleared. Overall pass if test_err==0." }, "TC3": { "Index": "3", "SS / Module": "GPIO", "Feature": "Interrupts can be generated based on positive edge detection at GPIO input", "Test Case Name": "test_gpio_pedge_all_pads_en", "Test Description": "Enables positive-edge detection on all monitored inputs, configures input mode, triggers a rising edge per pin using a stimulus register with a bounded wait, and in the interrupt context verifies group status, clears per-pin raw status across all pins, confirms group clear, and clears system-level status before re-enabling.", "Speed": "NA", "Mode": "Interrupt", "Memory Start Offset": "0xA0243ffc", "Memory End Offset": "0xA0243ffc", "Remarks": "An external stimulus register is used to drive signal transitions. The wait flag is armed before generating the edge to avoid race conditions. The polling wait uses a finite timeout (2000 iterations) with short delays. During service, group interrupts are masked, per-pin raw status is cleared for all pins, and system-level status is verified as cleared before re-enabling for the next iteration.", "Test Steps / Procedure": "Entry points: a test routine and an interrupt service routine. Test routine: enable the appropriate interrupt line for the selected instance and enable the corresponding system-level interrupt output. Configure per-pin control for all monitored inputs to enable positive-edge detection. Set input mode for the groups covering the monitored inputs. Enable all bits in the group interrupt enable register. For each input index, prepare a known low level via the stimulus register, wait briefly, arm the interrupt wait flag, and then drive a rising edge by setting all bits high. Enter a bounded wait loop with a finite retry counter and short delays; if the wait flag does not clear before the counter expires, record a timeout and stop further iterations. Optionally restore the stimulus to low for the next iteration. Finalize by reporting the aggregated error count. Interrupt service routine: compute the current bit position from the active index and clear the wait flag. Read the group interrupt status; temporarily mask the group interrupt enable while servicing. If any bit is set, treat group assertion as successful; otherwise, record an error. Clear per-pin raw status for all monitored inputs by writing the appropriate clear bit in each per-pin control register. Re-read the group status and require zero; on non-zero, record an error. Clear the system-level raw status for the selected instance and verify the status is cleared by reading back; record an error if not cleared. Re-enable the group interrupt for all bits and clear the platform interrupt controller state.", "Impacted Registers": "NA", "Validation / Acceptance Criteria": "Pass only if the interrupt is observed for each exercised input before the timeout, the group status indicates an assertion during service, the group status reads as zero after per-pin raw clear, and the system-level status is verified cleared before re-enabling. Any timeout or status mismatch results in failure.", "Code Generation (Required / Not)": "", "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en", "Hidden_Test_Description": "Positive-edge interrupt test for GPIO pins 8..39. Setup: optionally GIC_EnableIRQ(87/88) per instance; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Enable posedge per-pin: for (i=0..31) write_reg(MIZAR_GPIO_GP0_GPIO_8+(i4), 0x00020000) [WRITE, bit17]. Set input mode groups: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); ... GROUP2..GROUP4 similarly; wait_on(10). Enable group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Exercise per-pin: for (i=0..31){ write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while((int_pend==1) && (--timeout>0)){ wait_on(10)}; if(timeout==0){ printf timeout; test_err++; break;} write_reg(0xA0243ffc, 0x00000000); wait_on(10);} finish(test_err).", "Hidden_Remarks": "int_pend is declared volatile to ensure ISR-visible updates. Timeouts of 2000 iterations with 10-cycle waits are used to avoid infinite waiting. The ISR masks the group enable register during service, clears all per-pin raw status bits, confirms the group is fully cleared, and then re-enables the group. The stimulus at 0xA0243ffc is used to toggle input levels.", "Hidden_Test_Steps_Procedure": "Entry: void test_case(). 1) Instance interrupt enable: Ifdef GPIO0: GIC_EnableIRQ(87); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) [WRITE, mask: LSS_SYSREG_INTR_EN1_GPIO0_INTR]. Ifdef GPIO1: GIC_EnableIRQ(88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) [WRITE]. 2) Per-pin posedge config: for(i=0..31){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(i4), 0x00020000) [WRITE, bit17=1] }. 3) Set groups input mode: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); wait_on(10). 4) Enable all group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF) [WRITE]. 5) For each pin: write_reg(0xA0243ffc, 0x00000000) [WRITE, ensure low]; wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF) [WRITE, rising edge]; timeout=2000; while((int_pend==1) && (--timeout>0)){ wait_on(10)}; if(timeout==0){ test_err++; break;} write_reg(0xA0243ffc, 0x00000000) [WRITE]; wait_on(10). 6) finish(test_err). ISR: void Default_IRQHandler(). wr_val=(1<<i); int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) [WRITE, mask group]; if((rdata_grp & 0xffffffff)!=0){ success } else { printf error; test_err++}. Clear raws: for(j=0..31){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000) [WRITE, iclr bit16] }; wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) [READ]; if(rdata_grp==0x0){ success } else { printf error; test_err++ }. Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR) [WRITE,W1C]; rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1) [READ]; if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0){ printf error; test_err++ }. Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0){ printf error; test_err++ }. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF) [WRITE, re-enable]; GIC_ClearIRQ(87/88). Timing: wait_on(10/2) delays; polling wait with timeout=2000. Loops: per-pin loop (32) and ISR per event.", "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_LSS_SYSREG_RAW_STCR1", "Hidden_Validation_Acceptance_Criteria": "Main loop: int_pend must clear before timeout for each pin; else test_err++. ISR: group status must be non-zero on entry; after writing per-pin iclr across all pins, group status must be zero. System-level raw status must be cleared after W1C write and verified by readback. Re-enable group interrupts and clear platform IRQ at end. Overall pass if test_err==0." } }'''

# Final column order for TestPlan sheet (visible)
MAIN_COLUMNS_ORDER = [
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
    'Code Generation (Required / Not)',
]

# Meta columns (copied to very hidden sheet)
META_COLUMNS = [
    'Hidden_Test_Case_Name',
    'Hidden_Test_Description',
    'Hidden_Remarks',
    'Hidden_Test_Steps_Procedure',
    'Hidden_Impacted_Registers',
    'Hidden_Validation_Acceptance_Criteria',
]

# Styling helpers
THIN_BORDER = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
)
HEADER_FILL = PatternFill(fill_type='solid', start_color='4472C4', end_color='4472C4')  # solid blue


def ist_now():
    return datetime.now(timezone(timedelta(hours=5, minutes=30)))


def ensure_output_dir(path: str):
    os.makedirs(path, exist_ok=True)


def load_rows_from_json(payload: str):
    try:
        obj = json.loads(payload)
    except Exception as e:
        print(f"ERROR: Invalid JSON input: {e}", file=sys.stderr)
        sys.exit(2)

    if isinstance(obj, dict):
        # Preserve first-seen order
        rows = list(obj.values())
    elif isinstance(obj, list):
        rows = obj
    else:
        print("ERROR: JSON root must be array or object of rows", file=sys.stderr)
        sys.exit(2)

    if not rows:
        print("ERROR: JSON contains zero rows", file=sys.stderr)
        sys.exit(2)

    # Ensure each row is a dict
    for i, r in enumerate(rows, 1):
        if not isinstance(r, dict):
            print(f"ERROR: Row {i} is not an object", file=sys.stderr)
            sys.exit(2)
    return rows


def union_headers_preserve_order(rows):
    headers = []
    seen = set()
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                headers.append(k)
    return headers


def write_data_sheet(wb, headers, rows):
    ws = wb.active
    ws.title = 'Data'
    # Header row
    for c, h in enumerate(headers, 1):
        cell = ws.cell(row=1, column=c, value=h)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Data rows
    for r_idx, row in enumerate(rows, 2):
        for c_idx, h in enumerate(headers, 1):
            ws.cell(row=r_idx, column=c_idx, value=row.get(h, ''))

    # Approx column auto-fit (based on max line length)
    autofit_columns(ws)
    return ws


def autofit_columns(ws):
    for col in range(1, ws.max_column + 1):
        letter = get_column_letter(col)
        max_len = 0
        for row in range(1, ws.max_row + 1):
            v = ws.cell(row=row, column=col).value
            if v is None:
                continue
            s = str(v)
            for line in s.split('\n'):
                max_len = max(max_len, len(line))
        # heuristic: pad a bit, clamp to reasonable bounds
        width = max(10, min(max_len + 2, 120))
        ws.column_dimensions[letter].width = width


def create_meta_sheet(wb, headers, rows):
    ws_meta = wb.create_sheet('Meta_data_sheet')
    # Write headers (META columns only)
    for c, h in enumerate(META_COLUMNS, 1):
        ws_meta.cell(row=1, column=c, value=h).font = Font(bold=True)
    for r_idx, row in enumerate(rows, 2):
        for c_idx, h in enumerate(META_COLUMNS, 1):
            ws_meta.cell(row=r_idx, column=c_idx, value=row.get(h, ''))
    # Very hidden
    ws_meta.sheet_state = 'veryHidden'
    autofit_columns(ws_meta)
    return ws_meta


def rename_and_normalize_main_sheet(wb, headers):
    ws = wb['Data']
    ws.title = 'TestPlan'

    # Remove meta columns from TestPlan
    header_to_col = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    to_delete = [header_to_col[h] for h in META_COLUMNS if h in header_to_col]
    # Delete from rightmost to leftmost to avoid index shifts
    for col_idx in sorted(to_delete, reverse=True):
        ws.delete_cols(col_idx)

    # Recompute header map after deletions
    header_map = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Reorder columns to MAIN_COLUMNS_ORDER on the same sheet
    new_order = []
    for name in MAIN_COLUMNS_ORDER:
        if name not in header_map:
            # Ensure missing columns become blank columns with header
            ws.cell(row=1, column=ws.max_column + 1, value=name)
            header_map[name] = ws.max_column
        new_order.append(header_map[name])

    # Create a snapshot of current data by rows following the new order
    data = []
    for r in range(1, ws.max_row + 1):
        row_vals = []
        for src_col in new_order:
            row_vals.append(ws.cell(row=r, column=src_col).value)
        data.append(row_vals)

    # Clear and rewrite according to new order
    ws.delete_cols(1, ws.max_column)
    for c, h in enumerate(MAIN_COLUMNS_ORDER, 1):
        ws.cell(row=1, column=c, value=h)
    for r_idx, row_vals in enumerate(data[1:], 2):
        for c_idx, v in enumerate(row_vals, 1):
            ws.cell(row=r_idx, column=c_idx, value=v)

    return ws


def apply_strict_formatting(ws):
    # Header row style
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal='center', vertical='center', wrap_text=False)
        cell.fill = HEADER_FILL

    # Data rows formatting
    # Column alignments: text left, numeric/index center/right
    header_names = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    wrap_cols = {
        'Test Description',
        'Remarks',
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria',
    }

    for r in range(2, ws.max_row + 1):
        for c, h in enumerate(header_names, 1):
            cell = ws.cell(row=r, column=c)
            # Determine alignment
            if h == 'Index':
                cell.alignment = Alignment(horizontal='center', vertical='top', wrap_text=False)
            elif h in wrap_cols:
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=False)
            # Borders
            cell.border = THIN_BORDER

    # Borders for header too
    for c in range(1, ws.max_column + 1):
        ws.cell(row=1, column=c).border = THIN_BORDER


def number_wrapped_cells(ws):
    # Apply numbering inside cells for the two specified columns
    target_cols = {
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria',
    }
    header_to_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    def number_items(text: str) -> str:
        if text is None:
            return ''
        s = str(text).strip()
        if not s:
            return s
        # Deterministic split: by semicolons or newlines
        parts = []
        for chunk in re.split(r'[\n;]+', s):
            t = chunk.strip()
            if t:
                parts.append(t)
        if not parts:
            return s
        return "\n".join([f"{i+1}. {p}" for i, p in enumerate(parts)])

    for name in target_cols:
        if name in header_to_idx:
            col = header_to_idx[name]
            for r in range(2, ws.max_row + 1):
                cell = ws.cell(row=r, column=col)
                cell.value = number_items(cell.value)


def autofit_rows(ws):
    # Approximate row height based on wrapped content line counts
    header_height = 18
    ws.row_dimensions[1].height = header_height

    header_names = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    wrap_cols = {
        'Test Description',
        'Remarks',
        'Test Steps / Procedure',
        'Validation / Acceptance Criteria',
    }
    wrap_idx = [i + 1 for i, h in enumerate(header_names) if h in wrap_cols]

    base_height = 15
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in wrap_idx:
            v = ws.cell(row=r, column=c).value
            if v is None:
                continue
            lines = str(v).count('\n') + 1
            max_lines = max(max_lines, lines)
        ws.row_dimensions[r].height = base_height * max_lines


def apply_data_validation(ws):
    header_to_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}
    col_name = 'Code Generation (Required / Not)'
    if col_name not in header_to_idx:
        return
    col = header_to_idx[col_name]
    col_letter = get_column_letter(col)
    dv = DataValidation(type='list', formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
    rng = f"{col_letter}2:{col_letter}{ws.max_row}"
    dv.add(rng)
    ws.add_data_validation(dv)


def enforce_sheet_visibility(wb):
    # Only TestPlan (visible) and Meta_data_sheet (veryHidden) must exist
    names = list(wb.sheetnames)
    # Delete any lingering 'Data' sheets
    for n in names:
        if n == 'Data':
            ws = wb[n]
            wb.remove(ws)
    names = list(wb.sheetnames)
    if set(names) - {'TestPlan', 'Meta_data_sheet'}:
        raise RuntimeError(f"Unexpected worksheets present: {set(names)}")
    # Ensure visibility states
    if wb['Meta_data_sheet'].sheet_state != 'veryHidden':
        wb['Meta_data_sheet'].sheet_state = 'veryHidden'


def validate_xlsx_binary(path: str):
    # Check as ZIP and as loadable workbook
    with zipfile.ZipFile(path, 'r') as zf:
        if '[Content_Types].xml' not in zf.namelist() or 'xl/workbook.xml' not in zf.namelist():
            raise RuntimeError('XLSX content structure invalid')
    # Load via openpyxl
    _ = load_workbook(path, read_only=True, data_only=True)


def save_workbook(wb, out_dir, ip_name):
    ensure_output_dir(out_dir)
    ts = ist_now()
    fname = f"{ip_name}_TestPlan_{ts.strftime('%Y%m%d')}_{ts.strftime('%H%M%S')}.xlsx"
    out_path = os.path.join(out_dir, fname)
    wb.save(out_path)
    validate_xlsx_binary(out_path)
    return out_path


def git_commit_and_push(file_path: str):
    import subprocess
    # Configure bot identity
    subprocess.run(['git', 'config', 'user.name', 'github-actions[bot]'], check=True)
    subprocess.run(['git', 'config', 'user.email', '41898282+github-actions[bot]@users.noreply.github.com'], check=True)
    # Add only the generated XLSX
    subprocess.run(['git', 'add', '--force', file_path], check=True)
    # Commit
    msg = 'Final formatted Excel generated from JSON input'
    subprocess.run(['git', 'commit', '-m', msg], check=True)
    # Push
    subprocess.run(['git', 'push'], check=True)


def main():
    rows = load_rows_from_json(JSON_PAYLOAD)
    headers_union = union_headers_preserve_order(rows)

    wb = Workbook()
    write_data_sheet(wb, headers_union, rows)
    create_meta_sheet(wb, headers_union, rows)
    ws_main = rename_and_normalize_main_sheet(wb, headers_union)

    # Numbering before final autofit
    number_wrapped_cells(ws_main)

    # Strict formatting
    apply_strict_formatting(ws_main)
    autofit_columns(ws_main)
    autofit_rows(ws_main)

    # Data validation only on specified column
    apply_data_validation(ws_main)

    # Visibility enforcement
    enforce_sheet_visibility(wb)

    # Save and validate
    out_path = save_workbook(wb, OUTPUT_DIR, IP_NAME)
    print(f"Generated: {out_path}")

    if COMMIT_CHANGES:
        git_commit_and_push(out_path)


if __name__ == '__main__':
    main()
