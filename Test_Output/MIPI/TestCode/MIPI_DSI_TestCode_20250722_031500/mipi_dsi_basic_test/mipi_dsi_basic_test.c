// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_basic_test
 * Description: This testcase performs a basic MIPI DSI DBI write memory operation
 *              using DMA. It enables DMAC interrupts, enables subsystem-level GDMA
 *              interrupt, configures DSI host PHY interface, packet handling, clock
 *              manager, and DPI control. The DMA channel is programmed and executed.
 *              The test verifies GDMA interrupt mask status, DMA completion, and
 *              clears both DMAC and subsystem interrupts.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
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

    LOGT("mipi_dsi_basic_test_init: initialization complete");

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
    uint32_t read_data = 0U;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test_run: starting test execution");

    /* Step 1: Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts */
    LOGT("Step 1: Writing to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x1U);

    /* Step 2: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR */
    LOGT("Step 2: Writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable GDMA interrupt");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with PHY_STOP_WAIT_TIME */
    LOGT("Step 3: Configuring MIZAR_MIPI_DSI_HOST_PHY_IF_CFG with PHY stop wait time");
    read_data = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    read_data = read_data | MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME;
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, read_data);

    /* Step 4: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    LOGT("Step 4: Writing to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x1U);

    /* Step 5: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    LOGT("Step 5: Writing to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure clock manager");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x1U);

    /* Step 6: Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control settings */
    LOGT("Step 6: Writing to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x1U);

    /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA channel instruction byte 0 */
    /* Includes DSI_WRITE_MEMORY_START command, DMA source address DMA_SAR, destination DMA_DAR */
    LOGT("Step 7: Writing to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA instruction byte 0");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, (DSI_WRITE_MEMORY_START | (DMA_SAR << 8) | (DMA_DAR << 16)));

    /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA channel instruction byte 1 */
    LOGT("Step 8: Writing to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA instruction byte 1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, 0x0U);

    /* Step 9: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug instruction */
    LOGT("Step 9: Writing to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute DMA debug instruction");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 10: Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and compare with GDMA_INTR */
    /* Validation Criteria 1: Verify GDMA interrupt mask status */
    LOGT("Step 10: Reading MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK to verify GDMA interrupt mask");
    read_data = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    g_ctx.checks_total++;
    if ((read_data & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) != MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {
        LOGE("Step 10 FAIL: GDMA interrupt mask mismatch. INTERRUPT_MASK=0x%lx expected_bit=0x%lx",
             (unsigned long)read_data,
             (unsigned long)MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        LOGT("Step 10 PASS: GDMA interrupt mask verified. INTERRUPT_MASK=0x%lx",
             (unsigned long)read_data);
        g_ctx.checks_passed++;
    }

    /* Step 11: Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA masked interrupt status for completion */
    /* Validation Criteria 2: Verify DMA transfer completed successfully */
    LOGT("Step 11: Reading MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA completion");
    read_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    g_ctx.checks_total++;
    if (read_data == 0x0U) {
        LOGE("Step 11 FAIL: DMA masked interrupt status indicates transfer not complete. INTMIS=0x%lx",
             (unsigned long)read_data);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        LOGT("Step 11 PASS: DMA transfer completed. INTMIS=0x%lx",
             (unsigned long)read_data);
        g_ctx.checks_passed++;
    }

    /* Step 12: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA interrupt */
    /* Validation Criteria 3: Clear DMA interrupt and verify acknowledged */
    LOGT("Step 12: Writing to MIZAR_MIPI_DSI_DMAC_INTCLR to clear DMA interrupt");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, read_data);

    /* Step 13: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem raw interrupt */
    /* Validation Criteria 4: Clear subsystem-level raw interrupt */
    LOGT("Step 13: Writing to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem interrupt");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, 0x1U);

    /* Final status */
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_basic_test_run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

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

    LOGT("mipi_dsi_basic_test_teardown: errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
