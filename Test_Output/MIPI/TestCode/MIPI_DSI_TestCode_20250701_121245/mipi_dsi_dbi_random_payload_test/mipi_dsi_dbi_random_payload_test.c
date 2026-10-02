// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case : mipi_dsi_dbi_random_payload_test
 * Description: MIPI DSI DBI data transfers with randomized pixel payload sizes
 *              across 10 iterations using a 2-channel DMA engine.
 *              CH0 for data transfer, CH1 for command transfer.
 *              Polling-based DMA completion (no interrupt handler).
 */

/* ---------------------------------------------------------------------------
 * Test context structure
 * --------------------------------------------------------------------------*/
typedef struct {
    unsigned int iterations_completed;
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
    unsigned int phy_stop_wait_time;
    unsigned int n_lanes;
    unsigned int phy_if_cfg;
    unsigned int num_of_pixel;
    double dpi_clk_time_period;
    double dpi_clk_freq;

    (void)cfg;

    /* Initialize test context */
    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init: starting initialization");

    /* Step 1: Set PHY parameters */
    LOGT("Step 1: Setting PHY parameters");
    phy_stop_wait_time = 0x40U;
    n_lanes            = 3U;

    /* Step 2: Compute PHY interface configuration */
    LOGT("Step 2: Computing PHY_IF_CFG n_lanes=%u phy_stop_wait_time=0x%x",
         n_lanes, phy_stop_wait_time);
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg, phy_stop_wait_time,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME);

    /* Step 3: Write PHY_IF_CFG */
    LOGT("Step 3: Writing PHY_IF_CFG = 0x%x", phy_if_cfg);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);

    /* Step 4: Configure packet handling */
    LOGT("Step 4: Writing PCKHDL_CFG = 0x3d");
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);

    /* Step 5: Configure clock manager */
    LOGT("Step 5: Writing CLKMGR_CFG = 0x107");
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);

    /* Step 6: Enable DBI mode */
    LOGT("Step 6: Writing DPI_CONTROL = 0 (DBI mode)");
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0U);

    /* Step 7: Initialize DSI PHY */
    LOGT("Step 7: Calling phy_init()");
    phy_init();

    /* Step 8: Set DBI configuration variables */
    LOGT("Step 8: Setting DBI configuration variables");
    dbi_vcid                 = 0x3U;
    load_cmd_or_data_to_sram = 1U;
    lut_size_conf            = 0x1U;
    out_dbi_conf             = 0xbU;
    in_dbi_conf              = 0x0U;
    partitioning_en          = 0x0U;
    allowed_cmd_size         = 0x25U;
    wr_cmd_size              = 193U;

    /* Step 9: Compute initial pixel-to-byte conversion */
    num_of_pixel = 40U;
    LOGT("Step 9: Computing pixel_to_bytes_wr_cmd_size for %u pixels", num_of_pixel);
    pixel_to_bytes_wr_cmd_size(num_of_pixel);

    /* Step 10: Set command type variables */
    LOGT("Step 10: Setting command type variables");
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

    /* Step 11: Program DPI clock */
    dpi_clk_time_period = 16.012400;
    dpi_clk_freq = 1000000000.0 / dpi_clk_time_period;
    LOGT("Step 11: Programming DPI clock");
    program_dpi_clock(dpi_clk_freq);

    /* Step 12: Apply initial DBI configuration */
    LOGT("Step 12: Calling dbi_config()");
    dbi_config();

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
    unsigned int num_of_pixel;
    unsigned int num_bytes;
    unsigned int num_descriptors;
    unsigned long long ch0_desc_addr;
    unsigned long long ch1_desc_addr;
    unsigned long long ch0_desc_addr_act;
    unsigned long long ch1_desc_addr_act;
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

    LOGT("mipi_dsi_dbi_random_payload_test run: starting main execution");

    /* Step 13: Set number of descriptors */
    num_descriptors = 1U;
    LOGT("Step 13: num_descriptors = %u", num_descriptors);

    /* Step 14: Enable DMA interrupts for CH0 and CH1 */
    LOGT("Step 14: Writing 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);

    /* Step 15: Main loop - 10 iterations with random pixel counts */
    for (i = 0; i < 10; i++) {

        /* Step 15a: Generate random pixel count (multiples of 8) */
        num_of_pixel = (unsigned int)((rand() % 1024)) * 8U;
        LOGT("Step 15a: Iteration %d: num_of_pixel = %u", i, num_of_pixel);

        /* Step 15b: Recompute wr_cmd_size and num_bytes */
        pixel_to_bytes_wr_cmd_size(num_of_pixel);
        wr_cmd_size = pixel_attr.wr_cmd_size;
        num_bytes   = pixel_attr.num_bytes;

        /* Step 15c: Print iteration info */
        LOGT("Step 15c: i=%d wr_cmd_size=%u num_bytes=%u", i, wr_cmd_size, num_bytes);

        /* Step 15d: Reconfigure DBI with new wr_cmd_size */
        LOGT("Step 15d: Calling dbi_config()");
        dbi_config();

        /* Step 15e: Set DMA descriptor addresses */
        ch0_desc_addr     = RAM_BASE + 0x3F000ULL;
        ch1_desc_addr     = RAM_BASE + 0x3F800ULL;
        ch0_desc_addr_act = ch0_desc_addr;
        ch1_desc_addr_act = ch1_desc_addr;

        /* Step 15f: Descriptor loop */
        for (itter = 0; itter < (int)num_descriptors; itter++) {
            LOGT("Step 15f: Descriptor iteration %d", itter);

            /* Data transmit descriptor */
            data_tdbdcb[itter].addr = RAM_BASE + 0x10000ULL + ((unsigned long long)itter * 0x1000ULL);
            data_tdbdcb[itter].len  = num_bytes;
            data_tdbdcb[itter].eop  = 1U;

            /* Data receive descriptor */
            data_rebdcb[itter].addr = 0x10000000000ULL;
            data_rebdcb[itter].len  = num_bytes;

            /* Command transmit descriptor */
            cmd_tdbdcb[itter].addr  = RAM_BASE + 0x0000ULL;
            cmd_tdbdcb[itter].len   = 4U;
            cmd_tdbdcb[itter].eop   = 1U;

            /* Command receive descriptor */
            cmd_rebdcb[itter].addr  = 0x10000008000ULL;
            cmd_rebdcb[itter].len   = 4U;

            /* 8-byte aligned data length for CH0 */
            data_len_aligned = (unsigned int)(ceil((double)data_tdbdcb[itter].len / 8.0) * 8.0);

            /* Program CH0 DMA microcode (data channel) */
            LOGT("Step 15f: Programming CH0 DMA microcode, aligned len=%u", data_len_aligned);
            DMAMOV(&ch0_desc_addr, DMA_SAR, data_tdbdcb[itter].addr);
            DMAMOV(&ch0_desc_addr, DMA_DAR, data_rebdcb[itter].addr);
            program_data_num_bytes(&ch0_desc_addr, data_len_aligned);
            DMAWMB(&ch0_desc_addr);
            DMASEV(&ch0_desc_addr, 0);
            DMAEND(&ch0_desc_addr);

            /* Program CH1 DMA microcode (command channel) */
            LOGT("Step 15f: Programming CH1 DMA microcode");
            DMAMOV(&ch1_desc_addr, DMA_SAR, cmd_tdbdcb[itter].addr);
            DMAMOV(&ch1_desc_addr, DMA_DAR, cmd_rebdcb[itter].addr);
            program_data_num_bytes(&ch1_desc_addr, cmd_tdbdcb[itter].len);
            DMAWMB(&ch1_desc_addr);
            DMASEV(&ch1_desc_addr, 1);
            DMAEND(&ch1_desc_addr);

            /* Load random data into SRAM */
            LOGT("Step 15f: Loading random data into SRAM");
            load_rand_data(data_tdbdcb[itter]);

            /* Load write command into SRAM */
            LOGT("Step 15f: Loading write command into SRAM");
            load_wr_command(cmd_tdbdcb[itter].addr, num_bytes, DSI_WRITE_MEMORY_START);
        }

        /* Start CH0 DMA transfer */
        LOGT("Step 15g: Starting CH0 DMA: DBGINST0 = 0x00A00000");
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

        /* Start CH1 DMA transfer */
        LOGT("Step 15h: Starting CH1 DMA: DBGINST0 = 0x01A00000");
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000U);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);

        /* Step 15m: Poll INTMIS until both channels complete (rd_data == 0x3) */
        LOGT("Step 15m: Polling INTMIS for both channel completion");
        do {
            rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
        } while (rd_data != 0x3U);
        LOGT("Step 15n: INTMIS = 0x%x, both channels complete", rd_data);

        /* Step 15o: Clear DMA interrupt */
        LOGT("Step 15o: Clearing DMA interrupt: INTCLR = 0x%x", rd_data);
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);

        /* Step 15p: Settling delay */
        LOGT("Step 15p: Settling delay");
        wait_on(100000);

        g_ctx.iterations_completed++;
    }

    /* Update output status - test passes unconditionally after all iterations */
    out->status = 0;

    LOGT("Run complete: PASS iterations_completed=%u",
         g_ctx.iterations_completed);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // out->status based PASS/FAIL reporting is used instead.

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

    LOGT("mipi_dsi_dbi_random_payload_test teardown: iterations_completed=%u",
         g_ctx.iterations_completed);

    /* Validation: all 10 iterations must have completed */
    if (g_ctx.iterations_completed == 10U) {
        LOGT("mipi_dsi_dbi_random_payload_test PASSED: all 10 iterations completed");
    } else {
        LOGE("mipi_dsi_dbi_random_payload_test FAILED: only %u of 10 iterations completed",
             g_ctx.iterations_completed);
        return -1;
    }

    return 0;
}
