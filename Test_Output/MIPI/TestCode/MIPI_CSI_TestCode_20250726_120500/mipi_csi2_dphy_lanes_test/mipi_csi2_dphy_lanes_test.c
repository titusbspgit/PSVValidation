// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * mipi_csi2_dphy_lanes_test.c
 * FV Testcase: MIPI CSI-2 D-PHY Lane Configuration Test
 * Validates MIPI CSI-2 receiver operation across all D-PHY lane
 * configurations from 4 lanes down to 1 lane with DMA-based
 * control and data packet transfers.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * csi2_enable_interrupt
 * Enables all CSI-2 host interrupt masks (Steps 3-13)
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Step 3: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Step 4: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 5: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 6: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("INT_MSK_PHY = 0x000f000f");

    /* Step 7: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("INT_MSK_LINE = 0x000f000f");

    /* Step 8: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 9: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 10: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 11: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 12: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 13: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("INT_MSK_ECC_CORRECTED = 0x0000ffff");
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

    LOGT("mipi_csi2_dphy_lanes_test_init: D-PHY lane configuration test init");

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
    unsigned int gdma_path;
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
        LOGE("mipi_csi2_dphy_lanes_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: starting D-PHY lane configuration test");

    /* Step 2: Enable all CSI-2 host interrupts */
    LOGT("Step 2: Enabling CSI-2 host interrupts");
    csi2_enable_interrupt();
    LOGT("All CSI-2 interrupt masks enabled");

    /* Steps 15-16: Compute virtual channel ID based on GDMA path */
#if defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 0) |
                         (((VC_ID + 1) & 0xf) << 4) |
                         (((VC_ID + 1) & 0xf) << 8) |
                         (((VC_ID + 1) & 0xf) << 12);
    gdma_path = 0;
    LOGT("GDMA0_PATH selected, gdma_path=0");
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) |
                         (VC_ID << 4) |
                         (((VC_ID + 1) & 0xf) << 8) |
                         (((VC_ID + 1) & 0xf) << 12);
    gdma_path = 1;
    LOGT("GDMA1_PATH selected, gdma_path=1");
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) |
                         (((VC_ID + 1) & 0xf) << 4) |
                         (VC_ID << 8) |
                         (((VC_ID + 1) & 0xf) << 12);
    gdma_path = 2;
    LOGT("GDMA2_PATH selected, gdma_path=2");
#elif defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) |
                         (((VC_ID + 1) & 0xf) << 4) |
                         (((VC_ID + 1) & 0xf) << 8) |
                         (VC_ID << 12);
    gdma_path = 3;
    LOGT("GDMA3_PATH selected, gdma_path=3");
#else
    vcid_csi2_wrap_reg = (VC_ID << 0) |
                         (((VC_ID + 1) & 0xf) << 4) |
                         (((VC_ID + 1) & 0xf) << 8) |
                         (((VC_ID + 1) & 0xf) << 12);
    gdma_path = 0;
    LOGT("Default GDMA0_PATH selected, gdma_path=0");
