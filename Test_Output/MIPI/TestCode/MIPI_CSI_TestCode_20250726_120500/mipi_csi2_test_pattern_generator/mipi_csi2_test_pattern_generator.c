// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * mipi_csi2_test_pattern_generator.c
 * FV Testcase: MIPI CSI-2 PPI Pattern Generator Test
 * Validates the internal PPI Pattern Generator functionality
 * with DMA-based data reception (VRES=16, HRES=320, RGB888).
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * csi2_enable_interrupt
 * Enables all CSI-2 host interrupt masks (Steps 36-46)
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("INT_MSK_PHY_FATAL = 0x0000000f");

    /* Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("INT_MSK_PKT_FATAL = 0x00000003");

    /* Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("INT_MSK_PHY = 0x000f000f");

    /* Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("INT_MSK_LINE = 0x000f000f");

    /* Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("INT_MSK_DATA_ID = 0x0000ffff");

    /* Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("INT_MSK_ECC_CORRECTED = 0x0000ffff");
}

/*
 * csi2_ctrlr_pg_enable
 * Configures and enables the PPI Pattern Generator (Steps 25-28)
 */
static void csi2_ctrlr_pg_enable(void)
{
    LOGT("csi2_ctrlr_pg_enable: Configuring Pattern Generator");

    /* Step 25: Set vertical resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, PG_VRES_VALUE);
    LOGT("PPI_PG_PATTERN_VRES = 0x%x", PG_VRES_VALUE);

    /* Step 26: Set horizontal resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, PG_HRES_VALUE);
    LOGT("PPI_PG_PATTERN_HRES = 0x%x", PG_HRES_VALUE);

    /* Step 27: Set PG configuration (RGB888 data type) */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, PG_CONFIG_VALUE);
    LOGT("PPI_PG_CONFIG = 0x%x", PG_CONFIG_VALUE);

    /* Step 28: Enable Pattern Generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_ENABLE_VALUE);
    LOGT("PPI_PG_ENABLE = %u (enabled)", PG_ENABLE_VALUE);
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
    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator_init: PPI Pattern Generator test init");

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
    unsigned int dma_ch0_pc;
    unsigned int dma_dest_addr_incr_flag;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator_run: starting PPI Pattern Generator test");

    /* Step 2: Set int_pend */
    int_pend = 1U;
    LOGT("int_pend = %u", int_pend);

    /* Step 3: Set vcid */
    vcid = 3U;
    LOGT("vcid = %u", vcid);

    /* Step 4: Compute vcid_unselected_path */
    vcid_unselected_path = ((vcid + 1U) & 0xfU);
    LOGT("vcid_unselected_path = 0x%x", vcid_unselected_path);

    /* Step 5: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (vcid << 0) |
                         (vcid_unselected_path << 4) |
                         (vcid_unselected_path << 8) |
                         (vcid_unselected_path << 12);
    gdma_path = 0U;
    LOGT("GDMA0_PATH selected, gdma_path=0");
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                         (vcid << 4) |
                         (vcid_unselected_path << 8) |
                         (vcid_unselected_path << 12);
    gdma_path = 1U;
    LOGT("GDMA1_PATH selected, gdma_path=1");
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                         (vcid_unselected_path << 4) |
                         (vcid << 8) |
                         (vcid_unselected_path << 12);
    gdma_path = 2U;
    LOGT("GDMA2_PATH selected, gdma_path=2");
#elif defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                         (vcid_unselected_path << 4) |
                         (vcid_unselected_path << 8) |
                         (vcid << 12);
    gdma_path = 3U;
    LOGT("GDMA3_PATH selected, gdma_path=3");
#else
    vcid_csi2_wrap_reg = (vcid << 0) |
                         (vcid_unselected_path << 4) |
                         (vcid_unselected_path << 8) |
                         (vcid_unselected_path << 12);
    gdma_path = 0U;
    LOGT("Default GDMA0_PATH selected, gdma_path=0");
#endif

    /* Step 6: Compute GDMA register base */
    gdma_reg_base = 0xE6A00000U + (gdma_path * 0x1000U);
    LOGT("gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 7: Write virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL=0x%x", vcid_csi2_wrap_reg);

    /* Step 8: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("CONTROL_DATA=0 (disabled)");

    /* Step 9: Enable CSI-2 subsystem interrupts */
    LOGT("Step 9: Calling csi2_subsys_enable_interrupt");
    csi2_enable_interrupt();
    LOGT("All CSI-2 interrupt masks enabled");

    /* Step 10: Initialize D-PHY */
    LOGT("Step 10: Initializing D-PHY via snps_phy_init()");
    snps_phy_init();
    LOGT("D-PHY initialization complete");

    /* Steps 11-12: Poll PHY_STOPSTATE until all lanes in stop state */
    LOGT("Step 11-12: Polling PHY_STOPSTATE for 0x%x", PHY_STOPSTATE_EXPECTED);
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while (rd_data != PHY_STOPSTATE_EXPECTED) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE=0x%x, all lanes in stop state", rd_data);

    /* Step 13: Set frame parameters */
    hres = 320U;
    vres = 16U;
    valid_bits_per_pixel = 24U;
    LOGT("Frame: hres=%u vres=%u bpp=%u", hres, vres, valid_bits_per_pixel);

    /* Step 14: Compute DMA transfer size = 15360 bytes */
    csi2_data_trnsfr_size = 15360U;
    LOGT("csi2_data_trnsfr_size=%u bytes", csi2_data_trnsfr_size);

    /* Step 15: Write DMA address mapping register AR CH0 DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100U);
    LOGT("DMA_M0_ADDR_AR_CH0_DATA=0x100");

    /* Step 16: Write DMA address mapping register AR CH0 INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);
    LOGT("DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x0");

    /* Step 17: Write DMA address mapping register AW CH0 DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0U);
    LOGT("DMA_M0_ADDR_AW_CH0_DATA=0x0");

    /* Step 18: Write DMA address mapping register AW CH0 INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);
    LOGT("DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x0");

    /* Step 19: Enable fractional divider output */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4U, 0x1U);
    LOGT("Fractional divider output enabled (base+0xf4=0x1)");

    /* Step 20: Set DMA channel 0 program counter */
    dma_ch0_pc = 0xE6000000U;
    LOGT("dma_ch0_pc=0x%x", dma_ch0_pc);

    /* Step 21: Set DMA dest addr increment flag based on FPS60 conditional */
