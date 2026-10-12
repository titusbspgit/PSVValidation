// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

// Global variables as per Meta source
long int word_count;
int gdma_int_rsts;

// Testcase context for error tracking
static unsigned int g_errors;

/*
 * Function: csi2_enable_interrupt
 * Description: Enable CSI-2 interrupts by reading INT_ST_MAIN to clear
 *              pending interrupts, then writing interrupt mask registers.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    // Step 1b: Read INT_ST_MAIN to clear pending interrupts
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read to clear pending = 0x%x", rd_data);

    // Step 1c: Write INT_MSK_PHY_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("INT_MSK_PHY_FATAL = 0x0000000f");

    // Step 1d: Write INT_MSK_PKT_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("INT_MSK_PKT_FATAL = 0x00000003");

    // Step 1e: Write INT_MSK_PHY
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("INT_MSK_PHY = 0x000f000f");

    // Step 1f: Write INT_MSK_LINE
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("INT_MSK_LINE = 0x000f000f");

    // Step 1g: Write INT_MSK_BNDRY_FRAME_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    // Step 1h: Write INT_MSK_SEQ_FRAME_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    // Step 1i: Write INT_MSK_CRC_FRAME_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    // Step 1j: Write INT_MSK_PLD_CRC_FATAL
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    // Step 1k: Write INT_MSK_DATA_ID
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("INT_MSK_DATA_ID = 0x0000ffff");

    // Step 1l: Write INT_MSK_ECC_CORRECTED
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("INT_MSK_ECC_CORRECTED = 0x0000ffff");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Initialize testcase - enable CSI-2 interrupts, configure
 *              virtual channel and control data, initialize D-PHY, and
 *              poll PHY_STOPSTATE.
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    int rd_data;
    int vcid_csi2_wrap_reg;
    int gdma_path;

    (void)cfg;

    g_errors = 0;

    LOGT("mipi_csi2_dphy_lanes_test_init: start");

    // Step 1: Call csi2_enable_interrupt()
    csi2_enable_interrupt();

    // Step 2: Conditional GDMA path selection
#ifdef GDMA3_PATH
    vcid_csi2_wrap_reg = VC_ID;
    gdma_path = 3;
    LOGT("GDMA3_PATH selected: vcid_csi2_wrap_reg=VC_ID=%d, gdma_path=3", VC_ID);
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 4);
    gdma_path = 2;
    LOGT("GDMA2_PATH selected: vcid_csi2_wrap_reg=0x%x, gdma_path=2", vcid_csi2_wrap_reg);
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 8);
    gdma_path = 1;
    LOGT("GDMA1_PATH selected: vcid_csi2_wrap_reg=0x%x, gdma_path=1", vcid_csi2_wrap_reg);
#elif defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
    LOGT("GDMA0_PATH selected: vcid_csi2_wrap_reg=0x%x, gdma_path=0", vcid_csi2_wrap_reg);
#else
    vcid_csi2_wrap_reg = VC_ID;
    gdma_path = 3;
    LOGT("Default GDMA path: vcid_csi2_wrap_reg=VC_ID=%d, gdma_path=3", VC_ID);
#endif

    (void)gdma_path;

    // Step 4: write_reg VIRTUAL_CHANNEL (first write)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL = 0x%x (first write)", vcid_csi2_wrap_reg);

    // Step 5: write_reg CONTROL_DATA (first write)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("CONTROL_DATA = 1 (first write)");

    // Step 6: printf
    LOGT("vcid_csi2_wrap_reg=%0x", vcid_csi2_wrap_reg);

    // Step 7: write_reg VIRTUAL_CHANNEL (second write)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL = 0x%x (second write)", vcid_csi2_wrap_reg);

    // Step 8: write_reg CONTROL_DATA (second write)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("CONTROL_DATA = 1 (second write)");

    // Step 9: Call snps_phy_init()
    LOGT("Calling snps_phy_init()");
    snps_phy_init();
    LOGT("snps_phy_init() complete");

    // Step 10: rd_data = read_reg(PHY_STOPSTATE)
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("PHY_STOPSTATE initial read = 0x%x", rd_data);

    // Step 11: Poll PHY_STOPSTATE until 0x1000f
    LOGT("Polling PHY_STOPSTATE for 0x1000f");
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    // Validation Criteria 1: PHY_STOPSTATE must equal 0x1000f
    LOGT("PHY_STOPSTATE = 0x%x - all lanes in stop state", rd_data);

    LOGT("mipi_csi2_dphy_lanes_test_init: complete");
    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Main testcase execution - iterate through lane configurations
 *              (4 down to 1), perform DMA-based control and data packet
 *              transfers with interrupt-driven completion polling.
 * Parameters:
 *   cfg - pointer to test configuration
 *   out - pointer to test output
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    unsigned int gdma_reg_base;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    int cntrl_pkt_cnt;
    int csi_ctrl_data;
    int csi_data_size;
    int lane_num;
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: start");

    // Step 3: gdma_reg_base = 0xE6A00000
    gdma_reg_base = 0xE6A00000;
    LOGT("gdma_reg_base = 0x%x", gdma_reg_base);

    // Step 12: ch0_pc = 0xE6000000
    ch0_pc = 0xE6000000;
    LOGT("ch0_pc = 0x%x", ch0_pc);

    // Step 13: ch1_pc = 0xE6000500
    ch1_pc = 0xE6000500;
    LOGT("ch1_pc = 0x%x", ch1_pc);

    // Step 14: for(int lane_num=3; lane_num>=0; lane_num--)
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("Lane configuration: %d lane(s) (lane_num=%d)", lane_num + 1, lane_num);

        // Step 14a: write_reg N_LANES
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);
        LOGT("N_LANES = %d", lane_num);

        // Step 14b: cntrl_pkt_cnt = ((VRES * 3) + 2)
        cntrl_pkt_cnt = ((VRES * 3) + 2);

        // Step 14c: printf
        LOGT("DEBUG: cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

        // Step 14d: write_reg(0xa0243ffc, (lane_num+1))
        write_reg(0xa0243ffc, (lane_num + 1));
        LOGT("Trigger CSI-2 sequence: write_reg(0xa0243ffc, %d)", lane_num + 1);

        // Step 14e: for(int i=0; i< cntrl_pkt_cnt; i++)
        for (i = 0; i < cntrl_pkt_cnt; i++) {

            // Step 14e-i: ch0_preload_loc = ch0_pc
            ch0_preload_loc = ch0_pc;

            // Step 14e-ii: ch1_preload_loc = ch1_pc
            ch1_preload_loc = ch1_pc;

            // Step 14e-iii: write_reg DMA_INTEN
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);

            // Step 14e-iv: dma_trnsfr_instn_preload ch0
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0);

            // Step 14e-v: DMAGO_CSI ch0
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);

            // Step 14e-vi: rd_data = 0
            rd_data = 0;

            // Step 14e-vii: Poll DMA interrupt status for ch0 completion
            while ((rd_data & 0x1) == 0x0) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
            }
            // Validation Criteria 2: DMA ch0 completion bit 0 must be set

            // Step 14e-viii: Clear DMA ch0 interrupt
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            // Step 14e-ix: Read DMA status at offset 0x28
            rd_data = read_reg(gdma_reg_base + 0x28);

            // Step 14e-x: printf
            LOGT("DEBUG : irq polling completed ch0; rd_data = %0x", rd_data);

            // Step 14e-xi: Read control data
            csi_ctrl_data = read_reg(0xE6001000);

            // Step 14e-xii: printf
            LOGT("DEBUG: csi_ctrl_data=%0x", csi_ctrl_data);

            // Step 14e-xiii: if((csi_ctrl_data & 0x3f) > 0xf)
            if ((csi_ctrl_data & 0x3f) > 0xf) {

                // Step 14e-xiii-a: Extract word_count
                word_count = ((csi_ctrl_data >> 6) & 0xffff);
                LOGT("word_count = %ld", word_count);

                // Step 14e-xiii-b: Compute 8-byte aligned csi_data_size
                // Validation Criteria 5: CSI data size alignment
                csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count;
                LOGT("csi_data_size = %d (8-byte aligned)", csi_data_size);

                // Step 14e-xiii-c: dma_trnsfr_instn_preload ch1
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1);

                // Step 14e-xiii-d: DMAGO_CSI ch1
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);

                // Step 14e-xiii-e: rd_data = 0
                rd_data = 0;

                // Step 14e-xiii-f: Poll DMA interrupt status for ch1 completion
                while ((rd_data & 0x2) == 0x0) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
                }
                // Validation Criteria 4: DMA ch1 completion bit 1 must be set

                // Step 14e-xiii-g: Read final DMA interrupt status
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                // Step 14e-xiii-h: Clear DMA ch1 interrupt
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);

                // Step 14e-xiii-i: printf
                LOGT("DEBUG : irq polling completed ch1; rd_data=%0x", rd_data);
            }
        }

        // Validation Criteria 5: Lane configuration completed
        LOGT("Lane configuration %d lane(s) completed", lane_num + 1);
    }

    // Step 15: finish(0) -> FV/PSV status = PASS
    // Validation Criteria 6: Test completion indicates pass
    LOGT("All 4 lane configurations completed successfully");
    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Teardown and final status reporting.
 * Parameters:
 *   cfg - pointer to test configuration
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test_teardown: errors=%u", g_errors);
    return g_errors == 0U ? 0 : -1;
}
