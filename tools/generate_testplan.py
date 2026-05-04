#!/usr/bin/env python3
# coding: utf-8

import os
import re
import math
import json
import zipfile
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation

# -----------------------------
# Input data (normalized array)
# -----------------------------
records = [
    {
        "Index": "1",
        "SS / Module": "gpio",
        "Feature": "GPIO negative-edge interrupt enable and servicing",
        "Test Case Name": "test_gpio_negedge_intr_en",
        "Test Description": "Verifies that negative-edge interrupts on GPIO pins 8–39 can be enabled, triggered, and correctly handled.",
        "Speed": "NA",
        "Mode": "Interrupt",
        "Memory Start Offset": "0xA0243ffc",
        "Memory End Offset": "0xA0243ffc",
        "Remarks": "Timeout value is 5000 iterations and may need adjustment for the simulation time base. Assumes the raw interrupt clear register uses write‑1‑to‑clear behavior. All pads are driven high initially to establish a known state.",
        "Test Steps / Procedure": "1) Enable the platform interrupt for the GPIO block in the interrupt controller and system controller. 2) Drive all GPIO outputs high to set a known baseline. 3) For pins 8–39, configure each pin as input, enable negative-edge detection, and clear any pending status. 4) For each pin, clear the corresponding group raw interrupt bit. 5) Enable the interrupt mask only for the current pin. 6) Arm the wait flag, generate a falling edge on the current pin, and wait for the interrupt with a timeout. 7) In the interrupt handler, restore the pad state to high, read the pin status, and verify the input reads low. 8) Read the group status and verify the bit for the current pin is set. 9) Clear the per-pin raw status and the group raw status, then read back to confirm the group status is cleared. 10) Clear the system-level raw status and the interrupt in the interrupt controller.",
        "Imparted Registers": "",
        "Impacted Registers": "",
        "Validation / Acceptance Criteria": "- A falling edge on each tested pin causes an interrupt before the timeout; PASS if no timeout occurs for any pin.\n- The input value for the serviced pin is low when read in the handler; PASS if the read value is 0.\n- The group status bit for the serviced pin is set on entry to the handler; PASS if the bit is 1 prior to clears.\n- After clearing the per-pin and group raw status, the group status reads 0; PASS if the readback is 0 for the group.",
        "Code Generation (Required / Not)": "",
        "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
        "Hidden_Test_Description": "Test negative-edge GPIO interrupts for pins 8..39. Sequence: enable platform IRQ (GIC) and system interrupt for GPIO, set pads high, configure pins 8..39 as input with negedge and clear per-pin raw, then for each bit enable only that interrupt, generate a falling edge via 0xA0243ffc (drive all high then drive the specific bit low), wait for ISR with timeout, and in ISR verify DIN=0, per-pin raw (bit1) set, group status bit set, perform clears per-pin and group, verify group cleared, clear system RAW and GIC pending. Finish with test_err as pass/fail aggregator.",
        "Hidden_Remarks": "1) The bounded wait uses timeout = 5000 and wait_on(10) per poll; comment notes it may need adjustment for simulation time base. 2) Assumes RAW_STCLR1 is write-1-to-clear. 3) All pads are driven high initially using address 0xA0243ffc. 4) The handler computes the current bit mask from the global loop index i.",
        "Hidden_Test_Steps_Procedure": '''Entry Points:\nA) test_case()\nB) Default_IRQHandler() [invoked by hardware on GPIO interrupt]\n\nRuntime Trace (in order):\n1. test_case(): Initialize test_err = 0.\n2. Conditional enable of GIC interrupt:\n - If GPIO0 is defined: call GIC_EnableIRQ(87).\n - If GPIO1 is defined: call GIC_EnableIRQ(88).\n3. Conditional enable of system-level interrupt for GPIO:\n - If GPIO0 is defined: WRITE MIZAR_LSS_SYSREG_INTR_EN1 <- LSS_SYSREG_INTR_EN1_GPIO0_INTR.\n - If GPIO1 is defined: WRITE MIZAR_LSS_SYSREG_INTR_EN1 <- LSS_SYSREG_INTR_EN1_GPIO1_INTR.\n4. Set pad driver to a known state: WRITE 0xA0243ffc <- 0xffffffff (all high).\n\nPhase 1: Configure pins 8..39 for input + negedge, clear pending raw\n5. Loop entry: for (i = 0; i < 32; i++):\n 5.1 Loop body (per iteration i):\n - Compute addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n - WRITE addr1 <- ((1 << 20) | (1 << 18) | (1 << 16)) // doe=1 (input), neie=1, iclr=1.\n - Call wait_on(10).\n 5.2 Exit condition: i reaches 32.\n\nPhase 2: Per-pin enable, edge generation, and wait with timeout\n6. Loop entry: for (i = 0; i < 32; i++):\n 6.1 Set wr_val = (1u << i).\n 6.2 Pre-clear group raw status for this bit: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 <- wr_val.\n 6.3 Enable only this pin's interrupt: WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 <- wr_val.\n 6.4 Call wait_on(10).\n 6.5 Prepare to wait for interrupt: int_pend = 1.\n 6.6 Generate a falling edge on bit i:\n - WRITE 0xA0243ffc <- 0xffffffff (ensure all high).\n - Call wait_on(30).\n - WRITE 0xA0243ffc <- bitwise_not(wr_val) (drive current bit low; others high).\n 6.7 Bounded wait for ISR to clear int_pend:\n - Initialize unsigned int timeout = 5000.\n - While (int_pend && timeout--): call wait_on(10) each iteration.\n - On exit: if (timeout == 0):\n - printf("ERROR: Timeout waiting for GPIO%u negedge interrupt", i + 8).\n - test_err++.\n 6.8 Continue loop to next i.\n 6.9 Exit condition: i reaches 32.\n7. Call finish(test_err).\n\nInterrupt Handler (invoked during step 6.7 for each pin that interrupts):\n8. Default_IRQHandler():\n 8.1 Local variables: rdata_grp, raddr, raddr2; compute local_wr = (1u << i).\n 8.2 Signal main loop to proceed: int_pend = 0.\n 8.3 Restore pad driver to known state: WRITE 0xA0243ffc <- 0xffffffff.\n 8.4 Compute raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n 8.5 READ rdata <- read_reg(raddr).\n 8.6 Check DIN value for falling edge: if ((rdata & 0x1) != 0) then test_err++.\n 8.7 Check per-pin raw interrupt bit (bit1 expected set):\n - If ((rdata & 0x2) != 0x0) then:\n a) READ rdata_grp <- read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1).\n b) If ((rdata_grp & local_wr) == 0) then test_err++.\n c) Compute raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4).\n d) Clear per-pin raw while keeping direction: WRITE raddr2 <- ((1 << 20) | (1 << 16)).\n e) Clear group raw bit: WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 <- local_wr.\n f) Verify group clear: READ rdata_grp <- read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp != 0x0) then test_err++.\n g) Clear system raw and GIC pending:\n - If GPIO0 is defined: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 <- LSS_SYSREG_RAW_STCR1_GPIO0_INTR; call GIC_ClearIRQ(87).\n - If GPIO1 is defined: WRITE MIZAR_LSS_SYSREG_RAW_STCR1 <- LSS_SYSREG_RAW_STCR1_GPIO1_INTR; call GIC_ClearIRQ(88).\n - Else (raw bit not set): test_err++.\n\nTiming Details:\n- wait_on(10) used after configuration and per iteration for bounded waits.\n- wait_on(30) used between setting all high and driving specific pin low to create a detectable falling edge.\n- Timeout counter initialized to 5000 for ISR wait loop; loop decrements once per wait_on(10) iteration.\n\nRegister Access Summary within execution:\n- WRITE MIZAR_LSS_SYSREG_INTR_EN1 (enable system-level GPIO interrupt).\n- WRITE 0xA0243ffc (pad drive control) multiple times to set/restore pin states.\n- WRITE MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to configure per-pin doe/neie/iclr and to clear per-pin raw.\n- WRITE MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 to clear group raw bit for selected pin.\n- WRITE MIZAR_GPIO_GP0_INTR1_INTR_EN1 to enable interrupt mask for selected pin.\n- READ MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to sample DIN and raw bit.\n- READ/WRITE MIZAR_GPIO_GP0_INTR1_INTR_STS1 to verify/clear group status.\n- WRITE MIZAR_LSS_SYSREG_RAW_STCR1 to clear system-level raw status.\n- GIC enable/clear APIs used to manage platform interrupts.''',
        "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
        "Hidden_Validation_Acceptance_Criteria": '''1) No timeout during bounded wait for any i in 0..31. If timeout==0 for any pin, test_err++.\n2) In ISR, DIN bit (bit0) of per-pin register reads 0 after the falling edge; else test_err++.\n3) In ISR, per-pin raw bit (bit1) is set; else test_err++.\n4) Group interrupt status register has the bit for the current pin set; else test_err++.\n5) After clearing per-pin raw and group raw, the group status register reads 0; else test_err++.\n6) Final finish(test_err) reflects aggregated result: PASS if test_err==0; FAIL otherwise.'''
    }
]

