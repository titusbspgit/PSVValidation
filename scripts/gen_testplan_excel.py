#!/usr/bin/env python3
import json
import os
import re
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter
import zipfile

# ------------ Input JSON (embedded) ------------
TESTPLAN_JSON_STR = r'''{
  "TC1": {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "GPIO Register Access and Default Values",
    "Test Case Name": "gpio_reg_wr_rd_test/",
    "Test Description": "Validates default register values and masked read/write behavior for GPIO registers using predefined patterns.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Default-value comparison masks off bit 0 before compare. Certain registers are explicitly skipped for default or R/W checks per skip arrays; VRRW registers are skipped.",
    "Test Steps / Procedure": "1) Initialize and read default values for all targeted GPIO registers. 2) For each test pattern, write masked data to writable registers and then read back masked data. 3) Compare read data against expected value derived from mask and default values. 4) Report failure if any mismatch occurs; otherwise pass.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "PASS if all default-value reads match expected defaults (with bit 0 masked) and all masked read-backs match computed expected values across all patterns and addresses; otherwise FAIL.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test/",
    "Hidden_Test_Description": "This test checks default register values and verifies masked write/read behavior across GPIO address list using six data patterns.",
    "Hidden_Remarks": "Default-value check uses (data_rd & 0xfffffffe) before comparison. skip_array and skip_rst_array control which registers are excluded from write/read or reset checks. Comment notes VRRW registers are skipped and that forcing DIN can affect level-select during default reads.",
    "Hidden_Test_Steps_Procedure": "Entry Points: 1) test_case()\nRuntime Trace (in order):\n- Step 1: test_case() calls chk_rst_val().\n  - Step 1.1: For i = 0..48 (CNT=49):\n    - Load addr = addr_array[i] where addr_array contains register macros: {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4}.\n    - If skip_rst_array[i] == 1: continue (skip default check for this address).\n    - Else if read_mask_array[i] == 0x00000000: continue (non-readable address skipped).\n    - Else: READ data_rd = read_reg(addr). Compute data = (data_rd & 0xfffffffe). Compare: if (data == default_value_array[i]) then pass else def_fail_cnt++ and print failure with addr, expected, and read values.\n- Step 2: Return to test_case(); call chk_rd_wr().\n  - Step 2.1: Define chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}.\n  - Step 2.2: For j = 0..5:\n    - Set data_wr = chk_val[j].\n    - Step 2.2.1 (WRITE phase): For i = 0..48:\n      - addr = addr_array[i]. If skip_array[i] == 1: continue. If write_mask_array[i] == 0x00000000: continue. Else: WRITE write_reg(addr, (data_wr & write_mask_array[i])).\n    - Step 2.2.2 (READ/COMPARE phase): For i = 0..48:\n      - addr = addr_array[i]. If skip_array[i] == 1: continue. If write_mask_array[i] == 0x00000000: continue. If read_mask_array[i] == 0x00000000: continue. Else:\n        - READ data_rd = (read_reg(addr) & read_mask_array[i]).\n        - Compute wr_n = (write_mask_array[i] ^ 0xffffffff).\n        - Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]) ).\n        - If (data_rd == exp_val) pass; else wr_fail_cnt++ and print mismatch with addr, expected, read values.\n- Step 3: Return to test_case(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).\nTiming:\n- No explicit wait_on() calls in the executed paths of this test.\nRegister Access Summary per operation:\n- READ: read_reg(addr) where addr iterates over addr_array entries (macros listed above) filtered by read_mask_array and skip_rst_array/skip_array.\n- WRITE: write_reg(addr, masked_data) where addr iterates over addr_array entries subject to write_mask_array and skip_array.\n",
    "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
    "Hidden_Validation_Acceptance_Criteria": "Default-value phase: For each addr not skipped and readable, (read_reg(addr) & 0xfffffffe) must equal default_value_array[i]; else def_fail_cnt++. Write/Read phase for each pattern: For each addr not skipped and with nonzero write/read masks, (read_reg(addr) & read_mask_array[i]) must equal ((data_wr & read_mask & write_mask) | ((~write_mask) & read_mask & default_value)); else wr_fail_cnt++. Final: finish(0) if def_fail_cnt==0 and wr_fail_cnt==0; else finish(1)."
  },
  "TC2": {
    "Index": "2",
    "SS / Module": "GPIO",
    "Feature": "GPIO Interrupt - Falling Edge",
    "Test Case Name": "test_gpio_negedge_intr_en/",
    "Test Description": "Configures GPIO pins for falling-edge interrupts and verifies interrupt assertion, status reporting, and clear operations.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "A bounded wait with timeout=5000 is used to avoid infinite waits. Interrupt wait is armed before generating the falling edge to avoid race. All pads are driven high initially to a known state.",
    "Test Steps / Procedure": "1) Enable the relevant GPIO interrupt in the system register and unmask the CPU interrupt. 2) Configure pins 8–39 for input, falling-edge detection, and clear any raw status. 3) For each pin, clear group raw status, enable only that pin’s interrupt, and generate a falling edge. 4) Wait for the interrupt to assert within the timeout. 5) In the handler, verify the pin input is low, confirm group status for the pin, clear per‑pin raw and group raw status, then clear the system interrupt status and CPU interrupt.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "Interrupt must arrive before timeout for each pin. After the falling edge, the pin input must read low and the group status must indicate the correct pin. After clearing per‑pin raw and group status, the group status must read zero, and the system interrupt status must be cleared.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en/",
    "Hidden_Test_Description": "This test enables negedge interrupts on GPIO pins 8..39, generates a single falling edge per pin, waits with timeout for an ISR handshake (int_pend), and validates pin-level DIN, group status, and raw-status clear sequences.",
    "Hidden_Remarks": "Timeout used is 5000 loop iterations with wait_on(10) per iteration. The wait is armed (int_pend=1) before toggling the pad to avoid a race. Pads are pre-driven high (0xffffffff) via address 0xA0243ffc.",
    "Hidden_Test_Steps_Procedure": "Entry Points and Order: 1) test_case() 2) Default_IRQHandler() (upon interrupt)\nRuntime Trace (in order):\n- Step 1: test_case() initializes test_err = 0.\n- Step 2: If GPIO0 is defined: CALL GIC_EnableIRQ(87). If GPIO1 is defined: CALL GIC_EnableIRQ(88).\n- Step 3: Enable system-level interrupt:\n  - If GPIO0: WRITE write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR).\n  - If GPIO1: WRITE write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n- Step 4: Initialize pad driver to known high: WRITE write_reg(0xA0243ffc, 0xffffffff).\n- Step 5: Configure per-pin input + negedge + clear raw for GPIO[8..39]:\n  - For i = 0..31:\n    - Compute addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4)).\n    - WRITE write_reg(addr1, ((1u<<20) | (1u<<18) | (1u<<16)))  # doe=1 (input), neie=1, iclr=1.\n    - CALL wait_on(10).\n- Step 6: For each bit i = 0..31, generate falling edge and wait for ISR:\n  - Compute wr_val = (1u << i).\n  - WRITE write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val)  # clear group raw bit.\n  - WRITE write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val)    # enable only this bit.\n  - CALL wait_on(10).\n  - Set int_pend = 1  # arm before edge.\n  - WRITE write_reg(0xA0243ffc, 0xffffffff)  # drive all high.\n  - CALL wait_on(30).\n  - WRITE write_reg(0xA0243ffc, ~wr_val)     # create falling edge on bit i.\n  - Set timeout = 5000.\n  - while int_pend and timeout > 0:\n      timeout -= 1\n  - if timeout == 0: pass  # timeout handling occurs in actual environment\n- ISR Path: Default_IRQHandler():\n  - Pseudo path retained in META only.\n",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin i in 8..39: ISR must occur before timeout (int_pend cleared). After ISR entry, (read_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4) & 0x1) must be 0 (DIN low for negedge). Group status read (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must have bit (1<<i) set on entry and must read 0 after clearing per-pin (iclr) and group raw (RAW_STCLR1). System register RAW_STCR1 must be cleared after write. test_err remains 0 for PASS; any violation increments test_err leading to FAIL."
  },
  "TC3": {
    "Index": "3",
    "SS / Module": "GPIO",
    "Feature": "GPIO Interrupt - Rising Edge (All Pads)",
    "Test Case Name": "test_gpio_pedge_all_pads_en/",
    "Test Description": "Enables rising-edge interrupts on all GPIO pads and verifies group interrupt, status clear, and system interrupt clear operations.",
    "Speed": "NA",
    "Mode": "Interrupt",
    "Memory Start Offset": "0xA0243ffc",
    "Memory End Offset": "0xA0243ffc",
    "Remarks": "Interrupt wait uses a bounded timeout loop. Group interrupt is masked during service and re-enabled afterward. The ISR clears per-pin raw status for all pins before verifying group status is zero.",
    "Test Steps / Procedure": "1) Enable the GPIO interrupt in the system register and unmask the CPU interrupt. 2) Configure pins 8–39 for rising‑edge detection and set them as inputs. 3) Enable the group interrupt. 4) For each pin, drive the pad low, arm the wait, then generate a rising edge and wait for the interrupt. 5) In the handler, mask group interrupt, confirm group status is non‑zero, clear per‑pin raw status for all pins, verify group status is zero, clear the system interrupt status, then re‑enable group interrupt and clear CPU interrupt.",
    "Impacted Registers": "",
    "Validation / Acceptance Criteria": "For each pin, an interrupt must be observed before timeout. Group status must indicate an interrupt occurred, and after clearing per‑pin status the group status must be zero. The system interrupt status must be cleared before exiting the handler.",
    "Code Generation (Required / Not)": "",
    "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en/",
    "Hidden_Test_Description": "This test enables posedge interrupts for GPIO[8..39], toggles the pads to create a rising edge one by one, waits for ISR via volatile int_pend, and validates group status, per-pin clear, system clear, and proper masking/unmasking behavior.",
    "Hidden_Remarks": "The wait loop uses a timeout of 2000 iterations with wait_on(10). During ISR, group interrupt enable is written to 0 to mask, then restored to 0xFFFFFFFF after servicing. The int_pend variable is declared volatile for visibility between ISR and main loop. Pads are driven via address 0xA0243ffc.",
    "Hidden_Test_Steps_Procedure": "Entry Points and Order: 1) test_case() 2) Default_IRQHandler()\nRuntime Trace (in order):\n- Step 1: test_case() start; conditionally enable CPU interrupt: If GPIO0: GIC_EnableIRQ(87); If GPIO1: GIC_EnableIRQ(88).\n- Step 2: Enable system-level GPIO interrupt:\n  - If GPIO0: WRITE write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR).\n  - If GPIO1: WRITE write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR).\n- Step 3: Configure posedge on GPIO[8..39]: For i=0..31: WRITE write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000)  # posedge enable bit.\n- Step 4: CALL wait_on(10).\n- Step 5: Configure input mode via group IO control: WRITE write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); WRITE write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); WRITE write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); WRITE write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF).\n- Step 6: CALL wait_on(10).\n- Step 7: Enable group interrupts: WRITE write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).\n- Step 8: For i=0..31 do per-pin stimulus and wait:\n  - WRITE write_reg(0xA0243ffc, 0x00000000)  # drive low baseline.\n  - CALL wait_on(10).\n  - Set int_pend = 1.\n  - WRITE write_reg(0xA0243ffc, 0xFFFFFFFF)  # rising edge on bit i.\n  - Initialize timeout = 2000.\n  - while int_pend and timeout > 0:\n      timeout -= 1\n  - if timeout == 0: pass\n- ISR: Default_IRQHandler():\n  - Pseudo path retained in META only.\n",
    "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
    "Hidden_Validation_Acceptance_Criteria": "For each pin 8..39: an interrupt must be observed before timeout (int_pend cleared). On ISR entry, group status (MIZAR_GPIO_GP0_INTR1_INTR_STS1) must be non-zero (indicating an interrupt). After writing iclr to all pins, group status must read 0. System RAW_STCR1 must be cleared after write (confirmed by read). Final result PASS if test_err == 0; any increment constitutes FAIL."
  }
}'''

