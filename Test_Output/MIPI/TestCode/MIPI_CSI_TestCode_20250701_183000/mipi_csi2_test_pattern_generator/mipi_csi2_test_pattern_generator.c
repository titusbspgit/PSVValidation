// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI2 internal test pattern generator (PPI PG)
 *   functionality. Configures pattern generator resolution, mode, and data type,
 *   enables/disables the PG, configures CSI2 subsystem wrapper, polls PHY stop
 *   state, configures DMA address registers, enables clock gating, polls DMA
 *   interrupt status, reads main interrupt status, and masks all CSI2 host
 *   interrupt sources.
 */

/* Clock gating register offset from MIZAR_MIPI_CSI2_RB_REG_BASE */
#define MIPI_CSI2_CLK_GATE_OFFSET  0xf4UL

/* Polling timeout counts following FV Template timeout style */
#define MIPI_CSI2_PHY_POLL_TIMEOUT  1000U
#define MIPI_CSI2_DMA_POLL_TIMEOUT  1000U

/* Pattern generator configuration values from Meta TestPlan JSON */
#define MIPI_CSI2_PG_VRES_VAL       0x10U
#define MIPI_CSI2_PG_HRES_VAL       0x70140UL
#define MIPI_CSI2_PG_CONFIG_VAL     0xe401U
#define MIPI_CSI2_PG_ENABLE_VAL     0x1U
#define MIPI_CSI2_PG_DISABLE_VAL    0x0U

/* DMA address configuration values from Meta TestPlan JSON */
#define MIPI_CSI2_DMA_AR_CH0_DATA_VAL         0x100UL
#define MIPI_CSI2_DMA_AR_CH0_INSTRUCTION_VAL  0x0UL
#define MIPI_CSI2_DMA_AW_CH0_DATA_VAL         0x0UL
#define MIPI_CSI2_DMA_AW_CH0_INSTRUCTION_VAL  0x0UL

/* Clock gating enable value */
#define MIPI_CSI2_CLK_GATE_ENABLE_VAL  0x1U

typedef struct {
    unsigned int errors;
    unsigned int phy_poll_ok;
    unsigned int dma_poll_ok;
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

    LOGT("mipi_csi2_test_pattern_generator init: starting PPI PG test");

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
    unsigned int timeout;
    uint32_t reg_val;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting MIPI CSI2 internal test pattern generator test");

    /* Step 1: Write 0x10 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES to set vertical resolution to 16 lines */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, MIPI_CSI2_PG_VRES_VAL);
    LOGT("Wrote PPI_PG_PATTERN_VRES=0x%x", MIPI_CSI2_PG_VRES_VAL);

    /* Step 2: Write 0x70140 to MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES to set horizontal resolution (HRES=320, RGB888) */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, MIPI_CSI2_PG_HRES_VAL);
    LOGT("Wrote PPI_PG_PATTERN_HRES=0x%lx", (unsigned long)MIPI_CSI2_PG_HRES_VAL);

    /* Step 3: Write 0xe401 to MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG to configure pattern generator mode and data type */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, MIPI_CSI2_PG_CONFIG_VAL);
    LOGT("Wrote PPI_PG_CONFIG=0x%x", MIPI_CSI2_PG_CONFIG_VAL);

    /* Step 4: Write 1 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to enable the internal test pattern generator */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_ENABLE_VAL);
    LOGT("Enabled PPI PG (PPI_PG_ENABLE=0x%x)", MIPI_CSI2_PG_ENABLE_VAL);

    /* Step 5: Write virtual channel ID configuration to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL */
    // MANUAL_REVIEW: The exact virtual channel ID value is not specified in the Meta TestPlan JSON.
    // Using 0 as a placeholder; review and set the correct virtual channel configuration.
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x0U);
    LOGT("Wrote VIRTUAL_CHANNEL register");

    /* Step 6: Write 0 to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to clear the control data register */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0U);
    LOGT("Cleared CONTROL_DATA register");

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to check the current PHY stop state */
    reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("Initial PHY_STOPSTATE=0x%x", reg_val);

    /* Step 8: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until the expected stop state value is achieved */
    timeout = MIPI_CSI2_PHY_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        if (reg_val != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);

    if (timeout == 0U) {
        LOGE("PHY_STOPSTATE polling timeout");
        g_ctx.errors++;
    } else {
        g_ctx.phy_poll_ok = 1U;
        LOGT("PHY_STOPSTATE confirmed, value=0x%x", reg_val);
    }

    /* Step 9: Write 0x100 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, MIPI_CSI2_DMA_AR_CH0_DATA_VAL);
    LOGT("Wrote DMA_M0_ADDR_AR_CH0_DATA=0x%lx", (unsigned long)MIPI_CSI2_DMA_AR_CH0_DATA_VAL);

    /* Step 10: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, MIPI_CSI2_DMA_AR_CH0_INSTRUCTION_VAL);
    LOGT("Wrote DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x%lx", (unsigned long)MIPI_CSI2_DMA_AR_CH0_INSTRUCTION_VAL);

    /* Step 11: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, MIPI_CSI2_DMA_AW_CH0_DATA_VAL);
    LOGT("Wrote DMA_M0_ADDR_AW_CH0_DATA=0x%lx", (unsigned long)MIPI_CSI2_DMA_AW_CH0_DATA_VAL);

    /* Step 12: Write 0x0 to MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, MIPI_CSI2_DMA_AW_CH0_INSTRUCTION_VAL);
    LOGT("Wrote DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x%lx", (unsigned long)MIPI_CSI2_DMA_AW_CH0_INSTRUCTION_VAL);

    /* Step 13: Write 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 to enable clock gating for the CSI PHY */
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_CLK_GATE_OFFSET, MIPI_CSI2_CLK_GATE_ENABLE_VAL);
    LOGT("Enabled clock gating at MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4");

    /* Step 14: Write 0 to MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE to disable the pattern generator */
    writel_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, MIPI_CSI2_PG_DISABLE_VAL);
    LOGT("Disabled PPI PG (PPI_PG_ENABLE=0x%x)", MIPI_CSI2_PG_DISABLE_VAL);

    /* Step 15: Poll gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET for DMA transfer completion */
    // MANUAL_REVIEW: gdma_reg_base is a variable base address not captured as a standalone macro.
    // The polling uses gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET which requires runtime resolution.
    timeout = MIPI_CSI2_DMA_POLL_TIMEOUT;
    do {
        reg_val = readl_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        if (reg_val != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);

    if (timeout == 0U) {
        LOGE("DMA INTMIS polling timeout");
        g_ctx.errors++;
    } else {
        g_ctx.dma_poll_ok = 1U;
        LOGT("DMA transfer complete, INTMIS=0x%x", reg_val);
    }

    /* Step 16: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
    reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN=0x%x", reg_val);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0U);

    /* Step 18: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0U);

    /* Step 19: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0U);

    /* Step 20: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0U);

    /* Step 21: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0U);

    /* Step 22: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0U);

    /* Step 23: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0U);

    /* Step 24: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0U);

    /* Step 25: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0U);

    /* Step 26: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0U);

    LOGT("All interrupt mask registers written");

    /* Step 27: Validate that the DMA transfer completed successfully and no unexpected errors occurred */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u phy_poll_ok=%u dma_poll_ok=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.phy_poll_ok,
         g_ctx.dma_poll_ok);

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

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u phy_poll_ok=%u dma_poll_ok=%u",
         g_ctx.errors, g_ctx.phy_poll_ok, g_ctx.dma_poll_ok);

    return g_ctx.errors == 0U ? 0 : -1;
}
