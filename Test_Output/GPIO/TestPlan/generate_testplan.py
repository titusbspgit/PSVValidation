#!/usr/bin/env python3
"""
Agent 7 - Excel Generator Script
Generates GPIO_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx
This script is temporary and should be deleted after execution.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"GPIO_TestPlan_{timestamp}.xlsx"

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "GPIO",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Feature": "Register Write-Read Verification",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"test_define.c\"; <gpio/gpio_def.h>; <gpio/gpio_offset.h>",
        "Meta Macros": "SOFT_RST_REG_DATA; CNT",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase performs two phases of register validation on GPIO GP0 registers (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10). Phase 1 (chk_rst_val): Iterates over addr_array[49], skips entries where skip_rst_array[i]==1 or read_mask_array[i]==0x00000000, reads each register via read_reg(addr), masks the read data with 0xfffffffe, and compares against default_value_array[i]. Mismatches increment def_fail_cnt. Phase 2 (chk_rd_wr): Iterates over 6 test patterns in chk_val[6]={0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern, writes (data_wr & write_mask_array[i]) to each register via write_reg(addr, ...), skipping entries where skip_array[i]==1 or write_mask_array[i]==0x00000000. Then reads back each register via read_reg(addr), masks with read_mask_array[i], computes expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff), and compares. Mismatches increment wr_fail_cnt. Finally, test_case() calls finish(1) if either fail counter is > 0, otherwise finish(0).",
        "Test Description": "This test validates the GPIO GP0 registers (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) in two phases. Phase 1 verifies that each register's default (reset) value matches the expected default value after applying a read mask. Phase 2 writes six distinct test patterns to each register (respecting write masks), reads back the values (respecting read masks), computes the expected result considering writable and non-writable bit fields, and verifies correctness. The test passes only if all default value checks and all write-read checks succeed across all registers and all patterns.",
        "Meta Test Steps / Procedure": "1. Enter test_case(). 2. Call chk_rst_val(): For i=0 to CNT-1, set addr=addr_array[i]. If skip_rst_array[i]==1, skip (continue). If read_mask_array[i]==0x00000000, skip (continue). Read data_rd=read_reg(addr). Compute data=(data_rd & 0xfffffffe). Compare data against default_value_array[i]. If mismatch, increment def_fail_cnt and print failure. 3. Call chk_rd_wr(): Initialize chk_val[6]={0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For j=0 to 5, set data_wr=chk_val[j]. 3a. Write phase: For i=0 to CNT-1, set addr=addr_array[i]. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. Else write_reg(addr, (data_wr & write_mask_array[i])). 3b. Read-verify phase: For i=0 to CNT-1, set addr=addr_array[i]. If skip_array[i]==1, skip. If write_mask_array[i]==0x00000000, skip. If read_mask_array[i]==0x00000000, skip. Else read data_rd=(read_reg(addr) & read_mask_array[i]). Compute wr_n=(write_mask_array[i] ^ 0xffffffff). Compute exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Compare data_rd against exp_val. If mismatch, increment wr_fail_cnt. 4. Check results: If def_fail_cnt > 0 or wr_fail_cnt > 0, call finish(1) (fail). Else call finish(0) (pass).",
        "Test Steps / Procedure": "1. Read the default (reset) value of each GPIO GP0 register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10), skipping registers flagged in the reset-skip list or those that are not readable.\n2. Mask each read value and compare against the expected default value for that register. Record any mismatches.\n3. For each of six predefined test patterns, write the pattern to each register after applying the write mask, skipping registers flagged in the skip list or those that are not writable.\n4. After writing each pattern, read back each register value, applying the read mask.\n5. Compute the expected read-back value by combining the written data (masked by both read and write masks) with the default value for non-writable bits.\n6. Compare the actual read-back value against the computed expected value. Record any mismatches.\n7. Repeat steps 3 through 6 for all six test patterns.\n8. Determine overall test result: pass if no default-value mismatches and no write-read mismatches occurred; fail otherwise.",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Meta Validation / Acceptance Criteria": "Phase 1 (Default Value Check): For each register in addr_array (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10), read_reg(addr) is called, result is masked with 0xfffffffe, and compared to default_value_array[i]. If mismatch, def_fail_cnt is incremented. Phase 2 (Write-Read Check): For each of 6 patterns in chk_val[6], write_reg(addr, data_wr & write_mask_array[i]) is called, then read_reg(addr) is called and masked with read_mask_array[i]. Expected value is computed as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). If data_rd != exp_val, wr_fail_cnt is incremented. Final pass condition: def_fail_cnt == 0 AND wr_fail_cnt == 0 results in finish(0) (pass). Any non-zero fail counter results in finish(1) (fail).",
        "Validation / Acceptance Criteria": "1. All GPIO GP0 registers (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) must return their expected default values when read after reset, after applying the read mask.\n2. For each of the six test patterns written to each register, the read-back value (masked by the read mask) must match the expected value computed from the written data, write mask, read mask, and default values for non-writable bits.\n3. The test passes (finish with success) only if zero default-value mismatches and zero write-read mismatches are recorded across all registers and all test patterns.\n4. Any single mismatch in either phase causes the test to fail (finish with failure).",
        "Remarks": "The test uses a skip mechanism via skip_array and skip_rst_array to selectively bypass certain register indices during write-read and default-value checks respectively. The read data in the default value check phase is masked with a fixed mask that clears bit 0 before comparison. Six distinct bit patterns are used to exercise various bit combinations across writable fields. The soft_reset_chk function is disabled (inside #ifdef 0 block) and is not executed. A comment in test_define.c notes that the din value may automatically become 1 if not forced, affecting level select behavior and default value matching."
    },
    {
        "Index": "2",
        "SS / Module": "GPIO",
        "Test Case Name": "test_gpio_level_sel_intr_en",
        "Feature": "GPIO Level-Select Interrupt Enable Verification",
        "Meta Headers": "<lss_sysreg.h>; <stdio.h>; <test_define.c>; <test_common.h>; <gpio/gpio_def.h>; <gpio/gpio_offset.h>",
        "Meta Macros": "CNT",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates GPIO level-sensitive interrupt enable functionality for GPIO pads 8 through 39 (32 GPIOs). It operates in two phases. Phase 1 (Active High Level Interrupt): Enables GIC IRQ (87 for GPIO0, 88 for GPIO1). Writes MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO0_INTR (or GPIO1_INTR) to enable sysreg-level interrupt. For each GPIO i (0 to 31): writes 0x00180000 to MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to configure input mode with active-high level interrupt (bits 20 and 19 set). Calls wait_on(50). Writes wr_val = (1 << i) to MIZAR_GPIO_GP0_INTR1_INTR_EN1 to enable the group interrupt for that GPIO. Writes 0xffffffff to 0xA0243ffc (SRAM location). Sets int_pend = 1 and polls while(int_pend == 1) with wait_on(10) until the IRQ handler clears it. Phase 2 (Active Low Level Interrupt): For each GPIO i (0 to 31): writes 0x00100000 to MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to configure input mode with active-low level interrupt (bit 20 set, bit 19 clear). Calls wait_on(50). Writes wr_val = (1 << i) to MIZAR_GPIO_GP0_INTR1_INTR_EN1. Writes ~(wr_val) to 0xA0243ffc. Sets int_pend = 1 and polls until IRQ handler clears it. In Default_IRQHandler(): Writes 0xffffffff to 0xA0243ffc. Reads MIZAR_GPIO_GP0_GPIO_8 + (i * 4) into rdata. Checks (rdata & 0x2) != 0x0 for raw interrupt status bit. Reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp. Checks (rdata_grp & (1 << i)) != 0 for group interrupt status. Writes 0x00110000 to MIZAR_GPIO_GP0_GPIO_8 + (i * 4) to clear interrupt (bit 16). Calls wait_on(20). Reads back MIZAR_GPIO_GP0_GPIO_8 + (i * 4) and checks rdata == 0x100001. Writes 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1 to disable group interrupt. Reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and checks rdata_grp == 0x0. Writes MIZAR_LSS_SYSREG_RAW_STCR1 with LSS_SYSREG_RAW_STCR1_GPIO0_INTR (or GPIO1_INTR) to clear sysreg status. Reads back MIZAR_LSS_SYSREG_RAW_STCR1 and checks the GPIO interrupt bit is cleared. Clears GIC IRQ. If any check fails, test_err is incremented. Finally, finish(test_err) is called - 0 means pass, non-zero means fail.",
        "Test Description": "This test validates the GPIO level-sensitive interrupt enable functionality for 32 GPIO pads. It runs in two phases: Phase 1 configures each GPIO in input mode with active-high level interrupt selection, enables the group interrupt for that GPIO, triggers the interrupt via an SRAM write, and verifies in the interrupt handler that the raw interrupt status, group interrupt status, interrupt clearing, group interrupt clearing, and system register status clearing all behave correctly. Phase 2 repeats the same sequence but with active-low level interrupt selection. The test uses gp0_gpio_8 (with offset arithmetic for pads 8-39), gp0_intr1_intr_en1 for group interrupt enable, gp0_intr1_intr_sts1 for group interrupt status, and external system registers for top-level interrupt enable and status clearing. The test passes only if all interrupt assertions succeed across all 32 GPIOs in both active-high and active-low phases.",
        "Meta Test Steps / Procedure": "1. Enter test_case(). 2. Enable GIC IRQ: GIC_EnableIRQ(87) under GPIO0 or GIC_EnableIRQ(88) under GPIO1. 3. Enable sysreg interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) under GPIO0 or LSS_SYSREG_INTR_EN1_GPIO1_INTR under GPIO1. 4. Phase 1 - Active High Level Interrupt: For i = 0 to 31: 4a. Compute wr_val = 1 << i. 4b. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00180000). 4c. wait_on(50). 4d. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). 4e. wait_on(10). 4f. write_reg(0xA0243ffc, 0xffffffff). 4g. Set int_pend = 1. Poll while(int_pend == 1) with wait_on(10). 5. wait_on(100). 6. Phase 2 - Active Low Level Interrupt: For i = 0 to 31: 6a. Compute wr_val = 1 << i. 6b. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00100000). 6c. wait_on(50). 6d. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). 6e. wait_on(10). 6f. write_reg(0xA0243ffc, ~(wr_val)). 6g. Set int_pend = 1. Poll while(int_pend == 1) with wait_on(10). 7. Call finish(test_err). 8. Default_IRQHandler(): 8a-8n as described.",
        "Test Steps / Procedure": "1. Enable the GIC interrupt line for the GPIO controller.\n2. Enable the system-level interrupt for the GPIO block by writing to the system register interrupt enable register.\n3. For each of the 32 GPIO pads (active-high level interrupt phase): configure the GPIO in input mode with active-high level interrupt selection by writing to the corresponding gp0_gpio_8 register (with pad offset), wait for stabilization, enable the group interrupt for that pad in gp0_intr1_intr_en1, write a trigger pattern to the SRAM location, and wait for the interrupt to fire.\n4. In the interrupt handler: verify the raw interrupt status bit is set in the GPIO pad register, verify the group interrupt status bit is set in gp0_intr1_intr_sts1, clear the interrupt by writing the clear bit to the GPIO pad register, verify the interrupt is cleared by reading back the GPIO pad register, disable the group interrupt in gp0_intr1_intr_en1, verify the group interrupt status is cleared in gp0_intr1_intr_sts1, clear the system register interrupt status and verify it is cleared, and clear the GIC interrupt.\n5. Repeat step 3 for all 32 GPIO pads with active-low level interrupt selection (different configuration value written to the GPIO pad register, inverted trigger pattern written to SRAM).\n6. In the interrupt handler for active-low phase: perform the same verification and clearing sequence as step 4.\n7. After both phases complete for all 32 pads, determine the test result: pass if no errors were recorded, fail otherwise.",
        "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_INTR1_INTR_EN1; 0xA0243ffc; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Impacted Registers": "gp0_gpio_8; gp0_intr1_intr_en1; gp0_intr1_intr_sts1",
        "Meta Validation / Acceptance Criteria": "In Default_IRQHandler(): 1. (rdata & 0x2) != 0x0. 2. (rdata_grp & (1 << i)) != 0. 3. rdata == 0x100001 after clear. 4. rdata_grp == 0x0 after disable. 5. (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0. 6. finish(test_err) - test_err == 0 means pass.",
        "Validation / Acceptance Criteria": "1. For each GPIO pad, the raw interrupt status bit (bit 1) in the GPIO pad register must be set when the interrupt fires, for both active-high and active-low level configurations.\n2. The corresponding group interrupt bit in gp0_intr1_intr_sts1 must be set when the interrupt fires.\n3. After clearing the interrupt via the GPIO pad register, the read-back value must confirm the interrupt is cleared successfully.\n4. After disabling the group interrupt in gp0_intr1_intr_en1, the group interrupt status in gp0_intr1_intr_sts1 must read as zero.\n5. The system register interrupt status must be cleared successfully after writing the clear value.\n6. All validations must pass for all 32 GPIO pads in both active-high and active-low phases. The test passes only if zero errors are recorded (test_err == 0).",
        "Remarks": "The test exercises both active-high and active-low level-sensitive interrupt modes across 32 GPIO pads. An SRAM location is used to trigger or simulate the interrupt condition. The test relies on an interrupt-driven flow where the main loop polls int_pend and the IRQ handler performs all validation and clearing. Three Agent 4 register mappings are unresolved: two external system registers (used for top-level interrupt enable and status clearing) and one direct hex SRAM address (used for interrupt triggering), as these are outside the GPIO register specification. Conditional compilation via GPIO0/GPIO1 selects the appropriate GIC IRQ number and system register interrupt bit values."
    }
]

# Create workbook
wb = openpyxl.Workbook()

# ===== TestPlan Sheet =====
ws_tp = wb.active
ws_tp.title = "TestPlan"

tp_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

# Header formatting
header_font = Font(bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

# Write TestPlan headers
for col_idx, col_name in enumerate(tp_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    ws_tp.cell(row=row_idx, column=1, value=row_data.get("Index", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=2, value=row_data.get("SS / Module", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=3, value=row_data.get("Feature", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=4, value=row_data.get("Test Case Name", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=5, value=row_data.get("Test Description", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=6, value=row_data.get("Speed", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=7, value=row_data.get("Mode", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=8, value=row_data.get("Memory Start Offset", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=9, value=row_data.get("Memory End Offset", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=10, value=row_data.get("Remarks", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=11, value=row_data.get("Test Steps / Procedure", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=12, value=row_data.get("Impacted Registers", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=13, value=row_data.get("Validation / Acceptance Criteria", "")).alignment = wrap_alignment
    ws_tp.cell(row=row_idx, column=14, value="").alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = "A2"

# ===== MetaData Sheet =====
ws_md = wb.create_sheet("MetaData")

md_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Write MetaData headers
for col_idx, col_name in enumerate(md_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    ws_md.cell(row=row_idx, column=1, value=row_data.get("Index", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=2, value=row_data.get("Test Case Name", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=3, value=row_data.get("Meta Test Description", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=4, value=row_data.get("Meta Test Steps / Procedure", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=5, value=row_data.get("Meta Impacted Registers", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=6, value=row_data.get("Meta Validation / Acceptance Criteria", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=7, value=row_data.get("Meta Headers", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=8, value=row_data.get("Meta Macros", "")).alignment = wrap_alignment
    ws_md.cell(row=row_idx, column=9, value=row_data.get("Meta Arrays", "")).alignment = wrap_alignment

# Freeze first row
ws_md.freeze_panes = "A2"

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# Auto-size columns for TestPlan
for col_idx, col_name in enumerate(tp_columns, 1):
    max_len = len(col_name)
    for row in range(2, len(json_data) + 2):
        val = ws_tp.cell(row=row, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_tp.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = min(max_len + 4, 60)

# Auto-size columns for MetaData
for col_idx, col_name in enumerate(md_columns, 1):
    max_len = len(col_name)
    for row in range(2, len(json_data) + 2):
        val = ws_md.cell(row=row, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_md.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = min(max_len + 4, 60)

# Save
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
wb.save(output_path)

# Validation
wb2 = openpyxl.load_workbook(output_path)
assert "TestPlan" in wb2.sheetnames
assert "MetaData" in wb2.sheetnames
assert wb2["TestPlan"].max_row == 3  # header + 2 data rows
assert wb2["MetaData"].max_row == 3
file_size = os.path.getsize(output_path)
assert file_size > 0

print(f"SUCCESS: {filename}")
print(f"SIZE: {file_size}")
print(f"PATH: {output_path}")
print(f"SHEETS: {wb2.sheetnames}")
print(f"TESTPLAN_ROWS: {wb2['TestPlan'].max_row - 1}")
print(f"METADATA_ROWS: {wb2['MetaData'].max_row - 1}")
