#!/usr/bin/env python3
"""Agent 7 - MIPI_CSI TestPlan Excel Generator
Generates MIPI_CSI_TestPlan_20261105_151500.xlsx with:
- TestPlan sheet (visible) with data validation dropdown
- MetaData sheet (veryHidden)
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from openpyxl.worksheet.datavalidation import DataValidation
import json, base64, sys, os

def generate_workbook():
    wb = openpyxl.Workbook()

    # ── TestPlan Sheet ──
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    tp_headers = [
        "Index", "SS / Module", "Feature", "Test Case Name", "Speed", "Mode",
        "Memory Start Offset", "Memory End Offset", "Test Description",
        "Test Steps / Procedure", "Impacted Registers",
        "Validation / Acceptance Criteria", "Remarks",
        "Code Generation (Required / Not)"
    ]

    header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap_align = Alignment(wrap_text=True, vertical='top')

    for col_idx, header in enumerate(tp_headers, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    # TestPlan data row
    tp_data = [
        "1",
        "MIPI_CSI",
        "Register Write/Read Verification",
        "mipi_csi2_rb_reg_wr_rd_test",
        "NA",
        "NA",
        "NA",
        "NA",
        "This testcase verifies the register write/read functionality of the MIPI CSI-2 Receive Block. The test validates that all 60 registers in the register block hold their expected default values after reset, that writable registers can be written and read back correctly respecting their write masks, and that a soft reset restores all registers to their default values. The test covers control registers, interrupt registers, FIFO configuration registers, DPHY configuration registers, DMA address registers, and boot configuration registers.",
        "Step 1: Read all 60 MIPI CSI-2 RB registers and verify each register contains its expected default (reset) value. Registers that are not readable (read mask is zero) are skipped. Any mismatch is recorded as a default value check failure.\nStep 2: Write an all-ones pattern to all writable registers. Registers flagged in the skip array or registers that are not writable (write mask is zero) are skipped during the write phase.\nStep 3: Read back all written registers and verify the read data matches the expected value (all-ones pattern masked with the register\u2019s write mask). Any mismatch is recorded as a write/read check failure.\nStep 4: Perform a soft reset by writing the soft reset data value to the soft reset register, waiting for the reset to complete, and then restoring the original register value.\nStep 5: After the soft reset, read all 60 registers again and verify each register has returned to its expected default value.\nStep 6: Evaluate the overall test result. The test passes if both the default value check failure count and the write/read check failure count are zero. Otherwise, the test fails.",
        "virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch1_Instruction, dphy_rx",
        "1. Default Value Check: Each readable register must return its expected default value when read after power-on reset. The read data must exactly match the predefined default value for that register.\n2. Write/Read Verification: After writing an all-ones pattern to each writable register, the read-back value must equal the written value masked with the register\u2019s write mask. This confirms that all writable bit fields accept and retain data correctly, and read-only bits remain unaffected.\n3. Soft Reset Recovery: After performing a soft reset sequence, all registers must return to their default values. The post-reset default value check must pass with zero failures.\n4. Overall Pass Criteria: The test passes (returns 0) if and only if both the default value failure count and the write/read failure count are zero. Any non-zero failure count results in test failure (returns 1).",
        "The test iterates over 60 registers covering control, interrupt, FIFO, DPHY, DMA address, and boot configuration register groups. Some registers are skipped during write/read verification based on a skip array and write mask conditions. Seventeen address macros from the test source could not be resolved to register names in the IP register specification and may require manual verification.",
        ""  # Code Generation column - blank default
    ]

    for col_idx, value in enumerate(tp_data, 1):
        cell = ws_tp.cell(row=2, column=col_idx, value=value)
        cell.alignment = wrap_align

    # Data Validation dropdown for Code Generation column (column 14)
    dv = DataValidation(
        type="list",
        formula1='"Required,Not Required"',
        allow_blank=True,
        showDropDown=False,
        showErrorMessage=True,
        errorTitle="Invalid Entry",
        error="Please select Required or Not Required, or leave blank.",
        promptTitle="Code Generation",
        prompt="Select Required or Not Required"
    )
    dv.add('N2:N1048576')
    ws_tp.add_data_validation(dv)

    # Auto-size columns with max width cap
    for col_idx in range(1, len(tp_headers) + 1):
        max_len = len(str(tp_headers[col_idx - 1]))
        for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    longest = max(len(l) for l in lines)
                    if longest > max_len:
                        max_len = longest
        adjusted = min(max_len + 2, 60)
        ws_tp.column_dimensions[get_column_letter(col_idx)].width = adjusted

    # Freeze first row
    ws_tp.freeze_panes = 'A2'

    # ── MetaData Sheet ──
    ws_md = wb.create_sheet(title="MetaData")

    md_headers = [
        "Test Case Name", "Meta Headers", "Meta Macros", "Meta Arrays",
        "Meta Types", "Meta Test Description", "Meta Test Steps / Procedure",
        "Meta Impacted Registers", "Meta Validation / Acceptance Criteria"
    ]

    for col_idx, header in enumerate(md_headers, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=header)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    md_data = [
        "mipi_csi2_rb_reg_wr_rd_test",
        'SOURCE_FILE: program.c\n#include <stdio.h>\n#include <stdlib.h>\n#include "test_define.c"\n#include "test_common.h"\nEND_SOURCE_FILE\n\nSOURCE_FILE: test_define.c\n#include<mipi_csi2_rb_reg.h>\nEND_SOURCE_FILE',
        'SOURCE_FILE: test_define.c\n#define CNT 60\nEND_SOURCE_FILE\n\nSOURCE_FILE: program.c\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA\n#ifdef DEBUG_DISPLAY ... #endif (conditional compilation directive used throughout chk_rst_val, chk_rd_wr, test_case)\nEND_SOURCE_FILE',
        'SOURCE_FILE: test_define.c\nint data_rd,data_wr,rst_val;\n\naddr_array[60]={MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL,...,MIZAR_MIPI_CSI2_RB_REG_DPHY_RX};\n\ndefault_value_array[60]={...};\n\nread_mask_array[60]={...};\n\nwrite_mask_array[60]={...};\nEND_SOURCE_FILE\n\nSOURCE_FILE: program.c\nint data_rd,data_wr;\nint def_fail_cnt = 0,wr_fail_cnt = 0;\nEND_SOURCE_FILE',
        'OTHER_DECLARATIONS\nSOURCE_FILE: program.c\nLocal variable type usage visible in function bodies:\n  unsigned long int i,addr; (in chk_rst_val and chk_rd_wr)\n  int default_value; (in soft_reset_chk)\nNo typedef, struct, union, or enum definitions visible in supplied source.',
        'This testcase (mipi_csi2_rb_reg_wr_rd_test) is a MIPI CSI-2 Receive Block register write/read verification test for the MIPI_CSI IP. It performs: (1) chk_rst_val - iterates over 60 registers via addr_array, reads each using read_reg() and compares against default_value_array, skipping registers where read_mask_array is 0x00000000, tracking mismatches in def_fail_cnt; (2) chk_rd_wr - writes 0xFFFFFFFF to all writable registers (skipping those in skip_array or with write_mask_array==0x00000000), then reads back and compares data_rd against (data_wr & write_mask_array[i]), tracking mismatches in wr_fail_cnt; (3) soft_reset_chk - performs a soft reset by writing SOFT_RST_REG_DATA to SOFT_RST_REG_ADDRESS, waiting via wait_on(1000), then restoring the original value; (4) chk_rst_val is called again after soft reset to re-verify defaults. Final pass/fail is determined by (def_fail_cnt == 0) && (wr_fail_cnt == 0), returning 0 for PASS and 1 for FAIL.',
        'Full source code of test_define.c and program.c as provided by Agent 5A.',
        'Source: AGENT_4_OUTPUT (198 total mappings)\n\nMATCHED REGISTER MAPPINGS (44): virtual_channel, line_info, fifo_threshold_val, lane_clk, mem, interrupt_raw, interrupt_mask, interrupt_enable, control_data, flush, fifo_interrupt_raw, fifo_interrupt_enable, fifo_interrupt_mask, soft_rst, null_blank, dmac_boot, boot_addr_VC0, boot_addr_VC1, boot_addr_VC2, boot_addr_VC3, boot_irq_ns_VC0, boot_irq_ns_VC1, boot_irq_ns_VC2, boot_irq_ns_VC3, dphy_rx_cfgclk, dphy_rx_hs, dphy_IO_continuity, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m1_addr_ar_ch0_data, dma_m1_addr_ar_ch0_Instruction, dma_m2_addr_ar_ch0_data, dma_m2_addr_ar_ch0_Instruction, dma_m3_addr_ar_ch0_data, dma_m3_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, dma_m0_addr_ar_ch1_Instruction, dma_m1_addr_ar_ch1_Instruction, dma_m2_addr_ar_ch1_Instruction, dma_m0_addr_aw_ch1_Instruction, dma_m2_addr_aw_ch0_Instruction, dma_m2_addr_aw_ch1_Instruction, dphy_rx',
        'Validation criteria as provided by Agent 5A covering chk_rst_val, chk_rd_wr, and final test_case pass/fail logic.'
    ]

    for col_idx, value in enumerate(md_data, 1):
        cell = ws_md.cell(row=2, column=col_idx, value=value)
        cell.alignment = wrap_align

    # Auto-size MetaData columns
    for col_idx in range(1, len(md_headers) + 1):
        max_len = len(str(md_headers[col_idx - 1]))
        for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    longest = max(len(l) for l in lines)
                    if longest > max_len:
                        max_len = longest
        adjusted = min(max_len + 2, 60)
        ws_md.column_dimensions[get_column_letter(col_idx)].width = adjusted

    ws_md.freeze_panes = 'A2'

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'

    # Save workbook
    output_path = 'MIPI_CSI_TestPlan_20261105_151500.xlsx'
    wb.save(output_path)
    print(f'Workbook saved: {output_path}')
    print(f'File size: {os.path.getsize(output_path)} bytes')

    # Validate
    wb2 = openpyxl.load_workbook(output_path)
    assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
    assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
    assert wb2['MetaData'].sheet_state == 'veryHidden', 'MetaData not veryHidden'
    tp_rows = wb2['TestPlan'].max_row - 1
    md_rows = wb2['MetaData'].max_row - 1
    assert tp_rows == 1, f'TestPlan rows mismatch: {tp_rows}'
    assert md_rows == 1, f'MetaData rows mismatch: {md_rows}'
    # Check data validation exists
    assert len(wb2['TestPlan'].data_validations.dataValidation) > 0, 'No data validation found'
    print('Validation PASSED')
    print(f'TestPlan rows: {tp_rows}')
    print(f'MetaData rows: {md_rows}')

    # Output base64 for GitHub upload
    with open(output_path, 'rb') as f:
        b64 = base64.b64encode(f.read()).decode('utf-8')
    print(f'BASE64_START\n{b64}\nBASE64_END')
    return output_path

if __name__ == '__main__':
    generate_workbook()
