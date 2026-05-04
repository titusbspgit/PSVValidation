#!/usr/bin/env python3
import json
import os
import sys
import re
from datetime import datetime, timedelta, timezone
from zipfile import ZipFile

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# -------- Configuration --------
IP_NAME = "GPIO"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan"
FILENAME_RULE = "{ip}_TestPlan_{datestr}_{timestr}.xlsx"

# Source Test Plan JSON (latest produced by Ag-Emb-Mpsoc-TestPlan-Gen)
SOURCE_JSON_TEXT = r'''{
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "Independent control register for each GPIO.",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verifies default values and read/write behavior of GPIO control and group registers.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Some registers are intentionally skipped during default-value and write/read checks as marked by skip arrays. Note about input read behavior: without forcing a value, input may read as 1; forcing 0 can drive the level select bit high and cause mismatch.",
    "Test Steps / Procedure": "1) Read each per-pin and group register and record the value for comparison\n2) Compare each read value against the documented default for that register\n3) For each data pattern, write masked data to writable registers only\n4) Read back each register with the read mask applied\n5) Compute the expected value using write and read masks combined with the default value\n6) Compare the read value to the expected value and record any mismatches\n7) Report PASS if no mismatches were recorded; otherwise report FAIL",
    "Imparted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_INTR_RAW_STCLR1, INTR1_INTR_EN1, INTR1_INTR_STS1, INTR2_INTR_EN1, INTR2_INTR_STS1, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, GPIO_DOUT_GROUP1, GPIO_DOUT_GROUP2, GPIO_DOUT_GROUP3, GPIO_DOUT_GROUP4, GPIO_DIN_GROUP1, GPIO_DIN_GROUP2, GPIO_DIN_GROUP3, GPIO_DIN_GROUP4",
    "Validation / Acceptance Criteria": "1) Default read values match the documented defaults for all readable registers → PASS; any mismatch → FAIL\n2) After masked writes, masked read values equal the expected composition of written bits and preserved default bits → PASS; any mismatch → FAIL\n3) Overall result is PASS only if zero default mismatches and zero write/read mismatches are observed",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Default reset value verification for GPIO per-pin and group registers, followed by masked write/read verification using multiple data patterns across all non-skipped registers.",
    "Hidden_Remarks": "SKIPPING VRRW registers per skip_array and skip_rst_array. Note: When reading default values, DIN may become 1 automatically if not forced; if forced to 0 at DIN, bit-level select can become high, causing read value mismatch with expected default.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1) chk_rst_val(): For i=0..CNT-1 (CNT=49):\n   - addr = addr_array[i]\n   - If skip_rst_array[i] == 1: continue\n   - If read_mask_array[i] == 0x00000000: continue\n   - data_rd = read_reg(addr)\n   - data = (data_rd & 0xFFFFFFFE)\n   - If (data == default_value_array[i]) then PASS for this address else def_fail_cnt++ and log failure\n2) chk_rd_wr(): Define patterns chk_val = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}\n   For each data pattern data_wr in chk_val:\n   2a) Write phase for i=0..CNT-1:\n       - addr = addr_array[i]\n       - If skip_array[i] == 1: continue\n       - If write_mask_array[i] == 0x00000000: continue\n       - write_reg(addr, (data_wr & write_mask_array[i]))\n   2b) Read/verify phase for i=0..CNT-1:\n       - addr = addr_array[i]\n       - If skip_array[i] == 1: continue\n       - If write_mask_array[i] == 0x00000000: continue\n       - If read_mask_array[i] == 0x00000000: continue\n       - data_rd = (read_reg(addr) & read_mask_array[i])\n       - wr_n = (write_mask_array[i] ^ 0xFFFFFFFF)\n       - exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]))\n       - If data_rd == exp_val: PASS else wr_fail_cnt++ and log mismatch\n3) Test completion: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0)",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "1) For each i where read_mask_array[i] != 0 and skip_rst_array[i] == 0: (read_reg(addr_array[i]) & 0xFFFFFFFE) == default_value_array[i]; else increment def_fail_cnt\n2) For each data pattern and for each i where write_mask_array[i] and read_mask_array[i] are non-zero and skip_array[i] == 0: (read_reg(addr_array[i]) & read_mask_array[i]) == (((data_wr & read_mask_array[i] & write_mask_array[i])) | (((write_mask_array[i] ^ 0xFFFFFFFF) & read_mask_array[i] & default_value_array[i]))); else increment wr_fail_cnt\n3) Finish with code 0 (PASS) iff def_fail_cnt == 0 and wr_fail_cnt == 0; else finish(1)"
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "Programmable interrupt generation.",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies negative-edge interrupt generation, status, and clearing for GPIO inputs.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "A bounded wait is used to avoid infinite loops while waiting for the interrupt. Each iteration enables only one GPIO interrupt at a time to prevent ambiguity.",
    "Test Steps / Procedure": "1) Enable the relevant system interrupt output\n2) Drive the external pad control at 0xA0243ffc to a known high level\n3) Configure GPIO_8 through GPIO_39 for input and negative-edge detection, and clear per-pin raw status\n4) For each pin, clear the group raw status, enable only that pin in INTR1_INTR_EN1, and arm the wait\n5) Generate a falling edge using the pad control at 0xA0243ffc and wait within a timeout\n6) In the interrupt handler, verify the pin input level is low and the masked status bit is set in INTR1_INTR_STS1\n7) Clear per-pin raw status and group raw status, then verify INTR1_INTR_STS1 is cleared\n8) Clear the system raw status output before enabling the next pin",
    "Imparted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, INTR1_INTR_EN1, INTR1_INTR_STS1, GPIO_INTR_RAW_STCLR1, INTR_EN1, RAW_STCR1",
    "Validation / Acceptance Criteria": "1) The wait completes before the timeout for each pin after the falling edge is generated → PASS; a timeout indicates FAIL for that pin\n2) In the interrupt handler, the pin input level is low and the masked status bit is set for the active pin → PASS; any deviation → FAIL\n3) After issuing clears, the masked interrupt status becomes zero and the system raw status is cleared → PASS; otherwise → FAIL",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Negative-edge interrupt enable and validation across GPIO[8..39]. Sets input mode and negedge enable per pin, triggers a falling edge via external pad data register, checks pin DIN, masked status, and clearing at both GPIO and system-register levels.",
    "Hidden_Remarks": "Uses a single-pin enable per iteration to isolate events. Bounded timeout (e.g., 5000) prevents infinite hang while waiting for interrupt service.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1) Initialize: test_err = 0\n2) Conditionally enable GIC IRQ (87 for GPIO0, 88 for GPIO1) based on compile-time GPIO0/GPIO1 macro\n3) Enable system interrupt output: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) or LSS_SYSREG_INTR_EN1_GPIO1_INTR per macro\n4) Drive external pad bus to all ones: write_reg(0xA0243ffc, 0xFFFFFFFF)\n5) Configure per-pin for i=0..31:\n   - addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i*4)\n   - write_reg(addr1, (1<<20) | (1<<18) | (1<<16)) // doe=1 (input), neie=1, iclr=1\n   - wait_on(10)\n6) For each i=0..31 iteration:\n   - wr_val = 1u << i\n   - write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) // pre-clear group raw\n   - write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) // enable only this bit\n   - wait_on(10)\n   - int_pend = 1\n   - write_reg(0xA0243ffc, 0xFFFFFFFF); wait_on(30); write_reg(0xA0243ffc, ~wr_val) // generate falling edge\n   - timeout = 5000; while (int_pend && timeout--) wait_on(10); if (timeout == 0) { log timeout; test_err++; }\n7) On interrupt (Default_IRQHandler):\n   - local_wr = 1u << i; int_pend = 0\n   - Return pad to high: write_reg(0xA0243ffc, 0xFFFFFFFF)\n   - raddr = MIZAR_GPIO_GP0_GPIO_8 + (i*4); rdata = read_reg(raddr)\n   - Check DIN low: if ((rdata & 0x1) != 0) test_err++\n   - If ((rdata & 0x2) != 0x0) then:\n       * rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n       * If ((rdata_grp & local_wr) == 0) test_err++\n       * raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(raddr2, (1<<20) | (1<<16)) // doe=1, iclr=1\n       * write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr)\n       * rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) test_err++\n       * write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or GPIO1); GIC_ClearIRQ(87 or 88)\n     else test_err++\n8) Completion: finish(test_err)",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin i, the interrupt wait loop must exit before timeout after generating a falling edge; else log timeout and increment test_err\n2) In ISR: (a) (read_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4) & 0x1) == 0 (DIN low after negedge) and (b) (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) & (1<<i)) != 0; else increment test_err\n3) After clearing per-pin and group raw status, read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) == 0; else increment test_err\n4) System RAW_STCR1 corresponding bit is cleared after write; else increment test_err\n5) finish(test_err) == 0 indicates PASS"
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "Programmable interrupt generation.",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Validates positive-edge interrupt behavior on all pins, including status reporting and clearing.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "The group interrupt is masked during service and re-enabled afterward. A bounded wait prevents infinite hangs while waiting for the edge-triggered event.",
    "Test Steps / Procedure": "1) Enable the relevant system interrupt output\n2) Configure GPIO_8 through GPIO_39 for positive-edge detection\n3) Set the IO control groups so the pins operate as inputs\n4) Enable all pins in INTR1_INTR_EN1\n5) For each pin, drive the pad low, arm the wait, and then drive high at 0xA0243ffc to create a rising edge\n6) In the interrupt handler, mask the group, verify the group status is asserted, clear per-pin raw status for all pins, and verify the group status is cleared\n7) Clear the system raw status and re-enable the group for the next iteration",
    "Imparted Registers": "GPIO_8, GPIO_9, GPIO_10, GPIO_11, GPIO_12, GPIO_13, GPIO_14, GPIO_15, GPIO_16, GPIO_17, GPIO_18, GPIO_19, GPIO_20, GPIO_21, GPIO_22, GPIO_23, GPIO_24, GPIO_25, GPIO_26, GPIO_27, GPIO_28, GPIO_29, GPIO_30, GPIO_31, GPIO_32, GPIO_33, GPIO_34, GPIO_35, GPIO_36, GPIO_37, GPIO_38, GPIO_39, GPIO_IO_CTRL_GROUP1, GPIO_IO_CTRL_GROUP2, GPIO_IO_CTRL_GROUP3, GPIO_IO_CTRL_GROUP4, INTR1_INTR_EN1, INTR1_INTR_STS1, RAW_STCR1, INTR_EN1",
    "Validation / Acceptance Criteria": "1) The wait loop exits before timeout after a rising edge is generated for each pin → PASS; a timeout indicates FAIL\n2) Group interrupt status is asserted when the event occurs and is fully cleared after per-pin raw clears → PASS; otherwise → FAIL\n3) System raw status is cleared as expected after service → PASS; otherwise → FAIL",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Positive-edge interrupt enable across GPIO[8..39], input-mode configuration via group IO control registers, rising-edge generation via external pad driver, verification of group masked status, clearing per-pin raw status, and system raw clear checks.",
    "Hidden_Remarks": "Masks the group interrupt during service to avoid re-entrancy and re-enables it afterward. Uses a bounded timeout of ~2000 iterations for the wait loop.",
    "Hidden_Test_Steps_Procedure": "Entry: test_case()\n1) Conditionally enable GIC IRQ (87 for GPIO0, 88 for GPIO1) based on compile-time macro\n2) Enable system interrupt output: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or GPIO1)\n3) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000) // peie=1 per pin\n4) wait_on(10)\n5) Configure input mode via group IO control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); same for GROUP2..GROUP4; wait_on(10)\n6) Enable group pin interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF)\n7) Loop i=0..31:\n   - write_reg(0xA0243ffc, 0x00000000); wait_on(10)\n   - int_pend = 1\n   - write_reg(0xA0243ffc, 0xFFFFFFFF) // generate rising edge\n   - timeout=2000; while (int_pend==1 && --timeout>0) wait_on(10); if (timeout==0) { log; test_err++; break; }\n   - write_reg(0xA0243ffc, 0x00000000); wait_on(10)\n8) Default_IRQHandler():\n   - wr_val = 1<<i; int_pend = 0\n   - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)\n   - write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) // mask group\n   - If ((rdata_grp & 0xFFFFFFFF) != 0) success else { log; test_err++; }\n   - For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000) // iclr=1 per pin\n   - wait_on(2)\n   - rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) { log; test_err++; }\n   - Clear sysreg raw: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or GPIO1); verify cleared by read-back; if not cleared test_err++\n   - Re-enable: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); clear GIC IRQ\n9) Completion: finish(test_err)",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "1) For each pin, the bounded wait terminates before timeout after a rising edge is applied; else increment test_err\n2) In ISR: masked group status (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1)) is non-zero on entry, then becomes 0 after per-pin raw clears; else increment test_err\n3) System RAW_STCR1 bit is cleared after service; a residual bit indicates failure; else increment test_err\n4) finish(test_err) == 0 indicates PASS"
  }
}'''

