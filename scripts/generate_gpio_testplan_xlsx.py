#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Deterministic fallback automation to generate a binary .xlsx TestPlan from embedded JSON data
# Follows strict formatting, meta handling, numbering, validation, and visibility rules.

import argparse
import json
import os
import re
import sys
import zipfile
from datetime import datetime, timedelta, timezone
from pathlib import Path

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
    from openpyxl.worksheet.datavalidation import DataValidation
except Exception as e:
    print(f"ERROR: openpyxl import failed: {e}")
    sys.exit(2)

# Embedded JSON-like data as Python structure to preserve exact content
DATA_INPUT = [
    {
        "Index": 1,
        "SS / Module": "GPIO",
        "Feature": "GPIO negative edge interrupt",
        "Test Case Name": "test_gpio_nedge_random_pads_en",
        "Test Description": "Validates negative edge interrupt behavior on randomly selected GPIO pads and verifies group and system interrupt clearing.",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "Requires one GPIO instance define. Uses random pad selection within a fixed range. Relies on an external flag for synchronization. Uses a shared memory location to drive edges.",
        "Test Steps / Procedure": "1) Enable the interrupt controller for the selected instance\n2) Enable the system interrupt for the selected instance\n3) Configure a pad for input with negative edge interrupt\n4) Enable the group interrupt bit for the pad\n5) Drive a transition on the external stimulus source\n6) Wait for the interrupt to be serviced\n7) Read the pad register to check input and raw status\n8) Read the group status to confirm the bit is set\n9) Disable the group interrupt and clear the pad interrupt\n10) Read the pad register to confirm the interrupt is cleared\n11) Read the group status to confirm it is cleared\n12) Clear the system raw status for the selected instance",
        "Impacted Registers": "NA",
        "Validation / Acceptance Criteria": "1) Pad input reflects expected level during the edge → Status is set\n2) Raw interrupt status for the pad is set → Interrupt occurred\n3) Group status bit for the pad is set → Group interrupt occurred\n4) After clearing the pad, the pad status shows cleared value → Interrupt cleared\n5) After clearing, the group status reads zero → Group interrupt cleared\n6) After clearing, the system raw status reads zero → System status cleared",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_nedge_random_pads_en",
        "Hidden_Test_Description": "Enable input mode and negative-edge interrupt on randomly selected GPIO pads (8–39), trigger transitions through writes to 0xA0243ffc, validate pad input state and raw/group interrupt status on interrupt, then clear pad, group, and system interrupt statuses.",
        "Hidden_Remarks": "Requires GPIO0 or GPIO1 to be defined to select IRQ 87 or 88.\nUses address 0xA0243ffc to drive pad transitions.\nPads under test are 8 through 39 of group 0; selection is random and unique.\nUses int_pend as a flag to wait for the interrupt.",
        "Hidden_Test_Steps_Procedure": "#ifdef GPIO0: GIC_EnableIRQ(87); #ifdef GPIO1: GIC_EnableIRQ(88);\n#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);\nsrand(time(NULL)); write_reg(0xA0243ffc, 0xffffffff);\nFor i = 0..31: pick unique pad_num in [0..31]; wr_val = 1 << pad_num; write_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4), 0x00140000); wait_on(50); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 1 << pad_num); wait_on(10); write_reg(0xA0243ffc, ~(wr_val)); wait_on(10); write_reg(0xA0243ffc, 0xffffffff); int_pend = 1; while (int_pend == 0x1) { printf(\"Waiting for interrupt\"); wait_on(10); }\nDefault_IRQHandler: wr_val = 1 << pad_num; int_pend = 0; rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4)); if ((rdata & 0x1) != 0) { /* DIN OK */ } else { printf(\"ERROR: DIN mismatch\"); test_err++; }\nIf ((rdata & 0x2) != 0x0) { rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & (1 << pad_num)) != 0) { /* group intr OK */ } else { printf(\"ERROR: Group Interrupt not occured\"); test_err++; }\nwrite_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); write_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4), 0x00110001); wait_on(2); rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (pad_num * 4)); if (rdata == 0x100001) { /* pad intr cleared */ } else { printf(\"ERROR: Interrupt clear failed : %x\", rdata); test_err++; }\nrdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { /* group intr cleared */ } else { printf(\"ERROR: Group Interrupt clear failed: %x\", rdata_grp); test_err++; }\n#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) == 0) { /* sysreg cleared */ } else { printf(\"sysreg status not cleared\"); test_err++; }\n#ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) == 0) { /* sysreg cleared */ } else { printf(\"sysreg status not cleared\"); test_err++; }\n} else { printf(\"Interrupt Not occured\"); test_err++; }\n#ifdef GPIO0: GIC_ClearIRQ(87); #ifdef GPIO1: GIC_ClearIRQ(88);\nfinish(test_err);",
        "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
        "Hidden_Validation_Acceptance_Criteria": "In Default_IRQHandler: (rdata & 0x1) != 0 indicates DIN value matches expected during negedge; else test_err++.\n(rdata & 0x2) != 0x0 indicates raw interrupt raised; else print \"Interrupt Not occured\" and test_err++.\n(rdata_grp & (1 << pad_num)) != 0 indicates group interrupt raised; else test_err++.\nAfter clearing: read pad reg equals 0x100001 indicates interrupt cleared; else test_err++.\nAfter clearing: group status equals 0x0 indicates group interrupt cleared; else test_err++.\nAfter clearing system raw: reading MIZAR_LSS_SYSREG_RAW_STCR1 with respective mask yields zero for that bit; else test_err++."
    },
    {
        "Index": 2,
        "SS / Module": "GPIO",
        "Feature": "GPIO negative edge interrupt",
        "Test Case Name": "test_gpio_nedge_walking_zeros_pattern",
        "Test Description": "Tests negative-edge interrupt behavior across GPIO pads 8–39 using a walking-zeros stimulus and validates pad, group, and system interrupt handling.",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Uses either GPIO0 or GPIO1 instance interrupts. External source drives transitions via a fixed address. Pads 8–39 are configured for input and negative-edge triggering. Waits on a shared flag set in the handler.",
        "Test Steps / Procedure": "1) Enable the instance interrupt in INTREN1lss.\n2) Configure negative-edge triggering on per-pad registers starting from GPIO_8.\n3) Set input mode using GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, and GPIO_IO_CTRL_GROUP4.\n4) Enable group interrupt bits in INTR1_INTR_EN1.\n5) For each pad 8–39, drive a falling-edge stimulus and wait for the interrupt to occur.\n6) In the handler, read the per-pad register (starting at GPIO_8) and verify input and raw status bits.\n7) Read INTR1_INTR_STS1 to confirm the group status bit is set, then clear the per-pad interrupt and verify it is cleared.\n8) Confirm INTR1_INTR_STS1 is cleared and then clear RAWSTCR1lss for the instance.",
        "Impacted Registers": "INTREN1lss, GPIO_8, INTR1_INTR_EN1, INTR1_INTR_STS1, RAWSTCR1lss, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4",
        "Validation / Acceptance Criteria": "1) Per-pad input bit set during a falling edge capture → Input state reflects the expected level.\n2) Per-pad raw status bit set on a falling edge → Interrupt occurred for the pad.\n3) Group status bit set for the active pad in INTR1_INTR_STS1 → Group interrupt occurred.\n4) After writing the clear sequence to the per-pad register → Readback equals the cleared value.\n5) After clearing, INTR1_INTR_STS1 reads zero → Group interrupt cleared.\n6) After clearing RAWSTCR1lss for the instance → The corresponding system raw status bit reads zero.",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_nedge_walking_zeros_pattern",
        "Hidden_Test_Description": "Validates GPIO negative-edge interrupts on pads 8–39 using a walking-zeros pattern generated by writes to 0xA0243ffc. Configures input mode and negative-edge triggering, enables group interrupts, then for each pad generates a falling edge, waits for the interrupt, checks pad input/raw bits, verifies group status, clears pad interrupt, confirms group clear, and clears system raw status.",
        "Hidden_Remarks": "#ifdef GPIO0 uses IRQ 87; #ifdef GPIO1 uses IRQ 88. External stimulus at address 0xA0243ffc creates walking-zeros pattern. Pads 8–39 are configured via per-pad registers and GPIO_IO_CTRL_GROUP1..4 set to 0x000000FF. Group interrupt enable is set to 0xFFFFFFFF. Synchronization via int_pend flag.",
        "Hidden_Test_Steps_Procedure": "#ifdef GPIO0: GIC_EnableIRQ(87); #ifdef GPIO1: GIC_EnableIRQ(88);\nSet test_err = 0;\n#ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);\n#ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);\nFor i = 0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00040000); // enable negedge (bit 17 = 1)\nwait_on(10);\nwrite_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF);\nwrite_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF);\nwrite_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF);\nwrite_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);\nwait_on(10);\nwrite_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);\nFor i = 0..31:\n wr_val = 1 << i;\n write_reg(0xA0243ffc, 0xFFFFFFFF);\n wait_on(30);\n write_reg(0xA0243ffc, ~(wr_val));\n wait_on(30);\n int_pend = 1;\n while (int_pend == 1) { printf(\"Waiting for interrupt\"); wait_on(10); }\nfinish(test_err);\n\nDefault_IRQHandler:\nwr_val = 1 << i;\nint_pend = 0;\nwrite_reg(0xA0243ffc, 0xFFFFFFFF);\nrdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4));\nif ((rdata & 0x1) != 0) { /* DIN OK */ } else { printf(\"ERROR: DIN mismatch read_data = %0x\", rdata); test_err++; }\nif ((rdata & 0x2) != 0x0) {\n rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n if ((rdata_grp & (1 << i)) != 0) { /* group intr OK */ } else { printf(\"ERROR: Group Interrupt not occured\"); test_err++; }\n write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00110001);\n wait_on(2);\n rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4));\n if (rdata == 0x100001) { /* pad intr cleared */ } else { printf(\"ERROR: Interrupt clear failed : %x\", rdata); test_err++; }\n rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);\n if (rdata_grp == 0x0) { /* group intr cleared */ } else { printf(\"ERROR: Group Interrupt clear failed: %x\", rdata_grp); test_err++; }\n #ifdef GPIO0\n write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);\n rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);\n if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) == 0) { /* sysreg cleared */ } else { printf(\"sysreg status not cleared\"); test_err++; }\n #endif\n #ifdef GPIO1\n write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);\n rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);\n if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) == 0) { /* sysreg cleared */ } else { printf(\"sysreg status not cleared\"); test_err++; }\n #endif\n} else {\n printf(\"Interrupt Not occured\");\n test_err++;\n}\n#ifdef GPIO0: GIC_ClearIRQ(87); #ifdef GPIO1: GIC_ClearIRQ(88);",
        "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
        "Hidden_Validation_Acceptance_Criteria": "1) If (rdata & 0x1) != 0 → DIN value matches during negedge; else increment test_err.\n2) If (rdata & 0x2) != 0x0 → Raw interrupt raised; else print \"Interrupt Not occured\" and increment test_err.\n3) If (rdata_grp & (1 << i)) != 0 → Group interrupt raised; else increment test_err.\n4) After write_reg(..., 0x00110001) and readback → rdata == 0x100001 indicates pad interrupt cleared; else increment test_err.\n5) rdata_grp == 0x0 after clearing → Group interrupt cleared; else increment test_err.\n6) After write to MIZAR_LSS_SYSREG_RAW_STCR1 with respective mask and readback → masked bit reads zero; else increment test_err."
    },
    {
        "Index": 3,
        "SS / Module": "GPIO",
        "Feature": "Interrupts based on negative edge detection at GPIO input",
        "Test Case Name": "test_gpio_negedge_intr_en",
        "Test Description": "Enable input mode and negative-edge interrupts on GPIO pads 8–39, trigger falling edges via a memory-mapped stimulus, handle the interrupt, check pad input and raw/group status, then clear pad and system status.",
        "Speed": "NA",
        "Mode": "ISR",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "#ifdef GPIO0 uses GIC IRQ 87 and clears RAW_STCR1 with the GPIO0 bit; #ifdef GPIO1 uses GIC IRQ 88 and clears RAW_STCR1 with the GPIO1 bit. Uses 0xA0243ffc to generate stimulus. Pads 8–39 are configured via the per‑pad register base. int_pend flag synchronizes main loop and ISR.",
        "Test Steps / Procedure": "Initialization:\n- Set test_err = 0.\n- If GPIO0: call GIC_EnableIRQ(87).\n- If GPIO1: call GIC_EnableIRQ(88).\n- If GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR).\n- If GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n- write_reg(0xA0243ffc, 0xffffffff).\n\nMain loop for i = 0..31 (pads 8–39):\n- addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n- write_reg(addr1, 0x00140000). // configure input + negedge\n- wait_on(50).\n- wr_val = (1 << i).\n- write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). // enable group interrupt bit\n- wait_on(10).\n- write_reg(0xA0243ffc, 0xffffffff).\n- wait_on(30).\n- write_reg(0xA0243ffc, ~(wr_val)). // generate falling edge on selected pad\n- int_pend = 1.\n- while (int_pend) { wait_on(10); }\n\nAfter loop:\n- finish(test_err).\n\nDefault_IRQHandler:\n- int_pend = 0.\n- write_reg(0xA0243ffc, 0xffffffff). // restore stimulus\n- raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n- rdata = read_reg(raddr).\n- If ((rdata & 0x1) == 0x0): test_err++.\n- If ((rdata & 0x2) != 0x0):\n - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n - If ((rdata_grp & wr_val) == 0): test_err++.\n - raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n - write_reg(raddr2, 0x00110001). // clear per‑pad interrupt\n - rdata = read_reg(raddr2).\n - If (rdata != 0x100001): test_err++.\n - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n - If (rdata_grp != 0x0): test_err++.\n - If GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87).\n - If GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88).\n- Else: // raw status not set\n - test_err++.",
        "Impacted Registers": "INTR_EN1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1, gp0_intr2_intr_en1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
        "Validation / Acceptance Criteria": "1) If per‑pad input bit is set when the interrupt fires → Input state is valid; else increment error counter.\n2) If per‑pad raw status bit is set → Interrupt occurred; else increment error counter.\n3) If group status bit corresponding to the active pad is set → Group interrupt occurred; else increment error counter.\n4) After writing the per‑pad clear value, readback equals the cleared value (0x100001) → Per‑pad interrupt cleared; else increment error counter.\n5) After clearing, group status register reads 0x0 → Group interrupt cleared; else increment error counter.",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
        "Hidden_Test_Description": "Enable input mode and negative-edge interrupts for GPIOs 8–39; for each pad enable its group interrupt bit, drive a falling edge using writes to 0xA0243ffc, wait on int_pend for ISR, read per-pad register to verify DIN and raw status, read group status to confirm the bit is set, clear per-pad interrupt via 0x00110001 and verify readback equals 0x100001, confirm group status is 0x0, and clear system raw status and IRQ for the instance.",
        "Hidden_Remarks": "#ifdef GPIO0 uses IRQ 87 and RAW_STCR1 GPIO0 mask; #ifdef GPIO1 uses IRQ 88 and RAW_STCR1 GPIO1 mask. Uses 0xA0243ffc as external stimulus. Pads 8–39 accessed via MIZAR_GPIO_GP0_GPIO_8 + (i4). Synchronization via int_pend flag.",
        "Hidden_Test_Steps_Procedure": "test_err = 0; If GPIO0: GIC_EnableIRQ(87); If GPIO1: GIC_EnableIRQ(88); If GPIO0: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); If GPIO1: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR); write_reg(0xA0243ffc, 0xffffffff); For i = 0..31: addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i4); write_reg(addr1, 0x00140000); wait_on(50); wr_val = 1 << i; write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~(wr_val)); int_pend = 1; while (int_pend) { wait_on(10); } finish(test_err); Default_IRQHandler: int_pend = 0; write_reg(0xA0243ffc, 0xffffffff); raddr = MIZAR_GPIO_GP0_GPIO_8 + (i4); rdata = read_reg(raddr); if ((rdata & 0x1) == 0x0) { test_err++; } if ((rdata & 0x2) != 0x0) { rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & wr_val) == 0) { test_err++; } raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i4); write_reg(raddr2, 0x00110001); rdata = read_reg(raddr2); if (rdata != 0x100001) { test_err++; } rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { test_err++; } ifdef GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); ifdef GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); } else { test_err++; }",
        "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR",
        "Hidden_Validation_Acceptance_Criteria": "If ((rdata & 0x1) != 0x0) then input state is valid; else test_err++. If ((rdata & 0x2) != 0x0) then raw interrupt occurred; else test_err++. If ((rdata_grp & wr_val) != 0) then group interrupt occurred; else test_err++. After write_reg(per‑pad, 0x00110001) then readback rdata == 0x100001; else test_err++. After clearing, rdata_grp == 0x0; else test_err++."
    }
]

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

