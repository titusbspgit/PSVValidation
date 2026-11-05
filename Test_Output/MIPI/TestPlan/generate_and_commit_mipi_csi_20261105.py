#!/usr/bin/env python3
"""Agent7 MIPI_CSI TestPlan XLSX Generator - Self-executing via GitHub Actions
Generates and commits MIPI_CSI_TestPlan_20261105_181500.xlsx
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
import base64, os, subprocess, sys

def main():
    FILENAME = 'MIPI_CSI_TestPlan_20261105_181500.xlsx'
    wb = openpyxl.Workbook()
    hf = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    hfill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wa = Alignment(wrap_text=True, vertical='top')

    # ===== TestPlan Sheet =====
    ws = wb.active
    ws.title = 'TestPlan'
    hdrs = ['Index','SS / Module','Feature','Test Case Name','Speed','Mode',
            'Memory Start Offset','Memory End Offset','Test Description',
            'Test Steps / Procedure','Impacted Registers',
            'Validation / Acceptance Criteria','Remarks',
            'Code Generation (Required / Not)']
    for i,h in enumerate(hdrs,1):
        c = ws.cell(row=1,column=i,value=h); c.font=hf; c.fill=hfill; c.alignment=wa

    row_data = [
        '1',
        'MIPI_CSI',
        'Register Write/Read Verification',
        'mipi_csi2_rb_reg_wr_rd_test',
        'NA',
        'NA',
        'NA',
        'NA',
        'This testcase verifies the register write/read functionality of the MIPI CSI-2 Receive Block. The test validates that all 60 registers in the register block hold their expected default values after reset, that writable registers can be written and read back correctly respecting their write masks, and that a soft reset restores all registers to their default values. The test covers control registers, interrupt registers, FIFO configuration registers, DPHY configuration registers, DMA address registers, and boot configuration registers.',
        'Step 1: Read all 60 MIPI CSI-2 RB registers and verify each register contains its expected default (reset) value. Registers that are not readable (read mask is zero) are skipped. Any mismatch is recorded as a default value check failure.\nStep 2: Write an all-ones pattern to all writable registers. Registers flagged in the skip array or registers that are not writable (write mask is zero) are skipped during the write phase.\nStep 3: Read back all written registers and verify the read data matches the expected value (all-ones pattern masked with the register\'s write mask). Any mismatch is recorded as a write/read check failure.\nStep 4: Perform a soft reset by writing the soft reset data value to the soft reset register, waiting for the reset to complete, and then restoring the original register value.\nStep 5: After the soft reset, read all 60 registers again and verify each register has returned to its expected default value.\nStep 6: Evaluate the overall test result. The test passes if both the default value check failure count and the write/read check failure count are zero. Otherwise, the test fails.',
        'virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch1_Instruction, dphy_rx',
        '1. Default Value Check: Each readable register must return its expected default value when read after power-on reset. The read data must exactly match the predefined default value for that register.\n2. Write/Read Verification: After writing an all-ones pattern to each writable register, the read-back value must equal the written value masked with the register\'s write mask. This confirms that all writable bit fields accept and retain data correctly, and read-only bits remain unaffected.\n3. Soft Reset Recovery: After performing a soft reset sequence, all registers must return to their default values. The post-reset default value check must pass with zero failures.\n4. Overall Pass Criteria: The test passes (returns 0) if and only if both the default value failure count and the write/read failure count are zero. Any non-zero failure count results in test failure (returns 1).',
        'The test iterates over 60 registers covering control, interrupt, FIFO, DPHY, DMA address, and boot configuration register groups. Some registers are skipped during write/read verification based on a skip array and write mask conditions. Seventeen address macros from the test source could not be resolved to register names in the IP register specification and may require manual verification.',
        ''  # Code Generation - blank default
    ]
    for i,v in enumerate(row_data,1):
        c = ws.cell(row=2,column=i,value=v); c.alignment=wa

    # Data Validation dropdown
    dv = DataValidation(type='list', formula1='"Required,Not Required"', allow_blank=True, showDropDown=False, showErrorMessage=True,
                        errorTitle='Invalid Entry', error='Please select Required or Not Required, or leave blank.',
                        promptTitle='Code Generation', prompt='Select Required or Not Required')
    dv.add('N2:N1048576')
    ws.add_data_validation(dv)

    col_widths = [8,16,35,40,8,8,22,20,60,60,60,60,60,32]
    for i,w in enumerate(col_widths,1):
        ws.column_dimensions[get_column_letter(i)].width = w
    ws.freeze_panes = 'A2'

    # ===== MetaData Sheet (veryHidden) =====
    wm = wb.create_sheet('MetaData')
    mh = ['Test Case Name','Meta Headers','Meta Macros','Meta Arrays','Meta Types',
          'Meta Test Description','Meta Test Steps / Procedure',
          'Meta Impacted Registers','Meta Validation / Acceptance Criteria']
    for i,h in enumerate(mh,1):
        c = wm.cell(row=1,column=i,value=h); c.font=hf; c.fill=hfill; c.alignment=wa

    md = [
        'mipi_csi2_rb_reg_wr_rd_test',
        'SOURCE_FILE: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "test_define.c"\n#include "test_common.h"\nEND_SOURCE_FILE\n\nSOURCE_FILE: test_define.c\n#include<mipi_csi2_rb_reg.h>\nEND_SOURCE_FILE',
        'SOURCE_FILE: test_define.c\n#define CNT 60\nEND_SOURCE_FILE\n\nSOURCE_FILE: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA\n#ifdef DEBUG_DISPLAY ... #endif (conditional compilation directive used throughout chk_rst_val, chk_rd_wr, test_case)\nEND_SOURCE_FILE',
        'SOURCE_FILE: test_define.c\nint data_rd,data_wr,rst_val;\n\naddr_array[60]={MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,...,MIZAR_MIPI_CSI2_RB_REG_DPHY_RX};\n\ndefault_value_array[60]={...};\n\nread_mask_array[60]={...};\n\nwrite_mask_array[60]={...};\nEND_SOURCE_FILE\n\nSOURCE_FILE: program.c\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\nEND_SOURCE_FILE',
        'OTHER_DECLARATIONS\nSOURCE_FILE: program.c\nLocal variable type usage visible in function bodies:\n  unsigned long int i,addr; (in chk_rst_val and chk_rd_wr)\n  int default_value; (in soft_reset_chk)\nNo typedef, struct, union, or enum definitions visible in supplied source.',
        'This testcase (mipi_csi2_rb_reg_wr_rd_test) is a MIPI CSI-2 Receive Block register write/read verification test for the MIPI_CSI IP. It performs: (1) chk_rst_val - iterates over 60 registers via addr_array, reads each using read_reg() and compares against default_value_array, skipping registers where read_mask_array is 0x00000000, tracking mismatches in def_fail_cnt; (2) chk_rd_wr - writes 0xFFFFFFFF to all writable registers (skipping those in skip_array or with write_mask_array==0x00000000), then reads back and compares data_rd against (data_wr & write_mask_array[i]), tracking mismatches in wr_fail_cnt; (3) soft_reset_chk - performs a soft reset by writing SOFT_RST_REG_DATA to SOFT_RST_REG_ADDRESS, waiting via wait_on(1000), then restoring the original value; (4) chk_rst_val is called again after soft reset to re-verify defaults. Final pass/fail is determined by (def_fail_cnt == 0) && (wr_fail_cnt == 0), returning 0 for PASS and 1 for FAIL.',
        'Full source code of test_define.c and program.c as provided by Agent 5A.',
        'Source: AGENT_4_OUTPUT (198 total mappings)\n\nMATCHED REGISTER MAPPINGS (44): virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch1_Instruction, dphy_rx',
        'Validation criteria as provided by Agent 5A covering chk_rst_val, chk_rd_wr, and final test_case pass/fail logic.'
    ]
    for i,v in enumerate(md,1):
        c = wm.cell(row=2,column=i,value=v); c.alignment=wa

    md_widths = [40,50,50,60,50,60,60,60,60]
    for i,w in enumerate(md_widths,1):
        wm.column_dimensions[get_column_letter(i)].width = w
    wm.freeze_panes = 'A2'
    wm.sheet_state = 'veryHidden'

    # Save
    wb.save(FILENAME)
    fsize = os.path.getsize(FILENAME)
    print(f'Generated {FILENAME} ({fsize} bytes)')

    # Validate
    wb2 = openpyxl.load_workbook(FILENAME)
    assert 'TestPlan' in wb2.sheetnames, 'Missing TestPlan sheet'
    assert 'MetaData' in wb2.sheetnames, 'Missing MetaData sheet'
    assert wb2['MetaData'].sheet_state == 'veryHidden', 'MetaData not veryHidden'
    assert wb2['TestPlan'].max_row - 1 == 1, f'TestPlan row count wrong: {wb2["TestPlan"].max_row - 1}'
    assert wb2['MetaData'].max_row - 1 == 1, f'MetaData row count wrong: {wb2["MetaData"].max_row - 1}'
    assert len(wb2['TestPlan'].data_validations.dataValidation) > 0, 'No data validation'
    assert wb2['TestPlan'].cell(row=2, column=14).value == '' or wb2['TestPlan'].cell(row=2, column=14).value is None or wb2['TestPlan'].cell(row=2, column=14).value == '', 'Code Gen column not blank'
    print('All validations PASSED')

    # Output base64
    with open(FILENAME, 'rb') as f:
        b64 = base64.b64encode(f.read()).decode()
    print(f'BASE64_LENGTH={len(b64)}')
    with open(FILENAME + '.b64', 'w') as f:
        f.write(b64)
    print(f'Base64 written to {FILENAME}.b64')
    return FILENAME

if __name__ == '__main__':
    main()
