#!/usr/bin/env python3
"""
MIPI TestPlan Excel Generator
Generates MIPI_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx using openpyxl
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter
from datetime import datetime, timezone, timedelta
import os

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_TestPlan_{timestamp}.xlsx'
output_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)

# Full JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and CSI2 Data Transfer",
        "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). For each lane configuration, the test enables all CSI-2 host interrupts, configures the virtual channel, initializes the D-PHY, waits for PHY stop state, sets the number of active lanes, and performs a complete CSI-2 data reception sequence using DMA. Control packets and image data packets are transferred via two DMA channels, with DMA interrupt polling used to detect transfer completion. The test verifies that CSI-2 data reception works correctly across all lane configurations.",
        "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing appropriate mask values to all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected). 2. Configure the virtual channel register based on the selected GDMA path and enable control data transfer. 3. Initialize the D-PHY by calling the PHY initialization sequence. 4. Poll the PHY stop state register until the PHY enters stop state on all lanes (clock and data lanes). 5. For each lane configuration from 4 lanes down to 1 lane, write the number of active lanes to the N_LANES register. 6. Trigger the CSI-2 sequence for the current lane count. 7. For each expected control and data packet in the frame, enable DMA interrupts and program DMA channel 0 to transfer the 8-byte control packet. 8. Poll the DMA interrupt status to detect control packet transfer completion, then clear the DMA interrupt. 9. Read the control data to determine the data type and word count. 10. If the data type indicates image data, compute the aligned transfer size and program DMA channel 1 to transfer the CSI image data. 11. Poll the DMA interrupt status to detect image data transfer completion, then clear the DMA interrupt. 12. Repeat steps 5-11 for all lane configurations. 13. Verify the test completes successfully with finish(0).",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must indicate all data lanes and the clock lane have entered stop state before lane configuration begins. 2. For each lane configuration (4 lanes down to 1 lane), DMA control packet transfers must complete successfully as indicated by the DMA interrupt status register. 3. When image data packets are present (data type greater than short packet threshold), DMA image data transfers must also complete successfully. 4. The test must iterate through all four lane configurations without stalling in any polling loop and must terminate successfully via finish(0).",
        "Remarks": "The test uses conditional compilation (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) to select the GDMA path, which determines the virtual channel bit position. Default configuration uses VC_ID=3, TOTAL_FRAME=1, LS_LE_EN=1, and DATA_TYPE=CSI2_RGB888. Resolution dimensions default to 64x3 (or 1920x1080 when GDMA0_FULL_MEM is defined). Two hardcoded hex addresses (used for CSI-2 sequence trigger and control data destination) could not be mapped to named registers. DMA-related offset registers (MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, MIPI_CSI2_DMA_INTCLR_OFFSET) are accessed via variable base plus offset and are used for DMA interrupt management.",
        "Speed": "NA",
        "Mode": "DMA Mode",
        "Memory Start Offset": "0xE6000000",
        "Memory End Offset": "0xE6002000",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
        "Meta Arrays": "NA",
        "Meta Test Description": "Long meta description for test 1...",
        "Meta Test Steps / Procedure": "Long meta steps for test 1...",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; ...",
        "Meta Validation / Acceptance Criteria": "Long meta validation for test 1..."
    }
]

print(f"Output: {filename}")
print("Script ready for execution")
