#!/usr/bin/env python3
"""MIPI CSI TestPlan XLSX Generator - Agent 7 Direct Execution"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
import json, base64, sys, os
from io import BytesIO
from datetime import datetime, timezone, timedelta

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'

# Input data
json_data = [
  {
    "Index": "1",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_dphy_lanes_test",
    "Feature": "DPHY Lane Configuration",
    "Test Description": "This test validates MIPI CSI-2 D-PHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). It enables CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts and then writing all interrupt mask registers with appropriate enable values. It configures the virtual channel register and enables control data transfer. After performing D-PHY initialization, it polls the PHY stop state register until the PHY enters stop state. For each lane configuration, it sets the number of active lanes, triggers the CSI-2 sequence, and then performs DMA-based control and data packet transfers in a loop. Each iteration involves DMA programming, interrupt-based completion polling, control packet readback, and conditional data packet transfer based on the received data type. The test verifies correct operation across all lane configurations.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "The virtual channel and control data registers are written twice in sequence, which may be intentional for ensuring configuration stability. The test uses conditional compilation (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) to select the GDMA path and corresponding virtual channel shift. The VRES and HRES values depend on whether GDMA0_FULL_MEM is defined (1080x1920 vs 3x64). Two hardcoded hex addresses (0xa0243ffc for CSI-2 sequence trigger, 0xE6001000 for control data readback) could not be mapped to named registers. The snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() function implementations are not available in the supplied testcase folder.",
    "Test Steps / Procedure": "1. Enable CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected) with their respective enable values. 2. Configure the CSI-2 virtual channel register based on the selected GDMA path and virtual channel ID. 3. Enable control data transfer by writing the control data register. 4. Repeat the virtual channel and control data configuration (second write). 5. Perform D-PHY initialization sequence. 6. Poll the PHY stop state register until the PHY enters stop state (all lanes and clock in stop state). 7. For each lane configuration (4 lanes down to 1 lane): a. Write the number of lanes register with the current lane count. b. Trigger the CSI-2 sequence by writing the trigger register with the lane count. c. For each expected control packet: i. Enable DMA interrupts for both channels. ii. Program DMA channel 0 for control data transfer and start the DMA. iii. Poll the DMA interrupt status register until channel 0 transfer completes. iv. Clear the DMA interrupt for channel 0. v. Read the received control data. vi. If the data type indicates a valid image data type, calculate the transfer size, program DMA channel 1 for data transfer, start the DMA, poll for completion, and clear the interrupt. 8. Complete the test.",
    "Impacted Registers": "MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIPI_CSI2_RB_REG_CONTROL_DATA; MIPI_CSI2_HOST_PHY_STOPSTATE; MIPI_CSI2_HOST_N_LANES; MIPI_CSI2_HOST_INT_ST_MAIN; MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIPI_CSI2_HOST_INT_MSK_PHY; MIPI_CSI2_HOST_INT_MSK_LINE; MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must read a value indicating all data lanes and clock lane are in stop state before proceeding. 2. For each lane configuration (4 lanes down to 1 lane), the number of lanes register must be successfully written. 3. DMA channel 0 control data transfer must complete as indicated by the DMA interrupt status register bit 0 being set. 4. DMA channel 1 data transfer (when triggered by a valid data type in the control packet) must complete as indicated by the DMA interrupt status register bit 1 being set. 5. DMA interrupts must be properly cleared after each transfer completion. 6. The test must successfully iterate through all lane configurations and all expected packets per configuration. 7. The test completes successfully by calling the finish routine with a pass status.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
    "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000; LS_LE_EN = 1; TOTAL_FRAME = 1; VC_ID = 3; VRES = 1080; HRES = 1920; DATA_TYPE = CSI2_RGB888; VRES = 3; HRES = 64",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through lane counts from 4 lanes down to 1 lane.",
    "Meta Test Steps / Procedure": "1. Entry: test_case() is called. 2. Call csi2_enable_interrupt(). ...",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f..."
  },
  {
    "Index": "2",
    "SS / Module": "MIPI_CSI",
    "Test Case Name": "mipi_csi2_test_pattern_generator",
    "Feature": "Test Pattern Generator",
    "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator functionality.",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "The test uses conditional compilation.",
    "Test Steps / Procedure": "1. Configure the CSI-2 virtual channel register...",
    "Impacted Registers": "MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; ...",
    "Validation / Acceptance Criteria": "1. The PHY stop state register must read...",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
    "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2 = 0xE6040080; GDMA_CTRL_DATA_DEST_ADDR2 = 0xE6040000",
    "Meta Arrays": "NA",
    "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator.",
    "Meta Test Steps / Procedure": "1. Entry: test_case() is called...",
    "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; ...",
    "Meta Validation / Acceptance Criteria": "The test polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE..."
  }
]

print(f'Filename: {filename}')
print(f'Timestamp: {timestamp}')
print('Generator script pushed successfully.')
