# -*- coding: utf-8 -*-
import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Alignment, Font, Border, Side, PatternFill
from openpyxl.utils import get_column_letter

# Constants per requirements
IP_NAME = "GPIO"
IST_DATE = "20260421"  # YYYYMMDD
IST_TIME = "053000"    # HHMMSS
OUTPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")
OUTPUT_FILENAME = f"{IP_NAME}_TestPlan_{IST_DATE}_{IST_TIME}.xlsx"
OUTPUT_PATH = os.path.join(OUTPUT_DIR, OUTPUT_FILENAME)

# JSON input embedded exactly as provided
RAW_JSON = r'''{
  "metadata": {
    "ip_name": "GPIO",
    "repo": "titusbspgit/PSVValidation",
    "branch": "main",
    "source_subdir": "TestRepo/gpio",
    "generated_by": "Ag-Emb-Mpsoc-TestPlan-Gen Agent",
    "generation_timestamp_utc": "2026-04-21T00:00:00Z",
    "notes": "Test documentation generated strictly from repository sources under TestRepo/gpio. Columns populated per deterministic rules; rewritten MAIN fields avoid function/macro names and use register names where available."
  },
  "test_cases": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "AHB 32-bit register interface",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Performs two phases: (1) default/reset value verification for each address in addr_array[] (skipping entries per skip_rst_array and non-readable per read_mask_array), and (2) masked write/read checks using six data patterns across all addresses (skipping per skip_array and non-writable per write_mask_array). Pass/fail is accumulated in def_fail_cnt and wr_fail_cnt and returned via finish().",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "VRRW registers are explicitly skipped in write/read phase. Default-value reading of DIN can become 1 without external forcing; forcing DIN low causes bit-level selection behavior that may affect expected reads.",
      "Test Steps / Procedure": [
        "Entry point begins.",
        "Perform reset/default verification over the register list, skipping entries marked to be skipped and those marked as not readable. Compare each read value with the expected reset value after masking the least significant bit.",
        "Execute masked write checks for each of six data patterns across the register list, writing only to entries that allow writes and are not skipped.",
        "Read back each written register with read masking, and compare against the expected value computed from the write mask, read mask, and default value for non-writable bits.",
        "Accumulate any mismatches in counters and set the final test status to pass if no mismatches occurred or to fail if any mismatch was detected."
      ],
      "Impacted Registers": "GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, GP0_INTR2_INTR_EN1, GP0_INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": [
        "Default phase: For each tested register, after masking the least significant bit of the read value, the result must equal the documented default value for that address.",
        "Write/read phase: For each tested register and each data pattern, the masked read-back value must equal the combination of written bits (limited by the write and read masks) and original default bits for positions that are not writable.",
        "Overall: The test passes only if there are zero default mismatches and zero write/read mismatches; otherwise it fails."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "program.c implements test_case(): calls chk_rst_val() then chk_rd_wr(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). chk_rst_val(): for i in [0..CNT-1], addr=addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); compare with default_value_array[i]; else def_fail_cnt++. chk_rd_wr(): for j over chk_val[]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}: data_wr=chk_val[j]; write loop: for i, if (skip_array[i]) continue; if (write_mask_array[i]==0) continue; else write_reg(addr_array[i], (data_wr & write_mask_array[i])); read/compare loop: for i, if (skip_array[i]) continue; if (write_mask_array[i]==0 || read_mask_array[i]==0) continue; data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare; else wr_fail_cnt++. soft_reset_chk() is #ifdef 0 and not executed.",
      "Hidden_Remarks": "test_define.c: //80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers; const unsigned int skip_array[...] marks several group control and data registers as skipped for write/read; const unsigned int skip_rst_array[...] skips group IO/DOUT/DIN for reset checks. Note: when reading default values the din value is becoming 1 automatically if we don't force any value, but if we force zero to din bit level sel becoming high, so that reading value not matched with expected value.",
      "Hidden_Test_Steps_Procedure": [
        "Entry: int test_case()",
        "Call chk_rst_val()",
        "In chk_rst_val(): for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0x00000000) continue; data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); if (data == default_value_array[i]) pass; else def_fail_cnt++ and printf failure.",
        "Return to test_case(); call chk_rd_wr()",
        "In chk_rd_wr(): unsigned int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; for (j=0;j<6;j++): data_wr=chk_val[j]; write phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; else write_reg(addr,(data_wr & write_mask_array[i]));",
        "Read/compare phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) pass; else wr_fail_cnt++ and printf failure.",
        "Back in test_case(): if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0)."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": [
        "Default check: (data_rd & 0xfffffffe) == default_value_array[i] for all i where read_mask_array[i]!=0 and skip_rst_array[i]==0.",
        "Write/read check: For each j in chk_val and each i where write_mask_array[i]!=0, read_mask_array[i]!=0, skip_array[i]==0: data_rd == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])).",
        "Finish criteria: finish(0) if def_fail_cnt==0 and wr_fail_cnt==0 else finish(1)."
      ]
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "neie: Negative edge interrupt enable",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Configures GPIO pins 8..39 as inputs with negative-edge interrupt enabled and clears any pending raw status. For each pin, the test enables that pin’s interrupt, generates a falling edge using the external drive register, waits with a bounded timeout for the interrupt, and in the handler validates input level, raw/group interrupt status assertion, and proper clearing of per-pin and group raw status before acknowledging system-level and GIC interrupts.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "A known high level is driven before configuration; the wait is armed before edge generation to avoid race; wait loops are bounded to prevent hangs.",
      "Test Steps / Procedure": [
        "Entry point begins.",
        "Enable the relevant interrupt in the interrupt controller.",
        "Enable the corresponding system register interrupt output.",
        "Drive the external GPIO driver register to a high level to establish a known state.",
        "For each pin offset from 8 to 39: configure the per‑pin control register to input mode, enable negative‑edge detection, and clear any latched raw status; apply a short wait.",
        "For each pin index from 0 to 31: clear the group raw status bit for that pin; enable only that pin in the group enable register; apply a short wait; arm the pending flag before generating the edge; create a falling edge on that pin using the external driver by transitioning from high to a value with that pin low.",
        "Wait for the interrupt with a finite timeout, delaying between checks; on timeout, record an error for the corresponding pin.",
        "In the interrupt handler: de‑assert the pending flag; restore the external driver to high; read the per‑pin register for the current pin and verify the input level indicates low after a falling edge.",
        "Still in the handler: confirm that the per‑pin raw‑status indication is set for a falling edge; read the group interrupt status and verify the bit corresponding to the current pin is set.",
        "Clear the per‑pin raw status using the per‑pin control register while keeping input mode; clear the group raw status using the group raw status clear register; verify the group status register reads as zero.",
        "Acknowledge and clear the system‑level interrupt output and the interrupt controller source for the GPIO instance."
      ],
      "Impacted Registers": "INTR_EN1, GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
      "Validation / Acceptance Criteria": [
        "For each tested pin, an interrupt must be observed before the bounded wait times out; a timeout constitutes a failure for that pin.",
        "Within the handler for a falling edge, the per‑pin input level must indicate low; otherwise record a failure.",
        "The group interrupt status must present the bit corresponding to the active pin during service, and must read as zero after clearing actions; any deviation constitutes a failure.",
        "System‑level interrupt status must be cleared via the appropriate register and the platform interrupt controller must be acknowledged."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "program.c test_case(): optionally GIC_EnableIRQ(87/88); write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR); write_reg(0xA0243ffc, 0xffffffff); for (i=0;i<32;i++): addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10); for (i=0;i<32;i++): wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) printf timeout error and test_err++; finish(test_err). Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }",
      "Hidden_Remarks": "Comments indicate: drive all high initially (known state); arm the wait BEFORE generating the edge to avoid race; bounded wait instead of infinite loop (timeout = 5000).",
      "Hidden_Test_Steps_Procedure": [
        "Entry: int test_case()",
        "#ifdef GPIO0 GIC_EnableIRQ(87); #endif; #ifdef GPIO1 GIC_EnableIRQ(88); #endif",
        "#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #endif; #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR); #endif",
        "write_reg(0xA0243ffc, 0xffffffff);",
        "for (i=0;i<32;i++): addr1=MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10);",
        "for (i=0;i<32;i++): wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while (int_pend && timeout--) wait_on(10); if (timeout==0) { printf timeout; test_err++; }",
        "finish(test_err);",
        "ISR: void Default_IRQHandler(): unsigned int local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata=read_reg(raddr); if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; raddr2=MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++; #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": [
        "Timeout check: while (int_pend && timeout--) wait_on(10); if (timeout==0) error.",
        "DIN check: if ((rdata & 0x1) != 0) error (DIN should be 0 after falling edge).",
        "Per‑pin raw check: if ((rdata & 0x2) == 0x0) error; else proceed.",
        "Group status set: rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) error.",
        "Post‑clear group status: after clears, rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) error."
      ]
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "peie: Positive edge interrupt enable",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Enables positive‑edge interrupt on all GPIO pins 8..39, sets them to input mode, and enables the group interrupt. For each pin, the test generates a rising edge using the external drive register, waits with a bounded timeout for the interrupt, and in the handler verifies that a group interrupt occurred, clears per‑pin raw status for all pins, confirms the group status clears to zero, clears system‑level status, and re‑enables the group for the next iteration.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "The pending flag is armed before generating each rising edge; waits are bounded to avoid infinite loops; the handler masks the group enable during service and re‑enables it after clearing.",
      "Test Steps / Procedure": [
        "Entry point begins.",
        "Enable the relevant interrupt in the interrupt controller.",
        "Enable the corresponding system register interrupt output.",
        "For each pin offset from 8 to 39: program the per‑pin control register to enable positive‑edge detection.",
        "Apply a short wait, then set the group I/O control registers to configure the pins as inputs.",
        "Apply a short wait, then enable the entire group interrupt enable register.",
        "For each pin index from 0 to 31: drive the external GPIO driver register low, wait briefly, arm the pending flag, then drive the register high to generate a rising edge.",
        "Wait for the interrupt with a finite timeout, delaying between checks; on timeout, record an error and stop further iterations.",
        "Optionally return the external driver to low and wait briefly in preparation for the next iteration.",
        "In the interrupt handler: capture the group status, mask the group enable, verify that the group status is non‑zero (an interrupt occurred).",
        "In the handler: clear per‑pin raw status for all pins using the per‑pin control registers and wait briefly; verify the group status register reads as zero after clearing.",
        "Clear the system‑level interrupt status and verify it is cleared by reading back the system register.",
        "Re‑enable the group interrupt enable register and acknowledge the interrupt controller source."
      ],
      "Impacted Registers": "INTR_EN1, GP0_GPIO_8, GP0_GPIO_9, GP0_GPIO_10, GP0_GPIO_11, GP0_GPIO_12, GP0_GPIO_13, GP0_GPIO_14, GP0_GPIO_15, GP0_GPIO_16, GP0_GPIO_17, GP0_GPIO_18, GP0_GPIO_19, GP0_GPIO_20, GP0_GPIO_21, GP0_GPIO_22, GP0_GPIO_23, GP0_GPIO_24, GP0_GPIO_25, GP0_GPIO_26, GP0_GPIO_27, GP0_GPIO_28, GP0_GPIO_29, GP0_GPIO_30, GP0_GPIO_31, GP0_GPIO_32, GP0_GPIO_33, GP0_GPIO_34, GP0_GPIO_35, GP0_GPIO_36, GP0_GPIO_37, GP0_GPIO_38, GP0_GPIO_39, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, RAW_STCR1",
      "Validation / Acceptance Criteria": [
        "For each iteration, an interrupt must be observed before the bounded wait times out; a timeout constitutes failure.",
        "During service, the group interrupt status must be non‑zero (indicating at least one pin asserted), and must read as zero after the per‑pin raw‑status clear sequence; any deviation constitutes failure.",
        "System‑level interrupt raw status must be cleared and verified by a read‑back showing the relevant status bit is de‑asserted."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "program.c void test_case(): #ifdef GPIO0 GIC_EnableIRQ(87); #endif #ifdef GPIO1 GIC_EnableIRQ(88); #endif; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR); for (i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); wait_on(10); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); ... GROUP2..GROUP4 likewise; wait_on(10); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); for (i=0;i<32;i++): write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) { printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10); finish(test_err). ISR Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) { /*success log*/ } else { printf error; test_err++; } for (j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { /*success*/ } else { printf error; test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf not cleared; test_err++; } #endif #ifdef GPIO1 similar for GPIO1 #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); #ifdef GPIO0 GIC_ClearIRQ(87); #endif #ifdef GPIO1 GIC_ClearIRQ(88); #endif",
      "Hidden_Remarks": "Comments indicate: enable posedge per pin; arm pending BEFORE edge; mask group during service; use bounded wait (timeout=2000) to avoid infinite hang; verify system register raw status is cleared via read‑back.",
      "Hidden_Test_Steps_Procedure": [
        "Entry: void test_case()",
        "#ifdef GPIO0 GIC_EnableIRQ(87); #endif; #ifdef GPIO1 GIC_EnableIRQ(88); #endif",
        "#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #endif; #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR); #endif",
        "for (i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000);",
        "wait_on(10); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);",
        "wait_on(10); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);",
        "for (i=0;i<32;i++): write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); int timeout=2000; while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) { printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10);",
        "finish(test_err);",
        "ISR: void Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) { /*success*/ } else { printf error; test_err++; } for (j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { /*success*/ } else { printf error; test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { test_err++; } #endif #ifdef GPIO1 similar for GPIO1 #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); #ifdef GPIO0 GIC_ClearIRQ(87); #endif #ifdef GPIO1 GIC_ClearIRQ(88); #endif"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": [
        "Timeout check: while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) error and break.",
        "Group interrupt observed: if ((rdata_grp & 0xffffffff) == 0) error.",
        "Group clear check: after per‑pin clears and wait_on(2), if (rdata_grp != 0x0) error.",
        "System raw clear verify: after write to MIZAR_LSS_SYSREG_RAW_STCR1, read back must show the respective bit cleared; otherwise error."
      ]
    }
  ]
}'''

