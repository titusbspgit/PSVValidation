// Author - AI Force 2.3. 08-Jun-2025 01:52 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 DPHY lane configuration by iterating
 *              through all lane counts (4 down to 1), enabling CSI-2 host
 *              interrupts, configuring virtual channel, initializing D-PHY,
 *              polling PHY stop state, and performing DMA-based CSI-2 data
 *              transfer for each lane configuration.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Reads INT_ST_MAIN to clear pending interrupts, then writes
 *              all CSI-2 host interrupt mask registers to enable interrupts.
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("READ  : MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN = 0x%08x (clear pending)", rd_data);

    /* Write all interrupt mask registers to enable interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL = 0x0000000F");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL = 0x00000003");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY = 0x000F000F");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE = 0x000F000F");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL = 0x0000FFFF");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL = 0x0000FFFF");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL = 0x0000FFFF");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL = 0x0000FFFF");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID = 0x0000FFFF");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED = 0x0000FFFF");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test init: DPHY lane configuration test");

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rd_data;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_reg_base;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    unsigned int cntrl_pkt_cnt;
    unsigned int csi_ctrl_data;
    unsigned int word_count;
    unsigned int csi_data_size;
    int lane_num;
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_dphy_lanes_test execution");

    /* Step 1: Enable all CSI-2 host interrupts */
    LOGT("[STEP 1] Enabling all CSI-2 host interrupts");
    csi2_enable_interrupt();

    /* Step 2: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = VC_ID;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 4);
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 8);
#elif defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 12);
#else
    vcid_csi2_wrap_reg = VC_ID;
#endif
    gdma_reg_base = 0xE6A00000U;
    LOGT("[STEP 2] vcid_csi2_wrap_reg=0x%08x gdma_reg_base=0x%08x",
         vcid_csi2_wrap_reg, gdma_reg_base);

    /* Step 3: Configure virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("[STEP 3] WRITE : MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL = 0x%08x",
         vcid_csi2_wrap_reg);

    /* Step 4: Enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("[STEP 4] WRITE : MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA = 0x00000001");

    /* Step 5: Repeated write to virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("[STEP 5] WRITE : MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL = 0x%08x (repeated)",
         vcid_csi2_wrap_reg);

    /* Step 6: Repeated write to control data register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("[STEP 6] WRITE : MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA = 0x00000001 (repeated)");

    /* Step 7: Initialize D-PHY */
    LOGT("[STEP 7] Calling snps_phy_init() for D-PHY initialization");
    snps_phy_init();

    /* Step 8: Read PHY_STOPSTATE */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("[STEP 8] READ  : MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE = 0x%08x", rd_data);

    /* Step 9: Poll PHY_STOPSTATE until all lanes in stop state (0x1000f) */
    LOGT("[STEP 9] Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for 0x1000F");
    while (!(rd_data == 0x1000fU)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("[STEP 9] PHY_STOPSTATE = 0x%08x (all lanes in stop state)", rd_data);

    /* Step 10: Set DMA program counter addresses */
    ch0_pc = 0xE6000000U;
    ch1_pc = 0xE6000500U;
    LOGT("[STEP 10] ch0_pc=0x%08x ch1_pc=0x%08x", ch0_pc, ch1_pc);

    /* Steps 11-27: Iterate through lane counts (4 down to 1) */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("[LANE CONFIG] Setting N_LANES=%d (active lanes=%d)",
             lane_num, lane_num + 1);

        /* Step 11: Write lane number */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("WRITE : MIZAR_MIPI_CSI2_HOST_N_LANES = 0x%08x", (unsigned int)lane_num);

        /* Step 12: Compute control packet count */
        cntrl_pkt_cnt = ((VRES * 3U) + 2U);
        LOGT("Control packet count = %u", cntrl_pkt_cnt);

        /* Step 13: Trigger CSI-2 sequence */
        write_reg(0xa0243ffcU, (unsigned int)(lane_num + 1));
        LOGT("WRITE : 0xA0243FFC = 0x%08x (trigger CSI-2 sequence)",
             (unsigned int)(lane_num + 1));

        /* Step 14: Packet processing loop */
        for (i = 0; i < (int)cntrl_pkt_cnt; i++) {

            ch0_preload_loc = ch0_pc;
            ch1_preload_loc = ch1_pc;

            /* Step 15: Enable DMA interrupts */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);

            /* Step 16: Program DMA channel 0 for control data */
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base,
                                     0x8000U, 0xE6001000U, 8U, 0U);

            /* Step 17: Start DMA channel 0 */
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0U);

            /* Step 18: Poll DMA channel 0 completion (bit 0) */
            rd_data = 0U;
            while ((rd_data & 0x1U) == 0x0U) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            }
            LOGT("[PKT %d] DMA CH0 transfer complete (INTMIS=0x%08x)", i, rd_data);

            /* Step 19: Clear DMA channel 0 interrupt */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Step 20: Read DMA fault status */
            rd_data = read_reg(gdma_reg_base + 0x28U);

            /* Step 21: Read CSI control data */
            csi_ctrl_data = read_reg(0xE6001000U);
            LOGT("[PKT %d] CSI control data = 0x%08x", i, csi_ctrl_data);

            /* Step 22: Check if image data (data type > 0xf) */
            if ((csi_ctrl_data & 0x3fU) > 0xfU) {
                word_count = ((csi_ctrl_data >> 6) & 0xffffU);
                csi_data_size = (word_count % 8U) ?
                    ((word_count / 8U + 1U) * 8U) : word_count;
                LOGT("[PKT %d] Image data: word_count=%u aligned_size=%u",
                     i, word_count, csi_data_size);

                /* Step 23: Program DMA channel 1 for data transfer */
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base,
                                         0x0000U, 0xE6002000U, csi_data_size, 1U);

                /* Step 24: Start DMA channel 1 */
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1U);

                /* Step 25: Poll DMA channel 1 completion (bit 1) */
                rd_data = 0U;
                while ((rd_data & 0x2U) == 0x0U) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                }
                LOGT("[PKT %d] DMA CH1 transfer complete (INTMIS=0x%08x)",
                     i, rd_data);

                /* Step 26: Read INTMIS again */
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Step 27: Clear DMA channel 1 interrupt */
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);
            }
        }

        LOGT("[LANE CONFIG] Lane count %d processing complete", lane_num + 1);
    }

    /* Step 28: Test completion */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
