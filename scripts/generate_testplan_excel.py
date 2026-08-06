#!/usr/bin/env python3
import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment

# -------------------------
# Load input
# -------------------------
json_data = None
if os.path.exists('input.json'):
    with open('input.json','r',encoding='utf-8') as f:
        top = json.load(f)
    if isinstance(top, dict) and 'json_data' in top and isinstance(top['json_data'], list):
        json_data = top['json_data']
    elif isinstance(top, list):
        json_data = top

# Fallback to embedded data if no input.json provided
if json_data is None:
    json_blob = r'''[
  {
    "Index": "1",
    "SS / Module": "GPIO",
    "Feature": "AHB 32-bit register interface",
    "Test Case Name": "gpio_reg_wr_rd_test",
    "Test Description": "Validate GPIO register block by checking default reset values and verifying masked write/read behavior across a predefined register address list. The test iterates the unresolved register macro series MIZAR_GPIO_GP0_GPIO_8 through MIZAR_GPIO_GP0_GPIO_27, applies per-register write/read masks, writes data patterns, and confirms readback matches the expected value derived from masks and defaults. Test outcome is pass if no mismatches are detected; otherwise fail.",
    "Meta Test Description": "test_case(): calls chk_rst_val(); then calls chk_rd_wr(); finally calls finish(1) if (def_fail_cnt > 0 || wr_fail_cnt > 0) else finish(0).\n\nchk_rst_val():\n- for (i = 0; i < CNT; i++):\n  - addr = addr_array[i]  // addr_array contains {MIZAR_GPIO_GP0_GPIO_8 .. MIZAR_GPIO_GP0_GPIO_27}\n  - if (skip_rst_array[i] == 1): continue\n  - if (read_mask_array[i] == 0x00000000): continue\n  - data_rd = read_reg(addr)\n  - data = (data_rd & 0xfffffffe)\n  - if (data == default_value_array[i]): PASS else { def_fail_cnt++; printf failure }\n\nchk_rd_wr():\n- unsigned int chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}\n- for each pattern j in chk_val:\n  - data_wr = chk_val[j]\n  - Write phase for all i in [0..CNT-1]:\n    - addr = addr_array[i]\n    - if (skip_array[i] == 1): continue\n    - if (write_mask_array[i] == 0x00000000): continue\n    - write_reg(addr, (data_wr & write_mask_array[i]))\n  - Read/verify phase for all i in [0..CNT-1]:\n    - addr = addr_array[i]\n    - if (skip_array[i] == 1): continue\n    - if (write_mask_array[i] == 0x00000000): continue\n    - if (read_mask_array[i] == 0x00000000): continue\n    - data_rd = (read_reg(addr) & read_mask_array[i])\n    - wr_n = (write_mask_array[i] ^ 0xffffffff)\n    - exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]))\n    - if (data_rd == exp_val): PASS else { wr_fail_cnt++; printf failure }\n\nsoft_reset_chk(): disabled under #ifdef 0; would write/read SOFT_RST_REG_ADDRESS with SOFT_RST_REG_DATA and waits if enabled.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Commented note in source: when reading default values the DIN value can become 1 automatically if not forced; forcing zero to DIN may set level select high causing read mismatch against expected.",
    "Test Steps / Procedure": "1. Initialize test by preparing the register address list and associated default, read-mask, and write-mask tables for the GPIO block (addresses unresolved).\n2. For each address in the list, if readable by mask, read the register and compare the masked value against the expected default; record any mismatches.\n3. For each of six data patterns, write masked values to each writable register address in the list, skipping those marked to skip or not writable.\n4. Read back each addressed register using the read mask and compute the expected value from the previous write, the write mask, and the default; record any mismatches.\n5. Declare test PASS if no default-value mismatches and no write-read mismatches were recorded; otherwise declare FAIL.",
    "Meta Test Steps / Procedure": "1) test_case():\n   - chk_rst_val();\n   - chk_rd_wr();\n   - if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0);\n\n2) chk_rst_val():\n   - for (i = 0; i < CNT; i++):\n     - addr = addr_array[i]  // addr_array uses {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, ..., MIZAR_GPIO_GP0_GPIO_27}\n     - if (skip_rst_array[i] == 1) continue;\n     - if (read_mask_array[i] == 0x00000000) continue;\n     - data_rd = read_reg(addr);\n     - data = (data_rd & 0xfffffffe);\n     - if (data == default_value_array[i]) PASS else { def_fail_cnt++; printf failure };\n\n3) chk_rd_wr():\n   - chk_val = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000};\n   - for each data_wr in chk_val:\n     - Write loop for i in [0..CNT-1]:\n       - addr = addr_array[i]; if (skip_array[i] == 1) continue;\n       - if (write_mask_array[i] == 0x00000000) continue;\n       - write_reg(addr, (data_wr & write_mask_array[i]));\n     - Read/verify loop for i in [0..CNT-1]:\n       - addr = addr_array[i]; if (skip_array[i] == 1) continue;\n       - if (write_mask_array[i] == 0x00000000) continue;\n       - if (read_mask_array[i] == 0x00000000) continue;\n       - data_rd = (read_reg(addr) & read_mask_array[i]);\n       - wr_n = (write_mask_array[i] ^ 0xffffffff);\n       - exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]));\n       - if (data_rd == exp_val) PASS else { wr_fail_cnt++; printf failure };\n\n4) soft_reset_chk(): disabled under #ifdef 0; would interact with SOFT_RST_REG_ADDRESS and SOFT_RST_REG_DATA if enabled.",
    "Impacted Registers": "UNRESOLVED(MIZAR_GPIO_GP0_GPIO_8); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_9); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_10); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_11); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_12); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_13); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_14); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_15); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_16); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_17); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_18); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_19); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_20); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_21); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_22); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_23); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_24); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_25); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_26); UNRESOLVED(MIZAR_GPIO_GP0_GPIO_27); UNRESOLVED(GPIO_GP0_GPIO_8_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_9_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_10_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_11_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_12_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_13_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_14_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_15_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_16_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_17_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_18_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_19_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_20_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_21_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_22_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_23_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_24_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_25_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_26_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_27_DEFAULT_VAL); UNRESOLVED(GPIO_GP0_GPIO_8_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_9_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_10_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_11_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_12_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_13_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_14_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_15_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_16_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_17_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_18_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_19_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_20_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_21_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_22_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_23_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_24_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_25_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_26_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_27_READ_MASK); UNRESOLVED(GPIO_GP0_GPIO_8_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_9_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_10_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_11_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_12_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_13_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_14_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_15_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_16_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_17_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_18_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_19_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_20_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_21_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_22_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_23_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_24_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_25_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_26_WRITE_MASK); UNRESOLVED(GPIO_GP0_GPIO_27_WRITE_MASK",
    "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; MIZAR_GPIO_GP0_GPIO_11; MIZAR_GPIO_GP0_GPIO_12; MIZAR_GPIO_GP0_GPIO_13; MIZAR_GPIO_GP0_GPIO_14; MIZAR_GPIO_GP0_GPIO_15; MIZAR_GPIO_GP0_GPIO_16; MIZAR_GPIO_GP0_GPIO_17; MIZAR_GPIO_GP0_GPIO_18; MIZAR_GPIO_GP0_GPIO_19; MIZAR_GPIO_GP0_GPIO_20; MIZAR_GPIO_GP0_GPIO_21; MIZAR_GPIO_GP0_GPIO_22; MIZAR_GPIO_GP0_GPIO_23; MIZAR_GPIO_GP0_GPIO_24; MIZAR_GPIO_GP0_GPIO_25; MIZAR_GPIO_GP0_GPIO_26; MIZAR_GPIO_GP0_GPIO_27; GPIO_GP0_GPIO_8_DEFAULT_VAL; GPIO_GP0_GPIO_9_DEFAULT_VAL; GPIO_GP0_GPIO_10_DEFAULT_VAL; GPIO_GP0_GPIO_11_DEFAULT_VAL; GPIO_GP0_GPIO_12_DEFAULT_VAL; GPIO_GP0_GPIO_13_DEFAULT_VAL; GPIO_GP0_GPIO_14_DEFAULT_VAL; GPIO_GP0_GPIO_15_DEFAULT_VAL; GPIO_GP0_GPIO_16_DEFAULT_VAL; GPIO_GP0_GPIO_17_DEFAULT_VAL; GPIO_GP0_GPIO_18_DEFAULT_VAL; GPIO_GP0_GPIO_19_DEFAULT_VAL; GPIO_GP0_GPIO_20_DEFAULT_VAL; GPIO_GP0_GPIO_21_DEFAULT_VAL; GPIO_GP0_GPIO_22_DEFAULT_VAL; GPIO_GP0_GPIO_23_DEFAULT_VAL; GPIO_GP0_GPIO_24_DEFAULT_VAL; GPIO_GP0_GPIO_25_DEFAULT_VAL; GPIO_GP0_GPIO_26_DEFAULT_VAL; GPIO_GP0_GPIO_27_DEFAULT_VAL; GPIO_GP0_GPIO_8_READ_MASK; GPIO_GP0_GPIO_9_READ_MASK; GPIO_GP0_GPIO_10_READ_MASK; GPIO_GP0_GPIO_11_READ_MASK; GPIO_GP0_GPIO_12_READ_MASK; GPIO_GP0_GPIO_13_READ_MASK; GPIO_GP0_GPIO_14_READ_MASK; GPIO_GP0_GPIO_15_READ_MASK; GPIO_GP0_GPIO_16_READ_MASK; GPIO_GP0_GPIO_17_READ_MASK; GPIO_GP0_GPIO_18_READ_MASK; GPIO_GP0_GPIO_19_READ_MASK; GPIO_GP0_GPIO_20_READ_MASK; GPIO_GP0_GPIO_21_READ_MASK; GPIO_GP0_GPIO_22_READ_MASK; GPIO_GP0_GPIO_23_READ_MASK; GPIO_GP0_GPIO_24_READ_MASK; GPIO_GP0_GPIO_25_READ_MASK; GPIO_GP0_GPIO_26_READ_MASK; GPIO_GP0_GPIO_27_READ_MASK; GPIO_GP0_GPIO_8_WRITE_MASK; GPIO_GP0_GPIO_9_WRITE_MASK; GPIO_GP0_GPIO_10_WRITE_MASK; GPIO_GP0_GPIO_11_WRITE_MASK; GPIO_GP0_GPIO_12_WRITE_MASK; GPIO_GP0_GPIO_13_WRITE_MASK; GPIO_GP0_GPIO_14_WRITE_MASK; GPIO_GP0_GPIO_15_WRITE_MASK; GPIO_GP0_GPIO_16_WRITE_MASK; GPIO_GP0_GPIO_17_WRITE_MASK; GPIO_GP0_GPIO_18_WRITE_MASK; GPIO_GP0_GPIO_19_WRITE_MASK; GPIO_GP0_GPIO_20_WRITE_MASK; GPIO_GP0_GPIO_21_WRITE_MASK; GPIO_GP0_GPIO_22_WRITE_MASK; GPIO_GP0_GPIO_23 WRITE_MASK; GPIO_GP0_GPIO_24_WRITE_MASK; GPIO_GP0_GPIO_25_WRITE_MASK; GPIO_GP0_GPIO_26_WRITE_MASK; GPIO_GP0_GPIO_27_WRITE_MASK",
    "Validation / Acceptance Criteria": "Pass if for all addressed registers UNRESOLVED(MIZAR_GPIO_GP0_GPIO_8) through UNRESOLVED(MIZAR_GPIO_GP0_GPIO_27): (1) the masked default read equals the expected default value; and (2) for each data pattern, the masked readback equals the expected value derived from the write mask, read mask, and default value. Otherwise, the test fails.",
    "Meta Validation / Acceptance Criteria": "- Default check: for each i, data = (read_reg(addr_array[i]) & 0xfffffffe); pass if data == default_value_array[i]; else def_fail_cnt++.\n- Write/Read check for each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000} and each i:\n  - Only if (write_mask_array[i] != 0) and (read_mask_array[i] != 0) and skip_array[i] == 0.\n  - data_rd = (read_reg(addr_array[i]) & read_mask_array[i]).\n  - wr_n = (write_mask_array[i] ^ 0xffffffff).\n  - exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])).\n  - pass if (data_rd == exp_val); else wr_fail_cnt++.\n- Final decision: finish(1) if (def_fail_cnt > 0 || wr_fail_cnt > 0); else finish(0).",
    "Code Generation (Required / Not)": "Not Required",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include<gpio/gpio_def.h>\n#include<gpio/gpio_offset.h>",
    "Meta Macros": "#define CNT 49\n#define SOFT_RST_REG ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000",
    "Meta Arrays": "const unsigned long int addr_array[20]={MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,};\nconst unsigned int default_value_array[20]={GPIO_GP0_GPIO_8_DEFAULT_VAL,GPIO_GP0_GPIO_9_DEFAULT_VAL,GPIO_GP0_GPIO_10_DEFAULT_VAL,GPIO_GP0_GPIO_11_DEFAULT_VAL,GPIO_GP0_GPIO_12_DEFAULT_VAL,GPIO_GP0_GPIO_13_DEFAULT_VAL,GPIO_GP0_GPIO_14_DEFAULT_VAL,GPIO_GP0_GPIO_15_DEFAULT_VAL,GPIO_GP0_GPIO_16_DEFAULT_VAL,GPIO_GP0_GPIO_17_DEFAULT_VAL,GPIO_GP0_GPIO_18_DEFAULT_VAL,GPIO_GP0_GPIO_19_DEFAULT_VAL,GPIO_GP0_GPIO_20_DEFAULT_VAL,GPIO_GP0_GPIO_21_DEFAULT_VAL,GPIO_GP0_GPIO_22_DEFAULT_VAL,GPIO_GP0_GPIO_23_DEFAULT_VAL,GPIO_GP0_GPIO_24_DEFAULT_VAL,GPIO_GP0_GPIO_25_DEFAULT_VAL,GPIO_GP0_GPIO_26_DEFAULT_VAL,GPIO_GP0_GPIO_27_DEFAULT_VAL,};\nconst unsigned int read_mask_array[20]={GPIO_GP0_GPIO_8_READ_MASK,GPIO_GP0_GPIO_9_READ_MASK,GPIO_GP0_GPIO_10_READ_MASK,GPIO_GP0_GPIO_11_READ_MASK,GPIO_GP0_GPIO_12_READ_MASK,GPIO_GP0_GPIO_13_READ_MASK,GPIO_GP0_GPIO_14_READ_MASK,GPIO_GP0_GPIO_15_READ_MASK,GPIO_GP0_GPIO_16_READ_MASK,GPIO_GP0_GPIO_17_READ_MASK,GPIO_GP0_GPIO_18_READ_MASK,GPIO_GP0_GPIO_19_READ_MASK,GPIO_GP0_GPIO_20_READ_MASK,GPIO_GP0_GPIO_21_READ_MASK,GPIO_GP0_GPIO_22_READ_MASK,GPIO_GP0_GPIO_23_READ_MASK,GPIO_GP0_GPIO_24_READ_MASK,GPIO_GP0_GPIO_25_READ_MASK,GPIO_GP0_GPIO_26_READ_MASK,GPIO_GP0_GPIO_27_READ_MASK,};\nconst unsigned int write_mask_array[20]={GPIO_GP0_GPIO_8_WRITE_MASK,GPIO_GP0_GPIO_9_WRITE_MASK,GPIO_GP0_GPIO_10_WRITE_MASK,GPIO_GP0_GPIO_11_WRITE_MASK,GPIO_GP0_GPIO_12_WRITE_MASK,GPIO_GP0_GPIO_13_WRITE_MASK,GPIO_GP0_GPIO_14_WRITE_MASK,GPIO_GP0_GPIO_15_WRITE_MASK,GPIO_GP0_GPIO_16_WRITE_MASK,GPIO_GP0_GPIO_17_WRITE_MASK,GPIO_GP0_GPIO_18_WRITE_MASK,GPIO_GP0_GPIO_19_WRITE_MASK,GPIO_GP0_GPIO_20_WRITE_MASK,GPIO_GP0_GPIO_21_WRITE_MASK,GPIO_GP0_GPIO_22_WRITE_MASK,GPIO_GP0_GPIO_23_WRITE_MASK,GPIO_GP0_GPIO_24_WRITE_MASK,GPIO_GP0_GPIO_25_WRITE_MASK,GPIO_GP0_GPIO_26_WRITE_MASK,GPIO_GP0_GPIO_27_WRITE_MASK,}"
  }
]'''
    json_data = json.loads(json_blob)

