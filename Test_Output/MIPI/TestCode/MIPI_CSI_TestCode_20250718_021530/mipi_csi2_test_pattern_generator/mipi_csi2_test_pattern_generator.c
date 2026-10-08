// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case    : mipi_csi2_test_pattern_generator
 * Description  : Validates the MIPI CSI-2 host controller's internal test
 *                pattern generator by configuring the virtual channel, enabling
 *                CSI-2 and DMA interrupts, initializing the D-PHY, waiting for
 *                PHY stop state, programming DMA channel 0 for CSI-2 data
 *                capture with computed transfer size based on resolution
 *                (320x16, 24bpp), enabling the pattern generator, capturing
 *                data via DMA, then disabling the pattern generator and polling
 *                for DMA completion.
 */

typedef struct {
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    int gdma_path;
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: csi2_subsys_enable_interrupt
 * Description: Enable all CSI-2 host interrupt masks by reading INT_ST_MAIN
 *              to clear pending interrupts, then writing 0xFFFFFFFF to all
 *              10 interrupt mask registers.
 */
static void csi2_subsys_enable_interrupt(void)
{
    int rd_data;

    /* Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("Read INT_ST_MAIN to clear pending interrupts: 0x%x", rd_data);

    /* Enable PHY Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PHY_FATAL: 0xFFFFFFFF");

    /* Enable Packet Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PKT_FATAL: 0xFFFFFFFF");

    /* Enable PHY interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PHY: 0xFFFFFFFF");

    /* Enable Line interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0xFFFFFFFF);
    LOGT("Write INT_MSK_LINE: 0xFFFFFFFF");

    /* Enable Boundary Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_BNDRY_FRAME_FATAL: 0xFFFFFFFF");

    /* Enable Sequence Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_SEQ_FRAME_FATAL: 0xFFFFFFFF");

    /* Enable CRC Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_CRC_FRAME_FATAL: 0xFFFFFFFF");

    /* Enable Payload CRC Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PLD_CRC_FATAL: 0xFFFFFFFF");

    /* Enable Data ID interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0xFFFFFFFF);
    LOGT("Write INT_MSK_DATA_ID: 0xFFFFFFFF");

    /* Enable ECC Corrected interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0xFFFFFFFF);
    LOGT("Write INT_MSK_ECC_CORRECTED: 0xFFFFFFFF");
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Enable the internal test pattern generator with specified
 *              vertical resolution, horizontal resolution, and configuration.
 */
static void csi2_ctrlr_pg_enable(void)
{
    /* Configure Pattern Generator Vertical Resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, PG_PATTERN_VRES_VAL);
    LOGT("Write PPI_PG_PATTERN_VRES: 0x%x", PG_PATTERN_VRES_VAL);

    /* Configure Pattern Generator Horizontal Resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, PG_PATTERN_HRES_VAL);
    LOGT("Write PPI_PG_PATTERN_HRES: 0x%x", PG_PATTERN_HRES_VAL);

    /* Configure Pattern Generator Configuration */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, PG_CONFIG_VAL);
    LOGT("Write PPI_PG_CONFIG: 0x%x", PG_CONFIG_VAL);

    /* Enable Pattern Generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_ENABLE_VAL);
    LOGT("Write PPI_PG_ENABLE: %d (enabled)", PG_ENABLE_VAL);
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    int vcid;
    int vcid_unselected_path;

    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    /* Step 4: Initialize virtual channel ID */
    vcid = 3;
    vcid_unselected_path = ((vcid + 1) & 0xf);

    LOGT("vcid: %d, vcid_unselected_path: %d", vcid, vcid_unselected_path);

    /* Step 5: GDMA Path Selection (Conditional Compilation) */
#if defined(GDMA0_PATH)
    g_ctx.gdma_path = 0;
    g_ctx.vcid_csi2_wrap_reg = (vcid & 0xf) | ((vcid_unselected_path & 0xf) << 4) |
                               ((vcid_unselected_path & 0xf) << 8) | ((vcid_unselected_path & 0xf) << 12);
#elif defined(GDMA1_PATH)
    g_ctx.gdma_path = 1;
    g_ctx.vcid_csi2_wrap_reg = ((vcid_unselected_path & 0xf)) | ((vcid & 0xf) << 4) |
                               ((vcid_unselected_path & 0xf) << 8) | ((vcid_unselected_path & 0xf) << 12);
#elif defined(GDMA2_PATH)
    g_ctx.gdma_path = 2;
    g_ctx.vcid_csi2_wrap_reg = ((vcid_unselected_path & 0xf)) | ((vcid_unselected_path & 0xf) << 4) |
                               ((vcid & 0xf) << 8) | ((vcid_unselected_path & 0xf) << 12);
#elif defined(GDMA3_PATH)
    g_ctx.gdma_path = 3;
    g_ctx.vcid_csi2_wrap_reg = ((vcid_unselected_path & 0xf)) | ((vcid_unselected_path & 0xf) << 4) |
                               ((vcid_unselected_path & 0xf) << 8) | ((vcid & 0xf) << 12);
#else
    g_ctx.gdma_path = 0;
    g_ctx.vcid_csi2_wrap_reg = (vcid & 0xf) | ((vcid_unselected_path & 0xf) << 4) |
                               ((vcid_unselected_path & 0xf) << 8) | ((vcid_unselected_path & 0xf) << 12);
#endif

    /* Step 6: Calculate GDMA register base address */
    g_ctx.gdma_reg_base = 0xE6A00000 + ((unsigned int)(g_ctx.gdma_path) * 0x1000);

    LOGT("mipi_csi2_test_pattern_generator init: gdma_path=%d gdma_reg_base=0x%08x vcid_csi2_wrap_reg=0x%08x",
         g_ctx.gdma_path, g_ctx.gdma_reg_base, g_ctx.vcid_csi2_wrap_reg);

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for
 *              mipi_csi2_test_pattern_generator. Configures virtual channel,
 *              enables CSI-2 and DMA interrupts, initializes D-PHY, polls PHY
 *              stop state, programs DMA channel 0 for CSI-2 data capture,
 *              enables the pattern generator, captures data via DMA, disables
 *              the pattern generator, and polls for DMA completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    int hres;
    int vres;
    int valid_bits_per_pixel;
    long long int csi2_data_trnsfr_size;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_test_pattern_generator run");

    /* Step 3: Print start line */
    LOGT("start line");

    /* Step 7: Configure virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL: 0x%08x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 8: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0);
    LOGT("Write CONTROL_DATA: 0x0 (disabled)");

    /* Step 9: Enable CSI-2 subsystem and DMA interrupts */
    LOGT("=== Enabling CSI-2 Subsystem and DMA Interrupts ===");
    csi2_subsys_enable_interrupt();

    /* Step 10: Initialize the SNPS D-PHY */
    LOGT("=== Initializing SNPS D-PHY ===");
    snps_phy_init();
    LOGT("SNPS D-PHY initialization complete");

    /* Step 11: Poll PHY_STOPSTATE until D-PHY enters stop state */
    LOGT("Polling PHY_STOPSTATE for stop state (expected: 0x%x)...", PHY_STOPSTATE_EXPECTED);
    timeout = MIPI_CSI2_TIMEOUT_COUNT;
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout <= 0) {
            LOGE("PHY_STOPSTATE polling timeout: rd_data=0x%x expected=0x%x", rd_data, PHY_STOPSTATE_EXPECTED);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    } while (rd_data != PHY_STOPSTATE_EXPECTED);
    LOGT("PHY_STOPSTATE achieved: 0x%x", rd_data);
    g_ctx.checks_total++;
    g_ctx.checks_passed++;

    /* Step 12: Set resolution parameters */
    hres = 320;
    vres = 16;
    valid_bits_per_pixel = 24;
    LOGT("Resolution: hres=%d, vres=%d, bpp=%d", hres, vres, valid_bits_per_pixel);

    /* Step 13: Calculate total CSI-2 data transfer size with 8-byte alignment */
    csi2_data_trnsfr_size = (long long int)hres * vres * (valid_bits_per_pixel / 8);
    if (csi2_data_trnsfr_size % 8) {
        csi2_data_trnsfr_size = ((csi2_data_trnsfr_size / 8) + 1) * 8;
    }
    LOGT("CSI-2 data transfer size (8-byte aligned): %lld bytes", csi2_data_trnsfr_size);

    /* Step 14: Program DMA higher-order address registers for channel 0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0);
    LOGT("Write DMA_M0_ADDR_AR_CH0_DATA: 0x0");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("Write DMA_M0_ADDR_AR_CH0_INSTRUCTION: 0x0");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("Write DMA_M0_ADDR_AW_CH0_DATA: 0x0");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("Write DMA_M0_ADDR_AW_CH0_INSTRUCTION: 0x0");

    /* Step 15: Enable fractional divider output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + FRACDIV_OUTPUT_OFFSET, 0x1);
    LOGT("Fractional divider output enabled");

    /* Step 16: Set DMA channel 0 program counter */
    dma_trnsfr_instn_preload(0, GDMA_CTRL_DATA_DEST_ADDR2);
    LOGT("DMA CH0 program counter set: 0x%08x", GDMA_CTRL_DATA_DEST_ADDR2);

    /* Step 17: Start DMA channel 0 transfer */
    DMAGO_CSI(0, 0x0, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size);
    LOGT("DMA CH0 started: dest=0x%08x transfer_size=%lld bytes", GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size);

    /* Step 18: Enable internal test pattern generator */
    LOGT("=== Enabling Pattern Generator ===");
    csi2_ctrlr_pg_enable();

    /* Step 19: Wait for pattern generator to produce data (100 cycles) */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("Wait 100 cycles for pattern generator data production");

    /* Step 20: Disable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_DISABLE_VAL);
    LOGT("Write PPI_PG_ENABLE: %d (disabled)", PG_DISABLE_VAL);

    /* Step 21: Poll DMA interrupt status for channel 0 completion */
    LOGT("Polling DMA interrupt status for CH0 completion...");
    timeout = MIPI_CSI2_TIMEOUT_COUNT;
    do {
        rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout--;
        if (timeout <= 0) {
            LOGE("DMA CH0 polling timeout: intmis=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    } while ((rd_data & 0x1) == 0);
    LOGT("DMA CH0 transfer complete (intmis: 0x%x)", rd_data);
    g_ctx.checks_total++;
    g_ctx.checks_passed++;

    /* Step 22: Wait for post-transfer settling time (10000 cycles) */
    for (volatile int d1 = 0; d1 < 10000; d1++);
    LOGT("Post-transfer settling wait complete (10000 cycles)");

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native completion via out->status is used instead.

    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u checks_total=%u checks_passed=%u checks_failed=%u",
         g_ctx.errors, g_ctx.checks_total, g_ctx.checks_passed, g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
