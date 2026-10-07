#!/usr/bin/env python3
"""MIPI CSI TestPlan XLSX Generator - Agent 7
Execute this script to generate the XLSX workbook and push to GitHub.
Requires: openpyxl, PyGithub

Usage: python generate_and_push_mipi_csi_testplan.py
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from io import BytesIO
from datetime import datetime, timezone, timedelta
import base64
import json
import os
import sys

try:
    from github import Github
except ImportError:
    print('PyGithub not installed. Install with: pip install PyGithub')
    print('Will generate XLSX locally only.')
    Github = None

# ============================================================
# IST Timezone
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'

# ============================================================
# Input JSON Data
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000; LS_LE_EN = 1; TOTAL_FRAME = 1; VC_ID = 3; VRES = 1080; HRES = 1920; DATA_TYPE = CSI2_RGB888; VRES = 3; HRES = 64",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through lane counts from 4 lanes down to 1 lane. It first calls csi2_enable_interrupt() which reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear interrupts, then writes interrupt mask registers (MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with 0x0000000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with 0x00000003, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE with 0x000f000f, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID with 0x0000ffff, MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED with 0x0000ffff). Then it configures the virtual channel by writing MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg (derived from VC_ID shifted based on GDMA path), and enables control data transfer by writing MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 1. These two writes are performed twice. It then calls snps_phy_init() for D-PHY initialization. After PHY init, it polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f, confirming PHY stop state. Then it enters a loop from lane_num=3 down to 0, writing MIZAR_MIPI_CSI2_HOST_N_LANES with lane_num to configure the number of active lanes, and writing 0xa0243ffc with (lane_num+1) to trigger the CSI-2 sequence. Inside the lane loop, for each control packet (cntrl_pkt_cnt = VRES*3 + 2 iterations), it enables DMA interrupts, calls dma_trnsfr_instn_preload() for control data transfer (src_addr=0x8000, dest_addr=0xE6001000, trnsfr_size=8, irq_num=0), calls DMAGO_CSI() to start DMA channel 0, polls gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set, clears the DMA interrupt, reads control data from 0xE6001000, and if the data type field (bits[5:0]) > 0xf, calculates word_count from bits[21:6], computes csi_data_size aligned to 8 bytes, calls dma_trnsfr_instn_preload() for data transfer (src_addr=0x0000, dest_addr=0xE6002000, trnsfr_size=csi_data_size, irq_num=1), calls DMAGO_CSI() to start DMA channel 1, polls until bit 1 is set, and clears the DMA interrupt. Finally calls finish(0).",
        "Test Description": "This test validates MIPI CSI-2 D-PHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). It enables CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts and then writing all interrupt mask registers with appropriate enable values. It configures the virtual channel register and enables control data transfer. After performing D-PHY initialization, it polls the PHY stop state register until the PHY enters stop state. For each lane configuration, it sets the number of active lanes, triggers the CSI-2 sequence, and then performs DMA-based control and data packet transfers in a loop. Each iteration involves DMA programming, interrupt-based completion polling, control packet readback, and conditional data packet transfer based on the received data type. The test verifies correct operation across all lane configurations.",
        "Meta Test Steps / Procedure": "1. Entry: test_case() is called. 2. Call csi2_enable_interrupt(). 3. Inside csi2_enable_interrupt(): read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) to clear pending interrupts, store in rd_data. 4. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) to enable PHY fatal interrupts. 5. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) to enable packet fatal interrupts. 6. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) to enable PHY interrupts. 7. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) to enable line interrupts. 8. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) to enable boundary frame fatal interrupts. 9. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) to enable sequence frame fatal interrupts. 10. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) to enable CRC frame fatal interrupts. 11. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) to enable payload CRC fatal interrupts. 12. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) to enable data ID interrupts. 13. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) to enable ECC corrected interrupts. 14. Return from csi2_enable_interrupt(). 15. Determine vcid_csi2_wrap_reg based on GDMA path. 16. Set gdma_reg_base = 0xE6A00000. 17-54. Configure registers and perform DMA transfers for each lane configuration.",
        "Test Steps / Procedure": "1. Enable CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected) with their respective enable values. 2. Configure the CSI-2 virtual channel register based on the selected GDMA path and virtual channel ID. 3. Enable control data transfer by writing the control data register. 4. Repeat the virtual channel and control data configuration (second write). 5. Perform D-PHY initialization sequence. 6. Poll the PHY stop state register until the PHY enters stop state (all lanes and clock in stop state). 7. For each lane configuration (4 lanes down to 1 lane): a. Write the number of lanes register with the current lane count. b. Trigger the CSI-2 sequence by writing the trigger register with the lane count. c. For each expected control packet: i. Enable DMA interrupts for both channels. ii. Program DMA channel 0 for control data transfer and start the DMA. iii. Poll the DMA interrupt status register until channel 0 transfer completes. iv. Clear the DMA interrupt for channel 0. v. Read the received control data. vi. If the data type indicates a valid image data type, calculate the transfer size, program DMA channel 1 for data transfer, start the DMA, poll for completion, and clear the interrupt. 8. Complete the test.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIPI_CSI2_RB_REG_CONTROL_DATA; MIPI_CSI2_HOST_PHY_STOPSTATE; MIPI_CSI2_HOST_N_LANES; MIPI_CSI2_HOST_INT_ST_MAIN; MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIPI_CSI2_HOST_INT_MSK_PHY; MIPI_CSI2_HOST_INT_MSK_LINE; MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f, confirming all data lanes and clock lane are in stop state. DMA transfer completion is validated by polling gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET: for channel 0, bit 0 must be set (rd_data & 0x1 != 0x0); for channel 1, bit 1 must be set (rd_data & 0x2 != 0x0). After each DMA completion, the corresponding interrupt is cleared. The test iterates through all 4 lane configurations and processes (VRES*3 + 2) packets per lane configuration. The test completes by calling finish(0).",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must read a value indicating all data lanes and clock lane are in stop state before proceeding. 2. For each lane configuration (4 lanes down to 1 lane), the number of lanes register must be successfully written. 3. DMA channel 0 control data transfer must complete as indicated by the DMA interrupt status register bit 0 being set. 4. DMA channel 1 data transfer (when triggered by a valid data type in the control packet) must complete as indicated by the DMA interrupt status register bit 1 being set. 5. DMA interrupts must be properly cleared after each transfer completion. 6. The test must successfully iterate through all lane configurations and all expected packets per configuration. 7. The test completes successfully by calling the finish routine with a pass status.",
        "Remarks": "The virtual channel and control data registers are written twice in sequence, which may be intentional for ensuring configuration stability. The test uses conditional compilation (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) to select the GDMA path and corresponding virtual channel shift. The VRES and HRES values depend on whether GDMA0_FULL_MEM is defined (1080x1920 vs 3x64). Two hardcoded hex addresses (0xa0243ffc for CSI-2 sequence trigger, 0xE6001000 for control data readback) could not be mapped to named registers. The snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() function implementations are not available in the supplied testcase folder."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Test Pattern Generator",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator (PPI PG) functionality. The test configures the virtual channel register MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg (derived from vcid=3 and vcid_unselected_path=((vcid+1)&0xf) shifted based on GDMA path selection via GDMA3_PATH/GDMA2_PATH/GDMA1_PATH/GDMA0_PATH conditional compilation), and disables control data transfer by writing MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 0. It calls csi2_subsys_enable_interrupt() for interrupt setup. It then calls snps_phy_init() for D-PHY initialization and polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f. DMA address registers are configured: MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA with 0x100, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION with 0x0, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA with 0x0, MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION with 0x0. A write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 with 0x1 enables sending fracdiv output to the CSI-2 subsystem. The DMA transfer size csi2_data_trnsfr_size is computed from hres=320, vres=16, valid_bits_per_pixel=24, aligned to 8 bytes. dma_trnsfr_instn_preload_incr_addr() is called with src_addr=0x00, dest_addr=0xE6001000, trnsfr_size=csi2_data_trnsfr_size, src_incr_addr_flag=0, dest_incr_addr_flag based on FPS60 define, irq_num=0. DMAGO_CSI() starts DMA channel 0. Then csi2_ctrlr_pg_enable() is called which writes MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES with 0x10, MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES with 0x70140, MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG with 0xe401, and MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with 1. After wait_on(100), the pattern generator is disabled by writing MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with 0. The test then polls gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set (rd_data & 0x1 != 0), confirming DMA transfer completion. Finally wait_on(10000) and finish(0) are called. The csi2_enable_interrupt() function reads MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear interrupts, then writes all interrupt mask registers with their respective enable values.",
        "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator functionality. It configures the virtual channel register and disables control data transfer. After enabling CSI-2 and DMA interrupts, it performs D-PHY initialization and polls the PHY stop state register until the PHY enters stop state. It then programs the DMA higher-order address registers for channel 0 data and instruction paths, enables the fractional divider output to the CSI-2 subsystem, and programs a DMA transfer for receiving pattern data. The DMA channel 0 is started, and then the pattern generator is enabled with a vertical resolution of 16 lines, horizontal resolution of 320 pixels, and a specific configuration. After a short wait, the pattern generator is disabled. The test polls the DMA interrupt status until the channel 0 transfer completes, waits for a settling period, and then completes successfully.",
        "Meta Test Steps / Procedure": "1. Entry: test_case() is called. 2. Set int_pend = 1. 3. Set vcid = 3. 4. Compute vcid_unselected_path = ((vcid + 1) & 0xf). 5. Determine vcid_csi2_wrap_reg based on GDMA path. 6. Set gdma_path accordingly. 7. Compute gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000). 8. Print vcid_csi2_wrap_reg. 9. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg). 10. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0). 11. Call csi2_subsys_enable_interrupt(). 12. Call snps_phy_init(). 13-49. Configure DMA, enable pattern generator, poll for completion.",
        "Test Steps / Procedure": "1. Configure the CSI-2 virtual channel register based on the selected GDMA path and virtual channel ID, and disable control data transfer. 2. Enable CSI-2 and DMA interrupts by reading the main interrupt status register to clear pending interrupts, then writing all interrupt mask registers with their respective enable values. 3. Perform D-PHY initialization sequence. 4. Poll the PHY stop state register until the PHY enters stop state (all lanes and clock in stop state). 5. Compute the DMA transfer size based on horizontal resolution (320), vertical resolution (16), and bits per pixel (24), aligned to 8-byte boundaries. 6. Program the DMA higher-order AXI address registers for channel 0 data read, instruction read, data write, and instruction write paths. 7. Enable the fractional divider output to the CSI-2 subsystem by writing a register at a specific offset in the RB_REG block. 8. Program DMA channel 0 transfer instructions with source and destination addresses and the computed transfer size. 9. Start DMA channel 0. 10. Enable the internal test pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and enabling the generator. 11. Wait briefly, then disable the pattern generator. 12. Poll the DMA interrupt status register until channel 0 transfer completes (bit 0 set). 13. Wait for a settling period and complete the test.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIPI_CSI2_HOST_PPI_PG_CONFIG; MIPI_CSI2_HOST_PPI_PG_ENABLE; MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIPI_CSI2_RB_REG_CONTROL_DATA; MIPI_CSI2_HOST_PHY_STOPSTATE; MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIPI_CSI2_HOST_INT_ST_MAIN; MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIPI_CSI2_HOST_INT_MSK_PHY; MIPI_CSI2_HOST_INT_MSK_LINE; MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f, confirming all data lanes and clock lane are in stop state. DMA transfer completion is validated by polling gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set (rd_data & 0x1 != 0), confirming DMA channel 0 transfer has completed. The pattern generator is enabled with MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE = 1 and then disabled with MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE = 0 after wait_on(100). The test completes by calling finish(0) after wait_on(10000).",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must read a value indicating all data lanes and clock lane are in stop state before proceeding with DMA and pattern generator setup. 2. The DMA channel 0 transfer must complete as indicated by the DMA interrupt status register bit 0 being set. 3. The pattern generator must be successfully enabled and then disabled within the test sequence. 4. The test completes successfully by calling the finish routine with a pass status.",
        "Remarks": "The test uses conditional compilation (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) to select the GDMA path and corresponding virtual channel shift. The FPS60 define controls whether the DMA destination address increment flag is set to 0 or 1. The csi2_enable_interrupt() function is defined in the source file but the test calls csi2_subsys_enable_interrupt() which is an external function not available in the supplied testcase folder. The write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 targets a register at offset 0xf4 in the RB_REG block that could not be mapped to a named register. The snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() function implementations are not available in the supplied testcase folder."
    }
]

# ============================================================
# TestPlan Sheet Columns
# ============================================================
testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# ============================================================
# MetaData Sheet Columns
# ============================================================
metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

# ============================================================
# Create Workbook
# ============================================================
wb = openpyxl.Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

# ============================================================
# Formatting
# ============================================================
header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

# ============================================================
# Populate TestPlan Sheet
# ============================================================
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# ============================================================
# Populate MetaData Sheet
# ============================================================
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# ============================================================
# Auto-size columns
# ============================================================
MAX_WIDTH = 80
for ws in [ws_tp, ws_md]:
    for col_idx in range(1, ws.max_column + 1):
        max_len = 0
        col_letter = get_column_letter(col_idx)
        for row in ws.iter_rows(min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 2, MAX_WIDTH)
        if adjusted_width < 12:
            adjusted_width = 12
        ws.column_dimensions[col_letter].width = adjusted_width

# ============================================================
# Freeze first row
# ============================================================
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# ============================================================
# Set MetaData sheet to veryHidden
# ============================================================
ws_md.sheet_state = 'veryHidden'

# ============================================================
# Save workbook
# ============================================================
output_path = filename
wb.save(output_path)
print(f'Workbook saved: {output_path}')
print(f'File size: {os.path.getsize(output_path)} bytes')

# Verify
wb2 = openpyxl.load_workbook(output_path)
print(f'Sheets: {wb2.sheetnames}')
assert 'TestPlan' in wb2.sheetnames
assert 'MetaData' in wb2.sheetnames
print(f'TestPlan rows: {wb2["TestPlan"].max_row - 1}')
print(f'MetaData rows: {wb2["MetaData"].max_row - 1}')
print('Validation: PASSED')

# Output base64 for GitHub push
with open(output_path, 'rb') as f:
    b64 = base64.b64encode(f.read()).decode('ascii')
print(f'BASE64_LENGTH={len(b64)}')
print(f'FILENAME={filename}')

# Push to GitHub if PyGithub available
if Github and os.environ.get('GITHUB_TOKEN'):
    g = Github(os.environ['GITHUB_TOKEN'])
    repo = g.get_repo('titusbspgit/PSVValidation')
    with open(output_path, 'rb') as f:
        content = f.read()
    path = f'Test_Output/MIPI/TestPlan/{filename}'
    repo.create_file(path, f'Added generated TestPlan Excel - {filename}', content, branch='main')
    print(f'Pushed to GitHub: {path}')
else:
    print('GitHub push skipped - no token or PyGithub not available')
    print(f'Run manually: python {sys.argv[0]}')
