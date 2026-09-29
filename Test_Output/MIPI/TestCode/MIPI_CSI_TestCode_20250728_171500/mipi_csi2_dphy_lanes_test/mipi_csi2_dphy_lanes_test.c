// Author - AI Force 2.3. 28-Jul-2025 17:00 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * mipi_csi2_dphy_lanes_test.c
 *
 * Test Case : mipi_csi2_dphy_lanes_test
 * Description: Configures and validates MIPI CSI2 DPHY lane operation.
 *              Writes virtual channel, control data, number of active DPHY
 *              lanes, polls PHY stop state, writes/reads hardcoded addresses,
 *              reads main interrupt status, and masks all CSI2 host interrupts.
 */

static mipi_csi2_dphy_test_ctx_t g_ctx;

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

    g_ctx = (mipi_csi2_dphy_test_ctx_t){0};
    g_ctx.errors = 0U;

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
    uint32_t read_val;
    unsigned int poll_count;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_dphy_lanes_test run");

    /* Step 1: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel setting */
    LOGT("Step 1: Writing MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x0U);

    /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set the control data configuration */
    LOGT("Step 2: Writing MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0U);

    /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes */
    LOGT("Step 3: Writing MIZAR_MIPI_CSI2_HOST_N_LANES");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_N_LANES, 0x0U);

    /* Step 4: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait for PHY stop state condition */
    LOGT("Step 4: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE");
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

    /* Step 5: Write to hardcoded address 0xa0243ffc */
    LOGT("Step 5: Writing to hardcoded address 0xa0243ffc");
    writel_reg((uintptr_t)0xa0243ffcU, 0x0U);

    /* Step 6: Read from hardcoded address 0xE6001000 */
    LOGT("Step 6: Reading from hardcoded address 0xE6001000");
    read_val = (uint32_t)readl_reg((uintptr_t)0xE6001000U);
    LOGT("Read 0xE6001000 = 0x%lx", (unsigned long)read_val);

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
    LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    read_val = (uint32_t)readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN = 0x%lx", (unsigned long)read_val);

    /* Step 8: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    LOGT("Step 8: Masking PHY fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 9: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    LOGT("Step 9: Masking packet fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 10: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    LOGT("Step 10: Masking PHY interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ALL);

    /* Step 11: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    LOGT("Step 11: Masking line interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ALL);

    /* Step 12: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    LOGT("Step 12: Masking boundary frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    LOGT("Step 13: Masking sequence frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 14: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    LOGT("Step 14: Masking CRC frame fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 15: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    LOGT("Step 15: Masking payload CRC fatal interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ALL);

    /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    LOGT("Step 16: Masking data ID interrupts");
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ALL);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    LOGT("Step 17: Masking ECC corrected interrupts");
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
