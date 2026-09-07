#!/usr/bin/env python3
"""Auto-generated TestPlan Excel generator for GPIO IP."""
import json
import os
import sys
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))

JSON_DATA = [
    {
        "Index": "1",
        "SS / Module": "GPIO",
        "Test Case Name": "gpio_reg_wr_rd_test",
        "Feature": "Register Read/Write Validation",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"test_define.c\"; <gpio/gpio_def.h>; <gpio/gpio_offset.h>",
        "Meta Macros": "SOFT_RST_REG_DATA; CNT",
        "Meta Arrays": "addr_array[49]; default_value_array[49]; read_mask_array[49]; write_mask_array[49]; skip_array[49]; skip_rst_array[49]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x8",
        "Meta Test Description": "This testcase validates GPIO register default values and write/read-back behavior for registers addressed via addr_array (MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10). The test_case() function first calls chk_rst_val() which iterates over addr_array, skips entries flagged in skip_rst_array, skips entries with read_mask_array == 0x00000000, reads each register via read_reg(addr), masks the read data with 0xfffffffe, and compares against default_value_array[i]. If a mismatch occurs, def_fail_cnt is incremented. Then chk_rd_wr() is called, which iterates over six test patterns in chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each pattern, it writes (data_wr & write_mask_array[i]) to each register via write_reg(addr, ...), skipping entries flagged in skip_array or with write_mask_array == 0x00000000. Then it reads back each register via read_reg(addr), masks with read_mask_array[i], computes the expected value as ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])) where wr_n = (write_mask_array[i] ^ 0xffffffff), and compares. If a mismatch occurs, wr_fail_cnt is incremented. Finally, if def_fail_cnt > 0 or wr_fail_cnt > 0, finish(1) is called (fail); otherwise finish(0) is called (pass). The soft_reset_chk() function is disabled via #ifdef 0.",
        "Test Description": "This testcase validates the GPIO register default reset values and write/read-back integrity for the gp0_gpio_8, gp0_gpio_9, and gp0_gpio_10 registers. First, each register is read and its value is compared against the expected default value. Then, a series of six predefined data patterns are written to each register (respecting write masks), and the values are read back and verified against expected values computed using read masks, write masks, and default values. Registers flagged for skipping are excluded from the respective checks. The test passes only if all default value checks and all write/read-back checks succeed with zero mismatches.",
        "Meta Test Steps / Procedure": "1. Global variables initialized: data_rd, data_wr, data, def_fail_cnt=0, wr_fail_cnt=0. 2. test_case() calls chk_rst_val(). 3. chk_rst_val() iterates i from 0 to CNT-1: addr = addr_array[i]. 4. If skip_rst_array[i] == 1, skip this register. 5. If read_mask_array[i] == 0x00000000, skip this register (not readable). 6. data_rd = read_reg(addr). 7. data = (data_rd & 0xfffffffe). 8. Compare data against default_value_array[i]; if mismatch, increment def_fail_cnt and print failure. 9. test_case() calls chk_rd_wr(). 10. chk_rd_wr() defines chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. 11. Outer loop j from 0 to 5: data_wr = chk_val[j]. 12. Inner write loop i from 0 to CNT-1: addr = addr_array[i]. 13. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip. 14. write_reg(addr, (data_wr & write_mask_array[i])). 15. Inner read loop i from 0 to CNT-1: addr = addr_array[i]. 16. If skip_array[i] == 1, skip. If write_mask_array[i] == 0x00000000, skip. If read_mask_array[i] == 0x00000000, skip. 17. data_rd = (read_reg(addr) & read_mask_array[i]). 18. wr_n = (write_mask_array[i] ^ 0xffffffff). 19. exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])). 20. Compare data_rd against exp_val; if mismatch, increment wr_fail_cnt and print failure. 21. After all iterations, if def_fail_cnt > 0 or wr_fail_cnt > 0, call finish(1) (fail); else call finish(0) (pass).",
        "Test Steps / Procedure": "1. Initialize the test environment and set failure counters to zero. 2. Perform default value check: read each GPIO register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10), skipping registers flagged for reset-skip or that are not readable. 3. Compare each read value (masked) against the expected default reset value for that register. Record any mismatches. 4. Perform write/read-back check: for each of six predefined test data patterns, write the pattern (masked by the write mask) to each writable GPIO register. 5. Read back each register after writing, apply the read mask, and compute the expected value using the write mask, read mask, and default value. 6. Compare the read-back value against the computed expected value. Record any mismatches. 7. Repeat steps 4-6 for all six test patterns. 8. Evaluate overall result: if any default value mismatch or write/read-back mismatch occurred, report test failure; otherwise report test pass.",
        "Meta Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10",
        "Impacted Registers": "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(): data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); pass if data == default_value_array[i], else def_fail_cnt++ and print failure. In chk_rd_wr(): data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); pass if data_rd == exp_val, else wr_fail_cnt++ and print failure. Final: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). Six test patterns used: 0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000.",
        "Validation / Acceptance Criteria": "1. Each GPIO register (gp0_gpio_8, gp0_gpio_9, gp0_gpio_10) must return its expected default reset value when read after reset. 2. For each of six test data patterns written to the registers, the read-back value (after applying read and write masks) must match the computed expected value. 3. The expected read-back value accounts for writable bits taking the written value and non-writable bits retaining the default value. 4. The test passes only if zero default-value mismatches and zero write/read-back mismatches are recorded across all registers and all patterns. 5. Any single mismatch causes overall test failure.",
        "Remarks": "The soft_reset_chk() function is disabled via #ifdef 0 and is not executed. The SOFT_RST_REG_ADDRESS macro is ignored per instructions. The base address MIZAR_GPIO_BASE has an ambiguous conditional definition (GPIO0 vs GPIO1), so the resolved base is not determinable from context. The addr_array is declared with 49 elements but only 3 register address initializers are visible in the source. Skip arrays control which registers are excluded from default-value checks and write/read-back checks respectively. The read data in default value check is masked with 0xfffffffe (bit 0 cleared) before comparison. A source comment notes that the din value may become 1 automatically affecting level select behavior if not forced."
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
        "Mode": "NA",
        "Memory Start Offset": "0x0",
        "Memory End Offset": "0x88",
        "Meta Test Description": "This testcase validates GPIO level-sensitive interrupt generation and clearing for GPIOs 8 through 39 using both active-high and active-low level selection. In test_case(), GIC IRQ is enabled (87 for GPIO0, 88 for GPIO1). The system register interrupt enable is written via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) or the GPIO1 variant. The test then loops 32 times for active-high level interrupts: each GPIO register is configured in input mode with level interrupt enabled by writing 0x00180000 to MIZAR_GPIO_GP0_GPIO_8 + (i * 4) (bits 20 and 19 set), followed by wait_on(50). The group interrupt enable register MIZAR_GPIO_GP0_INTR1_INTR_EN1 is written with wr_val = (1 << i). An SRAM trigger is written via write_reg(0xA0243ffc, 0xffffffff), and the code polls int_pend until the interrupt handler clears it. A second loop of 32 iterations tests active-low level interrupts: each GPIO register is written with 0x00100000 (bit 20 set, bit 19 clear), the group interrupt enable is set, and the SRAM trigger is written with ~(wr_val). The interrupt handler Default_IRQHandler() writes 0xffffffff to 0xA0243ffc, reads the GPIO register via read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)), checks that the raw interrupt status bit (bit 1) is set (rdata & 0x2 != 0x0), reads the group interrupt status via read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) and verifies the corresponding bit is set (rdata_grp & (1 << i) != 0). The interrupt is cleared by writing 0x00110000 to the GPIO register (bit 16 set), followed by a read-back expecting 0x100001. The group interrupt enable is disabled by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1, and the group status is read and verified to be 0x0. The system register raw status is cleared via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, ...) and read back to verify the corresponding bit is cleared. GIC IRQ is cleared. test_err is incremented on each failure, and finish(test_err) is called at the end.",
        "Test Description": "This testcase validates GPIO level-sensitive interrupt generation and clearing for GPIO pads 8 through 39 using both active-high and active-low level selection modes. Each GPIO pad is configured in input mode with level interrupt enabled. For active-high testing, the level select bit is set; for active-low testing, it is cleared. After configuring each pad, the group interrupt enable register (gp0_intr1_intr_en1) is programmed to enable the corresponding GPIO interrupt. An SRAM trigger is written to stimulate the interrupt. The interrupt handler verifies that the raw interrupt status bit is asserted in the individual GPIO register (gp0_gpio_8 through gp0_gpio_15 range), confirms the group interrupt status bit is set in gp0_intr1_intr_sts1, clears the interrupt by writing the clear bit to the GPIO register, verifies the interrupt is cleared on read-back, disables the group interrupt, verifies the group status is cleared, and clears the system-level interrupt status. The test passes only if all 64 interrupt assertions (32 active-high plus 32 active-low) and their corresponding clear operations succeed without error.",
        "Meta Test Steps / Procedure": "1. Global variables initialized: gpio_number, test_err=0, i, extern int_pend. 2. test_case() enables GIC IRQ: GIC_EnableIRQ(87) for GPIO0 or GIC_EnableIRQ(88) for GPIO1. 3. Write system register interrupt enable: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR) or GPIO1 variant. 4. Active-high level interrupt loop: for i = 0 to 31. 5. wr_val = 1 << i. 6. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00180000) -- configure input mode with level interrupt, level select high (bits 20 and 19). 7. wait_on(50). 8. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) -- enable group interrupt for pad i. 9. wait_on(10). 10. write_reg(0xA0243ffc, 0xffffffff) -- SRAM trigger. 11. int_pend = 1; poll while(int_pend == 1) with printf and wait_on(10). 12. wait_on(100) between loops. 13. Active-low level interrupt loop: for i = 0 to 31. 14. wr_val = 1 << i. 15. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00100000) -- configure input mode with level interrupt, level select low (bit 20 set, bit 19 clear). 16. wait_on(50). 17. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val) -- enable group interrupt for pad i. 18. wait_on(10). 19. write_reg(0xA0243ffc, ~(wr_val)) -- SRAM trigger with inverted value. 20. int_pend = 1; poll while(int_pend == 1) with printf and wait_on(10). 21. Default_IRQHandler() entry: wr_val = 1 << i; int_pend = 0; write_reg(0xA0243ffc, 0xffffffff). 22. rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)). 23. Check (rdata & 0x2) != 0x0 -- raw interrupt status bit set. 24. rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). 25. Check (rdata_grp & (1 << i)) != 0 -- group interrupt bit set; else test_err++. 26. write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00110000) -- clear interrupt (bit 16). 27. wait_on(20). 28. rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)). 29. Check rdata == 0x100001 -- interrupt cleared successfully; else test_err++. 30. write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000) -- disable group interrupt. 31. rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1). 32. Check rdata_grp == 0x0 -- group interrupt cleared; else test_err++. 33. write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR) or GPIO1 variant. 34. rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1). 35. Check (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0 -- sysreg status cleared; else test_err++. 36. GIC_ClearIRQ(87) or GIC_ClearIRQ(88). 37. If (rdata & 0x2) == 0x0 at step 23, print error and test_err++. 38. finish(test_err) called at end of test_case().",
        "Test Steps / Procedure": "1. Initialize the test environment and set the error counter to zero. 2. Enable the GIC interrupt for the GPIO instance. 3. Write the system-level interrupt enable register to allow GPIO interrupts to propagate. 4. Begin active-high level interrupt testing: loop through 32 GPIO pads (pads 8 through 39). 5. For each pad, configure the GPIO register in input mode with level interrupt enabled and level select set to active-high. 6. Wait for configuration to settle. 7. Enable the corresponding bit in the group interrupt enable register (gp0_intr1_intr_en1). 8. Write an SRAM trigger value to stimulate the interrupt. 9. Wait for the interrupt handler to execute and clear the pending flag. 10. In the interrupt handler, read the GPIO register and verify the raw interrupt status bit is asserted. 11. Read the group interrupt status register (gp0_intr1_intr_sts1) and verify the corresponding pad bit is set. 12. Clear the interrupt by writing the clear bit to the GPIO register and verify the register reads back the expected cleared value. 13. Disable the group interrupt enable and verify the group interrupt status register reads zero. 14. Clear the system-level interrupt status register and verify it is cleared on read-back. 15. Clear the GIC interrupt. 16. Begin active-low level interrupt testing: repeat steps 5 through 15 for all 32 pads with level select set to active-low and an inverted SRAM trigger value. 17. Evaluate overall result: if any error was recorded during any iteration, report test failure; otherwise report test pass.",
        "Meta Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1; MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_INTR1_INTR_EN1; 0xA0243ffc; MIZAR_GPIO_GP0_INTR1_INTR_STS1; MIZAR_LSS_SYSREG_RAW_STCR1",
        "Impacted Registers": "gp0_gpio_8; gp0_intr1_intr_en1; gp0_intr1_intr_sts1",
        "Meta Validation / Acceptance Criteria": "In Default_IRQHandler(): 1. rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); check (rdata & 0x2) != 0x0 -- raw interrupt status bit must be set; else test_err++. 2. rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); check (rdata_grp & (1 << i)) != 0 -- group interrupt bit for the active pad must be set; else test_err++. 3. After writing 0x00110000 to clear interrupt: rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4)); check rdata == 0x100001 -- interrupt must be cleared; else test_err++. 4. After writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1: rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); check rdata_grp == 0x0 -- group interrupt must be cleared; else test_err++. 5. After writing to MIZAR_LSS_SYSREG_RAW_STCR1: rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); check (rdata & LSS_SYSREG_RAW_STCR1_GPIOx_INTR) == 0 -- sysreg status must be cleared; else test_err++. 6. Final: finish(test_err) -- pass if test_err == 0, fail otherwise.",
        "Validation / Acceptance Criteria": "1. For each of the 32 GPIO pads in active-high level mode, the raw interrupt status bit in the individual GPIO register must be asserted when the interrupt is triggered. 2. The corresponding bit in the group interrupt status register (gp0_intr1_intr_sts1) must be set for the active pad. 3. After writing the interrupt clear bit to the GPIO register, the read-back value must match the expected cleared state. 4. After disabling the group interrupt enable (gp0_intr1_intr_en1), the group interrupt status register must read zero. 5. The system-level interrupt status register must be cleared after writing the clear value and verified on read-back. 6. All five validation checks above must also pass for each of the 32 GPIO pads in active-low level mode. 7. The test passes only if zero errors are accumulated across all 64 iterations (32 active-high plus 32 active-low).",
        "Remarks": "The test uses conditional compilation (#ifdef GPIO0 / GPIO1) to select the appropriate GIC IRQ number and system register interrupt bits, making the active GPIO instance ambiguous without build-time context. Three Agent 4 register mappings are unresolved: MIZAR_LSS_SYSREG_INTR_EN1 and MIZAR_LSS_SYSREG_RAW_STCR1 belong to the LSS system register block (outside the GPIO register specification), and 0xA0243ffc is a hardcoded SRAM address used as an interrupt trigger mechanism. The GPIO register base MIZAR_GPIO_GP0_GPIO_8 is used with an offset of (i * 4) to address pads 8 through 39 across the loop. wait_on() calls are used for timing delays between configuration and interrupt assertion. The int_pend flag is polled in a while loop to synchronize with the interrupt handler."
    }
]

