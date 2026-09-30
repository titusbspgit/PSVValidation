// Author - AI Force 2.3. 01-Jul-2025 07:18 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Testcase: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI2 D-PHY lane configuration by iterating
 *              through different lane counts (4 down to 1). For each lane
 *              count, the test configures virtual channel, control data,
 *              number of active lanes, polls PHY stop state, performs
 *              system-level write/read, reads main interrupt status, and
 *              masks all interrupt sources.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
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

    LOGT("MIPI CSI2 D-PHY lanes test init: starting initialization");

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
    unsigned int lane_count;
    uint32_t rd_val;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("MIPI CSI2 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting MIPI CSI2 D-PHY lane configuration test");

    /* Step 18: Iterate lane configurations from 4 lanes down to 1 lane */
    for (lane_count = MIPI_CSI2_MAX_LANES; lane_count >= MIPI_CSI2_MIN_LANES; lane_count--) {

        LOGT("Lane iteration: configuring %u active lane(s)", lane_count);

        /* Step 1: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure virtual channel settings */
        LOGT("Step 1: Writing MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
        writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIPI_CSI2_VIRTUAL_CHANNEL_CFG);
        g_ctx.checks_total++;

        /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data configuration */
        LOGT("Step 2: Writing MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
        writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, MIPI_CSI2_CONTROL_DATA_CFG);
        g_ctx.checks_total++;

        /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure number of active D-PHY data lanes */
        LOGT("Step 3: Writing MIZAR_MIPI_CSI2_HOST_N_LANES with lane_count=%u", lane_count);
        writel_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (uint32_t)(lane_count - 1U));
        g_ctx.checks_total++;

        /* Step 4: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE to wait until PHY stop state is achieved */
        LOGT("Step 4: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for stop state");
        timeout = MIPI_CSI2_POLL_TIMEOUT;
        do {
            rd_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
            if (timeout == 0U) {
                LOGE("Timeout polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for lane_count=%u", lane_count);
                g_ctx.errors++;
                break;
            }
            timeout--;
        } while (rd_val == 0U);

        if (timeout > 0U) {
            LOGT("PHY stop state achieved: rd_val=0x%lx for lane_count=%u",
                 (unsigned long)rd_val, lane_count);
            g_ctx.checks_passed++;
        } else {
            g_ctx.checks_failed++;
        }
        g_ctx.checks_total++;

        /* Step 5: Write to 0xa0243ffc for system-level or DMA configuration */
        LOGT("Step 5: Writing to 0xa0243ffc for system-level configuration");
        writel_reg(MIPI_CSI2_SYS_CFG_ADDR, MIPI_CSI2_SYS_CFG_VAL);
        g_ctx.checks_total++;

        /* Step 6: Read from 0xE6001000 for system-level status or configuration verification */
        LOGT("Step 6: Reading from 0xE6001000 for system-level status");
        rd_val = readl_reg(MIPI_CSI2_SYS_STATUS_ADDR);
        LOGT("System-level status read: rd_val=0x%lx", (unsigned long)rd_val);
        g_ctx.checks_total++;

        /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
        LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
        rd_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
        LOGT("INT_ST_MAIN read: rd_val=0x%lx", (unsigned long)rd_val);
        if (rd_val != 0U) {
            LOGE("Unexpected interrupts asserted in INT_ST_MAIN=0x%lx for lane_count=%u",
                 (unsigned long)rd_val, lane_count);
            g_ctx.errors++;
            g_ctx.checks_failed++;
        } else {
            g_ctx.checks_passed++;
        }
        g_ctx.checks_total++;

        /* Step 8: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
        LOGT("Step 8: Masking PHY fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 9: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
        LOGT("Step 9: Masking packet fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 10: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
        LOGT("Step 10: Masking PHY interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ALL);

        /* Step 11: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
        LOGT("Step 11: Masking line interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ALL);

        /* Step 12: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
        LOGT("Step 12: Masking boundary frame fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
        LOGT("Step 13: Masking sequence frame fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 14: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
        LOGT("Step 14: Masking CRC frame fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 15: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
        LOGT("Step 15: Masking payload CRC fatal interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ALL);

        /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
        LOGT("Step 16: Masking data ID interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ALL);

        /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
        LOGT("Step 17: Masking ECC corrected interrupts");
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ALL);

        LOGT("Lane iteration complete for lane_count=%u", lane_count);
    }

    /* Step 19: Final status update */
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

    LOGT("MIPI CSI2 D-PHY lanes test teardown: errors=%u checks_total=%u",
         g_ctx.errors, g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
