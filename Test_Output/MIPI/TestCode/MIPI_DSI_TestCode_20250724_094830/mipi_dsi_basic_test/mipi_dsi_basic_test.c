// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_basic_test
 * Description: This testcase performs a basic MIPI DSI DBI command mode
 *              transfer using the DMAC. It enables DMAC and subsystem
 *              interrupts, configures DSI host PHY interface, packet handling,
 *              and clock manager, disables DPI control, programs and executes
 *              DMAC debug instructions, polls for interrupt status, and clears
 *              both DMAC and subsystem interrupts.
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
    uint32_t read_val;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test_run: starting MIPI DSI DBI command mode transfer");

    /* Step 1: Write to MIZAR_MIPI_DSI_DMAC_INTEN with value 0x3 to enable DMAC channel interrupts */
    LOGT("Step 1: Write MIZAR_MIPI_DSI_DMAC_INTEN = 0x3");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 2: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE to enable the GDMA interrupt bit */
    // MANUAL_REVIEW: Exact value for GDMA interrupt enable bit not provided in Meta TestPlan JSON.
    LOGT("Step 2: Write MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE, MIPI_DSI_SUBSYS_INT_ENABLE_VAL);

    /* Step 3: Write to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG to configure PHY interface */
    // MANUAL_REVIEW: Exact value for PHY_STOP_WAIT_TIME field not provided. set_data_mask mentioned but mask/value not supplied.
    LOGT("Step 3: Configure MIZAR_MIPI_DSI_HOST_PHY_IF_CFG (PHY_STOP_WAIT_TIME)");
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, MIPI_DSI_PHY_IF_CFG_VAL);

    /* Step 4: Write to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG to configure packet handling */
    // MANUAL_REVIEW: Exact value for PCKHDL_CFG not provided in Meta TestPlan JSON.
    LOGT("Step 4: Configure MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, MIPI_DSI_PCKHDL_CFG_VAL);

    /* Step 5: Write to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG to configure the clock manager */
    // MANUAL_REVIEW: Exact value for CLKMGR_CFG not provided in Meta TestPlan JSON.
    LOGT("Step 5: Configure MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, MIPI_DSI_CLKMGR_CFG_VAL);

    /* Step 6: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to disable DPI control for command mode */
    LOGT("Step 6: Write MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL = 0x0 (disable DPI)");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0U);

    /* Step 7: Write to MIZAR_MIPI_DSI_DMAC_DBGINST0 with first debug instruction word */
    // MANUAL_REVIEW: Exact debug instruction word for DBGINST0 not provided in Meta TestPlan JSON.
    LOGT("Step 7: Program MIZAR_MIPI_DSI_DMAC_DBGINST0");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, MIPI_DSI_DMAC_DBGINST0_VAL);

    /* Step 8: Write to MIZAR_MIPI_DSI_DMAC_DBGINST1 with second debug instruction word */
    // MANUAL_REVIEW: Exact debug instruction word for DBGINST1 not provided in Meta TestPlan JSON.
    LOGT("Step 8: Program MIZAR_MIPI_DSI_DMAC_DBGINST1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, MIPI_DSI_DMAC_DBGINST1_VAL);

    /* Step 9: Write to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute debug instruction and start DMA transfer */
    LOGT("Step 9: Execute MIZAR_MIPI_DSI_DMAC_DBGCMD to start DMA transfer");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 10: Poll MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for subsystem interrupt assertion */
    // MANUAL_REVIEW: Timeout value not provided by FV Template or Meta TestPlan JSON.
    LOGT("Step 10: Poll MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK for GDMA interrupt");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    do {
        read_val = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
        if ((read_val & MIPI_DSI_SUBSYS_GDMA_INT_BIT) != 0U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);

    if (timeout == 0U) {
        LOGE("Step 10: Timeout polling MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK last=0x%lx",
             (unsigned long)read_val);
        g_ctx.errors++;
    } else {
        LOGT("Step 10: GDMA interrupt detected in INTERRUPT_MASK=0x%lx",
             (unsigned long)read_val);
    }

    /* Step 11: Read MIZAR_MIPI_DSI_DMAC_INTMIS to check DMAC masked interrupt status */
    LOGT("Step 11: Read MIZAR_MIPI_DSI_DMAC_INTMIS to verify DMAC interrupt status");
    read_val = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
    LOGT("Step 11: MIZAR_MIPI_DSI_DMAC_INTMIS=0x%lx", (unsigned long)read_val);

    if (read_val == 0x0U) {
        LOGE("Step 11: DMAC INTMIS indicates no interrupt (unexpected)");
        g_ctx.errors++;
    } else {
        LOGT("Step 11: DMAC INTMIS indicates transfer completion");
    }

    /* Step 12: Write to MIZAR_MIPI_DSI_DMAC_INTCLR to clear the DMAC interrupt */
    LOGT("Step 12: Clear DMAC interrupt via MIZAR_MIPI_DSI_DMAC_INTCLR=0x%lx",
         (unsigned long)read_val);
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, read_val);

    /* Step 13: Write to MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW to clear subsystem raw interrupt */
    // MANUAL_REVIEW: Exact clear value for subsystem interrupt not provided. Using GDMA bit.
    LOGT("Step 13: Clear subsystem interrupt via MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, MIPI_DSI_SUBSYS_GDMA_INT_BIT);

    /* Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_basic_test_run complete: %s errors=%u",
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

    LOGT("mipi_dsi_basic_test_teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
