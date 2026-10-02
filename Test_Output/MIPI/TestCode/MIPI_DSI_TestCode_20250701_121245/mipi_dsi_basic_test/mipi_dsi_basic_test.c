// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_basic_test
 * Description: Basic MIPI DSI DBI data transfer using a 2-channel DMA engine.
 *              CH0 for pixel data, CH1 for commands.
 *              Interrupt-driven DMA completion with polling wait in main thread.
 */

/* ---------------------------------------------------------------------------
 * Test context structure
 * --------------------------------------------------------------------------*/
typedef struct {
    unsigned int errors;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/* ---------------------------------------------------------------------------
 * IRQ Handler: Default_IRQHandler
 * Description: Handles DSI DMA completion interrupts for CH0 and CH1.
 *              Reads subsystem interrupt mask, validates GDMA source,
 *              clears DMA and subsystem interrupts, tracks errors.
 * --------------------------------------------------------------------------*/
void Default_IRQHandler(void)
{
    unsigned int dsi_subsys_mask_st;
    unsigned int ch_mask_st;

    /* Step 28a: Set int_pend=0 to indicate handler entry */
    int_pend = 0U;

    /* Step 28b: Read subsystem interrupt mask register */
    dsi_subsys_mask_st = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    LOGT("[IRQ] SUBSYS_INTERRUPT_MASK = 0x%x", dsi_subsys_mask_st);

    /* Step 28c: Check if the interrupt source is GDMA */
    if (dsi_subsys_mask_st & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {

        /* Step 28d: Read DMA masked interrupt status */
        ch_mask_st = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        LOGT("[IRQ] DMAC_INTMIS = 0x%x", ch_mask_st);

        /* Step 28e: If valid channel mask, clear pending bits */
        if (ch_mask_st != 0U) {
            int_pend1 = int_pend1 & ~ch_mask_st;
            LOGT("[IRQ] Cleared int_pend1, new value = 0x%x", int_pend1);

            /* Clear DMA interrupt */
            writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, ch_mask_st);
            LOGT("[IRQ] Wrote DMAC_INTCLR = 0x%x", ch_mask_st);
        } else {
            /* Step 28f: ch_mask_st is zero - unexpected */
            LOGE("[ERROR2] GDMA interrupt matched but ch_mask_st is zero");
            err0++;
            g_ctx.errors++;
        }

        /* Step 28g: Clear subsystem interrupt */
        writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, dsi_subsys_mask_st);
        LOGT("[IRQ] Wrote SUBSYS_INTERRUPT_RAW = 0x%x", dsi_subsys_mask_st);

    } else {
        /* Step 28h: Unexpected interrupt source */
        LOGE("[ERROR1] Unexpected interrupt source: 0x%x", dsi_subsys_mask_st);
        err0++;
        g_ctx.errors++;
    }

    /* Step 28i: Clear GIC interrupt */
    GIC_ClearIRQ(DSI_INTR_NO);

    /* Step 28j: Set int_pend=1 to indicate handler exit */
    int_pend = 1U;
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
    unsigned int num_of_pixel;
    double dpi_clk_time_period;
    double dpi_clk_freq;

    (void)cfg;

    /* Initialize test context */
    g_ctx = (mipi_dsi_basic_test_ctx_t){0};

    LOGT("mipi_dsi_basic_test init: starting initialization");

    /* Step 1: Enable GIC IRQ for DSI interrupt */
    LOGT("Step 1: Enabling GIC IRQ for DSI_INTR_NO");
    GIC_EnableIRQ(DSI_INTR_NO);

    /* Step 2: Initialize variables */
    LOGT("Step 2: Initializing variables");
    int_pend           = 1U;
    int_pend1          = 0x3U;
    err0               = 0U;
    phy_stop_wait_time = 0x40U;
    n_lanes            = 3U;

    /* Step 3: Enable DMA interrupts for CH0 and CH1 */
    LOGT("Step 3: Writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 4: Enable GDMA interrupt at subsystem level */
    LOGT("Step 4: Enabling GDMA interrupt at subsystem level");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE,
              MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);

    /* Step 5: Compute PHY interface configuration */
    LOGT("Step 5: Computing PHY_IF_CFG n_lanes=%u phy_stop_wait_time=0x%x",
         n_lanes, phy_stop_wait_time);
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg, phy_stop_wait_time,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);

    /* Step 6: Write PHY_IF_CFG */
    LOGT("Step 6: Writing PHY_IF_CFG = 0x%x", phy_if_cfg);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);

    /* Step 7: Configure packet handling */
    LOGT("Step 7: Writing PCKHDL_CFG = 0x3d");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);

    /* Step 8: Configure clock manager */
    LOGT("Step 8: Writing CLKMGR_CFG = 0x107");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);

    /* Step 9: Enable DBI mode */
    LOGT("Step 9: Writing DPI_CONTROL = 0 (DBI mode)");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0U);

    /* Step 10: Initialize DSI PHY */
    LOGT("Step 10: Calling phy_init()");
    phy_init();

    /* Step 11: Set DBI configuration variables */
    LOGT("Step 11: Setting DBI configuration variables");
    dbi_vcid                 = 0x3U;
    load_cmd_or_data_to_sram = 1U;
    lut_size_conf            = 0x1U;
    out_dbi_conf             = 0xbU;
    in_dbi_conf              = 0x0U;
    partitioning_en          = 0x1U;
    allowed_cmd_size         = 0x7U;

    /* Step 12: Compute pixel-to-byte conversion */
    num_of_pixel = 40U;
    LOGT("Step 12: Computing pixel_to_bytes_wr_cmd_size for %u pixels", num_of_pixel);
    pixel_to_bytes_wr_cmd_size(num_of_pixel);

    /* Step 13: Set command type variables */
    LOGT("Step 13: Setting command type variables");
    max_rd_pkt_size = 0U;
    dcs_lw_tx       = 0U;
    dcs_sr_0p_tx    = 0U;
    dcs_sw_1p_tx    = 0U;
    dcs_sw_0p_tx    = 0U;
    gen_lw_tx       = 0U;
    gen_sr_2p_tx    = 0U;
    gen_sr_1p_tx    = 0U;
    gen_sr_0p_tx    = 0U;
    gen_sw_2p_tx    = 0U;
    gen_sw_1p_tx    = 0U;
    gen_sw_0p_tx    = 0U;
    ack_rqst_en     = 0U;
    tear_fx_en      = 1U;
    generic_vc_id   = 0x2U;

    /* Step 14: Program DPI clock */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    LOGT("Step 14: Programming DPI clock");
    program_dpi_clock(dpi_clk_freq);

    /* Step 15: Apply DBI configuration */
    LOGT("Step 15: Calling dbi_config()");
    dbi_config();

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
    unsigned int num_bytes;
    unsigned int num_descriptors;
    unsigned long long ch0_desc_addr;
    unsigned long long ch1_desc_addr;
    unsigned long long ch0_desc_addr_act;
    unsigned long long ch1_desc_addr_act;
    int itter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting main execution");

    /* Retrieve num_bytes from pixel_attr computed during init */
    num_bytes = pixel_attr.num_bytes;
    LOGT("num_bytes = %u", num_bytes);

    /* Step 16: Set number of descriptors */
    num_descriptors = 1U;
    LOGT("Step 16: num_descriptors = %u", num_descriptors);

    /* Step 17: Set DMA descriptor addresses */
    ch0_desc_addr     = RAM_BASE + 0x3F000ULL;
    ch1_desc_addr     = RAM_BASE + 0x3F800ULL;
    ch0_desc_addr_act = ch0_desc_addr;
    ch1_desc_addr_act = ch1_desc_addr;
    LOGT("Step 17: CH0 desc addr = 0x%llx, CH1 desc addr = 0x%llx",
         (unsigned long long)ch0_desc_addr_act,
         (unsigned long long)ch1_desc_addr_act);

    /* Step 18: Descriptor loop */
    for (itter = 0; itter < (int)num_descriptors; itter++) {
        LOGT("Step 18: Descriptor iteration %d", itter);

        /* Step 18a: Data transmit descriptor */
        data_tdbdcb[itter].addr = RAM_BASE + 0x10000ULL;
        data_tdbdcb[itter].len  = num_bytes;
        data_tdbdcb[itter].eop  = 1U;

        /* Step 18b: Data receive descriptor */
        data_rebdcb[itter].addr = 0x10000000000ULL;
        data_rebdcb[itter].len  = num_bytes;

        /* Step 18c: Command transmit descriptor */
        cmd_tdbdcb[itter].addr  = RAM_BASE + 0x0000ULL;
        cmd_tdbdcb[itter].len   = 4U;
        cmd_tdbdcb[itter].eop   = 1U;

        /* Step 18d: Command receive descriptor */
        cmd_rebdcb[itter].addr  = 0x10000008000ULL;
        cmd_rebdcb[itter].len   = 4U;

        /* Step 18e: Program CH0 DMA microcode (data channel) */
        LOGT("Step 18e: Programming CH0 DMA microcode");
        DMAMOV(&ch0_desc_addr, DMA_SAR, data_tdbdcb[itter].addr);
        DMAMOV(&ch0_desc_addr, DMA_DAR, data_rebdcb[itter].addr);
        program_data_num_bytes(&ch0_desc_addr, data_tdbdcb[itter].len);
        DMAWMB(&ch0_desc_addr);
        DMASEV(&ch0_desc_addr, 0);
        DMAEND(&ch0_desc_addr);

        /* Step 18f: Program CH1 DMA microcode (command channel) */
        LOGT("Step 18f: Programming CH1 DMA microcode");
        DMAMOV(&ch1_desc_addr, DMA_SAR, cmd_tdbdcb[itter].addr);
        DMAMOV(&ch1_desc_addr, DMA_DAR, cmd_rebdcb[itter].addr);
        program_data_num_bytes(&ch1_desc_addr, cmd_tdbdcb[itter].len);
        DMAWMB(&ch1_desc_addr);
        DMASEV(&ch1_desc_addr, 1);
        DMAEND(&ch1_desc_addr);

        /* Step 18g: Load random data into SRAM */
        LOGT("Step 18g: Loading random data into SRAM");
        load_rand_data(data_tdbdcb[itter]);

        /* Step 18h: Load write command into SRAM */
        LOGT("Step 18h: Loading write command into SRAM");
        load_wr_command(cmd_tdbdcb[itter].addr, num_bytes, DSI_WRITE_MEMORY_START);
    }

    /* Step 19: Start CH0 DMA transfer */
    LOGT("Step 19: Starting CH0 DMA: DBGINST0 = 0x00A00000");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);

    /* Step 20: Write CH0 descriptor address */
    LOGT("Step 20: CH0 DBGINST1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);

    /* Step 21: Execute CH0 DMA instruction */
    LOGT("Step 21: Executing CH0 DMA (DBGCMD = 0x0)");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 22: Start CH1 DMA transfer */
    LOGT("Step 22: Starting CH1 DMA: DBGINST0 = 0x01A00000");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000U);

    /* Step 23: Write CH1 descriptor address */
    LOGT("Step 23: CH1 DBGINST1");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);

    /* Step 24: Execute CH1 DMA instruction */
    LOGT("Step 24: Executing CH1 DMA (DBGCMD = 0x0)");
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

    /* Step 25: Poll for DMA completion */
    LOGT("Step 25: Polling for DMA completion (int_pend1 = 0x%x)", int_pend1);
    while (int_pend1) {
        wait_on(10);
    }
    LOGT("Step 25: DMA completion detected, int_pend1 = 0x%x", int_pend1);

    /* Step 26: Final settling delay */
    LOGT("Step 26: Final settling delay");
    wait_on(10000);

    /* Update output status based on error count */
    out->status = (err0 == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    // MANUAL_REVIEW: DV finish(err0) was present in the source flow. PSV/FV-native
    // out->status based PASS/FAIL reporting is used instead.

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

    /* Validation: err0 must be 0 and int_pend1 must be 0 */
    if (err0 == 0U) {
        LOGT("mipi_dsi_basic_test PASSED: err0=0, both CH0 and CH1 interrupts serviced");
    } else {
        LOGE("mipi_dsi_basic_test FAILED: err0=%u", err0);
    }

    return g_ctx.errors == 0U ? 0 : -1;
}
