#!/usr/bin/env python3
"""MIPI_CSI TestPlan Excel Generator - Agent 7"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment

# ============================================================
# INPUT DATA
# ============================================================
json_data = [
    {"Index":"1","SS / Module":"MIPI_CSI","Test Case Name":"mipi_csi2_dphy_lanes_test","Feature":"D-PHY Lane Configuration and CSI-2 Data Reception","Meta Headers":"<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"","Meta Macros":"#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined), else 3\n#define HRES 1920 (if GDMA0_FULL_MEM defined), else 64\n#define DATA_TYPE CSI2_RGB888","Meta Arrays":"NA","Speed":"NA","Mode":"NA","Memory Start Offset":"0xE6000000","Memory End Offset":"0xE6040080","Meta Test Description":"This testcase validates MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 lanes down to 1 lane). For each lane configuration, it programs the CSI-2 host controller with the number of active lanes, triggers a CSI-2 sequence, and then performs repeated DMA transfers to receive both control packets and data packets from the CSI-2 interface.\n\nThe test begins by calling csi2_enable_interrupt() which reads INT_ST_MAIN to clear pending interrupts, then enables all CSI-2 interrupt masks (PHY_FATAL, PKT_FATAL, PHY, LINE, BNDRY_FRAME_FATAL, SEQ_FRAME_FATAL, CRC_FRAME_FATAL, PLD_CRC_FATAL, DATA_ID, ECC_CORRECTED). It then configures the virtual channel register and enables control data transfer. The D-PHY is initialized via snps_phy_init() and the test polls PHY_STOPSTATE until the PHY enters stop state (value 0x1000f).\n\nFor each lane count (3 down to 0), the test configures N_LANES, triggers the CSI-2 sequence by writing (lane_num+1) to address 0xa0243ffc, then enters an inner loop iterating (VRES*3)+2 times. Each inner iteration performs a DMA control-packet transfer on channel 0 (8 bytes from src 0x8000 to dest 0xE6001000), polls DMA interrupt status until bit 0 is set, clears the DMA interrupt, reads the control data from 0xE6001000, and if the data type field (bits[5:0]) is greater than 0xf, calculates the word count and performs a DMA data transfer on channel 1 (variable size from src 0x0000 to dest 0xE6002000), polls DMA interrupt status until bit 1 is set, reads interrupt status again, and clears the DMA interrupt. The test completes by calling finish(0).","Test Description":"Verify MIPI CSI-2 D-PHY lane configuration by iterating through all lane counts (4 to 1 lane), enabling CSI-2 interrupts, initializing the D-PHY, waiting for PHY stop state, and performing DMA-based control and data packet transfers for each lane configuration. Validate DMA completion via interrupt polling and verify control packet reception for each lane count.","Meta Test Steps / Procedure":"1. Entry: test_case() is called.\n2. Declare local variables: rx_desc (long long int), tx_desc (long long int), gdma_tx_trnsfr_size (long long int), gdma_trnsfr_size (long long int), cntrl_pkt_cnt (int).\n3. printf(\"start line\\n\").\n4. Call csi2_enable_interrupt().\n5. [Inside csi2_enable_interrupt()]: Declare local int rd_data.\n6. [Inside csi2_enable_interrupt()]: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 Read INT_ST_MAIN register to clear interrupts.\n7. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 Enable phy_fatal interrupts.\n8. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 Enable pkt_fatal interrupts.\n9. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 Enable phy interrupts.\n10. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 Enable line interrupts.\n11. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 Enable boundary frame fatal interrupts.\n12. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 Enable seq frame fatal interrupts.\n13. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 Enable CRC frame fatal interrupts.\n14. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 Enable payload CRC fatal interrupts.\n15. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 Enable data ID interrupts.\n16. [Inside csi2_enable_interrupt()]: write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 Enable ECC corrected interrupts.\n17. [Return from csi2_enable_interrupt()].\n18. Conditional compilation: set vcid_csi2_wrap_reg and gdma_path based on GDMA path defines.\n19. gdma_reg_base = 0xE6A00000.\n20. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n21. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1).\n22. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n23. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) (second).\n24. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) (second).\n25. Call snps_phy_init().\n26. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE).\n27. Enter while loop: condition !(rd_data == 0x1000f).\n28. [Inside while loop]: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE).\n29. Exit while loop when rd_data == 0x1000f.\n30. ch0_pc = 0xE6000000.\n31. ch1_pc = 0xE6000500.\n32. Enter outer for loop: lane_num = 3; lane_num >= 0; lane_num--.\n33. write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num).\n34. cntrl_pkt_cnt = ((VRES * 3) + 2).\n35. printf(\"DEBUG: cntrl_pkt_cnt=%d\\n\", cntrl_pkt_cnt).\n36. write_reg(0xa0243ffc, (lane_num + 1)).\n37-69. Inner loop with DMA transfers, polling, and data packet handling.\n69. Call finish(0).","Test Steps / Procedure":"1. Call the CSI-2 interrupt enable routine to clear pending interrupts by reading the main interrupt status register, then enable all CSI-2 interrupt masks including PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected interrupts.\n2. Configure the CSI-2 virtual channel register with the selected virtual channel ID based on the active GDMA path.\n3. Enable control data transfer by writing to the control data register.\n4. Write the virtual channel register and control data register a second time to confirm configuration.\n5. Initialize the D-PHY by calling the SNPS PHY initialization routine.\n6. Poll the PHY stop state register until the PHY enters stop state (expected value indicating all lanes and clock in stop state).\n7. Set DMA channel 0 program counter to the designated memory location and DMA channel 1 program counter to a separate designated memory location.\n8. For each lane count from 4 lanes down to 1 lane, configure the number of active lanes in the CSI-2 host controller.\n9. Trigger the CSI-2 sequence by writing the lane count to the sequence trigger register.\n10. For each control packet in the frame (total count based on vertical resolution), enable DMA interrupts for both channels.\n11. Program and start a DMA transfer on channel 0 to receive the 8-byte control packet.\n12. Poll the DMA interrupt status register until the channel 0 transfer completion interrupt is asserted.\n13. Clear the channel 0 DMA interrupt and read the DMA status register.\n14. Read the received control data from the DMA destination address.\n15. If the data type field in the control data indicates a long packet (data type > 0xF), extract the word count, calculate the 8-byte aligned transfer size, program and start a DMA transfer on channel 1 for the data payload.\n16. Poll the DMA interrupt status register until the channel 1 transfer completion interrupt is asserted.\n17. Read the DMA interrupt status register again after channel 1 completion, then clear the channel 1 DMA interrupt.\n18. Repeat steps 10-17 for all packets in the current lane configuration.\n19. Repeat steps 8-18 for all lane counts (4, 3, 2, 1).\n20. Signal test completion with pass status.","Meta Impacted Registers":"MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED","Impacted Registers":"virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED","Meta Validation / Acceptance Criteria":"1. PHY Stop State Polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled in a while loop until rd_data == 0x1000f.\n2. DMA Channel 0 Completion Polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled until bit 0 is set.\n3. Control Data Type Check: (csi_ctrl_data & 0x3f) > 0xf evaluated.\n4. Word Count Extraction: word_count = ((csi_ctrl_data >> 6) & 0xffff).\n5. Data Size Alignment: csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count.\n6. DMA Channel 1 Completion Polling: polled until bit 1 is set.\n7. Post-Channel-1 Interrupt Status Read.\n8. DMA Interrupt Clear Channel 0: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1).\n9. DMA Interrupt Clear Channel 1: write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2).\n10. Test Completion: finish(0).","Validation / Acceptance Criteria":"1. The PHY stop state register must read the expected value indicating all data lanes and clock lane are in stop state before proceeding with lane configuration.\n2. DMA channel 0 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 0 completion bit is asserted.\n3. The received control data must be read and its data type field evaluated to determine whether a data payload transfer is required.\n4. When a long packet is detected, the word count must be correctly extracted and the transfer size must be 8-byte aligned before initiating the DMA data transfer on channel 1.\n5. DMA channel 1 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 1 completion bit is asserted.\n6. DMA interrupts must be properly cleared after each channel transfer completion.\n7. The test must iterate through all four lane configurations (4, 3, 2, 1 lanes) and process all packets for each configuration.\n8. The test must complete successfully by calling the finish routine with a pass indication.","Remarks":"The testcase uses external functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() whose implementations are not available in the testcase folder. The VRES and HRES macros have conditional definitions depending on GDMA0_FULL_MEM. The GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) is compile-time conditional. Two hex register tokens (0xa0243ffc and 0xE6001000) could not be mapped to named registers in the specification documents."},
    {"Index":"2","SS / Module":"MIPI_CSI","Test Case Name":"mipi_csi2_test_pattern_generator","Feature":"Test Pattern Generator","Meta Headers":"<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mipi_csi2.h\"","Meta Macros":"#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000","Meta Arrays":"NA","Speed":"NA","Mode":"NA","Memory Start Offset":"0xE6000000","Memory End Offset":"0xE6A00000","Meta Test Description":"This testcase validates the MIPI CSI-2 internal test pattern generator (PPI PG) functionality. The test configures the CSI-2 subsystem virtual channel register, disables control data transfer, enables CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt(), initializes the D-PHY via snps_phy_init(), and polls PHY_STOPSTATE until the PHY enters stop state (expected value 0x1000f). It then sets up DMA address mapping registers for channel 0 data and instruction paths (AR and AW), enables the fracdiv output to the CSI-2 subsystem by writing 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, and programs a DMA transfer using dma_trnsfr_instn_preload_incr_addr() with calculated transfer size based on hres=320, vres=16, valid_bits_per_pixel=24. The DMA is started via DMAGO_CSI() on channel 0. The test then enables the pattern generator by calling csi2_ctrlr_pg_enable() which configures PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, and PPI_PG_ENABLE=1. After a wait_on(100) delay, the pattern generator is disabled by writing 0 to PPI_PG_ENABLE. The test then polls the DMA interrupt status register (gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) until bit 0 is set indicating DMA channel 0 transfer completion. A final wait_on(10000) delay is applied before calling finish(0) to signal test pass.\n\nA local function csi2_enable_interrupt() is also defined which reads INT_ST_MAIN to clear pending interrupts and then enables all CSI-2 interrupt masks: INT_MSK_PHY_FATAL=0x0000000f, INT_MSK_PKT_FATAL=0x00000003, INT_MSK_PHY=0x000f000f, INT_MSK_LINE=0x000f000f, INT_MSK_BNDRY_FRAME_FATAL=0x0000ffff, INT_MSK_SEQ_FRAME_FATAL=0x0000ffff, INT_MSK_CRC_FRAME_FATAL=0x0000ffff, INT_MSK_PLD_CRC_FATAL=0x0000ffff, INT_MSK_DATA_ID=0x0000ffff, INT_MSK_ECC_CORRECTED=0x0000ffff. This function may be called internally by csi2_subsys_enable_interrupt().","Test Description":"Verify the MIPI CSI-2 internal test pattern generator by configuring the virtual channel, enabling CSI-2 interrupts, initializing the D-PHY, waiting for PHY stop state, programming DMA address registers and transfer parameters, starting a DMA transfer, enabling the pattern generator with specified vertical resolution, horizontal resolution, and configuration, then disabling the pattern generator and polling for DMA transfer completion to confirm successful data reception.","Meta Test Steps / Procedure":"1. Entry: test_case() is called.\n2. Declare local variables.\n3. int_pend = 1.\n4. printf(\"start line\\n\").\n5. vcid = 3.\n6. vcid_unselected_path = ((vcid + 1) & 0xf) = 4.\n7. Conditional compilation for GDMA path selection.\n8. gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000).\n9. printf(\"vcid_csi2_wrap_reg=%0x\\n\", vcid_csi2_wrap_reg).\n10. write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg).\n11. write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0).\n12. Call csi2_subsys_enable_interrupt().\n13. Call snps_phy_init().\n14-17. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until 0x1000f.\n18-21. Set hres=320, vres=16, valid_bits_per_pixel=24, calculate csi2_data_trnsfr_size=15360.\n22-25. Write DMA address registers.\n26. write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1).\n27-31. Program and start DMA transfer on channel 0.\n32-37. Call csi2_ctrlr_pg_enable() to configure and enable PG.\n38. Call wait_on(100).\n39. write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0).\n40-44. Poll DMA interrupt status until bit 0 set.\n45. Call wait_on(10000).\n46. Call finish(0).\n47-58. Local csi2_enable_interrupt() definition with interrupt mask writes.","Test Steps / Procedure":"1. Set the interrupt pending flag and initialize the virtual channel ID to 3, computing the unselected path value.\n2. Configure the virtual channel register based on the selected GDMA path and compute the GDMA register base address.\n3. Write the virtual channel register with the computed virtual channel mapping.\n4. Disable control data transfer by writing 0 to the control data register.\n5. Enable CSI-2 and DMA interrupts by calling the subsystem interrupt enable routine.\n6. Initialize the D-PHY by calling the SNPS PHY initialization routine.\n7. Poll the PHY stop state register until the PHY enters stop state.\n8. Set horizontal resolution to 320, vertical resolution to 16, and valid bits per pixel to 24, then calculate the 8-byte aligned total DMA transfer size.\n9. Program the higher-order DMA address bits for channel 0 AR data, AR instruction, AW data, and AW instruction paths.\n10. Enable the fracdiv output to the CSI-2 subsystem by writing to the clock gating control register.\n11. Set the DMA channel 0 program counter and configure the destination address increment flag based on frame rate mode.\n12. Program the DMA transfer instructions with preload and incremental address support.\n13. Start the DMA transfer on channel 0.\n14. Enable the test pattern generator by configuring vertical resolution, horizontal resolution, pattern configuration, and enabling the PG.\n15. Wait for 100 time units to allow pattern generation.\n16. Disable the test pattern generator.\n17. Poll the DMA interrupt status register until the channel 0 transfer completion interrupt bit is asserted.\n18. Wait for 10000 time units for completion settling.\n19. Signal test completion with pass status.","Meta Impacted Registers":"MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED","Impacted Registers":"PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED","Meta Validation / Acceptance Criteria":"1. PHY Stop State Polling: polled until rd_data == 0x1000f.\n2. DMA Channel 0 Completion Polling: polled until bit 0 is set.\n3. Transfer Size Calculation Validation: expected 15360 bytes.\n4. Pattern Generator Enable/Disable Sequence.\n5. Pattern Generator Configuration Values.\n6. DMA Address Register Values.\n7. Fracdiv Enable.\n8. Control Data Disabled.\n9. Interrupt Mask Values.\n10. Test Completion: finish(0).","Validation / Acceptance Criteria":"1. The PHY stop state register must read the expected value indicating all data lanes and clock lane are in stop state before proceeding with DMA and pattern generator configuration.\n2. The DMA transfer size must be correctly calculated as an 8-byte aligned value based on the configured horizontal resolution, vertical resolution, and bits per pixel.\n3. The test pattern generator must be enabled with the specified vertical resolution, horizontal resolution, and configuration values, then disabled after the wait period.\n4. DMA channel 0 transfer completion must be confirmed by polling the DMA interrupt status register until the channel 0 completion bit is asserted.\n5. All CSI-2 interrupt masks must be properly enabled before starting the test sequence.\n6. The test must complete successfully by calling the finish routine with a pass indication.","Remarks":"The testcase calls csi2_subsys_enable_interrupt() from test_case() but defines csi2_enable_interrupt() locally. The local csi2_enable_interrupt() function may be called internally by the external csi2_subsys_enable_interrupt(). External functions snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are not available in the testcase folder. The GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) and FPS60 are compile-time conditional defines. The MIZAR_MIPI_CSI2_RB_REG_BASE token used with offset 0xf4 could not be mapped to a named register by Agent 4 (status: unresolved). Two locally defined macros GDMA_CSI2_DATA_DEST_ADDR2 and GDMA_CTRL_DATA_DEST_ADDR2 are defined but not used in the testcase execution flow."}
]

IP_NAME = "MIPI_CSI"

# ============================================================
# COLUMN DEFINITIONS
# ============================================================
TESTPLAN_COLUMNS = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

METADATA_COLUMNS = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# ============================================================
# GENERATE FILENAME WITH IST TIMESTAMP
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"{IP_NAME}_TestPlan_{timestamp_str}.xlsx"
output_dir = os.environ.get("OUTPUT_DIR", ".")
output_path = os.path.join(output_dir, filename)

print(f"Generating: {filename}")
print(f"Output path: {output_path}")
print(f"IST timestamp: {now_ist.isoformat()}")

# ============================================================
# CREATE WORKBOOK
# ============================================================
wb = Workbook()

# -- TestPlan Sheet --
ws_tp = wb.active
ws_tp.title = "TestPlan"

# -- MetaData Sheet --
ws_md = wb.create_sheet(title="MetaData")

# ============================================================
# FORMATTING STYLES
# ============================================================
header_font = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
wrap_alignment = Alignment(wrap_text=True, vertical="top")

def write_header(ws, columns):
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

def write_data_rows(ws, columns, data):
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            value = row_data.get(col_name, "")
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

def auto_size_columns(ws, columns, max_width=80):
    for col_idx, col_name in enumerate(columns, 1):
        max_len = len(col_name)
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line))
        adjusted_width = min(max_len + 4, max_width)
        col_letter = ws.cell(row=1, column=col_idx).column_letter
        ws.column_dimensions[col_letter].width = adjusted_width

# ============================================================
# POPULATE TESTPLAN SHEET
# ============================================================
write_header(ws_tp, TESTPLAN_COLUMNS)
write_data_rows(ws_tp, TESTPLAN_COLUMNS, json_data)
auto_size_columns(ws_tp, TESTPLAN_COLUMNS)
ws_tp.freeze_panes = "A2"

# ============================================================
# POPULATE METADATA SHEET
# ============================================================
write_header(ws_md, METADATA_COLUMNS)
write_data_rows(ws_md, METADATA_COLUMNS, json_data)
auto_size_columns(ws_md, METADATA_COLUMNS)
ws_md.freeze_panes = "A2"

# Set MetaData sheet to veryHidden
ws_md.sheet_state = "veryHidden"

# ============================================================
# SAVE WORKBOOK
# ============================================================
os.makedirs(output_dir, exist_ok=True)
wb.save(output_path)
print(f"Workbook saved: {output_path}")

# ============================================================
# VALIDATE
# ============================================================
if not os.path.exists(output_path):
    print("VALIDATION FAILED: File does not exist")
    sys.exit(1)

file_size = os.path.getsize(output_path)
if file_size == 0:
    print("VALIDATION FAILED: File size is 0")
    sys.exit(1)

try:
    wb_check = load_workbook(output_path)
    sheets = wb_check.sheetnames
    assert "TestPlan" in sheets, "TestPlan sheet missing"
    assert "MetaData" in sheets, "MetaData sheet missing"
    tp_rows = wb_check["TestPlan"].max_row - 1  # exclude header
    md_rows = wb_check["MetaData"].max_row - 1
    print(f"VALIDATION PASSED")
    print(f"File size: {file_size} bytes")
    print(f"Sheets: {sheets}")
    print(f"TestPlan rows: {tp_rows}")
    print(f"MetaData rows: {md_rows}")
    print(f"MetaData visibility: {wb_check['MetaData'].sheet_state}")
    print(f"FILENAME={filename}")
except Exception as e:
    print(f"VALIDATION FAILED: {e}")
    sys.exit(1)
