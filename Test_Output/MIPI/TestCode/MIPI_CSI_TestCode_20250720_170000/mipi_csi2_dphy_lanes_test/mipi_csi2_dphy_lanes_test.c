// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * mipi_csi2_dphy_lanes_test
 * Validates MIPI CSI-2 DPHY lane configuration by iterating through lane
 * counts from 4 down to 1. Enables all CSI-2 host interrupts, configures
 * virtual channel and control data, initializes D-PHY, polls PHY stop state,
 * then performs DMA transfers for control and data packets per lane config.
 */

/* Testcase context structure */
typedef struct {
    uintptr_t gdma_reg_base;
    unsigned int errors;
    unsigned int lanes_completed;
} mipi_csi2_dphy_lanes_ctx_t;

static mipi_csi2_dphy_lanes_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Enables all CSI-2 host interrupt masks.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void csi2_enable_interrupt(void)
{
    /* Step 1: Read INT_ST_MAIN to clear pending interrupts */
    (void)readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("CSI2: Cleared pending interrupts via INT_ST_MAIN read");

    /* Step 2: Write INT_MSK_PHY_FATAL with 0x0000000f */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("CSI2: INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 3: Write INT_MSK_PKT_FATAL with 0x00000003 */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("CSI2: INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 4: Write INT_MSK_PHY with 0x000f000f */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("CSI2: INT_MSK_PHY = 0x000f000f");

    /* Step 5: Write INT_MSK_LINE with 0x000f000f */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("CSI2: INT_MSK_LINE = 0x000f000f");

    /* Step 6: Write INT_MSK_BNDRY_FRAME_FATAL with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 7: Write INT_MSK_SEQ_FRAME_FATAL with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 8: Write INT_MSK_CRC_FRAME_FATAL with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 9: Write INT_MSK_PLD_CRC_FATAL with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 10: Write INT_MSK_DATA_ID with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 11: Write INT_MSK_ECC_CORRECTED with 0x0000ffff */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("CSI2: INT_MSK_ECC_CORRECTED = 0x0000ffff");
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
    uint32_t reg_val;
    uint32_t vcid_csi2_wrap_reg;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_ctx_t){0};
    g_ctx.gdma_reg_base = (uintptr_t)MIPI_CSI2_GDMA_REG_BASE;

    LOGT("mipi_csi2_dphy_lanes_test init: starting");

    /* Step 1-11: Enable all CSI-2 host interrupts */
    csi2_enable_interrupt();

    /* Step 12: Determine vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = ((uint32_t)VC_ID << 0);
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = ((uint32_t)VC_ID << 4);
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = ((uint32_t)VC_ID << 8);
#elif defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = ((uint32_t)VC_ID << 12);
#else
    vcid_csi2_wrap_reg = ((uint32_t)VC_ID << 0);
#endif
    LOGT("CSI2: vcid_csi2_wrap_reg = 0x%lx", (unsigned long)vcid_csi2_wrap_reg);

    /* Step 13: Set gdma_reg_base */
    LOGT("CSI2: gdma_reg_base = 0x%lx", (unsigned long)g_ctx.gdma_reg_base);

    /* Step 14: Write virtual channel register */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("CSI2: VIRTUAL_CHANNEL = 0x%lx", (unsigned long)vcid_csi2_wrap_reg);

    /* Step 15: Enable control data transfer */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("CSI2: CONTROL_DATA = 1");

    /* Step 16-17: Repeat writes for virtual channel and control data */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);

    /* Step 18: Initialize D-PHY */
    snps_phy_init();
    LOGT("CSI2: D-PHY initialized via snps_phy_init()");

    /* Step 19: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("CSI2: Polling PHY_STOPSTATE for 0x1000f");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) && (timeout > 0U)) {
        reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) {
        LOGE("CSI2: PHY_STOPSTATE timeout, read=0x%lx expected=0x%lx",
             (unsigned long)reg_val,
             (unsigned long)MIPI_CSI2_PHY_STOPSTATE_EXPECTED);
        g_ctx.errors++;
        return -1;
    }

    LOGT("CSI2: PHY_STOPSTATE = 0x%lx (all lanes stopped)", (unsigned long)reg_val);
    LOGT("mipi_csi2_dphy_lanes_test init: complete");

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
    int lane_num;
    uint32_t dma_status;
    uint32_t csi_ctrl_data;
    uint32_t data_type;
    uint32_t word_count;
    unsigned int pkt;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("CSI2: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test run: starting lane loop");

    /* Steps 20-33: Lane loop from lane_num=3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        /* Configure N_LANES with current lane count */
        writel_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (uint32_t)lane_num);
        LOGT("CSI2: N_LANES = %d (lane config %d)", lane_num, (lane_num + 1));

        /* Trigger CSI-2 sequence for current lane configuration */
        writel_reg((uintptr_t)MIPI_CSI2_SEQ_TRIGGER_ADDR, (uint32_t)(lane_num + 1));
        LOGT("CSI2: Triggered CSI-2 sequence at 0x%lx with %d",
             (unsigned long)MIPI_CSI2_SEQ_TRIGGER_ADDR, (lane_num + 1));

        /* Process packets for this lane configuration */
        for (pkt = 0U; pkt < TOTAL_FRAME; pkt++) {

            /* Enable DMA interrupts, program and start DMA for control data on ch0 */
            // MANUAL_REVIEW: DMA channel 0 programming for control data transfer
            // requires platform-specific DMA setup APIs not fully specified in the
            // Meta TestPlan JSON. The DMA programming sequence should be implemented
            // per the platform DMA driver interface.

            /* Poll DMA interrupt status for channel 0 completion */
            timeout = MIPI_CSI2_POLL_TIMEOUT;
            dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            while (((dma_status & 0x01U) == 0U) && (timeout > 0U)) {
                dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
            }

            if ((dma_status & 0x01U) == 0U) {
                LOGE("CSI2: DMA ch0 timeout, lane=%d pkt=%u status=0x%lx",
                     lane_num, pkt, (unsigned long)dma_status);
                g_ctx.errors++;
                continue;
            }

            /* Clear DMA interrupt for channel 0 */
            writel_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x01U);
            LOGT("CSI2: DMA ch0 complete, lane=%d pkt=%u", lane_num, pkt);

            /* Read control data to determine packet type and word count */
            csi_ctrl_data = readl_reg((uintptr_t)MIPI_CSI2_CTRL_DATA_ADDR);
            data_type = (csi_ctrl_data >> 0U) & 0x3FU;
            word_count = (csi_ctrl_data >> 8U) & 0xFFFFU;
            LOGT("CSI2: ctrl_data=0x%lx data_type=0x%lx word_count=%lu",
                 (unsigned long)csi_ctrl_data,
                 (unsigned long)data_type,
                 (unsigned long)word_count);

            /* If long packet (data_type > 0xf), perform data DMA on channel 1 */
            if (data_type > 0x0FU) {

                // MANUAL_REVIEW: DMA channel 1 programming for pixel data transfer
                // requires platform-specific DMA setup APIs not fully specified in the
                // Meta TestPlan JSON. word_count is used for transfer size (8-byte aligned).

                /* Poll DMA interrupt status for channel 1 completion */
                timeout = MIPI_CSI2_POLL_TIMEOUT;
                dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                while (((dma_status & 0x02U) == 0U) && (timeout > 0U)) {
                    dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                }

                if ((dma_status & 0x02U) == 0U) {
                    LOGE("CSI2: DMA ch1 timeout, lane=%d pkt=%u status=0x%lx",
                         lane_num, pkt, (unsigned long)dma_status);
                    g_ctx.errors++;
                    continue;
                }

                /* Clear DMA interrupt for channel 1 */
                writel_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x02U);
                LOGT("CSI2: DMA ch1 complete, lane=%d pkt=%u wc=%lu",
                     lane_num, pkt, (unsigned long)word_count);
            }
        }

        g_ctx.lanes_completed++;
        LOGT("CSI2: Lane config %d completed", (lane_num + 1));
    }

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test run complete: %s errors=%u lanes_completed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.lanes_completed);

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

    /* Validate all 4 lane configurations were processed */
    if (g_ctx.lanes_completed != MIPI_CSI2_TOTAL_LANE_CONFIGS) {
        LOGE("CSI2: Not all lane configs completed: expected=%u actual=%u",
             MIPI_CSI2_TOTAL_LANE_CONFIGS,
             g_ctx.lanes_completed);
    }

    // MANUAL_REVIEW: DV finish(0) was present in the source flow, converted to
    // PSV/FV-native status reporting via out->status in the run function.

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u lanes_completed=%u",
         g_ctx.errors, g_ctx.lanes_completed);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
