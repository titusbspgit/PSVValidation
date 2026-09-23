// Author - AI Force 2.3. 22-Jul-2025 23:25 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_basic_test
 * Description: Performs a basic MIPI DSI DBI DMA transfer operation.
 *   Configures DMAC interrupt enable, subsystem interrupt, DSI Host PHY interface,
 *   packet handler, clock manager, disables DPI control, programs DMAC debug
 *   instruction registers, executes the DMAC command, polls for GDMA interrupt,
 *   reads DMAC masked interrupt status, clears DMAC and subsystem interrupts.
 */

typedef struct {
    unsigned int errors;
} mipi_dsi_basic_test_ctx_t;

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

    LOGT("mipi_dsi_basic_test init: starting basic DBI DMA transfer test");

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
    uint32_t rd_data = 0U;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_dsi_basic_test DBI DMA transfer sequence");

    /* Step 1: Enable DMAC interrupts for channel completion */
    LOGT("Step 1: Writing MIZAR_MIPI_DSI_DMAC_INTEN with 0x3");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 2: Enable subsystem GDMA interrupt */
    LOGT("Step 2: Enabling GDMA interrupt in MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE");
    rd_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE);
    rd_data |= MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR;
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, rd_data);

    /* Step 3: Configure DSI Host PHY interface stop wait time */
    LOGT("Step 3: Configuring MIZAR_MIPI_DSI_HOST_PHY_IF_CFG PHY_STOP_WAIT_TIME");
    rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    rd_data = set_data_mask(rd_data, MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, rd_data);

    /* Step 4: Configure packet handler */
    LOGT("Step 4: Writing MIZAR_MIPI_DSI_HOST_PCKHDL_CFG with 0x3d");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);

    /* Step 5: Configure clock manager */
    LOGT("Step 5: Writing MIZAR_MIPI_DSI_HOST_CLKMGR_CFG with 0x107");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);

    /* Step 6: Disable DPI control */
    LOGT("Step 6: Writing MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL with 0");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 7: Program DMAC debug instruction 0 */
    LOGT("Step 7: Writing MIZAR_MIPI_DSI_DMAC_DBGINST0 with 0x00A00000");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);

    /* Step 8: Program DMAC debug instruction 1 with RAM_BASE */
    LOGT("Step 8: Writing MIZAR_MIPI_DSI_DMAC_DBGINST1 with RAM_BASE");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, RAM_BASE);

    /* Step 9: Execute DMAC debug command to start DMA transfer */
    LOGT("Step 9: Writing MIZAR_MIPI_DSI_DMAC_DBGCMD with 0x0");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 10: Poll INTERRUPT_MASK for GDMA interrupt assertion */
    LOGT("Step 10: Polling MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for GDMA_INTR bit");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    do {
        rd_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
        if ((rd_data & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);

    if (timeout == 0U) {
        LOGE("mipi_dsi_basic_test: timeout waiting for GDMA interrupt in INTERRUPT_MASK");
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }

    LOGT("Step 10: GDMA interrupt detected in INTERRUPT_MASK=0x%lx",
         (unsigned long)rd_data);

    /* Step 11: Read DMAC masked interrupt status */
    LOGT("Step 11: Reading MIZAR_MIPI_DSI_DMAC_INTMIS");
    rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    LOGT("Step 11: DMAC_INTMIS=0x%lx", (unsigned long)rd_data);

    /* Step 12: Clear DMAC interrupt */
    LOGT("Step 12: Writing MIZAR_MIPI_DSI_DMAC_INTCLR with 0x%lx",
         (unsigned long)rd_data);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);

    /* Step 13: Clear subsystem interrupt */
    LOGT("Step 13: Writing MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW with GDMA_INTR bit");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR);

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
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