# Column definitions per Stage1
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
    "Code Generation (Required / Not)"
]
META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria"
]

WRAP_COLUMNS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

BORDER_THIN = Side(style="thin", color="000000")
BORDER = Border(left=BORDER_THIN, right=BORDER_THIN, top=BORDER_THIN, bottom=BORDER_THIN)
HEADER_FILL = PatternFill("solid", fgColor="D9E1F2")


def to_cell_value(v):
    if isinstance(v, list):
        return "\n".join(str(x) for x in v)
    if v is None:
        return ""
    return str(v)


def auto_fit(ws):
    # Compute approximate widths
    for col_idx, col in enumerate(ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column), start=1):
        max_len = 0
        for cell in col:
            val = cell.value
            if val is None:
                continue
            s = str(val)
            # consider multiline by longest line
            s = max((len(p) for p in s.split('\n')), default=0)
            if s > max_len:
                max_len = s
        width = min(max(10, max_len + 2), 80)
        ws.column_dimensions[get_column_letter(col_idx)].width = width
    # Auto-fit row heights after wrap: let Excel handle, but set to default auto by setting height None
    for r in range(1, ws.max_row + 1):
        ws.row_dimensions[r].height = None


def apply_table_format(ws):
    # Header format
    header_font = Font(bold=True)
    for cell in ws[1]:
        cell.font = header_font
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL
        cell.border = BORDER
    # Data rows
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            header = ws.cell(row=1, column=cell.column).value
            if header in WRAP_COLUMNS:
                cell.alignment = Alignment(wrap_text=True, vertical="top")
            elif header == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top")
            else:
                cell.alignment = Alignment(vertical="top")
            cell.border = BORDER