WRAP_COLUMNS = set([
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
])

HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
HEADER_FONT = Font(bold=True, color="FFFFFF")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=False)
DATA_ALIGN_TEXT = Alignment(horizontal="left", vertical="top", wrap_text=True)
DATA_ALIGN_NUM = Alignment(horizontal="center", vertical="top", wrap_text=True)
THIN_BORDER = Border(
    left=Side(style="thin", color="000000"),
    right=Side(style="thin", color="000000"),
    top=Side(style="thin", color="000000"),
    bottom=Side(style="thin", color="000000"),
)


def build_schema(rows):
    seen = []
    for obj in rows:
        for k in obj.keys():
            if k not in seen:
                seen.append(k)
    return seen


def normalize_rows(rows, schema):
    norm = []
    for obj in rows:
        norm.append({k: obj.get(k, "") for k in schema})
    return norm


def approximate_column_width(values):
    max_len = 0
    for v in values:
        s = "" if v is None else str(v)
        for line in s.split("\n"):
            max_len = max(max_len, len(line))
    # Excel width heuristic: cap to 100
    return min(max(10, max_len + 2), 100)


def estimate_row_height(cell_text, col_width):
    if not cell_text:
        return None
    # Estimate chars per line from column width (roughly 1 width unit ~ 1 char)
    try:
        cw = float(col_width)
    except Exception:
        cw = 10.0
    chars_per_line = max(1.0, cw - 1.0)
    lines = 0
    for para in str(cell_text).split("\n"):
        ln = para.strip()
        if not ln:
            lines += 1
            continue
        lines += max(1, int((len(ln) + chars_per_line - 1) // chars_per_line))
    base_height = 15.0
    return min(409.0, max(base_height, lines * 15.0))


def number_items(text):
    if text is None:
        return ""
    s = str(text)
    parts = [p for p in re.split(r"\r?\n+", s) if p.strip()]
    out = []
    for idx, p in enumerate(parts, 1):
        # Remove leading numbering or bullets
        p2 = re.sub(r"^\s*(?:\d+[\.)]\s*|[-•]\s*)", "", p).strip()
        out.append(f"{idx}. {p2}")
    return "\n".join(out) if out else s


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = THIN_BORDER


def make_meta_sheet(wb, norm_rows):
    ws = wb.create_sheet("Meta_data_sheet")
    # Header
    for c, key in enumerate(META_COLUMNS, 1):
        ws.cell(row=1, column=c, value=key)
    # Data
    for r, row_obj in enumerate(norm_rows, 2):
        for c, key in enumerate(META_COLUMNS, 1):
            ws.cell(row=r, column=c, value=row_obj.get(key, ""))
    # Basic header style
    for c in range(1, len(META_COLUMNS) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGN
    # Widths
    for c, key in enumerate(META_COLUMNS, 1):
        values = [key] + [row_obj.get(key, "") for row_obj in norm_rows]
        ws.column_dimensions[get_column_letter(c)].width = approximate_column_width(values)
    # Hide sheet (Very Hidden)
    ws.sheet_state = 'veryHidden'
    return ws


def main():
    parser = argparse.ArgumentParser(description="Generate formatted GPIO TestPlan XLSX from embedded JSON data")
    parser.add_argument("--ip-name", required=True, help="IP name for filename prefix")
    parser.add_argument("--output-dir", required=True, help="Repository-relative output directory for the Excel file")
    args = parser.parse_args()

    rows = DATA_INPUT
    if not isinstance(rows, list) or len(rows) == 0:
        print("ERROR: Input JSON array is empty or invalid", file=sys.stderr)
        sys.exit(3)

    # Build schema preserving first-seen order
    schema = build_schema(rows)
    norm_rows = normalize_rows(rows, schema)

    # Create workbook and 'Data' sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write headers
    for c, key in enumerate(schema, 1):
        cell = ws.cell(row=1, column=c, value=key)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGN

    # Write data rows
    for r, row_obj in enumerate(norm_rows, 2):
        for c, key in enumerate(schema, 1):
            ws.cell(row=r, column=c, value=row_obj.get(key, ""))

    # Freeze top row
    ws.freeze_panes = "A2"

    # Create META sheet and populate
    make_meta_sheet(wb, norm_rows)

    # Now normalize main sheet in-place: rename Data -> TestPlan, reorder/remove columns
    ws.title = "TestPlan"

    # Prepare visible columns order
    # Remove META columns from TestPlan; append any extra non-META columns after MAIN in original order
    extra_columns = [k for k in schema if k not in MAIN_COLUMNS and k not in META_COLUMNS]
    final_order = MAIN_COLUMNS + extra_columns

    # Rebuild TestPlan columns in final_order using current data
    # Create a mapping from current header to column index
    header_to_col = { ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1) }

    # Create a new row buffer according to final_order
    # First, rewrite header row
    for c, key in enumerate(final_order, 1):
        ws.cell(row=1, column=c, value=key)
    # Clear any remaining header cells beyond new max
    for c in range(len(final_order) + 1, ws.max_column + 1):
        ws.cell(row=1, column=c, value=None)

    # Rewrite data rows
    for r in range(2, ws.max_row + 1):
        for c, key in enumerate(final_order, 1):
            src_col = header_to_col.get(key)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            ws.cell(row=r, column=c, value=val)
        # Clear remaining cells
        for c in range(len(final_order) + 1, ws.max_column + 1):
            ws.cell(row=r, column=c, value=None)

    # Adjust max_column logically to final_order
    # Note: openpyxl determines max_column dynamically; leaving trailing Nones won't persist.

    # Apply wrapping and alignment for data rows
    col_index = { key: idx + 1 for idx, key in enumerate(final_order) }

    # Numbering inside specific columns (TestPlan sheet only)
    for target_key in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
        if target_key in col_index:
            cidx = col_index[target_key]
            for r in range(2, ws.max_row + 1):
                v = ws.cell(row=r, column=cidx).value
                ws.cell(row=r, column=cidx, value=number_items(v))

    # Auto-fit widths (approx), set wrap for specified columns
    col_widths = {}
    for key, cidx in col_index.items():
        values = [key]
        for r in range(2, ws.max_row + 1):
            values.append(ws.cell(row=r, column=cidx).value)
        width = approximate_column_width(values)
        ws.column_dimensions[get_column_letter(cidx)].width = width
        col_widths[cidx] = width

    # Header styling already applied; ensure header alignment
    for c in range(1, len(final_order) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGN

    # Data alignments and wrap
    for r in range(2, ws.max_row + 1):
        for key, cidx in col_index.items():
            cell = ws.cell(row=r, column=cidx)
            if key in WRAP_COLUMNS:
                cell.alignment = DATA_ALIGN_TEXT
            elif key == "Index":
                cell.alignment = DATA_ALIGN_NUM
            else:
                # Heuristic: treat numbers as numeric alignment center
                try:
                    float(str(cell.value))
                    cell.alignment = DATA_ALIGN_NUM
                except Exception:
                    cell.alignment = DATA_ALIGN_TEXT

    # Estimate row heights after wrapping
    for r in range(2, ws.max_row + 1):
        # Use the max estimated height among wrapped columns
        heights = []
        for key in WRAP_COLUMNS:
            if key in col_index:
                cidx = col_index[key]
                txt = ws.cell(row=r, column=cidx).value
                cw = col_widths.get(cidx, 10)
                h = estimate_row_height(txt, cw)
                if h:
                    heights.append(h)
        if heights:
            ws.row_dimensions[r].height = max(heights)

    # Borders for all populated cells
    apply_borders(ws)

    # Data Validation for 'Code Generation (Required / Not)' only
    if "Code Generation (Required / Not)" in col_index:
        cidx = col_index["Code Generation (Required / Not)"]
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=False, showErrorMessage=True)
        rng = f"{get_column_letter(cidx)}2:{get_column_letter(cidx)}{ws.max_row}"
        dv.add(rng)
        ws.add_data_validation(dv)

    # Safety: ensure no sheet named 'Data' remains
    if 'Data' in wb.sheetnames:
        # If a separate 'Data' exists (shouldn't), delete it
        if wb['Data'] != ws:
            wb.remove(wb['Data'])

    # Final allowed sheets: 'TestPlan' (visible) and 'Meta_data_sheet' (veryHidden)
    for name in list(wb.sheetnames):
        if name not in ('TestPlan', 'Meta_data_sheet'):
            ws_tmp = wb[name]
            if ws_tmp != ws and name != 'Meta_data_sheet':
                wb.remove(ws_tmp)

    # Validate as real XLSX by saving to a temp and reloading
    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    # Compute IST timestamp for filename
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(tz=ist)
    date_str = now_ist.strftime('%Y%m%d')
    time_str = now_ist.strftime('%H%M%S')
    filename = f"{args.ip_name}_TestPlan_{date_str}_{time_str}.xlsx"
    out_path = out_dir / filename

    # Save
    wb.save(str(out_path))

    # Validation
    ok = True
    try:
        if not zipfile.is_zipfile(str(out_path)):
            print("ERROR: Generated file is not a valid ZIP/XLSX")
            ok = False
        else:
            with zipfile.ZipFile(str(out_path), 'r') as zf:
                if '[Content_Types].xml' not in zf.namelist():
                    print("ERROR: Missing [Content_Types].xml in XLSX")
                    ok = False
        # Try re-open
        load_workbook(str(out_path))
    except Exception as e:
        print(f"ERROR: XLSX validation failed: {e}")
        ok = False

    if not ok:
        print("ERROR: XLSX generation/validation failed.")
        sys.exit(4)

    print(f"SUCCESS: Generated {out_path}")

if __name__ == '__main__':
    main()
