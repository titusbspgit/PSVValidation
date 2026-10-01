// Author - AI Force 2.3. 18-Jul-2025 01:07 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * mipi_dsi_basic_test
 * This testcase performs a basic MIPI DSI DBI write operation using DMA.
 * It configures the DSI host PHY, enables DBI mode, sets up DMA descriptors
 * for data and command channels, triggers DMA execution via the DMAC debug
 * interface, and verifies completion through an interrupt-driven ISR.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/* Volatile ISR-shared variables */
static volatile unsigned int int_pend;
static volatile unsigned int int_pend1;
static volatile unsigned int err0;

/*
 * ISR: Default_IRQHandler
 * Handles DSI subsystem GDMA interrupts for DMA channel completion.
 */
void Default_IRQHandler(void)
{
 unsigned int dsi_subsys_mask_st;
    unsigned int ch_mask_st;

    /* Step 34: ISR fires on interrupt, set int_pend=0 */
    int_pend = 0;

    /* Step 35: Read subsystem interrupt mask */
    dsi_subsys_mask_st = readl_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK);

    /* Step 36: Check if GDMA interrupt */
    if (dsi_subsys_mask_st == MIPI_DSI_SUBSYS_INTERRUPT_MASK_GDMA_INTR) {
        /* Step 37: Read DMAC masked interrupt status */
        ch_mask_st = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);

        if (ch_mask_st != 0U) {
            /* Step 38: Clear corresponding bit in int_pend1 */
            int_pend1 = int_pend1 & (~ch_mask_st);

            /* Step 39: Clear DMAC channel interrupt */
            writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, ch_mask_st);
        } else {
            /* Step 40: ch_mask_st is zero, unexpected */
            LOGE("ISR: DMAC INTMIS is zero, no channel completion detected");
            err0++;
        }

        /* Step 41: Clear subsystem interrupt */
        writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW, dsi_subsys_mask_st);
    } else {
        /* Step 42: Unexpected interrupt source */
        LOGE("ISR: Unexpected subsystem interrupt source: 0x%x",
             dsi_subsys_mask_st);
        err0++;
    }

    /* Step 43: Clear GIC-level interrupt */
    GIC_ClearIRQ(DSI_INTR_NO);

    /* Step 44: Re-enable polling */
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
    unsigned int phy_if_cfg;
    double dpi_clk_time_period;
    double dpi_clk_freq;

    (void)cfg;

    g_ctx = (mipi_dsi_basic_test_ctx_t){0};
    err0 = 0U;

    LOGT("mipi_dsi_basic_test init: starting initialization");

    /* Step 1: Enable GIC IRQ for DSI_INTR_NO */
    GIC_EnableIRQ(DSI_INTR_NO);

    /* Step 2: Initialize ISR-shared variables and PHY parameters */
    int_pend = 1;
    int_pend1 = 0x3;
    phy_stop_wait_time = 0x40;
    n_lanes = 3;

    /* Step 3: Enable DMAC interrupts for CH0 and CH1 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3);
    LOGT("Wrote 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");

    /* Step 4: Enable GDMA interrupt at subsystem level */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE,
              MIPI_DSI_SUBSYS_INTERRUPT_ENABLE_GDMA_INTR);
    LOGT("Enabled GDMA interrupt at subsystem level");

    /* Step 5: Compute phy_if_cfg with n_lanes and phy_stop_wait_time */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                              0x40);

    /* Step 6: Write phy_if_cfg to PHY_IF_CFG register */
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("Wrote phy_if_cfg=0x%x to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG",
         phy_if_cfg);

    /* Step 7: Write PCKHDL_CFG */
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d);
    LOGT("Wrote 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");

    /* Step 8: Write CLKMGR_CFG */
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107);
    LOGT("Wrote 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");

    /* Step 9: Enable DBI mode by writing 0 to DPI_CONTROL */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (DBI mode enabled)");

    /* Step 10: Initialize DSI PHY */
    phy_init();
    LOGT("PHY initialization complete");

    /* Step 11: Set DBI configuration variables */
    dbi_vcid = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf = 0x1;
    out_dbi_conf = 0xb;
    in_dbi_conf = 0x0;
    partitioning_en = 0x1;
    allowed_cmd_size = 0x7;

    /* Step 12: Compute pixel-to-byte conversion for 40 pixels */
    num_of_pixel = 40;
    pixel_to_bytes_wr_cmd_size(40);
    LOGT("Computed wr_cmd_size and num_bytes for %u pixels", num_of_pixel);

    /* Step 13: Set DBI command mode parameters */
    max_rd_pkt_size = 0x0;
    dcs_lw_tx = 0x0;
    dcs_sr_0p_tx = 0x0;
    dcs_sw_1p_tx = 0x0;
    dcs_sw_0p_tx = 0x0;
    gen_lw_tx = 0x0;
    gen_sr_2p_tx = 0x0;
    gen_sr_1p_tx = 0x0;
    gen_sr_0p_tx = 0x0;
    gen_sw_2p_tx = 0x0;
    gen_sw_1p_tx = 0x0;
    gen_sw_0p_tx = 0x0;
    ack_rqst_en = 0x0;
    tear_fx_en = 0x1;
    generic_vc_id = 0x2;

    /* Step 14: Compute and program DPI clock frequency */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    program_dpi_clock(dpi_clk_freq);
    LOGT("Programmed DPI clock frequency");

    /* Step 15: Apply DBI configuration to DSI host registers */
    dbi_config();
    LOGT("DBI configuration applied");

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
    unsigned int num_descriptors;
    uintptr_t ch0_desc_addr;
    uintptr_t ch1_desc_addr;
    uintptr_t ch0_desc_addr_act;
    uintptr_t ch1_desc_addr_act;
    unsigned int itter;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test run: starting DMA transfer sequence");

    /* Step 16: Set number of descriptors */
    num_descriptors = 1;

    /* Step 17: Set DMA descriptor addresses */
    ch0_desc_addr = RAM_BASE + 0x3F000;
    ch1_desc_addr = RAM_BASE + 0x3F800;
    ch0_desc_addr_act = ch0_desc_addr;
    ch1_desc_addr_act = ch1_desc_addr;

    /* Step 18: Configure data channel descriptor (CH0) */
    for (itter = 0; itter < num_descriptors; itter++) {
        data_tdbdcb[itter].addr = RAM_BASE + 0x10000;
        data_tdbdcb[itter].len = num_bytes;
        data_tdbdcb[itter].eop = 1;

        /* Step 19: Configure data receive descriptor */
        data_rebdcb[itter].addr = 0x10000000000ULL;
        data_rebdcb[itter].len = num_bytes;

        /* Step 20: Configure command channel descriptor (CH1) */
        cmd_tdbdcb[itter].addr = RAM_BASE + 0x0000;
        cmd_tdbdcb[itter].len = 4;
        cmd_tdbdcb[itter].eop = 1;

        /* Step 21: Configure command receive descriptor */
        cmd_rebdcb[itter].addr = 0x10000008000ULL;
        cmd_rebdcb[itter].len = 4;
    }

    /* Step 22: Build CH0 DMA microcode at ch0_desc_addr */
    DMAMOV(ch0_desc_addr_act, SAR, data_tdbdcb[0].addr);
    DMAMOV(ch0_desc_addr_act, DAR, data_rebdcb[0].addr);
    program_data_num_bytes(ch0_desc_addr_act, data_tdbdcb[0].len);
    DMAWMB(ch0_desc_addr_act);
    DMASEV(ch0_desc_addr_act, 0);
    DMAEND(ch0_desc_addr_act);
    LOGT("Built CH0 DMA microcode at 0x%lx", (unsigned long)ch0_desc_addr);

    /* Step 23: Build CH1 DMA microcode at ch1_desc_addr */
    DMAMOV(ch1_desc_addr_act, SAR, cmd_tdbdcb[0].addr);
    DMAMOV(ch1_desc_addr_act, DAR, cmd_rebdcb[0].addr);
    program_data_num_bytes(ch1_desc_addr_act, 4);
    DMAWMB(ch1_desc_addr_act);
    DMASEV(ch1_desc_addr_act, 1);
    DMAEND(ch1_desc_addr_act);
    LOGT("Built CH1 DMA microcode at 0x%lx", (unsigned long)ch1_desc_addr);

    /* Step 24: Load random pixel data into SRAM */
    load_rand_data(data_tdbdcb[0]);
    LOGT("Loaded random pixel data at 0x%lx",
         (unsigned long)data_tdbdcb[0].addr);

    /* Step 25: Load write_memory_start command into SRAM */
    load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START);
    LOGT("Loaded write_memory_start command at 0x%lx",
         (unsigned long)cmd_tdbdcb[0].addr);

    /* Step 26: Start DMA CH0 - write DMAGO instruction for CH0 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000);
    LOGT("Wrote 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH0 DMAGO)");

    /* Step 27: Provide CH0 descriptor start address */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);
    LOGT("Wrote ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1");

    /* Step 28: Execute CH0 DMAGO */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH0 execute)");

    /* Step 29: Start DMA CH1 - write DMAGO instruction for CH1 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000);
    LOGT("Wrote 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH1 DMAGO)");

    /* Step 30: Provide CH1 descriptor start address */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);
    LOGT("Wrote ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1");

    /* Step 31: Execute CH1 DMAGO */
    writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH1 execute)");

    /* Step 32: Poll int_pend1 until both channels complete (ISR clears bits) */
    timeout = MIPI_DSI_POLL_TIMEOUT;
    while (int_pend1 != 0U) {
        wait_on(10);
        timeout--;
        if (timeout == 0U) {
            LOGE("Timeout waiting for DMA channel completion, int_pend1=0x%x",
                 int_pend1);
            err0++;
            break;
        }
    }

    /* Step 33: Final settling delay */
    wait_on(10000);
    LOGT("DMA transfer sequence complete");

    /* Step 45: Report final status (PSV/FV adaptation of finish(err0)) */
    g_ctx.errors = err0;
    out->status = (err0 == 0U) ? 0 : -1;

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
