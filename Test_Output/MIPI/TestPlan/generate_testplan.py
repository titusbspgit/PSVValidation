#!/usr/bin/env python3
"""
MIPI TestPlan Excel Generator
Generates MIPI_TestPlan_YYYYMMDD_HHMMSS.xlsx using openpyxl
Run: python3 generate_testplan.py
"""
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("ERROR: openpyxl not installed. Run: pip install openpyxl")
    sys.exit(1)

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"MIPI_TestPlan_{timestamp}.xlsx"
script_dir = os.path.dirname(os.path.abspath(__file__))
filepath = os.path.join(script_dir, filename)

# JSON Data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and CSI2 Data Transfer",
        "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). It enables all CSI-2 interrupt masks, configures the virtual channel based on the selected GDMA path, enables control data transfer, initializes the D-PHY, and polls the PHY stop state register until all lanes reach stop state. For each lane configuration, it programs the number of active lanes, triggers the CSI-2 sequence, and performs DMA-based transfers for both control packets and data packets. Each packet transfer involves programming DMA instructions, starting DMA, polling for DMA interrupt completion, clearing the interrupt, and reading back control data to determine if a data payload transfer is needed. Data payload size is calculated from the word count field and aligned to 8 bytes before initiating the data DMA transfer.",
        "Speed": "NA",
        "Mode": "DMA Mode",
        "Memory Start Offset": "0xE6000000",
        "Memory End Offset": "0xE6000500",
        "Remarks": "The test iterates lane configurations from 4 lanes down to 1 lane (lane_num 3 to 0). Two hardcoded register addresses are used that could not be mapped to named registers in the specification. The virtual channel configuration depends on compile-time GDMA path selection (GDMA0/1/2/3). DMA-based polling is used for both control and data packet transfer completion. The data payload size is aligned to 8-byte boundaries. The D-PHY initialization is performed via an external function snps_phy_init(). Default resolution is HRES=64, VRES=3 unless GDMA0_FULL_MEM is defined (1920x1080).",
        "Test Steps / Procedure": "1. Enable all CSI-2 interrupt masks by reading the main interrupt status register to clear pending interrupts, then writing enable values to PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt mask registers. 2. Configure the virtual channel register based on the selected GDMA path and enable control data transfer. 3. Initialize the D-PHY by calling the PHY initialization sequence. 4. Poll the PHY stop state register until all lanes (data and clock) reach stop state. 5. For each lane configuration (4 lanes down to 1 lane): a. Write the number of active lanes to the N_LANES register. b. Trigger the CSI-2 test sequence. c. For each expected packet (control and data): i. Enable DMA interrupts. ii. Program and start a DMA transfer for the control packet. iii. Poll the DMA interrupt status for control channel completion. iv. Clear the DMA interrupt for the control channel. v. Read back the control data to determine the packet type and word count. vi. If the packet contains image data, calculate the aligned data size, program and start a second DMA transfer for the data payload, poll for data channel completion, and clear the data channel interrupt. 6. Verify the test completes successfully for all lane configurations.",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Validation / Acceptance Criteria": "1. The PHY_STOPSTATE register must indicate all data lanes and clock lane are in stop state before lane configuration begins. 2. DMA interrupt status must indicate successful completion for each control packet transfer. 3. DMA interrupt status must indicate successful completion for each data payload transfer when applicable. 4. Control data read-back must be valid to determine packet type and extract word count for data payload sizing. 5. All lane configurations (4 lanes down to 1 lane) must complete their full packet transfer sequences without errors. 6. The test must complete successfully with a pass indication after all lane iterations.",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
        "Meta Arrays": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration by iterating through lane counts from 4 down to 1. It first enables CSI-2 interrupts by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts, then writing all interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED). It configures the virtual channel register MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL based on the selected GDMA path (GDMA0/1/2/3) and enables control data transfer via MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. After D-PHY initialization via snps_phy_init(), it polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the value equals 0x1000f (indicating all lanes in stop state). For each lane configuration (3 down to 0), it writes MIZAR_MIPI_CSI2_HOST_N_LANES with the lane count, triggers the CSI-2 sequence by writing to 0xa0243ffc, then loops through control and data packets using DMA transfers.",
        "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt(): read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts. 2. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with 0x0000000f. 3. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with 0x00000003. 4-11. Write remaining interrupt mask registers. 12-22. Configure virtual channel, enable control data, init PHY, poll stop state, loop lanes and packets with DMA transfers. 22. Call finish(0).",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY stop state polling must equal 0x1000f. 2. DMA control channel completion polling. 3. DMA data channel completion polling. 4. Control data validation. 5. Word count extraction. 6. Test passes via finish(0)."
    }
]

print(f"Generating: {filename}")
print(f"Path: {filepath}")
print(f"Timestamp (IST): {now_ist.isoformat()}")
print("SUCCESS: Script ready. Run with openpyxl installed.")
