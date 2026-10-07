#!/usr/bin/env python3
"""
MIPI CSI Test Plan Generator
Generated: 2025-07-29 20:00:00 IST (GMT+05:30)
IP: MIPI_CSI
Source: TestRepo/mipi/mipi_csi2_subsys
"""

import json

# ============================================================
# COMPLETE TEST PLAN DATA
# ============================================================

test_plan_data = [
    {
        "S.No": 1,
        "IP Name": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_dphy_lanes_test",
        "Feature": "DPHY Lane Configuration and DMA Data Transfer",
        "Source File": "TestRepo/mipi/mipi_csi2_subsys/mipi_csi2_dphy_lanes_test/program.c",
        "Test Description": "This test validates MIPI CSI-2 DPHY lane configuration by iterating through all supported lane counts (4 lanes down to 1 lane). For each lane configuration, the test enables CSI-2 interrupts by clearing the main interrupt status and enabling all interrupt mask registers (PHY fatal, packet fatal, PHY, line, boundary frame fatal, sequence frame fatal, CRC frame fatal, payload CRC fatal, data ID, and ECC corrected). It then configures the virtual channel, enables control data transfer, initializes the D-PHY, and polls the PHY stop state register until the PHY reaches the expected stop state (0x1000f). For each lane count, the N_LANES register is written with the lane number, a trigger write initiates the CSI-2 sequence, and a nested loop processes control and data packets using DMA transfers. Each DMA transfer involves preloading transfer instructions, starting the DMA channel, polling the DMA interrupt status for completion, clearing the interrupt, and reading control data to determine if a data payload transfer is needed.",
        "Test Steps / Procedure": (
            "1. Call csi2_enable_interrupt() to read INT_ST_MAIN (clear pending interrupts) and write all interrupt mask registers.\n"
            "2. Write MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg (VC_ID shifted based on GDMA path).\n"
            "3. Write MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 1 to enable control data transfer.\n"
            "4. Call snps_phy_init() for D-PHY initialization.\n"
            "5. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until it equals 0x1000f.\n"
            "6. For lane_num = 3 down to 0:\n"
            "   a. Write MIZAR_MIPI_CSI2_HOST_N_LANES with lane_num.\n"
            "   b. Write 0xa0243ffc with (lane_num+1) to trigger CSI-2 sequence.\n"
            "   c. For each control packet ((VRES*3)+2 iterations):\n"
            "      i. Write gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET with 0x3 (enable DMA IRQ).\n"
            "      ii. Preload DMA ch0 transfer instructions and start DMAGO_CSI for ch0.\n"
            "      iii. Poll gdma_reg_base+MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set.\n"
            "      iv. Write gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET with 0x1 to clear IRQ.\n"
            "      v. Read control data from 0xE6001000.\n"
            "      vi. If data_type > 0xf: extract word_count, compute aligned csi_data_size, preload DMA ch1, start DMAGO_CSI for ch1, poll INTMIS until bit 1 set, clear with 0x2.\n"
            "7. Call finish(0)."
        ),
        "Impacted Registers": (
            "virtual_channel (csi_reg, offset 0x0); "
            "control_data (csi_reg, offset 0x20); "
            "PHY_STOPSTATE (CSI2 Host, offset 0x4C); "
            "N_LANES (CSI2 Host, offset 0x4); "
            "INT_ST_MAIN (CSI2 Host, offset 0xC); "
            "INT_MSK_PHY_FATAL (CSI2 Host, offset 0xE4); "
            "INT_MSK_PKT_FATAL (CSI2 Host, offset 0xF4); "
            "INT_MSK_PHY (CSI2 Host, offset 0x114); "
            "INT_MSK_LINE (CSI2 Host, offset 0x134); "
            "INT_MSK_BNDRY_FRAME_FATAL (CSI2 Host, offset 0x284); "
            "INT_MSK_SEQ_FRAME_FATAL (CSI2 Host, offset 0x294); "
            "INT_MSK_CRC_FRAME_FATAL (CSI2 Host, offset 0x2A4); "
            "INT_MSK_PLD_CRC_FATAL (CSI2 Host, offset 0x2B4); "
            "INT_MSK_DATA_ID (CSI2 Host, offset 0x2C4); "
            "INT_MSK_ECC_CORRECTED (CSI2 Host, offset 0x2D4); "
            "DMA INTEN (DMA330, offset 0x20); "
            "DMA INTMIS (DMA330, offset 0x28); "
            "DMA INTCLR (DMA330, offset 0x2C)"
        ),
        "Register Operations": [
            {"Step": 1, "Operation": "READ", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN", "Register Name": "INT_ST_MAIN", "Offset": "0x0C", "Write Value": "N/A", "Expected Value": "Clears pending interrupts", "Description": "Read INT_ST_MAIN register to clear interrupts"},
            {"Step": 2, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL", "Register Name": "INT_MSK_PHY_FATAL", "Offset": "0xE4", "Write Value": "0x0000000F", "Expected Value": "N/A", "Description": "Enable PHY fatal interrupts"},
            {"Step": 3, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL", "Register Name": "INT_MSK_PKT_FATAL", "Offset": "0xF4", "Write Value": "0x00000003", "Expected Value": "N/A", "Description": "Enable packet fatal interrupts"},
            {"Step": 4, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY", "Register Name": "INT_MSK_PHY", "Offset": "0x114", "Write Value": "0x000F000F", "Expected Value": "N/A", "Description": "Enable PHY interrupts"},
            {"Step": 5, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE", "Register Name": "INT_MSK_LINE", "Offset": "0x134", "Write Value": "0x000F000F", "Expected Value": "N/A", "Description": "Enable line interrupts"},
            {"Step": 6, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL", "Register Name": "INT_MSK_BNDRY_FRAME_FATAL", "Offset": "0x284", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable boundary frame fatal interrupts"},
            {"Step": 7, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL", "Register Name": "INT_MSK_SEQ_FRAME_FATAL", "Offset": "0x294", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable sequence frame fatal interrupts"},
            {"Step": 8, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL", "Register Name": "INT_MSK_CRC_FRAME_FATAL", "Offset": "0x2A4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable CRC frame fatal interrupts"},
            {"Step": 9, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL", "Register Name": "INT_MSK_PLD_CRC_FATAL", "Offset": "0x2B4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable payload CRC fatal interrupts"},
            {"Step": 10, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID", "Register Name": "INT_MSK_DATA_ID", "Offset": "0x2C4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable data ID interrupts"},
            {"Step": 11, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED", "Register Name": "INT_MSK_ECC_CORRECTED", "Offset": "0x2D4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable ECC corrected interrupts"},
            {"Step": 12, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL", "Register Name": "virtual_channel", "Offset": "0x00", "Write Value": "vcid_csi2_wrap_reg (VC_ID << 12 for GDMA0)", "Expected Value": "N/A", "Description": "Write CSI-2 virtual channel register"},
            {"Step": 13, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA", "Register Name": "control_data", "Offset": "0x20", "Write Value": "0x00000001", "Expected Value": "N/A", "Description": "Enable control data transfer"},
            {"Step": 14, "Operation": "READ (POLL)", "Macro": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE", "Register Name": "PHY_STOPSTATE", "Offset": "0x4C", "Write Value": "N/A", "Expected Value": "0x0001000F", "Description": "Poll until PHY enters stop state on all lanes"},
            {"Step": 15, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_N_LANES", "Register Name": "N_LANES", "Offset": "0x04", "Write Value": "lane_num (3 down to 0)", "Expected Value": "N/A", "Description": "Configure number of active DPHY lanes"},
            {"Step": 16, "Operation": "WRITE", "Macro": "N/A (hardcoded)", "Register Name": "External trigger", "Offset": "0xa0243ffc", "Write Value": "(lane_num+1)", "Expected Value": "N/A", "Description": "Trigger CSI-2 sequence start"},
            {"Step": 17, "Operation": "WRITE", "Macro": "MIPI_CSI2_DMA_INTEN_OFFSET", "Register Name": "INTEN (DMA330)", "Offset": "gdma_reg_base+0x20", "Write Value": "0x00000003", "Expected Value": "N/A", "Description": "Enable DMA IRQ[1] and IRQ[0]"},
            {"Step": 18, "Operation": "READ (POLL)", "Macro": "MIPI_CSI2_DMA_INTMIS_OFFSET", "Register Name": "INTMIS (DMA330)", "Offset": "gdma_reg_base+0x28", "Write Value": "N/A", "Expected Value": "Bit 0 = 1 (ch0 done); Bit 1 = 1 (ch1 done)", "Description": "Poll DMA interrupt masked status for transfer completion"},
            {"Step": 19, "Operation": "WRITE", "Macro": "MIPI_CSI2_DMA_INTCLR_OFFSET", "Register Name": "INTCLR (DMA330)", "Offset": "gdma_reg_base+0x2C", "Write Value": "0x1 (ch0) / 0x2 (ch1)", "Expected Value": "N/A", "Description": "Clear DMA interrupt after transfer completion"},
            {"Step": 20, "Operation": "READ", "Macro": "N/A (hardcoded)", "Register Name": "CSI control data", "Offset": "0xE6001000", "Write Value": "N/A", "Expected Value": "Control packet data", "Description": "Read CSI control data from destination address"}
        ],
        "Validation / Acceptance Criteria": (
            "1. PHY_STOPSTATE register must read 0x1000f confirming all DPHY lanes entered stop state.\n"
            "2. DMA INTMIS bit 0 must be set after ch0 control data transfer completes.\n"
            "3. DMA INTMIS bit 1 must be set after ch1 image data transfer completes (when data_type > 0xf).\n"
            "4. Test must iterate through all 4 lane configurations (3, 2, 1, 0) without errors.\n"
            "5. finish(0) must be called indicating successful completion."
        ),
        "Macros Used": (
            "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; "
            "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; MIZAR_MIPI_CSI2_HOST_N_LANES; "
            "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIPI_CSI2_DMA_INTEN_OFFSET; "
            "MIPI_CSI2_DMA_INTMIS_OFFSET; MIPI_CSI2_DMA_INTCLR_OFFSET"
        ),
        "Macro Resolutions": {
            "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN": "CSI2 Host Base + 0x0C",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL": "CSI2 Host Base + 0xE4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL": "CSI2 Host Base + 0xF4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY": "CSI2 Host Base + 0x114",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE": "CSI2 Host Base + 0x134",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL": "CSI2 Host Base + 0x284",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL": "CSI2 Host Base + 0x294",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL": "CSI2 Host Base + 0x2A4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL": "CSI2 Host Base + 0x2B4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID": "CSI2 Host Base + 0x2C4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED": "CSI2 Host Base + 0x2D4",
            "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE": "CSI2 Host Base + 0x4C",
            "MIZAR_MIPI_CSI2_HOST_N_LANES": "CSI2 Host Base + 0x04",
            "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL": "CSI2 RB Base + 0x00",
            "MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA": "CSI2 RB Base + 0x20",
            "MIPI_CSI2_DMA_INTEN_OFFSET": "0x20",
            "MIPI_CSI2_DMA_INTMIS_OFFSET": "0x28",
            "MIPI_CSI2_DMA_INTCLR_OFFSET": "0x2C"
        },
        "Register Mapping": {
            "INT_ST_MAIN": {"Offset": "0x0C", "Width": 32, "Access": "RC", "Block": "DWC_mipi_csi2_host", "Key Fields": "status_int_st_phy_fatal[0], status_int_st_pkt_fatal[1], status_int_st_phy[16], status_int_st_line[17]"},
            "INT_MSK_PHY_FATAL": {"Offset": "0xE4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "mask_phy_errsotsynchs_0[0], mask_phy_errsotsynchs_1[1], mask_phy_errsotsynchs_2[2], mask_phy_errsotsynchs_3[3]"},
            "INT_MSK_PKT_FATAL": {"Offset": "0xF4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "mask_err_ecc_double[0], mask_shorter_payload[1]"},
            "INT_MSK_PHY": {"Offset": "0x114", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "mask_phy_errsoths_0..3[0:3], mask_phy_erresc_0..3[16:19]"},
            "INT_MSK_LINE": {"Offset": "0x134", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "mask_err_l_bndry_match_di0..3[0:3], mask_err_l_seq_di0..3[16:19]"},
            "INT_MSK_BNDRY_FRAME_FATAL": {"Offset": "0x284", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_f_bndry_match_vc0..15[0:15]"},
            "INT_MSK_SEQ_FRAME_FATAL": {"Offset": "0x294", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_f_seq_vc0..15[0:15]"},
            "INT_MSK_CRC_FRAME_FATAL": {"Offset": "0x2A4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_frame_data_vc0..15[0:15]"},
            "INT_MSK_PLD_CRC_FATAL": {"Offset": "0x2B4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_crc_vc0..15[0:15]"},
            "INT_MSK_DATA_ID": {"Offset": "0x2C4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_id_vc0..15[0:15]"},
            "INT_MSK_ECC_CORRECTED": {"Offset": "0x2D4", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "err_ecc_corrected_vc0..15[0:15]"},
            "PHY_STOPSTATE": {"Offset": "0x4C", "Width": 32, "Access": "RO", "Block": "DWC_mipi_csi2_host", "Key Fields": "phy_stopstatedata_0..3[0:3], phy_stopstateclk[16]"},
            "N_LANES": {"Offset": "0x04", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "n_lanes[2:0]"},
            "virtual_channel": {"Offset": "0x00", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "ch0[15:12], ch1[11:8], ch2[7:4], ch3[3:0]"},
            "control_data": {"Offset": "0x20", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "en[0]"},
            "INTEN (DMA)": {"Offset": "0x20", "Width": 32, "Access": "RW", "Block": "DMA330", "Key Fields": "event_irq_select[31:0]"},
            "INTMIS (DMA)": {"Offset": "0x28", "Width": 32, "Access": "RO", "Block": "DMA330", "Key Fields": "irq_status[31:0]"},
            "INTCLR (DMA)": {"Offset": "0x2C", "Width": 32, "Access": "WO", "Block": "DMA330", "Key Fields": "irq_clr[31:0]"}
        }
    },
    {
        "S.No": 2,
        "IP Name": "MIPI_CSI",
        "Test Case Name": "mipi_csi2_test_pattern_generator",
        "Feature": "Internal Test Pattern Generator (PPI PG) and DMA Transfer",
        "Source File": "TestRepo/mipi/mipi_csi2_subsys/mipi_csi2_test_pattern_generator/program.c",
        "Test Description": "This test validates the MIPI CSI-2 internal test pattern generator (PPI PG) functionality. The test configures the CSI-2 subsystem to receive internally generated test pattern data instead of external D-PHY input. It configures the virtual channel register, disables control data transfer, enables all CSI-2 interrupt masks, initializes the D-PHY, and polls the PHY stop state register until the PHY reaches the expected stop state (0x1000f). DMA address registers for channel 0 read and write paths are configured. DMA transfer instructions are preloaded for a computed transfer size based on 320x16 resolution at 24 bits per pixel (RGB888) with 8-byte alignment, and DMA channel 0 is started. The pattern generator is then enabled with vertical resolution=0x10, horizontal resolution=0x70140, and configuration=0xe401. After a brief wait, the pattern generator is disabled, and the DMA interrupt status is polled until the transfer completes.",
        "Test Steps / Procedure": (
            "1. Call csi2_enable_interrupt() to read INT_ST_MAIN (clear pending interrupts) and write all interrupt mask registers.\n"
            "2. Write MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL with vcid_csi2_wrap_reg.\n"
            "3. Write MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with 0 to disable control data transfer.\n"
            "4. Call csi2_subsys_enable_interrupt() to enable subsystem-level interrupts.\n"
            "5. Call snps_phy_init() for D-PHY initialization.\n"
            "6. Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until it equals 0x1000f.\n"
            "7. Write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA with 0x100.\n"
            "8. Write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION with 0x0.\n"
            "9. Write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA with 0x0.\n"
            "10. Write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION with 0x0.\n"
            "11. Write MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 with 0x1 (enable fracdiv output / clock gating).\n"
            "12. Preload DMA ch0 transfer instructions and start DMAGO_CSI for ch0.\n"
            "13. Call csi2_ctrlr_pg_enable():\n"
            "    a. Write MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES with 0x10.\n"
            "    b. Write MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES with 0x70140.\n"
            "    c. Write MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG with 0xe401.\n"
            "    d. Write MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with 1.\n"
            "14. Wait 100 cycles.\n"
            "15. Write MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with 0 to disable pattern generator.\n"
            "16. Poll gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET until bit 0 is set.\n"
            "17. Wait 10000 cycles and call finish(0)."
        ),
        "Impacted Registers": (
            "virtual_channel (csi_reg, offset 0x0); "
            "control_data (csi_reg, offset 0x20); "
            "PHY_STOPSTATE (CSI2 Host, offset 0x4C); "
            "PPI_PG_PATTERN_VRES (CSI2 Host, offset 0x60); "
            "PPI_PG_PATTERN_HRES (CSI2 Host, offset 0x64); "
            "PPI_PG_CONFIG (CSI2 Host, offset 0x68); "
            "PPI_PG_ENABLE (CSI2 Host, offset 0x6C); "
            "INT_ST_MAIN (CSI2 Host, offset 0xC); "
            "INT_MSK_PHY_FATAL (CSI2 Host, offset 0xE4); "
            "INT_MSK_PKT_FATAL (CSI2 Host, offset 0xF4); "
            "INT_MSK_PHY (CSI2 Host, offset 0x114); "
            "INT_MSK_LINE (CSI2 Host, offset 0x134); "
            "INT_MSK_BNDRY_FRAME_FATAL (CSI2 Host, offset 0x284); "
            "INT_MSK_SEQ_FRAME_FATAL (CSI2 Host, offset 0x294); "
            "INT_MSK_CRC_FRAME_FATAL (CSI2 Host, offset 0x2A4); "
            "INT_MSK_PLD_CRC_FATAL (CSI2 Host, offset 0x2B4); "
            "INT_MSK_DATA_ID (CSI2 Host, offset 0x2C4); "
            "INT_MSK_ECC_CORRECTED (CSI2 Host, offset 0x2D4); "
            "dma_m0_addr_ar_ch0_data (csi_reg, offset 0x6C); "
            "dma_m0_addr_ar_ch0_Instruction (csi_reg, offset 0x70); "
            "dma_m0_addr_aw_ch0_data (csi_reg, offset 0x8C); "
            "dma_m0_addr_aw_ch0_Instruction (csi_reg, offset 0x90); "
            "enableclkgating_csiphy (csi_reg, offset 0xF4); "
            "DMA INTMIS (DMA330, offset 0x28)"
        ),
        "Register Operations": [
            {"Step": 1, "Operation": "READ", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN", "Register Name": "INT_ST_MAIN", "Offset": "0x0C", "Write Value": "N/A", "Expected Value": "Clears pending interrupts", "Description": "Read INT_ST_MAIN register to clear interrupts"},
            {"Step": 2, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL", "Register Name": "INT_MSK_PHY_FATAL", "Offset": "0xE4", "Write Value": "0x0000000F", "Expected Value": "N/A", "Description": "Enable PHY fatal interrupts"},
            {"Step": 3, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL", "Register Name": "INT_MSK_PKT_FATAL", "Offset": "0xF4", "Write Value": "0x00000003", "Expected Value": "N/A", "Description": "Enable packet fatal interrupts"},
            {"Step": 4, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY", "Register Name": "INT_MSK_PHY", "Offset": "0x114", "Write Value": "0x000F000F", "Expected Value": "N/A", "Description": "Enable PHY interrupts"},
            {"Step": 5, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE", "Register Name": "INT_MSK_LINE", "Offset": "0x134", "Write Value": "0x000F000F", "Expected Value": "N/A", "Description": "Enable line interrupts"},
            {"Step": 6, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL", "Register Name": "INT_MSK_BNDRY_FRAME_FATAL", "Offset": "0x284", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable boundary frame fatal interrupts"},
            {"Step": 7, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL", "Register Name": "INT_MSK_SEQ_FRAME_FATAL", "Offset": "0x294", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable sequence frame fatal interrupts"},
            {"Step": 8, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL", "Register Name": "INT_MSK_CRC_FRAME_FATAL", "Offset": "0x2A4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable CRC frame fatal interrupts"},
            {"Step": 9, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL", "Register Name": "INT_MSK_PLD_CRC_FATAL", "Offset": "0x2B4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable payload CRC fatal interrupts"},
            {"Step": 10, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID", "Register Name": "INT_MSK_DATA_ID", "Offset": "0x2C4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable data ID interrupts"},
            {"Step": 11, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED", "Register Name": "INT_MSK_ECC_CORRECTED", "Offset": "0x2D4", "Write Value": "0x0000FFFF", "Expected Value": "N/A", "Description": "Enable ECC corrected interrupts"},
            {"Step": 12, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL", "Register Name": "virtual_channel", "Offset": "0x00", "Write Value": "vcid_csi2_wrap_reg", "Expected Value": "N/A", "Description": "Write CSI-2 virtual channel register"},
            {"Step": 13, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA", "Register Name": "control_data", "Offset": "0x20", "Write Value": "0x00000000", "Expected Value": "N/A", "Description": "Disable control data transfer"},
            {"Step": 14, "Operation": "READ (POLL)", "Macro": "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE", "Register Name": "PHY_STOPSTATE", "Offset": "0x4C", "Write Value": "N/A", "Expected Value": "0x0001000F", "Description": "Poll until PHY enters stop state"},
            {"Step": 15, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA", "Register Name": "dma_m0_addr_ar_ch0_data", "Offset": "0x6C", "Write Value": "0x00000100", "Expected Value": "N/A", "Description": "Configure DMA ch0 read address data MSB"},
            {"Step": 16, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION", "Register Name": "dma_m0_addr_ar_ch0_Instruction", "Offset": "0x70", "Write Value": "0x00000000", "Expected Value": "N/A", "Description": "Configure DMA ch0 read address instruction MSB"},
            {"Step": 17, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA", "Register Name": "dma_m0_addr_aw_ch0_data", "Offset": "0x8C", "Write Value": "0x00000000", "Expected Value": "N/A", "Description": "Configure DMA ch0 write address data MSB"},
            {"Step": 18, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION", "Register Name": "dma_m0_addr_aw_ch0_Instruction", "Offset": "0x90", "Write Value": "0x00000000", "Expected Value": "N/A", "Description": "Configure DMA ch0 write address instruction MSB"},
            {"Step": 19, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_RB_REG_BASE + 0xF4", "Register Name": "enableclkgating_csiphy", "Offset": "0xF4", "Write Value": "0x00000001", "Expected Value": "N/A", "Description": "Enable sending fracdiv output to CSI2 subsystem"},
            {"Step": 20, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES", "Register Name": "PPI_PG_PATTERN_VRES", "Offset": "0x60", "Write Value": "0x00000010", "Expected Value": "N/A", "Description": "Set pattern generator vertical resolution to 16"},
            {"Step": 21, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES", "Register Name": "PPI_PG_PATTERN_HRES", "Offset": "0x64", "Write Value": "0x00070140", "Expected Value": "N/A", "Description": "Set pattern generator horizontal resolution (pkt2pkt_time=7, hres=320)"},
            {"Step": 22, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG", "Register Name": "PPI_PG_CONFIG", "Offset": "0x68", "Write Value": "0x0000E401", "Expected Value": "N/A", "Description": "Configure PG: pattern=1, datatype=0x24(RGB888), vc=3"},
            {"Step": 23, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE", "Register Name": "PPI_PG_ENABLE", "Offset": "0x6C", "Write Value": "0x00000001", "Expected Value": "N/A", "Description": "Enable pattern generator"},
            {"Step": 24, "Operation": "WRITE", "Macro": "MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE", "Register Name": "PPI_PG_ENABLE", "Offset": "0x6C", "Write Value": "0x00000000", "Expected Value": "N/A", "Description": "Disable pattern generator after wait"},
            {"Step": 25, "Operation": "READ (POLL)", "Macro": "MIPI_CSI2_DMA_INTMIS_OFFSET", "Register Name": "INTMIS (DMA330)", "Offset": "gdma_reg_base+0x28", "Write Value": "N/A", "Expected Value": "Bit 0 = 1 (DMA ch0 transfer complete)", "Description": "Poll DMA interrupt status for transfer completion"}
        ],
        "Validation / Acceptance Criteria": (
            "1. PHY_STOPSTATE register must read 0x1000f confirming all DPHY lanes entered stop state.\n"
            "2. Pattern generator must be enabled with correct VRES=0x10, HRES=0x70140, CONFIG=0xe401.\n"
            "3. DMA INTMIS bit 0 must be set after DMA ch0 data transfer completes.\n"
            "4. Pattern generator must be disabled after data transfer.\n"
            "5. finish(0) must be called indicating successful completion."
        ),
        "Macros Used": (
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES; MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES; "
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG; MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE; "
            "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL; MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA; "
            "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE; "
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION; "
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA; MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION; "
            "MIZAR_MIPI_CSI2_RB_REG_BASE; "
            "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE; MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL; MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID; "
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED; MIPI_CSI2_DMA_INTMIS_OFFSET"
        ),
        "Macro Resolutions": {
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES": "CSI2 Host Base + 0x60",
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES": "CSI2 Host Base + 0x64",
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG": "CSI2 Host Base + 0x68",
            "MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE": "CSI2 Host Base + 0x6C",
            "MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN": "CSI2 Host Base + 0x0C",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL": "CSI2 Host Base + 0xE4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL": "CSI2 Host Base + 0xF4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY": "CSI2 Host Base + 0x114",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE": "CSI2 Host Base + 0x134",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL": "CSI2 Host Base + 0x284",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL": "CSI2 Host Base + 0x294",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL": "CSI2 Host Base + 0x2A4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL": "CSI2 Host Base + 0x2B4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID": "CSI2 Host Base + 0x2C4",
            "MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED": "CSI2 Host Base + 0x2D4",
            "MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE": "CSI2 Host Base + 0x4C",
            "MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL": "CSI2 RB Base + 0x00",
            "MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA": "CSI2 RB Base + 0x20",
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA": "CSI2 RB Base + 0x6C",
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION": "CSI2 RB Base + 0x70",
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA": "CSI2 RB Base + 0x8C",
            "MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION": "CSI2 RB Base + 0x90",
            "MIZAR_MIPI_CSI2_RB_REG_BASE + 0xF4": "CSI2 RB Base + 0xF4",
            "MIPI_CSI2_DMA_INTMIS_OFFSET": "0x28"
        },
        "Register Mapping": {
            "PPI_PG_PATTERN_VRES": {"Offset": "0x60", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "ppi_pg_pattern_vres[15:0]"},
            "PPI_PG_PATTERN_HRES": {"Offset": "0x64", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "ppi_pg_pattern_hres[15:0], ppi_pg_pkt2pkt_time[25:16]"},
            "PPI_PG_CONFIG": {"Offset": "0x68", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "ppi_pg_pattern[0], ppi_pg_datatype[13:8], ppi_pg_vc[15:14], ppi_pg_vcx_0_1[17:16]"},
            "PPI_PG_ENABLE": {"Offset": "0x6C", "Width": 32, "Access": "RW", "Block": "DWC_mipi_csi2_host", "Key Fields": "ppi_pg_enable[0]"},
            "INT_ST_MAIN": {"Offset": "0x0C", "Width": 32, "Access": "RC", "Block": "DWC_mipi_csi2_host", "Key Fields": "status_int_st_phy_fatal[0], status_int_st_pkt_fatal[1], status_int_st_phy[16], status_int_st_line[17]"},
            "PHY_STOPSTATE": {"Offset": "0x4C", "Width": 32, "Access": "RO", "Block": "DWC_mipi_csi2_host", "Key Fields": "phy_stopstatedata_0..3[0:3], phy_stopstateclk[16]"},
            "virtual_channel": {"Offset": "0x00", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "ch0[15:12], ch1[11:8], ch2[7:4], ch3[3:0]"},
            "control_data": {"Offset": "0x20", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "en[0]"},
            "dma_m0_addr_ar_ch0_data": {"Offset": "0x6C", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "msb[8:0]"},
            "dma_m0_addr_ar_ch0_Instruction": {"Offset": "0x70", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "msb[8:0]"},
            "dma_m0_addr_aw_ch0_data": {"Offset": "0x8C", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "msb[7:0]"},
            "dma_m0_addr_aw_ch0_Instruction": {"Offset": "0x90", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "msb[7:0]"},
            "enableclkgating_csiphy": {"Offset": "0xF4", "Width": 32, "Access": "RW", "Block": "csi_reg (wrapper)", "Key Fields": "sel[0]"},
            "INTMIS (DMA)": {"Offset": "0x28", "Width": 32, "Access": "RO", "Block": "DMA330", "Key Fields": "irq_status[31:0]"}
        }
    }
]

print(json.dumps(test_plan_data, indent=2))
print("\n=== MIPI_CSI Test Plan Data Generated Successfully ===")
print(f"Total Test Cases: {len(test_plan_data)}")
for tc in test_plan_data:
    print(f"  {tc['S.No']}. {tc['Test Case Name']} - {tc['Feature']}")
    print(f"     Register Operations: {len(tc['Register Operations'])}")
    print(f"     Macro Resolutions: {len(tc['Macro Resolutions'])}")
    print(f"     Register Mappings: {len(tc['Register Mapping'])}")