if not isinstance(records, list) or len(records) == 0:
    raise SystemExit("Invalid or empty JSON input; expected non-empty list of records.")

# Column definitions
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

# Build union of all keys in first-seen order
all_keys = []
for rec in records:
    for k in rec.keys():
        if k not in all_keys:
            all_keys.append(k)

# -----------------------------
# Helper functions
# -----------------------------

def set_col_widths(ws):
    # Approximate auto-fit for TestPlan sheet
    max_len = {}
    for row in ws.iter_rows(values_only=True):
        for idx, val in enumerate(row, start=1):
            if val is None:
                l = 0
            else:
                s = str(val)
                l = max(len(part) for part in s.split("\n"))
            max_len[idx] = max(max_len.get(idx, 0), l)
    for idx, l in max_len.items():
        width = min(max(10, l + 2), 80)  # cap width
        ws.column_dimensions[chr(64 + idx) if idx <= 26 else _col_letter(idx)].width = width


def _col_letter(n):
    string = ""
    while n > 0:
        n, remainder = divmod(n - 1, 26)
        string = chr(65 + remainder) + string
    return string


def apply_borders(ws, max_row, max_col):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, max_row + 1):
        for c in range(1, max_col + 1):
            cell = ws.cell(row=r, column=c)
            cell.border = border