# ------------ Constants from task ------------
IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
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

# ------------ Helpers ------------
def normalize_json_to_array(obj):
    if isinstance(obj, list):
        rows = obj
    elif isinstance(obj, dict):
        rows = list(obj.values())
    else:
        raise ValueError("Unsupported json_data structure")
    # Sort rows by numeric Index if present
    def idx(v):
        try:
            return int(str(v.get("Index", "0")).strip())
        except Exception:
            return 0
    rows.sort(key=idx)
    # Build first-seen key order
    key_order = []
    seen = set()
    for r in rows:
        for k in r.keys():
            if k not in seen:
                seen.add(k)
                key_order.append(k)
    # Ensure META columns exist in key_order (append if missing, preserve given order)
    for m in META_COLS:
        if m not in key_order:
            key_order.append(m)
    # Fill blanks where keys missing
    norm_rows = []
    for r in rows:
        rr = {}
        for k in key_order:
            rr[k] = r.get(k, "")
        norm_rows.append(rr)
    return key_order, norm_rows


def estimate_col_width(values):
    max_len = 0
    for v in values:
        s = "" if v is None else str(v)
        # wider for multi-line
        s = s.replace("\t", "    ")
        for line in s.split("\n"):
            if len(line) > max_len:
                max_len = len(line)
    # approximate width factor
    return min(max(10, max_len + 2), 80)


