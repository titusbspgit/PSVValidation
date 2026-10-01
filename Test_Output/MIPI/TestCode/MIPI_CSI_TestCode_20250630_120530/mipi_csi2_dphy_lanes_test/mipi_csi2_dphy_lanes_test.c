// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Description: Validates the MIPI CSI-2 receiver across all D-PHY lane
 *              configurations (4 lanes down to 1 lane). Enables CSI-2 host
 *              interrupt masks, configures virtual channel, enables control
 *              data transfer, initializes D-PHY, and iterates through lane
 *              configurations with DMA-based control and data packet reception.
 */

typedef struct {
    unsigned int gdma_reg_base;
    unsigned int gdma_path;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Reads INT_ST_MAIN to clear pending interrupts, then writes all
 *              CSI-2 host interrupt mask registers.
 */
static void csi2_enable_interrupt(void)
{
    volatile unsigned int rd_data;

    LOGT("csi2_enable_interrupt: clearing pending interrupts");

    /* Step 3: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("Read INT_ST_MAIN = 0x%lx", (unsigned long)rd_data);

    /* Step 4: Write 0x0000000f to INT_MSK_PHY_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("Write INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 5: Write 0x00000003 to INT_MSK_PKT_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("Write INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 6: Write 0x000f000f to INT_MSK_PHY */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("Write INT_MSK_PHY = 0x000f000f");

    /* Step 7: Write 0x000f000f to INT_MSK_LINE */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("Write INT_MSK_LINE = 0x000f000f");

    /* Step 8: Write 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 9: Write 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 10: Write 0x0000ffff to INT_MSK_CRC_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 11: Write 0x0000ffff to INT_MSK_PLD_CRC_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 12: Write 0x0000ffff to INT_MSK_DATA_ID */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("Write INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 13: Write 0x0000ffff to INT_MSK_ECC_CORRECTED */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("Write INT_MSK_ECC_CORRECTED = 0x0000ffff");

    LOGT("csi2_enable_interrupt: all interrupt masks configured");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_dphy_lanes_test. Enables CSI-2 host interrupts,
 *              configures virtual channel, enables control data transfer,
 *              and initializes D-PHY.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    volatile unsigned int rd_data;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test_init: start");

    /* Step 2: Call csi2_enable_interrupt */
    csi2_enable_interrupt();

    /* Step 15-16: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = (VC_ID << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 0U;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (VC_ID << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 1U;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (VC_ID << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 2U;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf) << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (VC_ID << 12);
    g_ctx.gdma_path = 3U;
#else
    /* Default to GDMA0 path */
    g_ctx.vcid_csi2_wrap_reg = (VC_ID << 0) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
    g_ctx.gdma_path = 0U;
#endif

    /* Step 17: Set gdma_reg_base */
    g_ctx.gdma_reg_base = MIPI_CSI2_GDMA_REG_BASE;
    LOGT("gdma_path=%u gdma_reg_base=0x%lx vcid_csi2_wrap_reg=0x%lx",
         g_ctx.gdma_path,
         (unsigned long)g_ctx.gdma_reg_base,
         (unsigned long)g_ctx.vcid_csi2_wrap_reg);

    /* Step 18: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL = 0x%lx", (unsigned long)g_ctx.vcid_csi2_wrap_reg);

    /* Step 19: Write 1 to CONTROL_DATA to enable control data transfer */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("Write CONTROL_DATA = 0x1");

    /* Step 20: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL again (repeated) */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL = 0x%lx (repeated)", (unsigned long)g_ctx.vcid_csi2_wrap_reg);

    /* Step 21: Write 1 to CONTROL_DATA again (repeated) */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("Write CONTROL_DATA = 0x1 (repeated)");

    /* Step 22: Initialize D-PHY */
    LOGT("Calling snps_phy_init()");
    snps_phy_init();
    LOGT("snps_phy_init() complete");

    /* Step 23-24: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("Polling PHY_STOPSTATE for 0x%lx", (unsigned long)PHY_STOPSTATE_EXPECTED);
    rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    timeout = MIPI_CSI2_POLL_TIMEOUT;

    while ((rd_data != PHY_STOPSTATE_EXPECTED) && (timeout > 0U)) {
        rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (rd_data != PHY_STOPSTATE_EXPECTED) {
        LOGE("PHY_STOPSTATE timeout: expected=0x%lx actual=0x%lx",
             (unsigned long)PHY_STOPSTATE_EXPECTED,
             (unsigned long)rd_data);
        g_ctx.errors++;
        return -1;
    }

    LOGT("PHY_STOPSTATE = 0x%lx — all lanes in stop state", (unsigned long)rd_data);
    LOGT("mipi_csi2_dphy_lanes_test_init: complete");

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 *              Iterates lane configurations from 4 down to 1, performing
 *              DMA-based control and data packet reception for each.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    volatile unsigned int rd_data;
    volatile unsigned int csi_ctrl_data;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    unsigned int cntrl_pkt_cnt;
    unsigned int word_count;
    unsigned int csi_data_size;
    int lane_num;
    unsigned int i;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: start");

    /* Step 25: Set DMA program counter addresses */
    ch0_pc = MIPI_CSI2_CH0_PC;
    ch1_pc = MIPI_CSI2_CH1_PC;

    /* Step 26: Outer for-loop — lane_num from 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("Lane configuration: %d lanes (lane_num=%d)", lane_num + 1, lane_num);

        /* Step 27: Write lane_num to N_LANES */
        writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("Write N_LANES = 0x%x", (unsigned int)lane_num);

        /* Step 28: Compute control packet count */
        cntrl_pkt_cnt = (VRES * 3U) + 2U;
        LOGT("cntrl_pkt_cnt = %u", cntrl_pkt_cnt);

        /* Step 29: Write (lane_num + 1) to trigger address */
        writel_reg((uintptr_t)MIPI_CSI2_TRIGGER_ADDR, (unsigned int)(lane_num + 1));
        LOGT("Write trigger 0x%lx = 0x%x",
             (unsigned long)MIPI_CSI2_TRIGGER_ADDR, (unsigned int)(lane_num + 1));

        /* Step 30: Inner for-loop — iterate over control packets */
        for (i = 0U; i < cntrl_pkt_cnt; i++) {

            /* Step 31: Set preload locations */
            ch0_preload_loc = ch0_pc;
            ch1_preload_loc = ch1_pc;

            /* Step 32: Enable DMA interrupts for both channels */
            writel_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET), 0x3U);

            /* Step 33: Preload DMA channel 0 for control data transfer */
            dma_trnsfr_instn_preload(ch0_preload_loc, 0x8000U,
                                    GDMA_CTRL_DATA_DEST_ADDR2, 8U, 0U);

            /* Step 34: Start DMA channel 0 */
            DMAGO_CSI(0U, ch0_pc, g_ctx.gdma_reg_base);

            /* Step 35-36: Poll DMA INTMIS until bit 0 is set */
            rd_data = 0U;
            timeout = MIPI_CSI2_POLL_TIMEOUT;
            while (((rd_data & 0x1U) == 0U) && (timeout > 0U)) {
                rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET));
                timeout--;
            }

            if ((rd_data & 0x1U) == 0U) {
                LOGE("DMA ch0 INTMIS timeout: lane_num=%d pkt=%u rd_data=0x%lx",
                     lane_num, i, (unsigned long)rd_data);
                g_ctx.errors++;
                out->status = -1;
                return out->status;
            }

            /* Step 37: Clear DMA channel 0 interrupt */
            writel_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET), 0x1U);

            /* Step 38: Read debug status */
            rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_DEBUG_OFFSET));

            /* Step 39: Read control data */
            csi_ctrl_data = readl_reg((uintptr_t)GDMA_CTRL_DATA_DEST_ADDR2);

            /* Step 40: Check if long packet (data type > 0xf) */
            if ((csi_ctrl_data & 0x3fU) > 0xfU) {

                /* Step 41: Extract word_count from bits[21:6] */
                word_count = (csi_ctrl_data >> 6) & 0xFFFFU;

                /* Step 42: Compute 8-byte aligned data size */
                csi_data_size = (word_count + 7U) & ~7U;

                LOGT("Long packet: word_count=%u aligned_size=%u",
                     word_count, csi_data_size);

                /* Step 43: Preload DMA channel 1 for data transfer */
                dma_trnsfr_instn_preload(ch1_preload_loc, 0x0000U,
                                        GDMA_CSI2_DATA_DEST_ADDR2,
                                        csi_data_size, 1U);

                /* Step 44: Start DMA channel 1 */
                DMAGO_CSI(1U, ch1_pc, g_ctx.gdma_reg_base);

                /* Step 45-46: Poll DMA INTMIS until bit 1 is set */
                rd_data = 0U;
                timeout = MIPI_CSI2_POLL_TIMEOUT;
                while (((rd_data & 0x2U) == 0U) && (timeout > 0U)) {
                    rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET));
                    timeout--;
                }

                if ((rd_data & 0x2U) == 0U) {
                    LOGE("DMA ch1 INTMIS timeout: lane_num=%d pkt=%u rd_data=0x%lx",
                         lane_num, i, (unsigned long)rd_data);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }

                /* Step 47: Read INTMIS again */
                rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET));

                /* Step 48: Clear DMA channel 1 interrupt */
                writel_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET), 0x2U);

            } /* End of long-packet conditional (Step 49) */

        } /* End of inner for-loop (Step 50) */

        LOGT("Lane configuration %d lanes completed", lane_num + 1);

    } /* End of outer for-loop (Step 51) */

    /* Step 52: DV finish(0) converted to PSV/FV status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test_run: %s errors=%u",
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