META_COLUMNS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

VISIBLE_COLUMNS_FINAL = [
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

HEADER_FILL = PatternFill("solid", fgColor="FF4472C4")  # Office blue
THIN = Side(border_style="thin", color="FF000000")
BORDER_THIN = Border(left=THIN, right=THIN, top=THIN, bottom=THIN)


def ist_now():
    # IST is UTC+05:30
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(tz=ist)


def normalize_array_from_object(obj):
    if isinstance(obj, list):
        return obj
    if isinstance(obj, dict):
        return list(obj.values())
    raise ValueError("Unsupported JSON top-level type; expected array or object with test entries")


def union_keys_preserve_order(items):
    seen = []
    seen_set = set()
    for it in items:
        if isinstance(it, dict):
            for k in it.keys():
                if k not in seen_set:
                    seen.append(k)
                    seen_set.add(k)
    return seen


def strip_existing_prefix(s):
    # Remove common leading numbering/bullets like '1) ', '1. ', '- ', '* '
    return re.sub(r"^\s*(?:[-*]|\d+[\.)])\s*", "", s)


def numbered_block(text):
    if not isinstance(text, str):
        return text
    lines = [ln for ln in (text or "").splitlines()]
    # Drop empty lines at ends, keep internal empties as blanks with numbering
    out = []
    idx = 1
    for ln in lines:
        t = strip_existing_prefix(ln.rstrip())
        if t == "":
            # still number blank to preserve count
            out.append(f"{idx}. ")
        else:
            out.append(f"{idx}. {t}")
        idx += 1
    return "\n".join(out) if out else text


def set_autofit_col_width(ws):
    for col_cells in ws.iter_cols(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        max_len = 0
        for c in col_cells:
            v = c.value
            if v is None:
                l = 0
            else:
                s = str(v)
                l = max(len(line) for line in s.splitlines()) if "\n" in s else len(s)
            if l > max_len:
                max_len = l
        # small padding; Excel measures approx in chars
        col_letter = col_cells[0].column_letter
        ws.column_dimensions[col_letter].width = min(120, max(10, max_len + 2))


def apply_borders(ws):
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = BORDER_THIN


def apply_header_style(ws):
    for cell in ws[1]:
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center", wrap_text=True)
        cell.fill = HEADER_FILL


def apply_data_alignment(ws):
    # First column (Index) center; others left with vertical top
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            cell = ws.cell(row=r, column=c)
            if ws.cell(row=1, column=c).value == "Index":
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=(ws.cell(row=1, column=c).value in WRAP_COLUMNS))


def adjust_row_heights(ws):
    base = 15  # points
    for r in range(1, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            if isinstance(v, str) and "\n" in v:
                lines = v.count("\n") + 1
                if lines > max_lines:
                    max_lines = lines
        ws.row_dimensions[r].height = base * max_lines


def add_validation(ws):
    # Apply only to 'Code Generation (Required / Not)' column for data rows
    col_idx = None
    for c in range(1, ws.max_column + 1):
        if ws.cell(row=1, column=c).value == "Code Generation (Required / Not)":
            col_idx = c
            break
    if col_idx is None:
        return
    # Data validation list
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
    ws.add_data_validation(dv)
    rng = f"{ws.cell(row=2, column=col_idx).coordinate}:{ws.cell(row=max(2, ws.max_row), column=col_idx).coordinate}"
    dv.add(rng)


def validate_xlsx_zip(path):
    if not os.path.exists(path):
        return False
    try:
        with ZipFile(path, 'r') as z:
            must = {"[Content_Types].xml", "_rels/.rels", "xl/workbook.xml"}
            names = set(z.namelist())
            return must.issubset(names)
    except Exception:
        return False


def main():
    try:
        src_obj = json.loads(SOURCE_JSON_TEXT)
    except Exception as e:
        print(f"ERROR: Invalid JSON input: {e}", file=sys.stderr)
        sys.exit(1)

    rows = normalize_array_from_object(src_obj)
    if not rows:
        print("ERROR: Empty JSON input", file=sys.stderr)
        sys.exit(1)

    # Build union of keys (staging)
    all_keys = union_keys_preserve_order(rows)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Write staging headers and rows (preserving values exactly)
    ws.append(all_keys)
    for rec in rows:
        row_vals = [rec.get(k, "") for k in all_keys]
        ws.append(row_vals)

    ws.freeze_panes = "A2"
    apply_header_style(ws)
    set_autofit_col_width(ws)
    apply_borders(ws)

    # Meta sheet
    meta = wb.create_sheet("Meta_data_sheet")
    meta.append(META_COLUMNS)
    for rec in rows:
        meta.append([rec.get(k, "") for k in META_COLUMNS])
    meta.sheet_state = 'veryHidden'
    set_autofit_col_width(meta)
    apply_borders(meta)

    # Transform staging sheet -> final visible TestPlan sheet (no new visible sheet)
    ws.title = "TestPlan"
    # Clear and rebuild with final visible columns
    ws.delete_rows(1, ws.max_row)
    ws.append(VISIBLE_COLUMNS_FINAL)

    def map_visible(rec):
        m = {}
        for col in VISIBLE_COLUMNS_FINAL:
            if col == "Impacted Registers":
                m[col] = rec.get("Imparted Registers", "")
            elif col in {"Test Steps / Procedure", "Validation / Acceptance Criteria"}:
                m[col] = numbered_block(rec.get(col, ""))
            else:
                m[col] = rec.get(col, "")
        return [m[c] for c in VISIBLE_COLUMNS_FINAL]

    for rec in rows:
        ws.append(map_visible(rec))

    apply_header_style(ws)
    apply_data_alignment(ws)
    set_autofit_col_width(ws)
    adjust_row_heights(ws)
    apply_borders(ws)
    add_validation(ws)

    # Ensure no sheet named 'Data' remains
    if 'Data' in wb.sheetnames:
        del wb['Data']

    # Compute IST timestamp for filename
    now_ist = ist_now()
    datestr = now_ist.strftime('%Y%m%d')
    timestr = now_ist.strftime('%H%M%S')

    out_dir = OUTPUT_DIR
    os.makedirs(out_dir, exist_ok=True)
    out_name = FILENAME_RULE.format(ip=IP_NAME, datestr=datestr, timestr=timestr)
    out_path = os.path.join(out_dir, out_name)

    wb.save(out_path)

    if not validate_xlsx_zip(out_path):
        print("ERROR: XLSX validation failed", file=sys.stderr)
        sys.exit(2)

    print(f"Generated: {out_path}")
    print(f"IST Timestamp: {datestr}_{timestr}")

if __name__ == "__main__":
    main()
