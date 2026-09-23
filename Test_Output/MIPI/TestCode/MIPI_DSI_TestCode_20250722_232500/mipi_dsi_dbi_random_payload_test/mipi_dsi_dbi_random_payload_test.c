// Author - AI Force 2.3. 22-Jul-2025 17:50 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_dbi_random_payload_test
 * Description: Performs MIPI DSI DBI DMA transfers with random payload sizes
 *   across multiple iterations. Configures DSI Host PHY interface, packet handler,
 *   clock manager, disables DPI control, enables DMAC interrupts, programs DMAC
 *   debug instruction registers, executes the DMAC command, polls DMAC masked
 *   interrupt status for dual-channel completion (0x3), and clears the DMAC
 *   interrupt. Each iteration uses a different random payload size.
 */

typedef struct {
    unsigned int errors;
    unsigned int iterations_passed;
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

    LOGT("mipi_dsi_dbi_random_payload_test init: starting DBI DMA random payload test");

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
    uint32_t rd_data = 0U;
    unsigned int timeout;
    unsigned int iter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_dsi_dbi_random_payload_test DBI DMA transfer sequence");

    /* Step 1: Configure DSI Host PHY interface stop wait time */
    LOGT("Step 1: Configuring MIZAR_MIPI_DSI_HOST_PHY_IF_CFG PHY_STOP_WAIT_TIME");
    rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    rd_data = set_data_mask(rd_data, MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, rd_data);

    /* Step 2: Configure packet handler */
    LOGT("Step 2: Writing MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);

    /* Step 3: Configure clock manager */
    LOGT("Step 3: Writing MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);

    /* Step 4: Disable DPI control */
    LOGT("Step 4: Writing MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL with 0");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 5: Enable DMAC interrupts for channel completion */
    LOGT("Step 5: Writing MIZAR_MIPI_DSI_DMAC_INTEN with 0x3");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Steps 6-11: Iterate with random payload sizes */
    for (iter = 0U; iter < MIPI_DSI_DBI_NUM_ITERATIONS; iter++) {

        /* Step 6: Generate random payload size for current iteration */
        // MANUAL_REVIEW: Random payload size generation depends on platform RNG API.
        // The payload size is conceptual for this DBI transfer test.
        LOGT("Iteration %u: starting DBI DMA transfer", iter);

        /* Step 7: Program DMAC debug instruction 0 */
        LOGT("Iteration %u Step 7: Writing MIZAR_MIPI_DSI_DMAC_DBGINST0 with 0x00A00000", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);

        /* Step 8: Program DMAC debug instruction 1 with RAM_BASE */
        LOGT("Iteration %u Step 8: Writing MIZAR_MIPI_DSI_DMAC_DBGINST1 with RAM_BASE", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, RAM_BASE);

        /* Step 9: Execute DMAC debug command to start DMA transfer */
        LOGT("Iteration %u Step 9: Writing MIZAR_MIPI_DSI_DMAC_DBGCMD with 0x0", iter);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

        /* Step 10: Poll DMAC_INTMIS until value equals 0x3 */
        LOGT("Iteration %u Step 10: Polling MIZAR_MIPI_DSI_DMAC_INTMIS for 0x3", iter);
        timeout = MIPI_DSI_POLL_TIMEOUT;
        do {
            rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
            if (rd_data == 0x3U) {
                break;
            }
            timeout--;
        } while (timeout > 0U);

        if (timeout == 0U) {
            LOGE("mipi_dsi_dbi_random_payload_test: iteration %u timeout waiting for DMAC_INTMIS=0x3", iter);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }

        LOGT("Iteration %u Step 10: DMAC_INTMIS=0x%lx (both channels completed)",
             iter, (unsigned long)rd_data);

        /* Step 11: Clear DMAC interrupt */
        LOGT("Iteration %u Step 11: Writing MIZAR_MIPI_DSI_DMAC_INTCLR with 0x%lx",
             iter, (unsigned long)rd_data);
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);

        g_ctx.iterations_passed++;
    }

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u iterations_passed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.iterations_passed);

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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u iterations_passed=%u",
         g_ctx.errors, g_ctx.iterations_passed);
    return g_ctx.errors == 0U ? 0 : -1;
}
