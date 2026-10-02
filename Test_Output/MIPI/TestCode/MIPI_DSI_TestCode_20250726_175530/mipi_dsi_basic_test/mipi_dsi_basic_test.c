// Author - AI Force 2.3. 26-Jul-2025 17:55 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Testcase     : mipi_dsi_basic_test
 * Description  : Basic MIPI DSI DBI data transfer using 2-channel DMA engine.
 *                CH0 for data transfer, CH1 for command transfer.
 *                Interrupt-driven DMA completion via Default_IRQHandler.
 */

/* ---------------------------------------------------------------------------
 * Testcase context
 * --------------------------------------------------------------------------- */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/* ---------------------------------------------------------------------------
 * Global variables for interrupt-driven DMA completion
 * --------------------------------------------------------------------------- */
volatile int int_pend  = 1;
volatile int int_pend1 = 0x3;

/* DMA descriptor control block arrays */
dma_desc_t data_tdbdcb[2];
dma_desc_t data_rebdcb[2];
dma_desc_t cmd_tdbdcb[2];
dma_desc_t cmd_rebdcb[2];

/* ---------------------------------------------------------------------------
 * IRQ Handler: Default_IRQHandler
 * Description: Handles DSI DMA completion interrupts for CH0 and CH1.
 *              Reads subsystem interrupt mask, validates GDMA source,
 *              clears DMA and subsystem interrupts, updates int_pend1.
 * --------------------------------------------------------------------------- */
