#!/usr/bin/env python3
"""Generate MIPI TestPlan Excel workbook using openpyxl."""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'MIPI_TestPlan_{timestamp}.xlsx'

# Input data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2; LS_LE_EN; TOTAL_FRAME; VC_ID; VRES; HRES; DATA_TYPE",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration by iterating through lane counts from 4 down to 1. It first enables all CSI-2 host interrupts by reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts, then writing mask registers (MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED). It then configures the virtual channel register (MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL) based on the selected GDMA path and enables control data transfer via MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA. The D-PHY is initialized via snps_phy_init(), and MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE is polled until it reads 0x1000f indicating all lanes are in stop state. A loop from lane_num=3 down to 0 writes MIZAR_MIPI_CSI2_HOST_N_LANES with the current lane count, triggers a CSI-2 sequence by writing 0xa0243ffc with (lane_num+1), then performs DMA transfers for control and data packets. For each packet, DMA interrupt status is polled via gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET, interrupts are cleared via gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, and control data is read from 0xE6001000. If the data type field indicates a long packet (data_type > 0xf), a data DMA transfer is also performed with word_count extracted from the control data. The test completes by calling finish(0).",
        "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). It enables all CSI-2 host interrupt masks, configures the virtual channel and control data registers, initializes the D-PHY, and polls the PHY stop state register until all lanes reach stop state. For each lane configuration, the N_LANES register is updated, a CSI-2 sequence is triggered, and DMA transfers are performed for both control and data packets. DMA completion is verified by polling the DMA interrupt status. The test validates that the CSI-2 subsystem operates correctly across all lane configurations.",
        "Meta Test Steps / Procedure": "1. Call csi2_enable_interrupt(): read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to clear pending interrupts. 2. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL with 0x0000000f. 3. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL with 0x00000003. 4. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY with 0x000f000f. 5. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE with 0x000f000f. 6. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff. 7. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff. 8. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL with 0x0000ffff. 9. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL with 0x0000ffff. 10. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID with 0x0000ffff. 11. Write MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED with 0x0000ffff. 12. Determine vcid_csi2_wrap_reg based on GDMA path. 13. Set gdma_reg_base = 0xE6A00000. 14. Write MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg. 15. Write MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 1. 16-17. Repeat writes. 18. Call snps_phy_init(). 19. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until 0x1000f. 20-33. Lane loop with DMA transfers.",
        "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing appropriate mask values to the PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupt mask registers. 2. Configure the virtual channel register based on the selected GDMA path and enable control data transfer. 3. Initialize the D-PHY. 4. Poll the PHY stop state register until all lanes have entered stop state. 5. For each lane configuration (4 lanes down to 1 lane): configure the N_LANES register with the current lane count. 6. Trigger the CSI-2 sequence for the current lane configuration. 7. For each expected packet: enable DMA interrupts, program and start a DMA transfer for control data on channel 0. 8. Poll the DMA interrupt status register until the control data transfer completes, then clear the DMA interrupt. 9. Read the control data to determine the packet type and word count. 10. If the packet is a long packet, program and start a DMA transfer for pixel data on channel 1, poll for completion, and clear the DMA interrupt. 11. Repeat for all packets and all lane configurations. 12. Verify the test completes successfully.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE must read 0x1000f before proceeding. 2. DMA interrupt status register bit 0 must be set after channel 0 transfer. 3. DMA interrupt status register bit 1 must be set after channel 1 transfer. 4. csi_ctrl_data must contain valid data type and word count. 5. All 4 lane configurations processed. 6. finish(0) called.",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must indicate all lanes are in stop state before lane configuration begins. 2. DMA interrupt status must confirm successful completion of each control data transfer on channel 0. 3. For long packets, DMA interrupt status must confirm successful completion of each pixel data transfer on channel 1. 4. Control data read must contain valid data type and word count fields for proper packet classification. 5. All four lane configurations (4 lanes through 1 lane) must complete their respective packet transfers without errors. 6. The test must complete successfully for all lane configurations.",
        "Remarks": "The test iterates lane configurations from 4 lanes down to 1 lane in a descending loop. The GDMA path selection is compile-time configurable via preprocessor defines (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH). The virtual channel ID shift depends on the selected GDMA path. DMA transfer sizes for data packets are 8-byte aligned. Two unresolved hardcoded addresses are used: one to trigger the CSI-2 sequence and one as a DMA destination for control data readback. The D-PHY initialization is performed by an external function snps_phy_init(). Polling loops have no explicit timeout."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_csi2_rb_reg_wr_rd_test",
        "Feature": "Register Write-Read Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_define.c"; "test_common.h"; <mipi_csi2_rb_reg.h>',
        "Meta Macros": "SOFT_RST_REG_ADDRESS; SOFT_RST_REG_DATA; CNT",
        "Meta Arrays": "addr_array[60]; default_value_array[60]; read_mask_array[60]; write_mask_array[60]; skip_array[60]; chk_val[6]",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the default reset values and write-read functionality of 10 MIPI CSI-2 RB REG registers. The test has two phases: (1) chk_rst_val() reads each register and compares against default_value_array. (2) chk_rd_wr() writes 6 test patterns and reads back to verify.",
        "Test Description": "This test validates the default reset values and write-read functionality of 10 MIPI CSI-2 RB REG block registers. In the first phase, each register is read and its value is compared against the expected default reset value. In the second phase, six distinct test patterns are written to each writable register and read back. The read-back value is validated against an expected value computed using the register's read mask, write mask, and default value. Registers are selectively skipped based on their read/write accessibility masks and a skip array. The test passes only if all default value checks and all write-read verifications succeed across all registers and all test patterns.",
        "Meta Test Steps / Procedure": "1. test_case() calls chk_rst_val(). 2-6. Loop through registers reading and comparing defaults. 7. test_case() calls chk_rd_wr(). 8-21. Write patterns, read back, compute expected values. 22. Check fail counts. 23. soft_reset_chk() commented out.",
        "Test Steps / Procedure": "1. Read each register in the RB REG block and verify that the read value matches its expected default reset value, skipping non-readable registers. 2. For each of six test data patterns: write the pattern to each writable register. 3. Read back each written register and compute the expected value. 4. Compare the read-back value against the computed expected value. 5. Track mismatch counts. 6. If any mismatch is detected, the test fails; otherwise the test passes.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_LINE_INFO; MIZAR_MIPI_CSI2_RB_REG_FIFO_THRESHOLD_VAL; MIZAR_MIPI_CSI2_RB_REG_LANE_CLK; MIZAR_MIPI_CSI2_RB_REG_MEM; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_RAW; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_MASK; MIZAR_MIPI_CSI2_RB_REG_INTERRUPT_ENABLE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_FLUSH",
        "Impacted Registers": "virtual_channel; line_info; fifo_threshold_val; lane_clk; mem; interrupt_raw; interrupt_mask; interrupt_enable; control_data; flush",
        "Meta Validation / Acceptance Criteria": "1. Each readable register must match default_value_array. 2. Each writable register must read back expected value after write. 3. def_fail_cnt == 0 AND wr_fail_cnt == 0 for pass.",
        "Validation / Acceptance Criteria": "1. All readable registers must return their expected default reset values after reset. 2. For each of six test data patterns, all writable registers must read back the expected value. 3. The test passes only if zero mismatches detected. 4. Any single mismatch causes the test to fail.",
        "Remarks": "The addr_array is declared with size 60 but only 10 register entries are populated; CNT is defined as 60 so the loop iterates over uninitialized entries as well. The skip_array marks index 5 (interrupt_raw), index 9 (flush), and index 10 as skipped for write-read testing. The soft_reset_chk() function is present in source but commented out in test_case() and is not executed. Six distinct bit patterns are used to exercise all writable bit positions across the registers."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Test Pattern Generator",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality. It configures the virtual channel, disables control data, enables interrupts, initializes D-PHY, configures DMA addresses, enables fractional divider, programs DMA transfer for 320x16 frame, enables/disables pattern generator, and polls DMA completion.",
        "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator. It configures the virtual channel register and disables control data transfer. All CSI-2 host interrupt masks are enabled. The D-PHY is initialized and the PHY stop state register is polled until all lanes reach stop state. DMA address registers for channel 0 are configured for both read and write paths. The fractional divider output is enabled. A DMA transfer is programmed for a 320x16 pixel frame with 24 bits per pixel. The pattern generator is then enabled with specific vertical resolution, horizontal resolution, and configuration values. After a wait period, the pattern generator is disabled. The DMA interrupt status is polled until the transfer completes. The test verifies that the CSI-2 subsystem correctly receives and transfers data generated by the internal test pattern generator.",
        "Meta Test Steps / Procedure": "1-6. Configure vcid and virtual channel register. 7-17. Enable interrupts. 18-19. Init D-PHY and poll stop state. 20-26. Configure DMA addresses and fractional divider. 27-28. Program and start DMA. 29-32. Enable pattern generator. 33-34. Wait and disable. 35-37. Poll DMA and finish.",
        "Test Steps / Procedure": "1. Configure the virtual channel register and disable control data transfer. 2. Enable all CSI-2 host interrupts. 3. Initialize the D-PHY. 4. Poll the PHY stop state register. 5. Configure DMA channel 0 address registers. 6. Enable fractional divider output. 7. Program and start DMA transfer. 8. Enable test pattern generator. 9. Wait then disable pattern generator. 10. Poll DMA interrupt status. 11. Verify test completes successfully.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE must read 0x1000f. 2. DMA interrupt bit 0 must be set after transfer. 3. Pattern generator configured correctly. 4. finish(0) called.",
        "Validation / Acceptance Criteria": "1. The PHY stop state register must indicate all lanes are in stop state. 2. The DMA interrupt status must confirm successful completion. 3. The pattern generator must be correctly enabled and disabled. 4. The test must complete successfully.",
        "Remarks": "The test pattern generator is an internal CSI-2 host feature that generates test data without requiring an external DPHY transmitter. The pattern generator is enabled and then disabled after a short wait period. The GDMA path selection is compile-time configurable. The fractional divider output is enabled via a base+offset register write. Polling loops have no explicit timeout. The control data transfer is explicitly disabled for this test."
    },
    {
        "Index": "4",
        "SS / Module": "MIPI",
        "Test Case Name": "mipi_dphy_idi_test",
        "Feature": "DPHY IDI Data Transfer",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mipi_csi2.h"',
        "Meta Macros": "GDMA_CSI2_DATA_DEST_ADDR2; GDMA_CTRL_DATA_DEST_ADDR2",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY IDI data transfer across all 16 virtual channel IDs (vcid 0 to 15). D-PHY init, interrupt enable, PHY stop state poll, control data and null/blank enable, then loop through all 16 VCIDs with 129 packets each.",
        "Test Description": "This test validates MIPI CSI-2 DPHY IDI data transfer across all 16 virtual channel IDs (0 through 15). It initializes the D-PHY, enables all CSI-2 host interrupt masks, and polls the PHY stop state register until all lanes reach stop state. Control data transfer and null/blanking data transfer are both enabled. For each virtual channel ID, the virtual channel register is configured and a packet transfer sequence is triggered. For each of 129 control packets per virtual channel, a DMA transfer is performed on channel 0 to receive control data. The control data is read to determine the packet type. If the packet is a long packet, a second DMA transfer is performed on channel 1 to receive the pixel data with 8-byte aligned transfer size. DMA completion is verified by polling the DMA interrupt status for each transfer. The test validates that the CSI-2 subsystem correctly handles IDI data transfer across all virtual channels.",
        "Meta Test Steps / Procedure": "1. Init D-PHY. 2-12. Enable interrupts. 13. Poll PHY stop state. 14-15. Enable control data and null/blank. 16-31. Loop VCIDs 0-15 with 129 packets each, DMA transfers. 32. finish(0).",
        "Test Steps / Procedure": "1. Initialize the D-PHY. 2. Enable all CSI-2 host interrupts. 3. Poll the PHY stop state register. 4. Enable control data and null/blanking transfers. 5. For each VCID (0-15): configure and trigger. 6. For each packet: DMA transfer on channel 0. 7. Poll DMA completion. 8. Read control data. 9. If long packet, DMA on channel 1. 10. Repeat for all VCIDs. 11. Verify test completes.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PHY_STOPSTATE; control_data; null_blank; virtual_channel; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE must read 0x1000f. 2. DMA bit 0 set after ch0 transfer. 3. DMA bit 1 set after ch1 transfer. 4. Valid control data. 5. All 16 VCIDs with 129 packets. 6. finish(0) called.",
        "Validation / Acceptance Criteria": "1. PHY stop state must indicate all lanes stopped. 2. DMA must confirm each control data transfer. 3. For long packets, DMA must confirm pixel data transfer. 4. Valid data type and word count. 5. All 16 VCIDs complete. 6. Test completes successfully.",
        "Remarks": "The test iterates through all 16 virtual channel IDs (0 to 15) with 129 control packets per virtual channel, resulting in a total of 2064 packet iterations. Both control data transfer and null/blanking data transfer are enabled simultaneously. The virtual channel ID is shifted left by 12 bits before writing to the virtual channel register. DMA transfer sizes for data packets are 8-byte aligned. Two unresolved hardcoded addresses are used. Polling loops have no explicit timeout."
    }
]