# Validation
if not isinstance(json_data, list):
    raise SystemExit("json_data must be an array")

# Constants
OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
IP_NAME = "GPIO"
TIMEZONE = "Asia/Kolkata"

TESTPLAN_HEADERS = [
    "Index","SS / Module","Feature","Test Case Name","Test Description","Speed","Mode","Memory Start Offset","Memory End Offset","Remarks","Test Steps / Procedure","Impacted Registers","Validation / Acceptance Criteria","Code Generation (Required / Not)"
]

METADATA_HEADERS = [
    "Index","Test Case Name","Meta Test Description","Meta Test Steps / Procedure","Meta Impacted Registers","Meta Validation / Acceptance Criteria","Meta Headers","Meta Macros","Meta Arrays"
]

wb = Workbook()
ws_plan = wb.active
ws_plan.title = "TestPlan"
ws_meta = wb.create_sheet("MetaData")

header_font = Font(bold=True, color="FFFFFF")
header_fill = PatternFill(start_color="1F4E78", end_color="1F4E78", fill_type="solid")
wrap_top = Alignment(wrap_text=True, vertical="top")

ws_plan.append(TESTPLAN_HEADERS)
ws_meta.append(METADATA_HEADERS)
for cell in ws_plan[1]:
    cell.font = header_font; cell.fill = header_fill; cell.alignment = wrap_top
