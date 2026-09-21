// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI2 DPHY lane configuration. Writes to
 *   MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA,
 *   MIZAR_MIPI_CSI2_HOST_N_LANES, polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE,
 *   accesses direct hex addresses 0xa0243ffc and 0xE6001000, reads
 *   MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN, and enables all CSI2 host interrupt masks.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

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
    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test init: starting DPHY lane configuration test");

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
    uint32_t rd_data;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test run: begin");

    /* Step 1: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel */
    LOGT("Step 1: Writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIPI_CSI2_VIRTUAL_CHANNEL_VALUE);

    /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data parameters */
    LOGT("Step 2: Writing to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, MIPI_CSI2_CONTROL_DATA_VALUE);

    /* Step 3: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop for PHY stop state */
    LOGT("Step 3: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for stop state condition");
    timeout = MIPI_CSI2_PHY_STOPSTATE_TIMEOUT;
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((rd_data == 0U) && (timeout > 0U)) {
        rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("mipi_csi2_dphy_lanes_test: PHY_STOPSTATE polling timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("PHY_STOPSTATE reached stop state: rd_data=0x%lx", (unsigned long)rd_data);
    }

    /* Step 4: Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure active DPHY lanes */
    LOGT("Step 4: Writing to MIZAR_MIPI_CSI2_HOST_N_LANES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, MIPI_CSI2_N_LANES_VALUE);

    /* Step 5: Write to 0xa0243ffc (direct hex address) */
    LOGT("Step 5: Writing to direct hex address 0xa0243ffc");
    writel_reg(MIPI_CSI2_EXTERNAL_WR_ADDR, MIPI_CSI2_EXTERNAL_WR_VALUE);

    /* Step 6: Read from 0xE6001000 (direct hex address) */
    LOGT("Step 6: Reading from direct hex address 0xE6001000");
    rd_data = readl_reg(MIPI_CSI2_EXTERNAL_RD_ADDR);
    LOGT("Read from 0xE6001000: rd_data=0x%lx", (unsigned long)rd_data);

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
    LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN=0x%lx", (unsigned long)rd_data);

    /* Step 8: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to enable PHY fatal interrupt mask */
    LOGT("Step 8: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 9: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to enable packet fatal interrupt mask */
    LOGT("Step 9: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 10: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to enable PHY interrupt mask */
    LOGT("Step 10: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 11: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to enable line interrupt mask */
    LOGT("Step 11: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 12: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to enable boundary frame fatal interrupt mask */
    LOGT("Step 12: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to enable sequence frame fatal interrupt mask */
    LOGT("Step 13: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 14: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to enable CRC frame fatal interrupt mask */
    LOGT("Step 14: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 15: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to enable payload CRC fatal interrupt mask */
    LOGT("Step 15: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to enable data ID interrupt mask */
    LOGT("Step 16: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to enable ECC corrected interrupt mask */
    LOGT("Step 17: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

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

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
