// Author - AI Force 2.3. 01-Jul-2025 12:48 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Testcase: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI2 internal test pattern generator (PPI PG)
 *              functionality. Configures pattern generator resolution, mode, and
 *              data type. Enables pattern generation, configures DMA addresses,
 *              enables clock gating, disables pattern generator, polls DMA
 *              interrupt status, reads main interrupt status, and masks all
 *              interrupt sources.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_tpg_test_ctx_t;

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

    LOGT("MIPI CSI2 test pattern generator init: starting initialization");

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
    uint32_t rd_val;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("MIPI CSI2 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting MIPI CSI2 test pattern generator validation");

    /* Step 1: Write 0x10 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES */
    /* Set the pattern generator vertical resolution to 16 lines */
    LOGT("Step 1: Writing 0x10 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, MIPI_CSI2_PG_VRES_VAL);
    g_ctx.checks_total++;

    /* Step 2: Write 0x70140 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES */
    /* Set the pattern generator horizontal resolution (HRES=320, data type RGB888) */
    LOGT("Step 2: Writing 0x70140 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, MIPI_CSI2_PG_HRES_VAL);
    g_ctx.checks_total++;

    /* Step 3: Write 0xe401 to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG */
    /* Configure the pattern generator mode and data type settings */
    LOGT("Step 3: Writing 0xe401 to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, MIPI_CSI2_PG_CONFIG_VAL);
    g_ctx.checks_total++;

    /* Step 4: Write 1 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE */
    /* Enable the internal test pattern generator */
    LOGT("Step 4: Writing 1 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE (enable)");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_ENABLE_VAL);
    g_ctx.checks_total++;

    /* Step 5: Write virtual channel ID configuration to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL */
    // MANUAL_REVIEW: The exact virtual channel ID value is not specified in the Meta TestPlan JSON.
    // Set MIPI_CSI2_VIRTUAL_CHANNEL_CFG to the actual value required by the SoC.
    LOGT("Step 5: Writing MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIPI_CSI2_VIRTUAL_CHANNEL_CFG);
    g_ctx.checks_total++;

    /* Step 6: Write 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to clear the control data register */
    LOGT("Step 6: Writing 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0UL);
    g_ctx.checks_total++;

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to check current PHY stop state */
    LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE");
    rd_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("PHY_STOPSTATE initial read: rd_val=0x%lx", (unsigned long)rd_val);
    g_ctx.checks_total++;

    /* Step 8: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until expected stop state is achieved */
    LOGT("Step 8: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for stop state");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        rd_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        if (timeout == 0U) {
            LOGE("Timeout polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_val == 0U);

    if (timeout > 0U) {
        LOGT("PHY stop state achieved: rd_val=0x%lx", (unsigned long)rd_val);
        g_ctx.checks_passed++;
    } else {
        g_ctx.checks_failed++;
    }
    g_ctx.checks_total++;

    /* Step 9: Write 0x100 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA */
    LOGT("Step 9: Writing 0x100 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, MIPI_CSI2_DMA_AR_CH0_DATA_VAL);
    g_ctx.checks_total++;

    /* Step 10: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION */
    LOGT("Step 10: Writing 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, MIPI_CSI2_DMA_AR_CH0_INSTR_VAL);
    g_ctx.checks_total++;

    /* Step 11: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA */
    LOGT("Step 11: Writing 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, MIPI_CSI2_DMA_AW_CH0_DATA_VAL);
    g_ctx.checks_total++;

    /* Step 12: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION */
    LOGT("Step 12: Writing 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, MIPI_CSI2_DMA_AW_CH0_INSTR_VAL);
    g_ctx.checks_total++;

    /* Step 13: Write 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 to enable clock gating for CSI PHY */
    LOGT("Step 13: Writing 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 (clock gating enable)");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_CLK_GATE_OFFSET, MIPI_CSI2_CLK_GATE_ENABLE_VAL);
    g_ctx.checks_total++;

    /* Step 14: Write 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to disable pattern generator */
    LOGT("Step 14: Writing 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE (disable)");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_DISABLE_VAL);
    g_ctx.checks_total++;

    /* Step 15: Poll gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET for DMA transfer completion */
    LOGT("Step 15: Polling gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET for DMA completion");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        rd_val = readl_reg(MIPI_CSI2_GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
        if (timeout == 0U) {
            LOGE("Timeout polling DMA INTMIS for transfer completion");
            g_ctx.errors++;
            break;
        }
        timeout--;
    } while (rd_val == 0U);

    if (timeout > 0U) {
        LOGT("DMA transfer completion detected: INTMIS=0x%lx", (unsigned long)rd_val);
        g_ctx.checks_passed++;
    } else {
        g_ctx.checks_failed++;
    }
    g_ctx.checks_total++;

    /* Step 16: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
    LOGT("Step 16: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    rd_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN read: rd_val=0x%lx", (unsigned long)rd_val);
    if (rd_val != 0U) {
        LOGE("Unexpected interrupts asserted in INT_ST_MAIN=0x%lx",
             (unsigned long)rd_val);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        g_ctx.checks_passed++;
    }
    g_ctx.checks_total++;

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    LOGT("Step 17: Masking PHY fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 18: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    LOGT("Step 18: Masking packet fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 19: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    LOGT("Step 19: Masking PHY interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ALL);

    /* Step 20: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    LOGT("Step 20: Masking line interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ALL);

    /* Step 21: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    LOGT("Step 21: Masking boundary frame fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 22: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    LOGT("Step 22: Masking sequence frame fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 23: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    LOGT("Step 23: Masking CRC frame fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 24: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    LOGT("Step 24: Masking payload CRC fatal interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 25: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    LOGT("Step 25: Masking data ID interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ALL);

    /* Step 26: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    LOGT("Step 26: Masking ECC corrected interrupts");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ALL);

    /* Step 27: Final status update - validate DMA transfer and no unexpected errors */
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

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

    LOGT("MIPI CSI2 test pattern generator teardown: errors=%u checks_total=%u",
         g_ctx.errors, g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