# TestPlan columns
tp_cols = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData columns
md_cols = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

# Create workbook
wb = Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

# Header styling
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')

# Write TestPlan headers
for col_idx, col_name in enumerate(tp_cols, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(tp_cols, 1):
        value = row_data.get(col_name, '')
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_align

# Write MetaData headers
for col_idx, col_name in enumerate(md_cols, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(md_cols, 1):
        value = row_data.get(col_name, '')
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_align

# Auto-size columns
def auto_size(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(str(col_name))
        for row in range(2, ws.max_row + 1):
            val = ws.cell(row=row, column=col_idx).value
            if val:
                max_len = max(max_len, min(len(str(val)), 80))
        ws.column_dimensions[ws.cell(row=1, column=col_idx).column_letter].width = min(max_len + 4, 60)

auto_size(ws_tp, tp_cols)
auto_size(ws_md, md_cols)

# Freeze first row
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
output_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(output_dir, filename)
wb.save(output_path)

# Verify
assert os.path.exists(output_path), f'File not found: {output_path}'
assert os.path.getsize(output_path) > 0, 'File is empty'
wb2 = load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
assert wb2['TestPlan'].max_row == 5, f'Expected 5 rows, got {wb2["TestPlan"].max_row}'
assert wb2['MetaData'].max_row == 5, f'Expected 5 rows, got {wb2["MetaData"].max_row}'

print(f'SUCCESS: Generated {filename}')
print(f'Path: {output_path}')
print(f'Size: {os.path.getsize(output_path)} bytes')
print(f'TestPlan rows: {wb2["TestPlan"].max_row - 1}')
print(f'MetaData rows: {wb2["MetaData"].max_row - 1}')
print(f'Sheets: {wb2.sheetnames}')
