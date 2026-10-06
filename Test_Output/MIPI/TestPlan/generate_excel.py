#!/usr/bin/env python3
"""Generate MIPI_CSI TestPlan Excel workbook using openpyxl."""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_CSI_TestPlan_{timestamp}.xlsx'

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and CSI2 Data Transfer",
        "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by testing data reception across all supported lane counts (4 lanes down to 1 lane). The test enables CSI-2 interrupts by clearing pending interrupts via the main interrupt status register and configuring all interrupt mask registers for PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected events. It configures the virtual channel and enables control data transfer. After D-PHY initialization, it polls the PHY stop state register until all lanes and the clock lane confirm stop state entry. For each lane count (4 to 1), the N_LANES register is configured and a signal is sent to start the CSI-2 sequence. For each expected packet, DMA channel 0 transfers control data, the test waits for DMA completion via interrupt polling, reads the control packet, and if the data type indicates a long packet, programs DMA channel 1 to transfer the image data and waits for its completion. The test verifies successful data reception across all lane configurations.",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Remarks": "The test iterates lane_num from 3 to 0, writing each value to N_LANES, effectively testing 4-lane, 3-lane, 2-lane, and 1-lane DPHY configurations. The virtual channel and control data registers are written twice (repeated writes). The D-PHY initialization is performed via snps_phy_init() whose implementation is external to this testcase. DMA transfer functions dma_trnsfr_instn_preload() and DMAGO_CSI() are also external. Two hex addresses (used for signaling and DMA status) could not be mapped to named registers in the specification documents. Default configuration uses VC_ID=3, VRES=3, HRES=64, DATA_TYPE=CSI2_RGB888 unless overridden by compile-time defines.",
        "Test Steps / Procedure": "1. Enable CSI-2 interrupts by reading the main interrupt status register to clear pending interrupts, then configure all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, ECC corrected) with appropriate enable masks. 2. Configure the virtual channel register with the selected virtual channel ID based on the GDMA path. 3. Enable control data transfer by writing to the control data register. 4. Perform D-PHY initialization sequence. 5. Poll the PHY stop state register until all data lanes and the clock lane confirm stop state entry (expected value indicates all lanes stopped). 6. For each lane configuration (4 lanes down to 1 lane): a. Write the lane count to the N_LANES register. b. Signal the start of the CSI-2 sequence for the current lane count. c. For each expected packet in the frame: i. Enable DMA interrupts for both channels. ii. Program and start DMA channel 0 for control data transfer. iii. Poll the DMA interrupt status register until channel 0 completion is indicated. iv. Clear the DMA channel 0 interrupt. v. Read the received control packet data. vi. If the control packet indicates a long packet (image data), calculate the transfer size, program and start DMA channel 1 for image data transfer, poll for channel 1 completion, and clear the channel 1 interrupt. 7. Verify test completes successfully for all lane configurations.",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must read the expected value confirming all data lanes and the clock lane have entered stop state before proceeding with lane configuration. 2. For each DMA control data transfer, the DMA interrupt status must indicate channel 0 completion. 3. For each DMA image data transfer (when a long packet is detected), the DMA interrupt status must indicate channel 1 completion. 4. The test must successfully iterate through all lane configurations (4 lanes down to 1 lane) and complete all packet transfers for each configuration. 5. The test must call the finish routine with a pass indication (0) after all lane configurations are validated.",
        "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"",
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
        "Meta Arrays": "NA",
        "Meta Test Description": "(full meta test description)",
        "Meta Test Steps / Procedure": "(full meta test steps)",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; ...",
        "Meta Validation / Acceptance Criteria": "(full meta validation)"
    }
]

print(f'Filename: {filename}')
print('Script ready for execution')
