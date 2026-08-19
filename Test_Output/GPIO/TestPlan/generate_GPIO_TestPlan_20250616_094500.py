#!/usr/bin/env python3
"""GPIO TestPlan XLSX Generator - Run this script ONCE to produce the final .xlsx file.
Requirements: pip install openpyxl
Usage: python generate_GPIO_TestPlan_20250616_094500.py
Output: GPIO_TestPlan_20250616_094500.xlsx (in same directory)
"""
import os
from openpyxl import Workbook
from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
from openpyxl.utils import get_column_letter

def create_workbook():
    wb = Workbook()
    
    # ========== SHEET 1: TestPlan (visible) ==========
    ws = wb.active
    ws.title = "TestPlan"
    
    # 21 Headers
    headers = [
        "Index",
        "SS / Module",
        "Feature",
        "Test Case Name",
        "Test Description",
        "Meta Test Description",
        "Speed",
        "Mode",
        "Memory Start Offset",
        "Memory End Offset",
        "Remarks",
        "Test Steps / Procedure",
        "Meta Test Steps / Procedure",
        "Impacted Registers",
        "Meta Impacted Registers",
        "Validation / Acceptance Criteria",
        "Meta Validation / Acceptance Criteria",
        "Code Generation",
        "Meta Headers",
        "Meta Macros",
        "Meta Arrays"
    ]
    
    # Data row
    data = [
        1,
        "GPIO",
        "Register Read Write Verification",
        "gpio_reg_wr_rd_test",
        "This test verifies the register read and write functionality for GPIO GP0 per-pin configuration registers. The test reads default reset values from registers gp0_gpio_8 (offset 0x0), gp0_gpio_9 (offset 0x4), and gp0_gpio_10 (offset 0x8), compares them against expected default values using read masks, then writes multiple data patterns (0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, 0xA5A5A5A5, 0xF5F5F5F5, 0xFFFF0000) to each register using write masks, reads back, and verifies correctness. A soft reset verification flow is present but conditionally disabled.",
        "Test reads registers MIZAR_GPIO_GP0_GPIO_8 (base 0xA001A000 + offset 0x0), MIZAR_GPIO_GP0_GPIO_9 (base 0xA001A000 + offset 0x4), MIZAR_GPIO_GP0_GPIO_10 (base 0xA001A000 + offset 0x8) via read_reg(addr_array[i]). Default values compared using GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL with corresponding READ_MASK macros. Write patterns applied via write_reg(addr_array[i], (wr_data[j] & WRITE_MASK[i]) | (DEFAULT_VAL[i] & ~WRITE_MASK[i])). Readback verified as (read_val & READ_MASK[i]) == ((wr_data[j] & WRITE_MASK[i]) | (DEFAULT_VAL[i] & ~WRITE_MASK[i])) & READ_MASK[i]. SOFT_RST_REG_ADDRESS (0x00000000) used in disabled #ifdef 0 block for soft_reset_chk().",
        "NA",
        "Polling",
        "0x0",
        "0x8",
        "SOFT_RST_REG_ADDRESS is inside a disabled #ifdef 0 block and will not execute. RO fields (data_in bit[0], intr_raw_sts bit[1]) cannot be written and retain reset values on write attempts. WO field (intr_clr bit[16]) reads back as 0. RW2 fields (io_ctrl bit[20], dout bit[21]) behave as standard read-write for this test. Register width is 32 bits, AHB bus, ahb_clk domain, hreset_n active-low async reset.",
        "1. Initialize the GPIO subsystem and configure AHB bus access.\n2. Read the default value of register gp0_gpio_8 at offset 0x0.\n3. Read the default value of register gp0_gpio_9 at offset 0x4.\n4. Read the default value of register gp0_gpio_10 at offset 0x8.\n5. Compare each read value (masked with read mask 0x003F0003) against the expected default value 0x00100000 (io_ctrl=1 at bit 20).\n6. For each register, write pattern 0xFFFFFFFF applying write mask 0x003F0000, read back, and verify.\n7. Repeat step 6 with pattern 0x55555555.\n8. Repeat step 6 with pattern 0xAAAAAAAA.\n9. Repeat step 6 with patterns 0xA5A5A5A5, 0xF5F5F5F5, and 0xFFFF0000.\n10. Report PASS if all read-back values match expected values after masking; report FAIL otherwise.",
        "1. addr_array[49] initialized with {MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10}.\n2. chk_rst_val(): for i=0 to CNT-1: rd_data = read_reg(addr_array[i]); compare (rd_data & READ_MASK[i]) vs (DEFAULT_VAL[i] & READ_MASK[i]).\n3. chk_rd_wr(): for each wr_data in {0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, 0xA5A5A5A5, 0xF5F5F5F5, 0xFFFF0000}: for i=0 to CNT-1: write_reg(addr_array[i], (wr_data & WRITE_MASK[i]) | (DEFAULT_VAL[i] & ~WRITE_MASK[i])); rd_data = read_reg(addr_array[i]); verify (rd_data & READ_MASK[i]) == ((wr_data & WRITE_MASK[i]) | (DEFAULT_VAL[i] & ~WRITE_MASK[i])) & READ_MASK[i].\n4. soft_reset_chk() inside #ifdef 0: write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA); read_reg(SOFT_RST_REG_ADDRESS) \u2014 DISABLED.\n5. main() calls chk_rst_val() then chk_rd_wr(); prints PASS/FAIL.",
        "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10",
        "MIZAR_GPIO_GP0_GPIO_8; MIZAR_GPIO_GP0_GPIO_9; MIZAR_GPIO_GP0_GPIO_10; SOFT_RST_REG_ADDRESS",
        "PASS: All register default value reads match expected value 0x00100000 after applying read mask 0x003F0003. All write-read cycles for 6 data patterns return expected masked values for all 3 registers. FAIL: Any single read-back mismatch after masking causes test failure.",
        "PASS: For all i in [0, CNT): (read_reg(addr_array[i]) & READ_MASK[i]) == (DEFAULT_VAL[i] & READ_MASK[i]) AND for all wr_data patterns: (read_reg(addr_array[i]) & READ_MASK[i]) == ((wr_data & WRITE_MASK[i]) | (DEFAULT_VAL[i] & ~WRITE_MASK[i])) & READ_MASK[i]. FAIL: Any mismatch in above comparisons. Write Mask: 0x003F0000. Read Mask: 0x003F0003. Default: 0x00100000.",
        "Not",
        '#include <stdio.h>; #include <stdlib.h>; #include <string.h>; #include "common.h"; #include "test_define.c"; #include "gpio_headers.h"',
        "#define GPIO0 1; #define SOFT_RST_REG_ADDRESS 0x00000000; #define SOFT_RST_REG_DATA 0x00000400; #define CNT 3",
        "addr_array[49]={MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10}; default_val_array[49]={GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL}; rd_mask_array[49]={GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK}; wr_mask_array[49]={GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK}; wr_data[6]={0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, 0xA5A5A5A5, 0xF5F5F5F5, 0xFFFF0000}; rd_data_array[49]; exp_data_array[49]"
    ]
    
    # Styles
    header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
    data_alignment = Alignment(vertical='top', wrap_text=True)
    thin_border = Border(
        left=Side(style='thin'),
        right=Side(style='thin'),
        top=Side(style='thin'),
        bottom=Side(style='thin')
    )
    data_font = Font(name='Calibri', size=10)
    
    # Write headers (Row 1)
    for col_idx, header in enumerate(headers, 1):
        cell = ws.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border
    
    # Write data (Row 2)
    for col_idx, value in enumerate(data, 1):
        cell = ws.cell(row=2, column=col_idx, value=value)
        cell.font = data_font
        cell.alignment = data_alignment
        cell.border = thin_border
    
    # Freeze first row
    ws.freeze_panes = 'A2'
    
    # Column widths
    col_widths = {
        1: 8,    # Index
        2: 15,   # SS / Module
        3: 30,   # Feature
        4: 28,   # Test Case Name
        5: 60,   # Test Description
        6: 60,   # Meta Test Description
        7: 10,   # Speed
        8: 12,   # Mode
        9: 20,   # Memory Start Offset
        10: 20,  # Memory End Offset
        11: 50,  # Remarks
        12: 60,  # Test Steps / Procedure
        13: 60,  # Meta Test Steps / Procedure
        14: 40,  # Impacted Registers
        15: 50,  # Meta Impacted Registers
        16: 55,  # Validation / Acceptance Criteria
        17: 60,  # Meta Validation / Acceptance Criteria
        18: 16,  # Code Generation
        19: 50,  # Meta Headers
        20: 50,  # Meta Macros
        21: 70,  # Meta Arrays
    }
    for col_num, width in col_widths.items():
        ws.column_dimensions[get_column_letter(col_num)].width = width
    
    # Row height for data row
    ws.row_dimensions[2].height = 200
    
    # ========== SHEET 2: MetaData (very hidden) ==========
    ws_meta = wb.create_sheet(title="MetaData")
    ws_meta.sheet_state = 'veryHidden'
    
    meta_headers = ["Property", "Value"]
    meta_data = [
        ["IP_Name", "GPIO"],
        ["Token", "#define GPIO0 1"],
        ["Base_Address", "0xA001A000"],
        ["Register_Block", "gp0"],
        ["Bus_Type", "AHB"],
        ["Bus_Width", "32"],
        ["Clock", "ahb_clk"],
        ["Reset_Signal", "hreset_n"],
        ["Reset_Type", "Active-low asynchronous"],
        ["Source_Module", "gpio"],
        ["Generation_Timestamp_IST", "2025-06-16T09:45:00+05:30"],
        ["Generator", "Ag_Excel_Generator Agent"],
        ["Workflow", "PSV TestPlan Generation Pipeline"],
        ["Total_Test_Cases", "1"],
        ["Resolved_Registers", "gp0_gpio_8; gp0_gpio_9; gp0_gpio_10"],
        ["Unresolved_Macros", "SOFT_RST_REG_ADDRESS"],
        ["Write_Patterns", "0xFFFFFFFF; 0x55555555; 0xAAAAAAAA; 0xA5A5A5A5; 0xF5F5F5F5; 0xFFFF0000"],
        ["Read_Mask", "0x003F0003"],
        ["Write_Mask", "0x003F0000"],
        ["Default_Value", "0x00100000"],
        ["Spec_Source_Register", "gp0_autoreg.xlsx"],
        ["Spec_Source_System", "LSS_REGS (1).pdf"],
        ["GitHub_Source_Path", "TestRepo/gpio/gpio_reg_wr_rd_test/"],
    ]
    
    meta_header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    meta_header_fill = PatternFill(start_color='2F5496', end_color='2F5496', fill_type='solid')
    
    for col_idx, header in enumerate(meta_headers, 1):
        cell = ws_meta.cell(row=1, column=col_idx, value=header)
        cell.font = meta_header_font
        cell.fill = meta_header_fill
        cell.alignment = Alignment(horizontal='center', vertical='center')
        cell.border = thin_border
    
    for row_idx, row_data in enumerate(meta_data, 2):
        for col_idx, value in enumerate(row_data, 1):
            cell = ws_meta.cell(row=row_idx, column=col_idx, value=value)
            cell.font = data_font
            cell.alignment = Alignment(vertical='top', wrap_text=True)
            cell.border = thin_border
    
    ws_meta.column_dimensions['A'].width = 30
    ws_meta.column_dimensions['B'].width = 80
    ws_meta.freeze_panes = 'A2'
    
    # Save
    output_filename = "GPIO_TestPlan_20250616_094500.xlsx"
    output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), output_filename)
    wb.save(output_path)
    print(f"SUCCESS: Generated {output_path}")
    print(f"File: {output_filename}")
    print(f"Sheets: TestPlan (visible), MetaData (veryHidden)")
    print(f"Headers: 21 columns")
    print(f"Data rows: 1")
    return output_path

if __name__ == '__main__':
    create_workbook()
