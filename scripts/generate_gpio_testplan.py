#!/usr/bin/env python3
# Generates GPIO TestPlan Excel (.xlsx) from embedded JSON according to Stage1 rules
import os
import json
from datetime import datetime, timezone, timedelta
from copy import deepcopy
from zipfile import ZipFile
from io import BytesIO
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# Embedded consolidated Test Plan JSON as an array of 3 objects (rows)
json_data = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validates default register values and masked write/read behavior across GPIO registers 8–39 and related group/interrupt registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Some addresses are explicitly skipped for default-value checks and/or write/read checks per skip arrays. Reading default values for input data may show nonzero unless the input is forced; forcing input affects bit-level selection causing mismatches.",
    "Test Steps / Procedure": "Entry and initialization: begin test routine. Default value verification: iterate across the defined address list covering GPIO_8 through GPIO_39 and group/interrupt registers. For each address that is not marked to be skipped for reset checks and is readable, read the register value, apply the mask to ignore the least significant bit, and compare it against the expected default value from the table; record any mismatch. Masked write/read verification: for each of the predefined data patterns, iterate across the same address list. For each address that is not marked to be skipped for write/read and is writable, write the masked data. Then, for each address that is not skipped and is both writable and readable, read back the masked data, compute the expected value by combining writeable bits from the written pattern with non‑writeable bits taken from the default value, and compare; record any mismatch. Completion: declare pass when no mismatches were recorded in default checks and write/read verification; otherwise declare failure.",
    "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1, INTR2_INTR_EN1, INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
    "Validation / Acceptance Criteria": "Pass criteria: no default value mismatches and no write/read mismatches are recorded across the addressed registers. Fail criteria: any default value mismatch or any write/read mismatch is recorded.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Validate default values and masked write/read across an address list including per‑pin GPIO (8–39) and group/interrupt registers.",
    "Hidden_Remarks": "Registers are selectively skipped per skip_array and skip_rst_array in test_define.c. Comment notes: when reading default values, DIN becomes 1 automatically if no forcing; forcing 0 causes level select to become high leading to mismatches.",
    "Hidden_Test_Steps_Procedure": "A. Entry point(s)\n- test_case()\n\nB. Runtime ordered trace\n1) test_case: invoke chk_rst_val()\n2) chk_rst_val: initialize i=0; loop entry: for (i=0; i<CNT; i++)\n   2.1) addr = addr_array[i]\n   2.2) if (skip_rst_array[i] == 1) continue\n   2.3) if (read_mask_array[i] == 0x00000000) continue\n   2.4) READ: data_rd = read_reg(addr)\n   2.5) COMPUTE: data = (data_rd & 0xfffffffe)\n   2.6) COMPARE: if (data == default_value_array[i]) PASS else { def_fail_cnt++; printf failure with addr, expected, read }\n   2.7) Loop exit condition: i increments until i==CNT\n3) test_case: invoke chk_rd_wr()\n4) chk_rd_wr: define patterns chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}\n   Outer loop entry: for (j=0; j<6; j++)\n   4.1) data_wr = chk_val[j]\n   4.2) Inner write loop entry: for (i=0; i<CNT; i++)\n       4.2.1) addr = addr_array[i]\n       4.2.2) if (skip_array[i] == 1) continue\n       4.2.3) if (write_mask_array[i] == 0x00000000) continue\n       4.2.4) WRITE: write_reg(addr, (data_wr & write_mask_array[i]))\n       4.2.5) Loop exit condition: i increments until i==CNT\n   4.3) Inner read/verify loop entry: for (i=0; i<CNT; i++)\n       4.3.1) addr = addr_array[i]\n       4.3.2) if (skip_array[i] == 1) continue\n       4.3.3) if (write_mask_array[i] == 0x00000000) continue\n       4.3.4) if (read_mask_array[i] == 0x00000000) continue\n       4.3.5) READ: data_rd = (read_reg(addr) & read_mask_array[i])\n       4.3.6) COMPUTE: wr_n = (write_mask_array[i] ^ 0xffffffff)\n       4.3.7) COMPUTE: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]))\n       4.3.8) COMPARE: if (data_rd == exp_val) PASS else { wr_fail_cnt++; printf mismatch with addr, exp_val, data_rd }\n       4.3.9) Loop exit condition: i increments until i==CNT\n   4.4) Outer loop exit condition: j increments until j==6\n5) test_case: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0)\n\nC. Timing\n- No explicit wait_on/delay in executed paths.\n\nD. Register accesses (macro, operation, mask/bit usage)\n- addr_array[i] elements include: MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, MIZAR_GPIO_GPIO_DOUT_GROUP1..4, MIZAR_GPIO_GPIO_DIN_GROUP1..4\n- READ: read_reg(addr) masked by read_mask_array[i]\n- WRITE: write_reg(addr, (data_wr & write_mask_array[i]))\n- COMPUTE expected using write_mask_array and default_value_array\n\nE. Loop structure\n- For i-loops: entry at i=0, exit at i==CNT\n- For j-loops: entry at j=0, exit at j==6",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Pass if (def_fail_cnt == 0) AND (wr_fail_cnt == 0). Fail if (def_fail_cnt > 0) OR (wr_fail_cnt > 0)."
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "Negative edge interrupt enable (neie)",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies that enabling negative-edge interrupts on GPIOs 8–39 generates an interrupt per pin when a falling edge occurs, and that per-pin and group status can be cleared.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "A bounded wait with timeout is used while waiting for the interrupt to avoid infinite stalls. The wait is armed before generating the falling edge to avoid races.",
    "Test Steps / Procedure": "Entry and setup: enable the interrupt controller for the selected GPIO instance and enable the corresponding system interrupt output. Initialize the pad driver to a known state (all outputs driven high). Configure GPIO_8 through GPIO_39 as inputs, enable negative‑edge detection, and clear any pending raw status for each pin. For each pin, clear the corresponding raw status bit at the group level, enable only that pin’s group interrupt enable bit, arm the wait flag, and generate a falling edge on that pin by toggling the external drive from high to low. Poll the wait flag with a timeout and record an error on timeout. Interrupt service: on interrupt, restore the external drive to high, read the per‑pin register and ensure the input bit is low after the falling edge, confirm that the per‑pin raw status indicates an event and that the group masked status reflects the same pin, clear the per‑pin raw status and the group raw bit, and verify that the group status is cleared. Finally, clear the system interrupt status and the interrupt controller pending state. Completion: pass if no timeouts or validation errors are recorded; otherwise fail.",
    "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1",
    "Validation / Acceptance Criteria": "Per interrupt: after a falling edge on a pin, the corresponding input reads low, the per‑pin raw status indicates an event, the group masked status bit for that pin is set, and after clearing actions the group status reads zero. No timeout occurs while waiting for the interrupt. The overall test passes when no errors are recorded.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Enable negedge interrupt per pin (GPIOs 8..39), drive falling edge per pin, ISR verifies DIN=0, per-pin raw set and group masked set, then clears both and checks cleared; also clears system interrupt and GIC pending.",
    "Hidden_Remarks": "Use bounded wait (timeout=5000) while polling int_pend. Arm int_pend before generating the edge to avoid races.",
    "Hidden_Test_Steps_Procedure": "A. Entry point(s)\n- test_case()\n- Default_IRQHandler() (interrupt-driven)\n\nB. Runtime ordered trace\n1) test_case: test_err=0\n2) Conditionally enable GIC interrupt (GPIO0->87, GPIO1->88)\n3) Enable system interrupt output: WRITE: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR)\n4) Initialize external drive: WRITE: write_reg(0xA0243ffc, 0xffffffff)\n5) Configure per-pin for negedge + input + clear raw: loop i=0..31\n   5.1) addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i*4)\n   5.2) WRITE: write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) // doe=1 (input), neie=1, iclr=1\n   5.3) wait_on(10)\n6) For each pin i=0..31\n   6.1) wr_val = (1<<i)\n   6.2) WRITE: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) // pre-clear group raw\n   6.3) WRITE: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val)   // enable only this bit\n   6.4) wait_on(10)\n   6.5) int_pend = 1 // arm before edge\n   6.6) Generate negedge: WRITE: write_reg(0xA0243ffc, 0xffffffff); wait_on(30); WRITE: write_reg(0xA0243ffc, ~wr_val)\n   6.7) Bounded wait: timeout=5000; while(int_pend && timeout--) wait_on(10)\n   6.8) if (timeout==0) { printf timeout error; test_err++ }\n7) finish(test_err)\n\nInterrupt handler (Default_IRQHandler):\n8) Compute local_wr=(1<<i); int_pend=0\n9) Restore external drive: WRITE: write_reg(0xA0243ffc, 0xffffffff)\n10) READ per-pin: raddr=MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata=read_reg(raddr)\n11) Check DIN low: if ((rdata & 0x1)!=0) test_err++\n12) If raw set: if ((rdata & 0x2)!=0x0) then\n    12.1) READ group masked: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n    12.2) Check bit for pin: if ((rdata_grp & local_wr)==0) test_err++\n    12.3) Clear per-pin raw: WRITE: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), (1<<20)|(1<<16)) // doe=1, iclr=1\n    12.4) Clear group raw: WRITE: write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr)\n    12.5) Verify group clear: READ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++\n    12.6) Clear system raw + GIC: if GPIO0 { WRITE: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87) } else if GPIO1 { WRITE: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88) }\n    12.7) else branch (raw not set): test_err++\n\nC. Timing\n- wait_on(10) in configuration loop; wait_on(30) before falling edge; bounded wait loop with timeout=5000 and wait_on(10) per iteration\n\nD. Register accesses\n- Per-pin registers: MIZAR_GPIO_GP0_GPIO_8 + (i*4) [WRITE doe/neie/iclr; READ status]\n- Group raw status clear: MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 [WRITE W1C mask]\n- Group interrupt enable: MIZAR_GPIO_GP0_INTR1_INTR_EN1 [WRITE mask]\n- Group masked status: MIZAR_GPIO_GP0_INTR1_INTR_STS1 [READ]\n- System interrupt enable/clear: MIZAR_LSS_SYSREG_INTR_EN1 [WRITE], MIZAR_LSS_SYSREG_RAW_STCR1 [WRITE]\n- External drive control at 0xA0243ffc [WRITE]\n\nE. Loop structure\n- For i=0..31 in configuration and edge generation; bounded while loop with decrementing timeout",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin i in 8..39: (1) no timeout in bounded wait; (2) after falling edge, per-pin DIN bit reads 0; (3) per-pin raw bit set; (4) group masked status has bit i set; (5) after clearing per-pin raw and group raw, group masked status reads 0. System raw status is cleared in system register; GIC pending cleared. Overall pass if test_err==0; fail otherwise."
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Positive edge interrupt enable (peie)",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Verifies that enabling positive-edge interrupts on GPIOs 8–39 generates an interrupt per pin when a rising edge occurs, and that per-pin raw status and group masked status are cleared correctly.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "A bounded wait with timeout is used to avoid infinite hangs while waiting for the interrupt; the wait is armed before asserting the rising edge.",
    "Test Steps / Procedure": "Entry and setup: enable the interrupt controller for the selected GPIO instance and enable the corresponding system interrupt output. Configure all per‑pin registers for positive‑edge detection. Set the group I/O control for GPIOs 8–39 to input mode. Enable all bits in the group interrupt enable register. For each pin in sequence, drive the external value low, arm the wait flag, and then drive high to produce a single rising edge; poll the wait flag with a timeout and record an error on timeout; then drive low again for the next iteration. Interrupt service: on interrupt, read the group masked status; mask further group interrupts during service; confirm that at least one bit is set indicating a group event; clear per‑pin raw status for all pins; verify that the group masked status clears to zero; clear the system raw status for the selected instance and verify it clears; re‑enable the group interrupt and clear the interrupt controller pending state. Completion: pass if no timeouts or validation errors are recorded; otherwise fail.",
    "Impacted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, INTR1_INTR_EN1, INTR1_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4",
    "Validation / Acceptance Criteria": "During service, the group masked status indicates an event when a rising edge occurs, and after clearing per‑pin raw status for all pins the group masked status reads zero. The system raw status is cleared and reads as cleared. Each iteration completes without timeout. The overall test passes when no errors are recorded.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Enable posedge interrupt for GPIOs 8..39, configure input mode via group IO control, enable all group interrupt bits, generate a rising edge per iteration via external drive, wait for ISR, ISR validates group status, clears per‑pin raws, ensures group clears, clears system raw and re‑enables group.",
    "Hidden_Remarks": "Bounded wait uses timeout=2000 with wait_on(10) per iteration; int_pend is armed before the rising edge.",
    "Hidden_Test_Steps_Procedure": "A. Entry point(s)\n- test_case()\n- Default_IRQHandler() (interrupt-driven)\n\nB. Runtime ordered trace\n1) test_case: Conditionally enable GIC interrupt (GPIO0->87, GPIO1->88)\n2) test_err=0; Declare locals\n3) Enable system interrupt output: WRITE: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR)\n4) Configure posedge per pin: loop i=0..31\n   4.1) WRITE: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) // peie=1\n5) wait_on(10)\n6) Configure group IO control (input mode for GPIOs 8..39):\n   6.1) WRITE: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF)\n   6.2) WRITE: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF)\n   6.3) WRITE: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF)\n   6.4) WRITE: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF)\n7) wait_on(10)\n8) Enable group interrupt for 32 pins: WRITE: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF)\n9) For each pin i=0..31\n   9.1) Prepare low: WRITE: write_reg(0xA0243ffc, 0x00000000); wait_on(10)\n   9.2) Arm wait: int_pend=1\n   9.3) Generate posedge: WRITE: write_reg(0xA0243ffc, 0xFFFFFFFF)\n   9.4) Bounded wait: timeout=2000; while(int_pend==1 && --timeout>0) wait_on(10)\n   9.5) if (timeout==0) { printf timeout; test_err++; break; }\n   9.6) Drive low again: WRITE: write_reg(0xA0243ffc, 0x00000000); wait_on(10)\n10) finish(test_err)\n\nInterrupt handler (Default_IRQHandler):\n11) wr_val = (1<<i); int_pend=0\n12) READ group masked: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n13) Mask during service: WRITE: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000)\n14) Check group event: if ((rdata_grp & 0xffffffff) != 0) PASS else { printf error; test_err++; }\n15) Clear per-pin raw for all 32 pins: for (j=0; j<32; j++) WRITE: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) // iclr=1\n16) wait_on(2)\n17) Verify group clear: READ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) PASS else { printf error; test_err++; }\n18) Clear system raw + verify:\n    18.1) If GPIO0: WRITE: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); READ: rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf not cleared; test_err++; }\n    18.2) If GPIO1: WRITE: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); READ: rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) { printf not cleared; test_err++; }\n19) Re-enable group interrupt: WRITE: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF)\n20) Clear GIC pending: GIC_ClearIRQ(87 or 88)\n\nC. Timing\n- wait_on(10) between config stages; bounded wait loop with timeout=2000 and wait_on(10); wait_on(2) after per-pin raw clears\n\nD. Register accesses\n- Per-pin registers: MIZAR_GPIO_GP0_GPIO_8 + (i*4) [WRITE peie; later WRITE iclr]\n- Group IO control: MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4 [WRITE]\n- Group interrupt enable: MIZAR_GPIO_GP0_INTR1_INTR_EN1 [WRITE]\n- Group masked status: MIZAR_GPIO_GP0_INTR1_INTR_STS1 [READ]\n- System interrupt enable/clear and readback: MIZAR_LSS_SYSREG_INTR_EN1 [WRITE], MIZAR_LSS_SYSREG_RAW_STCR1 [WRITE/READ]\n- External drive control at 0xA0243ffc [WRITE]\n\nE. Loop structure\n- For i=0..31 in configuration and per-edge generation; bounded while loop with timeout; for j=0..31 clearing raws",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "Group masked status indicates an event during ISR; after clearing per‑pin raws for all 32 pins, group masked status reads 0; system raw status clears (readback shows bit cleared). No timeouts occur in the bounded wait. Overall pass if test_err==0; otherwise fail."
  }
]

