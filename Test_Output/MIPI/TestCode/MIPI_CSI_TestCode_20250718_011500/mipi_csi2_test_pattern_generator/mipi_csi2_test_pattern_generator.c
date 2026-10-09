// Author - AI Force 2.3. 18-Jul-2025 01:15 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator (PG)
 *              functionality. Configures virtual channel, disables control data,
 *              enables interrupts, initializes D-PHY, polls PHY_STOPSTATE,
 *              programs DMA, enables/disables pattern generator, polls DMA
 *              completion, and reports final status.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator init: test pattern generator validation");

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int hres = 320;
    int vres = 16;
    int valid_bits_per_pixel = 24;
    int csi2_data_trnsfr_size;
    int dma_dest_addr_incr_flag = 1;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    /* Step 1: Configure virtual channel register with VC_ID mapping */
    LOGT("Step 1: Configuring virtual channel register");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, 0x3);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL=0x3");

    /* Step 2: Disable control data transfer by writing 0 */
    LOGT("Step 2: Disabling control data transfer");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA=0x0");

    /* Step 3: Enable CSI-2 and DMA interrupts at subsystem level */
    LOGT("Step 3: Enabling CSI-2 and DMA interrupts via csi2_subsys_enable_interrupt()");
    csi2_subsys_enable_interrupt();

    /* Step 4: Perform D-PHY initialization sequence */
    LOGT("Step 4: Performing D-PHY initialization via snps_phy_init()");
    snps_phy_init();

    /* Step 5: Poll PHY_STOPSTATE until D-PHY enters stop state (expected 0x1000f) */
    LOGT("Step 5: Polling PHY_STOPSTATE for expected value 0x1000f");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    } while ((rd_data != 0x1000f) && (timeout > 0));

    if (timeout <= 0) {
        LOGE("PHY_STOPSTATE poll timeout: rd_data=0x%x expected=0x1000f", rd_data);
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }
    LOGT("PHY_STOPSTATE=0x%x, D-PHY entered stop state", rd_data);
    g_ctx.checks_passed++;
    g_ctx.checks_total++;

    /* Step 6: Compute total data transfer size */
    csi2_data_trnsfr_size = (hres * vres * valid_bits_per_pixel) / 8;
    LOGT("Step 6: Computed csi2_data_trnsfr_size=%d bytes (hres=%d vres=%d bpp=%d)",
         csi2_data_trnsfr_size, hres, vres, valid_bits_per_pixel);

    /* Step 7: Program DMA higher-order address registers for channel 0 */
    LOGT("Step 7: Programming DMA higher-order address registers");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA=0x0");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x0");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA=0x0");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x0");

    /* Step 8: Enable fracdiv output to CSI-2 subsystem */
    LOGT("Step 8: Enabling fracdiv output to CSI-2 subsystem");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("write (MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4)=0x1");

    /* Step 9: Program DMA channel 0 for data transfer */
    LOGT("Step 9: Programming DMA ch0: src=0x0000 dst=GDMA_CSI2_DATA_DEST_ADDR2 size=%d incr=%d",
         csi2_data_trnsfr_size, dma_dest_addr_incr_flag);
    dma_trnsfr_instn_preload_incr_addr(0, 0x0000, GDMA_CSI2_DATA_DEST_ADDR2,
                                       csi2_data_trnsfr_size, dma_dest_addr_incr_flag);

    /* Step 10: Start DMA channel 0 */
    LOGT("Step 10: Starting DMA channel 0");
    DMAGO_CSI(0);
    LOGT("DMA ch0 started via DMAGO_CSI(0)");

    /* Step 11: Enable internal test pattern generator */
    LOGT("Step 11: Enabling internal test pattern generator");

    /* Configure vertical resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, vres);
    LOGT("write MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES=0x%x", vres);

    /* Configure horizontal resolution with attribute 7 */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, (0x70000 | hres));
    LOGT("write MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES=0x%x", (0x70000 | hres));

    /* Configure PG: VC=3, data_type=0x24/RGB888, pattern_type enabled */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);
    LOGT("write MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG=0xe401");

    /* Enable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1);
    LOGT("write MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE=0x1");

    /* Step 12: Wait for pattern generation to complete */
    LOGT("Step 12: Waiting for pattern generation to complete");
    for (volatile int d0 = 0; d0 < MIPI_CSI2_WAIT_PG_COMPLETE; d0++);

    /* Step 13: Disable pattern generator by clearing enable bit */
    LOGT("Step 13: Disabling pattern generator");
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0);
    LOGT("write MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE=0x0");

    /* Step 14: Poll DMA interrupt status for channel 0 completion (bit 0) */
    LOGT("Step 14: Polling DMA INTMIS for ch0 completion (bit 0)");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        rd_data = read_reg(MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout--;
    } while (((rd_data & 0x1) == 0) && (timeout > 0));

    if (timeout <= 0) {
        LOGE("DMA ch0 poll timeout: INTMIS=0x%x", rd_data);
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }
    LOGT("DMA ch0 transfer complete: INTMIS=0x%x (%d bytes received)",
         rd_data, csi2_data_trnsfr_size);
    g_ctx.checks_passed++;
    g_ctx.checks_total++;

    /* Step 15: Wait for post-completion settling */
    LOGT("Step 15: Waiting for post-completion settling");
    for (volatile int d1 = 0; d1 < MIPI_CSI2_WAIT_SETTLE; d1++);

    /* Step 16: Complete test with pass status */
    // MANUAL_REVIEW: DV finish(0) converted to PSV/FV out->status based completion.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx.checks_failed = g_ctx.errors;

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u checks_passed=%u checks_failed=%u checks_total=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_failed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
