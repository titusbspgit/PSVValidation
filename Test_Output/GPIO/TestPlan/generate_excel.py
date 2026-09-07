#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
import json
import os
from datetime import datetime, timezone, timedelta

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'GPIO_TestPlan_{timestamp}.xlsx'

json_data = [
    {
        "Index": "1",
        "SS / Module": "GPIO",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Feature": "Register Write and Read Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"test_define.c\"; <gpio/gpio_def.h>; <gpio/gpio_offset.h>',
        "Meta Macros": "CNT",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x8",
        "Meta Test Description": "This testcase performs two main verification phases on GPIO GP0 registers (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10 and up to 49 registers defined in addr_array). Phase 1 (chk_rst_val): Iterates over all registers in addr_array, skips entries flagged in skip_rst_array or with zero read_mask_array, reads each register via read_reg(addr), masks the read value with 0xfffffffe, and compares against default_value_array[i]. Mismatches increment def_fail_cnt. Phase 2 (chk_rd_wr): Iterates over 6 test patterns in chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern, writes (data_wr & write_mask_array[i]) to each register via write_reg(addr, ...), skipping entries flagged in skip_array or with zero write_mask_array. Then reads back each register via read_reg(addr), masks with read_mask_array[i], and computes expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff). Mismatches increment wr_fail_cnt. Finally, test_case() calls finish(1) if def_fail_cnt > 0 or wr_fail_cnt > 0, otherwise finish(0). soft_reset_chk() is disabled via #ifdef 0.",
        "Test Description": "This testcase verifies GPIO GP0 registers by performing two phases: (1) Default Value Check - reads each register and validates the read-back value against its expected default value, skipping registers flagged as non-readable or excluded. (2) Write and Read-Back Check - writes six distinct test patterns to each writable register, then reads back and validates the result against the expected value computed using the write mask, read mask, and default value. Registers verified include gp0_gpio_8, gp0_gpio_9, and gp0_gpio_10. The test passes if all default value checks and all write/read-back checks succeed with zero failures.",
        "Meta Test Steps / Procedure": "1. test_case() is the entry point. 2. chk_rst_val() is called: iterates i from 0 to CNT-1 (49 registers). For each i: addr = addr_array[i]. If skip_rst_array[i] == 1, skip. If read_mask_array[i] == 0x00000000, skip. Otherwise, data_rd = read_reg(addr); data = (data_rd & 0xfffffffe). Compare data against default_value_array[i]. If mismatch, increment def_fail_cnt and print failure. 3. chk_rd_wr() is called: defines chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. Outer loop j from 0 to 5: data_wr = chk_val[j]. Inner write loop i from 0 to CNT-1: addr = addr_array[i]. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip. Otherwise write_reg(addr, (data_wr & write_mask_array[i])). Inner read loop i from 0 to CNT-1: addr = addr_array[i]. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip. If read_mask_array[i] == 0x00000000, skip. Otherwise data_rd = (read_reg(addr) & read_mask_array[i]). Compute wr_n = (write_mask_array[i] ^ 0xffffffff). Compute exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Compare data_rd against exp_val. If mismatch, increment wr_fail_cnt. 4. Back in test_case(): if def_fail_cnt > 0 or wr_fail_cnt > 0, call finish(1) (fail). Else call finish(0) (pass).",
        "Test Steps / Procedure": "1. Begin the test by invoking the default value check phase.\n2. For each GPIO GP0 register (including gp0_gpio_8, gp0_gpio_9, gp0_gpio_10), skip registers that are flagged for exclusion or are non-readable.\n3. Read each eligible register and compare the read-back value (with bit-0 masked off) against the expected default value. Record any mismatches.\n4. Begin the write and read-back verification phase.\n5. For each of six predefined test patterns, write the pattern (masked by the register's write mask) to each writable and non-skipped register.\n6. Read back each written register, apply the read mask, and compute the expected value using the write mask, read mask, and default value.\n7. Compare the read-back value against the computed expected value. Record any mismatches.\n8. Repeat steps 5 through 7 for all six test patterns.\n9. Evaluate overall results: if any default value check or write/read-back check failed, report test failure; otherwise report test pass.",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Meta Validation / Acceptance Criteria": "Phase 1 (chk_rst_val): For each register, data = (read_reg(addr) & 0xfffffffe) must equal default_value_array[i]. Any mismatch increments def_fail_cnt. Phase 2 (chk_rd_wr): For each test pattern and each register, data_rd = (read_reg(addr) & read_mask_array[i]) must equal exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). Any mismatch increments wr_fail_cnt. Final: if (def_fail_cnt > 0 || wr_fail_cnt > 0) then finish(1) indicating FAIL, else finish(0) indicating PASS.",
        "Validation / Acceptance Criteria": "1. Default value check: Each readable GPIO GP0 register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) must return its expected default value (with bit-0 masked off) upon read. Any mismatch is a failure.\n2. Write/read-back check: For each of six test patterns written to each writable register, the read-back value (masked by the read mask) must match the expected value computed from the write mask, read mask, and default value. Any mismatch is a failure.\n3. Overall pass: The test passes only if both the default value failure count and the write/read-back failure count are zero.",
        "Remarks": "The addr_array is declared with 49 entries but only 3 register address macros are visible in the source; the remaining entries may be populated in the full source. Registers flagged in skip_array or skip_rst_array are excluded from write/read or default-value checks respectively. The soft_reset_chk function is disabled via #ifdef 0 and is not executed. Bit-0 is masked off during default value comparison using 0xfffffffe. A comment in test_define.c notes that the din value may become 1 automatically if not forced, affecting level select behavior during default value reads."
    },
    {
        "Index": "2",
        "SS / Module": "GPIO",
        "Test Case Name": "test_gpio_level_sel_intr_en",
        "Feature": "GPIO Level Select Interrupt Enable",
        "Meta Headers": "<lss_sysreg.h>; <stdio.h>; <test_define.c>; <test_common.h>; <gpio/gpio_def.h>; <gpio/gpio_offset.h>",
        "Meta Macros": "CNT",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]",
        "Speed": "NA",
        "Mode": "Input Mode",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x88",
        "Meta Test Description": "This testcase verifies GPIO level-triggered interrupt enable functionality for GPIO pads 8 through 39 (32 GPIOs). It operates in two phases. Phase 1 (Active High Level Interrupt): For each GPIO i (0 to 31), configures the GPIO register MIZAR_GPIO_GP0_GPIO_8 + (i * 4) in input mode with level interrupt enabled by writing 0x00180000 (bits 20 and 19 set to 1). Then enables the corresponding group interrupt bit in MIZAR_GPIO_GP0_INTR1_INTR_EN1 by writing wr_val = (1 << i). Writes 0xffffffff to 0xA0243ffc (SRAM location) to trigger the interrupt stimulus. Sets int_pend = 1 and waits in a while loop polling int_pend until the ISR clears it. Phase 2 (Active Low Level Interrupt): For each GPIO i (0 to 31), configures the GPIO register MIZAR_GPIO_GP0_GPIO_8 + (i * 4) in input mode with level interrupt enabled but active-low by writing 0x00100000 (bit 20 set to 1, bit 19 set to 0). Enables the group interrupt bit in MIZAR_GPIO_GP0_INTR1_INTR_EN1 by writing wr_val = (1 << i). Writes ~(wr_val) to 0xA0243ffc. Sets int_pend = 1 and waits for ISR. In the ISR (Default_IRQHandler): Writes 0xffffffff to 0xA0243ffc. Reads MIZAR_GPIO_GP0_GPIO_8 + (i * 4) and checks bit 1 (raw interrupt status). If set, reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and checks group interrupt bit (1 << i). Clears the interrupt by writing 0x00110000 to MIZAR_GPIO_GP0_GPIO_8 + (i * 4) (bit 16 set). Reads back and verifies rdata == 0x100001. Disables group interrupt by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1. Reads MIZAR_GPIO_GP0_INTR1_INTR_STS1 and verifies it is 0x0. Clears sysreg status by writing to MIZAR_LSS_SYSREG_RAW_STCR1 and reading back to verify the GPIO interrupt bit is cleared. Clears GIC IRQ. test_err is incremented on each failure. test_case() calls finish(test_err) at the end.",
        "Test Description": "This testcase verifies GPIO level-triggered interrupt enable functionality for 32 GPIO pads. It runs in two phases: (1) Active High Level Interrupt - each GPIO is configured in input mode with level interrupt enabled (active high), the corresponding group interrupt is enabled in gp0_intr1_intr_en1, and an interrupt stimulus is applied. The interrupt service routine validates that the raw interrupt status bit is set in the individual GPIO register (gp0_gpio_8 through gp0_gpio_15 range), verifies the group interrupt status in gp0_intr1_intr_sts1, clears the interrupt, and confirms the clear was successful. (2) Active Low Level Interrupt - the same sequence is repeated with active-low level selection. The ISR also clears the system-level interrupt status register and verifies the clear. The test passes only if all 64 interrupt cycles (32 active-high + 32 active-low) complete without errors.",
        "Meta Test Steps / Procedure": "1. test_case() entry: test_err = 0. 2. GIC_EnableIRQ(87) under #ifdef GPIO0 or GIC_EnableIRQ(88) under #ifdef GPIO1. 3. Enable sysreg interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) under #ifdef GPIO0 or write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR) under #ifdef GPIO1. 4. Active High loop: for i = 0 to 31: wr_val = 1 << i. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00180000). wait_on(50). write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). wait_on(10). write_reg(0xA0243ffc, 0xffffffff). int_pend = 1. while(int_pend == 1) { printf; wait_on(10); }. 5. wait_on(100). 6. Active Low loop: for i = 0 to 31: wr_val = 1 << i. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00100000). wait_on(50). write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val). wait_on(10). write_reg(0xA0243ffc, ~(wr_val)). int_pend = 1. while(int_pend == 1) { printf; wait_on(10); }. 7. finish(test_err). 8. Default_IRQHandler(): wr_val = 1 << i. int_pend = 0. write_reg(0xA0243ffc, 0xffffffff). rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)). if ((rdata & 0x2) != 0x0): rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). if ((rdata_grp & (1 << i)) != 0) success else test_err++. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00110000). wait_on(20). rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)). if (rdata == 0x100001) success else test_err++. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000). rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). if (rdata_grp == 0x0) success else test_err++. write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIOx_INTR). rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1). if ((rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0) success else test_err++. else test_err++ (interrupt not occurred). GIC_ClearIRQ(87 or 88).",
        "Test Steps / Procedure": "1. Enable the GIC interrupt line for the GPIO instance.\n2. Enable the system-level interrupt for the GPIO block by writing to the system register interrupt enable register.\n3. For each of the 32 GPIO pads (active-high level interrupt phase): configure the GPIO in input mode with level interrupt enabled (active high), enable the corresponding bit in the group interrupt enable register (gp0_intr1_intr_en1), apply the interrupt stimulus, and wait for the interrupt to fire.\n4. In the interrupt service routine: verify the raw interrupt status bit is set in the individual GPIO register, verify the group interrupt status bit is set in gp0_intr1_intr_sts1, clear the interrupt by writing the clear bit to the GPIO register, read back and verify the interrupt is cleared, disable the group interrupt in gp0_intr1_intr_en1, verify the group interrupt status register reads zero, clear the system-level interrupt status and verify it is cleared, then clear the GIC interrupt.\n5. Wait for a settling period after completing all active-high iterations.\n6. For each of the 32 GPIO pads (active-low level interrupt phase): configure the GPIO in input mode with level interrupt enabled (active low), enable the corresponding bit in the group interrupt enable register, apply the inverted interrupt stimulus, and wait for the interrupt to fire.\n7. In the interrupt service routine: repeat the same validation and clearing sequence as in step 4.\n8. Evaluate overall results: the test passes if no errors were recorded across all 64 interrupt cycles.",
        "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_INTR1_INTR_EN1; 0xA0243ffc; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Impacted Registers": "gp0_gpio_8; gp0_intr1_intr_en1; gp0_intr1_intr_sts1",
        "Meta Validation / Acceptance Criteria": "In Default_IRQHandler: 1. rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); check (rdata & 0x2) != 0x0 - raw interrupt status bit must be set. If not set, test_err++ ('Interrupt Not occured'). 2. rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); check (rdata_grp & (1 << i)) != 0 - group interrupt bit for current GPIO must be set. If not, test_err++ ('Group Interrupt not occured'). 3. After writing 0x00110000 to clear interrupt: rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); check rdata == 0x100001. If not, test_err++ ('Interrupt clear failed'). 4. After writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); check rdata_grp == 0x0. If not, test_err++ ('Group Interrupt clear failed'). 5. After writing to MIZAR_LSS_SYSREG_RAW_STCR1: rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); check (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0. If not, test_err++ ('sysreg status not cleared'). Final: finish(test_err) - pass if test_err == 0, fail otherwise.",
        "Validation / Acceptance Criteria": "1. For each GPIO pad, the raw interrupt status bit (bit 1) in the individual GPIO register must be set when the level interrupt is triggered.\n2. The corresponding group interrupt status bit in gp0_intr1_intr_sts1 must be set for the active GPIO pad.\n3. After writing the interrupt clear bit to the GPIO register, the read-back value must confirm the interrupt is cleared (expected value confirms input mode with din bit set).\n4. After disabling the group interrupt in gp0_intr1_intr_en1, the group interrupt status register gp0_intr1_intr_sts1 must read zero.\n5. After clearing the system-level interrupt status register, the read-back must confirm the GPIO interrupt bit is cleared.\n6. All validations must pass for both active-high and active-low level interrupt phases across all 32 GPIO pads (64 total interrupt cycles).\n7. The test passes only if the total error count is zero.",
        "Remarks": "The testcase uses interrupt-driven validation via Default_IRQHandler with a polling loop (int_pend) in the main test flow to synchronize with the ISR. GIC interrupt line 87 is used for GPIO0 and 88 for GPIO1, selected via conditional compilation. The SRAM location at a hardcoded address is used as an external stimulus to trigger GPIO level interrupts. Three registers (system-level interrupt enable, system-level raw status clear, and the SRAM stimulus address) could not be mapped to the GPIO register specification as they belong to a different IP block or are external memory. The addr_array in test_define.c contains 8 visible register macros but is declared with size 49; the test_case function iterates over 32 GPIOs using address arithmetic from the base GPIO register. wait_on() calls are used for timing delays between configuration and interrupt assertion."
    }
]

# TestPlan columns
tp_cols = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description', 'Speed', 'Mode',
           'Memory Start Offset', 'Memory End Offset', 'Remarks', 'Test Steps / Procedure',
           'Impacted Registers', 'Validation / Acceptance Criteria', 'Code Generation']

# MetaData columns
md_cols = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
           'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
           'Meta Headers', 'Meta Macros', 'Meta Arrays']

wb = openpyxl.Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

header_font = Font(bold=True, color='FFFFFF')
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

ws_tp.freeze_panes = 'A2'

# Auto-size columns
for col_idx, col_name in enumerate(tp_cols, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = str(ws_tp.cell(row=row_idx, column=col_idx).value or '')
        lines = val.split('\n')
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 2, 60)
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = width

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

for col_idx, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(md_cols, 1):
        val = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align

ws_md.freeze_panes = 'A2'

for col_idx, col_name in enumerate(md_cols, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = str(ws_md.cell(row=row_idx, column=col_idx).value or '')
        lines = val.split('\n')
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    width = min(max_len + 2, 60)
    ws_md.column_dimensions[get_column_letter(col_idx)].width = width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)
wb.save(output_path)
print(f'SUCCESS: {filename}')
print(f'PATH: {output_path}')