def normalize_numbering(text):
    if not isinstance(text, str) or not text.strip():
        return text
    # If already contains line breaks, re-number per line
    lines = [l.strip() for l in text.split("\n") if l.strip()]
    if len(lines) > 1:
        out = []
        for i, l in enumerate(lines, 1):
            # remove any leading numbering or bullets
            l2 = re.sub(r"^(\d+)[\.)]\s*", "", l)
            l2 = re.sub(r"^[-•]\s*", "", l2)
            out.append(f"{i}. {l2}")
        return "\n".join(out)
    # Single-line with inline steps like '1) ... 2) ...'
    parts = re.split(r"\s*(\d+)[\)]\s*", text)
    # parts pattern yields text before first number, then pairs of number, segment...
    assembled = []
    i = 1
    tmp = []
    # Find segments in order
    tokens = re.split(r"\s*\d+[\).]\s*", text)
    nums = re.findall(r"\d+[\).]", text)
    if len(tokens) >= 2 and len(nums) >= 1:
        steps = []
        for seg in tokens:
            if seg.strip():
                steps.append(seg.strip())
        out = []
        for i, seg in enumerate(steps, 1):
            out.append(f"{i}. {seg}")
        return "\n".join(out)
    # fallback: replace ) with .
    return re.sub(r"(\d+)\)", r"\1.", text)


