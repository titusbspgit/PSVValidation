// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

// Testcase error tracking context
static unsigned int g_errors;

/*
 * Function: csi2_enable_interrupt
 * Description: Enable all CSI-2 host controller interrupts by reading
 *              INT_ST_MAIN to clear pending interrupts, then writing mask
 *              values to enable all interrupt categories.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    // Step 6: Read INT_ST_MAIN register to clear pending interrupts
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN read to clear pending interrupts, rd_data=0x%x", rd_data);

    // Step 7: Enable phy_fatal interrupts for all 4 lanes
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("INT_MSK_PHY_FATAL written with 0x0000000f");

    // Step 8: Enable pkt_fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("INT_MSK_PKT_FATAL written with 0x00000003");

    // Step 9: Enable phy interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("INT_MSK_PHY written with 0x000f000f");

    // Step 10: Enable line interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("INT_MSK_LINE written with 0x000f000f");

    // Step 11: Enable boundary frame fatal interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL written with 0x0000ffff");

    // Step 12: Enable seq frame fatal interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_SEQ_FRAME_FATAL written with 0x0000ffff");

    // Step 13: Enable CRC frame fatal interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_CRC_FRAME_FATAL written with 0x0000ffff");

    // Step 14: Enable payload CRC fatal interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("INT_MSK_PLD_CRC_FATAL written with 0x0000ffff");

    // Step 15: Enable data ID interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("INT_MSK_DATA_ID written with 0x0000ffff");

    // Step 16: Enable ECC corrected interrupts for all 16 VCs
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("INT_MSK_ECC_CORRECTED written with 0x0000ffff");

    // Step 17: Return from csi2_enable_interrupt
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Initialize testcase context and perform early setup.
 * Parameters:
 *   cfg - pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_errors = 0;

    LOGT("mipi_csi2_dphy_lanes_test init");

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Main testcase execution. Iterates through all lane counts
 *              (4 down to 1), performing DMA-based control data and CSI
 *              image data transfers for each packet in a frame.
 * Parameters:
 *   cfg - pointer to test configuration item
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    // Step 2: Declare local variables
    long long int rx_desc, tx_desc;
    long long int gdma_tx_trnsfr_size, gdma_trnsfr_size;
    int cntrl_pkt_cnt;
    int rd_data;
    int csi_ctrl_data;
    int word_count;
    long long int csi_data_size;
    int vcid_csi2_wrap_reg;
    int gdma_reg_base;
    int gdma_path;
    int ch0_pc, ch1_pc;
    int ch0_preload_loc, ch1_preload_loc;
    int lane_num;
    int i;

    (void)cfg;
    (void)rx_desc;
    (void)tx_desc;
    (void)gdma_tx_trnsfr_size;
    (void)gdma_trnsfr_size;
    (void)gdma_path;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    // Step 3: Print start line
    LOGT("start line");

    // Step 4: Call csi2_enable_interrupt() to enable CSI-2 interrupts
    LOGT("Enabling CSI-2 interrupts");
    csi2_enable_interrupt();
    LOGT("CSI-2 interrupts enabled successfully");

    // Step 18: Conditional compilation - set vcid_csi2_wrap_reg based on GDMA path
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = VC_ID;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 4);
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 8);
#elif defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
#else
    // Default to GDMA0_PATH behavior if none specified
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
#endif

    // Step 19: Set GDMA register base address
    gdma_reg_base = 0xE6A00000;
    LOGT("gdma_reg_base set to 0x%x", gdma_reg_base);

    // Step 20: First write to CSI-2 virtual channel register
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("First write to VIRTUAL_CHANNEL register with 0x%x", vcid_csi2_wrap_reg);

    // Step 21: First enable control data transfer
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("First write to CONTROL_DATA register with 1");

    // Step 22: Print vcid_csi2_wrap_reg
    LOGT("vcid_csi2_wrap_reg=%0x", vcid_csi2_wrap_reg);

    // Step 23: Second write to CSI-2 virtual channel register
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("Second write to VIRTUAL_CHANNEL register with 0x%x", vcid_csi2_wrap_reg);

    // Step 24: Second enable control data transfer
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("Second write to CONTROL_DATA register with 1");

    // Step 25: Initialize SNPS D-PHY
    LOGT("Initializing SNPS D-PHY");
    snps_phy_init();
    LOGT("SNPS D-PHY initialization complete");

    // Step 26: First read of PHY stop state
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("Initial PHY_STOPSTATE read = 0x%x", rd_data);

    // Steps 27-29: Poll PHY_STOPSTATE until all 4 data lanes and clock lane
    // are in stop state (expected value 0x1000f)
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE confirmed = 0x%x (all lanes in stop state)", rd_data);

    // Step 30: Set DMA program counter address for channel 0
    ch0_pc = 0xE6000000;
    // Step 31: Set DMA program counter address for channel 1
    ch1_pc = 0xE6000500;
    LOGT("ch0_pc=0x%x, ch1_pc=0x%x", ch0_pc, ch1_pc);

    // Steps 32-68: Iterate through lane configurations from 4 lanes (lane_num=3)
    // down to 1 lane (lane_num=0)
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        // Step 33: Configure number of lanes in CSI-2 controller
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);
        LOGT("N_LANES register written with %d (= %d active lanes)", lane_num, lane_num + 1);

        // Step 34: Compute control packet count
        cntrl_pkt_cnt = ((VRES * 3) + 2);

        // Step 35: Debug print
        LOGT("DEBUG: cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

        // Step 36: Trigger CSI-2 sequence with lane count value
        write_reg(0xa0243ffc, (lane_num + 1));
        LOGT("CSI-2 sequence triggered with lane count = %d", lane_num + 1);

        // Steps 37-67: Inner loop - process each packet in the frame
        for (i = 0; i < cntrl_pkt_cnt; i++) {

            // Step 38: Set preload location for channel 0
            ch0_preload_loc = ch0_pc;
            // Step 39: Set preload location for channel 1
            ch1_preload_loc = ch1_pc;

            // Step 40: Enable DMA interrupts for both channels
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);

            // Step 41: Preload DMA channel 0 transfer instructions for control data
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0);

            // Step 42: Start DMA channel 0
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);

            // Step 43: Set rd_data = 0
            rd_data = 0;

            // Steps 44-47: Poll DMA interrupt status for channel 0 completion
            while ((rd_data & 0x1) == 0x0) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
            }

            // Step 48: Clear DMA IRQ bit 0
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            // Step 49: Read DMA interrupt status register at offset 0x28
            rd_data = read_reg(gdma_reg_base + 0x28);

            // Step 50: Debug print
            LOGT("DEBUG : irq polling completed ch0; rd_data = %0x", rd_data);

            // Step 51: Read control data from address 0xE6001000
            csi_ctrl_data = read_reg(0xE6001000);

            // Step 52: Debug print
            LOGT("DEBUG: csi_ctrl_data=%0x", csi_ctrl_data);

            // Step 53: Check if data type field indicates CSI image data
            if ((csi_ctrl_data & 0x3f) > 0xf) {

                // Step 54: Extract word count from control data bits[21:6]
                word_count = ((csi_ctrl_data >> 6) & 0xffff);
                LOGT("word_count extracted = %d", word_count);

                // Step 55: Align transfer size to 8-byte boundary
                csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count;
                LOGT("csi_data_size (aligned) = %lld", csi_data_size);

                // Step 56: Preload DMA channel 1 transfer instructions for CSI data
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1);

                // Step 57: Start DMA channel 1
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);

                // Step 58: Set rd_data = 0
                rd_data = 0;

                // Steps 59-62: Poll DMA interrupt status for channel 1 completion
                while ((rd_data & 0x2) == 0x0) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
                }

                // Step 63: Additional read of DMA interrupt status after loop exit
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                // Step 64: Clear DMA IRQ bit 1
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);

                // Step 65: Debug print
                LOGT("DEBUG : irq polling completed ch1; rd_data=%0x", rd_data);

            // Step 66: End of if block
            }

        // Step 67: End of inner for loop
        }

        LOGT("Lane configuration %d (= %d active lanes) completed", lane_num, lane_num + 1);

    // Step 68: End of outer for loop
    }

    // Step 69: Test completion (finish(0) converted to FV status)
    LOGT("All lane configurations tested successfully");
    out->status = (g_errors == 0) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test teardown: no additional cleanup required");
    return g_errors == 0 ? 0 : -1;
}
