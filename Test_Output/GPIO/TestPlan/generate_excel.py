#!/usr/bin/env python3
"""Generate GPIO TestPlan Excel workbook using openpyxl."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

def generate_workbook():
    # IST timezone
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"GPIO_TestPlan_{timestamp}.xlsx"
    output_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(output_dir, filename)

    # Input data
    row_data = {
        "Index": "1",
        "SS / Module": "gpio",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Feature": "Register Write Read Test",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "test_define.c"; <gpio/gpio_def.h>; <gpio/gpio_offset.h>',
        "Meta Macros": "SOFT_RST_REG_ADDRESS; SOFT_RST_REG_DATA; CNT; DEBUG_DISPLAY",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Test Description": "This testcase validates GPIO register default values and write-read functionality across 49 GPIO registers including gp0_gpio_8, gp0_gpio_9, and gp0_gpio_10. In the first phase, each register is read and its value (with the least significant bit masked off) is compared against the expected default value. Registers that are not readable or are marked for skipping are excluded. In the second phase, six distinct data patterns are written to each writable register and then read back. The read-back value is compared against an expected value computed using the write mask, read mask, and default value for each register. The test passes only if all default value checks and all write-read checks succeed with zero mismatches.",
        "Meta Test Description": "This testcase performs two phases of GPIO register validation. Phase 1 (chk_rst_val): Iterates over 49 register addresses in addr_array. For each register, it checks skip_rst_array to determine if the register should be skipped for reset value checking. It also checks read_mask_array; if the read mask is 0x00000000, the register is not readable and is skipped. For readable registers, it reads the register value using read_reg(addr), masks the read data with 0xfffffffe, and compares against default_value_array[i]. Mismatches increment def_fail_cnt. Phase 2 (chk_rd_wr): Iterates over 6 test patterns in chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern, it writes the pattern (masked with write_mask_array[i]) to each register address in addr_array, skipping entries where skip_array[i]==1 or write_mask_array[i]==0x00000000. Then it reads back each register, masking with read_mask_array[i], and computes the expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = write_mask_array[i] ^ 0xffffffff. Mismatches increment wr_fail_cnt. The test passes (finish(0)) if both def_fail_cnt and wr_fail_cnt are zero; otherwise it fails (finish(1)). The addr_array contains register address macros including MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10. A soft_reset_chk function is defined but conditionally compiled out (#ifdef 0).",
        "Test Steps / Procedure": "1. Read all 49 GPIO registers and verify that each readable register contains its expected default reset value (with the least significant bit masked off). Skip registers marked as non-readable or excluded from reset checking.\n2. For each of six test data patterns, write the pattern to all writable GPIO registers, applying the appropriate write mask for each register. Skip registers marked as non-writable or excluded.\n3. After each write pass, read back all writable and readable GPIO registers and compare the read value against the expected value, which accounts for the write mask, read mask, and default value of each register.\n4. Verify that all default value checks and all write-read checks pass with zero mismatches. If any mismatch is detected, the test fails; otherwise the test passes.",
        "Meta Test Steps / Procedure": "1. Call chk_rst_val(): Loop i from 0 to CNT-1 (49 registers). For each i: get addr = addr_array[i]. If skip_rst_array[i]==1, skip. If read_mask_array[i]==0x00000000, skip (not readable). Otherwise, data_rd = read_reg(addr); data = data_rd & 0xfffffffe; compare data with default_value_array[i]. If mismatch, increment def_fail_cnt. 2. Call chk_rd_wr(): Loop j from 0 to 5 over chk_val[6]={0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern: (a) Write phase: loop i from 0 to CNT-1. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. Otherwise write_reg(addr, data_wr & write_mask_array[i]). (b) Read phase: loop i from 0 to CNT-1. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. If read_mask_array[i]==0x00000000, skip. Otherwise data_rd = read_reg(addr) & read_mask_array[i]; compute wr_n = write_mask_array[i] ^ 0xffffffff; exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]); compare data_rd with exp_val. If mismatch, increment wr_fail_cnt. 3. Check if def_fail_cnt > 0 or wr_fail_cnt > 0: if yes, finish(1) (fail); else finish(0) (pass).",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Validation / Acceptance Criteria": "1. Each readable GPIO register must return its expected default reset value after masking the least significant bit.\n2. For each of six test patterns, after writing to each writable register, the read-back value must match the expected value computed from the write mask, read mask, and default value.\n3. The test passes only if zero mismatches are detected across both the default value check phase and the write-read check phase. Any mismatch causes test failure.",
        "Meta Validation / Acceptance Criteria": "Phase 1 (default value check): For each readable, non-skipped register, (read_reg(addr) & 0xfffffffe) must equal default_value_array[i]. Any mismatch increments def_fail_cnt. Phase 2 (write-read check): For each writable and readable, non-skipped register, after writing (data_wr & write_mask_array[i]), the read-back value (read_reg(addr) & read_mask_array[i]) must equal ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Any mismatch increments wr_fail_cnt. Final pass condition: def_fail_cnt == 0 AND wr_fail_cnt == 0 results in finish(0) (pass). Otherwise finish(1) (fail).",
        "Remarks": "The addr_array contains 49 register entries but only 3 register macros are visible in test_define.c (the remaining entries are truncated in the source). Some registers are skipped for write-read testing via skip_array and for reset value checking via skip_rst_array. A comment in test_define.c notes that the din bit value may become 1 automatically if not forced, causing the bit-level select to go high and potentially causing default value mismatches. The soft_reset_chk function is defined but compiled out via #ifdef 0. The least significant bit is masked off during default value comparison (mask 0xfffffffe).",
        "Code Generation": ""
    }

    # TestPlan columns
    tp_cols = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
        "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
        "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
        "Code Generation"
    ]

    # MetaData columns
    md_cols = [
        "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
        "Meta Headers", "Meta Macros", "Meta Arrays"
    ]

    # Create workbook
    wb = openpyxl.Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Header formatting
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    header_align = Alignment(horizontal="center", vertical="center", wrap_text=True)
    cell_align = Alignment(vertical="top", wrap_text=True)

    # Write TestPlan headers
    for col_idx, col_name in enumerate(tp_cols, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align

    # Write TestPlan data row
    for col_idx, col_name in enumerate(tp_cols, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=2, column=col_idx, value=value)
        cell.alignment = cell_align

    # Freeze first row
    ws_tp.freeze_panes = "A2"

    # Auto-size columns with max width cap
    for col_idx, col_name in enumerate(tp_cols, 1):
        max_len = len(col_name)
        value = str(row_data.get(col_name, ""))
        # For multi-line, take the longest line
        for line in value.split("\n"):
            if len(line) > max_len:
                max_len = len(line)
        # Cap at 60 for very wide columns
        adjusted_width = min(max_len + 4, 60)
        # Minimum width
        adjusted_width = max(adjusted_width, 12)
        ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted_width

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet(title="MetaData")

    # Write MetaData headers
    for col_idx, col_name in enumerate(md_cols, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_align

    # Write MetaData data row
    for col_idx, col_name in enumerate(md_cols, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=2, column=col_idx, value=value)
        cell.alignment = cell_align

    # Freeze first row
    ws_md.freeze_panes = "A2"

    # Auto-size columns with max width cap
    for col_idx, col_name in enumerate(md_cols, 1):
        max_len = len(col_name)
        value = str(row_data.get(col_name, ""))
        for line in value.split("\n"):
            if len(line) > max_len:
                max_len = len(line)
        adjusted_width = min(max_len + 4, 60)
        adjusted_width = max(adjusted_width, 12)
        ws_md.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted_width

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save workbook
    wb.save(filepath)
    print(f"SAVED:{filepath}")
    print(f"FILENAME:{filename}")

    # Validation
    if os.path.exists(filepath) and os.path.getsize(filepath) > 0:
        # Reopen to validate
        wb2 = openpyxl.load_workbook(filepath)
        sheets = wb2.sheetnames
        if "TestPlan" in sheets and "MetaData" in sheets:
            tp_rows = wb2["TestPlan"].max_row - 1  # minus header
            md_rows = wb2["MetaData"].max_row - 1
            print(f"VALIDATION:PASSED")
            print(f"ROWS_TP:{tp_rows}")
            print(f"ROWS_MD:{md_rows}")
            print(f"SIZE:{os.path.getsize(filepath)}")
        else:
            print("VALIDATION:FAILED - Missing sheets")
        wb2.close()
    else:
        print("VALIDATION:FAILED - File not found or empty")

    wb.close()
    return filepath, filename

if __name__ == "__main__":
    generate_workbook()
