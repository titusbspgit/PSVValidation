// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dphy_idi_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dphy_idi_test
 * Description: Validates the MIPI CSI2 DPHY IDI (Image Data Interface) functionality.
 *   Polls PHY_STOPSTATE, configures control_data, null_blank, and virtual_channel,
 *   accesses direct hex addresses 0xa0243ffc and 0xE6001000, reads INT_ST_MAIN,
 *   and enables all CSI2 host interrupt masks.
 */

typedef struct {
    unsigned int errors;
} mipi_dphy_idi_test_ctx_t;

static mipi_dphy_idi_test_ctx_t g_ctx;

/*
 * Function: mipi_dphy_idi_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dphy_idi_test_ctx_t){0};

    LOGT("mipi_dphy_idi_test init: starting DPHY IDI test");

    return 0;
}

/*
 * Function: mipi_dphy_idi_test_run
 * Description: Executes the main testcase flow for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t rd_data;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dphy_idi_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dphy_idi_test run: begin");

    /* Step 1: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE */
    LOGT("Step 1: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for stop state");
    timeout = MIPI_CSI2_PHY_STOPSTATE_TIMEOUT;
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while ((rd_data == 0U) && (timeout > 0U)) {
        rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("mipi_dphy_idi_test: PHY_STOPSTATE polling timeout");
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("PHY_STOPSTATE reached stop state: rd_data=0x%lx", (unsigned long)rd_data);
    }

    /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA (value 1) */
    LOGT("Step 2: Writing to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, MIPI_CSI2_CONTROL_DATA_VALUE);

    /* Step 3: Write to MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK (value 1) */
    LOGT("Step 3: Writing to MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_NULL_BLANK, MIPI_CSI2_NULL_BLANK_VALUE);

    /* Step 4: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL */
    LOGT("Step 4: Writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, MIPI_CSI2_VIRTUAL_CHANNEL_VALUE);

    /* Step 5: Write to 0xa0243ffc (direct hex address) */
    LOGT("Step 5: Writing to direct hex address 0xa0243ffc");
    writel_reg(MIPI_CSI2_EXTERNAL_WR_ADDR, MIPI_CSI2_EXTERNAL_WR_VALUE);

    /* Step 6: Read from 0xE6001000 (direct hex address) */
    LOGT("Step 6: Reading from direct hex address 0xE6001000");
    rd_data = readl_reg(MIPI_CSI2_EXTERNAL_RD_ADDR);
    LOGT("Read from 0xE6001000: rd_data=0x%lx", (unsigned long)rd_data);

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN */
    LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    rd_data = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("INT_ST_MAIN=0x%lx", (unsigned long)rd_data);

    /* Steps 8-17: Enable all CSI2 host interrupt masks */
    LOGT("Steps 8-17: Enabling all CSI2 host interrupt masks");

    LOGT("Step 8: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 9: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 10: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 11: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 12: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 13: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 14: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 15: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 16: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("Step 17: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, MIPI_CSI2_INT_MASK_ENABLE_ALL);

    LOGT("mipi_dphy_idi_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_dphy_idi_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_dphy_idi_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dphy_idi_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dphy_idi_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
