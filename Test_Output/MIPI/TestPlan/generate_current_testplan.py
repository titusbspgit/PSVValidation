#!/usr/bin/env python3
"""Agent 7 - Direct XLSX Generator for MIPI_CSI TestPlan.
Generates the workbook, saves locally, and commits to repo.
Run: python3 generate_current_testplan.py
Generated: 2026-10-07 IST
"""
import os, sys, json, base64
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'

json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "DPHY Lane Configuration",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
    "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000; LS_LE_EN = 1; TOTAL_FRAME = 1; VC_ID = 3; VRES; HRES; DATA_TYPE = CSI2_RGB888",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration by iterating through lane counts from 4 lanes down to 1 lane. For each lane configuration, the test first calls csi2_enable_interrupt() which reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear interrupts, then writes interrupt mask registers (MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with 0x0000000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with 0x00000003, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED with 0x0000ffff). It then writes MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg (derived from VC_ID shifted based on GDMA path), writes MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 1 to enable control data transfer, calls snps_phy_init() for D-PHY initialization, and polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until it equals 0x1000f. A for loop iterates lane_num from 3 down to 0, writing MIZAR_MIPI_CSI2_HOST_N_LANES with lane_num and writing 0xa0243ffc with (lane_num+1) to trigger the CSI-2 sequence.",
    "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). For each lane configuration, the test enables CSI-2 interrupts by clearing the main interrupt status and enabling all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected). It then configures the virtual channel, enables control data transfer, initializes the D-PHY, and polls the PHY stop state register until the PHY reaches the expected stop state. For each lane count, the N_LANES register is written with the lane number, a trigger write initiates the CSI-2 sequence, and a nested loop processes control and data packets using DMA transfers. Each DMA transfer involves preloading transfer instructions, starting the DMA channel, polling the DMA interrupt status for completion, clearing the interrupt, and reading control data to determine if a data payload transfer is needed. The test verifies correct operation across all lane configurations.",
    "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt() function.\n2. Inside csi2_enable_interrupt(): Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts.\n3. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with value 0x0000000f to enable PHY fatal interrupts.\n4. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with value 0x00000003 to enable packet fatal interrupts.\n5. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY with value 0x000f000f to enable PHY interrupts.\n6. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE with value 0x000f000f to enable line interrupts.\n7. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL with value 0x0000ffff to enable boundary frame fatal interrupts.\n8. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL with value 0x0000ffff to enable sequence frame fatal interrupts.\n9. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL with value 0x0000ffff to enable CRC frame fatal interrupts.\n10. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL with value 0x0000ffff to enable payload CRC fatal interrupts.\n11. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID with value 0x0000ffff to enable data ID interrupts.\n12. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED with value 0x0000ffff to enable ECC corrected interrupts.\n13. Return from csi2_enable_interrupt().\n14-50. Configure virtual channel, control data, PHY init, lane iteration, DMA transfers.",
    "Test Steps / Procedure": "1. Enable CSI-2 interrupts by clearing the main interrupt status register and enabling all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected).\n2. Configure the virtual channel register with the appropriate virtual channel ID based on the selected GDMA path.\n3. Enable control data transfer by writing to the control data register.\n4. Initialize the D-PHY by calling the PHY initialization sequence.\n5. Poll the PHY stop state register until the PHY reaches the expected stop state.\n6. For each lane configuration (4 lanes down to 1 lane), write the lane count to the N_LANES register.\n7. Trigger the CSI-2 sequence by writing the lane count to the trigger register.\n8. For each control packet in the frame, enable DMA interrupts for both channels.\n9. Preload DMA transfer instructions for channel 0 (control data) and start the DMA channel 0 transfer.\n10. Poll the DMA interrupt status register until channel 0 transfer completes (bit 0 set), then clear the DMA interrupt.\n11. Read the CSI control data from the destination address to determine the packet type.\n12. If the packet contains image data (data type greater than short packet threshold), extract the word count, compute the aligned data size, preload DMA transfer instructions for channel 1 (image data), and start DMA channel 1 transfer.\n13. Poll the DMA interrupt status register until channel 1 transfer completes (bit 1 set), then clear the DMA interrupt.\n14. Repeat steps 8-13 for all packets in the frame.\n15. Repeat steps 6-14 for all lane configurations.\n16. Complete the test by calling the finish routine.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until rd_data equals 0x1000f. DMA channel 0 completion validated by polling until bit 0 is set. DMA channel 1 completion validated by polling until bit 1 is set. The test calls finish(0) upon successful completion.",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must reach the expected stop state value confirming all lanes have entered stop state after PHY initialization.\n2. DMA channel 0 transfer must complete successfully, indicated by the DMA interrupt status register showing channel 0 completion.\n3. DMA channel 1 transfer must complete successfully for image data packets, indicated by the DMA interrupt status register showing channel 1 completion.\n4. Control data must be correctly read from the destination address after each DMA channel 0 transfer to determine packet type.\n5. The test must successfully iterate through all lane configurations (4 lanes down to 1 lane) and process all control and data packets for each configuration.\n6. The test completes by calling the finish routine with a pass indication.",
    "Remarks": "The test iterates lane_num from 3 down to 0, testing 4-lane, 3-lane, 2-lane, and 1-lane configurations. VRES and HRES default values depend on GDMA0_FULL_MEM conditional compilation (1080x1920 or 3x64). The default data type is CSI2_RGB888. Two hex addresses (0xa0243ffc and 0xE6001000) could not be mapped to named registers in the specification documents."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generator",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
    "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000",
    "Meta Arrays": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator (PPI PG) functionality. The test configures the CSI-2 subsystem to receive data generated by the host controller's built-in pattern generator rather than from an external D-PHY source.",
    "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator functionality. The test configures the CSI-2 subsystem to receive internally generated test pattern data instead of external D-PHY input. It configures the virtual channel register, disables control data transfer, enables all CSI-2 interrupt masks (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected), initializes the D-PHY, and polls the PHY stop state register until the PHY reaches the expected stop state. DMA address registers for channel 0 read and write paths are configured for data and instruction addresses. DMA transfer instructions are preloaded for a computed transfer size based on a 320x16 resolution at 24 bits per pixel with 8-byte alignment, and DMA channel 0 is started. The pattern generator is then enabled with specific vertical resolution, horizontal resolution, and configuration parameters. After a brief wait, the pattern generator is disabled, and the DMA interrupt status is polled until the transfer completes. The test verifies that internally generated test pattern data is correctly received and transferred via DMA.",
    "Meta Test Steps / Procedure": "1. Set int_pend = 1.\n2. Set vcid = 3.\n3-6. Compute vcid_csi2_wrap_reg and gdma_reg_base.\n7. Write MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL.\n8. Write MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 0.\n9-21. Enable interrupts.\n22-24. PHY init and poll.\n25-36. DMA configuration and start.\n37-42. Enable pattern generator.\n43-48. Disable PG, poll DMA, finish.",
    "Test Steps / Procedure": "1. Configure the virtual channel register with the appropriate virtual channel ID based on the selected GDMA path.\n2. Disable control data transfer by writing to the control data register.\n3. Enable CSI-2 interrupts by clearing the main interrupt status register and enabling all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected).\n4. Initialize the D-PHY by calling the PHY initialization sequence.\n5. Poll the PHY stop state register until the PHY reaches the expected stop state.\n6. Configure DMA address registers for channel 0 read and write paths (data and instruction addresses).\n7. Enable the fracdiv output to the CSI-2 subsystem.\n8. Compute the total DMA transfer size based on 320x16 resolution at 24 bits per pixel with 8-byte alignment.\n9. Preload DMA transfer instructions for channel 0 with the computed transfer size and start DMA channel 0.\n10. Enable the internal test pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and enabling the pattern generator output.\n11. Wait briefly, then disable the pattern generator.\n12. Poll the DMA interrupt status register until channel 0 transfer completes (bit 0 set).\n13. Wait for a settling period and complete the test by calling the finish routine.",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until rd_data equals 0x1000f. DMA channel 0 completion validated by polling until bit 0 is set. The test calls finish(0) upon successful completion.",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must reach the expected stop state value confirming all lanes have entered stop state after PHY initialization.\n2. The internal test pattern generator must be successfully enabled and then disabled after a wait period.\n3. DMA channel 0 transfer must complete successfully, indicated by the DMA interrupt status register showing channel 0 completion (bit 0 set).\n4. The test completes by calling the finish routine with a pass indication.",
    "Remarks": "This test uses the CSI-2 host controller's internal PPI pattern generator instead of an external D-PHY source. The pattern generator is configured with vres=0x10 (16 lines), hres=0x70140 (encoding both horizontal parameters), and config=0xe401. The DMA transfer size is computed for 320x16 resolution at 24 bits per pixel with 8-byte alignment."
  }
]

testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

wb = Workbook()
ws_tp = wb.active
ws_tp.title = 'TestPlan'

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

ws_tp.freeze_panes = 'A2'

for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = ws_tp.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_tp.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

ws_md = wb.create_sheet('MetaData')

for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

ws_md.freeze_panes = 'A2'

for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        val = ws_md.cell(row=row_idx, column=col_idx).value
        if val:
            max_len = max(max_len, min(len(str(val)), 80))
    ws_md.column_dimensions[get_column_letter(col_idx)].width = min(max_len + 2, 60)

ws_md.sheet_state = 'veryHidden'

wb.save(filename)

# Validate
wb2 = load_workbook(filename)
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
file_size = os.path.getsize(filename)
assert file_size > 0

print(f'FILENAME={filename}')
print(f'FILE_SIZE={file_size}')
print(f'VALIDATION=PASSED')
print(f'ROWS_TESTPLAN={len(json_data)}')
print(f'ROWS_METADATA={len(json_data)}')
