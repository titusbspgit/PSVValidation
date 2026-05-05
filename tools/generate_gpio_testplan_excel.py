#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Deterministic Excel (.xlsx) generator for GPIO Test Plan
- Consumes embedded JSON data (array of objects)
- Produces a single workbook with two sheets:
  * TestPlan (visible) — formatted per Stage1 rules
  * Meta_data_sheet (veryHidden) — raw META fields only
- Enforces column order, wrapping, borders, data validation
- Uses IST (Asia/Kolkata) timestamp in filename: GPIO_TestPlan_YYYYMMDD_HHMMSS.xlsx
- Saves into: Test_Output/GPIO/TestPlan/
- Validates XLSX as a ZIP-based OOXML before exit
"""

import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
import zipfile

# ---------------------- Embedded JSON data (array) ----------------------
TESTPLAN_JSON = [
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Checks default values of GPIO-related registers and verifies masked write/read behavior across multiple registers using defined patterns.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "VRRW registers are skipped per skip_array. During default reads, DIN may become 1 unless forced; forcing 0 can raise level-select, causing mismatch.",
    "Test Steps / Procedure": "1) Initialize default and write fail counters to zero.\n2) Execute default value check loop across all entries.\n3) Execute six-pattern masked write followed by masked read/compare across all entries.\n4) Determine pass/fail based on accumulated failure counters.",
    "Impacted Registers": "gp0_gpio_8, gp0_gpio_9, gp0_gpio_10, gp0_gpio_11, gp0_gpio_12, gp0_gpio_13, gp0_gpio_14, gp0_gpio_15, gp0_gpio_16, gp0_gpio_17, gp0_gpio_18, gp0_gpio_19, gp0_gpio_20, gp0_gpio_21, gp0_gpio_22, gp0_gpio_23, gp0_gpio_24, gp0_gpio_25, gp0_gpio_26, gp0_gpio_27, gp0_gpio_28, gp0_gpio_29, gp0_gpio_30, gp0_gpio_31, gp0_gpio_32, gp0_gpio_33, gp0_gpio_34, gp0_gpio_35, gp0_gpio_36, gp0_gpio_37, gp0_gpio_38, gp0_gpio_39, gpio_intr_raw_stclr1, gp0_intr1_intr_en1, gp0_intr1_intr_sts1, gp0_intr2_intr_en1, gp0_intr2_intr_sts1, gpio_io_ctrl_group1, gpio_io_ctrl_group2, gpio_io_ctrl_group3, gpio_io_ctrl_group4, gpio_dout_group1, gpio_dout_group2, gpio_dout_group3, gpio_dout_group4, gpio_din_group1, gpio_din_group2, gpio_din_group3, gpio_din_group4",
    "Validation / Acceptance Criteria": "- Default check: For each address i, after applying mask (data_rd & 0xfffffffe), value must equal default_value_array[i]; on mismatch, increment def_fail_cnt.\n- Write/read check: For each pattern and address i, read value masked must equal exp_val computed from write/read/default masks; on mismatch, increment wr_fail_cnt.\n- Final result: PASS if def_fail_cnt == 0 and wr_fail_cnt == 0; else FAIL.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
    "Hidden_Test_Description": "Checks default values of GPIO-related registers and verifies masked write/read behavior across multiple registers using defined patterns.",
    "Hidden_Remarks": "VRRW registers are skipped per skip_array. During default reads, DIN may become 1 unless forced; forcing 0 can raise level-select, causing mismatch.",
    "Hidden_Test_Steps_Procedure": "1) Initialize: def_fail_cnt = 0; wr_fail_cnt = 0.\n2) Call chk_rst_val():\n   2.1) For i from 0 to CNT-1:\n        a) addr = addr_array[i].\n        b) If skip_rst_array[i] == 1: continue to next i.\n        c) If read_mask_array[i] == 0x00000000: continue to next i.\n        d) data_rd = read_reg(addr).\n        e) data = (data_rd & 0xfffffffe).\n        f) If data == default_value_array[i]: (pass path; optional debug print under DEBUG_DISPLAY) else:\n           - def_fail_cnt++.\n           - printf(\"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\\tDATA : 0x%x\\n\", addr, default_value_array[i], data, data_rd).\n3) Call chk_rd_wr():\n   3.1) Define patterns chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}.\n   3.2) For j from 0 to 5:\n        a) data_wr = chk_val[j].\n        b) Write phase: For i from 0 to CNT-1:\n           i)   addr = addr_array[i].\n           ii)  If skip_array[i] == 1: continue to next i.\n           iii) If write_mask_array[i] == 0x00000000: continue to next i.\n           iv)  Else write_reg(addr, (data_wr & write_mask_array[i])).\n        c) Read/compare phase: For i from 0 to CNT-1:\n           i)   addr = addr_array[i].\n           ii)  If skip_array[i] == 1: continue to next i.\n           iii) If write_mask_array[i] == 0x00000000: continue to next i.\n           iv)  If read_mask_array[i] == 0x00000000: continue to next i.\n           v)   data_rd = (read_reg(addr) & read_mask_array[i]).\n           vi)  wr_n = (write_mask_array[i] ^ 0xffffffff).\n           vii) exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n           viii)If data_rd == exp_val: (pass path; optional debug print under DEBUG_DISPLAY) else:\n                - wr_fail_cnt++.\n                - printf(\"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\\n\", addr, exp_val, data_rd).\n4) In test_case():\n   4.1) Call chk_rst_val().\n   4.2) Call chk_rd_wr().\n   4.3) If (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) else finish(0).\n5) soft_reset_chk() is compiled out under #ifdef 0 and not executed.",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "In chk_rst_val: (data_rd & 0xfffffffe) must equal default_value_array[i]; else def_fail_cnt++.\nIn chk_rd_wr: data_rd (masked) must equal exp_val; else wr_fail_cnt++.\nFinal: finish(0) if both counters zero; otherwise finish(1)."
  },
  {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "neie",
    "Test Case Name": "test_gpio_negedge_intr_en",
    "Test Description": "Verifies negative-edge interrupt generation for GPIOs 8–39, including raw status set/clear and group status handling with ISR.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Uses system interrupt routing and GIC IRQs (87/88) depending on configuration. Uses a bounded wait to avoid infinite loops. Tests pins 8–39.",
    "Test Steps / Procedure": "1) Enable GPIO interrupt output in the interrupt controller.\n2) Route the GPIO interrupt via the system interrupt enable register.\n3) Set the external pad driver at 0xA0243ffc high.\n4) For each pin 8–39, configure input mode, enable negative-edge interrupt, and clear raw status using the per-pin control register series starting at gp0_gpio_8.\n5) For each pin, pre-clear raw status in gpio_intr_raw_stclr1, enable the specific bit in gp0_intr1_intr_en1, arm wait, then generate a falling edge via 0xA0243ffc.\n6) Wait for the interrupt to arrive, with a timeout; record an error if it does not arrive in time.\n7) In the handler, verify per-pin DIN is low and the group status bit is set in gp0_intr1_intr_sts1, clear both per-pin raw and group raw, verify the group status is cleared, clear the system raw status, and clear the GIC IRQ.",
    "Impacted Registers": "intr_en1, gp0_gpio_8, gpio_intr_raw_stclr1, gp0_intr1_intr_en1, gp0_intr1_intr_sts1, raw_stcr1",
    "Validation / Acceptance Criteria": "- Interrupt arrival: For each pin, the handler must be invoked before timeout; otherwise, the test fails for that pin.\n- Per-pin checks: DIN must be low after the falling edge and the group status must indicate the pin; both must be cleared successfully afterward.\n- System interrupt: The system raw status must be cleared after servicing the interrupt.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
    "Hidden_Test_Description": "Verifies negative-edge interrupt generation for GPIOs 8–39, including raw status set/clear and group status handling with ISR.",
    "Hidden_Remarks": "Uses system interrupt routing and GIC IRQs (87/88) depending on configuration. Uses a bounded wait to avoid infinite loops. Tests pins 8–39.",
    "Hidden_Test_Steps_Procedure": "1) test_err = 0.\n2) If GPIO0 defined: GIC_EnableIRQ(87). If GPIO1 defined: GIC_EnableIRQ(88).\n3) If GPIO0 defined: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR). If GPIO1 defined: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n4) write_reg(0xA0243ffc, 0xffffffff) to drive all high (known state).\n5) Phase 1 per-pin configuration (i=0..31):\n   5.1) addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n   5.2) write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16)) to set doe=1 (input), neie=1, iclr=1.\n   5.3) wait_on(10).\n6) Phase 2 per-pin interrupt generation (i=0..31):\n   6.1) wr_val = 1u << i.\n   6.2) write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) to clear any latched raw.\n   6.3) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) to enable only this bit.\n   6.4) wait_on(10).\n   6.5) int_pend = 1.\n   6.6) Create falling edge: write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val).\n   6.7) timeout = 5000; while (int_pend && timeout--) { wait_on(10); }.\n   6.8) If timeout == 0: printf timeout error for pin (i+8); test_err++.\n7) finish(test_err).\n8) Default_IRQHandler():\n   8.1) local_wr = 1u << i.\n   8.2) int_pend = 0.\n   8.3) write_reg(0xA0243ffc, 0xffffffff) to return to known state.\n   8.4) raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4); rdata = read_reg(raddr).\n   8.5) If ((rdata & 0x1) != 0): test_err++ (DIN not low).\n   8.6) If ((rdata & 0x2) != 0x0) then:\n        a) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n        b) If ((rdata_grp & local_wr) == 0): test_err++.\n        c) raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4); write_reg(raddr2, (1u << 20) | (1u << 16)) to clear per-pin raw while keeping input mode.\n        d) write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr) to clear group raw.\n        e) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0): test_err++.\n        f) If GPIO0 defined: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87).\n           If GPIO1 defined: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88).\n      Else (raw not set): test_err++.",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "Timeout must not occur while waiting for interrupt per pin; else test_err++.\nIn ISR: (rdata & 0x1) must be 0 (DIN low); (rdata & 0x2) must be non-zero (raw set) else test_err++.\nGroup status must indicate the pin, then clear per-pin raw and group raw, and verify gp0_intr1_intr_sts1 == 0.\nSystem raw status must be cleared via MIZAR_LSS_SYSREG_RAW_STCR1.\nFinish with finish(test_err)."
  },
  {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "peie",
    "Test Case Name": "test_gpio_pedge_all_pads_en",
    "Test Description": "Verifies positive-edge interrupt generation on GPIOs 8–39 with group enable, raw status clear, and system/GIC acknowledgement.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Interrupt enable routing through system register; masks/unmasks group enable around ISR service. Uses a timeout to prevent infinite waits.",
    "Test Steps / Procedure": "1) Enable the platform interrupt for the GPIO block.\n2) Route the GPIO interrupt using the system interrupt enable register.\n3) For each pin 8–39, enable positive-edge detection using the per-pin control register series starting at gp0_gpio_8.\n4) Put pins in input mode via gpio_io_ctrl_group1..4.\n5) Enable all group interrupts in gp0_intr1_intr_en1.\n6) For each pin, drive the pad low, arm the wait, generate a rising edge at 0xA0243ffc, and wait for the interrupt with a timeout.\n7) In the handler, read gp0_intr1_intr_sts1, mask group enable, clear each per-pin raw status via per-pin control (iclr), verify group status cleared, clear system raw status, re-enable group interrupt, and clear the GIC IRQ.",
    "Impacted Registers": "intr_en1, gp0_gpio_8, gpio_io_ctrl_group1, gpio_io_ctrl_group2, gpio_io_ctrl_group3, gpio_io_ctrl_group4, gp0_intr1_intr_en1, gp0_intr1_intr_sts1, raw_stcr1",
    "Validation / Acceptance Criteria": "- Each pin must generate a rising-edge interrupt before the timeout; otherwise, the test fails for that pin.\n- After servicing, the group interrupt status must be cleared and the system raw status must report cleared.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
    "Hidden_Test_Description": "Verifies positive-edge interrupt generation on GPIOs 8–39 with group enable, raw status clear, and system/GIC acknowledgement.",
    "Hidden_Remarks": "Interrupt enable routing through system register; masks/unmasks group enable around ISR service. Uses a timeout to prevent infinite waits.",
    "Hidden_Test_Steps_Procedure": "1) If GPIO0 defined: GIC_EnableIRQ(87); If GPIO1 defined: GIC_EnableIRQ(88).\n2) test_err = 0.\n3) If GPIO0 defined: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR). If GPIO1 defined: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n4) For i=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000) to enable positive-edge (peie=1) per pin.\n5) wait_on(10).\n6) Set input mode by groups: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF).\n7) wait_on(10).\n8) Enable group interrupts: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).\n9) For i=0..31:\n   9.1) write_reg(0xA0243ffc, 0x00000000) to drive low; wait_on(10).\n   9.2) int_pend = 1.\n   9.3) write_reg(0xA0243ffc, 0xFFFFFFFF) to generate rising edge.\n   9.4) timeout = 2000; while (int_pend == 1 && --timeout > 0) wait_on(10).\n   9.5) If timeout == 0: printf timeout error; test_err++; break.\n   9.6) write_reg(0xA0243ffc, 0x00000000); wait_on(10).\n10) finish(test_err).\n11) Default_IRQHandler():\n   11.1) wr_val = 1 << i; int_pend = 0.\n   11.2) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n   11.3) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) to mask group during service.\n   11.4) If ((rdata_grp & 0xffffffff) != 0): success path; else: printf error; test_err++.\n   11.5) For j=0..31: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000) to clear per-pin raw (iclr=1).\n   11.6) wait_on(2).\n   11.7) rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0): success; else: printf error and test_err++.\n   11.8) If GPIO0: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if (bit not cleared) test_err++.\n         If GPIO1: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if (bit not cleared) test_err++.\n   11.9) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF) to re-enable group interrupt.\n   11.10) If GPIO0: GIC_ClearIRQ(87). If GPIO1: GIC_ClearIRQ(88).",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "Each pin must trigger an interrupt before timeout; else test_err++.\nAfter ISR executes, gp0_intr1_intr_sts1 must read 0; otherwise test_err++.\nSystem raw status readback must show the respective bit cleared after writing RAW_STCR1; otherwise test_err++.\nFinish with finish(test_err)."
  }
]

# ---------------------- Configuration ----------------------
IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", IP_NAME, "TestPlan")
TIMEZONE = ZoneInfo("Asia/Kolkata")
FILENAME = f"{IP_NAME}_TestPlan_{datetime.now(TIMEZONE).strftime('%Y%m%d_%H%M%S')}.xlsx"
OUTPUT_PATH = os.path.join(OUTPUT_DIR, FILENAME)

# META and MAIN column definitions
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

WRAP_COLS = {
    "Test Description",
    "Remarks",
    "Test Steps / Procedure",
    "Validation / Acceptance Criteria",
}

# ---------------------- Utility ----------------------
def normalize_schema(rows):
    # Determine union of keys preserving first seen order
    seen = []
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.append(k)
    # Normalize rows: ensure all keys exist
    norm = []
    for r in rows:
        norm.append({k: r.get(k, "") for k in seen})
    return seen, norm


def ensure_numbered(text: str) -> str:
    if not text:
        return text
    # Split on newlines; produce 1., 2., 3. prefixes deterministically
    lines = [ln.strip() for ln in str(text).splitlines() if ln.strip()]
    if not lines:
        return ""
    numbered = []
    for i, ln in enumerate(lines, start=1):
        # Remove any existing leading numbering/bullets and re-apply numeric
        # This keeps content but enforces normalized numbering
        cleaned = ln
        numbered.append(f"{i}. {cleaned}")
    return "\n".join(numbered)


def autosize_columns(ws):
    # Approximate width by max char length per column
    for col in range(1, ws.max_column + 1):
        max_len = 0
        col_letter = get_column_letter(col)
        for row in range(1, ws.max_row + 1):
            val = ws.cell(row=row, column=col).value
            if val is None:
                continue
            l = len(str(val))
            if l > max_len:
                max_len = l
        ws.column_dimensions[col_letter].width = min(80, max(10, max_len + 2))


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


# ---------------------- Main generation ----------------------

def main():
    # Validate input JSON as list
    if not isinstance(TESTPLAN_JSON, list) or len(TESTPLAN_JSON) == 0:
        raise SystemExit("Invalid or empty JSON data for Test Plan")

    # Normalize schema
    all_keys, data_rows = normalize_schema(TESTPLAN_JSON)

    # Create workbook
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"  # staging authoritative sheet

    # Write header
    for c, key in enumerate(all_keys, start=1):
        ws.cell(row=1, column=c, value=key)
    # Write rows
    for r_idx, row in enumerate(data_rows, start=2):
        for c, key in enumerate(all_keys, start=1):
            ws.cell(row=r_idx, column=c, value=row.get(key, ""))

    # Header formatting and freeze top row
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill("solid", fgColor="4472C4")  # blue fill
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=False)
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align
    ws.freeze_panes = "A2"

    autosize_columns(ws)

    # Create META sheet and copy META columns as-is
    meta_ws = wb.create_sheet(title="Meta_data_sheet")
    # Write META headers
    for c, key in enumerate(META_COLS, start=1):
        meta_ws.cell(row=1, column=c, value=key)
    # Map from all_keys to column index
    key_to_idx = {k: i + 1 for i, k in enumerate(all_keys)}
    for r in range(2, ws.max_row + 1):
        for c, key in enumerate(META_COLS, start=1):
            src_col = key_to_idx.get(key)
            val = ws.cell(row=r, column=src_col).value if src_col else ""
            meta_ws.cell(row=r, column=c, value=val)
    # Very hidden
    meta_ws.sheet_state = 'veryHidden'

    # Now operate IN-PLACE on the main sheet: rename to TestPlan
    ws.title = "TestPlan"

    # Remove META columns from TestPlan
    # Compute indices of META columns in current order
    meta_idx = [key_to_idx.get(k) for k in META_COLS if k in key_to_idx]
    # Delete from rightmost to leftmost
    for idx in sorted([i for i in meta_idx if i], reverse=True):
        ws.delete_cols(idx, 1)

    # Rebuild header and data in MAIN_ORDER
    # Build current header list after deletions
    current_headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    # Map header -> index
    cur_map = {h: i for i, h in enumerate(current_headers)}

    # Build new matrix with MAIN_ORDER columns; missing columns become blank
    rows_data = []
    for r in range(2, ws.max_row + 1):
        row_dict = {h: ws.cell(row=r, column=cur_map[h] + 1).value if h in cur_map else "" for h in current_headers}
        rows_data.append(row_dict)

    # Clear sheet and write MAIN_ORDER
    ws.delete_rows(1, ws.max_row)
    for c, key in enumerate(MAIN_ORDER, start=1):
        ws.cell(row=1, column=c, value=key)
    for r_idx, row_dict in enumerate(rows_data, start=2):
        for c, key in enumerate(MAIN_ORDER, start=1):
            val = row_dict.get(key, "")
            # Enforce numbering for specific columns
            if key in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
                val = ensure_numbered(val)
            ws.cell(row=r_idx, column=c, value=val)

    # Apply formatting to TestPlan
    # Header styling already set earlier, re-apply to the new header
    for c in range(1, ws.max_column + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align

    # Data alignments
    for r in range(2, ws.max_row + 1):
        for c in range(1, ws.max_column + 1):
            key = ws.cell(row=1, column=c).value
            align = Alignment(vertical="top", wrap_text=(key in WRAP_COLS), horizontal=("center" if key == "Index" else "left"))
            ws.cell(row=r, column=c).alignment = align

    autosize_columns(ws)

    # Apply thin borders
    apply_borders(ws)

    # Data Validation on Code Generation (Required / Not)
    # Find the column index
    headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
    try:
        codegen_col = headers.index("Code Generation (Required / Not)") + 1
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        rng = f"{get_column_letter(codegen_col)}2:{get_column_letter(codegen_col)}{ws.max_row}"
        dv.add(rng)
    except ValueError:
        pass  # Column not found; skip

    # Safety check: no sheet named "Data"
    if any(sh.title == "Data" for sh in wb.worksheets):
        # Try to remove; if cannot, fail
        try:
            data_sheet = next(sh for sh in wb.worksheets if sh.title == "Data")
            wb.remove(data_sheet)
        except Exception as e:
            raise SystemExit(f"Validation failed: residual 'Data' sheet could not be removed: {e}")

    # Ensure only allowed sheets exist
    allowed = {"TestPlan", "Meta_data_sheet"}
    for sh in wb.worksheets:
        if sh.title not in allowed:
            raise SystemExit(f"Validation failed: unexpected sheet '{sh.title}' present")

    # Make sure directories exist
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(OUTPUT_PATH)

    # Validate XLSX as ZIP-based OOXML
    if not zipfile.is_zipfile(OUTPUT_PATH):
        raise SystemExit("XLSX validation failed: not a ZIP-based file")
    with zipfile.ZipFile(OUTPUT_PATH, 'r') as zf:
        if "[Content_Types].xml" not in zf.namelist():
            raise SystemExit("XLSX validation failed: missing [Content_Types].xml")

    print(f"Generated: {OUTPUT_PATH}")


if __name__ == "__main__":
    main()