def add_borders(ws, min_row, max_row, min_col, max_col):
    thin = Side(style='thin', color='000000')
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(min_row, max_row + 1):
        for c in range(min_col, max_col + 1):
            ws.cell(row=r, column=c).border = border


def set_wrap_for_cols(ws, col_names, header_map, total_rows):
    wrap = Alignment(wrap_text=True, vertical='top', horizontal='left')
    for name in col_names:
        cidx = header_map.get(name)
        if cidx is None:
            continue
        for r in range(2, total_rows + 1):
            ws.cell(row=r, column=cidx).alignment = wrap


def autosize(ws):
    for col in range(1, ws.max_column + 1):
        col_letter = get_column_letter(col)
        values = [ws.cell(row=1, column=col).value]
        for r in range(2, ws.max_row + 1):
            values.append(ws.cell(row=r, column=col).value)
        ws.column_dimensions[col_letter].width = estimate_col_width(values)


def adjust_row_heights(ws):
    base_height = 15
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, ws.max_column + 1):
            v = ws.cell(row=r, column=c).value
            s = "" if v is None else str(v)
            lines = s.count("\n") + 1
            if lines > max_lines:
                max_lines = lines
        ws.row_dimensions[r].height = base_height * max_lines


# ------------ Main ------------
try:
    data_obj = json.loads(TESTPLAN_JSON_STR)
except Exception as e:
    raise SystemExit(f"Invalid JSON input: {e}")

key_order, rows = normalize_json_to_array(data_obj)

# Create workbook and Data sheet
wb = Workbook()
ws = wb.active
ws.title = "Data"

# Write headers
for col, key in enumerate(key_order, start=1):
    ws.cell(row=1, column=col, value=key)

# Write data
for r_idx, row in enumerate(rows, start=2):
    for c_idx, key in enumerate(key_order, start=1):
        ws.cell(row=r_idx, column=c_idx, value=row.get(key, ""))

# Base formatting
ws.freeze_panes = 'A2'
for c in range(1, ws.max_column + 1):
    cell = ws.cell(row=1, column=c)
    cell.font = Font(bold=True, color='FFFFFFFF')
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.fill = PatternFill("solid", fgColor="4472C4")

# Create META sheet and copy META columns (AS-IS)
meta = wb.create_sheet("Meta_data_sheet")
for col, key in enumerate(META_COLS, start=1):
    meta.cell(row=1, column=col, value=key)
for r_idx, row in enumerate(rows, start=2):
    for c_idx, key in enumerate(META_COLS, start=1):
        meta.cell(row=r_idx, column=c_idx, value=row.get(key, ""))
# Set very hidden
meta.sheet_state = 'veryHidden'

