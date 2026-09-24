// Author - AI Force 2.3. 25-Jul-2025 10:00 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Description: MIPI DSI DBI write-memory-start operation with random payload
 *   data using the integrated GDMA (DMA330) with two DMA channels.
 *   Configures DSI host PHY, packet handling, and clock manager.
 *   Disables DPI control. Enables DMA interrupts for both channels.
 *   Programs two DMA channels via DMAGO debug instructions.
 *   Polls INTMIS until both channels complete, then clears interrupts.
 */

/* Testcase context for error tracking */
typedef struct {
    unsigned int errors;
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

    g_ctx.errors = 0U;

    LOGT("mipi_dsi_dbi_random_payload_test init: starting testcase initialization");

    /* Step 1: Configure DSI host PHY interface timing */
    LOGT("Step 1: Configuring PHY stop wait time in MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");
    {
        uint32_t rd_data;
        uint32_t wr_data;
        rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
        wr_data = set_data_mask(rd_data, MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
        writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, wr_data);
        LOGT("Step 1: MIZAR_MIPI_DSI_HOST_PHY_IF_CFG written=0x%x",
             (unsigned int)wr_data);
    }

    /* Step 2: Configure packet handling by writing 0x3d */
    LOGT("Step 2: Writing 0x%x to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG",
         (unsigned int)PCKHDL_CFG_VALUE);
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, PCKHDL_CFG_VALUE);

    /* Step 3: Configure clock manager by writing 0x107 */
    LOGT("Step 3: Writing 0x%x to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG",
         (unsigned int)CLKMGR_CFG_VALUE);
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, CLKMGR_CFG_VALUE);

    /* Step 4: Disable DPI control */
    LOGT("Step 4: Writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 5: Enable DMA interrupts for both channels */
    LOGT("Step 5: Writing 0x%x to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA interrupts",
         (unsigned int)DMA_INTEN_CH_MASK);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, DMA_INTEN_CH_MASK);

    LOGT("mipi_dsi_dbi_random_payload_test init: initialization complete");

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
    uint32_t rd_data;
    uint32_t poll_count;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting DMA channel programming");

    /* Step 6: Program DMA channel 0 via DMAGO instruction */
    LOGT("Step 6: Programming DMA channel 0 DMAGO with descriptor address 0x%lx",
         (unsigned long)DMA_CH0_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMA_CH0_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, DMA_CH0_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("Step 6: DMA channel 0 programmed and executed");

    /* Step 7: Program DMA channel 1 via DMAGO instruction */
    LOGT("Step 7: Programming DMA channel 1 DMAGO with descriptor address 0x%lx",
         (unsigned long)DMA_CH1_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMA_CH1_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, DMA_CH1_DESC_ADDR);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("Step 7: DMA channel 1 programmed and executed");

    /* Step 8: Poll INTMIS until both DMA channels complete (value == 0x3) */
    LOGT("Step 8: Polling MIZAR_MIPI_DSI_DMAC_INTMIS for both channel completion (expected=0x%x)",
         (unsigned int)DMA_INTMIS_BOTH_CH_DONE);
    poll_count = 0U;
    rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    while (rd_data != DMA_INTMIS_BOTH_CH_DONE) {
        rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        poll_count++;
        if (poll_count >= POLL_TIMEOUT) {
            LOGE("Step 8: FAIL - Polling timeout reached, INTMIS=0x%x expected=0x%x poll_count=%u",
                 (unsigned int)rd_data,
                 (unsigned int)DMA_INTMIS_BOTH_CH_DONE,
                 poll_count);
            g_ctx.errors++;
            break;
        }
    }
    if (rd_data == DMA_INTMIS_BOTH_CH_DONE) {
        LOGT("Step 8: PASS - Both DMA channels completed, INTMIS=0x%x poll_count=%u",
             (unsigned int)rd_data, poll_count);
    }

    /* Step 9: Clear DMA interrupts for both channels */
    LOGT("Step 9: Clearing DMA interrupts by writing 0x%x to MIZAR_MIPI_DSI_DMAC_INTCLR",
         (unsigned int)rd_data);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);
    LOGT("Step 9: DMA interrupts cleared");

    /* Update final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_dbi_random_payload_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
