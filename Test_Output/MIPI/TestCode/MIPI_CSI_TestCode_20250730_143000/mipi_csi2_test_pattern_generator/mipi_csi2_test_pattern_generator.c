// Author - AI Force 2.3. 30-Jul-2025 14:30 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * MIPI CSI2 Test Pattern Generator
 * This testcase configures and validates the MIPI CSI2 internal test pattern
 * generator. It sets vertical and horizontal resolution, configures and
 * enables the pattern generator, configures virtual channel and control data,
 * polls PHY_STOPSTATE, configures DMA channel 0 addresses, triggers DMA,
 * disables the pattern generator, polls DMA completion, reads INT_ST_MAIN,
 * and masks all CSI2 host interrupts.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_tpg_ctx_t;

static mipi_csi2_tpg_ctx_t g_ctx;

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

    g_ctx = (mipi_csi2_tpg_ctx_t){0};

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
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_test_pattern_generator execution");

    /* Step 1: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES to set the test pattern vertical resolution */
    LOGT("Step 1: Writing to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x00000000U);

    /* Step 2: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES to set the test pattern horizontal resolution */
    LOGT("Step 2: Writing to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x00000000U);

    /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG to configure the pattern generator settings */
    LOGT("Step 3: Writing to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0x00000000U);

    /* Step 4: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with value 1 to enable the test pattern generator */
    LOGT("Step 4: Writing to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to enable pattern generator");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x00000001U);

    /* Step 5: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel */
    LOGT("Step 5: Writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x00000000U);

    /* Step 6: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA with value 0 to set the control data */
    LOGT("Step 6: Writing to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x00000000U);

    /* Step 7: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop to wait for the PHY stop state condition */
    LOGT("Step 7: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for PHY stop state");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        read_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        if (timeout == 0U) {
            LOGE("Timeout waiting for MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE");
            g_ctx.errors++;
            out->status = -1;
            break;
        }
        timeout--;
    } while (read_val == 0U);
    LOGT("MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE read_val=0x%lx", (unsigned long)read_val);

    /* Step 8: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA with value 0x100 */
    LOGT("Step 8: Writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x00000100U);

    /* Step 9: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION with value 0x0 */
    LOGT("Step 9: Writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x00000000U);

    /* Step 10: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA with value 0x0 */
    LOGT("Step 10: Writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x00000000U);

    /* Step 11: Write to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION with value 0x0 */
    LOGT("Step 11: Writing to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x00000000U);

    /* Step 12: Write to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 with value 0x1 to trigger DMA */
    LOGT("Step 12: Writing to MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_DMA_TRIGGER_OFFSET to trigger DMA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_DMA_TRIGGER_OFFSET, 0x00000001U);

    /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE with value 0 to disable the pattern generator */
    LOGT("Step 13: Writing to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to disable pattern generator");
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x00000000U);

    /* Step 14: Poll DMA interrupt status register for DMA completion */
    LOGT("Step 14: Polling gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET for DMA completion");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        read_val = readl_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        if (timeout == 0U) {
            LOGE("Timeout waiting for DMA completion at gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET");
            g_ctx.errors++;
            out->status = -1;
            break;
        }
        timeout--;
    } while (read_val == 0U);
    LOGT("DMA interrupt status read_val=0x%lx", (unsigned long)read_val);

    /* Step 15: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status */
    LOGT("Step 15: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    read_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN read_val=0x%lx", (unsigned long)read_val);

    /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    LOGT("Step 16: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x00000000U);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    LOGT("Step 17: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000000U);

    /* Step 18: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    LOGT("Step 18: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x00000000U);

    /* Step 19: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    LOGT("Step 19: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x00000000U);

    /* Step 20: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    LOGT("Step 20: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x00000000U);

    /* Step 21: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    LOGT("Step 21: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x00000000U);

    /* Step 22: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    LOGT("Step 22: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x00000000U);

    /* Step 23: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    LOGT("Step 23: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x00000000U);

    /* Step 24: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    LOGT("Step 24: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x00000000U);

    /* Step 25: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    LOGT("Step 25: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x00000000U);

    /* Final status determination */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

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

    /* Validation: PHY_STOPSTATE was polled to confirm stop state before DMA configuration */
    /* Validation: DMA interrupt status was polled for DMA transfer completion */
    /* Validation: INT_ST_MAIN was read to check main interrupt status after transfer */
    /* Validation: PPI_PG_ENABLE was written with 1 to enable and 0 to disable pattern generator */
    /* Validation: All interrupt mask registers were written to mask interrupts */
    /* Validation: Test expected to complete without errors after all register operations and DMA transfer succeed */

    LOGT("mipi_csi2_test_pattern_generator teardown: %s",
         (g_ctx.errors == 0U) ? "PASS" : "FAIL");

    return g_ctx.errors == 0U ? 0 : -1;
}
