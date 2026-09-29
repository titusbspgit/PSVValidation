// Author - AI Force 2.3. 28-Jul-2025 17:15 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * mipi_csi2_test_pattern_generator.c
 *
 * Test Case : mipi_csi2_test_pattern_generator
 * Description: Configures and validates the MIPI CSI2 internal test pattern
 *              generator. Sets vertical/horizontal resolution, configures and
 *              enables the pattern generator, sets up the receive path, polls
 *              PHY stop state, configures DMA channel 0, triggers DMA,
 *              disables the pattern generator, waits for DMA completion,
 *              reads main interrupt status, and masks all CSI2 host interrupts.
 */

static mipi_csi2_tpg_test_ctx_t g_ctx;

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_tpg_test_ctx_t){0};
    g_ctx.errors = 0U;

    LOGT("mipi_csi2_test_pattern_generator init: starting test pattern generator test");

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
    uint32_t read_val;
    unsigned int poll_count;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_test_pattern_generator run");

    /* Step 1: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES to set the test pattern vertical resolution */
    LOGT("Step 1: Writing MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x0U);

    /* Step 2: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES to set the test pattern horizontal resolution */
    LOGT("Step 2: Writing MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x0U);

    /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG to configure the pattern generator settings */
    LOGT("Step 3: Writing MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0x0U);

    /* Step 4: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with value 1 to enable the test pattern generator */
    LOGT("Step 4: Enabling pattern generator (PPI_PG_ENABLE = 1)");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1U);

    /* Step 5: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel */
    LOGT("Step 5: Writing MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x0U);

    /* Step 6: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with value 0 to set the control data */
    LOGT("Step 6: Writing MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA = 0");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0U);

    /* Step 7: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for PHY stop state condition */
    LOGT("Step 7: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE");
    poll_count = 0U;
    do {
        read_val = (uint32_t)readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        poll_count++;
        if (poll_count >= MIPI_CSI2_PHY_STOPSTATE_POLL_MAX) {
            LOGE("PHY_STOPSTATE poll timeout after %u iterations", poll_count);
            g_ctx.errors++;
            out->status = -1;
            break;
        }
    } while ((read_val & MIPI_CSI2_PHY_STOPSTATE_MASK) != MIPI_CSI2_PHY_STOPSTATE_MASK);

    if (out->status == 0) {
        LOGT("PHY_STOPSTATE condition met after %u polls, val=0x%lx",
             poll_count, (unsigned long)read_val);
    }

    /* Step 8: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA with value 0x100 */
    LOGT("Step 8: Writing DMA_M0_ADDR_AR_CH0_DATA = 0x100");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100U);

    /* Step 9: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION with value 0x0 */
    LOGT("Step 9: Writing DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x0");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);

    /* Step 10: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA with value 0x0 */
    LOGT("Step 10: Writing DMA_M0_ADDR_AW_CH0_DATA = 0x0");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0U);

    /* Step 11: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION with value 0x0 */
    LOGT("Step 11: Writing DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x0");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);

    /* Step 12: Write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 with value 0x1 to trigger DMA */
    LOGT("Step 12: Triggering DMA (DMA_TRIGGER = 0x1)");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_TRIGGER, 0x1U);

    /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with value 0 to disable pattern generator */
    LOGT("Step 13: Disabling pattern generator (PPI_PG_ENABLE = 0)");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0U);

    /* Step 14: Poll DMA interrupt status register for DMA completion */
    LOGT("Step 14: Polling DMA interrupt status for completion");
    poll_count = 0U;
    do {
        read_val = (uint32_t)readl_reg((uintptr_t)(MIPI_CSI2_GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET));
        poll_count++;
        if (poll_count >= MIPI_CSI2_DMA_POLL_MAX) {
            LOGE("DMA completion poll timeout after %u iterations", poll_count);
            g_ctx.errors++;
            out->status = -1;
            break;
        }
    } while ((read_val & MIPI_CSI2_DMA_COMPLETION_MASK) != MIPI_CSI2_DMA_COMPLETION_MASK);

    if (out->status == 0) {
        LOGT("DMA transfer complete after %u polls, val=0x%lx",
             poll_count, (unsigned long)read_val);
    }

    /* Step 15: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status */
    LOGT("Step 15: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    read_val = (uint32_t)readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN = 0x%lx", (unsigned long)read_val);

    /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    LOGT("Step 16: Masking PHY fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    LOGT("Step 17: Masking packet fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 18: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    LOGT("Step 18: Masking PHY interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ALL);

    /* Step 19: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    LOGT("Step 19: Masking line interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ALL);

    /* Step 20: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    LOGT("Step 20: Masking boundary frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 21: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    LOGT("Step 21: Masking sequence frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 22: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    LOGT("Step 22: Masking CRC frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 23: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    LOGT("Step 23: Masking payload CRC fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 24: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    LOGT("Step 24: Masking data ID interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ALL);

    /* Step 25: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    LOGT("Step 25: Masking ECC corrected interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ALL);

    /* Final status update */
    if (g_ctx.errors != 0U) {
        out->status = -1;
    }

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
