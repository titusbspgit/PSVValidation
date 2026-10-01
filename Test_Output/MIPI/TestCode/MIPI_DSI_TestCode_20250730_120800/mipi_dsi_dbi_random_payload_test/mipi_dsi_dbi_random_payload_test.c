// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_dbi_random_payload_test.h"
#include "test_define.inc"

/*
 * mipi_dsi_dbi_random_payload_test
 * This testcase performs MIPI DSI DBI data transfers with randomized pixel
 * payload sizes using the integrated DMA controller (DMA330). It uses
 * polling-based completion across 10 iterations with varying payload sizes.
 * No ISR is used; the INTMIS register is polled directly.
 */

/* ---------------------------------------------------------------------------
 * Test context
 * --------------------------------------------------------------------------- */
typedef struct {
    unsigned int errors;
    unsigned int iterations_completed;
} mipi_dsi_dbi_random_payload_test_ctx_t;

static mipi_dsi_dbi_random_payload_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_dbi_random_payload_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_dsi_dbi_random_payload_test including PHY configuration,
 *              DBI parameter setup, and DMA interrupt enable.
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

    (void)cfg;

    g_ctx = (mipi_dsi_dbi_random_payload_test_ctx_t){0};

    LOGT("mipi_dsi_dbi_random_payload_test init start");

    /* Step 1: Configure PHY parameters */
    phy_stop_wait_time = 0x40U;
    n_lanes = 3U;
    LOGT("phy_stop_wait_time=0x%x n_lanes=%u",
         (unsigned int)phy_stop_wait_time, (unsigned int)n_lanes);

    /* Step 2: Configure PHY interface register */
    phy_if_cfg = n_lanes;
    phy_if_cfg = set_data_mask(phy_if_cfg,
                               MIPI_DSI_HOST_PHY_IF_CFG_PHY_STOP_WAIT_TIME,
                               phy_stop_wait_time);
    writel_reg(MIZAR_MIPI_DSI_HOST_PHY_IF_CFG, phy_if_cfg);
    LOGT("PHY_IF_CFG=0x%x written", (unsigned int)phy_if_cfg);

    /* Step 3: Configure packet handling */
    writel_reg(MIZAR_MIPI_DSI_HOST_PCKHDL_CFG, 0x3dU);
    LOGT("PCKHDL_CFG=0x3d written");

    /* Step 4: Configure clock manager */
    writel_reg(MIZAR_MIPI_DSI_HOST_CLKMGR_CFG, 0x107U);
    LOGT("CLKMGR_CFG=0x107 written");

    /* Step 5: Disable DPI control to enable DBI mode */
    writel_reg(MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL, 0U);
    LOGT("DPI_CONTROL=0x0 written DBI mode enabled");

    /* Step 6: Initialize DSI PHY */
    phy_init();
    LOGT("phy_init() called");

    /* Step 7: Configure DBI parameters */
    dbi_vcid = 0x3U;
    load_cmd_or_data_to_sram = 1U;
    lut_size_conf = 0x1U;
    out_dbi_conf = 0xbU;
    in_dbi_conf = 0x0U;
    partitioning_en = 0x0U;
    allowed_cmd_size = 0x25U;
    LOGT("DBI params: dbi_vcid=0x%x lut_size_conf=0x%x out_dbi_conf=0x%x",
         (unsigned int)dbi_vcid, (unsigned int)lut_size_conf,
         (unsigned int)out_dbi_conf);
    LOGT("DBI params: partitioning_en=0x%x allowed_cmd_size=0x%x",
         (unsigned int)partitioning_en, (unsigned int)allowed_cmd_size);

    /* Step 8: Compute initial write command size and byte count */
    pixel_to_bytes_wr_cmd_size(40U);
    LOGT("pixel_to_bytes_wr_cmd_size(40) called");

    /* Step 9: Set DSI command type flags */
    tear_fx_en = 0x1U;
    generic_vc_id = 0x2U;
    LOGT("tear_fx_en=0x%x generic_vc_id=0x%x",
         (unsigned int)tear_fx_en, (unsigned int)generic_vc_id);

    /* Step 10: Program DPI clock */
    program_dpi_clock();
    LOGT("program_dpi_clock() called");

    /* Step 11: Apply initial DBI configuration */
    dbi_config();
    LOGT("dbi_config() called");

    /* Step 12: Enable DMA interrupts for CH0 and CH1 */
    writel_reg(MIZAR_MIPI_DSI_DMAC_INTEN, 0x3U);
    LOGT("DMAC_INTEN=0x3 written");

    LOGT("mipi_dsi_dbi_random_payload_test init complete");
    return 0;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_run
 * Description: Executes the main testcase flow for mipi_dsi_dbi_random_payload_test.
 *              Runs 10 iterations with randomized pixel payload sizes using
 *              polling-based DMA completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_dbi_random_payload_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int ch0_desc_addr_act;
    unsigned int ch1_desc_addr_act;
    unsigned int rd_data;
    unsigned int data_len_aligned;
    unsigned int timeout;
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_dbi_random_payload_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_dbi_random_payload_test run start iterations=%u",
         (unsigned int)MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_ITERATIONS);

    /* Steps 13-25: Iterate 10 times with randomized pixel payload sizes */
    for (i = 0; i < (int)MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_ITERATIONS; i++) {

        LOGT("--- iteration %d/%u start ---", i + 1,
             (unsigned int)MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_ITERATIONS);

        /* Step 13: Generate random pixel count (multiples of 8, up to 8192) */
        num_of_pixel = ((unsigned int)(rand() % 1024)) * 8U;
        LOGT("iter %d random num_of_pixel=%u", i + 1,
             (unsigned int)num_of_pixel);

        /* Step 14: Recompute write command size and byte count */
        pixel_to_bytes_wr_cmd_size(num_of_pixel);
        LOGT("iter %d pixel_to_bytes_wr_cmd_size(%u) called wr_cmd_size=%u num_bytes=%u",
             i + 1, (unsigned int)num_of_pixel,
             (unsigned int)wr_cmd_size, (unsigned int)num_bytes);

        /* Step 15: Reapply DBI configuration with updated payload size */
        dbi_config();
        LOGT("iter %d dbi_config() reapplied", i + 1);

        /* Step 16: Set up DMA descriptors for CH0 (data) */
        data_tdbdcb[i] = RAM_BASE + 0x10000UL;
        data_rebdcb[i] = 0x10000000000ULL;
        LOGT("iter %d CH0 data_tdbdcb=0x%lx data_rebdcb=0x%llx",
             i + 1, (unsigned long)data_tdbdcb[i],
             (unsigned long long)data_rebdcb[i]);

        /* Step 17: Set up DMA descriptors for CH1 (command) */
        cmd_tdbdcb[i] = RAM_BASE + 0x0000UL;
        cmd_rebdcb[i] = 0x10000008000ULL;
        LOGT("iter %d CH1 cmd_tdbdcb=0x%lx cmd_rebdcb=0x%llx",
             i + 1, (unsigned long)cmd_tdbdcb[i],
             (unsigned long long)cmd_rebdcb[i]);

        /* Step 18: Program DMA microcode for CH0 (data) */
        /* CH0 data transfer length is 8-byte aligned: ceil(num_bytes/8)*8 */
        data_len_aligned = (unsigned int)(ceil((double)num_bytes / 8.0) * 8);
        program_dma_microcode(0, &data_tdbdcb[i], &data_rebdcb[i], data_len_aligned);
        LOGT("iter %d DMA microcode CH0 data_len_aligned=%u",
             i + 1, (unsigned int)data_len_aligned);

        /* Step 19: Program DMA microcode for CH1 (command) */
        program_dma_microcode(1, &cmd_tdbdcb[i], &cmd_rebdcb[i], wr_cmd_size);
        LOGT("iter %d DMA microcode CH1 wr_cmd_size=%u",
             i + 1, (unsigned int)wr_cmd_size);

        /* Step 20: Load random pixel data */
        load_rand_data(data_tdbdcb[i], data_len_aligned);
        LOGT("iter %d load_rand_data called len=%u",
             i + 1, (unsigned int)data_len_aligned);

        /* Step 21: Load write command */
        load_wr_command(cmd_tdbdcb[i], DSI_WRITE_MEMORY_START);
        LOGT("iter %d load_wr_command called with DSI_WRITE_MEMORY_START", i + 1);

        /* Step 22: Start DMA CH0 */
        ch0_desc_addr_act = (unsigned int)data_tdbdcb[i];
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x00A00000U);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch0_desc_addr_act);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
        LOGT("iter %d DMA CH0 started DBGINST0=0x00A00000 DBGINST1=0x%x",
             i + 1, (unsigned int)ch0_desc_addr_act);

        /* Step 23: Start DMA CH1 */
        ch1_desc_addr_act = (unsigned int)cmd_tdbdcb[i];
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST0, 0x01A00000U);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGINST1, ch1_desc_addr_act);
        writel_reg(MIZAR_MIPI_DSI_DMAC_DBGCMD, 0x0U);
        LOGT("iter %d DMA CH1 started DBGINST0=0x01A00000 DBGINST1=0x%x",
             i + 1, (unsigned int)ch1_desc_addr_act);

        /* Step 24: Poll INTMIS until both channels complete (rd_data == 0x3) */
        LOGT("iter %d polling INTMIS for both channel completion", i + 1);
        timeout = MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_TIMEOUT;
        do {
            rd_data = readl_reg(MIZAR_MIPI_DSI_DMAC_INTMIS);
            timeout--;
        } while ((rd_data != 0x3U) && (timeout > 0U));

        if (timeout == 0U) {
            LOGE("iter %d timeout polling INTMIS rd_data=0x%x expected=0x3",
                 i + 1, (unsigned int)rd_data);
            g_ctx.errors++;
            break;
        }

        LOGT("iter %d INTMIS=0x%x both channels completed",
             i + 1, (unsigned int)rd_data);

        /* Step 25: Clear DMA interrupts */
        writel_reg(MIZAR_MIPI_DSI_DMAC_INTCLR, rd_data);
        LOGT("iter %d DMAC_INTCLR=0x%x written",
             i + 1, (unsigned int)rd_data);

        /* Step 26: Settling delay before next iteration */
        wait_on(100000U);
        LOGT("iter %d settling delay complete", i + 1);

        g_ctx.iterations_completed++;
        LOGT("--- iteration %d/%u complete ---", i + 1,
             (unsigned int)MIPI_DSI_DBI_RANDOM_PAYLOAD_TEST_ITERATIONS);
    }

    /* Report result */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u iterations_completed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors, g_ctx.iterations_completed);

    return out->status;
}

/*
 * Function: mipi_dsi_dbi_random_payload_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for mipi_dsi_dbi_random_payload_test.
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

    // MANUAL_REVIEW: DV finish(0) was present in the source flow with hardcoded 0.
    // PSV/FV-native status is reported through out->status in the run function.

    return g_ctx.errors == 0U ? 0 : -1;
}
