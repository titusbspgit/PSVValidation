// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * mipi_dsi_basic_test
 * This testcase performs a basic MIPI DSI DBI (Display Bus Interface) data
 * transfer using the integrated DMA controller (DMA330). It uses interrupt-driven
 * completion with an ISR to validate DMA channel transfers for pixel data and
 * DBI write commands.
 */

/* ---------------------------------------------------------------------------
 * Test context
 * --------------------------------------------------------------------------- */
typedef struct {
    unsigned int errors;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/* ---------------------------------------------------------------------------
 * ISR: Default_IRQHandler
 * Reads subsystem interrupt mask, validates GDMA source, reads DMA channel
 * masked interrupt status, clears pending bits, clears DMA and subsystem
 * interrupts, and clears the GIC IRQ.
 * --------------------------------------------------------------------------- */
static void Default_IRQHandler(void)
{
    unsigned int dsi_subsys_mask_st;
    unsigned int ch_mask_st;

    LOGT("ISR Default_IRQHandler entered");

    /* Step ISR-1: Read subsystem interrupt mask register */
    dsi_subsys_mask_st = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    LOGT("ISR INTERRUPT_MASK=0x%x", (unsigned int)dsi_subsys_mask_st);

    /* Step ISR-2: Check if interrupt source is GDMA */
    if (dsi_subsys_mask_st != MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {
        g_ctx.errors++;
        int_pend1_flag = 0U;
        LOGE("ISR unexpected interrupt source exp=GDMA_INTR actual=0x%x",
             (unsigned int)dsi_subsys_mask_st);
    } else {
        /* Step ISR-3: Read DMA masked interrupt status */
        ch_mask_st = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        LOGT("ISR DMAC_INTMIS=0x%x", (unsigned int)ch_mask_st);

        if (ch_mask_st != 0U) {
            /* Step ISR-4: Clear pending channel bits */
            int_pend1_flag = int_pend1_flag & ~ch_mask_st;
            LOGT("ISR cleared int_pend1_flag bits remaining=0x%x",
                 (unsigned int)int_pend1_flag);

            /* Step ISR-5: Clear DMA channel interrupt */
            writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, ch_mask_st);
            LOGT("ISR cleared DMAC_INTCLR=0x%x", (unsigned int)ch_mask_st);
        } else {
            g_ctx.errors++;
            LOGE("ISR DMAC_INTMIS is zero during GDMA interrupt");
        }
    }

    /* Step ISR-6: Clear subsystem interrupt */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, dsi_subsys_mask_st);
    LOGT("ISR cleared INTERRUPT_RAW=0x%x", (unsigned int)dsi_subsys_mask_st);

    /* Step ISR-7: Clear GIC IRQ */
    GIC_ClearIRQ(DSI_INTR_NO);
    LOGT("ISR GIC_ClearIRQ done");
}

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
    unsigned int phy_stop_wait_time;
    unsigned int n_lanes;
    unsigned int phy_if_cfg;

    (void)cfg;

    g_ctx = (mipi_dsi_basic_test_ctx_t){0};

    LOGT("mipi_dsi_basic_test init start");

    /* Step 1: Enable the DSI interrupt in the GIC */
    GIC_EnableIRQ(DSI_INTR_NO);
    LOGT("GIC_EnableIRQ(DSI_INTR_NO) called");

    /* Step 2: Initialize pending flags */
    int_pend_flag = 1U;
    int_pend1_flag = 0x3U;
    LOGT("int_pend_flag=0x%x int_pend1_flag=0x%x",
         (unsigned int)int_pend_flag, (unsigned int)int_pend1_flag);

    /* Step 3: Configure PHY parameters */
    phy_stop_wait_time = 0x40U;
    n_lanes = 3U;
    LOGT("phy_stop_wait_time=0x%x n_lanes=%u",
         (unsigned int)phy_stop_wait_time, (unsigned int)n_lanes);

    /* Step 4: Enable DMA interrupts for CH0 and CH1 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);
    LOGT("DMAC_INTEN=0x3 written");

    /* Step 5: Enable GDMA interrupt at subsystem level */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE,
              MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);
    LOGT("SUBSYS_INTERRUPT_ENABLE=GDMA_INTR written");

    /* Step 6: Configure PHY interface register */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                               MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                               phy_stop_wait_time);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("PHY_IF_CFG=0x%x written", (unsigned int)phy_if_cfg);

    /* Step 7: Configure packet handling */
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);
    LOGT("PCKHDL_CFG=0x3d written");

    /* Step 8: Configure clock manager */
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);
    LOGT("CLKMGR_CFG=0x107 written");

    /* Step 9: Disable DPI control to enable DBI mode */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0U);
    LOGT("DPI_CONTROL=0x0 written DBI mode enabled");

    /* Step 10: Initialize DSI PHY */
    phy_init();
    LOGT("phy_init() called");

    /* Step 11: Configure DBI parameters */
    dbi_vcid = 0x3U;
    load_cmd_or_data_to_sram = 1U;
    lut_size_conf = 0x1U;
    out_dbi_conf = 0xbU;
    in_dbi_conf = 0x0U;
    partitioning_en = 0x1U;
    allowed_cmd_size = 0x7U;
    LOGT("DBI params: dbi_vcid=0x%x lut_size_conf=0x%x out_dbi_conf=0x%x",
         (unsigned int)dbi_vcid, (unsigned int)lut_size_conf,
         (unsigned int)out_dbi_conf);

    /* Step 12: Compute write command size and byte count */
    pixel_to_bytes_wr_cmd_size(40U);
    LOGT("pixel_to_bytes_wr_cmd_size(40) called");

    /* Step 13: Set DSI command type flags */
    tear_fx_en = 0x1U;
    generic_vc_id = 0x2U;
    LOGT("tear_fx_en=0x%x generic_vc_id=0x%x",
         (unsigned int)tear_fx_en, (unsigned int)generic_vc_id);

    /* Step 14: Program DPI clock */
    program_dpi_clock();
    LOGT("program_dpi_clock() called");

    /* Step 15: Apply DBI configuration */
    dbi_config();
    LOGT("dbi_config() called");

    LOGT("mipi_dsi_basic_test init complete");
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
    unsigned int ch0_desc_addr_act;
    unsigned int ch1_desc_addr_act;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run start");

    /* Step 16: Set up DMA descriptors for CH0 (data) */
    data_tdbdcb[0] = RAM_BASE + 0x10000UL;
    data_rebdcb[0] = 0x10000000000ULL;
    LOGT("CH0 data_tdbdcb[0]=0x%lx data_rebdcb[0]=0x%llx",
         (unsigned long)data_tdbdcb[0], (unsigned long long)data_rebdcb[0]);

    /* Step 17: Set up DMA descriptors for CH1 (command) */
    cmd_tdbdcb[0] = RAM_BASE + 0x0000UL;
    cmd_rebdcb[0] = 0x10000008000ULL;
    LOGT("CH1 cmd_tdbdcb[0]=0x%lx cmd_rebdcb[0]=0x%llx",
         (unsigned long)cmd_tdbdcb[0], (unsigned long long)cmd_rebdcb[0]);

    /* Step 18: Program DMA microcode for CH0 (data) */
    program_dma_microcode(0, data_tdbdcb, data_rebdcb, num_bytes);
    LOGT("DMA microcode programmed for CH0 num_bytes=%u",
         (unsigned int)num_bytes);

    /* Step 19: Program DMA microcode for CH1 (command) */
    program_dma_microcode(1, cmd_tdbdcb, cmd_rebdcb, wr_cmd_size);
    LOGT("DMA microcode programmed for CH1 wr_cmd_size=%u",
         (unsigned int)wr_cmd_size);

    /* Step 20: Load random pixel data */
    load_rand_data(data_tdbdcb[0], num_bytes);
    LOGT("load_rand_data called");

    /* Step 21: Load write command */
    load_wr_command(cmd_tdbdcb[0], DSI_WRITE_MEMORY_START);
    LOGT("load_wr_command called with DSI_WRITE_MEMORY_START");

    /* Step 22: Start DMA CH0 */
    ch0_desc_addr_act = (unsigned int)data_tdbdcb[0];
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch0_desc_addr_act);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("DMA CH0 started DBGINST0=0x00A00000 DBGINST1=0x%x",
         (unsigned int)ch0_desc_addr_act);

    /* Step 23: Start DMA CH1 */
    ch1_desc_addr_act = (unsigned int)cmd_tdbdcb[0];
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000U);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch1_desc_addr_act);
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
    LOGT("DMA CH1 started DBGINST0=0x01A00000 DBGINST1=0x%x",
         (unsigned int)ch1_desc_addr_act);

    /* Step 24: Poll for both DMA channel completions (interrupt-driven) */
    LOGT("Polling int_pend1_flag for DMA completion");
    timeout = MIPI_DSI_BASIC_TEST_TIMEOUT;
    while ((int_pend1_flag != 0U) && (timeout > 0U)) {
        wait_on(10U);
        timeout--;
    }

    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA completion int_pend1_flag=0x%x",
             (unsigned int)int_pend1_flag);
        g_ctx.errors++;
    } else {
        LOGT("DMA completion detected int_pend1_flag=0x%x",
             (unsigned int)int_pend1_flag);
    }

    /* Step 25: Final settling delay */
    wait_on(10000U);
    LOGT("Final settling delay complete");

    /* Report result */
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

    // MANUAL_REVIEW: DV finish(err0) was present in the source flow. PSV/FV-native
    // status is reported through out->status in the run function.

    return g_ctx.errors == 0U ? 0 : -1;
}