void Default_IRQHandler(void)
{
    unsigned int dsi_subsys_mask_st;
    unsigned int ch_mask_st;

    /* Step 28a: Clear pending flag */
    int_pend = 0;

    /* Step 28b: Read subsystem interrupt mask status */
    dsi_subsys_mask_st = readl(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    LOGT("[IRQ] SUBSYS_INTERRUPT_MASK = 0x%x", dsi_subsys_mask_st);

    /* Step 28c: Check if GDMA interrupt source */
    if (dsi_subsys_mask_st & MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {

        /* Step 28d: Read DMA masked interrupt status */
        ch_mask_st = readl(MIZAR_MIPI_DSI_DMAC_INTMIS);
        LOGT("[IRQ] DMAC_INTMIS = 0x%x", ch_mask_st);

        if (ch_mask_st) {
            /* Step 28e: Clear pending bits for serviced channels */
            int_pend1 = int_pend1 & ~ch_mask_st;
            writel(ch_mask_st, MIZAR_MIPI_DSI_DMAC_INTCLR);
            LOGT("[IRQ] Cleared DMA interrupt for channels: 0x%x", ch_mask_st);
        } else {
            /* Step 28f: GDMA matched but no channel interrupt - ERROR2 */
            LOGE("[IRQ] ERROR2: GDMA interrupt matched but INTMIS is zero");
            g_ctx.errors++;
        }

        /* Step 28g: Clear subsystem interrupt */
        writel(dsi_subsys_mask_st, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW);

    } else {
        /* Step 28h: Unexpected interrupt source - ERROR1 */
        LOGE("[IRQ] ERROR1: Unexpected interrupt source: 0x%x", dsi_subsys_mask_st);
        g_ctx.errors++;
        /* Still clear subsystem interrupt */
        writel(dsi_subsys_mask_st, MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW);
    }

    /* Step 28i: Clear GIC interrupt */
    GIC_ClearIRQ(DSI_INTR_NO);

    /* Step 28j: Re-enable pending flag */
    int_pend = 1;
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
    (void)cfg;

    /* Initialize testcase context */
    g_ctx = (mipi_dsi_basic_test_ctx_t){0};

    LOGT("mipi_dsi_basic_test init: starting initialization");

    /* Step 1: Enable GIC IRQ for DSI interrupt */
    GIC_EnableIRQ(DSI_INTR_NO);
    LOGT("[STEP 1] GIC IRQ enabled for DSI_INTR_NO");

    /* Step 2: Initialize interrupt and PHY variables */
    int_pend  = 1;
    int_pend1 = 0x3;
    LOGT("[STEP 2] int_pend=1, int_pend1=0x3");

    LOGT("mipi_dsi_basic_test init: complete");
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
    unsigned int phy_stop_wait_time;
    unsigned int n_lanes;
    unsigned int phy_if_cfg;
    unsigned int num_of_pixel;
    unsigned int wr_cmd_size;
    unsigned int num_bytes;
    unsigned int num_descriptors;
    unsigned long long ch0_desc_addr;
    unsigned long long ch1_desc_addr;
    unsigned long long ch0_desc_addr_act;
    unsigned long long ch1_desc_addr_act;
    double dpi_clk_time_period;
    double dpi_clk_freq;
    int itter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting main testcase flow");

    /* Step 2 continued: Initialize PHY configuration variables */
    phy_stop_wait_time = 0x40;
    n_lanes = 3;
    LOGT("[STEP 2] phy_stop_wait_time=0x%x, n_lanes=%u", phy_stop_wait_time, n_lanes);

    /* Step 3: Enable DMA interrupts for CH0 and CH1 */
    writel(0x3, MIZAR_MIPI_DSI_DMAC_INTEN);
    LOGT("[STEP 3] DMAC_INTEN = 0x3");

    /* Step 4: Enable GDMA interrupt at subsystem level */
    writel(MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR,
           MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE);
    LOGT("[STEP 4] SUBSYS_INTERRUPT_ENABLE = GDMA_INTR");

    /* Step 5: Compute phy_if_cfg with n_lanes and phy_stop_wait_time */
    phy_if_cfg = n_lanes;
    set_data_mask(&phy_if_cfg, phy_stop_wait_time,
                  MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
    LOGT("[STEP 5] phy_if_cfg computed = 0x%x", phy_if_cfg);

    /* Step 6: Write PHY_IF_CFG */
    writel(phy_if_cfg, MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    LOGT("[STEP 6] PHY_IF_CFG = 0x%x", phy_if_cfg);

    /* Step 7: Configure packet handling */
    writel(0x3d, MIZAR_MIPI_DSI_HOST_PCKHDL_CFG);
    LOGT("[STEP 7] PCKHDL_CFG = 0x3d");

    /* Step 8: Configure clock manager */
    writel(0x107, MIZAR_MIPI_DSI_HOST_CLKMGR_CFG);
    LOGT("[STEP 8] CLKMGR_CFG = 0x107");

    /* Step 9: Enable DBI mode */
    writel(0, MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL);
    LOGT("[STEP 9] DPI_CONTROL = 0 (DBI mode enabled)");

    /* Step 10: Initialize DSI PHY */
    phy_init();
    LOGT("[STEP 10] PHY initialized");

    /* Step 11: Set DBI configuration variables */
    dbi_vcid              = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf         = 0x1;
    out_dbi_conf          = 0xb;
    in_dbi_conf           = 0x0;
    partitioning_en       = 0x1;
    allowed_cmd_size      = 0x7;
    LOGT("[STEP 11] DBI config vars set");

    /* Step 12: Compute pixel-to-byte conversion */
    num_of_pixel = 40;
    pixel_to_bytes_wr_cmd_size(num_of_pixel);
    wr_cmd_size = pixel_attr.wr_cmd_size;
    num_bytes   = pixel_attr.num_bytes;
    LOGT("[STEP 12] num_of_pixel=%u, wr_cmd_size=%u, num_bytes=%u",
         num_of_pixel, wr_cmd_size, num_bytes);

    /* Step 13: Set command type variables */
    max_rd_pkt_size = 0;
    dcs_lw_tx       = 0;
    dcs_sr_0p_tx    = 0;
    dcs_sw_1p_tx    = 0;
    dcs_sw_0p_tx    = 0;
    gen_lw_tx       = 0;
    gen_sr_2p_tx    = 0;
    gen_sr_1p_tx    = 0;
    gen_sr_0p_tx    = 0;
    gen_sw_2p_tx    = 0;
    gen_sw_1p_tx    = 0;
    gen_sw_0p_tx    = 0;
    ack_rqst_en     = 0;
    tear_fx_en      = 1;
    generic_vc_id   = 0x2;
    LOGT("[STEP 13] Command type variables set");

    /* Step 14: Program DPI clock */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    program_dpi_clock(dpi_clk_freq);
    LOGT("[STEP 14] DPI clock programmed");

    /* Step 15: Apply DBI configuration */
    dbi_config();
    LOGT("[STEP 15] DBI configuration applied");

    /* Step 16: Set number of descriptors */
    num_descriptors = 1;

    /* Step 17: Set DMA descriptor addresses */
    ch0_desc_addr     = RAM_BASE + 0x3F000;
    ch1_desc_addr     = RAM_BASE + 0x3F800;
    ch0_desc_addr_act = ch0_desc_addr;
    ch1_desc_addr_act = ch1_desc_addr;
    LOGT("[STEP 17] CH0 desc addr=0x%llx, CH1 desc addr=0x%llx",
         (unsigned long long)ch0_desc_addr_act,
         (unsigned long long)ch1_desc_addr_act);

    /* Step 18: Descriptor loop */
    for (itter = 0; itter < (int)num_descriptors; itter++) {

        /* Step 18a: Data TX descriptor */
        data_tdbdcb[itter].addr = RAM_BASE + 0x10000;
        data_tdbdcb[itter].len  = num_bytes;
        data_tdbdcb[itter].eop  = 1;

        /* Step 18b: Data RX descriptor */
        data_rebdcb[itter].addr = 0x10000000000ULL;
        data_rebdcb[itter].len  = num_bytes;

        /* Step 18c: Command TX descriptor */
        cmd_tdbdcb[itter].addr  = RAM_BASE + 0x0000;
        cmd_tdbdcb[itter].len   = 4;
        cmd_tdbdcb[itter].eop   = 1;

        /* Step 18d: Command RX descriptor */
        cmd_rebdcb[itter].addr  = 0x10000008000ULL;
        cmd_rebdcb[itter].len   = 4;

        LOGT("[STEP 18] Descriptor %d configured", itter);

        /* Step 18e: Program CH0 DMA microcode (data channel) */
        DMAMOV(&ch0_desc_addr, DMA_SAR, data_tdbdcb[itter].addr);
        DMAMOV(&ch0_desc_addr, DMA_DAR, data_rebdcb[itter].addr);
        program_data_num_bytes(&ch0_desc_addr, data_tdbdcb[itter].len);
        DMAWMB(&ch0_desc_addr);
        DMASEV(&ch0_desc_addr, 0);
        DMAEND(&ch0_desc_addr);

        /* Step 18f: Program CH1 DMA microcode (command channel) */
        DMAMOV(&ch1_desc_addr, DMA_SAR, cmd_tdbdcb[itter].addr);
        DMAMOV(&ch1_desc_addr, DMA_DAR, cmd_rebdcb[itter].addr);
        program_data_num_bytes(&ch1_desc_addr, cmd_tdbdcb[itter].len);
        DMAWMB(&ch1_desc_addr);
        DMASEV(&ch1_desc_addr, 1);
        DMAEND(&ch1_desc_addr);

        /* Step 18g: Load random data into SRAM */
        load_rand_data(data_tdbdcb[itter]);
        LOGT("[STEP 18g] Random data loaded for descriptor %d", itter);

        /* Step 18h: Load write command into SRAM */
        load_wr_command(cmd_tdbdcb[itter].addr, num_bytes,
                        DSI_WRITE_MEMORY_START);
        LOGT("[STEP 18h] Write command loaded for descriptor %d", itter);
    }

    /* Step 19: Write CH0 GO instruction to DBGINST0 */
    writel(0x00A00000, MIZAR_MIPI_DSI_DMAC_DBGINST0);
    LOGT("[STEP 19] DMAC_DBGINST0 = 0x00A00000 (CH0 GO)");

    /* Step 20: Write CH0 descriptor address to DBGINST1 */
    writel(ch0_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGINST1);
    LOGT("[STEP 20] DMAC_DBGINST1 = 0x%llx (CH0 desc addr)",
         (unsigned long long)ch0_desc_addr_act);

    /* Step 21: Execute CH0 DMA instruction */
    writel(0x0, MIZAR_MIPI_DSI_DMAC_DBGCMD);
    LOGT("[STEP 21] DMAC_DBGCMD = 0x0 (CH0 execute)");

    /* Step 22: Write CH1 GO instruction to DBGINST0 */
    writel(0x01A00000, MIZAR_MIPI_DSI_DMAC_DBGINST0);
    LOGT("[STEP 22] DMAC_DBGINST0 = 0x01A00000 (CH1 GO)");

    /* Step 23: Write CH1 descriptor address to DBGINST1 */
    writel(ch1_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGINST1);
    LOGT("[STEP 23] DMAC_DBGINST1 = 0x%llx (CH1 desc addr)",
         (unsigned long long)ch1_desc_addr_act);

    /* Step 24: Execute CH1 DMA instruction */
    writel(0x0, MIZAR_MIPI_DSI_DMAC_DBGCMD);
    LOGT("[STEP 24] DMAC_DBGCMD = 0x0 (CH1 execute)");

    /* Step 25: Poll for DMA completion */
    // MANUAL_REVIEW: The original DV test uses unbounded while(int_pend1) polling
    // with wait_on(10). No timeout macro or timeout polling pattern was provided
    // in the FV Template or Meta TestPlan JSON. A bounded timeout should be added
    // for PSV/FV execution to prevent infinite hangs.
    LOGT("[STEP 25] Polling for DMA completion (int_pend1=0x%x)", int_pend1);
    while (int_pend1) {
        wait_on(10);
    }
    LOGT("[STEP 25] DMA transfer complete, int_pend1=0x%x", int_pend1);

    /* Step 26: Final settling delay */
    wait_on(10000);
    LOGT("[STEP 26] Final settling delay complete");

    /* Step 27: Report final status */
    // MANUAL_REVIEW: DV finish(err0) converted to PSV/FV out->status reporting.
    g_ctx.checks_failed = g_ctx.errors;
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
    return g_ctx.errors == 0U ? 0 : -1;
}
