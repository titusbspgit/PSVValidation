#!/usr/bin/env python3
"""
GPIO TestPlan Excel Generator
Generates GPIO_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
Auto-triggered by GitHub Actions workflow
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os
import json
import sys

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"GPIO_TestPlan_{timestamp_str}.xlsx"
output_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(output_dir, filename)

# Input data
data = [
    {
        "Index": 1,
        "SS_Module": "GPIO",
        "Test_Case_Name": "gpio_reg_wr_rd_test",
        "Feature": "Register Read/Write Validation",
        "Meta_Test_Description": "This testcase validates the default (reset) values and read/write accessibility of GPIO GP0 registers (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10) at base address 0xA001A000 with offsets 0x0, 0x4, and 0x8 respectively. The test_define.c file defines addr_array[49] containing register address macros, default_value_array[49] with expected reset values (GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL), read_mask_array[49] with read masks (GPIO_GP0_GPIO_8_READ_MASK, etc.), write_mask_array[49] with write masks (GPIO_GP0_GPIO_8_WRITE_MASK, etc.), skip_array[49] to skip VRRW registers during write/read, and skip_rst_array[49] to skip certain registers during reset value check. The program.c test_case() function calls chk_rst_val() to verify default values, then chk_rd_wr() to perform write-then-read-back verification using six test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000). The soft_reset_chk() function is inside #ifdef 0 and is compile-time disabled. SOFT_RST_REG_ADDRESS macro is ignored per instructions. Each register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) is a 32-bit register containing fields: data_in (bit 0, RO, reset 0), intr_raw_sts (bit 1, RO, reset 0), intr_clr (bit 16, WO, reset 0), pedge_intr_en (bit 17, RW, reset 0), nedge_intr_en (bit 18, RW, reset 0), level_sel (bit 19, RW, reset 0), io_ctrl (bit 20, RW2, reset 1), dout (bit 21, RW2, reset 0).",
        "Test_Description": "This test validates the reset default values and read/write functionality of three GPIO registers: gp0_gpio_8, gp0_gpio_9, and gp0_gpio_10. First, each register is read and its value is compared against the expected default value after reset. Then, six distinct data patterns are written to the writable fields of each register and read back to verify data integrity. The test uses read masks and write masks to account for read-only and write-only fields. Registers marked in the skip arrays (such as VRRW-type registers) are excluded from the respective checks. The test reports pass or fail based on whether all default value checks and write-read-back comparisons succeed.",
        "Meta_Test_Steps": "1. Include headers: gpio/gpio_def.h, gpio/gpio_offset.h, test_common.h, test_define.c.\n2. Define CNT=49, addr_array[49] = {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, ...}.\n3. Define default_value_array[49] = {GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL, ...}.\n4. Define read_mask_array[49] = {GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK, ...}.\n5. Define write_mask_array[49] = {GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK, ...}.\n6. Define skip_array[49] and skip_rst_array[49] to skip VRRW registers and certain registers during reset check.\n7. test_case() entry: call chk_rst_val().\n8. chk_rst_val(): for i=0 to CNT-1, addr = addr_array[i]. If skip_rst_array[i]==1, skip. If read_mask_array[i]==0x00000000, skip (not readable). data_rd = read_reg(addr). data = (data_rd & 0xfffffffe). Compare data with default_value_array[i]. If mismatch, increment def_fail_cnt and print error.\n9. test_case(): call chk_rd_wr().\n10. chk_rd_wr(): define chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}. For j=0 to 5: data_wr = chk_val[j]. Write pass: for i=0 to CNT-1, addr = addr_array[i]. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip (not writable). Else write_reg(addr, data_wr & write_mask_array[i]). Read pass: for i=0 to CNT-1, addr = addr_array[i]. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. If read_mask_array[i]==0x00000000, skip. Else data_rd = read_reg(addr) & read_mask_array[i]. wr_n = write_mask_array[i] ^ 0xFFFFFFFF. exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]). Compare data_rd with exp_val. If mismatch, increment wr_fail_cnt.\n11. test_case(): if def_fail_cnt > 0 or wr_fail_cnt > 0, finish(1) (FAIL). Else finish(0) (PASS).\n12. soft_reset_chk() is inside #ifdef 0 -- compile-time disabled, not executed.",
        "Test_Steps": "1. Read each GPIO register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) and verify the read value matches the expected reset default value, skipping registers flagged in the reset-skip array and non-readable registers.\n2. For each of six test data patterns (all-ones, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000), write the pattern (masked by the write mask) to each writable GPIO register, skipping registers flagged in the skip array and non-writable registers.\n3. After each write pass, read back each register (masked by the read mask) and compare the read value against the expected value computed from the write data, read mask, write mask, and default value.\n4. Verify that all default value checks and all write-read-back comparisons pass without mismatch.\n5. Report overall test result as PASS if no failures occurred, or FAIL if any default value mismatch or write-read mismatch was detected.",
        "Meta_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Impacted_Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Validation_Criteria": "All register default values must match the expected reset values after masking. All write-read-back operations for six test patterns must return the expected computed value based on read mask, write mask, and default value. Zero failures in both default value check and write-read check indicate PASS.",
        "Speed": "NA",
        "Mode": "NA",
        "Remarks": "The soft reset check function is compile-time disabled and not executed. VRRW-type registers are skipped during write-read testing via the skip array. Certain registers are also skipped during reset value checking via the reset-skip array. The read value in default check is masked with 0xFFFFFFFE (bit 0 masked out) before comparison."
    }
]

# Create workbook
wb = openpyxl.Workbook()

# ============================================================
# TESTPLAN SHEET
# ============================================================
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_headers = [
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
    "Code Generation"
]

# Header formatting
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
cell_alignment = Alignment(vertical='top', wrap_text=True)
thin_border = Border(
    left=Side(style='thin'),
    right=Side(style='thin'),
    top=Side(style='thin'),
    bottom=Side(style='thin')
)

# Write TestPlan headers
for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, item in enumerate(data, 2):
    row_data = [
        item.get("Index", ""),
        item.get("SS_Module", ""),
        item.get("Feature", ""),
        item.get("Test_Case_Name", ""),
        item.get("Test_Description", ""),
        item.get("Speed", ""),
        item.get("Mode", ""),
        "",  # Memory Start Offset
        "",  # Memory End Offset
        item.get("Remarks", ""),
        item.get("Test_Steps", ""),
        item.get("Impacted_Registers", ""),
        item.get("Validation_Criteria", ""),
        ""   # Code Generation
    ]
    for col_idx, value in enumerate(row_data, 1):
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = 'A2'

# ============================================================
# METADATA SHEET
# ============================================================
ws_md = wb.create_sheet(title="MetaData")

md_headers = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

# Write MetaData headers
for col_idx, header in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Extract meta fields and write MetaData data
for row_idx, item in enumerate(data, 2):
    meta_headers = "gpio/gpio_def.h; gpio/gpio_offset.h; test_common.h; test_define.c"
    meta_macros = "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; GPIO_GP0_GPIO_8_DEFAULT_VAL; GPIO_GP0_GPIO_9_DEFAULT_VAL; GPIO_GP0_GPIO_10_DEFAULT_VAL; GPIO_GP0_GPIO_8_READ_MASK; GPIO_GP0_GPIO_9_READ_MASK; GPIO_GP0_GPIO_10_READ_MASK; GPIO_GP0_GPIO_8_WRITE_MASK; GPIO_GP0_GPIO_9_WRITE_MASK; GPIO_GP0_GPIO_10_WRITE_MASK; SOFT_RST_REG_ADDRESS; SOFT_RST_REG_DATA; CNT"
    meta_arrays = "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]"

    row_data = [
        item.get("Index", ""),
        item.get("Test_Case_Name", ""),
        item.get("Meta_Test_Description", ""),
        item.get("Meta_Test_Steps", ""),
        item.get("Meta_Impacted_Registers", ""),
        item.get("Validation_Criteria", ""),
        meta_headers,
        meta_macros,
        meta_arrays
    ]
    for col_idx, value in enumerate(row_data, 1):
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = 'A2'

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# ============================================================
# AUTO-SIZE COLUMNS
# ============================================================
def auto_size_columns(ws, max_width=60):
    for col in ws.columns:
        max_length = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    if len(line) > max_length:
                        max_length = len(line)
        adjusted_width = min(max_length + 4, max_width)
        if adjusted_width < 12:
            adjusted_width = 12
        ws.column_dimensions[col_letter].width = adjusted_width

auto_size_columns(ws_tp, max_width=55)
auto_size_columns(ws_md, max_width=55)

# ============================================================
# SAVE WORKBOOK
# ============================================================
wb.save(filepath)
wb.close()

# ============================================================
# VALIDATION
# ============================================================
if os.path.exists(filepath) and os.path.getsize(filepath) > 0:
    wb_check = openpyxl.load_workbook(filepath)
    sheets = wb_check.sheetnames
    if "TestPlan" in sheets and "MetaData" in sheets:
        print(f"VALIDATION=PASSED")
        print(f"FILENAME={filename}")
        print(f"FILEPATH={filepath}")
        print(f"FILESIZE={os.path.getsize(filepath)}")
        print(f"SHEETS={sheets}")
        print(f"TESTPLAN_ROWS={ws_tp.max_row - 1}")
        print(f"METADATA_ROWS={ws_md.max_row - 1}")
    else:
        print("VALIDATION=FAILED - Missing sheets")
    wb_check.close()
else:
    print("VALIDATION=FAILED - File not found or empty")
