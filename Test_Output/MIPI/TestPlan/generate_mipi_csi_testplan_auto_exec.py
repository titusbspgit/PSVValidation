#!/usr/bin/env python3
"""Agent 7 - MIPI CSI TestPlan XLSX Generator
Generates a real Office Open XML workbook using openpyxl.
"""
import os
import json
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    import subprocess, sys
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# ============================================================
# INPUT DATA
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI CSI2 Subsystem",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "NA",
        "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase configures and enables the MIPI CSI-2 internal test pattern generator (PG) to generate a known image pattern, routes the generated data through the CSI-2 subsystem via a selected virtual channel and GDMA path, programs the DMA engine to transfer the received pixel data from the CSI-2 receive buffer to a destination memory address, waits for the DMA transfer completion interrupt, and then finishes. The test exercises the CSI-2 host controller pattern generator registers (PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE), the CSI-2 subsystem wrapper registers (virtual_channel, control_data), the D-PHY initialization and stop-state polling (PHY_STOPSTATE), the DMA higher-order address configuration registers (dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction), interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED), and the main interrupt status register (INT_ST_MAIN). The pattern generator is configured for a vertical resolution of 16 lines and horizontal resolution of 320 pixels with 24 valid bits per pixel. The DMA transfer size is computed based on these parameters with 8-byte alignment.",
        "Test Description": "Configure and enable the MIPI CSI-2 internal test pattern generator to produce a known image pattern (320x16 pixels, 24 bits per pixel). Route the generated data through a selected virtual channel and GDMA path. Program the DMA engine to transfer received pixel data to destination memory. Enable all relevant CSI-2 and DMA interrupts. Initialize the D-PHY and wait for stop-state. Start the pattern generator, wait, then disable it. Poll for DMA transfer completion interrupt. Wait and finish the test.",
        "Meta Test Steps / Procedure": "1. Global variable declarations:\n int data_rd, data_wr;\n int def_fail_cnt = 0, wr_fail_cnt = 0;\n int vcid_csi2_wrap_reg;\n int gdma_path;\n long int csi_ctrl_data;\n int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;\n extern int_pend;\n unsigned int tx_trnsfr_size;\n\n2. Function void csi2_ctrlr_pg_enable():\n 2a. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10);\n 2b. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140);\n 2c. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);\n 2d. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1);\n\n3. Function int test_case():\n 3a. Local variable declarations:\n long long int rx_desc, tx_desc;\n long long int csi2_data_trnsfr_size, single_axi_trnsfr_size;\n int vcid_unselected_path, vcid;\n int num_beats;\n unsigned long int dma_ch0_pc, dma_ch0_instn_preload_addr;\n unsigned int dmago_instn_0, dmago_instn_1, instn;\n int total_loop_cnt, lc1_iter, lc0_iter;\n int dma_dest_addr_incr_flag;\n int valid_bits_per_pixel;\n unsigned int rd_data;\n int vres, hres;\n 3b. int_pend = 1;\n 3c. printf(\"start line\\n\");\n 3d. vcid = 3;\n 3e. vcid_unselected_path = ((vcid + 1)&0xf);\n 3f. Conditional compilation for GDMA path selection:\n #ifdef GDMA3_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+vcid); gdma_path = 3;\n #elif GDMA2_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid<<4)+(vcid_unselected_path)); gdma_path = 2;\n #elif GDMA1_PATH: vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 1;\n #else (GDMA0_PATH): vcid_csi2_wrap_reg = ((vcid<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path)); gdma_path = 0;\n 3g. gdma_reg_base = 0xE6A00000 + ((gdma_path)*0x1000);\n 3h. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg);\n 3i. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);\n 3j. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);\n 3k. csi2_subsys_enable_interrupt();\n 3l. snps_phy_init();\n 3m. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);\n 3n. while(!(rd_data == 0x1000f)) { rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE); }\n 3o. hres = 320;\n 3p. vres = 16;\n 3q. valid_bits_per_pixel = 24;\n 3r. csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres)/8 )%8)? (((valid_bits_per_pixel * hres)/8 ) + 8 - (((valid_bits_per_pixel * hres)/8 )%8)) : ((valid_bits_per_pixel * hres)/8))*vres);\n 3s. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100);\n 3t. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);\n 3u. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);\n 3v. write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);\n 3w. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);\n 3x. dma_ch0_pc = 0xE6000000;\n 3y. dma_ch0_instn_preload_addr = dma_ch0_pc;\n 3z. Conditional compilation for FPS60: #ifdef FPS60: dma_dest_addr_incr_flag = 0; #else: dma_dest_addr_incr_flag = 1;\n 3aa. dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0);\n 3ab. DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0);\n 3ac. csi2_ctrlr_pg_enable();\n 3ad. wait_on(100);\n 3ae. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0);\n 3af. rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);\n 3ag. while((rd_data&0x1)==0) { rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET); }\n 3ah. wait_on(10000);\n 3ai. finish(0);\n\n4. Function void csi2_enable_interrupt():\n 4a. int rd_data;\n 4b. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);\n 4c. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);\n 4d. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);\n 4e. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);\n 4f. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);\n 4g. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);\n 4h. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);\n 4i. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);\n 4j. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);\n 4k. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);\n 4l. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);",
        "Test Steps / Procedure": "1. Set interrupt pending flag.\n2. Set virtual channel ID (vcid=3) and compute the unselected path value.\n3. Based on the selected GDMA path (GDMA0/1/2/3), compute the virtual channel wrapper register value and set the GDMA path index.\n4. Compute gdma_reg_base address from the GDMA path index.\n5. Write the computed virtual channel value to the virtual_channel register.\n6. Disable control data transfer by writing 0 to the control_data register.\n7. Enable CSI-2 and DMA interrupts by calling the interrupt enable routine, which reads INT_ST_MAIN to clear pending interrupts and writes interrupt mask values to INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, and INT_MSK_ECC_CORRECTED.\n8. Initialize the SNPS D-PHY.\n9. Poll PHY_STOPSTATE register until it reads 0x1000f (PHY in stop state).\n10. Set image parameters: hres=320, vres=16, valid_bits_per_pixel=24.\n11. Compute DMA transfer size with 8-byte alignment.\n12. Program DMA higher-order address registers: write 0x100 to dma_m0_addr_ar_ch0_data, write 0x0 to dma_m0_addr_ar_ch0_Instruction, write 0x0 to dma_m0_addr_aw_ch0_data, write 0x0 to dma_m0_addr_aw_ch0_Instruction.\n13. Enable fracdiv output to CSI-2 subsystem by writing 0x1 to base+0xf4.\n14. Set DMA channel 0 program counter and instruction preload address.\n15. Configure DMA transfer instruction preload with source address 0x00, destination address 0xE6001000, computed transfer size, and address increment flags.\n16. Issue DMAGO command for channel 0.\n17. Enable the pattern generator by calling csi2_ctrlr_pg_enable, which writes PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, PPI_PG_ENABLE=1.\n18. Wait 100 cycles.\n19. Disable the pattern generator by writing 0 to PPI_PG_ENABLE.\n20. Poll DMA interrupt masked status register until bit 0 is set (DMA transfer complete).\n21. Wait 10000 cycles.\n22. Call finish(0) to end the test.",
        "Meta Impacted Registers": "PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE, virtual_channel, control_data, PHY_STOPSTATE, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES, PPI_PG_PATTERN_HRES, PPI_PG_CONFIG, PPI_PG_ENABLE, virtual_channel, control_data, PHY_STOPSTATE, dma_m0_addr_ar_ch0_data, dma_m0_addr_ar_ch0_Instruction, dma_m0_addr_aw_ch0_data, dma_m0_addr_aw_ch0_Instruction, INT_ST_MAIN, INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY stop-state polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) must return 0x1000f before proceeding. The while loop condition is while(!(rd_data == 0x1000f)).\n2. DMA transfer completion polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) must have bit 0 set (rd_data & 0x1 != 0) to exit the polling loop. The while loop condition is while((rd_data&0x1)==0).\n3. Test completes by calling finish(0), indicating successful completion with return code 0.",
        "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must read 0x1000f indicating the D-PHY has entered stop state on all lanes before proceeding with DMA and pattern generator configuration.\n2. The DMA interrupt masked status register bit 0 must be set, indicating the DMA transfer has completed successfully.\n3. The test finishes with finish(0) indicating successful test completion.",
        "Remarks": "The testcase uses conditional compilation (#ifdef GDMA3_PATH, GDMA2_PATH, GDMA1_PATH, else GDMA0_PATH) to select which GDMA path to use. An additional conditional compilation (#ifdef FPS60) controls whether the DMA destination address increment flag is set. The function csi2_enable_interrupt() is defined in the source but the test_case() function calls csi2_subsys_enable_interrupt() which is not defined in this source file. The macro MIZAR_MIPI_CSI2_RB_REG_BASE is used with an offset (+0xf4) for fracdiv output enable but has unresolved register mapping in Agent 4 output. The extern declaration 'extern int_pend' is missing the type specifier 'int' in the source."
    }
]

