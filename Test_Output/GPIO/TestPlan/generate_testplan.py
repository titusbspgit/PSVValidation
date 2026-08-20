#!/usr/bin/env python3
"""Auto-generated TestPlan Excel Generator."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"GPIO_TestPlan_{timestamp}.xlsx"
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)

wb = openpyxl.Workbook()

# TestPlan Sheet
ws_tp = wb.active
ws_tp.title = "TestPlan"
tp_headers = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Code Generation"
]
ws_tp.append(tp_headers)

tp_row = [
    1,
    "GPIO",
    "Register Write-Read Verification",
    "gpio_reg_wr_rd_test",
    "This test verifies the default reset values and write-read integrity of GPIO GP0 registers (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10). First, it reads each register after reset and validates that the read value (with bit 0 masked out) matches the expected default value. Then, it performs a write-read verification using six different data patterns (all-ones, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000). For each pattern, the test writes the pattern (masked with the write mask) to each register, reads it back (masked with the read mask), and compares the result against the expected value computed from the write mask, read mask, and default values. The test passes only if all default value checks and all write-read comparisons succeed across all registers and all patterns.",
    "NA",
    "NA",
    "",
    "",
    "The test uses six distinct data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000) to exercise all bit positions. Bit 0 is masked out during default value comparison due to the data_in field being a read-only input pin status that may not match the static default. Each register contains fields of types RO, WO, RW, and RW2, and the test correctly handles mixed-access registers using write and read masks. Skip arrays are used to exclude specific registers from reset-value checks and write-read checks. The soft_reset_chk function is disabled (dead code). The array size CNT is declared as 49 but only 3 register entries are populated in the address array.",
    "1. Initialize the test environment and prepare the GPIO GP0 register set for validation.\n2. Perform default reset value verification: Read each GPIO register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) after reset, mask out bit 0 from the read value, and compare against the expected default reset value. Skip any register flagged in the reset-skip list or marked as non-readable.\n3. Perform write-read verification using six test data patterns (all-ones, alternating-bit patterns, and mixed patterns):\n   a. For each pattern, write the pattern (masked with the register's write mask) to each writable GPIO register.\n   b. Read back each register (masked with the read mask) and compute the expected value accounting for read-only bits retaining their default values.\n   c. Compare the read-back value against the expected value.\n4. Skip any register flagged in the skip list or marked as non-writable or non-readable during the write-read phase.\n5. Aggregate all default-value mismatches and write-read mismatches.\n6. If any mismatch is detected across any register or any pattern, report the test as FAIL. Otherwise, report PASS.",
    "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
    "1. All GPIO registers must return their expected default reset values after reset (with bit 0 masked out). 2. For each of the six test data patterns, the write-read-back value of each register must match the expected value computed using the write mask, read mask, and default values. 3. Read-only bits must retain their default values after write operations. 4. Non-writable and non-readable registers must be correctly skipped. 5. The test passes only if zero mismatches are detected across all registers and all patterns.",
    ""
]
ws_tp.append(tp_row)

# MetaData Sheet
ws_md = wb.create_sheet("MetaData")
md_headers = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]
ws_md.append(md_headers)

md_row = [
    1,
    "gpio_reg_wr_rd_test",
    "This testcase validates the default reset values and write-read integrity of GPIO GP0 registers. It includes test_define.c which defines addr_array[49] containing register address macros {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10}, along with corresponding default_value_array (GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL), read_mask_array (GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK), and write_mask_array (GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK). Headers included are gpio/gpio_def.h and gpio/gpio_offset.h. CNT is defined as 49. skip_array and skip_rst_array control which registers are skipped during write-read and reset-value checks respectively. The test_case() function first calls chk_rst_val() which iterates over all registers, skips entries where skip_rst_array[i]==1 or read_mask_array[i]==0x00000000, reads each register via read_reg(addr), masks the read data with 0xfffffffe (clearing bit 0), and compares against default_value_array[i]. Any mismatch increments def_fail_cnt. Then chk_rd_wr() is called which iterates over 6 test patterns chk_val[6]={0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern, it writes (data_wr & write_mask_array[i]) to each register via write_reg(addr, ...), skipping entries where skip_array[i]==1 or write_mask_array[i]==0x00000000. Then it reads back each register via read_reg(addr) masked with read_mask_array[i], computes expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff), and compares. Any mismatch increments wr_fail_cnt. Finally, if def_fail_cnt > 0 or wr_fail_cnt > 0, finish(1) is called (fail), otherwise finish(0) (pass). The soft_reset_chk() function is inside #ifdef 0 and is dead code.",
    "1. Entry: test_case() is called.\n2. chk_rst_val() is invoked for default value verification.\n3. Loop i from 0 to CNT-1:\n   a. addr = addr_array[i] (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, ...).\n   b. If skip_rst_array[i] == 1, skip this register (continue).\n   c. If read_mask_array[i] == 0x00000000, skip this register as not readable (continue).\n   d. data_rd = read_reg(addr).\n   e. data = (data_rd & 0xfffffffe) - mask out bit 0.\n   f. Compare data with default_value_array[i].\n   g. If mismatch: increment def_fail_cnt, print failure with address, expected, and read values.\n   h. If match: print pass (under DEBUG_DISPLAY).\n4. chk_rd_wr() is invoked for write-read verification.\n5. Define chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}.\n6. Outer loop j from 0 to 5 (6 test patterns):\n   a. data_wr = chk_val[j].\n   b. Write phase - inner loop i from 0 to CNT-1:\n      i. addr = addr_array[i].\n      ii. If skip_array[i] == 1, skip (continue).\n      iii. If write_mask_array[i] == 0x00000000, skip as not writable (continue).\n      iv. write_reg(addr, (data_wr & write_mask_array[i])).\n   c. Read-back phase - inner loop i from 0 to CNT-1:\n      i. addr = addr_array[i].\n      ii. If skip_array[i] == 1, skip (continue).\n      iii. If write_mask_array[i] == 0x00000000, skip (continue).\n      iv. If read_mask_array[i] == 0x00000000, skip as not readable (continue).\n      v. data_rd = (read_reg(addr) & read_mask_array[i]).\n      vi. wr_n = (write_mask_array[i] ^ 0xffffffff) - invert write mask.\n      vii. exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n      viii. Compare data_rd with exp_val.\n      ix. If mismatch: increment wr_fail_cnt, print failure.\n      x. If match: print pass (under DEBUG_DISPLAY).\n7. Return to test_case().\n8. If (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) - test FAIL.\n9. Else: finish(0) - test PASS.",
    "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
    "1. All GPIO registers must return their expected default reset values after reset (with bit 0 masked out). 2. For each of the six test data patterns, the write-read-back value of each register must match the expected value computed using the write mask, read mask, and default values. 3. Read-only bits must retain their default values after write operations. 4. Non-writable and non-readable registers must be correctly skipped. 5. The test passes only if zero mismatches are detected across all registers and all patterns.",
    "",
    "",
    ""
]
ws_md.append(md_row)

# Formatting
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

for ws in [ws_tp, ws_md]:
    for cell in ws[1]:
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = Alignment(wrap_text=True, vertical="center", horizontal="center")
    for row in ws.iter_rows(min_row=2):
        for cell in row:
            cell.alignment = wrap_alignment
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                if cell.value:
                    max_length = max(max_length, max(len(line) for line in str(cell.value).split('\n')))
            except:
                pass
        adjusted_width = min(max_length + 4, 60)
        ws.column_dimensions[col_letter].width = max(adjusted_width, 12)
    ws.freeze_panes = "A2"

ws_md.sheet_state = "veryHidden"

wb.save(output_path)
print(f"FILE:{filename}")
print(f"PATH:{output_path}")