# Rename Data -> TestPlan and transform in-place
ws.title = "TestPlan"

# Remove META columns from visible sheet
meta_indices = [i for i, k in enumerate(key_order, start=1) if k in META_COLS]
for i in sorted(meta_indices, reverse=True):
    ws.delete_cols(i)

# Rebuild header and rows to MAIN_ORDER only
# First, map from current headers
current_headers = [ws.cell(row=1, column=c).value for c in range(1, ws.max_column + 1)]
# Clear existing content (delete all columns)
ws.delete_cols(1, ws.max_column)

# Write MAIN headers
for c, h in enumerate(MAIN_ORDER, start=1):
    ws.cell(row=1, column=c, value=h)

# Map each data row from original 'rows'
for r_idx, src in enumerate(rows, start=2):
    for c, h in enumerate(MAIN_ORDER, start=1):
        val = src.get(h, "")
        # Numbering normalization only for visible sheet, specific columns
        if h in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
            val = normalize_numbering(val)
        ws.cell(row=r_idx, column=c, value=val)

# Re-apply header formatting and freeze
ws.freeze_panes = 'A2'
for c in range(1, ws.max_column + 1):
    cell = ws.cell(row=1, column=c)
    cell.font = Font(bold=True, color='FFFFFFFF')
    cell.alignment = Alignment(horizontal='center', vertical='center')
    cell.fill = PatternFill("solid", fgColor="4472C4")

# Data row alignment: top, text-left; Index centered
for r in range(2, ws.max_row + 1):
    for c in range(1, ws.max_column + 1):
        align = Alignment(vertical='top', horizontal='left', wrap_text=False)
        ws.cell(row=r, column=c).alignment = align
# Wrap text for specific columns
header_map = {ws.cell(row=1, column=i).value: i for i in range(1, ws.max_column + 1)}
set_wrap_for_cols(ws, ["Test Description", "Remarks", "Test Steps / Procedure", "Validation / Acceptance Criteria"], header_map, ws.max_row)
# Center Index column
idx_col = header_map.get("Index")
if idx_col:
    for r in range(2, ws.max_row + 1):
        ws.cell(row=r, column=idx_col).alignment = Alignment(vertical='top', horizontal='center')

# Borders
add_borders(ws, 1, ws.max_row, 1, ws.max_column)

# Autosize and row heights
autosize(ws)
adjust_row_heights(ws)

# Data validation for Code Generation (Required / Not)
code_col = header_map.get("Code Generation (Required / Not)")
if code_col:
    code_letter = get_column_letter(code_col)
    dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
    dv.error = 'Invalid selection. Choose one of: Required, Blank, Not Required.'
    dv.prompt = 'Select value for Code Generation requirement.'
    ws.add_data_validation(dv)
    dv.add(f"{code_letter}2:{code_letter}{ws.max_row}")

# Safety check: ensure no sheet named 'Data' remains
if 'Data' in [s.title for s in wb.worksheets]:
    # try delete if exists
    for s in wb.worksheets:
        if s.title == 'Data':
            wb.remove(s)

# Ensure only allowed sheets exist
allowed = {"TestPlan", "Meta_data_sheet"}
for s in list(wb.worksheets):
    if s.title not in allowed:
        wb.remove(s)

# Compute IST timestamp for filename
ist = ZoneInfo("Asia/Kolkata")
now = datetime.now(ist)
file_ts = now.strftime('%Y%m%d_%H%M%S')
filename = f"{IP_NAME}_TestPlan_{file_ts}.xlsx"
output_path = os.path.join(OUTPUT_DIR, filename)

# Ensure directory exists
os.makedirs(OUTPUT_DIR, exist_ok=True)

# Save workbook
wb.save(output_path)

# Validate OOXML zip structure
with zipfile.ZipFile(output_path, 'r') as zf:
    names = set(zf.namelist())
    if '[Content_Types].xml' not in names or 'xl/workbook.xml' not in names:
        raise SystemExit('XLSX validation failed: missing core OOXML parts')

# Emit outputs for the workflow
commit_msg = f"Add {IP_NAME} TestPlan Excel generated on {now.strftime('%Y-%m-%d %H:%M:%S')} IST"
outputs_path = os.environ.get('GITHUB_OUTPUT')
if outputs_path:
    with open(outputs_path, 'a', encoding='utf-8') as f:
        print(f"excel_path={output_path}", file=f)
        print(f"commit_msg={commit_msg}", file=f)

print(f"Generated: {output_path}")