META_COLS = [
  "Hidden_Test_Case_Name",
  "Hidden_Test_Description",
  "Hidden_Remarks",
  "Hidden_Test_Steps_Procedure",
  "Hidden_Impacted_Registers",
  "Hidden_Validation_Acceptance_Criteria"
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
  "Code Generation (Required / Not)"
]

ALLOWED_CODEGEN = ["Required", "Blank", "Not Required"]

# Utilities

def now_ist():
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist)

def ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def normalize_schema(rows):
    if not isinstance(rows, list) or len(rows) == 0:
        raise ValueError("JSON array is empty or invalid")
    # Collect union of keys preserving first-seen order
    ordered_keys = []
    seen = set()
    for row in rows:
        if not isinstance(row, dict):
            raise ValueError("Each array element must be an object")
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                ordered_keys.append(k)
    # Fill missing keys with blanks
    norm_rows = []
    for row in rows:
        norm_row = {}
        for k in ordered_keys:
            norm_row[k] = row.get(k, "")
        norm_rows.append(norm_row)
    return ordered_keys, norm_rows


def estimate_col_width(value):
    s = str(value) if value is not None else ""
    length = max(len(s), 3)
    return min(length + 2, 80)


def apply_borders(ws, max_row, max_col):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            cell.border = border


