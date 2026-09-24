// Author - AI Force 2.3. 25-Jul-2025 04:30 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dsi_basic_test
 * Description: Basic MIPI DSI DBI write-memory-start operation using
 *   the integrated GDMA (DMA330). Enables DMA and subsystem interrupts,
 *   configures DSI host PHY, packet handling, and clock manager.
 *   Programs DMA channel via debug instruction interface to transfer
 *   payload from RAM to DSI for write-memory-start command.
 *   Validates GDMA interrupt at subsystem and DMA levels, then clears.
 */

/* Testcase context for error tracking */
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

    g_ctx.errors = 0U;

    LOGT("mipi_dsi_basic_test init: starting testcase initialization");

    /* Step 1: Enable DMA interrupt channels by writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN */
    LOGT("Step 1: Writing 0x%x to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA interrupts",
         (unsigned int)DMA_INTEN_CH_MASK);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, DMA_INTEN_CH_MASK);

    /* Step 2: Enable GDMA interrupt at subsystem level */
    LOGT("Step 2: Enabling GDMA interrupt at subsystem level via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE");
    {
        uint32_t rd_data;
        uint32_t wr_data;
        rd_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE);
        wr_data = set_data_mask(rd_data, MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);
        writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, wr_data);
        LOGT("Step 2: MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE written=0x%x",
             (unsigned int)wr_data);
    }

    /* Step 3: Configure DSI host PHY interface timing */
    LOGT("Step 3: Configuring PHY stop wait time in MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");
    {
        uint32_t rd_data;
        uint32_t wr_data;
        rd_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
        wr_data = set_data_mask(rd_data, MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
        writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, wr_data);
        LOGT("Step 3: MIZAR_MIPI_DSI_HOST_PHY_IF_CFG written=0x%x",
             (unsigned int)wr_data);
    }

    /* Step 4: Configure packet handling */
    LOGT("Step 4: Writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x0U);

    /* Step 5: Configure clock manager */
    LOGT("Step 5: Writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x0U);

    /* Step 6: Disable DPI control */
    LOGT("Step 6: Writing 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    LOGT("mipi_dsi_basic_test init: initialization complete");

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
    uint32_t rd_data;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting DMA programming and execution");

    /* Step 7: Program DMA source address (DMAMOV SAR) */
    LOGT("Step 7: Programming DMAMOV SAR via DBGINST0/DBGINST1/DBGCMD");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMAMOV_SAR_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, RAM_BASE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("Step 7: DMA SAR programmed, source=0x%lx", (unsigned long)RAM_BASE);

    /* Step 8: Program DMA destination address (DMAMOV DAR) */
    LOGT("Step 8: Programming DMAMOV DAR via DBGINST0/DBGINST1/DBGCMD");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMAMOV_DAR_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, DSI_WRITE_MEMORY_START);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("Step 8: DMA DAR programmed, dest=0x%x", (unsigned int)DSI_WRITE_MEMORY_START);

    /* Step 9: Program and execute additional DMA instructions */
    LOGT("Step 9: Programming DMALP, DMALD, DMAST, DMALPEND, DMASEV, DMAEND");

    /* DMALP */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMALP_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* DMALD */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMALD_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* DMAST */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMAST_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* DMALPEND */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMALPEND_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* DMASEV */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMASEV_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* DMAEND */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, DMAEND_OPCODE);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    LOGT("Step 9: All DMA instructions programmed and executed");

    /* Step 10: Wait for DSI interrupt (DSI_INTR_NO) */
    // MANUAL_REVIEW: The Meta TestPlan requires waiting for DSI interrupt
    // (DSI_INTR_NO). The FV Template does not provide an interrupt wait API.
    // Replace with the platform-specific interrupt wait mechanism.
    LOGT("Step 10: Waiting for DSI interrupt DSI_INTR_NO=%u", (unsigned int)DSI_INTR_NO);

    /* Step 11: Read interrupt_mask and verify GDMA interrupt bit */
    LOGT("Step 11: Reading MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to validate GDMA interrupt");
    rd_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    LOGT("Step 11: MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK=0x%x", (unsigned int)rd_data);
    if ((rd_data & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) != 0U) {
        LOGT("Step 11: PASS - GDMA interrupt bit is set in subsystem interrupt mask");
    } else {
        LOGE("Step 11: FAIL - GDMA interrupt bit is NOT set in subsystem interrupt mask, read=0x%x",
             (unsigned int)rd_data);
        g_ctx.errors++;
    }

    /* Step 12: Read INTMIS to confirm DMA interrupt status */
    LOGT("Step 12: Reading MIZAR_MIPI_DSI_DMAC_INTMIS to confirm DMA interrupt");
    rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    LOGT("Step 12: MIZAR_MIPI_DSI_DMAC_INTMIS=0x%x", (unsigned int)rd_data);
    if (rd_data != 0x0U) {
        LOGT("Step 12: PASS - DMA channel interrupt status is asserted");
    } else {
        LOGE("Step 12: FAIL - DMA channel interrupt status is NOT asserted, read=0x%x",
             (unsigned int)rd_data);
        g_ctx.errors++;
    }

    /* Step 13: Clear DMA interrupt by writing to INTCLR */
    LOGT("Step 13: Clearing DMA interrupt by writing 0x%x to MIZAR_MIPI_DSI_DMAC_INTCLR",
         (unsigned int)rd_data);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);

    /* Step 14: Clear subsystem interrupt by writing to interrupt_raw */
    LOGT("Step 14: Clearing subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW");
    {
        uint32_t mask_val;
        mask_val = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
        writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, mask_val);
        LOGT("Step 14: MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW written=0x%x",
             (unsigned int)mask_val);
    }

    /* Update final status */
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
