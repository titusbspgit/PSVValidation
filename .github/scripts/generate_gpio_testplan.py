#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generates a REAL .xlsx TestPlan workbook from embedded JSON using openpyxl.
- Sheet1: TestPlan (visible)
- Sheet2: MetaData (VERY HIDDEN)
- Formatting: bold blue headers, wrap text, freeze first row, reasonable column widths
- Filename: <IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx (IST)
- Output directory created if missing

This script is designed for GitHub Actions but can run locally as well.
"""
import os
import json
from datetime import datetime, timezone, timedelta
from pathlib import Path
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter


def ist_now_string():
    ist_env = os.environ.get("IST_TS")
    if ist_env and len(ist_env) >= 15:  # Expecting YYYYMMDD_HHMMSS
        return ist_env
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz=ist).strftime("%Y%m%d_%H%M%S")


# Embedded JSON (final aggregated Test Plan JSON) — MUST remain verbatim
JSON_TEXT = r'''[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "data_in",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validate basic GPIO register write and read behavior on direction, data, and input registers to ensure values written are read back correctly and read-only inputs reflect pin state.",
    "Meta Test Description": "Performs write and read operations on GPIO registers and checks that writeable fields retain programmed values while input capture reflects external pin state. Exact register access patterns are not available from source in this step.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Initialize the platform and ensure the GPIO block is out of reset and clocked. 2) Program GPIO_SWPORTA_DDR with known patterns to set pin directions. 3) Program GPIO_SWPORTA_DR with known patterns for output data and read it back to confirm write/read integrity. 4) Read GPIO_EXT_PORTA (and data_in) to observe input values and confirm they match the expected external pin state. 5) Repeat with multiple patterns and edge cases; record pass/fail based on comparisons.",
    "Meta Test Steps / Procedure": "NA",
    "Impacted Registers": "GPIO_SWPORTA_DDR; GPIO_SWPORTA_DR; GPIO_EXT_PORTA; gp0_gpio_8",
    "Meta Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "PASS if read-back values from GPIO_SWPORTA_DDR and GPIO_SWPORTA_DR match written patterns and input reads from GPIO_EXT_PORTA/gp0_gpio_8 reflect the expected external pin state for all test patterns; otherwise FAIL.",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "Interrupt - Negative Edge",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Validate negative-edge triggered GPIO interrupts across a bank of pins by configuring per-pin control registers for falling-edge detection, enabling the interrupt, applying a falling-edge stimulus, and verifying that the interrupt status sets and clears correctly.",
    "Meta Test Description": "The test enables GIC IRQ for the GPIO block and unmasks the SoC interrupt for the selected GPIO instance. It initializes a pad-control register at 0xA0243ffc to drive the GPIO inputs high, then for each pin index i=0..31, configures the per-pin control register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with bits (1<<20)|(1<<18)|(1<<16) to enable input/interrupt on negative edge. For each pin, it clears any pending raw interrupt (MIZAR_GPIO_GPIO_INTR_RAW_STCLR1) for bit i, enables the per-pin interrupt in MIZAR_GPIO_GP0_INTR1_INTR_EN1, and then generates a falling edge by toggling 0xA0243ffc from 0xffffffff to ~wr_val, where wr_val = 1u<<i. It polls on a global flag (int_pend) with timeout while the Default_IRQHandler handles the interrupt. In the ISR, int_pend is cleared, the pad driver is restored to 0xffffffff, and the current pin control register value is read. It asserts that the pin level bit (rdata & 0x1) is low, and that an interrupt indicator bit (rdata & 0x2) is set. It then reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 to confirm the corresponding bit is set, reprograms the per-pin control register to (1<<20)|(1<<16) (disabling the edge selection bit), clears the raw interrupt with MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, and verifies the status register is 0x0. Finally, it clears the SoC aggregator raw status (MIZAR_LSS_SYSREG_RAW_STCR1) and the GIC IRQ (87 or 88 based on instance). Any timeout or failed condition increments test_err; finish(test_err) reports the result.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Ensure the GPIO interrupt line is enabled at the SoC level (INTR_EN1) and that the aggregated raw status (RAW_STCR1) is cleared within the ISR. External or board-level stimulus must be capable of producing a falling edge on the targeted GPIO pins. GIC routing for the GPIO interrupt must be active (platform IRQ 87 or 88).",
    "Test Steps / Procedure": "1) Enable the CPU interrupt for the GPIO block in the interrupt controller and unmask the SoC-level GPIO interrupt in INTR_EN1. 2) Initialize the pad drive so all targeted GPIO inputs are at logic high. 3) Configure the per-pin control registers (gp0_gpio_8 block) for negative-edge detection and input enable across the target pin range. 4) For each pin in the range: a) Clear any pending raw interrupt in GPIO_INTR_RAW_STCLR1 for that pin. b) Enable that pin’s mask in INTR1_INTR_EN1. c) Apply a falling-edge stimulus on the corresponding pin. d) Wait for the interrupt to be serviced. 5) In the interrupt service routine: a) Confirm the pin level reads low in the per-pin control register. b) Verify the group interrupt status bit is set in INTR1_INTR_STS1. c) Clear the raw interrupt in GPIO_INTR_RAW_STCLR1 and confirm the status clears. d) Clear the SoC aggregated status in RAW_STCR1 and the interrupt controller pending bit. 6) Repeat for all pins; report pass/fail based on accumulated results.",
    "Meta Test Steps / Procedure": "1) test_err = 0. 2) If GPIO0: GIC_EnableIRQ(87); if GPIO1: GIC_EnableIRQ(88). 3) If GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); if GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR). 4) write_reg(0xA0243ffc, 0xffffffff). 5) For i=0..31: addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10). 6) For i=0..31: wr_val = 1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0){ printf(\"ERROR: Timeout waiting for GPIO%u negedge interrupt\\n\", (unsigned)(i+8)); test_err++; }. 7) Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata=read_reg(raddr); if((rdata & 0x1)!=0){ test_err++; } if((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr)==0){ test_err++; } raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0){ test_err++; } If GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); If GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); } else { test_err++; }. 8) finish(test_err).",
    "Impacted Registers": "INTR_EN1; gp0_gpio_8; GPIO_INTR_RAW_STCLR1; INTR1_INTR_EN1; INTR1_INTR_STS1; RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; LSS_SYSREG_INTR_EN1_GPIO0_INTR; LSS_SYSREG_INTR_EN1_GPIO1_INTR; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GPIO_INTR_RAW_STCLR1; MIZAR_GPIO_GP0_INTR1_INTR_EN1; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1; LSS_SYSREG_RAW_STCR1_GPIO0_INTR; LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Validation / Acceptance Criteria": "PASS if, for each tested pin, a falling edge triggers an interrupt, the corresponding bit sets in INTR1_INTR_STS1, the pin-level check indicates low at the time of service, and the status clears to 0 after writing GPIO_INTR_RAW_STCLR1 and handling the aggregated status in RAW_STCR1; any timeout or mismatches result in FAIL.",
    "Meta Validation / Acceptance Criteria": "Failure conditions: (a) ISR not reached before timeout; (b) (rdata & 0x1) != 0 (pin not low) in ISR; (c) ((read INTR1_INTR_STS1) & (1u<<i)) == 0 when expected; (d) INTR1_INTR_STS1 != 0 after clearing via MIZAR_GPIO_GPIO_INTR_RAW_STCLR1; (e) missing/incorrect aggregator clear. PASS when test_err == 0 at finish().",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>; #include <lss_sysreg.h>; #include \"test_define.c\"; #include <test_common.h>; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\n\nconst int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\n\nconst int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\n\nconst int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};"
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Interrupt - Positive Edge",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validate positive-edge triggered GPIO interrupts across pins 8–39 by configuring per-pin control for rising-edge detection, enabling group interrupt output, driving a controlled pad data register to generate a single rising edge per pin, and verifying the group interrupt status asserts and fully clears after service.",
    "Meta Test Description": "The test enables the platform interrupt for the selected GPIO instance (IRQ 87 for GPIO0 or 88 for GPIO1) and unmasks the SoC interrupt in INTR_EN1. All 32 per-pin control registers starting at gp0_gpio_8 are programmed with 0x00020000 to enable positive-edge detection (bit17=1). GPIO_IO_CTRL_GROUP1..4 are written with 0x000000FF to place GPIOs 8–39 in input mode. The group interrupt mask GP0_INTR1_INTR_EN1 is set to 0xFFFFFFFF. For each iteration (i=0..31), the pad data register at 0xA0243ffc is first driven low (0x00000000), then int_pend is armed (set to 1), and the pad is driven high (0xFFFFFFFF) to generate a single rising edge. The main loop waits with a timeout for the ISR to clear int_pend; timeout increments test_err and breaks. After each iteration, the pad is driven low again to reset input level. In Default_IRQHandler, rdata_grp = read(GP0_INTR1_INTR_STS1); GP0_INTR1_INTR_EN1 is masked to 0 during service. If rdata_grp indicates no set bits, test_err increments. To clear per-pin raw status, each gp0_gpio_(8+i) is written with 0x00010000 (iclr bit16). After a short wait, GP0_INTR1_INTR_STS1 is re-read and must be 0x0; otherwise test_err increments. The SoC interrupt aggregator RAW_STCR1 is cleared with the instance-specific bit (GPIO0 or GPIO1) and verified to be cleared. GP0_INTR1_INTR_EN1 is then re-enabled (0xFFFFFFFF), and the GIC pending IRQ (87/88) is cleared. finish(test_err) reports overall pass/fail.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Ensure the GPIO interrupt is enabled in INTR_EN1 and routed through the interrupt controller. Confirm the pad data control at 0xA0243ffc drives the GPIO pads used by this test. After each interrupt, RAW_STCR1 must be cleared to avoid reassertion. GPIOs 8–39 must be configured in input mode via GPIO_IO_CTRL_GROUP1–4.",
    "Test Steps / Procedure": "1) Enable the interrupt controller line for the target GPIO instance and unmask the SoC interrupt in INTR_EN1. 2) Configure per-pin control starting at gp0_gpio_8 for rising-edge detection across all 32 pins. 3) Set GPIO_IO_CTRL_GROUP1–4 to place GPIOs 8–39 in input mode. 4) Enable the group interrupt output using GP0_INTR1_INTR_EN1. 5) For each pin iteration, force the pad data control at 0xA0243ffc low, arm the wait, then drive it high to create one rising edge; wait for the ISR with timeout. 6) In the ISR, confirm the group interrupt status is asserted, then clear all per-pin raw statuses via the per-pin control registers and verify the group status clears to zero. 7) Clear RAW_STCR1 for the GPIO instance and verify it is cleared; re-enable GP0_INTR1_INTR_EN1 and clear the pending interrupt in the controller. 8) Repeat for all iterations and report pass/fail based on accumulated errors.",
    "Meta Test Steps / Procedure": "1) Initialize: test_err = 0. If GPIO0: GIC_EnableIRQ(87); else if GPIO1: GIC_EnableIRQ(88). 2) Unmask SoC interrupt: If GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); if GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR). 3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); // enable posedge detect (bit17). 4) wait_on(10). 5) Configure IO direction to input: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). 6) wait_on(10). 7) Enable group mask: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 8) For i=0..31: a) write_reg(0xA0243ffc, 0x00000000); wait_on(10); b) int_pend = 1; c) write_reg(0xA0243ffc, 0xFFFFFFFF); d) timeout = 2000; while ((int_pend == 1) && (--timeout > 0)) wait_on(10); e) if (timeout == 0) { printf(\"ERROR: Timeout waiting for GPIO IRQ at i=%u\\n\", i); test_err++; break; } f) write_reg(0xA0243ffc, 0x00000000); wait_on(10). 9) Default_IRQHandler(): a) wr_val = 1 << i; int_pend = 0; b) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); c) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); d) if ((rdata_grp & 0xFFFFFFFF) == 0) { printf(\"ERROR: Group Interrupt not occured\\n\"); test_err++; } e) for j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); // per-pin raw clear (iclr bit16); wait_on(2); f) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { printf(\"ERROR : Group Interrupt clear failed: Interrupt value:%x\\n\", rdata_grp); test_err++; } g) If GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf(\"sysreg status not cleared : %0x\\n\", MIZAR_LSS_SYSREG_RAW_STCR1); test_err++; } h) If GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { printf(\"sysreg status not cleared : %0x\\n\", MIZAR_LSS_SYSREG_RAW_STCR1); test_err++; } i) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); j) If GPIO0: GIC_ClearIRQ(87); if GPIO1: GIC_ClearIRQ(88). 10) finish(test_err).",
    "Impacted Registers": "INTR_EN1; gp0_gpio_8; GPIO_IO_CTRL_GROUP1; GPIO_IO_CTRL_GROUP2; GPIO_IO_CTRL_GROUP3; GPIO_IO_CTRL_GROUP4; GP0_INTR1_INTR_EN1; GP0_INTR1_INTR_STS1; RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; LSS_SYSREG_INTR_EN1_GPIO0_INTR; LSS_SYSREG_INTR_EN1_GPIO1_INTR; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GPIO_IO_CTRL_GROUP1; MIZAR_GPIO_GPIO_IO_CTRL_GROUP2; MIZAR_GPIO_GPIO_IO_CTRL_GROUP3; MIZAR_GPIO_GPIO_IO_CTRL_GROUP4; MIZAR_GPIO_GP0_INTR1_INTR_EN1; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1; LSS_SYSREG_RAW_STCR1_GPIO0_INTR; LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Validation / Acceptance Criteria": "PASS if each generated rising edge results in the group interrupt status asserting in GP0_INTR1_INTR_STS1, and after clearing per-pin raw statuses and RAW_STCR1 the status reads 0x0 with no pending SoC status; no timeouts occur. Any timeout, missing group status, or uncleared status results in FAIL.",
    "Meta Validation / Acceptance Criteria": "Conditions for PASS: (a) In Default_IRQHandler, (read GP0_INTR1_INTR_STS1) has at least one bit set when the interrupt fires; (b) after writing 0x00010000 to all gp0_gpio_8+(j*4) and wait_on(2), (read GP0_INTR1_INTR_STS1) == 0x0; (c) after writing instance bit to RAW_STCR1, (read RAW_STCR1 & instance_bit) == 0; (d) main loop does not hit timeout while waiting for int_pend to clear. Any violation increments test_err, yielding FAIL when finish(test_err) != 0.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <lss_sysreg.h>; #include <stdio.h>; #include <test_define.c>; #include <test_common.h>; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\n\nconst unsigned int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\n\nconst unsigned int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\n\nconst unsigned int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};"
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Interrupt - Positive Edge",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validate positive-edge triggered GPIO interrupts across pins 8–39 by configuring per-pin control for rising-edge detection, enabling group interrupt output, driving a controlled pad data register to generate a single rising edge per pin, and verifying the group interrupt status asserts and fully clears after service.",
    "Meta Test Description": "The test enables the platform interrupt for the selected GPIO instance (IRQ 87 for GPIO0 or 88 for GPIO1) and unmasks the SoC interrupt in INTR_EN1. All 32 per-pin control registers starting at gp0_gpio_8 are programmed with 0x00020000 to enable positive-edge detection (bit17=1). GPIO_IO_CTRL_GROUP1..4 are written with 0x000000FF to place GPIOs 8–39 in input mode. The group interrupt mask GP0_INTR1_INTR_EN1 is set to 0xFFFFFFFF. For each iteration (i=0..31), the pad data register at 0xA0243ffc is first driven low (0x00000000), then int_pend is armed (set to 1), and the pad is driven high (0xFFFFFFFF) to generate a single rising edge. The main loop waits with a timeout for the ISR to clear int_pend; timeout increments test_err and breaks. After each iteration, the pad is driven low again to reset input level. In Default_IRQHandler, rdata_grp = read(GP0_INTR1_INTR_STS1); GP0_INTR1_INTR_EN1 is masked to 0 during service. If rdata_grp indicates no set bits, test_err increments. To clear per-pin raw status, each gp0_gpio_(8+i) is written with 0x00010000 (iclr bit16). After a short wait, GP0_INTR1_INTR_STS1 is re-read and must be 0x0; otherwise test_err increments. The SoC interrupt aggregator RAW_STCR1 is cleared with the instance-specific bit (GPIO0 or GPIO1) and verified to be cleared. GP0_INTR1_INTR_EN1 is then re-enabled (0xFFFFFFFF), and the GIC pending IRQ (87/88) is cleared. finish(test_err) reports overall pass/fail.",
    "Speed": "NA",
    "Mode": "ISR",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Ensure the GPIO interrupt is enabled in INTR_EN1 and routed through the interrupt controller. Confirm the pad data control at 0xA0243ffc drives the GPIO pads used by this test. After each interrupt, RAW_STCR1 must be cleared to avoid reassertion. GPIOs 8–39 must be configured in input mode via GPIO_IO_CTRL_GROUP1–4.",
    "Test Steps / Procedure": "1) Enable the interrupt controller line for the target GPIO instance and unmask the SoC interrupt in INTR_EN1. 2) Configure per-pin control starting at gp0_gpio_8 for rising-edge detection across all 32 pins. 3) Set GPIO_IO_CTRL_GROUP1–4 to place GPIOs 8–39 in input mode. 4) Enable the group interrupt output using GP0_INTR1_INTR_EN1. 5) For each pin iteration, force the pad data control at 0xA0243ffc low, arm the wait, then drive it high to create one rising edge; wait for the ISR with timeout. 6) In the ISR, confirm the group interrupt status is asserted, then clear all per-pin raw statuses via the per-pin control registers and verify the group status clears to zero. 7) Clear RAW_STCR1 for the GPIO instance and verify it is cleared; re-enable GP0_INTR1_INTR_EN1 and clear the pending interrupt in the controller. 8) Repeat for all iterations and report pass/fail based on accumulated errors.",
    "Meta Test Steps / Procedure": "1) Initialize: test_err = 0. If GPIO0: GIC_EnableIRQ(87); else if GPIO1: GIC_EnableIRQ(88). 2) Unmask SoC interrupt: If GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); if GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR). 3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); // enable posedge detect (bit17). 4) wait_on(10). 5) Configure IO direction to input: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF). 6) wait_on(10). 7) Enable group mask: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 8) For i=0..31: a) write_reg(0xA0243ffc, 0x00000000); wait_on(10); b) int_pend = 1; c) write_reg(0xA0243ffc, 0xFFFFFFFF); d) timeout = 2000; while ((int_pend == 1) && (--timeout > 0)) wait_on(10); e) if (timeout == 0) { printf(\"ERROR: Timeout waiting for GPIO IRQ at i=%u\\n\", i); test_err++; break; } f) write_reg(0xA0243ffc, 0x00000000); wait_on(10). 9) Default_IRQHandler(): a) wr_val = 1 << i; int_pend = 0; b) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); c) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); d) if ((rdata_grp & 0xFFFFFFFF) == 0) { printf(\"ERROR: Group Interrupt not occured\\n\"); test_err++; } e) for j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); // per-pin raw clear (iclr bit16); wait_on(2); f) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { printf(\"ERROR : Group Interrupt clear failed: Interrupt value:%x\\n\", rdata_grp); test_err++; } g) If GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf(\"sysreg status not cleared : %0x\\n\", MIZAR_LSS_SYSREG_RAW_STCR1); test_err++; } h) If GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { printf(\"sysreg status not cleared : %0x\\n\", MIZAR_LSS_SYSREG_RAW_STCR1); test_err++; } i) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); j) If GPIO0: GIC_ClearIRQ(87); if GPIO1: GIC_ClearIRQ(88). 10) finish(test_err).",
    "Impacted Registers": "INTR_EN1; gp0_gpio_8; GPIO_IO_CTRL_GROUP1; GPIO_IO_CTRL_GROUP2; GPIO_IO_CTRL_GROUP3; GPIO_IO_CTRL_GROUP4; GP0_INTR1_INTR_EN1; GP0_INTR1_INTR_STS1; RAW_STCR1",
    "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; LSS_SYSREG_INTR_EN1_GPIO0_INTR; LSS_SYSREG_INTR_EN1_GPIO1_INTR; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GPIO_IO_CTRL_GROUP1; MIZAR_GPIO_GPIO_IO_CTRL_GROUP2; MIZAR_GPIO_GPIO_IO_CTRL_GROUP3; MIZAR_GPIO_GPIO_IO_CTRL_GROUP4; MIZAR_GPIO_GP0_INTR1_INTR_EN1; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1; LSS_SYSREG_RAW_STCR1_GPIO0_INTR; LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
    "Validation / Acceptance Criteria": "PASS if each generated rising edge results in the group interrupt status asserting in GP0_INTR1_INTR_STS1, and after clearing per-pin raw statuses and RAW_STCR1 the status reads 0x0 with no pending SoC status; no timeouts occur. Any timeout, missing group status, or uncleared status results in FAIL.",
    "Meta Validation / Acceptance Criteria": "Conditions for PASS: (a) In Default_IRQHandler, (read GP0_INTR1_INTR_STS1) has at least one bit set when the interrupt fires; (b) after writing 0x00010000 to all gp0_gpio_8+(j*4) and wait_on(2), (read GP0_INTR1_INTR_STS1) == 0x0; (c) after writing instance bit to RAW_STCR1, (read RAW_STCR1 & instance_bit) == 0; (d) main loop does not hit timeout while waiting for int_pend to clear. Any violation increments test_err, yielding FAIL when finish(test_err) != 0.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <lss_sysreg.h>; #include <stdio.h>; #include <test_define.c>; #include <test_common.h>; #include<gpio/gpio_def.h>; #include<gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\n\nconst unsigned int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\n\nconst unsigned int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\n\nconst unsigned int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};"
  }
]'''


def main():
    data = json.loads(JSON_TEXT)
    if not isinstance(data, list):
        raise SystemExit("json_data must be an array of objects")

    # Mandatory bindings (with defaults)
    ip_name = os.environ.get("IP_NAME", "GPIO")
    output_directory = os.environ.get("OUTPUT_DIRECTORY", "Test_Output/GPIO/TestPlan/")
    owner = os.environ.get("OWNER", "titusbspgit")
    repo = os.environ.get("REPO", "PSVValidation")
    branch = os.environ.get("BRANCH", "main")
    source_subdir = os.environ.get("SOURCE_SUBDIR", "TestRepo/gpio")

    ts = ist_now_string()
    file_name = f"{ip_name}_TestPlan_{ts}.xlsx"

    out_dir = Path(output_directory)
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / file_name

    # Define columns for sheets in the specified order
    testplan_cols = [
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

    metadata_cols = [
        "Index",
        "Test Case Name",
        "Meta Test Description",
        "Meta Test Steps / Procedure",
        "Meta Impacted Registers",
        "Meta Validation / Acceptance Criteria",
        "Meta Headers",
        "Meta Macros",
        "Meta Arrays",
    ]

    wb = Workbook()
    ws1 = wb.active
    ws1.title = "TestPlan"
    ws2 = wb.create_sheet("MetaData")

    # Styles
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    align_wrap = Alignment(wrap_text=True, vertical="top")

    # Write headers
    ws1.append(testplan_cols)
    ws2.append(metadata_cols)

    for cell in ws1[1]:
        cell.font = header_font
        cell.fill = header_fill
    for cell in ws2[1]:
        cell.font = header_font
        cell.fill = header_fill

    # Write data preserving order exactly
    def getv(obj, key):
        return obj.get(key, "")

    for obj in data:
        ws1.append([
            getv(obj, "Index"),
            getv(obj, "SS / Module"),
            getv(obj, "Feature"),
            getv(obj, "Test Case Name"),
            getv(obj, "Test Description"),
            getv(obj, "Speed"),
            getv(obj, "Mode"),
            getv(obj, "Memory Start Offset"),
            getv(obj, "Memory End Offset"),
            getv(obj, "Remarks"),
            getv(obj, "Test Steps / Procedure"),
            getv(obj, "Impacted Registers"),
            getv(obj, "Validation / Acceptance Criteria"),
            getv(obj, "Code Generation (Required / Not)"),
        ])
        ws2.append([
            getv(obj, "Index"),
            getv(obj, "Test Case Name"),
            getv(obj, "Meta Test Description"),
            getv(obj, "Meta Test Steps / Procedure"),
            getv(obj, "Meta Impacted Registers"),
            getv(obj, "Meta Validation / Acceptance Criteria"),
            getv(obj, "Meta Headers"),
            getv(obj, "Meta Macros"),
            getv(obj, "Meta Arrays"),
        ])

    # Apply wrap text and vertical align
    for ws in (ws1, ws2):
        for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
            for c in row:
                c.alignment = align_wrap

    # Reasonable column widths
    widths1 = [8, 12, 22, 28, 60, 8, 10, 18, 18, 12, 80, 40, 60, 20]
    for i, w in enumerate(widths1, 1):
        ws1.column_dimensions[get_column_letter(i)].width = w

    widths2 = [8, 28, 60, 80, 40, 60, 40, 30, 40]
    for i, w in enumerate(widths2, 1):
        ws2.column_dimensions[get_column_letter(i)].width = w

    # Freeze first row
    ws1.freeze_panes = "A2"
    ws2.freeze_panes = "A2"

    # Very hidden MetaData sheet
    ws2.sheet_state = "veryHidden"

    # Append exporter metadata as an extra row in MetaData without altering schema
    exporter_meta = {
        "generation_time_IST": ts,
        "IP_NAME": ip_name,
        "source": {
            "owner": owner,
            "repo": repo,
            "branch": branch,
            "subdirectory": source_subdir,
        },
        "target_output_directory": output_directory,
        "record_count": len(data),
    }
    ws2.append(["", "__EXPORTER_METADATA__", json.dumps(exporter_meta), "", "", "", "", "", ""])

    # Save workbook (REAL .xlsx)
    wb.save(str(out_path))

    # Emit path for workflow logging
    print(str(out_path))


if __name__ == "__main__":
    main()
