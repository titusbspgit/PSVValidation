#!/usr/bin/env python3
"""Auto-generated TestPlan Excel Generator - Run once and delete."""
import json, os, sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"GPIO_TestPlan_{timestamp}.xlsx"

json_data = [{"index":1,"testcase_name":"gpio_reg_wr_rd_test","ss_module":"GPIO","feature":"Register Read Write Validation","meta_test_description":"This testcase validates the GPIO register block (gp0) by performing two main checks: (1) chk_rst_val() reads all 49 registers via addr_array[] (MIZAR_GPIO_GP0_GPIO_8 through the full set) using read_reg(addr), masks the read data with 0xFFFFFFFE, and compares against default_value_array[]. Registers with skip_rst_array[i]==1 or read_mask_array[i]==0x00000000 are skipped. (2) chk_rd_wr() iterates over 6 write patterns from chk_val[]={0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}. For each pattern, it writes (data_wr & write_mask_array[i]) to each register via write_reg(addr, ...), then reads back via read_reg(addr), masks with read_mask_array[i], and computes expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xFFFFFFFF). Registers with skip_array[i]==1 or write_mask_array[i]==0x00000000 or read_mask_array[i]==0x00000000 are skipped. The test calls finish(1) on any failure (def_fail_cnt > 0 or wr_fail_cnt > 0), otherwise finish(0). A soft_reset_chk() function exists but is disabled via #ifdef 0. CNT is defined as 49. The addr_array contains macros MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10 (only 3 shown, array declared as 49). Headers included: gpio/gpio_def.h, gpio/gpio_offset.h, test_common.h. VRRW registers are skipped via skip_array (indices 32, 37-44 set to 1). skip_rst_array skips indices 37-48 for reset value check.","test_description":"This test validates the GPIO register block by performing reset value verification and write-read verification across all 49 GPIO registers. First, it reads each register after reset and verifies the value matches the expected default. Second, it writes six distinct data patterns (all-ones, alternating bits, nibble patterns, and half-word pattern) to each writable register, reads back the value, and verifies correctness using the register's read and write masks. Registers that are not readable or not writable are automatically skipped. VRRW-type registers are excluded from write-read checks as configured. The test reports pass if all comparisons succeed, or fail if any mismatch is detected.","meta_test_steps":"1. Include headers: stdio.h, stdlib.h, test_common.h, test_define.c (which includes gpio/gpio_def.h, gpio/gpio_offset.h).\n2. Define global variables: data_rd, data_wr, data, def_fail_cnt=0, wr_fail_cnt=0.\n3. Define SOFT_RST_REG_ADDRESS=0x00000000 and SOFT_RST_REG_DATA=0x00000000 (unused, inside #ifdef 0 block).\n4. Define CNT=49.\n5. Initialize addr_array[49] with register address macros: {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, ...}.\n6. Initialize default_value_array[49] with *_DEFAULT_VAL macros.\n7. Initialize read_mask_array[49] with *_READ_MASK macros.\n8. Initialize write_mask_array[49] with *_WRITE_MASK macros.\n9. Initialize skip_array[49] = {0,0,0,...,1,...,1,1,1,1,1,1,1,1,0,0,0,0} (VRRW registers skipped at indices 32, 37-44).\n10. Initialize skip_rst_array[49] = {0,0,0,...,0,...,1,1,1,1,1,1,1,1,1,1,1,1} (indices 37-48 skipped for reset check).\n11. test_case() entry: call chk_rst_val().\n12. chk_rst_val(): for i=0 to CNT-1: addr = addr_array[i]; if skip_rst_array[i]==1 then continue; if read_mask_array[i]==0x00000000 then continue; data_rd = read_reg(addr); data = (data_rd & 0xFFFFFFFE); if data == default_value_array[i] then PASS else def_fail_cnt++.\n13. test_case(): call chk_rd_wr().\n14. chk_rd_wr(): define chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000}.\n15. For j=0 to 5: data_wr = chk_val[j].\n16. Write phase: for i=0 to CNT-1: addr = addr_array[i]; if skip_array[i]==1 then continue; if write_mask_array[i]==0x00000000 then continue; write_reg(addr, (data_wr & write_mask_array[i])).\n17. Read-verify phase: for i=0 to CNT-1: addr = addr_array[i]; if skip_array[i]==1 then continue; if write_mask_array[i]==0x00000000 then continue; if read_mask_array[i]==0x00000000 then continue; data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xFFFFFFFF); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd == exp_val then PASS else wr_fail_cnt++.\n18. test_case(): if def_fail_cnt > 0 or wr_fail_cnt > 0 then finish(1) else finish(0).\n19. soft_reset_chk(): disabled via #ifdef 0 - would read SOFT_RST_REG_ADDRESS, write SOFT_RST_REG_DATA, wait 1000, restore, wait 1000.","test_steps":"1. Initialize the test environment and load the GPIO register configuration arrays containing 49 register entries with their addresses, default values, read masks, write masks, and skip conditions.\n2. Perform reset value verification: For each of the 49 GPIO registers (gp0_gpio_8 through the full register set), read the register value after reset.\n3. Skip registers marked in the reset-skip configuration or registers that are not readable (read mask is zero).\n4. Compare each read value (masked with 0xFFFFFFFE to exclude bit 0 variability) against the expected default value from the register specification.\n5. Record any reset value mismatches as failures.\n6. Perform write-read verification: For each of six test patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000), write the pattern (masked with the write mask) to each writable register.\n7. Skip registers marked in the write-read skip configuration, non-writable registers (write mask is zero), and VRRW-type registers.\n8. Read back each written register and apply the read mask.\n9. Compute the expected read-back value accounting for read-only bits retaining their default values and writable bits reflecting the written pattern.\n10. Compare the actual read-back value against the computed expected value and record any mismatches.\n11. Report overall test result: PASS if no mismatches detected in either reset value check or write-read check; FAIL otherwise.","meta_impacted_registers":"MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10","impacted_registers":"gp0_gpio_8; gp0_gpio_9; gp0_gpio_10","validation_criteria":"1. All GPIO registers must return their expected default values after reset (with bit 0 masked out). 2. For each of the six write patterns, all writable register fields must correctly reflect the written data when read back through the read mask. 3. Read-only fields must retain their default values regardless of write attempts. 4. Non-readable and non-writable registers must be correctly skipped without errors. 5. VRRW-type registers must be excluded from write-read checks as configured. 6. The test must report PASS (finish 0) when all checks succeed and FAIL (finish 1) when any mismatch is detected.","remarks":"The soft reset check function is disabled (compiled out). VRRW-type registers (interrupt raw status/clear, IO control groups, data-out groups, data-in groups) are skipped during write-read verification via the skip array. The reset value check masks out bit 0 using 0xFFFFFFFE to handle data_in pin level variability. The register block uses AHB bus interface with 32-bit address width and hreset_n as the active-low asynchronous reset signal.","speed":"NA","mode":"NA","register_details":[{"register_name":"gp0_gpio_8","macro":"MIZAR_GPIO_GP0_GPIO_8","base_address":"0xA001A000","offset":"0x0","absolute_address":"0xA001A000","width":32,"reset_signal":"hreset_n","operation":"read_modify_write","fields":[{"field_name":"data_in","bit_index":0,"access_type":"RO","reset_value":0,"description":"Determines the level gpio pin. Read returns the current value of the corresponding pin regardless of whether the pin is an input or an output"},{"field_name":"intr_raw_sts","bit_index":1,"access_type":"RO","reset_value":0,"description":"RAWST bit: Interrupt raw status On Read returns latched raw interrupt status based on interrupt settings"},{"field_name":"intr_clr","bit_index":16,"access_type":"WO","reset_value":0,"description":"Writing 1 clears the interrupt raw status. Writing 0 No change"},{"field_name":"pedge_intr_en","bit_index":17,"access_type":"RW","reset_value":0,"description":"Pos-edge interrupt enable; 1-Enable interrupt when rising edge is detected on gpio pin; 0-Disable interrupt on pos-edge detection"},{"field_name":"nedge_intr_en","bit_index":18,"access_type":"RW","reset_value":0,"description":"Neg-edge interrupt enable; 1-Enable interrupt when falling edge is detected on gpio pin; 0-Disable interrupt on neg-edge detection"},{"field_name":"level_sel","bit_index":19,"access_type":"RW","reset_value":0,"description":"GPIO Pin signal level on which interrupt should be generated. 0-generate interrupt on level 0; 1-generate interrupt on level 1"},{"field_name":"io_ctrl","bit_index":20,"access_type":"RW2","reset_value":1,"description":"GPIO mode select: selects gpio either in output mode or input mode; 0-Set GPIO in output mode; 1-Set GPIO in input mode"},{"field_name":"dout","bit_index":21,"access_type":"RW2","reset_value":0,"description":"dout: Value that to be written to gpio pin in output mode"}]},{"register_name":"gp0_gpio_9","macro":"MIZAR_GPIO_GP0_GPIO_9","base_address":"0xA001A000","offset":"0x4","absolute_address":"0xA001A004","width":32,"reset_signal":"hreset_n","operation":"read_modify_write","fields":[{"field_name":"data_in","bit_index":0,"access_type":"RO","reset_value":0,"description":"Determines the level gpio pin. Read returns the current value of the corresponding pin regardless of whether the pin is an input or an output"},{"field_name":"intr_raw_sts","bit_index":1,"access_type":"RO","reset_value":0,"description":"RAWST bit: Interrupt raw status On Read returns latched raw interrupt status based on interrupt settings"},{"field_name":"intr_clr","bit_index":16,"access_type":"WO","reset_value":0,"description":"Writing 1 clears the interrupt raw status. Writing 0 No change"},{"field_name":"pedge_intr_en","bit_index":17,"access_type":"RW","reset_value":0,"description":"Pos-edge interrupt enable; 1-Enable interrupt when rising edge is detected on gpio pin; 0-Disable interrupt on pos-edge detection"},{"field_name":"nedge_intr_en","bit_index":18,"access_type":"RW","reset_value":0,"description":"Neg-edge interrupt enable; 1-Enable interrupt when falling edge is detected on gpio pin; 0-Disable interrupt on neg-edge detection"},{"field_name":"level_sel","bit_index":19,"access_type":"RW","reset_value":0,"description":"GPIO Pin signal level on which interrupt should be generated. 0-generate interrupt on level 0; 1-generate interrupt on level 1"},{"field_name":"io_ctrl","bit_index":20,"access_type":"RW2","reset_value":1,"description":"GPIO mode select: selects gpio either in output mode or input mode; 0-Set GPIO in output mode; 1-Set GPIO in input mode"},{"field_name":"dout","bit_index":21,"access_type":"RW2","reset_value":0,"description":"dout: Value that to be written to gpio pin in output mode"}]},{"register_name":"gp0_gpio_10","macro":"MIZAR_GPIO_GP0_GPIO_10","base_address":"0xA001A000","offset":"0x8","absolute_address":"0xA001A008","width":32,"reset_signal":"hreset_n","operation":"read_modify_write","fields":[{"field_name":"data_in","bit_index":0,"access_type":"RO","reset_value":0,"description":"Determines the level gpio pin. Read returns the current value of the corresponding pin regardless of whether the pin is an input or an output"},{"field_name":"intr_raw_sts","bit_index":1,"access_type":"RO","reset_value":0,"description":"RAWST bit: Interrupt raw status On Read returns latched raw interrupt status based on interrupt settings"},{"field_name":"intr_clr","bit_index":16,"access_type":"WO","reset_value":0,"description":"Writing 1 clears the interrupt raw status. Writing 0 No change"},{"field_name":"pedge_intr_en","bit_index":17,"access_type":"RW","reset_value":0,"description":"Pos-edge interrupt enable; 1-Enable interrupt when rising edge is detected on gpio pin; 0-Disable interrupt on pos-edge detection"},{"field_name":"nedge_intr_en","bit_index":18,"access_type":"RW","reset_value":0,"description":"Neg-edge interrupt enable; 1-Enable interrupt when falling edge is detected on gpio pin; 0-Disable interrupt on neg-edge detection"},{"field_name":"level_sel","bit_index":19,"access_type":"RW","reset_value":0,"description":"GPIO Pin signal level on which interrupt should be generated. 0-generate interrupt on level 0; 1-generate interrupt on level 1"},{"field_name":"io_ctrl","bit_index":20,"access_type":"RW2","reset_value":1,"description":"GPIO mode select: selects gpio either in output mode or input mode; 0-Set GPIO in output mode; 1-Set GPIO in input mode"},{"field_name":"dout","bit_index":21,"access_type":"RW2","reset_value":0,"description":"dout: Value that to be written to gpio pin in output mode"}]}]}]

