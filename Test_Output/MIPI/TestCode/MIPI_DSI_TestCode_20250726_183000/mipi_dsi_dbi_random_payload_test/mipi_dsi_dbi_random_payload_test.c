// Author - AI Force 2.3. 26-Jul-2025 18:30 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_dbi_random_payload_test
 * Description: MIPI DSI DBI random payload transfers using DMAC in a
 *              10-iteration loop. Configures DSI host PHY interface, packet
 *              handling, clock manager, disables DPI control, enables DMAC
 *              interrupts. In each iteration generates random payload data,
 *              programs DMAC debug instructions, initiates transfer, polls
 *              INTMIS for completion (0x3), and clears interrupt via INTCLR.
 */

static mipi_dsi_dbi_random_payload_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_dbi_random_payload_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_dsi_dbi_random_payload_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init: starting testcase initialization");

    return 0;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_run
 * Description: Executes the main testcase flow for mipi_dsi_dbi_random_payload_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t read_data;
    uint32_t random_payload;
    unsigned int timeout;
    unsigned int iter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting DBI random payload transfer via DMAC");

    /* Step 1: Configure DSI host PHY interface (set PHY_STOP_WAIT_TIME field) */
    LOGT("Step 1: Write MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure PHY interface");
    read_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    // MANUAL_REVIEW: PHY_STOP_WAIT_TIME field mask and value not explicitly provided in Meta TestPlan JSON.
    read_data = set_data_mask(read_data, MIPI_DSI_PHY_STOP_WAIT_TIME_MASK, MIPI_DSI_PHY_STOP_WAIT_TIME_VAL);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, read_data);
    LOGT("Step 1: MIZAR_MIPI_DSI_HOST_PHY_IF_CFG written = 0x%lx", (unsigned long)read_data);

    /* Step 2: Configure packet handling by writing to PCKHDL_CFG */
    LOGT("Step 2: Write MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling");
    // MANUAL_REVIEW: Exact write value for PCKHDL_CFG not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_PCKHDL_CFG_VAL);

    /* Step 3: Configure clock manager settings by writing to CLKMGR_CFG */
    LOGT("Step 3: Write MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager");
    // MANUAL_REVIEW: Exact write value for CLKMGR_CFG not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_CLKMGR_CFG_VAL);

    /* Step 4: Disable DPI control for command mode operation */
    LOGT("Step 4: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 5: Enable DMAC channel interrupts by writing 0x3 to INTEN */
    LOGT("Step 5: Write MIZAR_MIPI_DSI_DMAC_INTEN = 0x3 to enable DMAC channel interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Steps 6-12: 10-iteration loop with random payload data */
    LOGT("Starting %u-iteration random payload transfer loop", MIPI_DSI_NUM_ITERATIONS);

    for (iter = 0U; iter < MIPI_DSI_NUM_ITERATIONS; iter++) {

        /* Step 6: Generate random payload data for the current iteration */
        random_payload = (uint32_t)((iter + 1U) * 0xDEAD0000U) ^ 0xBEEF0000U;
        // MANUAL_REVIEW: Random payload generation method not specified in Meta TestPlan JSON.
        // Using a deterministic pattern based on iteration index for reproducibility.
        LOGT("Iteration %u/%u: random_payload = 0x%lx",
             iter + 1U, MIPI_DSI_NUM_ITERATIONS, (unsigned long)random_payload);

        /* Step 7: Program first DMA debug instruction word (DBGINST0) */
        LOGT("Step 7: Write MIZAR_MIPI_DSI_DMAC_DBGINST0 with first debug instruction word");
        // MANUAL_REVIEW: Exact DBGINST0 value (DMA channel thread and instruction encoding) not provided in Meta TestPlan JSON.
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, MIPI_DSI_DMAC_DBGINST0_VAL);

        /* Step 8: Program second DMA debug instruction word (DBGINST1) */
        LOGT("Step 8: Write MIZAR_MIPI_DSI_DMAC_DBGINST1 with second debug instruction word");
        // MANUAL_REVIEW: Exact DBGINST1 value (descriptor address) not provided in Meta TestPlan JSON.
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

        /* Step 9: Execute DMA debug command to initiate data transfer */
        LOGT("Step 9: Write MIZAR_MIPI_DSI_DMAC_DBGCMD to execute debug instruction and start DMA transfer");
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

        /* Step 10: Poll MIZAR_MIPI_DSI_DMAC_INTMIS until expected value 0x3 is detected */
        LOGT("Step 10: Polling MIZAR_MIPI_DSI_DMAC_INTMIS for DMA completion (expected 0x3)");
        timeout = MIPI_DSI_POLL_TIMEOUT;
        do {
            read_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
            if (timeout == 0U) {
                LOGE("Iteration %u: Timeout polling MIZAR_MIPI_DSI_DMAC_INTMIS for DMA completion",
                     iter + 1U);
                g_ctx.errors++;
                out->status = -1;
                return out->status;
            }
            timeout--;
        } while (read_data != MIPI_DSI_EXPECTED_INTMIS);
        LOGT("Iteration %u: MIZAR_MIPI_DSI_DMAC_INTMIS = 0x%lx, DMA transfer complete",
             iter + 1U, (unsigned long)read_data);

        /* Validation: Verify INTMIS equals expected value 0x3 */
        if (read_data == MIPI_DSI_EXPECTED_INTMIS) {
            LOGT("Iteration %u: Validation PASS: INTMIS = 0x%lx indicates both DMA channels completed",
                 iter + 1U, (unsigned long)read_data);
            g_ctx.checks_passed++;
        } else {
            LOGE("Iteration %u: Validation FAIL: INTMIS = 0x%lx, expected 0x%lx",
                 iter + 1U, (unsigned long)read_data, (unsigned long)MIPI_DSI_EXPECTED_INTMIS);
            g_ctx.errors++;
        }
        g_ctx.checks_total++;

        /* Step 11: Clear DMAC interrupt by writing to INTCLR */
        LOGT("Step 11: Write MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMAC interrupt");
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, MIPI_DSI_EXPECTED_INTMIS);

        LOGT("Iteration %u/%u complete", iter + 1U, MIPI_DSI_NUM_ITERATIONS);
    }

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_dbi_random_payload_test run complete: %s errors=%u checks_passed=%u checks_total=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return out->status;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_dsi_dbi_random_payload_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u checks_passed=%u checks_total=%u",
         g_ctx.errors, g_ctx.checks_passed, g_ctx.checks_total);
    return g_ctx.errors == 0U ? 0 : -1;
}