def to_numbered_list(text):
    if text is None:
        return ""
    t = str(text).strip()
    if not t:
        return ""
    parts = []
    if "\n" in t:
        for line in t.splitlines():
            line = line.strip()
            if not line:
                continue
            line = re.sub(r'^\s*(?:-|\d+[.)])\s*', '', line)
            parts.append(line)
    else:
        parts = [p.strip() for p in re.split(r'\s*(?:\d+[.)]|-)\s+', t) if p.strip()]
    return "\n".join(f"{i+1}. {p}" for i, p in enumerate(parts))


def estimate_row_height_for_row(ws, row_idx, wrap_cols_idx):
    # Estimate based on newline counts across wrapped columns
    max_lines = 1
    for c in wrap_cols_idx:
        val = ws.cell(row=row_idx, column=c).value
        if val is None:
            continue
        s = str(val)
        lines = s.count('\n') + 1
        if lines > max_lines:
            max_lines = lines
    # 15 points per line as a heuristic
    ws.row_dimensions[row_idx].height = max_lines * 15


# -----------------------------
# Workbook creation
# -----------------------------
wb = Workbook()
ws_data = wb.active
ws_data.title = "Data"

# Write Data sheet with all keys in header
header_font = Font(bold=True)
ws_data.append(all_keys)
for cell in ws_data[1]:
    cell.font = header_font
ws_data.freeze_panes = "A2"

for rec in records:
    ws_data.append([rec.get(k, "") for k in all_keys])

# Create META sheet and copy META columns as-is
ws_meta = wb.create_sheet("Meta_data_sheet")
ws_meta.append(META_COLUMNS)
for rec in records:
    ws_meta.append([rec.get(k, "") for k in META_COLUMNS])
# Very hidden meta sheet
ws_meta.sheet_state = 'veryHidden'

# Rename Data -> TestPlan (do not create new main data sheet)
ws_main = ws_data
ws_main.title = "TestPlan"

