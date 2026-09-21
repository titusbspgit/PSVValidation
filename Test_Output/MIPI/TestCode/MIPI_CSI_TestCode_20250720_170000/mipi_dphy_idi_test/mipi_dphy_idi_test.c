// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dphy_idi_test.h"
#include "test_define.inc"

/*
 * mipi_dphy_idi_test
 * Validates MIPI CSI-2 DPHY IDI data transfer across all 16 virtual channel
 * IDs (0 through 15). Initializes D-PHY, enables interrupts, polls PHY stop
 * state, enables control data and null/blanking transfers, then iterates
 * through all VCIDs with 129 packets each performing DMA transfers.
 */

/* Testcase context structure */
typedef struct {
    uintptr_t gdma_reg_base;
    unsigned int errors;
    unsigned int vcids_completed;
} mipi_dphy_idi_ctx_t;

static mipi_dphy_idi_ctx_t g_ctx;

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
    /* Read INT_ST_MAIN to clear pending interrupts */
    (void)readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("IDI: Cleared pending interrupts via INT_ST_MAIN read");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("IDI: INT_MSK_PHY_FATAL = 0x0000000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("IDI: INT_MSK_PKT_FATAL = 0x00000003");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("IDI: INT_MSK_PHY = 0x000f000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("IDI: INT_MSK_LINE = 0x000f000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("IDI: INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("IDI: INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("IDI: INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("IDI: INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("IDI: INT_MSK_DATA_ID = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("IDI: INT_MSK_ECC_CORRECTED = 0x0000ffff");
}

/*
 * Function: mipi_dphy_idi_test_init
 * Description: Performs testcase initialization for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_init(const TestsItem *cfg)
{
    uint32_t reg_val;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (mipi_dphy_idi_ctx_t){0};
    g_ctx.gdma_reg_base = (uintptr_t)MIPI_CSI2_GDMA_REG_BASE;

    LOGT("mipi_dphy_idi_test init: starting");

    /* Step 1: Initialize D-PHY */
    snps_phy_init();
    LOGT("IDI: D-PHY initialized via snps_phy_init()");

    /* Steps 2-12: Enable all CSI-2 host interrupts */
    csi2_enable_interrupt();

    /* Step 13: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("IDI: Polling PHY_STOPSTATE for 0x1000f");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) && (timeout > 0U)) {
        reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) {
        LOGE("IDI: PHY_STOPSTATE timeout, read=0x%lx expected=0x%lx",
             (unsigned long)reg_val,
             (unsigned long)MIPI_CSI2_PHY_STOPSTATE_EXPECTED);
        g_ctx.errors++;
        return -1;
    }

    LOGT("IDI: PHY_STOPSTATE = 0x%lx (all lanes stopped)", (unsigned long)reg_val);

    /* Steps 14-15: Enable control data and null/blanking transfers */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("IDI: CONTROL_DATA = 1 (enabled)");

    writel_reg(MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK, 1U);
    LOGT("IDI: NULL_BLANK = 1 (enabled)");

    LOGT("mipi_dphy_idi_test init: complete");

    return 0;
}

/*
 * Function: mipi_dphy_idi_test_run
 * Description: Executes the main testcase flow for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int vcid;
    unsigned int pkt;
    uint32_t vcid_reg_val;
    uint32_t dma_status;
    uint32_t csi_ctrl_data;
    uint32_t data_type;
    uint32_t word_count;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("IDI: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dphy_idi_test run: starting VCID loop");

    /* Steps 16-31: Loop through all 16 VCIDs (0-15) */
    for (vcid = 0U; vcid < IDI_TOTAL_VCIDS; vcid++) {

        /* Configure virtual channel register (VCID shifted left by 12 bits) */
        vcid_reg_val = ((uint32_t)vcid << 12U);
        writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_reg_val);
        LOGT("IDI: VCID=%u VIRTUAL_CHANNEL=0x%lx", vcid, (unsigned long)vcid_reg_val);

        /* Trigger CSI-2 sequence for this VCID */
        writel_reg((uintptr_t)MIPI_CSI2_SEQ_TRIGGER_ADDR, vcid);
        LOGT("IDI: Triggered sequence for VCID=%u", vcid);

        /* Process 129 packets per VCID */
        for (pkt = 0U; pkt < IDI_PACKETS_PER_VCID; pkt++) {

            /* DMA transfer on channel 0 for control data */
            // MANUAL_REVIEW: DMA channel 0 programming for control data transfer
            // requires platform-specific DMA setup APIs not fully specified in the
            // Meta TestPlan JSON.

            /* Poll DMA interrupt status for channel 0 completion */
            timeout = MIPI_CSI2_POLL_TIMEOUT;
            dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            while (((dma_status & 0x01U) == 0U) && (timeout > 0U)) {
                dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
            }

            if ((dma_status & 0x01U) == 0U) {
                LOGE("IDI: DMA ch0 timeout, vcid=%u pkt=%u status=0x%lx",
                     vcid, pkt, (unsigned long)dma_status);
                g_ctx.errors++;
                continue;
            }

            /* Clear DMA interrupt for channel 0 */
            writel_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x01U);
            LOGT("IDI: DMA ch0 complete, vcid=%u pkt=%u", vcid, pkt);

            /* Read control data to determine packet type */
            csi_ctrl_data = readl_reg((uintptr_t)MIPI_CSI2_CTRL_DATA_ADDR);
            data_type = (csi_ctrl_data >> 0U) & 0x3FU;
            word_count = (csi_ctrl_data >> 8U) & 0xFFFFU;
            LOGT("IDI: ctrl_data=0x%lx data_type=0x%lx word_count=%lu",
                 (unsigned long)csi_ctrl_data,
                 (unsigned long)data_type,
                 (unsigned long)word_count);

            /* If long packet (data_type > 0xf), perform data DMA on channel 1 */
            if (data_type > 0x0FU) {

                // MANUAL_REVIEW: DMA channel 1 programming for pixel data transfer
                // requires platform-specific DMA setup APIs. word_count is used
                // for transfer size (8-byte aligned).

                /* Poll DMA interrupt status for channel 1 completion */
                timeout = MIPI_CSI2_POLL_TIMEOUT;
                dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                while (((dma_status & 0x02U) == 0U) && (timeout > 0U)) {
                    dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                }

                if ((dma_status & 0x02U) == 0U) {
                    LOGE("IDI: DMA ch1 timeout, vcid=%u pkt=%u status=0x%lx",
                         vcid, pkt, (unsigned long)dma_status);
                    g_ctx.errors++;
                    continue;
                }

                /* Clear DMA interrupt for channel 1 */
                writel_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x02U);
                LOGT("IDI: DMA ch1 complete, vcid=%u pkt=%u wc=%lu",
                     vcid, pkt, (unsigned long)word_count);
            }
        }

        g_ctx.vcids_completed++;
        LOGT("IDI: VCID %u completed", vcid);
    }

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dphy_idi_test run complete: %s errors=%u vcids_completed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.vcids_completed);

    return out->status;
}

/*
 * Function: mipi_dphy_idi_test_teardown
 * Description: Performs testcase cleanup for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    /* Validate all 16 VCIDs were processed */
    if (g_ctx.vcids_completed != IDI_TOTAL_VCIDS) {
        LOGE("IDI: Not all VCIDs completed: expected=%u actual=%u",
             IDI_TOTAL_VCIDS,
             g_ctx.vcids_completed);
    }

    // MANUAL_REVIEW: DV finish(0) was present in the source flow, converted to
    // PSV/FV-native status reporting via out->status in the run function.

    LOGT("mipi_dphy_idi_test teardown: errors=%u vcids_completed=%u",
         g_ctx.errors, g_ctx.vcids_completed);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