# ============================================================
# TIMESTAMP (IST = UTC+05:30)
# ============================================================
ist = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(ist)
timestamp_str = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp_str}.xlsx'
output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)))
output_path = os.path.join(output_dir, filename)

# ============================================================
# TESTPLAN SHEET COLUMNS
# ============================================================
testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers',
    'Validation / Acceptance Criteria', 'Code Generation'
]

# ============================================================
# METADATA SHEET COLUMNS
# ============================================================
metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description',
    'Meta Test Steps / Procedure', 'Meta Impacted Registers',
    'Meta Validation / Acceptance Criteria', 'Meta Headers',
    'Meta Macros', 'Meta Arrays'
]

# ============================================================
# CREATE WORKBOOK
# ============================================================
wb = Workbook()

# Rename default sheet to TestPlan
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# Create MetaData sheet
ws_md = wb.create_sheet('MetaData')

# ============================================================
# FORMATTING STYLES
# ============================================================
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

# ============================================================
# POPULATE TESTPLAN SHEET
# ============================================================
# Write headers
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        if col_name == 'Code Generation':
            value = ''
        else:
            value = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# ============================================================
# POPULATE METADATA SHEET
# ============================================================
# Write headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write data rows
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# ============================================================
# AUTO-SIZE COLUMNS
# ============================================================
def auto_size_columns(ws, max_width=60):
    for col_cells in ws.columns:
        max_len = 0
        col_letter = get_column_letter(col_cells[0].column)
        for cell in col_cells:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    if len(line) > max_len:
                        max_len = len(line)
        adjusted_width = min(max(max_len + 2, 12), max_width)
        ws.column_dimensions[col_letter].width = adjusted_width

