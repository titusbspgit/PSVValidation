#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
import base64, sys

wb = openpyxl.Workbook()
ws1 = wb.active
ws1.title = "TestPlan"

tp_headers = ["Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation (Required / Not)"]
hf = Font(bold=True, color="FFFFFF")
hfill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wa = Alignment(wrap_text=True, vertical="top")

for c, h in enumerate(tp_headers, 1):
    cell = ws1.cell(row=1, column=c, value=h)
    cell.font = hf; cell.fill = hfill; cell.alignment = wa

row = [1,"gpio","Register Write/Read","gpio_reg_wr_rd_test","Verify GPIO register write and read operations by writing known patterns to writable registers and reading back to confirm data integrity. Checks default reset values and write/read consistency for gp0_gpio_8, gp0_gpio_9, gp0_gpio_10 registers.","NA","NA","NA","NA","Registers with skip_array=1 are skipped for write/read. Registers with skip_rst_array=1 are skipped for reset value check.","1. Read default/reset values of all GPIO registers and verify against expected defaults.\n2. Write test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000) to each writable register.\n3. Read back each register and compare against expected values considering read/write masks.\n4. Report PASS/FAIL for each register access.\n5. If any mismatch is detected, test finishes with failure status.","gp0_gpio_8, gp0_gpio_9, gp0_gpio_10","1. All register default values must match expected reset values after masking.\n2. Written data must be read back correctly for all test patterns.\n3. def_fail_cnt and wr_fail_cnt must both be 0 for test to pass.\n4. Test must call finish(0) on success and finish(1) on any failure.","Not Required"]
for c, v in enumerate(row, 1):
    cell = ws1.cell(row=2, column=c, value=v); cell.alignment = wa

cw = [8,15,22,28,50,10,10,22,20,40,55,35,50,28]
for i, w in enumerate(cw, 1):
    ws1.column_dimensions[openpyxl.utils.get_column_letter(i)].width = w
ws1.freeze_panes = "A2"

ws2 = wb.create_sheet("MetaData")
ws2.sheet_state = "veryHidden"
md_headers = ["Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"]
mf = PatternFill(start_color="2F5496", end_color="2F5496", fill_type="solid")
for c, h in enumerate(md_headers, 1):
    cell = ws2.cell(row=1, column=c, value=h); cell.font = hf; cell.fill = mf; cell.alignment = wa

mr = [1,"gpio_reg_wr_rd_test","Verify GPIO register write and read operations by writing known patterns to writable registers and reading back to confirm data integrity.","1. Read default/reset values.\n2. Write test patterns.\n3. Read back and compare.\n4. Report PASS/FAIL.\n5. Finish with status.","gp0_gpio_8 (MIZAR_GPIO_GP0_GPIO_8) [matched], gp0_gpio_9 (MIZAR_GPIO_GP0_GPIO_9) [matched], gp0_gpio_10 (MIZAR_GPIO_GP0_GPIO_10) [matched]","1. Default values match expected.\n2. Write/read data matches.\n3. Zero fail counts.\n4. finish(0) on success.","gpio/gpio_def.h, gpio/gpio_offset.h","MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL, GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK, GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK","addr_array[49], default_value_array[49], read_mask_array[49], write_mask_array[49], skip_array[49], skip_rst_array[49]"]
for c, v in enumerate(mr, 1):
    cell = ws2.cell(row=2, column=c, value=v); cell.alignment = wa

mw = [8,28,50,55,50,50,35,55,45]
for i, w in enumerate(mw, 1):
    ws2.column_dimensions[openpyxl.utils.get_column_letter(i)].width = w
ws2.freeze_panes = "A2"

from io import BytesIO
buf = BytesIO()
wb.save(buf)
b64 = base64.b64encode(buf.getvalue()).decode()
print(b64)
