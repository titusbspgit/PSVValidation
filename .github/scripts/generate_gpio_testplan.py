#!/usr/bin/env python3
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timezone, timedelta
from collections import OrderedDict

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl not available: {e}")
    sys.exit(1)

# Canonical Test Plan JSON (object keyed by TC1, TC2, ...). Keep EXACTLY as provided.
TESTPLAN_JSON = r'''{
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface.",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validates default values and masked read/write behavior across the GPIO per-pin and group registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Default read behavior note: input value can float high unless driven; forcing input low can alter level-select causing mismatch versus expected defaults.",
    "Test Steps / Procedure": "1) Read masked defaults from GPIO_8 through GPIO_39 and the listed group registers; skip registers marked non-readable or flagged for reset-skip.\n2) For each test pattern, write only write-enabled bits to each listed register and then read back using the read mask.\n3) For each register, compute expected as written masked bits OR default masked bits for non-writable fields and compare.\n4) Repeat for all patterns; accumulate any mismatches.\n5) Declare pass if no default or write/read mismatches; otherwise fail.",
    "Impacted Registers": [
      "GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39",
      "GPIO_INTR_RAW_STCLR1","INTR1_INTR_EN1","INTR1_INTR_STS1","INTR2_INTR_EN1","INTR2_INTR_STS1",
      "GPIO_IO_CTRL_GROUP1","GPIO_IO_CTRL_GROUP2","GPIO_IO_CTRL_GROUP3","GPIO_IO_CTRL_GROUP4",
      "GPIO_DOUT_GROUP1","GPIO_DOUT_GROUP2","GPIO_DOUT_GROUP3","GPIO_DOUT_GROUP4",
      "GPIO_DIN_GROUP1","GPIO_DIN_GROUP2","GPIO_DIN_GROUP3","GPIO_DIN_GROUP4"
    ],
    "Validation / Acceptance Criteria": "1) Default check: For each readable register not in the reset-skip list, the masked read equals the documented default. Pass if all match; any mismatch fails.\n2) Write/read check: For each pattern and each writable+readable register, masked readback equals (written masked bits OR default for non-writable masked bits). Pass if all match; any mismatch fails.\n3) Overall: Pass only if both default and write/read mismatch counters remain zero; else fail.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Verify default reset values and masked write/read behavior for GPIO registers listed in test_define.c. Uses addr_array[49] spanning per-pin (GPIO_8..GPIO_39) and group registers. Default check masks read with 0xfffffffe and compares to default_value_array[i] when readable and not skipped. Write/read check iterates 6 patterns; writes (pattern & write_mask_array[i]) to each writable addr; reads back masked; expected computed as ((pattern & read_mask & write_mask) | ((~write_mask) & read_mask & default)). Accumulates def_fail_cnt and wr_fail_cnt; finish(0) if both zero else finish(1).",
    "Hidden_Remarks": "when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
    "Hidden_Test_Steps_Procedure": "1) Initialize def_fail_cnt=0, wr_fail_cnt=0.\n2) Call chk_rst_val(): For i=0..CNT-1 (CNT=49), addr=addr_array[i]. If skip_rst_array[i]==1, continue. If read_mask_array[i]==0x00000000, continue. Read data_rd=read_reg(addr); data=(data_rd & 0xfffffffe). If data==default_value_array[i] then (optional PASS print) else increment def_fail_cnt and print failure with addr, expected, read.\n3) Call chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}. For each j=0..5: data_wr=chk_val[j]. Write phase: For i=0..CNT-1, addr=addr_array[i]. If skip_array[i]==1, continue. If write_mask_array[i]==0x00000000, continue. Else write_reg(addr, (data_wr & write_mask_array[i])). Read/verify phase: For i=0..CNT-1, addr=addr_array[i]. If skip_array[i]==1, continue. If write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000, continue. Else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); If data_rd==exp_val then (optional PASS print) else wr_fail_cnt++ and print mismatch with addr, expected, read.\n4) After loops, if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).\n5) Address list (addr_array[49]): MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4.\n6) Skip controls: skip_array marks some group registers to skip in write/read; skip_rst_array marks some group registers to skip in reset-value check.",
    "Hidden_Impacted_Registers": [
      "MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GP0_GPIO_9","MIZAR_GPIO_GP0_GPIO_10","MIZAR_GPIO_GP0_GPIO_11","MIZAR_GPIO_GP0_GPIO_12","MIZAR_GPIO_GP0_GPIO_13","MIZAR_GPIO_GP0_GPIO_14","MIZAR_GPIO_GP0_GPIO_15","MIZAR_GPIO_GP0_GPIO_16","MIZAR_GPIO_GP0_GPIO_17","MIZAR_GPIO_GP0_GPIO_18","MIZAR_GPIO_GP0_GPIO_19","MIZAR_GPIO_GP0_GPIO_20","MIZAR_GPIO_GP0_GPIO_21","MIZAR_GPIO_GP0_GPIO_22","MIZAR_GPIO_GP0_GPIO_23","MIZAR_GPIO_GP0_GPIO_24","MIZAR_GPIO_GP0_GPIO_25","MIZAR_GPIO_GP0_GPIO_26","MIZAR_GPIO_GP0_GPIO_27","MIZAR_GPIO_GP0_GPIO_28","MIZAR_GPIO_GP0_GPIO_29","MIZAR_GPIO_GP0_GPIO_30","MIZAR_GPIO_GP0_GPIO_31","MIZAR_GPIO_GP0_GPIO_32","MIZAR_GPIO_GP0_GPIO_33","MIZAR_GPIO_GP0_GPIO_34","MIZAR_GPIO_GP0_GPIO_35","MIZAR_GPIO_GP0_GPIO_36","MIZAR_GPIO_GP0_GPIO_37","MIZAR_GPIO_GP0_GPIO_38","MIZAR_GPIO_GP0_GPIO_39",
      "MIZAR_GPIO_GPIO_INTR_RAW_STCLR1","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_GPIO_GP0_INTR2_INTR_EN1","MIZAR_GPIO_GP0_INTR2_INTR_STS1",
      "MIZAR_GPIO_GPIO_IO_CTRL_GROUP1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP2","MIZAR_GPIO_GPIO_IO_CTRL_GROUP3","MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
      "MIZAR_GPIO_GPIO_DOUT_GROUP1","MIZAR_GPIO_GPIO_DOUT_GROUP2","MIZAR_GPIO_GPIO_DOUT_GROUP3","MIZAR_GPIO_GPIO_DOUT_GROUP4",
      "MIZAR_GPIO_GPIO_DIN_GROUP1","MIZAR_GPIO_GPIO_DIN_GROUP2","MIZAR_GPIO_GPIO_DIN_GROUP3","MIZAR_GPIO_GPIO_DIN_GROUP4"
    ],
    "Hidden_Validation_Acceptance_Criteria": "1) Default-value check passes only if for every i where read_mask_array[i] != 0 and skip_rst_array[i] == 0, (read_reg(addr_array[i]) & 0xfffffffe) == default_value_array[i].\n2) Write/read check passes only if for every data_wr in {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000} and for every i where write_mask_array[i] != 0 and read_mask_array[i] != 0 and skip_array[i] == 0: (read_reg(addr_array[i]) & read_mask_array[i]) == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])).\n3) Overall pass if def_fail_cnt == 0 and wr_fail_cnt == 0; else fail."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "neie",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Validates negative-edge interrupt generation and servicing for GPIO pins 8 to 39.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Uses IRQ 87 or 88 depending on GPIO instance and enables system-level interrupt. A pad-drive register at 0xA0243ffc is used to create falling edges.",
    "Test Steps / Procedure": "1) Enable the appropriate interrupt line in the interrupt controller and system interrupt enable register.\n2) Drive the pad control register at 0xA0243ffc high to establish a known state.\n3) For each pin 8 through 39, configure the per-pin register (GPIO_8 + index offset) for input mode, enable negative-edge interrupts, and clear pending status.\n4) For each pin 8 through 39, clear its raw status in GPIO_INTR_RAW_STCLR1 and enable only that pin in INTR1_INTR_EN1.\n5) Arm the wait condition, force a high level, then drive a falling edge for the selected pin using 0xA0243ffc.\n6) Wait with timeout for the interrupt to arrive.\n7) In the handler, return the pad to high, read the per-pin register to confirm input is low after the edge, verify the group status in INTR1_INTR_STS1, clear the per-pin status and the group raw status, confirm the group status is cleared, clear the system raw status, and clear the interrupt at the controller.",
    "Impacted Registers": [
      "GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39",
      "GPIO_INTR_RAW_STCLR1","INTR1_INTR_EN1","INTR1_INTR_STS1","INTR_EN1","RAW_STCR1"
    ],
    "Validation / Acceptance Criteria": "1) Each falling edge produces a pending interrupt within the timeout; otherwise the test fails for that pin.\n2) After the edge, the per-pin input value reads low; any other value fails.\n3) The corresponding group status bit becomes set and is then cleared to zero after servicing; any deviation fails.\n4) The system raw status is cleared after servicing; if it remains set, the test fails.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Configure GPIO[8..39] for input + negative-edge interrupt and verify interrupt behavior per pin. Enable GIC IRQ 87 (GPIO0) or 88 (GPIO1) based on defines. Enable system interrupt by writing MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR. Initialize pad to all-high by write_reg(0xA0243ffc, 0xffffffff). For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) to set doe=1 (input), neie=1, and iclr=1; wait_on(10). For each i: wr_val=1u<<i; pre-clear group raw by write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enable only this bit via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); set int_pend=1; generate falling edge: write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val). Poll with timeout=5000 while(int_pend) wait_on(10); on timeout: print error and test_err++. finish(test_err). ISR Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff) to return to known state; raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr). If ((rdata & 0x1) != 0) test_err++. If ((rdata & 0x2) != 0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr) == 0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(raddr2, (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; if GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); if GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88);} else { test_err++; }",
    "Hidden_Remarks": "Uses 0xA0243ffc to drive pad stimulus; compile-time defines GPIO0/GPIO1 select IRQ line (87/88) and system-interrupt bits; bounded wait loop with timeout=5000 and wait_on(10) increments.",
    "Hidden_Test_Steps_Procedure": "1) test_err=0.\n2) Ifdef GPIO0: GIC_EnableIRQ(87); Ifdef GPIO1: GIC_EnableIRQ(88).\n3) Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); Ifdef GPIO1: write_reg(MIZAR_LSS_SYS_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n4) write_reg(0xA0243ffc, 0xffffffff).\n5) For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8 + (i * 4); write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16)); wait_on(10).\n6) For i=0..31: wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { printf timeout; test_err++; }\n7) finish(test_err).\n8) Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8 + (i * 4); rdata=read_reg(raddr); if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr) == 0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8 + (i * 4); write_reg(raddr2, (1u<<20) | (1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); } else { test_err++; }",
    "Hidden_Impacted_Registers": [
      "MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GPIO_INTR_RAW_STCLR1","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_LSS_SYSREG_INTR_EN1","MIZAR_LSS_SYSREG_RAW_STCR1"
    ],
    "Hidden_Validation_Acceptance_Criteria": "1) Timeout path: If interrupt is not received within the bounded loop (timeout reaches 0), increment test_err and continue; overall pass requires test_err==0 at end.\n2) Post-edge input check: After ISR edge handling, (rdata & 0x1) must be 0; else test_err++.\n3) Group status check: If (rdata & 0x2) != 0, then read MIZAR_GPIO_GP0_INTR1_INTR_STS1; (rdata_grp & (1<<i)) must be nonzero; else test_err++.\n4) Clear verification: After write_reg(per-pin, doe|iclr) and write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1<<i)), read MIZAR_GPIO_GP0_INTR1_INTR_STS1 must return 0; else test_err++.\n5) System raw clear: After write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, GPIOx bit), the system raw should be cleared; ISR also clears the GIC IRQ.",
    "Traceability": "https://github.com/titusbspgit/PSVValidation/blob/main/TestRepo/gpio/test_gpio_negedge_intr_en/program.c"
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "peie",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validates positive-edge interrupt generation and servicing for GPIO pins 8 to 39 with group input configuration.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Uses system interrupt enable and interrupt controller lines 87 or 88. A pad-drive register at 0xA0243ffc provides the rising-edge stimulus.",
    "Test Steps / Procedure": "1) Enable the appropriate interrupt line in the interrupt controller and enable the system interrupt.\n2) For each pin 8 through 39, set the per-pin register (GPIO_8 + index offset) to enable positive-edge interrupt.\n3) Configure all pins 8 through 39 as inputs using GPIO_IO_CTRL_GROUP1 through GPIO_IO_CTRL_GROUP4.\n4) Enable all per-pin interrupt lines through INTR1_INTR_EN1.\n5) For each pin, force a low level, arm the wait condition, then drive a rising edge using 0xA0243ffc and wait with timeout.\n6) In the handler, read INTR1_INTR_STS1, mask group interrupt output, clear per-pin raw status across all pins, verify the group status clears to zero, clear the system raw status, and re-enable group interrupt output.",
    "Impacted Registers": [
      "GPIO_8","GPIO_9","GPIO_10","GPIO_11","GPIO_12","GPIO_13","GPIO_14","GPIO_15","GPIO_16","GPIO_17","GPIO_18","GPIO_19","GPIO_20","GPIO_21","GPIO_22","GPIO_23","GPIO_24","GPIO_25","GPIO_26","GPIO_27","GPIO_28","GPIO_29","GPIO_30","GPIO_31","GPIO_32","GPIO_33","GPIO_34","GPIO_35","GPIO_36","GPIO_37","GPIO_38","GPIO_39",
      "GPIO_IO_CTRL_GROUP1","GPIO_IO_CTRL_GROUP2","GPIO_IO_CTRL_GROUP3","GPIO_IO_CTRL_GROUP4",
      "INTR1_INTR_EN1","INTR1_INTR_STS1","INTR_EN1","RAW_STCR1"
    ],
    "Validation / Acceptance Criteria": "1) Each rising edge generates an interrupt within the timeout; a timeout indicates failure.\n2) Group status becomes non-zero on an event and returns to zero after clearing; any deviation fails.\n3) The system raw status clears after servicing; if it remains set, the test fails.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Enable posedge interrupts for GPIO[8..39], configure input mode via group IO control, generate rising edges via pad drive, and verify group status and clearing. Steps: Ifdef GPIO0: GIC_EnableIRQ(87); Ifdef GPIO1: GIC_EnableIRQ(88). Enable system interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) to enable posedge (bit17). wait_on(10). Set inputs: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); same for GROUP2..GROUP4. wait_on(10). Enable group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); Timeout loop: int timeout=2000; while(int_pend==1 && --timeout>0) wait_on(10); if(timeout==0) { print error; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10). finish(test_err). ISR Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask during service. If((rdata_grp & 0xffffffff)!=0) success print else print error and test_err++. Clear per-pin raw: for j=0..31 write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000). wait_on(2). rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp==0x0) success else error and test_err++. Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0) print error and test_err++. Ifdef GPIO1: similar for GPIO1. Re-enable group: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). Clear GIC IRQ (87/88).",
    "Hidden_Remarks": "Uses volatile int for interrupt completion detection; relies on 0xA0243ffc to toggle pad levels; masks/re-enables group interrupt output within ISR.",
    "Hidden_Test_Steps_Procedure": "1) Ifdef GPIO0: GIC_EnableIRQ(87); Ifdef GPIO1: GIC_EnableIRQ(88).\n2) Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); Ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000) to enable posedge (bit17=1).\n4) wait_on(10).\n5) Set inputs: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF).\n6) wait_on(10).\n7) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).\n8) For i=0..31: write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); int timeout=2000; while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) { printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10).\n9) finish(test_err).\n10) Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) { optional success print } else { printf error; test_err++; } for (j=0..31) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { optional success print } else { printf error; test_err++; } Ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf error; test_err++; } Ifdef GPIO1: analogous for GPIO1. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); Ifdef GPIO0: GIC_ClearIRQ(87); Ifdef GPIO1: GIC_ClearIRQ(88).",
    "Hidden_Impacted_Registers": [
      "MIZAR_GPIO_GP0_GPIO_8","MIZAR_GPIO_GP0_INTR1_INTR_EN1","MIZAR_GPIO_GP0_INTR1_INTR_STS1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP1","MIZAR_GPIO_GPIO_IO_CTRL_GROUP2","MIZAR_GPIO_GPIO_IO_CTRL_GROUP3","MIZAR_GPIO_GPIO_IO_CTRL_GROUP4","MIZAR_LSS_SYSREG_INTR_EN1","MIZAR_LSS_SYSREG_RAW_STCR1"
    ],
    "Hidden_Validation_Acceptance_Criteria": "1) Timeout: If rising edge does not produce an interrupt before timeout reaches zero, increment test_err and break; overall pass requires test_err==0.\n2) Group status: On ISR entry, MIZAR_GPIO_GP0_INTR1_INTR_STS1 must be non-zero; otherwise test_err++.\n3) Clear verification: After per-pin raw clear (writing 0x00010000 to each per-pin register), MIZAR_GPIO_GP0_INTR1_INTR_STS1 must read 0; otherwise test_err++.\n4) System raw clear: After writing MIZAR_LSS_SYSREG_RAW_STCR1 with the appropriate GPIOx bit, a subsequent read must show the bit cleared; otherwise test_err++.\n5) Group re-enable and IRQ clear occur before exiting the ISR."
  }
}'''

