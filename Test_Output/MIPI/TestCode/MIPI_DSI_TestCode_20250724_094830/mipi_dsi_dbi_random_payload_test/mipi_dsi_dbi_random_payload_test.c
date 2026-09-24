// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_dbi_random_payload_test
 * Description: This testcase performs MIPI DSI DBI random payload transfers
 *              using the DMAC in a 10-iteration loop. It configures the DSI
 *              host PHY interface via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, configures
 *              packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, configures the
 *              clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, disables DPI
 *              control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, enables DMAC
 *              interrupts via MIZAR_MIPI_DSI_DMAC_INTEN. In each iteration,
 *              random payload data is generated, the DMAC is programmed via
 *              MIZAR_MIPI_DSI_DMAC_DBGINST0 and MIZAR_MIPI_DSI_DMAC_DBGINST1,
 *              the transfer is initiated via MIZAR_MIPI_DSI_DMAC_DBGCMD,
 *              MIZAR_MIPI_DSI_DMAC_INTMIS is polled for DMA completion, and
 *              MIZAR_MIPI_DSI_DMAC_INTCLR is written to clear the interrupt.
 */

typedef struct {
    unsigned int errors;
    unsigned int iterations_completed;
} mipi_dsi_dbi_random_payload_test_ctx_t;

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

    LOGT("mipi_dsi_dbi_random_payload_test_init: initialization complete");

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
    uint32_t read_val;
    unsigned int timeout;
    unsigned int iter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test_run: starting MIPI DSI DBI random payload transfer");

    /* Step 1: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface (set PHY_STOP_WAIT_TIME field) */
    // MANUAL_REVIEW: Exact value for PHY_STOP_WAIT_TIME field not provided in Meta TestPlan JSON.
    LOGT("Step 1: Write MIZAR_MIPI_DSI_HOST_PHY_IF_CFG (PHY_STOP_WAIT_TIME)");
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, MIPI_DSI_PHY_IF_CFG_VAL);

    /* Step 2: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    // MANUAL_REVIEW: Exact value for PCKHDL_CFG not provided in Meta TestPlan JSON.
    LOGT("Step 2: Write MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_PCKHDL_CFG_VAL);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    // MANUAL_REVIEW: Exact value for CLKMGR_CFG not provided in Meta TestPlan JSON.
    LOGT("Step 3: Write MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_CLKMGR_CFG_VAL);

    /* Step 4: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for command mode */
    LOGT("Step 4: Write MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL = 0x0 (disable DPI)");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 5: Write to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3 to enable DMAC channel interrupts */
    LOGT("Step 5: Write MIZAR_MIPI_DSI_DMAC_INTEN = 0x3");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Steps 6-12: 10-iteration loop with random payload data */
    LOGT("Starting %u-iteration random payload transfer loop", MIPI_DSI_DBI_ITERATION_COUNT);

    for (iter = 0U; iter < MIPI_DSI_DBI_ITERATION_COUNT; iter++) {

        LOGT("Iteration %u of %u", iter + 1U, MIPI_DSI_DBI_ITERATION_COUNT);

        /* Step 6: Generate random payload data for the current iteration */
        // MANUAL_REVIEW: Random payload generation mechanism not specified in Meta TestPlan JSON.
        // The payload values for DBGINST0 and DBGINST1 should be generated or provided per iteration.

        /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with first debug instruction word */
        // MANUAL_REVIEW: Exact debug instruction word for DBGINST0 not provided in Meta TestPlan JSON.
        LOGT("Step 7: Write MIZAR_MIPI_DSI_DMAC_DBGINST0 (iter=%u)", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, MIPI_DSI_DMAC_DBGINST0_VAL);

        /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with second debug instruction word */
        // MANUAL_REVIEW: Exact debug instruction word for DBGINST1 not provided in Meta TestPlan JSON.
        LOGT("Step 8: Write MIZAR_MIPI_DSI_DMAC_DBGINST1 (iter=%u)", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

        /* Step 9: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute debug instruction and start DMA transfer */
        LOGT("Step 9: Write MIZAR_MIPI_DSI_DMAC_DBGCMD = 0x0 to start DMA (iter=%u)", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

        /* Step 10: Poll MIZAR_MIPI_DSI_DMAC_INTMIS until expected value 0x3 is detected */
        // MANUAL_REVIEW: Timeout value not provided by FV Template or Meta TestPlan JSON.
        // Using bounded loop to avoid infinite polling. Adjust MIPI_DSI_POLL_TIMEOUT as needed.
        LOGT("Step 10: Poll MIZAR_MIPI_DSI_DMAC_INTMIS for 0x%lx (iter=%u)",
             (unsigned long)MIPI_DSI_DMAC_INTMIS_EXPECTED, iter);
        timeout = MIPI_DSI_POLL_TIMEOUT;
        do {
            read_val = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
            if (read_val == MIPI_DSI_DMAC_INTMIS_EXPECTED) {
                break;
            }
            timeout--;
        } while (timeout > 0U);

        if (timeout == 0U) {
            LOGE("Step 10: Timeout polling MIZAR_MIPI_DSI_DMAC_INTMIS iter=%u last=0x%lx",
                 iter, (unsigned long)read_val);
            g_ctx.errors++;
        } else {
            LOGT("Step 10: INTMIS=0x%lx detected (iter=%u)",
                 (unsigned long)read_val, iter);
        }

        /* Step 11: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC interrupt */
        LOGT("Step 11: Write MIZAR_MIPI_DSI_DMAC_INTCLR = 0x%lx (iter=%u)",
             (unsigned long)MIPI_DSI_DMAC_INTMIS_EXPECTED, iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, MIPI_DSI_DMAC_INTMIS_EXPECTED);

        /* Step 12: End of loop iteration */
        g_ctx.iterations_completed++;
    }

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_dbi_random_payload_test_run complete: %s errors=%u iterations_completed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.iterations_completed);

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

    LOGT("mipi_dsi_dbi_random_payload_test_teardown: errors=%u iterations_completed=%u",
         g_ctx.errors, g_ctx.iterations_completed);

    return g_ctx.errors == 0U ? 0 : -1;
}
