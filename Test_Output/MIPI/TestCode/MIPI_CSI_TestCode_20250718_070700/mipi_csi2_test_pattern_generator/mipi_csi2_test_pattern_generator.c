// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI2 internal test pattern generator. Configures
 *   pattern generator resolution and type, enables it, configures virtual channel,
 *   control data, polls PHY_STOPSTATE, configures DMA channel 0 addresses, reads
 *   INT_ST_MAIN, enables all interrupt masks, and disables the pattern generator.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

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

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

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
    uint32_t rd_data;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator run: begin");

    /* Step 1: Write vertical resolution value to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES */
    LOGT("Step 1: Writing vertical resolution to PPI_PG_PATTERN_VRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, MIPI_CSI2_PG_VRES_VALUE);

    /* Step 2: Write horizontal resolution value to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES */
    LOGT("Step 2: Writing horizontal resolution to PPI_PG_PATTERN_HRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, MIPI_CSI2_PG_HRES_VALUE);

    /* Step 3: Write pattern configuration to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG */
    LOGT("Step 3: Writing pattern configuration to PPI_PG_CONFIG");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, MIPI_CSI2_PG_CONFIG_VALUE);

    /* Step 4: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to enable the test pattern generator */
    LOGT("Step 4: Enabling pattern generator via PPI_PG_ENABLE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_ENABLE_VALUE);

    /* Step 5: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL */
    LOGT("Step 5: Writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIPI_CSI2_VIRTUAL_CHANNEL_VALUE);

    /* Step 6: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA */
    LOGT("Step 6: Writing to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, MIPI_CSI2_CONTROL_DATA_VALUE);

    /* Step 7: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE */
    LOGT("Step 7: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for stop state");
    timeout = MIPI_CSI2_PHY_STOPSTATE_TIMEOUT;
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((rd_data == 0U) && (timeout > 0U)) {
        rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("mipi_csi2_test_pattern_generator: PHY_STOPSTATE polling timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("PHY_STOPSTATE reached stop state: rd_data=0x%lx", (unsigned long)rd_data);
    }

    /* Step 8: Write DMA channel 0 read address - data */
    LOGT("Step 8: Writing to DMA_M0_ADDR_AR_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, MIPI_CSI2_DMA_ADDR_VALUE);

    /* Step 9: Write DMA channel 0 read address - instruction */
    LOGT("Step 9: Writing to DMA_M0_ADDR_AR_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, MIPI_CSI2_DMA_ADDR_VALUE);

    /* Step 10: Write DMA channel 0 write address - data */
    LOGT("Step 10: Writing to DMA_M0_ADDR_AW_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, MIPI_CSI2_DMA_ADDR_VALUE);

    /* Step 11: Write DMA channel 0 write address - instruction */
    LOGT("Step 11: Writing to DMA_M0_ADDR_AW_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, MIPI_CSI2_DMA_ADDR_VALUE);

    /* Step 12: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN */
    LOGT("Step 12: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN=0x%lx", (unsigned long)rd_data);

    /* Steps 13-22: Enable all CSI2 host interrupt masks */
    LOGT("Steps 13-22: Enabling all CSI2 host interrupt masks");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ENABLE_ALL);
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 23: Disable pattern generator by writing 0 to PPI_PG_ENABLE */
    LOGT("Step 23: Disabling pattern generator via PPI_PG_ENABLE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_DISABLE_VALUE);

    LOGT("mipi_csi2_test_pattern_generator run complete: %s errors=%u",
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