META_COLS = [
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
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation (Required / Not)",
]

APPEND_COLS = ["Owner", "Priority", "Category", "Traceability"]

ALLOWED_DV = ["Required", "Blank", "Not Required"]

BLUE_FILL = PatternFill(fill_type="solid", start_color="FF4F81BD", end_color="FF4F81BD")
HEADER_FONT = Font(bold=True, color="FFFFFFFF")
CENTER = Alignment(horizontal="center", vertical="center", wrap_text=False)
LEFT_WRAP = Alignment(horizontal="left", vertical="top", wrap_text=True)
TOP_LEFT = Alignment(horizontal="left", vertical="top", wrap_text=False)
THIN_BORDER = Border(left=Side(style='thin'), right=Side(style='thin'), top=Side(style='thin'), bottom=Side(style='thin'))


def ist_now():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist)


def json_to_array(obj):
    # Convert dict of TCs to array in key order without mutating keys/values
    if not isinstance(obj, dict):
        raise ValueError("Top-level JSON must be an object keyed by TC ids")
    arr = []
    for key in sorted(obj.keys(), key=lambda x: (len(x), x)):
        arr.append(obj[key])
    return arr


def union_keys(records):
    seen = OrderedDict()
    for rec in records:
        for k in rec.keys():
            if k not in seen:
                seen[k] = True
    return list(seen.keys())