def number_items(text):
    if text is None:
        return ""
    s = str(text).strip()
    if not s:
        return ""
    parts = []
    for rawline in s.replace('\r\n', '\n').replace('\r', '\n').split('\n'):
        subs = [p.strip() for p in rawline.split(';') if p.strip()]
        if subs:
            parts.extend(subs)
    if not parts:
        parts = [s]
    return "\n".join(f"{i+1}. {p}" for i, p in enumerate(parts))


def build_workbook(rows):
    headers, norm_rows = normalize_schema(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = 'Data'  # authoritative staging sheet

    # Write headers
    for ci, key in enumerate(headers, start=1):
        cell = ws.cell(row=1, column=ci, value=key)
        cell.font = Font(bold=True)
    ws.freeze_panes = 'A2'

    # Write data rows
    for ri, row in enumerate(norm_rows, start=2):
        for ci, key in enumerate(headers, start=1):
            ws.cell(row=ri, column=ci, value=row.get(key, ""))

    # Approximate autofit
    for ci, key in enumerate(headers, start=1):
        maxw = estimate_col_width(key)
        for ri in range(2, len(norm_rows) + 2):
            maxw = max(maxw, estimate_col_width(ws.cell(row=ri, column=ci).value))
        ws.column_dimensions[ws.cell(row=1, column=ci).column_letter].width = maxw

    # Create Meta_data_sheet and copy meta cols
    meta_ws = wb.create_sheet(title='Meta_data_sheet')
    for ci, key in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=ci, value=key).font = Font(bold=True)
    for ri, row in enumerate(norm_rows, start=2):
        for ci, key in enumerate(META_COLS, start=1):
            meta_ws.cell(row=ri, column=ci, value=rows[ri-2].get(key, ""))
    meta_ws.sheet_state = 'veryHidden'

    # Remove META columns from Data and reorder to MAIN_ORDER
    # Build mapping from header to column index
    header_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    # Build data rows according to MAIN_ORDER
    visible_headers = MAIN_ORDER
    # Rebuild the sheet content
    data_matrix = [visible_headers]
    for r in range(2, ws.max_row + 1):
        row_vals = []
        for h in visible_headers:
            val = None
            if h in header_idx:
                val = ws.cell(row=r, column=header_idx[h]).value
            else:
                val = ""
            row_vals.append(val)
        data_matrix.append(row_vals)

    # Clear and rewrite into the same sheet
    ws.delete_rows(1, ws.max_row)
    ws.delete_cols(1, ws.max_column)

    for ci, h in enumerate(visible_headers, start=1):
        cell = ws.cell(row=1, column=ci, value=h)
        cell.font = Font(bold=True)
    for ri in range(2, len(data_matrix) + 1):
        for ci, val in enumerate(data_matrix[ri-1], start=1):
            ws.cell(row=ri, column=ci, value=val)

    # Rename Data -> TestPlan
    ws.title = 'TestPlan'

    # Formatting rules for TestPlan
    header_fill = PatternFill("solid", fgColor="4472C4")  # blue fill
    header_align = Alignment(horizontal='center', vertical='center', wrap_text=False)
    data_text_align = Alignment(horizontal='left', vertical='top', wrap_text=True)
    data_num_align = Alignment(horizontal='center', vertical='top', wrap_text=True)

    # Apply header formatting
    for ci in range(1, len(visible_headers) + 1):
        cell = ws.cell(row=1, column=ci)
        cell.font = Font(bold=True)
        cell.fill = header_fill
        cell.alignment = header_align

    # Numbering for specified columns and apply alignments
    col_name_to_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    wrap_cols = [
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    ]

    for r in range(2, ws.max_row + 1):
        # Number items inside test steps and acceptance criteria
        for cname in ["Test Steps / Procedure", "Validation / Acceptance Criteria"]:
            cidx = col_name_to_idx.get(cname)
            if cidx:
                orig = ws.cell(row=r, column=cidx).value
                ws.cell(row=r, column=cidx, value=number_items(orig))
        for cname in visible_headers:
            cidx = col_name_to_idx.get(cname)
            if not cidx:
                continue
            cell = ws.cell(row=r, column=cidx)
            if cname in wrap_cols:
                cell.alignment = data_text_align
            elif cname == "Index":
                cell.alignment = data_num_align
            else:
                # Default text alignment
                cell.alignment = Alignment(horizontal='left', vertical='top', wrap_text=False)

    # Autofit columns (approx) and keep wrap for specified columns
    for cname, cidx in col_name_to_idx.items():
        maxw = estimate_col_width(cname)
        for r in range(2, ws.max_row + 1):
            maxw = max(maxw, estimate_col_width(ws.cell(row=r, column=cidx).value))
        ws.column_dimensions[ws.cell(row=1, column=cidx).column_letter].width = maxw

    # Apply thin borders
    apply_borders(ws, ws.max_row, ws.max_column)

    # Data validation for Code Generation (Required / Not) on data rows only
    cg_col = col_name_to_idx.get("Code Generation (Required / Not)")
    if cg_col:
        dv_list = DataValidation(type="list", formula1='"' + ",".join(ALLOWED_CODEGEN) + '"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv_list)
        dv_list.add(f"{ws.cell(row=1, column=cg_col).column_letter}2:{ws.cell(row=1, column=cg_col).column_letter}{ws.max_row}")

    # Ensure only TestPlan (visible) and Meta_data_sheet (veryHidden) exist
    # No sheet named 'Data' should exist
    if any(s.title == 'Data' for s in wb.worksheets):
        # Attempt to remove it
        for s in wb.worksheets:
            if s.title == 'Data':
                wb.remove(s)
        if any(s.title == 'Data' for s in wb.worksheets):
            raise RuntimeError("Validation failed: 'Data' sheet still present after normalization")

    return wb


def validate_xlsx_bytes(filepath: str):
    # Basic OOXML validation via zipfile and openpyxl load
    with open(filepath, 'rb') as f:
        data = f.read()
    # ZIP checks
    try:
        with ZipFile(BytesIO(data)) as z:
            names = z.namelist()
            assert '[Content_Types].xml' in names
            assert any(n.startswith('xl/') for n in names)
    except Exception as e:
        raise RuntimeError(f"ZIP/OOXML validation failed: {e}")
    # Try loading with openpyxl
    try:
        _ = load_workbook(filename=BytesIO(data), read_only=True, data_only=True)
    except Exception as e:
        raise RuntimeError(f"openpyxl load validation failed: {e}")


def main():
    # Build workbook from JSON rows
    rows = deepcopy(json_data)
    wb = build_workbook(rows)

    # Compute IST timestamp for filename
    ts = now_ist()
    fname = f"GPIO_TestPlan_{ts.strftime('%Y%m%d')}_{ts.strftime('%H%M%S')}.xlsx"
    out_dir = os.path.join('Test_Output', 'GPIO', 'TestPlan')
    ensure_dir(out_dir)
    out_path = os.path.join(out_dir, fname)

    # Save and validate
    wb.save(out_path)
    validate_xlsx_bytes(out_path)

    print(f"Generated: {out_path}")

if __name__ == '__main__':
    main()