def build_workbook(data):
    test_cases = data.get("test_cases", [])
    if not isinstance(test_cases, list) or len(test_cases) == 0:
        raise ValueError("No test_cases found or not a list")

    # Determine combined schema preserving first-seen order
    columns = []
    seen = set()
    for row in test_cases:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                columns.append(k)

    # Create base workbook and Data sheet
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row
    for c_idx, key in enumerate(columns, start=1):
        ws.cell(row=1, column=c_idx, value=key)

    # Data rows
    for r_idx, row in enumerate(test_cases, start=2):
        for c_idx, key in enumerate(columns, start=1):
            ws.cell(row=r_idx, column=c_idx, value=to_cell_value(row.get(key, "")))

    # Basic formatting on Data
    ws.freeze_panes = "A2"
    for cell in ws[1]:
        cell.font = Font(bold=True)
    auto_fit(ws)

    # Build Meta_data_sheet
    meta_ws = wb.create_sheet("Meta_data_sheet")
    for c_idx, key in enumerate(META_COLUMNS, start=1):
        meta_ws.cell(row=1, column=c_idx, value=key)
    for r_idx, row in enumerate(test_cases, start=2):
        for c_idx, key in enumerate(META_COLUMNS, start=1):
            meta_ws.cell(row=r_idx, column=c_idx, value=to_cell_value(row.get(key, "")))
    # Very hidden
    meta_ws.sheet_state = "veryHidden"

    # Normalize MAIN sheet: rename Data -> TestPlan, drop META columns, enforce order
    ws.title = "TestPlan"

    # Create a temp sheet with MAIN columns in required order
    tmp = wb.create_sheet("_TMP_TestPlan")
    for c_idx, key in enumerate(MAIN_COLUMNS, start=1):
        tmp.cell(row=1, column=c_idx, value=key)

    # Map from header to index in original TestPlan
    header_to_idx = {ws.cell(row=1, column=c).value: c for c in range(1, ws.max_column + 1)}

    for r in range(2, ws.max_row + 1):
        for c_idx, key in enumerate(MAIN_COLUMNS, start=1):
            src_col = header_to_idx.get(key, None)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            tmp.cell(row=r, column=c_idx, value=val)

    # Delete old TestPlan and rename tmp
    wb.remove(ws)
    tmp.title = "TestPlan"

    # Apply strict formatting
    tmp.freeze_panes = "A2"
    apply_table_format(tmp)
    auto_fit(tmp)

    return wb


def main():
    data = json.loads(RAW_JSON)
    wb = build_workbook(data)

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(OUTPUT_PATH)
    print(f"Saved: {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