def auto_col_width(ws):
    # compute width based on text length
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            val = cell.value
            if val is None:
                continue
            s = str(val)
            s = s.replace("\n", " ")
            if len(s) > max_len:
                max_len = len(s)
        ws.column_dimensions[col_letter].width = min(max(10, max_len + 2), 80)


def normalize_numbering(text):
    if text is None:
        return text
    lines = str(text).split('\n')
    out = []
    idx = 1
    for ln in lines:
        s = ln.strip()
        if not s:
            continue
        # strip any leading bullets or numbering
        s = re.sub(r'^(?:\d+\)|\d+\.|[-*•])+\s*', '', s)
        out.append(f"{idx}. {s}")
        idx += 1
    return "\n".join(out)


def write_workbook(records, out_path):
    if not records:
        raise ValueError("No records to write")
    # Phase 1: Create workbook with a single sheet named Data
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Normalize schema union and preserve first-seen order
    cols = union_keys(records)

    # Write header
    for j, key in enumerate(cols, start=1):
        cell = ws.cell(row=1, column=j, value=key)
        cell.font = HEADER_FONT
        cell.fill = BLUE_FILL
        cell.alignment = CENTER
        cell.border = THIN_BORDER
    ws.freeze_panes = "A2"

    # Write data rows exactly
    for i, rec in enumerate(records, start=2):
        for j, key in enumerate(cols, start=1):
            val = rec.get(key, "")
            ws.cell(row=i, column=j, value=val)

    auto_col_width(ws)

    # Phase 2: Create Meta_data_sheet and copy META columns as-is, then Very Hidden
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    # Meta header
    for j, key in enumerate(META_COLS + ["macros_to_registers_mapping"], start=1):
        cell = meta_ws.cell(row=1, column=j, value=key)
        cell.font = HEADER_FONT
        cell.fill = BLUE_FILL
        cell.alignment = CENTER
        cell.border = THIN_BORDER
    # Copy meta column data
    for i, rec in enumerate(records, start=2):
        for j, key in enumerate(META_COLS, start=1):
            meta_ws.cell(row=i, column=j, value=rec.get(key, ""))
    # Build macros mapping by scanning repo headers (best-effort)
    mapping = resolve_macros_mapping(records)
    # Put mapping JSON in last column as a compact JSON string for transparency
    import json as _json
    mapping_json = _json.dumps(mapping, ensure_ascii=False, indent=None)
    for i in range(2, 2 + len(records)):
        meta_ws.cell(row=i, column=len(META_COLS) + 1, value=mapping_json)
    meta_ws.sheet_state = 'veryHidden'

    # Phase 2 continued: Rename Data -> TestPlan and reorder columns; remove META columns
    ws.title = "TestPlan"

    # Map column indices
    col_index = {name: idx for idx, name in enumerate(cols, start=1)}

    # Build ordered list: MAIN_ORDER, then append extra fields if they exist among cols but not in META
    main_cols_present = [c for c in MAIN_ORDER if c in col_index]

    # Append Owner, Priority, Category, Traceability as new tail columns
    final_cols = main_cols_present + APPEND_COLS

    # Build a new 2D array for TestPlan
    data_rows = []
    # header
    data_rows.append(final_cols)
    # rows
    for rec in records:
        row = []
        for key in final_cols:
            if key in APPEND_COLS:
                # Fill defaults
                if key == "Owner":
                    row.append("GPIO_Team")
                elif key == "Priority":
                    # Interrupt tests P1, register R/W P2
                    title = str(rec.get("Test Case Name", "")).lower()
                    if "intr" in title or rec.get("Mode", "").lower() == "interrupt":
                        row.append("P1")
                    else:
                        row.append("P2")
                elif key == "Category":
                    title = str(rec.get("Test Case Name", "")).lower()
                    if "intr" in title or rec.get("Mode", "").lower() == "interrupt":
                        row.append("Interrupt")
                    else:
                        row.append("Sanity")
                elif key == "Traceability":
                    # Use provided link if present; otherwise leave blank (no inference)
                    row.append(rec.get("Traceability", ""))
                else:
                    row.append("")
            else:
                row.append(rec.get(key, ""))
        data_rows.append(row)

    # Clear existing sheet content then write new in-place
    ws.delete_rows(1, ws.max_row)
    for r, row in enumerate(data_rows, start=1):
        for c, val in enumerate(row, start=1):
            cell = ws.cell(row=r, column=c, value=val)
            if r == 1:
                cell.font = HEADER_FONT
                cell.fill = BLUE_FILL
                cell.alignment = CENTER
            cell.border = THIN_BORDER
    ws.freeze_panes = "A2"

    # Wrap specific columns
    wrap_cols = set([
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    ])
    header_to_col = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}

    # Normalize numbering for steps and VAC inside TestPlan sheet only
    steps_col = header_to_col.get("Test Steps / Procedure")
    vac_col = header_to_col.get("Validation / Acceptance Criteria")
    if steps_col:
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=steps_col)
            cell.value = normalize_numbering(cell.value)
    if vac_col:
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=vac_col)
            cell.value = normalize_numbering(cell.value)

    for name, col in header_to_col.items():
        for r in range(2, ws.max_row + 1):
            cell = ws.cell(row=r, column=col)
            if name in wrap_cols:
                cell.alignment = LEFT_WRAP
            else:
                # Align text left by default; numeric-like center/right can be inferred by field, but do not mutate values
                if name == "Index":
                    cell.alignment = Alignment(horizontal="center", vertical="top")
                else:
                    cell.alignment = TOP_LEFT

    # Auto-fit widths after writing
    auto_col_width(ws)

    # Data validation for Code Generation (Required / Not)
    code_col = header_to_col.get("Code Generation (Required / Not)")
    if code_col:
        dv_list = ", ".join(ALLOWED_DV)
        dv = DataValidation(type="list", formula1=f'"{dv_list}"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        dv.add(f"{ws.cell(row=2, column=code_col).coordinate}:{ws.cell(row=ws.max_row, column=code_col).coordinate}")

    # Safety: Ensure no sheet named 'Data' remains
    if any(sh.title == 'Data' for sh in wb.worksheets):
        # Attempt to delete; else fail
        try:
            for sh in wb.worksheets:
                if sh.title == 'Data':
                    wb.remove(sh)
        except Exception as e:
            raise RuntimeError(f"Failed to remove residual 'Data' sheet: {e}")

    # Save workbook
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    wb.save(out_path)

    # Validate XLSX by attempting to open as zip and checking core parts
    with zipfile.ZipFile(out_path, 'r') as zf:
        must_have = [
            '[Content_Types].xml',
            '_rels/.rels',
            'xl/workbook.xml',
            'xl/worksheets/sheet1.xml',
        ]
        names = zf.namelist()
        for part in must_have:
            if part not in names:
                raise RuntimeError(f"XLSX validation failed: missing {part}")


def resolve_macros_mapping(records):
    # Collect macro-like tokens from Hidden_Impacted_Registers arrays
    macros = set()
    for rec in records:
        val = rec.get("Hidden_Impacted_Registers", [])
        if isinstance(val, list):
            for item in val:
                if isinstance(item, str) and (item.startswith("MIZAR_") or item.startswith("LSS_SYSREG_")):
                    macros.add(item)
    # Search headers in repo for #define lines
    mapping = []
    header_paths = []
    for root, _, files in os.walk('.', topdown=True):
        for fn in files:
            if fn.endswith(('.h', '.hpp')) and ('gpio' in fn.lower() or 'sysreg' in fn.lower()):
                header_paths.append(os.path.join(root, fn))
    defines = {}
    define_re = re.compile(r'^\s*#\s*define\s+(\w+)\s+(.+?)(?:\s*/\/(.*))?$')
    for hp in header_paths:
        try:
            with open(hp, 'r', errors='ignore') as f:
                for line in f:
                    m = define_re.match(line)
                    if m:
                        name, value, comment = m.group(1), m.group(2), (m.group(3) or '').strip()
                        if name not in defines:
                            defines[name] = {"value": value.strip(), "comment": comment, "file": hp}
        except Exception:
            continue
    for m in sorted(macros):
        info = defines.get(m)
        if info:
            resolved = info["comment"] if info["comment"] else info["value"]
            notes = f"from {info['file']}"
        else:
            resolved = "UNRESOLVED"
            notes = "Not found in scanned headers"
        mapping.append({"macro": m, "register": resolved, "notes": notes})
    return mapping


def main():
    # Inputs via environment (provided by workflow) or defaults per task
    ip_name = os.environ.get('IP_NAME', 'GPIO')
    out_dir = os.environ.get('OUTPUT_DIR', 'Test_Output/GPIO/TestPlan')

    # Build records array from canonical object
    try:
        obj = json.loads(TESTPLAN_JSON)
    except Exception as e:
        print(f"FAILURE: Invalid JSON: {e}")
        sys.exit(2)

    records = json_to_array(obj)
    if not isinstance(records, list) or len(records) == 0:
        print("FAILURE: Empty JSON array after normalization")
        sys.exit(2)

    # Compute IST timestamp for filename
    now = ist_now()
    stamp_name = now.strftime('%Y%m%d_%H%M%S')
    out_name = f"{ip_name}_TestPlan_{stamp_name}.xlsx"
    out_path = os.path.join(out_dir, out_name)

    # Write workbook
    write_workbook(records, out_path)

    # Emit outputs for subsequent commit step
    print(f"OUTPUT_XLSX={out_path}")
    print(f"IST_HUMAN={now.strftime('%Y-%m-%d %H:%M:%S IST')}")

    # Persist for workflow steps
    with open(os.environ.get('GITHUB_ENV', '/tmp/github_env'), 'a') as envf:
        envf.write(f"OUTPUT_XLSX={out_path}\n")
        envf.write(f"IST_HUMAN={now.strftime('%Y-%m-%d %H:%M:%S IST')}\n")

if __name__ == '__main__':
    main()
