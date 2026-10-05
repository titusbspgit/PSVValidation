// Author - AI Force 2.3. 01-Oct-2026 13:31 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal PPI Pattern Generator
 *              functionality with DMA-based data reception using RGB888
 *              frame parameters (HRES=320, VRES=16, 24bpp).
 */

typedef struct {
    uint32_t gdma_reg_base;
    uint32_t vcid_csi2_wrap_reg;
    uint32_t gdma_path;
    uint32_t dma_ch0_pc;
    unsigned int errors;
} mipi_csi2_tpg_ctx_t;

static mipi_csi2_tpg_ctx_t g_ctx;

/* Helper: Enable all CSI-2 host interrupts (csi2_enable_interrupt) */
static void csi2_enable_interrupt(void)
{
    uint32_t rd_data;

    /* Step 36: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("CSI2 INT_ST_MAIN read to clear pending: 0x%x", rd_data);

    /* Step 37: Write 0x0000000f to INT_MSK_PHY_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("Wrote 0x0000000f to INT_MSK_PHY_FATAL");

    /* Step 38: Write 0x00000003 to INT_MSK_PKT_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("Wrote 0x00000003 to INT_MSK_PKT_FATAL");

    /* Step 39: Write 0x000f000f to INT_MSK_PHY */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("Wrote 0x000f000f to INT_MSK_PHY");

    /* Step 40: Write 0x000f000f to INT_MSK_LINE */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("Wrote 0x000f000f to INT_MSK_LINE");

    /* Step 41: Write 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL");

    /* Step 42: Write 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL");

    /* Step 43: Write 0x0000ffff to INT_MSK_CRC_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_CRC_FRAME_FATAL");

    /* Step 44: Write 0x0000ffff to INT_MSK_PLD_CRC_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_PLD_CRC_FATAL");

    /* Step 45: Write 0x0000ffff to INT_MSK_DATA_ID */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_DATA_ID");

    /* Step 46: Write 0x0000ffff to INT_MSK_ECC_CORRECTED */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_ECC_CORRECTED");
}

/* Helper: Enable Pattern Generator (csi2_ctrlr_pg_enable) */
static void csi2_ctrlr_pg_enable(void)
{
    /* Step 25: Write 0x10 to PPI_PG_PATTERN_VRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10U);
    LOGT("Wrote 0x10 to PPI_PG_PATTERN_VRES");

    /* Step 26: Write 0x70140 to PPI_PG_PATTERN_HRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140U);
    LOGT("Wrote 0x70140 to PPI_PG_PATTERN_HRES");

    /* Step 27: Write 0xe401 to PPI_PG_CONFIG */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401U);
    LOGT("Wrote 0xe401 to PPI_PG_CONFIG");

    /* Step 28: Write 1 to PPI_PG_ENABLE */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1U);
    LOGT("Pattern Generator enabled");
}

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
    uint32_t vcid;
    uint32_t vcid_unselected_path;

    (void)cfg;

    g_ctx = (mipi_csi2_tpg_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator init: starting PPI PG test initialization");

    /* Step 2: Set int_pend = 1 (internal variable, no register write) */

    /* Step 3: Set vcid = 3 */
    vcid = 3U;

    /* Step 4: Compute vcid_unselected_path */
    vcid_unselected_path = ((vcid + 1U) & 0xfU);
    LOGT("vcid=%u, vcid_unselected_path=0x%x", vcid, vcid_unselected_path);

    /* Step 5: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 1U;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 2U;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid << 12);
    g_ctx.gdma_path = 3U;
#else
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#endif

    /* Step 6: Compute gdma_reg_base */
    g_ctx.gdma_reg_base = 0xE6A00000U + (g_ctx.gdma_path * 0x1000U);
    LOGT("GDMA path=%u, gdma_reg_base=0x%x, vcid_csi2_wrap_reg=0x%x",
         g_ctx.gdma_path, g_ctx.gdma_reg_base, g_ctx.vcid_csi2_wrap_reg);

    /* Step 7: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Wrote 0x%x to VIRTUAL_CHANNEL", g_ctx.vcid_csi2_wrap_reg);

    /* Step 8: Write 0 to CONTROL_DATA to disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("Wrote 0 to CONTROL_DATA (disabled)");

    /* Step 9: Enable CSI-2 interrupts */
    // MANUAL_REVIEW: Source calls csi2_subsys_enable_interrupt() but only csi2_enable_interrupt() is defined in the source. Using csi2_enable_interrupt() as available.
    csi2_enable_interrupt();
    LOGT("CSI-2 host interrupts enabled");

    /* Step 10: Initialize the D-PHY */
    // MANUAL_REVIEW: snps_phy_init() implementation not in supplied source.
    snps_phy_init();
    LOGT("D-PHY initialized via snps_phy_init()");

    /* Steps 11-12: Poll PHY_STOPSTATE until all lanes in stop state (0x1000f) */
    {
        uint32_t rd_data;
        uint32_t timeout = MIPI_CSI2_POLL_TIMEOUT;

        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        LOGT("PHY_STOPSTATE initial read: 0x%x", rd_data);

        while ((rd_data != 0x1000fU) && (timeout > 0U)) {
            rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
            timeout--;
        }

        if (timeout == 0U) {
            LOGE("PHY_STOPSTATE polling timeout, rd_data=0x%x", rd_data);
            g_ctx.errors++;
            return -1;
        }
        LOGT("PHY_STOPSTATE reached 0x1000f, all lanes in stop state");
    }

    LOGT("mipi_csi2_test_pattern_generator init complete");
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
    uint32_t hres;
    uint32_t vres;
    uint32_t valid_bits_per_pixel;
    uint32_t csi2_data_trnsfr_size;
    uint32_t rd_data;
    uint32_t timeout;
    uint32_t dma_dest_addr_incr_flag;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_test_pattern_generator run");

    /* Step 13: Set frame parameters */
    hres = 320U;
    vres = 16U;
    valid_bits_per_pixel = 24U;
    LOGT("Frame params: hres=%u, vres=%u, bpp=%u", hres, vres, valid_bits_per_pixel);

    /* Step 14: Compute total DMA transfer size (8-byte aligned per line) */
    {
        uint32_t bytes_per_line = (hres * valid_bits_per_pixel) / 8U;
        uint32_t aligned_bytes_per_line = (bytes_per_line + 7U) & ~7U;
        csi2_data_trnsfr_size = aligned_bytes_per_line * vres;
    }
    LOGT("csi2_data_trnsfr_size=%u bytes", csi2_data_trnsfr_size);

    /* Step 15: Write 0x100 to DMA_M0_ADDR_AR_CH0_DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100U);
    LOGT("Wrote 0x100 to DMA_M0_ADDR_AR_CH0_DATA");

    /* Step 16: Write 0x0 to DMA_M0_ADDR_AR_CH0_INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);
    LOGT("Wrote 0x0 to DMA_M0_ADDR_AR_CH0_INSTRUCTION");

    /* Step 17: Write 0x0 to DMA_M0_ADDR_AW_CH0_DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0U);
    LOGT("Wrote 0x0 to DMA_M0_ADDR_AW_CH0_DATA");

    /* Step 18: Write 0x0 to DMA_M0_ADDR_AW_CH0_INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);
    LOGT("Wrote 0x0 to DMA_M0_ADDR_AW_CH0_INSTRUCTION");

    /* Step 19: Write 0x1 to subsystem register (base + 0xf4) for fractional divider output */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4U, 0x1U);
    LOGT("Wrote 0x1 to subsystem register (base+0xf4) for fractional divider output");

    /* Step 20: Set dma_ch0_pc */
    g_ctx.dma_ch0_pc = 0xE6000000U;
    LOGT("dma_ch0_pc=0x%x", g_ctx.dma_ch0_pc);

    /* Step 21: Set dma_dest_addr_incr_flag based on FPS60 conditional */