# Rebuild TestPlan columns: remove META and non-main columns, enforce order
# Clear existing content and rewrite with MAIN schema
for row in ws_main[ws_main.dimensions]:
    for cell in row:
        cell.value = None

ws_main.delete_cols(1, ws_main.max_column)

ws_main.append(MAIN_COLUMNS)

# Prepare styles
header_fill = PatternFill("solid", fgColor="4472C4")
center_center = Alignment(horizontal='center', vertical='center', wrap_text=True)

for cell in ws_main[1]:
    cell.font = Font(bold=True)
    cell.alignment = center_center
    cell.fill = header_fill

# Map column names to indices
col_idx = {name: idx + 1 for idx, name in enumerate(MAIN_COLUMNS)}

# Write data rows with numbering for specified columns
for rec in records:
    row_vals = []
    for name in MAIN_COLUMNS:
        val = rec.get(name, "")
        if name == "Test Steps / Procedure":
            val = to_numbered_list(val)
        elif name == "Validation / Acceptance Criteria":
            val = to_numbered_list(val)
        row_vals.append(val)
    ws_main.append(row_vals)

# Apply alignment and wrapping
for r in range(2, ws_main.max_row + 1):
    for c in range(1, ws_main.max_column + 1):
        col_name = MAIN_COLUMNS[c - 1]
        val_align = Alignment(
            horizontal=('center' if col_name == 'Index' else 'left'),
            vertical='top',
            wrap_text=(col_name in WRAP_COLUMNS)
        )
        ws_main.cell(row=r, column=c).alignment = val_align

# Freeze top row
ws_main.freeze_panes = "A2"

# Auto-fit columns (approximate) and row heights after wrapping
set_col_widths(ws_main)
wrap_cols_idx = [MAIN_COLUMNS.index(n) + 1 for n in MAIN_COLUMNS if n in WRAP_COLUMNS]
for r in range(2, ws_main.max_row + 1):
    estimate_row_height_for_row(ws_main, r, wrap_cols_idx)

# Borders for all populated cells
apply_borders(ws_main, ws_main.max_row, ws_main.max_column)

# Data validation ONLY for "Code Generation (Required / Not)" on data rows
code_col = col_idx["Code Generation (Required / Not)"]
dv = DataValidation(type="list", formula1='"Required,Blank,Not Required"', allow_blank=True, showErrorMessage=True)
ws_main.add_data_validation(dv)
if ws_main.max_row >= 2:
    dv.add(f"{_col_letter(code_col)}2:{_col_letter(code_col)}{ws_main.max_row}")

# Final visibility enforcement: ensure only TestPlan and Meta_data_sheet exist; no 'Data' sheet remains
for name in list(wb.sheetnames):
    if name not in ("TestPlan", "Meta_data_sheet"):
        ws_other = wb[name]
        if name != "TestPlan" and name != "Meta_data_sheet":
            wb.remove(ws_other)

if "Data" in wb.sheetnames:
    # Safety: delete if somehow present
    wb.remove(wb["Data"])

# Output path and file name with IST timestamp
ip_name = "GPIO"
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(tz=ist)
stamp_date = now_ist.strftime("%Y%m%d")
stamp_time = now_ist.strftime("%H%M%S")
file_name = f"{ip_name}_TestPlan_{stamp_date}_{stamp_time}.xlsx"
output_dir = os.path.join("Test_Output", "GPIO", "TestPlan")
os.makedirs(output_dir, exist_ok=True)
file_path = os.path.join(output_dir, file_name)

# Save workbook
wb.save(file_path)

# Validate that it's a true XLSX (ZIP with key parts)
with zipfile.ZipFile(file_path, 'r') as zf:
    namelist = set(zf.namelist())
    required = {"[Content_Types].xml", "xl/workbook.xml"}
    if not required.issubset(namelist):
        raise SystemExit("XLSX validation failed: core parts missing")

# Emit path for GitHub Action step
os.makedirs('tools', exist_ok=True)
with open('tools/generated_path.txt', 'w', encoding='utf-8') as f:
    f.write(file_path)
print(file_path)
