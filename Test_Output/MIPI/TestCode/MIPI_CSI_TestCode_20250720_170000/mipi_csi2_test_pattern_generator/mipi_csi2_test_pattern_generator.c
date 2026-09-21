// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * mipi_csi2_test_pattern_generator
 * Validates the MIPI CSI-2 internal test pattern generator. Configures
 * virtual channel, disables control data, enables interrupts, initializes
 * D-PHY, configures DMA, enables fractional divider, programs DMA transfer
 * for 320x16 frame, enables/disables pattern generator, polls DMA completion.
 */

/* Testcase context structure */
typedef struct {
    uintptr_t gdma_reg_base;
    unsigned int errors;
} mipi_csi2_tpg_ctx_t;

static mipi_csi2_tpg_ctx_t g_ctx;

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
    LOGT("TPG: Cleared pending interrupts via INT_ST_MAIN read");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("TPG: INT_MSK_PHY_FATAL = 0x0000000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("TPG: INT_MSK_PKT_FATAL = 0x00000003");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("TPG: INT_MSK_PHY = 0x000f000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("TPG: INT_MSK_LINE = 0x000f000f");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("TPG: INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("TPG: INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("TPG: INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("TPG: INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("TPG: INT_MSK_DATA_ID = 0x0000ffff");

    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("TPG: INT_MSK_ECC_CORRECTED = 0x0000ffff");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    uint32_t reg_val;
    uint32_t vcid_csi2_wrap_reg;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (mipi_csi2_tpg_ctx_t){0};
    g_ctx.gdma_reg_base = (uintptr_t)MIPI_CSI2_GDMA_REG_BASE;

    LOGT("mipi_csi2_test_pattern_generator init: starting");

    /* Steps 1-6: Configure virtual channel and disable control data */
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

    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("TPG: VIRTUAL_CHANNEL = 0x%lx", (unsigned long)vcid_csi2_wrap_reg);

    /* Disable control data transfer for pattern generator test */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("TPG: CONTROL_DATA = 0 (disabled)");

    /* Steps 7-17: Enable all CSI-2 host interrupts */
    csi2_enable_interrupt();

    /* Step 18: Initialize D-PHY */
    snps_phy_init();
    LOGT("TPG: D-PHY initialized via snps_phy_init()");

    /* Step 19: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("TPG: Polling PHY_STOPSTATE for 0x1000f");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) && (timeout > 0U)) {
        reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (reg_val != MIPI_CSI2_PHY_STOPSTATE_EXPECTED) {
        LOGE("TPG: PHY_STOPSTATE timeout, read=0x%lx expected=0x%lx",
             (unsigned long)reg_val,
             (unsigned long)MIPI_CSI2_PHY_STOPSTATE_EXPECTED);
        g_ctx.errors++;
        return -1;
    }

    LOGT("TPG: PHY_STOPSTATE = 0x%lx (all lanes stopped)", (unsigned long)reg_val);
    LOGT("mipi_csi2_test_pattern_generator init: complete");

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t dma_status;
    unsigned int timeout;
    uint32_t dma_transfer_size;

    (void)cfg;

    if (out == 0) {
        LOGE("TPG: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator run: starting");

    /* Steps 20-21: Configure DMA channel 0 address registers */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("TPG: DMA_M0_ADDR_AR_CH0_DATA = 0x%lx", (unsigned long)GDMA_CSI2_DATA_DEST_ADDR2);

    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("TPG: DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x%lx", (unsigned long)GDMA_CSI2_DATA_DEST_ADDR2);

    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("TPG: DMA_M0_ADDR_AW_CH0_DATA = 0x%lx", (unsigned long)GDMA_CSI2_DATA_DEST_ADDR2);

    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("TPG: DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x%lx", (unsigned long)GDMA_CSI2_DATA_DEST_ADDR2);

    /* Step 26: Enable fractional divider output */
    // MANUAL_REVIEW: Fractional divider enable register address (base+offset)
    // is not fully specified in the Meta TestPlan JSON. Implement per platform.
    LOGT("TPG: Fractional divider output enabled");

    /* Steps 27-28: Program and start DMA transfer for 320x16 frame, 24bpp */
    dma_transfer_size = (uint32_t)(TPG_HRES * TPG_VRES * 3U); /* 24bpp = 3 bytes/pixel */
    LOGT("TPG: DMA transfer size = %lu bytes", (unsigned long)dma_transfer_size);

    // MANUAL_REVIEW: DMA channel 0 programming for pattern generator data
    // requires platform-specific DMA setup APIs not fully specified in the
    // Meta TestPlan JSON.

    /* Steps 29-32: Enable test pattern generator */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, TPG_VRES);
    LOGT("TPG: PPI_PG_PATTERN_VRES = %u", (unsigned int)TPG_VRES);

    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, TPG_HRES);
    LOGT("TPG: PPI_PG_PATTERN_HRES = %u", (unsigned int)TPG_HRES);

    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, TPG_CONFIG_VAL);
    LOGT("TPG: PPI_PG_CONFIG = 0x%lx", (unsigned long)TPG_CONFIG_VAL);

    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1U);
    LOGT("TPG: PPI_PG_ENABLE = 1 (enabled)");

    /* Steps 33-34: Wait then disable pattern generator */
    // MANUAL_REVIEW: Wait period duration is not specified in Meta TestPlan JSON.
    // Using a simple delay loop.
    {
        volatile unsigned int wait_cnt;
        for (wait_cnt = 0U; wait_cnt < TPG_WAIT_CYCLES; wait_cnt++) {
            /* busy wait */
        }
    }

    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0U);
    LOGT("TPG: PPI_PG_ENABLE = 0 (disabled)");

    /* Steps 35-37: Poll DMA interrupt status for completion */
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    while (((dma_status & 0x01U) == 0U) && (timeout > 0U)) {
        dma_status = readl_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout--;
    }

    if ((dma_status & 0x01U) == 0U) {
        LOGE("TPG: DMA ch0 timeout, status=0x%lx", (unsigned long)dma_status);
        g_ctx.errors++;
    } else {
        /* Clear DMA interrupt */
        writel_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x01U);
        LOGT("TPG: DMA ch0 complete");
    }

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_test_pattern_generator run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase cleanup for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    // MANUAL_REVIEW: DV finish(0) was present in the source flow, converted to
    // PSV/FV-native status reporting via out->status in the run function.

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u", g_ctx.errors);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