IP_NAME = "GPIO"
OUTPUT_DIR = "Test_Output/GPIO/TestPlan"

TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers",
    "Validation / Acceptance Criteria", "Code Generation"
]

METADATA_COLUMNS = [
    "Index", "Test Case Name", "Meta Test Description",
    "Meta Test Steps / Procedure", "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria", "Meta Headers",
    "Meta Macros", "Meta Arrays"
]


def create_workbook():
    now_ist = datetime.now(IST)
    ts = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"{IP_NAME}_TestPlan_{ts}.xlsx"

    wb = Workbook()

    # --- TestPlan sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_tp.append(TESTPLAN_COLUMNS)

    for row in JSON_DATA:
        data_row = []
        for col in TESTPLAN_COLUMNS:
            data_row.append(row.get(col, ""))
        ws_tp.append(data_row)

    # --- MetaData sheet ---
    ws_md = wb.create_sheet("MetaData")
    ws_md.append(METADATA_COLUMNS)

    for row in JSON_DATA:
        data_row = []
        for col in METADATA_COLUMNS:
            data_row.append(row.get(col, ""))
        ws_md.append(data_row)

    # --- Formatting ---
    header_font = Font(bold=True, color="FFFFFF")
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for ws in [ws_tp, ws_md]:
        # Freeze first row
        ws.freeze_panes = "A2"

        # Header formatting
        for cell in ws[1]:
            cell.font = header_font
            cell.fill = header_fill
            cell.alignment = wrap_align

        # Wrap text on all data cells and auto-size columns
        for col_idx in range(1, ws.max_column + 1):
            max_len = 0
            col_letter = get_column_letter(col_idx)
            for row_idx in range(1, ws.max_row + 1):
                cell = ws.cell(row=row_idx, column=col_idx)
                cell.alignment = wrap_align
                val = str(cell.value) if cell.value else ""
                lines = val.split("\n")
                longest_line = max((len(l) for l in lines), default=0)
                if longest_line > max_len:
                    max_len = longest_line
            width = min(max_len + 4, 60)
            if width < 12:
                width = 12
            ws.column_dimensions[col_letter].width = width

    # MetaData sheet: veryHidden
    ws_md.sheet_state = "veryHidden"

    # Save
    out_path = os.path.join(OUTPUT_DIR, filename)
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    wb.save(out_path)
    print(f"FILE_SAVED:{out_path}")
    print(f"FILENAME:{filename}")

    # Validate
    wb2 = load_workbook(out_path)
    sheets = wb2.sheetnames
    assert "TestPlan" in sheets, "TestPlan sheet missing"
    assert "MetaData" in sheets, "MetaData sheet missing"
    fsize = os.path.getsize(out_path)
    assert fsize > 0, "File size is 0"
    print(f"VALIDATION:PASSED")
    print(f"FILE_SIZE:{fsize}")
    print(f"ROWS_TESTPLAN:{ws_tp.max_row - 1}")
    print(f"ROWS_METADATA:{ws_md.max_row - 1}")


if __name__ == "__main__":
    create_workbook()
