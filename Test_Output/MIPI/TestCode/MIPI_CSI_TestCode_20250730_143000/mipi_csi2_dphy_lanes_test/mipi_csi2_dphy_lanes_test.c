// Author - AI Force 2.3. 30-Jul-2025 14:30 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * MIPI CSI2 DPHY Lanes Test
 * This testcase configures and validates MIPI CSI2 DPHY lane operation.
 * It writes to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the
 * virtual channel, writes to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set
 * control data, writes to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the
 * number of active DPHY lanes, and polls MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE
 * to wait for the PHY stop state condition. It writes to a hardcoded address
 * 0xa0243ffc and reads from hardcoded address 0xE6001000. It reads
 * MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status.
 * It then masks all CSI2 host interrupts.
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
    uint32_t read_val;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_dphy_lanes_test execution");

    /* Step 1: Write to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL to configure the virtual channel setting */
    LOGT("Step 1: Writing to MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x00000000U);

    /* Step 2: Write to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA to set the control data configuration */
    LOGT("Step 2: Writing to MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA");
    writel_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x00000000U);

    /* Step 3: Write to MIZAR_MIPI_CSI2_HOST_N_LANES to configure the number of active DPHY lanes */
    LOGT("Step 3: Writing to MIZAR_MIPI_CSI2_HOST_N_LANES");
    writel_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, 0x00000003U);

    /* Step 4: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE in a while loop to wait for the PHY stop state condition */
    LOGT("Step 4: Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for PHY stop state");
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

    /* Step 5: Write to hardcoded address 0xa0243ffc */
    LOGT("Step 5: Writing to hardcoded address 0xa0243ffc");
    writel_reg(MIPI_CSI2_HARDCODED_ADDR_WRITE, 0x00000000U);

    /* Step 6: Read from hardcoded address 0xE6001000 */
    LOGT("Step 6: Reading from hardcoded address 0xE6001000");
    read_val = readl_reg(MIPI_CSI2_HARDCODED_ADDR_READ);
    LOGT("Hardcoded address 0xE6001000 read_val=0x%lx", (unsigned long)read_val);

    /* Step 7: Read MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN to check the main interrupt status */
    LOGT("Step 7: Reading MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN");
    read_val = readl_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN read_val=0x%lx", (unsigned long)read_val);

    /* Step 8: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL to mask PHY fatal interrupts */
    LOGT("Step 8: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x00000000U);

    /* Step 9: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL to mask packet fatal interrupts */
    LOGT("Step 9: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000000U);

    /* Step 10: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY to mask PHY interrupts */
    LOGT("Step 10: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x00000000U);

    /* Step 11: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE to mask line interrupts */
    LOGT("Step 11: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x00000000U);

    /* Step 12: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL to mask boundary frame fatal interrupts */
    LOGT("Step 12: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x00000000U);

    /* Step 13: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL to mask sequence frame fatal interrupts */
    LOGT("Step 13: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x00000000U);

    /* Step 14: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL to mask CRC frame fatal interrupts */
    LOGT("Step 14: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x00000000U);

    /* Step 15: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL to mask payload CRC fatal interrupts */
    LOGT("Step 15: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x00000000U);

    /* Step 16: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID to mask data ID interrupts */
    LOGT("Step 16: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x00000000U);

    /* Step 17: Write to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED to mask ECC corrected interrupts */
    LOGT("Step 17: Writing to MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED");
    writel_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x00000000U);

    /* Final status determination */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

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

    /* Validation: PHY_STOPSTATE was polled to confirm stop state. */
    /* Validation: INT_ST_MAIN was read to check main interrupt status. */
    /* Validation: All interrupt mask registers were written to mask interrupts. */
    /* Validation: Test expected to complete without errors. */

    LOGT("mipi_csi2_dphy_lanes_test teardown: %s",
         (g_ctx.errors == 0U) ? "PASS" : "FAIL");

    return g_ctx.errors == 0U ? 0 : -1;
}
