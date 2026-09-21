// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dsi_basic_test
 * Description: This testcase performs a basic MIPI DSI DBI write memory operation
 * using DMA. It enables DMAC interrupts, enables subsystem-level GDMA interrupt,
 * configures the DSI host PHY interface, packet handling, clock manager, and DPI
 * control. The DMA channel is programmed and executed via debug instruction
 * registers. The test verifies GDMA interrupt mask status, DMA completion via
 * masked interrupt status, then clears both DMAC and subsystem interrupts.
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

    LOGT("mipi_dsi_basic_test init: starting testcase initialization");

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
    uint32_t read_val;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting DBI write memory via DMA");

    /* Step 1: Write to MIZAR_MIPI_DSI_DMAC_INTEN to enable DMA controller interrupts */
    LOGT("Step 1: Enable DMA controller interrupts via MIZAR_MIPI_DSI_DMAC_INTEN");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, MIPI_DSI_DMAC_INTEN_ENABLE_VAL);

    /* Step 2: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE with MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR */
    LOGT("Step 2: Enable GDMA interrupt at subsystem level via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG using set_data_mask with PHY_STOP_WAIT_TIME */
    LOGT("Step 3: Configure PHY stop wait time via MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");
    {
        uint32_t phy_cfg_val;
        phy_cfg_val = readl_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
        phy_cfg_val = set_data_mask(phy_cfg_val,
                                   MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                                   MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME_MASK);
        writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_cfg_val);
    }

    /* Step 4: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    LOGT("Step 4: Configure packet handling via MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_HOST_PCKHDL_CFG_VAL);

    /* Step 5: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    LOGT("Step 5: Configure clock manager via MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_HOST_CLKMGR_CFG_VAL);

    /* Step 6: Write to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to configure DPI control settings */
    LOGT("Step 6: Configure DPI control via MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, MIPI_DSI_SUBSYS_DPI_CONTROL_VAL);

    /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with DMA channel instruction byte 0 */
    LOGT("Step 7: Program DMA channel instruction byte 0 via MIZAR_MIPI_DSI_DMAC_DBGINST0");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0,
              (DSI_WRITE_MEMORY_START | ((uint32_t)DMA_SAR << 8U) | ((uint32_t)DMA_DAR << 16U)));

    /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with DMA channel instruction byte 1 */
    LOGT("Step 8: Program DMA channel instruction byte 1 via MIZAR_MIPI_DSI_DMAC_DBGINST1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

    /* Step 9: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute the DMA debug instruction */
    LOGT("Step 9: Execute DMA debug instruction via MIZAR_MIPI_DSI_DMAC_DBGCMD");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, MIPI_DSI_DMAC_DBGCMD_EXECUTE);

    /* Step 10: Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK and verify GDMA interrupt mask status */
    LOGT("Step 10: Verify GDMA interrupt mask status via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK");
    read_val = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    g_ctx.checks_total++;
    if ((read_val & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) != MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {
        LOGE("GDMA interrupt mask verification failed: read=0x%lx expected_bit=0x%lx",
             (unsigned long)read_val,
             (unsigned long)MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR);
        g_ctx.errors++;
    } else {
        LOGT("GDMA interrupt mask verification passed: read=0x%lx",
             (unsigned long)read_val);
        g_ctx.checks_passed++;
    }

    /* Step 11: Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMA masked interrupt status for completion */
    LOGT("Step 11: Check DMA completion via MIZAR_MIPI_DSI_DMAC_INTMIS");
    read_val = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    g_ctx.checks_total++;
    if (read_val == 0U) {
        LOGE("DMA masked interrupt status indicates transfer not complete: read=0x%lx",
             (unsigned long)read_val);
        g_ctx.errors++;
    } else {
        LOGT("DMA transfer completion verified via INTMIS: read=0x%lx",
             (unsigned long)read_val);
        g_ctx.checks_passed++;
    }

    /* Step 12: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMA interrupt */
    LOGT("Step 12: Clear DMA interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, MIPI_DSI_DMAC_INTCLR_VAL);

    /* Step 13: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear the subsystem raw interrupt */
    LOGT("Step 13: Clear subsystem raw interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, MIPI_DSI_SUBSYS_INTERRUPT_RAW_GDMA_INTR);

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

    LOGT("mipi_dsi_basic_test teardown: errors=%u checks_passed=%u checks_total=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
