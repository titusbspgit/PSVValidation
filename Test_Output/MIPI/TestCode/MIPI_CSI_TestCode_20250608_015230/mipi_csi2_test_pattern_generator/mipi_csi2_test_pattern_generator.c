// Author - AI Force 2.3. 08-Jun-2025 01:52 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator
 *              functionality by configuring the pattern generator with
 *              vres=16, hres=320, 24bpp, enabling/disabling it, and
 *              confirming DMA transfer of generated pattern data.
 */

typedef struct {
    unsigned int errors;
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
    unsigned int rd_data;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_reg_base;
    unsigned int int_pend;
    unsigned int vcid;
    unsigned int vcid_unselected_path;
    unsigned int hres;
    unsigned int vres;
    unsigned int valid_bits_per_pixel;
    unsigned int csi2_data_trnsfr_size;
    unsigned int raw_size;
    unsigned int gdma_path;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_test_pattern_generator execution");

    /* Step 1: Initialize local variables */
    int_pend = 1U;
    vcid = 3U;
    vcid_unselected_path = ((vcid + 1U) & 0xfU);
    LOGT("[STEP 1] int_pend=%u vcid=%u vcid_unselected_path=0x%x",
         int_pend, vcid, vcid_unselected_path);

    /* Step 2: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = vcid;
    gdma_path = 3U;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (vcid << 4);
    gdma_path = 2U;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (vcid << 8);
    gdma_path = 1U;
#elif defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (vcid << 12);
    gdma_path = 0U;
#else
    vcid_csi2_wrap_reg = vcid;
    gdma_path = 3U;
#endif
    LOGT("[STEP 2] vcid_csi2_wrap_reg=0x%08x gdma_path=%u",
         vcid_csi2_wrap_reg, gdma_path);

    /* Step 3: Set GDMA register base address */
    gdma_reg_base = 0xE6A00000U + (gdma_path * 0x1000U);
    LOGT("[STEP 3] gdma_reg_base=0x%08x", gdma_reg_base);

    /* Step 4: Configure virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("[STEP 4] WRITE : MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL = 0x%08x",
         vcid_csi2_wrap_reg);

    /* Step 5: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("[STEP 5] WRITE : MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA = 0x00000000 (disabled)");

    /* Step 6: Enable CSI-2 and DMA interrupts */
    LOGT("[STEP 6] Calling csi2_subsys_enable_interrupt()");
    csi2_subsys_enable_interrupt();

    /* Step 7: Initialize D-PHY */
    LOGT("[STEP 7] Calling snps_phy_init() for D-PHY initialization");
    snps_phy_init();

    /* Step 8: Read PHY_STOPSTATE */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("[STEP 8] READ  : MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE = 0x%08x", rd_data);

    /* Step 9: Poll PHY_STOPSTATE until all lanes in stop state (0x1000f) */
    LOGT("[STEP 9] Polling MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE for 0x1000F");
    while (!(rd_data == 0x1000fU)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("[STEP 9] PHY_STOPSTATE = 0x%08x (all lanes in stop state)", rd_data);

    /* Step 10: Set image parameters */
    hres = 320U;
    vres = 16U;
    valid_bits_per_pixel = 24U;
    LOGT("[STEP 10] Image params: hres=%u vres=%u bpp=%u",
         hres, vres, valid_bits_per_pixel);

    /* Step 11: Compute 8-byte aligned data transfer size */
    raw_size = (hres * vres * valid_bits_per_pixel) / 8U;
    csi2_data_trnsfr_size = (raw_size % 8U) ?
        ((raw_size / 8U + 1U) * 8U) : raw_size;
    LOGT("[STEP 11] raw_size=%u csi2_data_trnsfr_size(aligned)=%u",
         raw_size, csi2_data_trnsfr_size);

    /* Step 12: Program DMA channel 0 higher-order address register (AR data) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0U);
    LOGT("[STEP 12] WRITE : MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA = 0x00000000");

    /* Step 13: Program DMA channel 0 higher-order address register (AR instruction) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);
    LOGT("[STEP 13] WRITE : MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x00000000");

    /* Step 14: Program DMA channel 0 higher-order address register (AW data) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0U);
    LOGT("[STEP 14] WRITE : MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA = 0x00000000");

    /* Step 15: Program DMA channel 0 higher-order address register (AW instruction) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);
    LOGT("[STEP 15] WRITE : MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x00000000");

    /* Step 16: Enable fracdiv output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4U, 0x1U);
    LOGT("[STEP 16] WRITE : MIZAR_MIPI_CSI2_RB_REG_BASE + 0xF4 = 0x00000001 (fracdiv enable)");

    /* Step 17: Enable DMA interrupts */
    write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);
    LOGT("[STEP 17] WRITE : gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET = 0x00000003");

    /* Step 18: Program DMA transfer instructions for channel 0 */
    LOGT("[STEP 18] Calling dma_trnsfr_instn_preload(0xE6000000, gdma_reg_base, 0x0000, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size, 0)");
    dma_trnsfr_instn_preload(0xE6000000U, gdma_reg_base, 0x0000U,
                              GDMA_CSI2_DATA_DEST_ADDR2,
                              csi2_data_trnsfr_size, 0U);

    /* Step 19: Start DMA channel 0 */
    LOGT("[STEP 19] Calling DMAGO_CSI(gdma_reg_base, 0xE6000000, 0x0)");
    DMAGO_CSI(gdma_reg_base, 0xE6000000U, 0x0U);
    LOGT("[STEP 19] DMA channel 0 started");

    /* Step 20: Write PPI_PG_PATTERN_VRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, vres);
    LOGT("[STEP 20] WRITE : MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES = 0x%08x", vres);

    /* Step 21: Write PPI_PG_PATTERN_HRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, hres);
    LOGT("[STEP 21] WRITE : MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES = 0x%08x", hres);

    /* Step 22: Write PPI_PG_CONFIG */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0x2BU);
    LOGT("[STEP 22] WRITE : MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG = 0x0000002B");

    /* Step 23: Enable test pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1U);
    LOGT("[STEP 23] WRITE : MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE = 0x00000001 (enabled)");

    /* Step 24: Busy-wait replacing DV wait_on(100) */
    LOGT("[STEP 24] Busy-wait delay (replacing DV wait_on(100))");
    for (volatile int d0 = 0; d0 < 100; d0++);

    /* Step 25: Disable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0U);
    LOGT("[STEP 25] WRITE : MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE = 0x00000000 (disabled)");

    /* Step 26: Poll DMA channel 0 completion (bit 0) */
    LOGT("[STEP 26] Polling DMA interrupt status for channel 0 completion");
    rd_data = 0U;
    while ((rd_data & 0x1U) == 0x0U) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("[STEP 26] DMA CH0 transfer complete (INTMIS=0x%08x)", rd_data);

    /* Step 27: Clear DMA interrupt */
    write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);
    LOGT("[STEP 27] WRITE : gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET = 0x00000001 (cleared)");

    /* Step 28: Busy-wait replacing DV wait_on(10000) for settling */
    LOGT("[STEP 28] Busy-wait delay (replacing DV wait_on(10000))");
    for (volatile int d1 = 0; d1 < 10000; d1++);

    /* Step 29: Test completion */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

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

    LOGT("mipi_csi2_test_pattern_generator teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
