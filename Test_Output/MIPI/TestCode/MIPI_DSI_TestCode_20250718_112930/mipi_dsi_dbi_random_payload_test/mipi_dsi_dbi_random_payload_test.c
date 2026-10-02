// Author - AI Force 2.3. 18-Jul-2025 05:59 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * Test Case  : mipi_dsi_dbi_random_payload_test
 * Feature    : DBI Command Mode Random Payload DMA Write
 * Description: This testcase performs a MIPI DSI DBI random payload stress test
 *              using DMA. It configures the DSI host PHY with 4 lanes, sets packet
 *              handling and clock manager configurations, enables DBI command mode,
 *              initializes the PHY, and configures DBI parameters. The test runs 10
 *              iterations, each with a randomly generated pixel count (multiples of
 *              8, up to 8192). In each iteration, DBI configuration is updated, DMA
 *              microcode is rebuilt, and DMA completion is verified by polling INTMIS.
 *              The test passes after all 10 iterations complete successfully.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int iterations_completed;
} mipi_dsi_dbi_random_payload_test_ctx_t;

static mipi_dsi_dbi_random_payload_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_dbi_random_payload_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_dsi_dbi_random_payload_test. Configures PHY interface, packet
 *              handling, clock manager, DBI mode, PHY initialization, DBI parameters,
 *              DPI clock, and enables DMAC interrupts.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_init(const TestsItem *cfg)
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
    g_ctx.iterations_completed = 0U;

    LOGT("mipi_dsi_dbi_random_payload_test: init start");

    /* Step 1: Initialize phy_stop_wait_time=0x40, n_lanes=3 */
    LOGT("Initialized phy_stop_wait_time=0x%x, n_lanes=%u", phy_stop_wait_time, n_lanes);

    /* Step 2: Compute phy_if_cfg */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                              MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                              phy_stop_wait_time);
    LOGT("Computed phy_if_cfg: 0x%x", phy_if_cfg);

    /* Step 3: Write phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("Wrote phy_if_cfg to MIZAR_MIPI_DSI_HOST_PHY_IF_CFG");

    /* Step 4: Write 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3d);
    LOGT("Wrote 0x3d to MIZAR_MIPI_DSI_HOST_PCKHDL_CFG");

    /* Step 5: Write 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107);
    LOGT("Wrote 0x107 to MIZAR_MIPI_DSI_HOST_CLKMGR_CFG");

    /* Step 6: Write 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL to enable DBI mode */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0);
    LOGT("Wrote 0 to MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL (DBI mode enabled)");

    /* Step 7: Initialize DSI PHY */
    phy_init();
    LOGT("Called phy_init()");

    /* Step 8: Set DBI configuration variables */
    dbi_vcid             = 0x3;
    load_cmd_or_data_to_sram = 1;
    lut_size_conf        = 0x1;
    out_dbi_conf         = 0xb;
    in_dbi_conf          = 0x0;
    partitioning_en      = 0x0;
    allowed_cmd_size     = 0x25;
    LOGT("Set DBI config: dbi_vcid=0x3, lut_size_conf=0x1, out_dbi_conf=0xb, in_dbi_conf=0x0, partitioning_en=0x0, allowed_cmd_size=0x25");

    /* Step 9: Set initial wr_cmd_size=193 (192+1) */
    wr_cmd_size = 193;
    LOGT("Set initial wr_cmd_size=%u", wr_cmd_size);

    /* Step 10: Compute initial pixel-to-byte conversion for 40 pixels */
    pixel_to_bytes_wr_cmd_size(num_of_pixel);
    wr_cmd_size = pixel_attr.wr_cmd_size;
    LOGT("Called pixel_to_bytes_wr_cmd_size(%u), wr_cmd_size=%u, num_bytes=%u",
         num_of_pixel, pixel_attr.wr_cmd_size, pixel_attr.num_bytes);

    /* Step 11: Set DBI command mode parameters */
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

    /* Step 12: Compute dpi_clk_freq and program DPI clock */
    dpi_clk_freq = (unsigned int)(1000000000.0 / dpi_clk_time_period);
    program_dpi_clock(dpi_clk_freq);
    LOGT("Programmed DPI clock: %u Hz", dpi_clk_freq);

    /* Step 13: Apply initial DBI configuration */
    dbi_config();
    LOGT("Called dbi_config() for initial configuration");

    /* Step 14: Set num_descriptors=1 */
    LOGT("num_descriptors=1");

    /* Step 15: Write 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN */
    writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTEN, 0x3);
    LOGT("Wrote 0x3 to MIZAR_MIPI_DSI_DMAC_INTEN");

    LOGT("mipi_dsi_dbi_random_payload_test: init complete");
    return 0;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_run
 * Description: Executes the main testcase flow for mipi_dsi_dbi_random_payload_test.
 *              Runs 10 iterations with random payload sizes. In each iteration,
 *              reconfigures DBI, builds DMA microcode, starts DMA channels, and
 *              polls INTMIS for completion.
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
    unsigned int num_descriptors = 1;
    unsigned long long ch0_desc_addr;
    unsigned long long ch1_desc_addr;
    unsigned long long ch0_desc_addr_act;
    unsigned long long ch1_desc_addr_act;
    unsigned int rd_data;
    unsigned int aligned_len;
    unsigned int timeout;
    int i;
    int itter;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test: run start");

    /* Step 16-40: Iteration loop - 10 iterations with random payload sizes */
    for (i = 0; i < 10; i++) {

        /* Step 17: Generate random pixel count (multiples of 8, up to 8192) */
        num_of_pixel = ((unsigned int)(rand() % 1024)) * 8;
        LOGT("Iteration %d: num_of_pixel=%u", i, num_of_pixel);

        /* Step 18: Recompute pixel-to-byte conversion */
        pixel_to_bytes_wr_cmd_size(num_of_pixel);
        wr_cmd_size = pixel_attr.wr_cmd_size;
        num_bytes   = pixel_attr.num_bytes;

        /* Step 19: Print iteration info */
        LOGT("Iteration %d: wr_cmd_size=%u, num_bytes=%u", i, wr_cmd_size, num_bytes);

        /* Step 20: Re-apply DBI configuration with updated wr_cmd_size */
        dbi_config();
        LOGT("Iteration %d: Called dbi_config()", i);

        /* Step 21: Set descriptor addresses */
        ch0_desc_addr     = RAM_BASE + 0x3F000;
        ch1_desc_addr     = RAM_BASE + 0x3F800;
        ch0_desc_addr_act = ch0_desc_addr;
        ch1_desc_addr_act = ch1_desc_addr;

        /* Steps 22-27: Build DMA descriptors and microcode */
        for (itter = 0; itter < (int)num_descriptors; itter++) {

            /* Step 22: Data channel (CH0) transmit descriptor */
            data_tdbdcb[itter].addr = RAM_BASE + 0x10000;
            data_tdbdcb[itter].len  = num_bytes;
            data_tdbdcb[itter].eop  = 1;

            /* Step 23: Data channel (CH0) receive descriptor */
            data_rebdcb[itter].addr = 0x10000000000ULL;
            data_rebdcb[itter].len  = num_bytes;

            /* Step 24: Command channel (CH1) transmit descriptor */
            cmd_tdbdcb[itter].addr = RAM_BASE + 0x0000;
            cmd_tdbdcb[itter].len  = 4;
            cmd_tdbdcb[itter].eop  = 1;

            /* Step 25: Command channel (CH1) receive descriptor */
            cmd_rebdcb[itter].addr = 0x10000008000ULL;
            cmd_rebdcb[itter].len  = 4;

            /* Step 26: Build CH0 DMA microcode with 8-byte aligned length */
            aligned_len = (unsigned int)(ceil((double)data_tdbdcb[itter].len / 8.0)) * 8;
            LOGT("Iteration %d: Building CH0 DMA microcode (aligned_len=%u)", i, aligned_len);
            DMAMOV(ch0_desc_addr_act, SAR, data_tdbdcb[itter].addr);
            DMAMOV(ch0_desc_addr_act, DAR, data_rebdcb[itter].addr);
            program_data_num_bytes(ch0_desc_addr_act, aligned_len);
            DMAWMB(ch0_desc_addr_act);
            DMASEV(ch0_desc_addr_act, 0);
            DMAEND(ch0_desc_addr_act);

            /* Step 27: Build CH1 DMA microcode */
            LOGT("Iteration %d: Building CH1 DMA microcode", i);
            DMAMOV(ch1_desc_addr_act, SAR, cmd_tdbdcb[itter].addr);
            DMAMOV(ch1_desc_addr_act, DAR, cmd_rebdcb[itter].addr);
            program_data_num_bytes(ch1_desc_addr_act, cmd_tdbdcb[itter].len);
            DMAWMB(ch1_desc_addr_act);
            DMASEV(ch1_desc_addr_act, 1);
            DMAEND(ch1_desc_addr_act);
        }

        /* Step 28: Load random pixel data into SRAM */
        load_rand_data(data_tdbdcb[0]);
        LOGT("Iteration %d: Loaded random pixel data", i);

        /* Step 29: Load write_memory_start command into SRAM */
        load_wr_command(cmd_tdbdcb[0].addr, num_bytes, DSI_WRITE_MEMORY_START);
        LOGT("Iteration %d: Loaded write_memory_start command", i);

        /* Step 30: Write 0x00A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for CH0 */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000);
        LOGT("Iteration %d: Wrote 0x00A00000 to DBGINST0 (CH0 DMAGO)", i);

        /* Step 31: Write ch0_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch0_desc_addr_act);
        LOGT("Iteration %d: Wrote CH0 descriptor address to DBGINST1", i);

        /* Step 32: Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH0 DMAGO */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
        LOGT("Iteration %d: Wrote 0x0 to DBGCMD (CH0 execute)", i);

        /* Step 33: Write 0x01A00000 to MIZAR_MIPI_DSI_DMAC_DBGINST0 for CH1 */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000);
        LOGT("Iteration %d: Wrote 0x01A00000 to DBGINST0 (CH1 DMAGO)", i);

        /* Step 34: Write ch1_desc_addr_act to MIZAR_MIPI_DSI_DMAC_DBGINST1 */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGINST1, (unsigned int)ch1_desc_addr_act);
        LOGT("Iteration %d: Wrote CH1 descriptor address to DBGINST1", i);

        /* Step 35: Write 0x0 to MIZAR_MIPI_DSI_DMAC_DBGCMD to execute CH1 DMAGO */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0);
        LOGT("Iteration %d: Wrote 0x0 to DBGCMD (CH1 execute)", i);

        /* Step 36: Read MIZAR_MIPI_DSI_DMAC_INTMIS */
        rd_data = readl_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTMIS);

        /* Step 37: Poll INTMIS until both channels complete (rd_data == 0x3) with timeout */
        LOGT("Iteration %d: Polling INTMIS for DMA completion", i);
        timeout = MIPI_DSI_POLL_TIMEOUT;
        while ((rd_data != 0x3) && (timeout > 0U)) {
            rd_data = readl_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTMIS);
            timeout--;
        }

        if (timeout == 0U) {
            LOGE("Iteration %d: Timeout polling INTMIS, rd_data=0x%x", i, rd_data);
            g_ctx.errors++;
        } else {
            LOGT("Iteration %d: Both DMA channels completed (INTMIS=0x%x)", i, rd_data);
        }

        /* Step 38: Clear DMAC interrupts */
        writel_reg((uintptr_t)MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);
        LOGT("Iteration %d: Cleared DMAC interrupts (wrote 0x%x to INTCLR)", i, rd_data);

        /* Step 39: Settling delay */
        wait_on(100000);
        LOGT("Iteration %d: Settling delay complete", i);

        /* Step 40: Track completed iteration */
        g_ctx.iterations_completed++;
    }

    /* Step 41: Report final status */
    // MANUAL_REVIEW: DV finish(0) converted to PSV/FV out->status pattern.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_dbi_random_payload_test run complete: %s errors=%u iterations=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors, g_ctx.iterations_completed);

    return out->status;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              mipi_dsi_dbi_random_payload_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_dbi_random_payload_test teardown: errors=%u iterations_completed=%u",
         g_ctx.errors, g_ctx.iterations_completed);

    /* Validation: All 10 iterations must complete without polling timeout */
    if (g_ctx.iterations_completed != 10U) {
        LOGE("mipi_dsi_dbi_random_payload_test: FAIL - only %u of 10 iterations completed",
             g_ctx.iterations_completed);
    }

    if (g_ctx.errors != 0U) {
        LOGE("mipi_dsi_dbi_random_payload_test: FAIL - %u errors detected", g_ctx.errors);
    } else {
        LOGT("mipi_dsi_dbi_random_payload_test: PASS - all 10 iterations completed successfully");
    }

    LOGT("mipi_dsi_dbi_random_payload_test teardown: complete");
    return g_ctx.errors == 0U ? 0 : -1;
}