# Create workbook
wb = Workbook()
ws_tp = wb.active
ws_tp.title = "TestPlan"
ws_md = wb.create_sheet("MetaData")

# Header styles
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_align = Alignment(wrap_text=True, vertical="top")

# TestPlan columns
tp_headers = ["Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
              "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
              "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
              "Code Generation"]

# MetaData columns
md_headers = ["Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
              "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
              "Meta Headers", "Meta Macros", "Meta Arrays"]

# Write headers
for col_idx, h in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=h)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for col_idx, h in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=h)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Populate data
for row_idx, item in enumerate(json_data, 2):
    # Build Code Generation content
    code_gen_parts = []
    for reg in item.get("register_details", []):
        reg_line = f"Register: {reg['register_name']} | Macro: {reg['macro']} | Base: {reg['base_address']} | Offset: {reg['offset']} | Addr: {reg['absolute_address']} | Width: {reg['width']} | Reset: {reg['reset_signal']} | Op: {reg['operation']}"
        code_gen_parts.append(reg_line)
        for f in reg.get("fields", []):
            field_line = f"  Field: {f['field_name']} | Bit: {f['bit_index']} | Access: {f['access_type']} | Reset: {f['reset_value']} | Desc: {f['description']}"
            code_gen_parts.append(field_line)
    code_gen = "\n".join(code_gen_parts)

    # Extract headers/macros/arrays from meta
    meta_headers = "gpio/gpio_def.h; gpio/gpio_offset.h; test_common.h"
    meta_macros = "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; CNT=49"
    meta_arrays = "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]"

    # TestPlan row
    tp_row = [
        item.get("index", ""),
        item.get("ss_module", ""),
        item.get("feature", ""),
        item.get("testcase_name", ""),
        item.get("test_description", ""),
        item.get("speed", ""),
        item.get("mode", ""),
        "",  # Memory Start Offset
        "",  # Memory End Offset
        item.get("remarks", ""),
        item.get("test_steps", ""),
        item.get("impacted_registers", ""),
        item.get("validation_criteria", ""),
        code_gen
    ]
    for col_idx, val in enumerate(tp_row, 1):
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

    # MetaData row
    md_row = [
        item.get("index", ""),
        item.get("testcase_name", ""),
        item.get("meta_test_description", ""),
        item.get("meta_test_steps", ""),
        item.get("meta_impacted_registers", ""),
        item.get("validation_criteria", ""),
        meta_headers,
        meta_macros,
        meta_arrays
    ]
    for col_idx, val in enumerate(md_row, 1):
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

# Freeze first row
ws_tp.freeze_panes = "A2"
ws_md.freeze_panes = "A2"

# Auto-size columns
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_len = 0
        col_letter = col[0].column_letter
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_len = max(max_len, len(line))
        adjusted = min(max_len + 2, 60)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Save
output_dir = "Test_Output/GPIO/TestPlan"
os.makedirs(output_dir, exist_ok=True)
filepath = os.path.join(output_dir, filename)
wb.save(filepath)
print(f"GENERATED:{filepath}")
print(f"FILENAME:{filename}")
