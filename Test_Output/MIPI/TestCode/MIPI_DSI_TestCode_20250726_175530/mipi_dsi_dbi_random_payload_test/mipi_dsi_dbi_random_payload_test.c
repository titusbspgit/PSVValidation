// Author - AI Force 2.3. 26-Jul-2025 17:55 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Testcase     : mipi_dsi_dbi_random_payload_test
 * Description  : MIPI DSI DBI data transfers with randomized pixel payload
 *                sizes using a 2-channel DMA engine. CH0 for data transfer,
 *                CH1 for command transfer. 10 iterations with random pixel
 *                counts (multiples of 8). Polling-based DMA completion.
 */

/* ---------------------------------------------------------------------------
 * Testcase context
 * --------------------------------------------------------------------------- */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    unsigned int iterations_completed;
} mipi_dsi_dbi_random_payload_test_ctx_t;

static mipi_dsi_dbi_random_payload_test_ctx_t g_ctx;

/* ---------------------------------------------------------------------------
 * Global arrays: DMA descriptor control blocks and command arrays
 * --------------------------------------------------------------------------- */
dma_desc_t data_tdbdcb[10];
dma_desc_t data_rebdcb[10];
dma_desc_t cmd_tdbdcb[10];
dma_desc_t cmd_rebdcb[10];