for cell in ws_meta[1]:
    cell.font = header_font; cell.fill = header_fill; cell.alignment = wrap_top

ws_plan.freeze_panes = "A2"
ws_meta.freeze_panes = "A2"

for obj in json_data:
    ws_plan.append([obj.get(k, "") for k in TESTPLAN_HEADERS])
    ws_meta.append([obj.get(k, "") for k in METADATA_HEADERS])

# Set column widths
plan_widths = [6, 14, 26, 26, 80, 10, 10, 20, 20, 36, 80, 80, 80, 24]
for i, w in enumerate(plan_widths, start=1):
    ws_plan.column_dimensions[chr(64 + i)].width = w
meta_widths = [6, 26, 80, 80, 80, 80, 60, 40, 60]
for i, w in enumerate(meta_widths, start=1):
    ws_meta.column_dimensions[chr(64 + i)].width = w

for row in ws_plan.iter_rows(min_row=2, max_row=ws_plan.max_row, min_col=1, max_col=len(TESTPLAN_HEADERS)):
    for c in row: c.alignment = wrap_top
for row in ws_meta.iter_rows(min_row=2, max_row=ws_meta.max_row, min_col=1, max_col=len(METADATA_HEADERS)):
    for c in row: c.alignment = wrap_top

ws_plan.auto_filter.ref = ws_plan.dimensions
ws_meta.auto_filter.ref = ws_meta.dimensions

ws_meta.sheet_state = "veryHidden"

ist_now = datetime.now(ZoneInfo(TIMEZONE))
ts = ist_now.strftime("%Y%m%d_%H%M%S")
filename = f"{IP_NAME}_TestPlan_{ts}.xlsx"

os.makedirs(OUTPUT_DIR, exist_ok=True)
out_path = os.path.join(OUTPUT_DIR, filename)
wb.save(out_path)

# Reopen and validate
wb2 = load_workbook(out_path)
if "TestPlan" not in wb2.sheetnames or "MetaData" not in wb2.sheetnames:
    raise SystemExit("Validation failed: required sheets not found")
if getattr(wb2["MetaData"], "sheet_state", "visible") != "veryHidden":
    raise SystemExit("Validation failed: MetaData is not VeryHidden")

input_rows = len(json_data)
plan_rows = ws_plan.max_row - 1
meta_rows = ws_meta.max_row - 1
if not (plan_rows == input_rows == meta_rows):
    raise SystemExit("Validation failed: Row counts do not match input")

print(out_path)