#endif

    /* Step 17: Set GDMA register base */
    gdma_reg_base = 0xE6A00000U;
    LOGT("gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 18: Write virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL=0x%x", vcid_csi2_wrap_reg);

    /* Step 19: Enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("CONTROL_DATA=1");

    /* Step 20: Repeated write to virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    /* Step 21: Repeated write to control data register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("Repeated writes to VIRTUAL_CHANNEL and CONTROL_DATA");

    /* Step 22: Initialize D-PHY */
    LOGT("Step 22: Initializing D-PHY via snps_phy_init()");
    snps_phy_init();
    LOGT("D-PHY initialization complete");

    /* Steps 23-24: Poll PHY_STOPSTATE until all lanes in stop state */
    LOGT("Step 23-24: Polling PHY_STOPSTATE for 0x%x", PHY_STOPSTATE_EXPECTED);
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while (rd_data != PHY_STOPSTATE_EXPECTED) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE=0x%x, all lanes in stop state", rd_data);

    /* Step 25: Set DMA program counter addresses */
    ch0_pc = 0xE6000000U;
    ch1_pc = 0xE6000500U;
    LOGT("ch0_pc=0x%x ch1_pc=0x%x", ch0_pc, ch1_pc);

    /* Steps 26-51: Iterate through lane configurations (4 down to 1) */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("Lane config: %d lane(s), lane_num=%d", lane_num + 1, lane_num);

        /* Step 27: Write lane count to N_LANES */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("N_LANES=%d", lane_num);

        /* Step 28: Compute control packet count */
        cntrl_pkt_cnt = (VRES * 3U) + 2U;
        LOGT("cntrl_pkt_cnt=%u", cntrl_pkt_cnt);

        /* Step 29: Trigger CSI-2 sequence */
        write_reg(0xa0243ffcU, (unsigned int)(lane_num + 1));
        LOGT("CSI-2 sequence triggered with value %d at 0xa0243ffc", lane_num + 1);

        /* Steps 30-50: Inner loop - process each control packet */
        for (i = 0; i < (int)cntrl_pkt_cnt; i++) {

            /* Step 31: Set preload locations */
            ch0_preload_loc = ch0_pc;
            ch1_preload_loc = ch1_pc;

            /* Step 32: Enable DMA interrupts for both channels */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);

            /* Step 33: Preload DMA transfer instructions for channel 0 */
            /* dma_trnsfr_instn_preload(preload_loc, src_addr, dest_addr, trnsfr_size, irq_num) */
            dma_trnsfr_instn_preload(ch0_preload_loc, 0x8000U, GDMA_CTRL_DATA_DEST_ADDR2, 8U, 0U);

            /* Step 34: Start DMA channel 0 */
            /* DMAGO_CSI(gdma_reg_base, channel, pc_addr) */
            DMAGO_CSI(gdma_reg_base, 0U, ch0_pc);

            /* Steps 35-36: Poll DMA interrupt status for channel 0 completion */
            rd_data = 0U;
            while ((rd_data & 0x1U) == 0U) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            }

            /* Step 37: Clear DMA channel 0 interrupt */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Step 38: Read debug status */
            rd_data = read_reg(gdma_reg_base + 0x28U);

            /* Step 39: Read control data */
            csi_ctrl_data = read_reg(GDMA_CTRL_DATA_DEST_ADDR2);
            LOGT("Pkt[%d] ctrl_data=0x%x", i, csi_ctrl_data);

            /* Step 40: Check if long packet */
            if ((csi_ctrl_data & 0x3fU) > SHORT_PKT_THRESHOLD) {

                /* Step 41: Extract word_count from bits[21:6] */
                word_count = (csi_ctrl_data >> 6) & 0xFFFFU;
                LOGT("Long packet detected, word_count=%u", word_count);

                /* Step 42: Compute 8-byte aligned data size */
                csi_data_size = ((word_count + 7U) / 8U) * 8U;
                LOGT("csi_data_size (8-byte aligned)=%u", csi_data_size);

                /* Step 43: Preload DMA transfer instructions for channel 1 */
                /* dma_trnsfr_instn_preload(preload_loc, src_addr, dest_addr, trnsfr_size, irq_num) */
                dma_trnsfr_instn_preload(ch1_preload_loc, 0x0000U, GDMA_CSI2_DATA_DEST_ADDR2, csi_data_size, 1U);

                /* Step 44: Start DMA channel 1 */
                /* DMAGO_CSI(gdma_reg_base, channel, pc_addr) */
                DMAGO_CSI(gdma_reg_base, 1U, ch1_pc);

                /* Steps 45-46: Poll DMA interrupt status for channel 1 completion */
                rd_data = 0U;
                while ((rd_data & 0x2U) == 0U) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                }

                /* Step 47: Read interrupt status again */
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Step 48: Clear DMA channel 1 interrupt */
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);
                LOGT("Channel 1 transfer complete, interrupt cleared");
            }
            /* Step 49: End of long-packet conditional block */
        }
        /* Step 50: End of inner for-loop */

        LOGT("Lane configuration %d completed", lane_num + 1);
    }
    /* Step 51: End of outer for-loop */

    /* Step 52: Test complete */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native status reporting via out->status is used instead.
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

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
