// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * mipi_csi2_test_pattern_generator
 * Validates the MIPI CSI-2 internal test pattern generator functionality.
 * Configures virtual channel, disables control data, enables interrupts,
 * initializes D-PHY, computes frame transfer size, programs DMA address
 * registers, enables fracdiv output, preloads and starts DMA, enables
 * and disables pattern generator, polls DMA completion.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
} mipi_csi2_tpg_test_ctx_t;

static mipi_csi2_tpg_test_ctx_t g_ctx;

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

    g_ctx = (mipi_csi2_tpg_test_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator init: starting test pattern generator test");

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
    unsigned int int_pend;
    unsigned int vcid;
    unsigned int vcid_unselected_path;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_path;
    unsigned int gdma_reg_base;
    unsigned int hres;
    unsigned int vres;
    unsigned int valid_bits_per_pixel;
    unsigned int csi2_data_trnsfr_size;
    unsigned int line_bytes;
    unsigned int ch0_pc;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator run: begin");

    /* Step 1: Set int_pend */
    int_pend = 1;
    (void)int_pend;

    /* Step 2: Set vcid = 3 */
    vcid = 3;

    /* Step 3: Compute vcid_unselected_path */
    vcid_unselected_path = ((vcid + 1) & 0xf);

    /* Step 4: Configure vcid_csi2_wrap_reg and gdma_path based on compile-time GDMA path define */
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 12) | (vcid_unselected_path << 8) | (vcid_unselected_path << 4) | vcid;
    gdma_path = 3;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 12) | (vcid_unselected_path << 8) | (vcid << 4) | vcid_unselected_path;
    gdma_path = 2;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 12) | (vcid << 8) | (vcid_unselected_path << 4) | vcid_unselected_path;
    gdma_path = 1;
#else /* GDMA0_PATH */
    vcid_csi2_wrap_reg = (vcid << 12) | (vcid_unselected_path << 8) | (vcid_unselected_path << 4) | vcid_unselected_path;
    gdma_path = 0;
#endif
    LOGT("CSI2 TPG: GDMA path=%u, vcid_csi2_wrap_reg=0x%x", gdma_path, vcid_csi2_wrap_reg);

    /* Step 5: Compute gdma_reg_base */
    gdma_reg_base = 0xE6A00000 + (gdma_path * 0x1000);
    LOGT("CSI2 TPG: gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 6: Configure virtual channel */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("CSI2 TPG: Virtual channel configured with 0x%x", vcid_csi2_wrap_reg);

    /* Step 7: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);
    LOGT("CSI2 TPG: Control data transfer disabled");

    /* Step 8: Call csi2_subsys_enable_interrupt() (external function) */
    csi2_subsys_enable_interrupt();
    LOGT("CSI2 TPG: csi2_subsys_enable_interrupt() called");

    /* Step 9: Initialize the D-PHY */
    snps_phy_init();
    LOGT("CSI2 TPG: D-PHY initialized via snps_phy_init()");

    /* Step 10: Poll MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE until 0x1000f */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    timeout = PHY_STOPSTATE_TIMEOUT;
    while (rd_data != 0x1000f) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout == 0) {
            LOGE("CSI2 TPG: PHY_STOPSTATE poll timeout, rd_data=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("CSI2 TPG: PHY entered stop state, rd_data=0x%x", rd_data);

    /* Step 5 (frame size): Compute total frame data transfer size */
    hres = 320;
    vres = 16;
    valid_bits_per_pixel = 24;
    /* Compute line bytes: (hres * valid_bits_per_pixel / 8) aligned to 8 bytes */
    line_bytes = (hres * valid_bits_per_pixel) / 8;
    if ((line_bytes % 8) != 0) {
        line_bytes = ((line_bytes / 8) + 1) * 8;
    }
    csi2_data_trnsfr_size = line_bytes * vres;
    LOGT("CSI2 TPG: hres=%u vres=%u bpp=%u line_bytes=%u total_size=%u",
         hres, vres, valid_bits_per_pixel, line_bytes, csi2_data_trnsfr_size);

    /* Steps 11-17: Configure DMA higher-order address registers for channel 0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0);
    LOGT("CSI2 TPG: DMA M0 ADDR AR CH0 DATA configured");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("CSI2 TPG: DMA M0 ADDR AR CH0 INSTRUCTION configured");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("CSI2 TPG: DMA M0 ADDR AW CH0 DATA configured");

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("CSI2 TPG: DMA M0 ADDR AW CH0 INSTRUCTION configured");

    /* Step 18: Enable fractional divider output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("CSI2 TPG: Fractional divider output enabled (base+0xf4)");

    /* Steps 19-22: Preload DMA channel 0 transfer instructions and start DMA */
    write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x1);
    LOGT("CSI2 TPG: DMA interrupt enable set");

    ch0_pc = 0xE6000000;

    /* Preload DMA ch0 transfer instructions with source, destination, and transfer size */
#if defined(FPS60)
    dma_trnsfr_instn_preload_incr_addr(ch0_pc, gdma_reg_base, 0x0000, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size, 0, 1);
#else
    dma_trnsfr_instn_preload_incr_addr(ch0_pc, gdma_reg_base, 0x0000, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size, 0, 0);
#endif
    LOGT("CSI2 TPG: DMA ch0 transfer instructions preloaded");

    /* Start DMA channel 0 */
    DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);
    LOGT("CSI2 TPG: DMA channel 0 started");

    /* Steps 23-28: Enable and disable pattern generator */
    /* Configure vertical resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, vres);
    LOGT("CSI2 TPG: Pattern generator VRES=%u", vres);

    /* Configure horizontal resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, hres);
    LOGT("CSI2 TPG: Pattern generator HRES=%u", hres);

    /* Configure pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0x24);
    LOGT("CSI2 TPG: Pattern generator config set");

    /* Enable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1);
    LOGT("CSI2 TPG: Pattern generator enabled");

    /* Wait for pattern generation to complete */
    for (volatile int d0 = 0; d0 < 1000; d0++);

    /* Disable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0);
    LOGT("CSI2 TPG: Pattern generator disabled");

    /* Steps 29-32: Poll DMA interrupt status for channel 0 completion */
    rd_data = 0;
    timeout = DMA_POLL_TIMEOUT;
    while ((rd_data & 0x1) == 0x0) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout--;
        if (timeout == 0) {
            LOGE("CSI2 TPG: DMA ch0 poll timeout");
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("CSI2 TPG: DMA ch0 transfer complete, rd_data=0x%x", rd_data);

    /* Clear DMA interrupt for channel 0 */
    write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

    /* Steps 33-34: Wait for final settling period */
    for (volatile int d1 = 0; d1 < 1000; d1++);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV completion is handled via out->status.

    out->status = (g_ctx.errors == 0) ? 0 : -1;

    LOGT("mipi_csi2_test_pattern_generator run complete: %s errors=%u",
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

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0 ? 0 : -1;
}
