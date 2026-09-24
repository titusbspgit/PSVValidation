// Author - AI Force 2.3. 26-Jul-2025 18:05 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_basic_test
 * Description: Basic MIPI DSI DBI command mode transfer using DMAC with
 *              subsystem and DMAC interrupt handling. Enables DMAC and subsystem
 *              interrupts, configures DSI host PHY interface, packet handling,
 *              and clock manager, disables DPI control, programs and executes a
 *              DMA transfer via DMAC debug instruction registers, polls for
 *              interrupt assertion, and clears both DMAC and subsystem interrupts.
 */

static mipi_dsi_basic_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_basic_test_ctx_t){0};

    LOGT("mipi_dsi_basic_test init: starting testcase initialization");

    return 0;
}

/*
 * Function: mipi_dsi_basic_test_run
 * Description: Executes the main testcase flow for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t read_data;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting DBI command mode transfer via DMAC");

    /* Step 1: Enable DMAC channel interrupts by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN */
    LOGT("Step 1: Write MIZAR_MIPI_DSI_DMAC_INTEN = 0x3 to enable DMAC channel interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 2: Enable GDMA interrupt in subsystem interrupt_enable register */
    LOGT("Step 2: Write MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable GDMA interrupt");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, 0x1U);

    /* Step 3: Configure DSI host PHY interface (set PHY_STOP_WAIT_TIME field) */
    LOGT("Step 3: Configure MIZAR_MIPI_DSI_HOST_PHY_IF_CFG via read-modify-write");
    read_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    // MANUAL_REVIEW: PHY_STOP_WAIT_TIME field mask and value not explicitly provided in Meta TestPlan JSON.
    // Preserving set_data_mask style from test steps description.
    // set_data_mask(read_data, mask, value) is assumed available from framework.
    read_data = set_data_mask(read_data, MIPI_DSI_PHY_STOP_WAIT_TIME_MASK, MIPI_DSI_PHY_STOP_WAIT_TIME_VAL);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, read_data);
    LOGT("Step 3: MIZAR_MIPI_DSI_HOST_PHY_IF_CFG written = 0x%lx", (unsigned long)read_data);

    /* Step 4: Configure packet handling by writing to PCKHDL_CFG */
    LOGT("Step 4: Configure MIZAR_MIPI_DSI_HOST_PCKHDL_CFG for packet handling");
    // MANUAL_REVIEW: Exact write value for PCKHDL_CFG not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_PCKHDL_CFG_VAL);

    /* Step 5: Configure clock manager settings by writing to CLKMGR_CFG */
    LOGT("Step 5: Configure MIZAR_MIPI_DSI_HOST_CLKMGR_CFG for clock manager");
    // MANUAL_REVIEW: Exact write value for CLKMGR_CFG not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_CLKMGR_CFG_VAL);

    /* Step 6: Disable DPI control for command mode operation */
    LOGT("Step 6: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 7: Program first DMA debug instruction word (DBGINST0) */
    LOGT("Step 7: Write MIZAR_MIPI_DSI_DMAC_DBGINST0 with first debug instruction word");
    // MANUAL_REVIEW: Exact DBGINST0 value (DMA channel thread and instruction encoding) not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, MIPI_DSI_DMAC_DBGINST0_VAL);

    /* Step 8: Program second DMA debug instruction word (DBGINST1) */
    LOGT("Step 8: Write MIZAR_MIPI_DSI_DMAC_DBGINST1 with second debug instruction word");
    // MANUAL_REVIEW: Exact DBGINST1 value not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

    /* Step 9: Execute DMA debug command to initiate data transfer */
    LOGT("Step 9: Write MIZAR_MIPI_DSI_DMAC_DBGCMD to execute debug instruction and start DMA transfer");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 10: Poll MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for subsystem interrupt assertion */
    LOGT("Step 10: Polling MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for GDMA interrupt assertion");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    do {
        read_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
        if (timeout == 0U) {
            LOGE("Step 10: Timeout polling MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for GDMA interrupt");
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
        timeout--;
    } while ((read_data & MIPI_DSI_GDMA_INTERRUPT_BIT) == 0U);
    LOGT("Step 10: MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK = 0x%lx, GDMA interrupt detected", (unsigned long)read_data);

    /* Step 11: Read MIZAR_MIPI_DSI_DMAC_INTMIS to verify DMAC masked interrupt status */
    LOGT("Step 11: Reading MIZAR_MIPI_DSI_DMAC_INTMIS to verify DMA interrupt status");
    read_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    LOGT("Step 11: MIZAR_MIPI_DSI_DMAC_INTMIS = 0x%lx", (unsigned long)read_data);

    /* Validation: Verify DMAC INTMIS indicates transfer completion */
    if (read_data == 0U) {
        LOGE("Validation FAIL: MIZAR_MIPI_DSI_DMAC_INTMIS = 0x0, expected non-zero for transfer completion");
        g_ctx.errors++;
    } else {
        LOGT("Validation PASS: MIZAR_MIPI_DSI_DMAC_INTMIS = 0x%lx indicates transfer completion", (unsigned long)read_data);
    }

    /* Step 12: Clear DMAC interrupt by writing read INTMIS value to INTCLR */
    LOGT("Step 12: Write MIZAR_MIPI_DSI_DMAC_INTCLR = 0x%lx to clear DMAC interrupt", (unsigned long)read_data);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, read_data);

    /* Step 13: Clear subsystem interrupt by writing to interrupt_raw */
    LOGT("Step 13: Write MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, MIPI_DSI_GDMA_INTERRUPT_BIT);

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_basic_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_dsi_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_basic_test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
