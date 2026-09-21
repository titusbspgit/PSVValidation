// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dsi_dbi_random_payload_test
 * Description: This testcase performs a MIPI DSI DBI random payload transfer
 * using DMA. It configures the DSI host PHY interface, packet handling, clock
 * manager, and DPI control. DMAC interrupts are enabled. The DMA channel is
 * programmed and executed via debug instruction registers. The test polls
 * DMAC masked interrupt status for DMA completion, then clears the DMA
 * interrupt. The payload data used for the DBI write memory command is random.
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
    uint32_t read_val;
    uint32_t poll_count;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting DBI random payload transfer via DMA");

    /* Step 1: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with PHY_STOP_WAIT_TIME */
    LOGT("Step 1: Configure PHY stop wait time via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");
    {
        uint32_t phy_cfg_val;
        phy_cfg_val = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
        phy_cfg_val = set_data_mask(phy_cfg_val,
                                   MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                                   MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME_MASK);
        writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_cfg_val);
    }

    /* Step 2: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    LOGT("Step 2: Configure packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_HOST_PCKHDL_CFG_VAL);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    LOGT("Step 3: Configure clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_HOST_CLKMGR_CFG_VAL);

    /* Step 4: Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control settings */
    LOGT("Step 4: Configure DPI control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, MIPI_DSI_SUBSYS_DPI_CONTROL_VAL);

    /* Step 5: Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts */
    LOGT("Step 5: Enable DMA controller interrupts via MIZAR_MIPI_DSI_DMAC_INTEN");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, MIPI_DSI_DMAC_INTEN_ENABLE_VAL);

    /* Step 6: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA channel instruction byte 0 (random payload) */
    LOGT("Step 6: Program DMA channel instruction byte 0 via MIZAR_MIPI_DSI_DMAC_DBGINST0 (random payload)");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0,
              (DSI_WRITE_MEMORY_START | ((uint32_t)DMA_SAR << 8U) | ((uint32_t)DMA_DAR << 16U)));

    /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA channel instruction byte 1 */
    LOGT("Step 7: Program DMA channel instruction byte 1 via MIZAR_MIPI_DSI_DMAC_DBGINST1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

    /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug instruction */
    LOGT("Step 8: Execute DMA debug instruction via MIZAR_MIPI_DSI_DMAC_DBGCMD");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, MIPI_DSI_DMAC_DBGCMD_EXECUTE);

    /* Step 9: Poll MIZAR_MIPI_DSI_DMAC_INTMIS in a loop to wait for DMA transfer completion */
    LOGT("Step 9: Poll MIZAR_MIPI_DSI_DMAC_INTMIS for DMA transfer completion");
    poll_count = 0U;
    g_ctx.checks_total++;
    do {
        read_val = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        poll_count++;
        if (poll_count > MIPI_DSI_POLL_MAX_RETRIES) {
            LOGE("DMA transfer completion poll TIMEOUT after %u retries",
                 (unsigned int)MIPI_DSI_POLL_MAX_RETRIES);
            g_ctx.errors++;
            break;
        }
    } while (read_val == 0U);

    if (g_ctx.errors == 0U) {
        LOGT("DMA transfer completion verified via INTMIS: read=0x%lx polls=%u",
             (unsigned long)read_val, (unsigned int)poll_count);
        g_ctx.checks_passed++;
    }

    /* Step 10: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA interrupt */
    LOGT("Step 10: Clear DMA interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, MIPI_DSI_DMAC_INTCLR_VAL);

    /* Update final status */
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u checks_passed=%u checks_total=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
