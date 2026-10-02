// Author - AI Force 2.3. 18-Jul-2025 05:59 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_basic_test
 * Feature    : DBI Command Mode DMA Write
 * Description: This testcase performs a basic MIPI DSI DBI write operation using
 *              DMA. It configures the DSI host PHY with 4 lanes, sets packet
 *              handling and clock manager configurations, enables DBI command mode,
 *              initializes the PHY, configures DBI parameters, builds DMA microcode
 *              for data (CH0) and command (CH1) channels, triggers DMA execution
 *              via the DMAC debug interface, and verifies completion through an
 *              interrupt-driven ISR. The test passes when both DMA channels complete
 *              without unexpected interrupts.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/* Global variables for interrupt handling */
static volatile int int_pend  = 1;
static volatile int int_pend1 = 0x3;

/*
 * Function: Default_IRQHandler
 * Description: ISR for DSI interrupts. Handles GDMA interrupt from DSI subsystem,
 *              identifies completed DMA channels, clears DMAC and subsystem-level
 *              interrupts, and tracks unexpected interrupt errors.
 * Parameters:
 *   None (ISR context).
 * Returns:
 *   void.
 */
void Default_IRQHandler(void)
{
    unsigned int dsi_subsys_mask_st;
    unsigned int ch_mask_st;

    /* Step 34: ISR fires on interrupt, set int_pend=0 */
    int_pend = 0;

    /* Step 35: Read MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK */
    dsi_subsys_mask_st = readl_reg((uintptr_t)MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);
    LOGT("[ISR] DSI Subsystem Interrupt Mask: 0x%x", dsi_subsys_mask_st);

    /* Step 36: Check if dsi_subsys_mask_st equals MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR */
    if (dsi_subsys_mask_st == MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {

        /* Step 37: Read MIZAR_MIPI_DSI_DMAC_INTMIS */
        ch_mask_st = readl_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTMIS);
        LOGT("[ISR] DMAC INTMIS: 0x%x", ch_mask_st);

        if (ch_mask_st != 0U) {
            /* Step 38: Clear corresponding bit in int_pend1 */
            int_pend1 = int_pend1 & (~ch_mask_st);
            LOGT("[ISR] int_pend1 updated to: 0x%x", int_pend1);

            /* Step 39: Clear DMAC channel interrupt */
            writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTCLR, ch_mask_st);
            LOGT("[ISR] Cleared DMAC interrupt: 0x%x", ch_mask_st);
        } else {
            /* Step 40: Unexpected - INTMIS is zero */
            LOGE("[ISR] ERROR: GDMA interrupt received but INTMIS is zero");
            g_ctx.errors++;
        }

        /* Step 41: Clear subsystem interrupt */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, dsi_subsys_mask_st);
        LOGT("[ISR] Cleared subsystem interrupt: 0x%x", dsi_subsys_mask_st);

    } else {
        /* Step 42: Unexpected interrupt source */
        LOGE("[ISR] ERROR: Unexpected interrupt source: 0x%x", dsi_subsys_mask_st);
        g_ctx.errors++;
    }

    /* Step 43: Clear GIC-level interrupt */
    GIC_ClearIRQ(DSI_INTR_NO);

    /* Step 44: Re-enable polling */
    int_pend = 1;
}

