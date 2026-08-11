#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Generate GPIO TestPlan Excel from Stage 6 aggregated JSON.
- Creates two sheets: 'TestPlan' (visible) and 'MetaData' (very hidden)
- Preserves row order and data exactly
- Applies formatting: bold headers, colored header fill, wrapped text, freeze first row, autosized columns
- Names file: <IP_NAME>_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx with IST time
- Writes commit_message.txt including IST timestamp for the workflow to use
"""
import json
import os
import hashlib
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# ---- Constants ----
OWNER = "titusbspgit"
REPO = "PSVValidation"
BRANCH = "main"
IP_NAME = "GPIO"
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
HEADER_FILL = PatternFill(start_color="FFD9E1F2", end_color="FFD9E1F2", fill_type="solid")  # light blue
HEADER_FONT = Font(bold=True)
WRAP_ALIGN = Alignment(wrap_text=True, vertical="top")

# Stage 6 aggregated JSON (preserved exactly)
STAGE6_JSON = r'''[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "NA",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Verify basic write/read functionality on GPIO GP0 registers 8–27.",
    "Meta Test Description": "This test targets GPIO GP0 register definitions referenced by macros MIZAR_GPIO_GP0_GPIO_8 through MIZAR_GPIO_GP0_GPIO_27. For each targeted register, the test writes a test value and then reads back the same register to confirm the value matches the write. Any mismatch is reported as a failure for that register and contributes to the overall test failure.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1. Initialize the GPIO test environment and ensure access to GPIO GP0 registers. 2. For each targeted GPIO GP0 register (8–27), write a test value. 3. Read back the register value. 4. Compare read data with the written value for each register. 5. Record results and summarize pass/fail.",
    "Meta Test Steps / Procedure": "1) Enumerate target register macros: MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27. 2) For each macro in the list: a) write a chosen test value to the corresponding register; b) read back the register; c) if (read_value != written_value) then log the macro/register context and flag failure for this entry; continue to next. 3) After processing all macros, if any entry failed, mark the overall test as FAIL; otherwise mark as PASS.",
    "Impacted Registers": "NA",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27",
    "Validation / Acceptance Criteria": "All targeted GPIO GP0 registers must read back the exact values written; any mismatch causes test failure.",
    "Meta Validation / Acceptance Criteria": "PASS if for every macro in {MIZAR_GPIO_GP0_GPIO_8..MIZAR_GPIO_GP0_GPIO_27}, read_value == written_value. FAIL if any register readback differs from the written value; report the first failing macro and the observed vs expected values.",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  }
]'''


def compute_ist_timestamp():
    ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    return ist, ist.strftime("%Y%m%d_%H%M%S")


def autosize_columns(ws, max_width=80, padding=2):
    dims = {}
    for row in ws.iter_rows(values_only=True):
        for i, value in enumerate(row, 1):
            text = "" if value is None else str(value)
            length = len(text)
            dims[i] = max(dims.get(i, 0), length)
    for col_idx, length in dims.items():
        width = min(length + padding, max_width)
        ws.column_dimensions[get_column_letter(col_idx)].width = width


def apply_header_format(ws, row=1):
    for cell in ws[row]:
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = WRAP_ALIGN


def apply_wrap_alignment(ws):
    for row in ws.iter_rows():
        for cell in row:
            cell.alignment = WRAP_ALIGN


def create_workbook(data_list):
    if not data_list:
        raise SystemExit("No data provided to generate workbook")

    headers = list(data_list[0].keys())

    wb = Workbook()
    ws = wb.active
    ws.title = "TestPlan"

    # Write header
    ws.append(headers)
    apply_header_format(ws, row=1)
    ws.freeze_panes = "A2"

    # Write rows preserving order
    for entry in data_list:
        row = [entry.get(h, "") for h in headers]
        ws.append(row)

    apply_wrap_alignment(ws)
    autosize_columns(ws)

    # MetaData sheet (very hidden)
    meta = wb.create_sheet("MetaData")
    meta.sheet_state = "veryHidden"
    meta.append(["Field", "Value"])
    apply_header_format(meta, row=1)

    ist_dt, ist_str = compute_ist_timestamp()
    src_hash = hashlib.sha256(STAGE6_JSON.encode("utf-8")).hexdigest()
    meta_rows = [
        ("Owner", OWNER),
        ("Repo", REPO),
        ("Branch", BRANCH),
        ("IP_NAME", IP_NAME),
        ("Output_Directory", OUTPUT_DIR),
        ("Generated_At_IST", ist_dt.isoformat()),
        ("Row_Count", str(len(data_list))),
        ("Source_JSON_SHA256", src_hash),
    ]
    for k, v in meta_rows:
        meta.append([k, v])
    apply_wrap_alignment(meta)
    autosize_columns(meta, max_width=120)

    return wb, ist_str


def main():
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    data_list = json.loads(STAGE6_JSON)
    wb, ist_str = create_workbook(data_list)

    filename = f"{IP_NAME}_TestPlan_{ist_str}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, filename)
    wb.save(out_path)

    # Prepare commit message with IST timestamp
    commit_msg = f"Add {IP_NAME} TestPlan Excel generated at {ist_str} IST (IP_NAME={IP_NAME})"
    with open(os.path.join(OUTPUT_DIR, "commit_message.txt"), "w", encoding="utf-8") as f:
        f.write(commit_msg + "\n")

    print(f"Generated: {out_path}")


if __name__ == "__main__":
    main()
