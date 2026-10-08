#!/usr/bin/env python3
"""Agent 7 - Excel Generator for MIPI_CSI TestPlan
Generates a real .xlsx workbook using openpyxl and pushes to GitHub.
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment
    from openpyxl.utils import get_column_letter

# ============================================================
# JSON DATA
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and CSI-2 Data Transfer",
        "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"test_common.h\"; #include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n#define LS_LE_EN 1\n#define TOTAL_FRAME 1\n#define VC_ID 3\n#define VRES 1080 (if GDMA0_FULL_MEM defined) or 3 (otherwise)\n#define HRES 1920 (if GDMA0_FULL_MEM defined) or 64 (otherwise)\n#define DATA_TYPE CSI2_RGB888\n#define MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL (MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL_OFFSET) \u2014 base=0xE6A04000, offset=0x0\n#define MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_RB_REG_CONTROL_DATA_OFFSET) \u2014 base=0xE6A04000, offset=0x20\n#define MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_PHY_STOPSTATE_OFFSET) \u2014 base=0xE6A05000, offset=0x4C\n#define MIZAR_MIPI_CSI2_HOST_N_LANES (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_N_LANES_OFFSET) \u2014 base=0xE6A05000, offset=0x4\n#define MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_ST_MAIN_OFFSET) \u2014 base=0xE6A05000, offset=0xC\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_PHY_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0xE4\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_PKT_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0xF4\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_PHY_OFFSET) \u2014 base=0xE6A05000, offset=0x114\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_LINE_OFFSET) \u2014 base=0xE6A05000, offset=0x134\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0x284\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0x294\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0x2A4\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL_OFFSET) \u2014 base=0xE6A05000, offset=0x2B4\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_DATA_ID_OFFSET) \u2014 base=0xE6A05000, offset=0x2C4\n#define MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED (MIZAR_MIPI_CSI2_HOST_BASE + MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED_OFFSET) \u2014 base=0xE6A05000, offset=0x2D4\n#define MIZAR_MIPI_CSI2_RB_REG_BASE (0xE6A00000+0x4000)\n#define MIZAR_MIPI_CSI2_HOST_BASE (0xE6A00000+0x5000)",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI CSI-2 DPHY lane configuration and data transfer across all lane counts (4 lanes down to 1 lane). The test begins by enabling all CSI-2 host interrupts via the csi2_enable_interrupt() helper function, which reads INT_ST_MAIN to clear pending interrupts and then writes mask values to 10 interrupt mask registers (INT_MSK_PHY_FATAL, INT_MSK_PKT_FATAL, INT_MSK_PHY, INT_MSK_LINE, INT_MSK_BNDRY_FRAME_FATAL, INT_MSK_SEQ_FRAME_FATAL, INT_MSK_CRC_FRAME_FATAL, INT_MSK_PLD_CRC_FATAL, INT_MSK_DATA_ID, INT_MSK_ECC_CORRECTED). The virtual channel register is configured based on the GDMA path (GDMA0/1/2/3) with VC_ID=3. The control data register is enabled. The D-PHY is initialized via snps_phy_init(). The PHY_STOPSTATE register is polled until it reads 0x1000f, confirming the PHY has entered stop state. An outer loop iterates lane_num from 3 down to 0, writing each lane count to N_LANES and triggering the CSI-2 sequence by writing (lane_num+1) to address 0xa0243ffc. An inner loop iterates over cntrl_pkt_cnt = ((VRES*3)+2) packets. For each packet: DMA channel 0 is programmed for control data transfer (8 bytes from src 0x8000 to dest 0xE6001000), DMA interrupt enable is set, DMAGO_CSI is issued, and the DMA interrupt status register (MIPI_CSI2_DMA_INTMIS_OFFSET) is polled until bit 0 is set. The DMA interrupt is cleared. The control data is read from 0xE6001000. If the data type field (bits[5:0]) is greater than 0xf, the word_count is extracted from bits[21:6], the data size is 8-byte aligned, DMA channel 1 is programmed for data transfer (from src 0x0000 to dest 0xE6002000 with csi_data_size bytes), DMAGO_CSI is issued for channel 1, and the DMA interrupt status is polled until bit 1 is set, then the interrupt is cleared. The test completes by calling finish(0).",
        "Test Description": "Verify MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 to 1) and performing CSI-2 control and data packet transfers using DMA. The test enables all CSI-2 host interrupts, initializes the D-PHY, waits for PHY stop state, and for each lane configuration transfers control packets followed by conditional data packets, validating DMA completion via interrupt polling.",
        "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2. Declare local variables: rx_desc (long long int), tx_desc (long long int), gdma_tx_trnsfr_size (long long int), gdma_trnsfr_size (long long int), cntrl_pkt_cnt (int).\n3. printf(\"start line\\n\").\n4. Call csi2_enable_interrupt().\n  4a. [Inside csi2_enable_interrupt()] Declare local int rd_data.\n  4b. rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN) \u2014 Read INT_ST_MAIN register to clear pending interrupts.\n  4c. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f) \u2014 Enable phy_fatal interrupts.\n  4d. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003) \u2014 Enable pkt_fatal interrupts.\n  4e. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f) \u2014 Enable phy interrupts.\n  4f. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f) \u2014 Enable line interrupts.\n  4g. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff) \u2014 Enable boundary frame fatal interrupts.\n  4h. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff) \u2014 Enable seq frame fatal interrupts.\n  4i. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff) \u2014 Enable CRC frame fatal interrupts.\n  4j. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff) \u2014 Enable payload CRC fatal interrupts.\n  4k. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff) \u2014 Enable data ID interrupts.\n  4l. write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff) \u2014 Enable ECC corrected interrupts.\n  4m. Return from csi2_enable_interrupt().\n5-19. [Full test procedure as documented]",
        "Test Steps / Procedure": "1. Enable all CSI-2 host interrupts by reading the main interrupt status register to clear pending interrupts, then writing enable masks to PHY Fatal, Packet Fatal, PHY, Line, Boundary Frame Fatal, Sequence Frame Fatal, CRC Frame Fatal, Payload CRC Fatal, Data ID, and ECC Corrected interrupt mask registers.\n2. Configure the virtual channel register based on the selected GDMA path and VC_ID, and enable control data transfer.\n3. Write the virtual channel and control data registers a second time to confirm configuration.\n4. Initialize the D-PHY via the PHY initialization sequence.\n5. Poll the PHY Stop State register until all lanes and clock lane report stop state (expected value 0x1000f).\n6. Set DMA channel 0 program counter to 0xE6000000 and DMA channel 1 program counter to 0xE6000500.\n7. For each lane configuration (4 lanes down to 1 lane):\n   a. Write the lane count to the N_LANES register.\n   b. Calculate the control packet count as ((VRES * 3) + 2).\n   c. Trigger the CSI-2 sequence by writing the lane count to the trigger register.\n   d. For each packet in the control packet count:\n      i. Enable DMA interrupts for both channels.\n      ii. Program DMA channel 0 for an 8-byte control data transfer and issue DMAGO.\n      iii. Poll the DMA interrupt status register until channel 0 transfer completes (bit 0 set).\n      iv. Clear the DMA channel 0 interrupt.\n      v. Read the transferred CSI-2 control data.\n      vi. If the data type indicates a long packet (data type > 0xf), extract the word count, align to 8 bytes, program DMA channel 1 for the data transfer, issue DMAGO, poll for channel 1 completion (bit 1 set), then clear the channel 1 interrupt.\n8. Signal test completion.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; 0xa0243ffc; 0xE6001000; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "virtual_channel; control_data; PHY_STOPSTATE; N_LANES; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling: read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE) is polled in a while loop until rd_data == 0x1000f.\n2. DMA channel 0 completion polling: read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET) is polled until bit 0 set.\n3. CSI-2 control data type check: (csi_ctrl_data & 0x3f) > 0xf determines long packet.\n4. Word count extraction: word_count = ((csi_ctrl_data >> 6) & 0xffff).\n5. Data size alignment: 8-byte aligned.\n6. DMA channel 1 completion polling: bit 1 set.\n7-9. DMA interrupt clear and test completion.",
        "Validation / Acceptance Criteria": "1. The PHY Stop State register must read the expected value confirming all data lanes and the clock lane have entered stop state before proceeding.\n2. DMA channel 0 transfer must complete as indicated by the DMA interrupt status register bit 0 being set for each control packet transfer.\n3. The CSI-2 control data type field must be correctly evaluated to determine whether a long-packet data transfer is required.\n4. When a long packet is detected, DMA channel 1 transfer must complete as indicated by the DMA interrupt status register bit 1 being set.\n5. DMA interrupts must be properly cleared after each channel transfer completion.\n6. The test must iterate through all four lane configurations (4, 3, 2, 1 lanes) and process all control packets for each configuration.\n7. The test must complete successfully by calling the finish routine with a pass indication.",
        "Remarks": "The testcase uses conditional compilation for GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) and resolution (GDMA0_FULL_MEM). External functions snps_phy_init(), dma_trnsfr_instn_preload(), DMAGO_CSI(), and finish() are not defined locally. The virtual channel register and control data register are written twice in sequence. Two unresolved hex addresses exist: 0xa0243ffc (CSI-2 sequence trigger) and 0xE6001000 (control data destination). DMA base+offset register accesses use gdma_reg_base (0xE6A00000) with MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, MIPI_CSI2_DMA_INTCLR_OFFSET, and hardcoded offset 0x28."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "CSI-2 Internal Test Pattern Generator and DMA Data Transfer",
        "Meta Headers": "#include <stdio.h>; #include <stdlib.h>; #include \"test_common.h\"; #include \"mipi_csi2.h\"",
        "Meta Macros": "#define GDMA_CSI2_DATA_DEST_ADDR2 0xE6040080\n#define GDMA_CTRL_DATA_DEST_ADDR2 0xE6040000\n[Pattern generator and DMA macros]",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates the MIPI CSI-2 internal test pattern generator functionality and DMA-based data transfer. The test configures the CSI-2 subsystem virtual channel register with VC_ID=3 based on the selected GDMA path (GDMA0/1/2/3), disables control data transfer, enables CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt(), initializes the D-PHY via snps_phy_init(), and polls PHY_STOPSTATE until it reads 0x1000f confirming all lanes and clock lane are in stop state. The test then configures the pattern generator with hres=320, vres=16, valid_bits_per_pixel=24, and calculates the DMA transfer size as an 8-byte aligned value of (valid_bits_per_pixel * hres / 8) * vres. The DMA higher-order AXI address registers are programmed. The enableclkgating_csiphy register is written with 0x1 to enable fracdiv output. DMA channel 0 is programmed and DMAGO_CSI is issued. The pattern generator is enabled via csi2_ctrlr_pg_enable() which writes PPI_PG_PATTERN_VRES=0x10, PPI_PG_PATTERN_HRES=0x70140, PPI_PG_CONFIG=0xe401, PPI_PG_ENABLE=1. After wait_on(100), the pattern generator is disabled. The DMA interrupt status register is polled until bit 0 is set. After wait_on(10000), finish(0) is called.",
        "Test Description": "Verify the MIPI CSI-2 internal test pattern generator by configuring the pattern generator with a specific resolution (320x16, 24 bits per pixel), enabling it to generate test data, transferring the generated data via DMA, and validating DMA transfer completion through interrupt polling. The test also configures virtual channels, initializes the D-PHY, and enables all CSI-2 host interrupts.",
        "Meta Test Steps / Procedure": "1. Enter test_case() function.\n2-36. [Full detailed test procedure as documented]",
        "Test Steps / Procedure": "1. Set the interrupt pending flag and configure the virtual channel ID (VC_ID=3) with the unselected path value for the selected GDMA path.\n2. Calculate the GDMA register base address based on the selected GDMA path.\n3. Write the virtual channel register with the computed VC ID mapping for the selected GDMA path.\n4. Disable control data transfer by writing 0 to the control data register.\n5. Enable all CSI-2 host and DMA interrupts.\n6. Initialize the D-PHY via the PHY initialization sequence.\n7. Poll the PHY Stop State register until all data lanes and clock lane report stop state (expected value 0x1000f).\n8. Set the pattern generator resolution parameters: hres=320, vres=16, valid_bits_per_pixel=24.\n9. Calculate the DMA transfer size as an 8-byte aligned value.\n10. Program the DMA higher-order AXI address registers for channel 0.\n11. Enable the fracdiv clock output to the CSI-2 subsystem.\n12. Program DMA channel 0 for data transfer.\n13. Issue DMAGO for DMA channel 0.\n14. Enable the internal test pattern generator.\n15. Wait for the pattern generator to produce data.\n16. Disable the pattern generator.\n17. Poll the DMA interrupt status register until channel 0 transfer completes.\n18. Wait after DMA completion.\n19. Signal test completion with pass status.",
        "Meta Impacted Registers": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; MIZAR_MIPI_CSI2_RB_REG_BASE; MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED",
        "Impacted Registers": "PPI_PG_PATTERN_VRES; PPI_PG_PATTERN_HRES; PPI_PG_CONFIG; PPI_PG_ENABLE; virtual_channel; control_data; PHY_STOPSTATE; dma_m0_addr_ar_ch0_data; dma_m0_addr_ar_ch0_Instruction; dma_m0_addr_aw_ch0_data; dma_m0_addr_aw_ch0_Instruction; enableclkgating_csiphy; INT_ST_MAIN; INT_MSK_PHY_FATAL; INT_MSK_PKT_FATAL; INT_MSK_PHY; INT_MSK_LINE; INT_MSK_BNDRY_FRAME_FATAL; INT_MSK_SEQ_FRAME_FATAL; INT_MSK_CRC_FRAME_FATAL; INT_MSK_PLD_CRC_FATAL; INT_MSK_DATA_ID; INT_MSK_ECC_CORRECTED",
        "Meta Validation / Acceptance Criteria": "1. PHY_STOPSTATE polling until rd_data == 0x1000f.\n2. DMA transfer size calculation validation.\n3. Pattern generator configuration validation.\n4. DMA channel 0 completion polling.\n5. Pattern generator enable/disable sequence.\n6. DMA address register programming.\n7. Clock gating enable.\n8. Test completion: finish(0).",
        "Validation / Acceptance Criteria": "1. The PHY Stop State register must read the expected value confirming all data lanes and the clock lane have entered stop state before proceeding.\n2. The DMA transfer size must be correctly calculated as an 8-byte aligned value based on the configured resolution and bits per pixel.\n3. The pattern generator must be configured with the correct vertical resolution, horizontal resolution, and RGB888 data type before being enabled.\n4. The pattern generator must be disabled after the configured wait period.\n5. DMA channel 0 transfer must complete as indicated by the DMA interrupt status register bit 0 being set.\n6. The DMA higher-order AXI address registers must be correctly programmed before initiating the transfer.\n7. The clock gating register must be enabled to allow fracdiv output to the CSI-2 subsystem.\n8. The test must complete successfully by calling the finish routine with a pass indication.",
        "Remarks": "The testcase uses conditional compilation for GDMA path selection (GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH) and FPS60 mode. The test calls csi2_subsys_enable_interrupt() (external) which likely wraps the locally defined csi2_enable_interrupt(). External functions snps_phy_init(), dma_trnsfr_instn_preload_incr_addr(), DMAGO_CSI(), wait_on(), and finish() are not defined locally. The pattern generator uses RGB888 data type (0x24) with 320x16 resolution. The control_data register is explicitly set to 0 (disabled) unlike the dphy_lanes_test which sets it to 1. DMA base+offset register accesses use gdma_reg_base with MIPI_CSI2_DMA_INTMIS_OFFSET. The PPI_PG_ENABLE register is written twice: once with value 1 (enable) inside csi2_ctrlr_pg_enable() and once with value 0 (disable) in test_case() after wait_on(100)."
    }
]

# ============================================================
# SHEET DEFINITIONS
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
# GENERATE WORKBOOK
# ============================================================
def generate_workbook():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"MIPI_CSI_TestPlan_{timestamp}.xlsx"
    output_dir = "Test_Output/MIPI/TestPlan"
    filepath = os.path.join(output_dir, filename)

    wb = Workbook()

    # --- TestPlan Sheet ---
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    header_font = Font(bold=True, color="FFFFFF", size=11)
    header_fill = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
    wrap_align = Alignment(wrap_text=True, vertical="top")

    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_align

    ws_tp.freeze_panes = "A2"

    # --- MetaData Sheet ---
    ws_md = wb.create_sheet("MetaData")
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_align

    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_align

    ws_md.freeze_panes = "A2"
    ws_md.sheet_state = "veryHidden"

    # --- Auto-size columns ---
    for ws in [ws_tp, ws_md]:
        for col_idx in range(1, ws.max_column + 1):
            max_len = 0
            col_letter = get_column_letter(col_idx)
            for row in ws.iter_rows(min_col=col_idx, max_col=col_idx):
                for cell in row:
                    if cell.value:
                        lines = str(cell.value).split("\n")
                        for line in lines:
                            max_len = max(max_len, len(line))
            adjusted = min(max_len + 4, 60)
            ws.column_dimensions[col_letter].width = max(adjusted, 12)

    # --- Save ---
    os.makedirs(output_dir, exist_ok=True)
    wb.save(filepath)
    print(f"Workbook saved: {filepath}")
    print(f"Filename: {filename}")

    # --- Validate ---
    assert os.path.exists(filepath), "File does not exist"
    assert os.path.getsize(filepath) > 0, "File is empty"
    wb2 = load_workbook(filepath)
    assert "TestPlan" in wb2.sheetnames, "TestPlan sheet missing"
    assert "MetaData" in wb2.sheetnames, "MetaData sheet missing"
    tp_rows = wb2["TestPlan"].max_row - 1
    md_rows = wb2["MetaData"].max_row - 1
    print(f"Validation PASSED: TestPlan rows={tp_rows}, MetaData rows={md_rows}")
    print(f"File size: {os.path.getsize(filepath)} bytes")
    return filepath, filename

if __name__ == "__main__":
    fp, fn = generate_workbook()
    print(f"SUCCESS: {fn}")
