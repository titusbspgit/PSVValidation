#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Deterministic generator for GPIO TestPlan Excel (.xlsx) from embedded JSON.
- Generates true binary XLSX using openpyxl
- Applies schema-preserving transformations and formatting
- Creates TestPlan (visible) and Meta_data_sheet (Very Hidden)
- Enforces data validation only on 'Code Generation (Required / Not)'
- Validates OOXML structure before exit
"""
import os
import sys
import json
import zipfile
from collections import OrderedDict
from datetime import datetime
try:
    from zoneinfo import ZoneInfo
except Exception:
    from backports.zoneinfo import ZoneInfo  # type: ignore

from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# -------------------------
# Embedded JSON as array
# -------------------------
DATA = [
    OrderedDict([
        ("Index", "1"),
        ("SS / Module", "gpio"),
        ("Feature", "GPIO negative-edge interrupt enable and servicing"),
        ("Test Case Name", "test_gpio_negedge_intr_en"),
        ("Test Description", "Verifies that negative-edge interrupts on GPIO pins 8–39 can be enabled, triggered, and correctly handled."),
        ("Speed", "NA"),
        ("Mode", "Interrupt"),
        ("Memory Start Offset", "0xA0243ffc"),
        ("Memory End Offset", "0xA0243ffc"),
        ("Remarks", "Timeout value is 5000 iterations and may need adjustment for the simulation time base. Assumes the raw interrupt clear register uses write‑1‑to‑clear behavior. All pads are driven high initially to establish a known state."),
        ("Test Steps / Procedure", "1) Enable the platform interrupt for the GPIO block in the interrupt controller and system controller. 2) Drive all GPIO outputs high to set a known baseline. 3) For pins 8–39, configure each pin as input, enable negative-edge detection, and clear any pending status. 4) For each pin, clear the corresponding group raw interrupt bit. 5) Enable the interrupt mask only for the current pin. 6) Arm the wait flag, generate a falling edge on the current pin, and wait for the interrupt with a timeout. 7) In the interrupt handler, restore the pad state to high, read the pin status, and verify the input reads low. 8) Read the group status and verify the bit for the current pin is set. 9) Clear the per-pin raw status and the group raw status, then read back to confirm the group status is cleared. 10) Clear the system-level raw status and the interrupt in the interrupt controller."),
        ("Imparted Registers", ""),
        ("Impacted Registers", ""),
        ("Validation / Acceptance Criteria", "- A falling edge on each tested pin causes an interrupt before the timeout; PASS if no timeout occurs for any pin.\n- The input value for the serviced pin is low when read in the handler; PASS if the read value is 0.\n- The group status bit for the serviced pin is set on entry to the handler; PASS if the bit is 1 prior to clears.\n- After clearing the per-pin and group raw status, the group status reads 0; PASS if the readback is 0 for the group."),
        ("Code Generation (Required / Not)", ""),
        ("Hidden_Test_Case_Name", "test_gpio_negedge_intr_en"),
        ("Hidden_Test_Description", "Test negative-edge GPIO interrupts for pins 8..39. Sequence: enable platform IRQ (GIC) and system interrupt for GPIO, set pads high, configure pins 8..39 as input with negedge and clear per-pin raw, then for each bit enable only that interrupt, generate a falling edge via 0xA0243ffc (drive all high then drive the specific bit low), wait for ISR with timeout, and in ISR verify DIN=0, per-pin raw (bit1) set, group status bit set, perform clears per-pin and group, verify group cleared, clear system RAW and GIC pending. Finish with test_err as pass/fail aggregator."),
        ("Hidden_Remarks", "1) The bounded wait uses timeout = 5000 and wait_on(10) per poll; comment notes it may need adjustment for simulation time base. 2) Assumes RAW_STCLR1 is write-1-to-clear. 3) All pads are driven high initially using address 0xA0243ffc. 4) The handler computes the current bit mask from the global loop index i."),
        ("Hidden_Test_Steps_Procedure", "Entry Points:\nA) test_case()\nB) Default_IRQHandler() [invoked by hardware on GPIO interrupt]\n\nRuntime Trace (in order):\n1. test_case(): Initialize test_err = 0.\n2. Conditional enable of GIC interrupt:\n - If GPIO0 is defined: call GIC_EnableIRQ(87).\n - If GPIO1 is defined: call GIC_EnableIRQ(88).\n3. Conditional enable of system-level interrupt for GPIO:\n - If GPIO0 is defined: WRITE MIZAR_LSS_SYSREG_INTR_EN1 <- LSS_SYSREG_INTR_EN1_GPIO0_INTR.\n - If GPIO1 is defined: WRITE MIZAR_LSS_SYSREG_INTR_EN1 <- LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n4. Set pad driver to a known state: WRITE 0xA0243ffc <- 0xffffffff (all high).\n\nPhase 1: Configure pins 8..39 for input + negedge, clear pending raw\n5. Loop entry: for (i = 0; i < 32; i++):\n 5.1 Loop body (per iteration i):\n - Compute addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n - WRITE addr1 <- ((1 << 20) | (1 << 18) | (1 << 16)) // doe=1 (input), neie=1, iclr=1.\n - Call wait_on(10).\n 5.2 Exit condition: i reaches 32.\n\nPhase 2: Per-pin enable, edge generation, and wait with timeout\n6. Loop entry: for (i = 0; i < 32; i++):\n 6.1 Set wr_val = (1u << i).\n 6.2 Pre-clear group raw status for this bit: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 <- wr_val.\n 6.3 Enable only this pin's interrupt: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 <- wr_val.\n 6.4 Call wait_on(10).\n 6.5 Prepare to wait for interrupt: int_pend = 1.\n 6.6 Generate a falling edge on bit i:\n - WRITE 0xA0243ffc <- 0xffffffff (ensure all high).\n - Call wait_on(30).\n - WRITE 0xA0243ffc <- bitwise_not(wr_val) (drive current bit low; others high).\n 6.7 Bounded wait for ISR to clear int_pend:\n - Initialize unsigned int timeout = 5000.\n - While (int_pend && timeout--): call wait_on(10) each iteration.\n - On exit: if (timeout == 0):\n - printf("ERROR: Timeout waiting for GPIO%u negedge interrupt", i + 8).\n - test_err++.\n 6.8 Continue loop to next i.\n 6.9 Exit condition: i reaches 32.\n7. Call finish(test_err).\n\nInterrupt Handler (invoked during step 6.7 for each pin that interrupts):\n8. Default_IRQHandler():\n 8.1 Local variables: rdata_grp, raddr, raddr2; compute local_wr = (1u << i).\n 8.2 Signal main loop to proceed: int_pend = 0.\n 8.3 Restore pad driver to known state: WRITE 0xA0243ffc <- 0xffffffff.\n 8.4 Compute raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n 8.5 READ rdata <- read_reg(raddr).\n 8.6 Check DIN value for falling edge: if ((rdata & 0x1) != 0) then test_err++.\n 8.7 Check per-pin raw interrupt bit (bit1 expected set):\n - If ((rdata & 0x2) != 0x0) then:\n a) READ rdata_grp <- read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n b) If ((rdata_grp & local_wr) == 0) then test_err++.\n c) Compute raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n d) Clear per-pin raw while keeping direction: WRITE raddr2 <- ((1 << 20) | (1 << 16)).\n e) Clear group raw bit: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 <- local_wr.\n f) Verify group clear: READ rdata_grp <- read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) then test_err++.\n g) Clear system raw and GIC pending:\n - If GPIO0 is defined: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 <- LSS_SYSREG_RAW_STCR1_GPIO0_INTR; call GIC_ClearIRQ(87).\n - If GPIO1 is defined: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 <- LSS_SYSREG_RAW_STCR1_GPIO1_INTR; call GIC_ClearIRQ(88).\n - Else (raw bit not set): test_err++.\n\nTiming Details:\n- wait_on(10) used after configuration and per iteration for bounded waits.\n- wait_on(30) used between setting all high and driving specific pin low to create a detectable falling edge.\n- Timeout counter initialized to 5000 for ISR wait loop; loop decrements once per wait_on(10) iteration.\n\nRegister Access Summary within execution:\n- WRITE MIZAR_LSS_SYSREG_INTR_EN1 (enable system-level GPIO interrupt).\n- WRITE 0xA0243ffc (pad drive control) multiple times to set/restore pin states.\n- WRITE MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to configure per-pin doe/neie/iclr and to clear per-pin raw.\n- WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 to clear group raw bit for selected pin.\n- WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 to enable interrupt mask for selected pin.\n- READ MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to sample DIN and raw bit.\n- READ/WRITE MIZAR_GPIO_GP0_INTR1_INTR_STS1 to verify/clear group status.\n- WRITE MIZAR_LSS_SYSREG_RAW_STCR1 to clear system-level raw status.\n- GIC enable/clear APIs used to manage platform interrupts."),
        ("Hidden_Impacted_Registers", "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1"),
        ("Hidden_Validation_Acceptance_Criteria", "1) No timeout during bounded wait for any i in 0..31. If timeout==0 for any pin, test_err++.\n2) In ISR, DIN bit (bit0) of per-pin register reads 0 after the falling edge; else test_err++.\n3) In ISR, per-pin raw bit (bit1) is set; else test_err++.\n4) Group interrupt status register has the bit for the current pin set; else test_err++.\n5) After clearing per-pin raw and group raw, the group status register reads 0; else test_err++.\n6) Final finish(test_err) reflects aggregated result: PASS if test_err==0; FAIL otherwise."),
    ])
]

# Constants
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


def fail(msg: str, code: int = 1):
    sys.stderr.write(msg + "\n")
    sys.exit(code)


def validate_json(data_list):
    if not isinstance(data_list, list):
        fail("Input JSON is not an array")
    if len(data_list) == 0:
        fail("Input JSON array is empty")


def get_header_order(data_list):
    seen = set()
    order = []
    for row in data_list:
        if not isinstance(row, dict):
            fail("JSON row is not an object")
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                order.append(k)
    return order


def normalize_rows(data_list, headers):
    out = []
    for row in data_list:
        out.append({k: row.get(k, "") for k in headers})
    return out


def renumber_multiline(value: str) -> str:
    if value is None:
        return ""
    s = str(value)
    s = s.replace("\r\n", "\n").replace("\r", "\n")
    raw_lines = [ln.strip() for ln in s.split("\n")]
    # Drop empty lines
    lines = [ln for ln in raw_lines if ln]
    if not lines:
        return ""
    out = []
    import re
    for i, ln in enumerate(lines, start=1):
        cleaned = ln
        # strip common bullet/number prefixes
        cleaned = re.sub(r"^[-*•–—]\s+", "", cleaned)
        cleaned = re.sub(r"^\s*\(?\d+\)?[\.)]\s*", "", cleaned)
        out.append(f"{i}. {cleaned}")
    return "\n".join(out)


def autofit_columns(ws, headers):
    for i, col in enumerate(headers, start=1):
        maxlen = len(str(col))
        for v in ws.iter_cols(min_col=i, max_col=i, min_row=2, max_row=ws.max_row, values_only=True):
            for cell_val in v:
                L = len(str(cell_val)) if cell_val is not None else 0
                if L > maxlen:
                    maxlen = L
        width = min(120, max(10, int(maxlen * 1.2 + 2)))
        ws.column_dimensions[get_column_letter(i)].width = width


def apply_formatting(ws, headers):
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    header_fill = PatternFill("solid", fgColor="1F4E78")  # blue

    # Header
    for c in range(1, len(headers) + 1):
        cell = ws.cell(row=1, column=c)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")
        cell.fill = header_fill
        cell.border = border

    # Data rows
    wrap_set = set(WRAP_COLUMNS)
    for r in range(2, ws.max_row + 1):
        max_lines = 1
        for c in range(1, len(headers) + 1):
            col_name = headers[c - 1]
            cell = ws.cell(row=r, column=c)
            if col_name in wrap_set:
                cell.alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")
                txt = "" if cell.value is None else str(cell.value)
                n = (txt.count("\n") + 1) if txt else 1
                if n > max_lines:
                    max_lines = n
            else:
                if col_name == "Index":
                    cell.alignment = Alignment(horizontal="center", vertical="top")
                else:
                    cell.alignment = Alignment(horizontal="left", vertical="top")
            cell.border = border
        ws.row_dimensions[r].height = 15 * max(1, max_lines)

    ws.freeze_panes = "A2"


def main():
    # Phase 1 — Validate and normalize
    validate_json(DATA)
    header_order = get_header_order(DATA)
    rows = normalize_rows(DATA, header_order)

    # Phase 3 — Base workbook
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"  # authoritative staging sheet per spec

    # Write headers and data
    ws.append(header_order)
    for row in rows:
        ws.append([row.get(k, "") for k in header_order])

    # Create META sheet (Very Hidden)
    meta_ws = wb.create_sheet("Meta_data_sheet")
    meta_ws.append(META_COLUMNS)
    for row in rows:
        meta_ws.append([row.get(k, "") for k in META_COLUMNS])
    meta_ws.sheet_state = 'veryHidden'

    # Normalize MAIN sheet on the same 'Data' sheet
    remaining = [k for k in header_order if (k not in META_COLUMNS and k not in MAIN_COLUMNS)]
    final_headers = MAIN_COLUMNS + remaining

    # Extract current rows from Data
    current = []
    for r in ws.iter_rows(min_row=2, max_row=ws.max_row, min_col=1, max_col=ws.max_column, values_only=True):
        current.append(dict(zip(header_order, r)))

    # Rebuild Data sheet with final_headers
    ws.delete_rows(1, ws.max_row)
    ws.append(final_headers)
    for r in current:
        ws.append([r.get(k, "") for k in final_headers])

    # Numbering inside cells (only TestPlan sheet data, META untouched)
    name_to_idx = {h: i + 1 for i, h in enumerate(final_headers)}
    for r in range(2, ws.max_row + 1):
        for field in ("Test Steps / Procedure", "Validation / Acceptance Criteria"):
            if field in name_to_idx:
                c = ws.cell(row=r, column=name_to_idx[field])
                c.value = renumber_multiline(c.value)

    # Rename Data -> TestPlan
    ws.title = "TestPlan"

    # Data validation ONLY for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in name_to_idx:
        col_idx = name_to_idx["Code Generation (Required / Not)"]
        dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showDropDown=True)
        ws.add_data_validation(dv)
        dv.add(f"{get_column_letter(col_idx)}2:{get_column_letter(col_idx)}{ws.max_row}")

    # Formatting and sizing
    apply_formatting(ws, final_headers)
    autofit_columns(ws, final_headers)

    # Safety check: no sheet named 'Data'
    for s in wb.worksheets:
        if s.title == "Data":
            fail("Validation failed: worksheet 'Data' still exists", 2)

    # Save with IST timestamp
    tz = ZoneInfo("Asia/Kolkata")
    ts = datetime.now(tz).strftime("%Y%m%d_%H%M%S")
    out_dir = os.environ.get("OUTPUT_DIR", os.path.join("Test_Output", "GPIO", "TestPlan"))
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, f"GPIO_TestPlan_{ts}.xlsx")
    wb.save(out_path)

    # OOXML validation
    if not zipfile.is_zipfile(out_path):
        fail("Validation failed: file is not a valid OOXML zip (.xlsx)", 3)
    with zipfile.ZipFile(out_path, 'r') as z:
        names = set(z.namelist())
        if "[Content_Types].xml" not in names or "xl/workbook.xml" not in names:
            fail("Validation failed: missing OOXML core entries", 4)

    print(out_path)


if __name__ == "__main__":
    main()
