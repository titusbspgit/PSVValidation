// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Testcase: mipi_dsi_dbi_random_payload_test
 * Description: Performs a MIPI DSI DBI random payload transfer using DMA.
 *              Configures PHY interface, packet handling, clock manager,
 *              disables DPI control for DBI mode, enables DMA channel
 *              interrupts, issues DMA channel transfer instructions, polls
 *              DMAC interrupt status for completion, and clears DMA interrupts.
 */

typedef struct {
    unsigned int errors;
    unsigned int dma_ch_status;
} mipi_dsi_dbi_random_payload_test_ctx_t;

static mipi_dsi_dbi_random_payload_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_dbi_random_payload_test_init
 * Description: Performs testcase initialization and pre-condition setup.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init: DBI random payload DMA transfer");

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
    unsigned int poll_count;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting DBI random payload DMA transfer");

    /* Step 1: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure the PHY interface */
    /*         with lane count and stop wait time. */
    LOGT("Step 1: Configure PHY interface with lane count and stop wait time");
    // MANUAL_REVIEW: Exact PHY_IF_CFG value not provided in Meta TestPlan JSON.
    rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, rd_data);
    LOGT("PHY_IF_CFG: value=0x%lx", (unsigned long)rd_data);

    /* Step 2: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling. */
    LOGT("Step 2: Configure packet handling");
    // MANUAL_REVIEW: Exact PCKHDL_CFG value not provided in Meta TestPlan JSON.
    rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG);
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, rd_data);
    LOGT("PCKHDL_CFG: value=0x%lx", (unsigned long)rd_data);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager. */
    LOGT("Step 3: Configure clock manager");
    // MANUAL_REVIEW: Exact CLKMGR_CFG value not provided in Meta TestPlan JSON.
    rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG);
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, rd_data);
    LOGT("CLKMGR_CFG: value=0x%lx", (unsigned long)rd_data);

    /* Step 4: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control */
    /*         and select DBI mode. */
    LOGT("Step 4: Disable DPI control to select DBI mode");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 5: Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA channel interrupts. */
    LOGT("Step 5: Enable DMA channel interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 6: Write DMA instruction bytes to MIZAR_MIPI_DSI_DMAC_DBGINST0 for channel 0. */
    LOGT("Step 6: Write DMA instruction bytes to DBGINST0 for channel 0");
    // MANUAL_REVIEW: Exact DMA instruction bytes for channel 0 DBGINST0 not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x0U);

    /* Step 7: Write DMA instruction data to MIZAR_MIPI_DSI_DMAC_DBGINST1 for channel 0. */
    LOGT("Step 7: Write DMA instruction data to DBGINST1 for channel 0");
    // MANUAL_REVIEW: Exact DMA instruction data for channel 0 DBGINST1 not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, 0x0U);

    /* Step 8: Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA instruction for channel 0. */
    LOGT("Step 8: Execute DMA instruction for channel 0");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 9: Repeat steps 6-8 for channel 1 with appropriate instruction bytes. */
    LOGT("Step 9: Issue DMA transfer instructions for channel 1");
    // MANUAL_REVIEW: Exact DMA instruction bytes for channel 1 DBGINST0 not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x0U);
    // MANUAL_REVIEW: Exact DMA instruction data for channel 1 DBGINST1 not provided in Meta TestPlan JSON.
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, 0x0U);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 10: Poll MIZAR_MIPI_DSI_DMAC_INTMIS until both DMA channels report */
    /*          completion (expected value 0x3). */
    LOGT("Step 10: Poll DMAC interrupt status for both channels completion");
    poll_count = 0U;
    do {
        rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        poll_count++;
        if (poll_count >= MIPI_DSI_POLL_TIMEOUT) {
            LOGE("DMA transfer not completed after %u polls, DMAC_INTMIS=0x%lx expected=0x3",
                 poll_count, (unsigned long)rd_data);
            g_ctx.errors++;
            out->status = -1;
            break;
        }
    } while (rd_data != 0x3U);

    if (rd_data == 0x3U) {
        LOGT("Both DMA channels completed, DMAC_INTMIS=0x%lx after %u polls",
             (unsigned long)rd_data, poll_count);
    }

    g_ctx.dma_ch_status = (unsigned int)rd_data;

    /* Step 11: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA channel interrupts. */
    LOGT("Step 11: Clear DMA channel interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);
    LOGT("DMAC_INTCLR written=0x%lx", (unsigned long)rd_data);

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u dma_ch_status=0x%x",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.dma_ch_status);

    return out->status;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling.
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