auto_size_columns(ws_tp)
auto_size_columns(ws_md)

# ============================================================
# FREEZE FIRST ROW
# ============================================================
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# ============================================================
# SET METADATA SHEET TO VERY HIDDEN
# ============================================================
ws_md.sheet_state = 'veryHidden'

# ============================================================
# SAVE WORKBOOK
# ============================================================
wb.save(output_path)
print(f'Workbook saved: {output_path}')

# ============================================================
# POST-SAVE VALIDATION
# ============================================================
assert os.path.exists(output_path), 'File does not exist'
assert os.path.getsize(output_path) > 0, 'File is empty'

# Verify workbook can be reopened
wb_verify = load_workbook(output_path)
assert 'TestPlan' in wb_verify.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb_verify.sheetnames, 'MetaData sheet missing'
assert wb_verify['MetaData'].sheet_state == 'veryHidden', 'MetaData not veryHidden'

# Count rows
tp_rows = ws_tp.max_row - 1  # exclude header
md_rows = ws_md.max_row - 1  # exclude header

print(f'Validation PASSED')
print(f'Filename: {filename}')
print(f'TestPlan rows: {tp_rows}')
print(f'MetaData rows: {md_rows}')
print(f'File size: {os.path.getsize(output_path)} bytes')

wb_verify.close()
wb.close()