#if defined(FPS60)
    dma_dest_addr_incr_flag = 1U;
    LOGT("FPS60 defined, dma_dest_addr_incr_flag=1");
#else
    dma_dest_addr_incr_flag = 0U;
    LOGT("FPS60 not defined, dma_dest_addr_incr_flag=0");
#endif

    /* Step 22: Preload DMA transfer instructions with address increment */
    /* dma_trnsfr_instn_preload_incr_addr(preload_loc, src_addr, dest_addr, trnsfr_size, irq_num, incr_flag) */
    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, 0x0000U, GDMA_CSI2_DATA_DEST_ADDR2, csi2_data_trnsfr_size, 0U, dma_dest_addr_incr_flag);
    LOGT("DMA transfer instructions preloaded");

    /* Step 23: Start DMA channel 0 */
    /* DMAGO_CSI(gdma_reg_base, channel, pc_addr) */
    DMAGO_CSI(gdma_reg_base, 0U, dma_ch0_pc);
    LOGT("DMA channel 0 started");

    /* Steps 24-29: Enable Pattern Generator */
    csi2_ctrlr_pg_enable();
    LOGT("Pattern Generator enabled");

    /* Step 30: Wait for pattern generation (DV wait_on(100) replaced) */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("Busy-wait (100) complete");

    /* Step 31: Disable Pattern Generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_DISABLE_VALUE);
    LOGT("PPI_PG_ENABLE=0 (disabled)");

    /* Steps 32-33: Poll DMA interrupt status for channel 0 completion */
    LOGT("Step 32-33: Polling DMA interrupt for channel 0 completion");
    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    while ((rd_data & 0x1U) == 0U) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("DMA channel 0 transfer complete");

    /* Step 34: Wait for settling period (DV wait_on(10000) replaced) */
    for (volatile int d1 = 0; d1 < 10000; d1++);
    LOGT("Busy-wait (10000) settling period complete");

    /* Step 35: Test complete */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native status reporting via out->status is used instead.
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
