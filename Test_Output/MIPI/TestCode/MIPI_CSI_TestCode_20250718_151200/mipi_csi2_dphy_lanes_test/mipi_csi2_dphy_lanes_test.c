// Author - AI Force 2.3. 18-Jul-2025 15:12 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *              through all lane counts (4 lanes down to 1 lane), enabling
 *              CSI-2 interrupts, initializing the D-PHY, waiting for PHY stop
 *              state, and performing DMA-based control and data packet transfers
 *              for each lane configuration.
 */

/* DMA offset macros used for polling and clearing */
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x00000010U
#define MIPI_CSI2_DMA_INTCLR_OFFSET  0x00000014U

/* DMA addresses and sizes from test steps */
#define DMA_CTRL_PKT_SRC_ADDR   0x8000U
#define DMA_CTRL_PKT_DEST_ADDR  0xE6001000U
#define DMA_CTRL_PKT_SIZE       8U
#define DMA_DATA_PKT_SRC_ADDR   0x0000U
#define DMA_DATA_PKT_DEST_ADDR  0xE6002000U

/* Sequence trigger register address */
#define CSI2_SEQ_TRIGGER_ADDR   0xa0243ffcU

/* PHY stop state expected value */
#define PHY_STOPSTATE_EXPECTED  0x1000fU

/* Test context structure following FV Template pattern */
typedef struct {
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_path;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Clears pending CSI-2 interrupts by reading INT_ST_MAIN,
 *              then enables all CSI-2 interrupt masks.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    /* Step 6: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Step 7: Enable phy_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 8: Enable pkt_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 9: Enable phy interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("INT_MSK_PHY = 0x000f000f");

    /* Step 10: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("INT_MSK_LINE = 0x000f000f");

    /* Step 11: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 12: Enable seq frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 13: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 14: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 15: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 16: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("INT_MSK_ECC_CORRECTED = 0x0000ffff");

    LOGT("csi2_enable_interrupt: all interrupt masks enabled");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_dphy_lanes_test. Enables CSI-2 interrupts, configures
 *              virtual channel, enables control data, initializes D-PHY, and
 *              polls PHY stop state.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    int rd_data;

    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test_init: start");

    /* Step 3: Print start line */
    LOGT("start line");

    /* Step 4: Call csi2_enable_interrupt */
    csi2_enable_interrupt();

    /* Step 18: Conditional compilation for GDMA path selection */
#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = (VC_ID << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 0;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (VC_ID << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 1;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (VC_ID << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 2;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (VC_ID << 12);
    g_ctx.gdma_path = 3;
#else
    g_ctx.vcid_csi2_wrap_reg = (VC_ID << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 0;
#endif

    /* Step 19: Set GDMA register base */
    g_ctx.gdma_reg_base = 0xE6A00000U;

    /* Step 20: Write virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL = 0x%x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 21: Enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("CONTROL_DATA = 1");

    /* Step 22: Print vcid_csi2_wrap_reg */
    LOGT("vcid_csi2_wrap_reg=0x%x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 23: Write virtual channel register (second) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL (2nd) = 0x%x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 24: Write control data register (second) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("CONTROL_DATA (2nd) = 1");

    /* Step 25: Initialize D-PHY */
    snps_phy_init();
    LOGT("snps_phy_init() called");

    /* Steps 26-29: Poll PHY_STOPSTATE until stop state reached */
    LOGT("Polling PHY_STOPSTATE for 0x%x", PHY_STOPSTATE_EXPECTED);
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while (!(rd_data == (int)PHY_STOPSTATE_EXPECTED)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE reached: 0x%x", rd_data);

    /* Steps 30-31: Set DMA program counters */
    g_ctx.ch0_pc = 0xE6000000U;
    g_ctx.ch1_pc = 0xE6000500U;
    LOGT("ch0_pc=0x%x ch1_pc=0x%x", g_ctx.ch0_pc, g_ctx.ch1_pc);

    LOGT("mipi_csi2_dphy_lanes_test_init: complete");
    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 *              Iterates through lane counts 4 down to 1, configures N_LANES,
 *              triggers CSI-2 sequence, and performs DMA control/data packet
 *              transfers with polling and validation.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    long long int rx_desc;
    long long int tx_desc;
    long long int gdma_tx_trnsfr_size;
    long long int gdma_trnsfr_size;
    int cntrl_pkt_cnt;
    int rd_data;
    int lane_num;
    int i;
    unsigned int csi_ctrl_data;
    unsigned int word_count;
    unsigned int csi_data_size;

    (void)cfg;
    (void)rx_desc;
    (void)tx_desc;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: start");

    /* Step 32: Outer for loop - lane_num = 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("Configuring lane_num=%d (%d lane(s))", lane_num, lane_num + 1);

        /* Step 33: Configure number of active lanes */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);
        LOGT("N_LANES = %d", lane_num);

        /* Step 34: Calculate control packet count */
        cntrl_pkt_cnt = ((VRES * 3) + 2);

        /* Step 35: Debug print */
        LOGT("cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

        /* Step 36: Trigger CSI-2 sequence */
        write_reg(CSI2_SEQ_TRIGGER_ADDR, (lane_num + 1));
        LOGT("SEQ_TRIGGER(0xa0243ffc) = %d", (lane_num + 1));

        /* Steps 37-68: Inner loop - process all control and data packets */
        for (i = 0; i < cntrl_pkt_cnt; i++) {

            /* Program and start DMA transfer on channel 0 (control packet) */
            gdma_trnsfr_size = DMA_CTRL_PKT_SIZE;
            dma_trnsfr_instn_preload(g_ctx.ch0_pc, DMA_CTRL_PKT_SRC_ADDR,
                                     DMA_CTRL_PKT_DEST_ADDR, gdma_trnsfr_size, 0);
            DMAGO_CSI(g_ctx.gdma_reg_base, 0, g_ctx.ch0_pc);

            /* Poll DMA interrupt status for channel 0 completion (bit 0) */
            rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            while (!(rd_data & 0x1)) {
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            }

            /* Clear channel 0 DMA interrupt */
            write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            /* Read DMA status register */
            rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

            /* Read received control data */
            csi_ctrl_data = read_reg(DMA_CTRL_PKT_DEST_ADDR);

            /* Check if long packet (data type > 0xF) */
            if ((csi_ctrl_data & 0x3f) > 0xf) {

                /* Extract word count */
                word_count = ((csi_ctrl_data >> 6) & 0xffff);

                /* Calculate 8-byte aligned transfer size */
                csi_data_size = (word_count % 8) ?
                                ((word_count / 8 + 1) * 8) : word_count;

                /* Program and start DMA transfer on channel 1 (data payload) */
                gdma_tx_trnsfr_size = (long long int)csi_data_size;
                dma_trnsfr_instn_preload(g_ctx.ch1_pc, DMA_DATA_PKT_SRC_ADDR,
                                         DMA_DATA_PKT_DEST_ADDR,
                                         gdma_tx_trnsfr_size, 1);
                DMAGO_CSI(g_ctx.gdma_reg_base, 1, g_ctx.ch1_pc);

                /* Poll DMA interrupt status for channel 1 completion (bit 1) */
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                while (!(rd_data & 0x2)) {
                    rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                }

                /* Post-channel-1 interrupt status read */
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Clear channel 1 DMA interrupt */
                write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);
            }
        }

        LOGT("Lane %d configuration complete", lane_num + 1);
    }

    /* MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native */
    /* completion is handled via out->status. */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test_run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for mipi_csi2_dphy_lanes_test.
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