#if defined(FPS60)
    dma_dest_addr_incr_flag = 1U;
#else
    dma_dest_addr_incr_flag = 0U;
#endif
    LOGT("dma_dest_addr_incr_flag=%u", dma_dest_addr_incr_flag);

    /* Step 22: Preload DMA transfer instructions with address increment */
    // MANUAL_REVIEW: dma_trnsfr_instn_preload_incr_addr() exact 6-argument signature preserved; implementation not in supplied source.
    dma_trnsfr_instn_preload_incr_addr(g_ctx.dma_ch0_pc, 0x0000U, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size, 0U, dma_dest_addr_incr_flag);
    LOGT("DMA transfer instructions preloaded for ch0");

    /* Step 23: Start DMA channel 0 */
    // MANUAL_REVIEW: DMAGO_CSI() exact 3-argument signature preserved; implementation not in supplied source.
    DMAGO_CSI(g_ctx.gdma_reg_base, 0U, g_ctx.dma_ch0_pc);
    LOGT("DMA channel 0 started");

    /* Step 24: Enable Pattern Generator */
    csi2_ctrlr_pg_enable();
    LOGT("Pattern Generator configuration complete");

    /* Step 30: wait_on(100) replaced with busy-wait loop */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("Short wait period complete");

    /* Step 31: Disable Pattern Generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0U);
    LOGT("Pattern Generator disabled");

    /* Steps 32-33: Poll DMA INTMIS until bit 0 is set (ch0 completion) */
    rd_data = 0U;
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    while (((rd_data & 0x1U) == 0U) && (timeout > 0U)) {
        rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("DMA ch0 poll timeout, rd_data=0x%x", rd_data);
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }
    LOGT("DMA ch0 transfer complete, INTMIS=0x%x", rd_data);

    /* Step 34: wait_on(10000) replaced with busy-wait loop (settling period) */
    for (volatile int d1 = 0; d1 < 10000; d1++);
    LOGT("Settling wait period complete");

    /* Step 35: DV finish(0) converted to PSV/FV pass status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to FV out->status PASS.
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

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
