// Author - AI Force 2.3. 18-Jul-2025 01:07 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * mipi_dsi_dbi_random_payload_test
 * This testcase performs a MIPI DSI DBI random payload stress test using DMA.
 * It configures the DSI host PHY, enables DBI mode, and runs 10 iterations
 * of random-sized DMA transfers via the DMAC debug interface, polling for
 * completion of both data and command channels in each iteration.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
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
    unsigned int phy_if_cfg;
    double dpi_clk_time_period;
    double dpi_clk_freq;

    (void)cfg;

    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init: starting initialization");

    /* Step 1: Initialize PHY parameters */
    phy_stop_wait_time = 0x40;
    n_lanes = 3;

    /* Step 2: Compute phy_if_cfg with n_lanes and phy_stop_wait_time */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                              0x40);

    /* Step 3: Write phy_if_cfg to PHY_IF_CFG register */
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("Wrote phy_if_cfg=0x%x to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG",
         phy_if_cfg);

    /* Step 4: Write PCKHDL_CFG */
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d);
    LOGT("Wrote 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");

    /* Step 5: Write CLKMGR_CFG */
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107);
    LOGT("Wrote 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");

    /* Step 6: Enable DBI mode by writing 0 to DPI_CONTROL */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0x0);
    LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (DBI mode enabled)");

    /* Step 7: Initialize DSI PHY */
    phy_init();
    LOGT("PHY initialization complete");

    /* Step 8: Set DBI configuration variables */
    dbi_vcid = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf = 0x1;
    out_dbi_conf = 0xb;
    in_dbi_conf = 0x0;
    partitioning_en = 0x0;
    allowed_cmd_size = 0x25;

    /* Step 9: Set initial wr_cmd_size */
    wr_cmd_size = 193;

    /* Step 10: Compute pixel-to-byte conversion for initial 40 pixels */
    num_of_pixel = 40;
    pixel_to_bytes_wr_cmd_size(40);
    LOGT("Computed initial wr_cmd_size and num_bytes for %u pixels",
         num_of_pixel);

    /* Step 11: Set DBI command mode parameters */
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

    /* Step 12: Compute and program DPI clock frequency */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    program_dpi_clock(dpi_clk_freq);
    LOGT("Programmed DPI clock frequency");

    /* Step 13: Apply initial DBI configuration to DSI host registers */
    dbi_config();
    LOGT("Initial DBI configuration applied");

    /* Step 14: Set num_descriptors */
    num_descriptors = 1;

    /* Step 15: Enable DMAC interrupts for CH0 and CH1 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3);
    LOGT("Wrote 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");

    LOGT("mipi_dsi_dbi_random_payload_test init: initialization complete");

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
    int i;
    unsigned int itter;
    uintptr_t ch0_desc_addr;
    uintptr_t ch1_desc_addr;
    uintptr_t ch0_desc_addr_act;
    uintptr_t ch1_desc_addr_act;
    unsigned int rd_data;
    unsigned int timeout;
    unsigned int aligned_len;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting %u iteration DMA stress test",
         MIPI_DSI_DBI_ITERATION_COUNT);

    /* Step 16: Begin iteration loop for 10 iterations */
    for (i = 0; i < (int)MIPI_DSI_DBI_ITERATION_COUNT; i++) {

        /* Step 17: Generate random pixel count (multiples of 8, up to 8192) */
        num_of_pixel = ((rand() % 1024)) * 8;

        /* Step 18: Recompute wr_cmd_size and num_bytes */
        pixel_to_bytes_wr_cmd_size(num_of_pixel);

        /* Step 19: Log iteration index and wr_cmd_size */
        LOGT("Iteration %d: num_of_pixel=%u wr_cmd_size=%u",
             i, num_of_pixel, wr_cmd_size);

        /* Step 20: Re-apply DBI configuration with updated wr_cmd_size */
        dbi_config();

        /* Step 21: Set DMA descriptor addresses */
        ch0_desc_addr = RAM_BASE + 0x3F000;
        ch1_desc_addr = RAM_BASE + 0x3F800;
        ch0_desc_addr_act = ch0_desc_addr;
        ch1_desc_addr_act = ch1_desc_addr;

        /* Step 22: Configure data channel descriptor (CH0) */
        for (itter = 0; itter < num_descriptors; itter++) {
            data_tdbdcb[itter].addr = RAM_BASE + 0x10000;
            data_tdbdcb[itter].len = num_bytes;
            data_tdbdcb[itter].eop = 1;

            /* Step 23: Configure data receive descriptor */
            data_rebdcb[itter].addr = 0x10000000000ULL;
            data_rebdcb[itter].len = num_bytes;

            /* Step 24: Configure command channel descriptor (CH1) */
            cmd_tdbdcb[itter].addr = RAM_BASE + 0x0000;
            cmd_tdbdcb[itter].len = 4;
            cmd_tdbdcb[itter].eop = 1;

            /* Step 25: Configure command receive descriptor */
            cmd_rebdcb[itter].addr = 0x10000008000ULL;
            cmd_rebdcb[itter].len = 4;
        }

        /* Step 26: Build CH0 DMA microcode at ch0_desc_addr */
        DMAMOV(ch0_desc_addr_act, SAR, data_tdbdcb[0].addr);
        DMAMOV(ch0_desc_addr_act, DAR, data_rebdcb[0].addr);
        aligned_len = (unsigned int)(ceil((double)data_tdbdcb[0].len / 8.0)) * 8;
        program_data_num_bytes(ch0_desc_addr_act, aligned_len);
        DMAWMB(ch0_desc_addr_act);
        DMASEV(ch0_desc_addr_act, 0);
        DMAEND(ch0_desc_addr_act);
        LOGT("Built CH0 DMA microcode at 0x%lx, aligned_len=%u",
             (unsigned long)ch0_desc_addr, aligned_len);

        /* Step 27: Build CH1 DMA microcode at ch1_desc_addr */
        DMAMOV(ch1_desc_addr_act, SAR, cmd_tdbdcb[0].addr);
        DMAMOV(ch1_desc_addr_act, DAR, cmd_rebdcb[0].addr);
        program_data_num_bytes(ch1_desc_addr_act, 4);
        DMAWMB(ch1_desc_addr_act);
        DMASEV(ch1_desc_addr_act, 1);
        DMAEND(ch1_desc_addr_act);
        LOGT("Built CH1 DMA microcode at 0x%lx",
             (unsigned long)ch1_desc_addr);

        /* Step 28: Load random pixel data into SRAM */
        load_rand_data(data_tdbdcb[0]);
        LOGT("Loaded random pixel data at 0x%lx",
             (unsigned long)data_tdbdcb[0].addr);

        /* Step 29: Load write_memory_start command into SRAM */
        load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START);
        LOGT("Loaded write_memory_start command at 0x%lx",
             (unsigned long)cmd_tdbdcb[0].addr);

        /* Step 30: Start DMA CH0 - write DMAGO instruction for CH0 */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000);
        LOGT("Wrote 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH0 DMAGO)");

        /* Step 31: Provide CH0 descriptor start address */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);
        LOGT("Wrote ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1");

        /* Step 32: Execute CH0 DMAGO */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
        LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH0 execute)");

        /* Step 33: Start DMA CH1 - write DMAGO instruction for CH1 */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000);
        LOGT("Wrote 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 (CH1 DMAGO)");

        /* Step 34: Provide CH1 descriptor start address */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);
        LOGT("Wrote ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1");

        /* Step 35: Execute CH1 DMAGO */
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
        LOGT("Wrote 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD (CH1 execute)");

        /* Step 36-37: Poll MIZAR_MIPI_DSI_DMAC_INTMIS until rd_data == 0x3 */
        rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        timeout = MIPI_DSI_POLL_TIMEOUT;
        while (rd_data != 0x3U) {
            rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
            timeout--;
            if (timeout == 0U) {
                LOGE("Iteration %d: Timeout polling DMAC_INTMIS, rd_data=0x%x",
                     i, rd_data);
                g_ctx.errors++;
                break;
            }
        }

        /* Step 38: Clear both channel interrupts */
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);
        LOGT("Iteration %d: Cleared DMAC interrupts, rd_data=0x%x", i, rd_data);

        /* Step 39: Settling delay */
        wait_on(100000);
        LOGT("Iteration %d: complete", i);
    }

    /* Step 41: Report final status (PSV/FV adaptation of finish(0)) */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