/*
 * Function: mipi_dsi_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_dsi_basic_test. Enables GIC IRQ, configures DMAC and subsystem
 *              interrupts, sets up PHY interface, packet handling, clock manager,
 *              DBI mode, PHY initialization, and DBI configuration.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_init(const TestsItem *cfg)
{
    unsigned int phy_if_cfg;
    unsigned int phy_stop_wait_time = 0x40;
    unsigned int n_lanes = 3;
    unsigned int num_of_pixel = 40;
    double dpi_clk_time_period = 16.012400;
    unsigned int dpi_clk_freq;

    (void)cfg;

    /* Initialize test context */
    g_ctx.errors = 0U;
    int_pend  = 1;
    int_pend1 = 0x3;

    LOGT("mipi_dsi_basic_test: init start");

    /* Step 1: Enable GIC IRQ for DSI_INTR_NO */
    GIC_EnableIRQ(DSI_INTR_NO);
    LOGT("Enabled GIC IRQ for DSI_INTR_NO");

    /* Step 2: Initialize variables (int_pend, int_pend1 already set above) */
    LOGT("Initialized int_pend=1, int_pend1=0x3, phy_stop_wait_time=0x%x, n_lanes=%u",
         phy_stop_wait_time, n_lanes);

    /* Step 3: Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTEN, 0x3);
    LOGT("Wrote 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");

    /* Step 4: Enable GDMA interrupt at subsystem level */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE,
              MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);
    LOGT("Enabled GDMA interrupt at subsystem level");

    /* Step 5: Compute phy_if_cfg */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                              phy_stop_wait_time);
    LOGT("Computed phy_if_cfg: 0x%x", phy_if_cfg);

    /* Step 6: Write phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("Wrote phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");

    /* Step 7: Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d);
    LOGT("Wrote 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");

    /* Step 8: Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107);
    LOGT("Wrote 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");

    /* Step 9: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to enable DBI mode */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0);
    LOGT("Wrote 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (DBI mode enabled)");

    /* Step 10: Initialize DSI PHY */
    phy_init();
    LOGT("Called phy_init()");

    /* Step 11: Set DBI configuration variables */
    dbi_vcid             = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf        = 0x1;
    out_dbi_conf         = 0xb;
    in_dbi_conf          = 0x0;
    partitioning_en      = 0x1;
    allowed_cmd_size     = 0x7;
    LOGT("Set DBI config: dbi_vcid=0x3, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x1, allowed_cmd_size=0x7");

    /* Step 12: Compute pixel-to-byte conversion for 40 pixels */
    pixel_to_bytes_wr_cmd_size(num_of_pixel);
    LOGT("Called pixel_to_bytes_wr_cmd_size(%u)", num_of_pixel);

    /* Step 13: Set DBI command mode parameters */
    max_rd_pkt_size = 0x0;
    dcs_lw_tx       = 0x0;
    dcs_sr_0p_tx    = 0x0;
    dcs_sw_1p_tx    = 0x0;
    dcs_sw_0p_tx    = 0x0;
    gen_lw_tx       = 0x0;
    gen_sr_2p_tx    = 0x0;
    gen_sr_1p_tx    = 0x0;
    gen_sr_0p_tx    = 0x0;
    gen_sw_2p_tx    = 0x0;
    gen_sw_1p_tx    = 0x0;
    gen_sw_0p_tx    = 0x0;
    ack_rqst_en     = 0x0;
    tear_fx_en      = 0x1;
    generic_vc_id   = 0x2;
    LOGT("Set DBI command mode parameters");

    /* Step 14: Compute dpi_clk_freq and program DPI clock */
    dpi_clk_freq = (unsigned int)(1000000000.0 / dpi_clk_time_period);
    program_dpi_clock(dpi_clk_freq);
    LOGT("Programmed DPI clock: %u Hz", dpi_clk_freq);

    /* Step 15: Apply DBI configuration */
    dbi_config();
    LOGT("Called dbi_config()");

    LOGT("mipi_dsi_basic_test: init complete");
    return 0;
}

