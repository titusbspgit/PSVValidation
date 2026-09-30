// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI2 D-PHY lane configuration by iterating
 *   through different lane counts (4 down to 1). Configures virtual channel,
 *   control data, N_LANES, polls PHY_STOPSTATE, performs system-level accesses,
 *   reads INT_ST_MAIN, and masks all CSI2 host interrupt sources.
 */

/* Maximum lane count for iteration */
#define MIPI_CSI2_MAX_LANES  4U
#define MIPI_CSI2_MIN_LANES  1U

/* System-level register addresses from impacted registers */
#define MIPI_CSI2_SYS_DMA_CFG_ADDR   0xa0243ffcUL
#define MIPI_CSI2_SYS_STATUS_ADDR    0xE6001000UL

/* Polling timeout count following FV Template timeout style */
#define MIPI_CSI2_PHY_POLL_TIMEOUT   1000U

typedef struct {
    unsigned int errors;
    unsigned int lanes_tested;
} mipi_csi2_dphy_test_ctx_t;

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

    LOGT("mipi_csi2_dphy_lanes_test init: starting D-PHY lane configuration test");

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
    unsigned int timeout;
    uint32_t reg_val;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting MIPI CSI2 D-PHY lane configuration test");

    /* Step 18: Repeat steps 1-17 for each lane configuration (4 lanes down to 1 lane) */
    for (lane_count = MIPI_CSI2_MAX_LANES; lane_count >= MIPI_CSI2_MIN_LANES; lane_count--) {

        LOGT("Configuring D-PHY for %u lane(s)", lane_count);

        /* Step 1: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure virtual channel settings */
        writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, lane_count);
        LOGT("Wrote virtual channel register for lane_count=%u", lane_count);

        /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set control data configuration */
        writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, lane_count);
        LOGT("Wrote control data register for lane_count=%u", lane_count);

        /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure active D-PHY data lanes */
        writel_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (lane_count - 1U));
        LOGT("Wrote N_LANES=%u (lane_count=%u)", (lane_count - 1U), lane_count);

        /* Step 4: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until PHY stop state is achieved */
        timeout = MIPI_CSI2_PHY_POLL_TIMEOUT;
        do {
            reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
            if (reg_val != 0U) {
                break;
            }
            timeout--;
        } while (timeout > 0U);

        if (timeout == 0U) {
            LOGE("PHY_STOPSTATE polling timeout for lane_count=%u", lane_count);
            g_ctx.errors++;
        } else {
            LOGT("PHY_STOPSTATE confirmed for lane_count=%u, value=0x%x", lane_count, reg_val);
        }

        /* Step 5: Write to 0xa0243ffc for system-level or DMA configuration */
        writel_reg(MIPI_CSI2_SYS_DMA_CFG_ADDR, lane_count);
        LOGT("Wrote system-level DMA config register 0xa0243ffc");

        /* Step 6: Read from 0xE6001000 for system-level status or configuration verification */
        reg_val = readl_reg(MIPI_CSI2_SYS_STATUS_ADDR);
        LOGT("Read system-level status register 0xE6001000, value=0x%x", reg_val);

        /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check main interrupt status */
        reg_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
        LOGT("INT_ST_MAIN=0x%x for lane_count=%u", reg_val, lane_count);

        /* Step 8: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0U);

        /* Step 9: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0U);

        /* Step 10: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0U);

        /* Step 11: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0U);

        /* Step 12: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0U);

        /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0U);

        /* Step 14: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0U);

        /* Step 15: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0U);

        /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0U);

        /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
        writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0U);

        LOGT("All interrupt mask registers written for lane_count=%u", lane_count);

        g_ctx.lanes_tested++;
    }

    /* Step 19: Validate that no errors occurred during the lane configuration iterations */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u lanes_tested=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.lanes_tested);

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

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u lanes_tested=%u",
         g_ctx.errors, g_ctx.lanes_tested);

    return g_ctx.errors == 0U ? 0 : -1;
}