unsigned int wr_cmd_arr[27];
unsigned int WR_CMD_SIZE[27];

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
    (void)cfg;

    /* Initialize testcase context */
    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init: starting initialization");

    LOGT("mipi_dsi_dbi_random_payload_test init: complete");
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
    unsigned int rd_data;
    unsigned int data_len_aligned;
    int i;
    int itter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run: starting main testcase flow");

    /* ------------------------------------------------------------------
     * Step 1: Set PHY configuration variables
     * ------------------------------------------------------------------ */
    phy_stop_wait_time = 0x40;
    n_lanes = 3;
    LOGT("[STEP 1] phy_stop_wait_time=0x%x, n_lanes=%u",
         phy_stop_wait_time, n_lanes);

    /* ------------------------------------------------------------------
     * Step 2: Compute phy_if_cfg with n_lanes and phy_stop_wait_time
     * ------------------------------------------------------------------ */
    phy_if_cfg = n_lanes;
    set_data_mask(&phy_if_cfg, phy_stop_wait_time,
                  MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);
    LOGT("[STEP 2] phy_if_cfg computed = 0x%x", phy_if_cfg);

    /* ------------------------------------------------------------------
     * Step 3: Write PHY_IF_CFG
     * ------------------------------------------------------------------ */
    writel(phy_if_cfg, MIZAR_MIPI_DSI_HOST_PHY_IF_CFG);
    LOGT("[STEP 3] PHY_IF_CFG = 0x%x", phy_if_cfg);

    /* ------------------------------------------------------------------
     * Step 4: Configure packet handling
     * ------------------------------------------------------------------ */
    writel(0x3d, MIZAR_MIPI_DSI_HOST_PCKHDL_CFG);
    LOGT("[STEP 4] PCKHDL_CFG = 0x3d");

    /* ------------------------------------------------------------------
     * Step 5: Configure clock manager
     * ------------------------------------------------------------------ */
    writel(0x107, MIZAR_MIPI_DSI_HOST_CLKMGR_CFG);
    LOGT("[STEP 5] CLKMGR_CFG = 0x107");

    /* ------------------------------------------------------------------
     * Step 6: Enable DBI mode
     * ------------------------------------------------------------------ */
    writel(0, MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL);
    LOGT("[STEP 6] DPI_CONTROL = 0 (DBI mode enabled)");

    /* ------------------------------------------------------------------
     * Step 7: Initialize DSI PHY
     * ------------------------------------------------------------------ */
    phy_init();
    LOGT("[STEP 7] PHY initialized");

    /* ------------------------------------------------------------------
     * Step 8: Set DBI configuration variables
     * ------------------------------------------------------------------ */
    dbi_vcid              = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf         = 0x1;
    out_dbi_conf          = 0xb;
    in_dbi_conf           = 0x0;
    partitioning_en       = 0x0;
    allowed_cmd_size      = 0x25;
    wr_cmd_size           = 193;
    LOGT("[STEP 8] DBI config vars set (partitioning_en=0x0, allowed_cmd_size=0x25, wr_cmd_size=193)");

    /* ------------------------------------------------------------------
     * Step 9: Compute initial pixel-to-byte conversion
     * ------------------------------------------------------------------ */
    num_of_pixel = 40;
    pixel_to_bytes_wr_cmd_size(num_of_pixel);
    wr_cmd_size = pixel_attr.wr_cmd_size;
    num_bytes   = pixel_attr.num_bytes;
    LOGT("[STEP 9] Initial: num_of_pixel=%u, wr_cmd_size=%u, num_bytes=%u",
         num_of_pixel, wr_cmd_size, num_bytes);

    /* ------------------------------------------------------------------
     * Step 10: Set command type variables
     * ------------------------------------------------------------------ */
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
    LOGT("[STEP 10] Command type variables set");

    /* ------------------------------------------------------------------
     * Step 11: Program DPI clock
     * ------------------------------------------------------------------ */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    program_dpi_clock(dpi_clk_freq);
    LOGT("[STEP 11] DPI clock programmed");

    /* ------------------------------------------------------------------
     * Step 12: Apply initial DBI configuration
     * ------------------------------------------------------------------ */
    dbi_config();
    LOGT("[STEP 12] Initial DBI configuration applied");

    /* ------------------------------------------------------------------
     * Step 13: Set number of descriptors
     * ------------------------------------------------------------------ */
    num_descriptors = 1;

    /* ------------------------------------------------------------------
     * Step 14: Enable DMA interrupts for CH0 and CH1
     * ------------------------------------------------------------------ */
    writel(0x3, MIZAR_MIPI_DSI_DMAC_INTEN);
    LOGT("[STEP 14] DMAC_INTEN = 0x3");

    /* ------------------------------------------------------------------
     * Step 15: Main loop - 10 iterations with random pixel counts
     * ------------------------------------------------------------------ */
    for (i = 0; i < 10; i++) {

        /* Step 15a: Generate random pixel count (multiples of 8) */
        num_of_pixel = ((rand() % 1024)) * 8;
        LOGT("[ITER %d] num_of_pixel = %u", i, num_of_pixel);

        /* Step 15b: Recompute wr_cmd_size and num_bytes */
        pixel_to_bytes_wr_cmd_size(num_of_pixel);
        wr_cmd_size = pixel_attr.wr_cmd_size;
        num_bytes   = pixel_attr.num_bytes;

        /* Step 15c: Log iteration info */
        LOGT("[ITER %d] wr_cmd_size = %u, num_bytes = %u",
             i, wr_cmd_size, num_bytes);

        /* Step 15d: Reconfigure DBI with new wr_cmd_size */
        dbi_config();
        LOGT("[ITER %d] DBI reconfigured", i);

        /* Step 15e: Set DMA descriptor addresses */
        ch0_desc_addr     = RAM_BASE + 0x3F000;
        ch1_desc_addr     = RAM_BASE + 0x3F800;
        ch0_desc_addr_act = ch0_desc_addr;
        ch1_desc_addr_act = ch1_desc_addr;

        /* Step 15f: Descriptor loop */
        for (itter = 0; itter < (int)num_descriptors; itter++) {

            /* Data TX descriptor */
            data_tdbdcb[itter].addr = RAM_BASE + 0x10000 + (itter * 0x1000);
            data_tdbdcb[itter].len  = num_bytes;
            data_tdbdcb[itter].eop  = 1;

            /* 8-byte aligned data length */
            data_len_aligned = (unsigned int)(ceil((double)data_tdbdcb[itter].len / 8.0) * 8);

            /* Data RX descriptor */
            data_rebdcb[itter].addr = 0x10000000000ULL;
            data_rebdcb[itter].len  = num_bytes;

            /* Command TX descriptor */
            cmd_tdbdcb[itter].addr  = RAM_BASE + 0x0000;
            cmd_tdbdcb[itter].len   = 4;
            cmd_tdbdcb[itter].eop   = 1;

            /* Command RX descriptor */
            cmd_rebdcb[itter].addr  = 0x10000008000ULL;
            cmd_rebdcb[itter].len   = 4;

            LOGT("[ITER %d] Descriptor %d configured (data_len_aligned=%u)",
                 i, itter, data_len_aligned);

            /* Program CH0 DMA microcode (data channel) - use aligned length */
            DMAMOV(&ch0_desc_addr, DMA_SAR, data_tdbdcb[itter].addr);
            DMAMOV(&ch0_desc_addr, DMA_DAR, data_rebdcb[itter].addr);
            program_data_num_bytes(&ch0_desc_addr, data_len_aligned);
            DMAWMB(&ch0_desc_addr);
            DMASEV(&ch0_desc_addr, 0);
            DMAEND(&ch0_desc_addr);

            /* Program CH1 DMA microcode (command channel) */
            DMAMOV(&ch1_desc_addr, DMA_SAR, cmd_tdbdcb[itter].addr);
            DMAMOV(&ch1_desc_addr, DMA_DAR, cmd_rebdcb[itter].addr);
            program_data_num_bytes(&ch1_desc_addr, cmd_tdbdcb[itter].len);
            DMAWMB(&ch1_desc_addr);
            DMASEV(&ch1_desc_addr, 1);
            DMAEND(&ch1_desc_addr);

            /* Load random data into SRAM */
            load_rand_data(data_tdbdcb[itter]);
            LOGT("[ITER %d] Random data loaded for descriptor %d", i, itter);

            /* Load write command into SRAM */
            load_wr_command(cmd_tdbdcb[itter].addr, num_bytes,
                            DSI_WRITE_MEMORY_START);
            LOGT("[ITER %d] Write command loaded for descriptor %d", i, itter);
        }

        /* Step 15g: Start CH0 DMA transfer */
        writel(0x00A00000, MIZAR_MIPI_DSI_DMAC_DBGINST0);
        LOGT("[ITER %d] DMAC_DBGINST0 = 0x00A00000 (CH0 GO)", i);

        /* Step 15h: Write CH0 descriptor address */
        writel(ch0_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGINST1);
        LOGT("[ITER %d] DMAC_DBGINST1 = CH0 desc addr", i);

        /* Step 15i: Execute CH0 DMA instruction */
        writel(0x0, MIZAR_MIPI_DSI_DMAC_DBGCMD);
        LOGT("[ITER %d] DMAC_DBGCMD = 0x0 (CH0 execute)", i);

        /* Step 15j: Start CH1 DMA transfer */
        writel(0x01A00000, MIZAR_MIPI_DSI_DMAC_DBGINST0);
        LOGT("[ITER %d] DMAC_DBGINST0 = 0x01A00000 (CH1 GO)", i);

        /* Step 15k: Write CH1 descriptor address */
        writel(ch1_desc_addr_act, MIZAR_MIPI_DSI_DMAC_DBGINST1);
        LOGT("[ITER %d] DMAC_DBGINST1 = CH1 desc addr", i);

        /* Step 15l: Execute CH1 DMA instruction */
        writel(0x0, MIZAR_MIPI_DSI_DMAC_DBGCMD);
        LOGT("[ITER %d] DMAC_DBGCMD = 0x0 (CH1 execute)", i);

        /* Step 15m: Poll INTMIS until both channels complete (rd_data == 0x3) */
        // MANUAL_REVIEW: The original DV test uses unbounded while(rd_data != 0x3)
        // polling on MIZAR_MIPI_DSI_DMAC_INTMIS. No timeout macro or timeout polling
        // pattern was provided in the FV Template or Meta TestPlan JSON. A bounded
        // timeout should be added for PSV/FV execution to prevent infinite hangs.
        LOGT("[ITER %d] Polling for DMA completion...", i);
        rd_data = 0;
        while (rd_data != 0x3) {
            rd_data = readl(MIZAR_MIPI_DSI_DMAC_INTMIS);
        }
        LOGT("[ITER %d] DMA complete, INTMIS = 0x%x", i, rd_data);

        /* Step 15n: Clear DMA interrupt */
        writel(rd_data, MIZAR_MIPI_DSI_DMAC_INTCLR);
        LOGT("[ITER %d] DMA interrupt cleared", i);

        /* Step 15o: Settling delay */
        wait_on(100000);
        LOGT("[ITER %d] Settling delay complete", i);

        g_ctx.iterations_completed++;
    }

    /* ------------------------------------------------------------------
     * Step 16: Report final status
     * ------------------------------------------------------------------ */
    // MANUAL_REVIEW: DV finish(0) converted to PSV/FV out->status reporting.
    // Original test passes unconditionally after all 10 iterations complete.
    g_ctx.checks_total  = 10;
    g_ctx.checks_passed = g_ctx.iterations_completed;
    g_ctx.checks_failed = 10U - g_ctx.iterations_completed;
    out->status = (g_ctx.iterations_completed == 10U) ? 0 : -1;

    LOGT("Run complete: %s iterations_completed=%u errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.iterations_completed,
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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: iterations_completed=%u errors=%u",
         g_ctx.iterations_completed, g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
