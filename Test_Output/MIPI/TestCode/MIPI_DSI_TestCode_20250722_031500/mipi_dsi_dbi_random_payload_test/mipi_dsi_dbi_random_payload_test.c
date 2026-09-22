// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_dbi_random_payload_test
 * Description: This testcase performs a MIPI DSI DBI random payload transfer using DMA.
 *              It configures DSI host PHY interface, packet handling, clock manager, and
 *              DPI control. DMAC interrupts are enabled, DMA channel is programmed with
 *              random payload and executed. The test polls DMAC masked interrupt status
 *              for completion, then clears the DMA interrupt.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
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
    uint32_t read_data = 0U;
    uint32_t timeout = 0U;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test_run: starting test execution");

    /* Step 1: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with PHY_STOP_WAIT_TIME */
    LOGT("Step 1: Configuring MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with PHY stop wait time");
    read_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    read_data = read_data | MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME;
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, read_data);

    /* Step 2: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    LOGT("Step 2: Writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x1U);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    LOGT("Step 3: Writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x1U);

    /* Step 4: Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control settings */
    LOGT("Step 4: Writing to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x1U);

    /* Step 5: Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts */
    LOGT("Step 5: Writing to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x1U);

    /* Step 6: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA instruction byte 0 (random payload) */
    LOGT("Step 6: Writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA instruction byte 0 (random payload)");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, (DSI_WRITE_MEMORY_START | (DMA_SAR << 8) | (DMA_DAR << 16)));

    /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA channel instruction byte 1 */
    LOGT("Step 7: Writing to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA instruction byte 1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, 0x0U);

    /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug instruction */
    LOGT("Step 8: Writing to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute DMA debug instruction");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 9: Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a loop for DMA transfer completion */
    LOGT("Step 9: Polling MIZAR_MIPI_DSI_DMAC_INTMIS for DMA transfer completion");
    timeout = MIPI_DSI_DBI_RANDOM_PAYLOAD_POLL_TIMEOUT;
    read_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    while ((read_data == 0x0U) && (timeout > 0U)) {
        read_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        timeout--;
    }
    g_ctx.checks_total++;
    if (read_data == 0x0U) {
        LOGE("Step 9 FAIL: DMA transfer completion poll timeout. INTMIS=0x%lx",
             (unsigned long)read_data);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        LOGT("Step 9 PASS: DMA transfer completed. INTMIS=0x%lx",
             (unsigned long)read_data);
        g_ctx.checks_passed++;
    }

    /* Step 10: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA interrupt */
    LOGT("Step 10: Writing to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupt");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, read_data);

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_dbi_random_payload_test_run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

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

    LOGT("mipi_dsi_dbi_random_payload_test_teardown: errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