/*
 * Function: mipi_dsi_basic_test_run
 * Description: Executes the main testcase flow for mipi_dsi_basic_test. Builds DMA
 *              microcode descriptors for data and command channels, loads pixel data
 *              and write command into SRAM, starts DMA channels via debug interface,
 *              and polls for interrupt-driven completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int num_bytes;
    unsigned int num_descriptors = 1;
    unsigned long long ch0_desc_addr;
    unsigned long long ch1_desc_addr;
    unsigned long long ch0_desc_addr_act;
    unsigned long long ch1_desc_addr_act;
    unsigned int timeout;
    int itter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test: run start");

    /* Retrieve computed num_bytes from pixel_attr */
    num_bytes = pixel_attr.num_bytes;
    LOGT("num_bytes=%u, wr_cmd_size=%u", num_bytes, pixel_attr.wr_cmd_size);

    /* Step 16: Set num_descriptors=1 */
    num_descriptors = 1;

    /* Step 17: Set descriptor addresses */
    ch0_desc_addr     = RAM_BASE + 0x3F000;
    ch1_desc_addr     = RAM_BASE + 0x3F800;
    ch0_desc_addr_act = ch0_desc_addr;
    ch1_desc_addr_act = ch1_desc_addr;
    LOGT("CH0 desc addr=0x%llx, CH1 desc addr=0x%llx",
         (unsigned long long)ch0_desc_addr, (unsigned long long)ch1_desc_addr);

    /* Steps 18-23: Build DMA descriptors and microcode */
    for (itter = 0; itter < (int)num_descriptors; itter++) {

        /* Step 18: Data channel (CH0) transmit descriptor */
        data_tdbdcb[itter].addr = RAM_BASE + 0x10000;
        data_tdbdcb[itter].len  = num_bytes;
        data_tdbdcb[itter].eop  = 1;
        LOGT("data_tdbdcb[%d]: addr=0x%llx len=%u eop=%u",
             itter, (unsigned long long)data_tdbdcb[itter].addr,
             data_tdbdcb[itter].len, data_tdbdcb[itter].eop);

        /* Step 19: Data channel (CH0) receive descriptor */
        data_rebdcb[itter].addr = 0x10000000000ULL;
        data_rebdcb[itter].len  = num_bytes;
        LOGT("data_rebdcb[%d]: addr=0x%llx len=%u",
             itter, (unsigned long long)data_rebdcb[itter].addr,
             data_rebdcb[itter].len);

        /* Step 20: Command channel (CH1) transmit descriptor */
        cmd_tdbdcb[itter].addr = RAM_BASE + 0x0000;
        cmd_tdbdcb[itter].len  = 4;
        cmd_tdbdcb[itter].eop  = 1;
        LOGT("cmd_tdbdcb[%d]: addr=0x%llx len=%u eop=%u",
             itter, (unsigned long long)cmd_tdbdcb[itter].addr,
             cmd_tdbdcb[itter].len, cmd_tdbdcb[itter].eop);

        /* Step 21: Command channel (CH1) receive descriptor */
        cmd_rebdcb[itter].addr = 0x10000008000ULL;
        cmd_rebdcb[itter].len  = 4;
        LOGT("cmd_rebdcb[%d]: addr=0x%llx len=%u",
             itter, (unsigned long long)cmd_rebdcb[itter].addr,
             cmd_rebdcb[itter].len);

        /* Step 22: Build CH0 DMA microcode */
        LOGT("Building CH0 DMA microcode at 0x%llx", (unsigned long long)ch0_desc_addr_act);
        DMAMOV(ch0_desc_addr_act, SAR, data_tdbdcb[itter].addr);
        DMAMOV(ch0_desc_addr_act, DAR, data_rebdcb[itter].addr);
        program_data_num_bytes(ch0_desc_addr_act, data_tdbdcb[itter].len);
        DMAWMB(ch0_desc_addr_act);
        DMASEV(ch0_desc_addr_act, 0);
        DMAEND(ch0_desc_addr_act);

        /* Step 23: Build CH1 DMA microcode */
        LOGT("Building CH1 DMA microcode at 0x%llx", (unsigned long long)ch1_desc_addr_act);
        DMAMOV(ch1_desc_addr_act, SAR, cmd_tdbdcb[itter].addr);
        DMAMOV(ch1_desc_addr_act, DAR, cmd_rebdcb[itter].addr);
        program_data_num_bytes(ch1_desc_addr_act, cmd_tdbdcb[itter].len);
        DMAWMB(ch1_desc_addr_act);
        DMASEV(ch1_desc_addr_act, 1);
        DMAEND(ch1_desc_addr_act);
    }

    /* Step 24: Load random pixel data into SRAM */
    load_rand_data(data_tdbdcb[0]);
    LOGT("Loaded random pixel data via load_rand_data()");

    /* Step 25: Load write_memory_start command into SRAM */
    load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START);
    LOGT("Loaded write_memory_start command via load_wr_command()");

    /* Step 26: Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for CH0 */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000);
    LOGT("Wrote 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH0 DMAGO)");

    /* Step 27: Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);
    LOGT("Wrote CH0 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1");

    /* Step 28: Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH0 DMAGO */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH0 execute)");

    /* Step 29: Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for CH1 */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000);
    LOGT("Wrote 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH1 DMAGO)");

    /* Step 30: Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);
    LOGT("Wrote CH1 descriptor address to MIZAR_MIPI_DSI_DMAC_DBGINST1");

    /* Step 31: Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH1 DMAGO */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH1 execute)");

    /* Step 32: Poll int_pend1 with timeout until both channels complete */
    LOGT("Polling int_pend1 for DMA completion");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    while ((int_pend1 != 0) && (timeout > 0U)) {
        wait_on(10);
        timeout--;
    }

    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA completion, int_pend1=0x%x", int_pend1);
        g_ctx.errors++;
    } else {
        LOGT("DMA completion detected, int_pend1=0x%x", int_pend1);
    }

    /* Step 33: Final settling delay */
    wait_on(10000);
    LOGT("Final settling delay complete");

    /* Step 45: Report final status via out->status */
    // MANUAL_REVIEW: DV finish(err0) converted to PSV/FV out->status pattern.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_basic_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_dsi_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_basic_test teardown: errors=%u", g_ctx.errors);

    /* Validation: The test passes when err0==0 (no unexpected interrupts) */
    if (g_ctx.errors != 0U) {
        LOGE("mipi_dsi_basic_test: FAIL - unexpected interrupt errors detected, errors=%u",
             g_ctx.errors);
    } else {
        LOGT("mipi_dsi_basic_test: PASS - both DMA channels completed without errors");
    }

    LOGT("mipi_dsi_basic_test teardown: complete");
    return g_ctx.errors == 0U ? 0 : -1;
}
