// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal PPI Pattern Generator with
 *              DMA-based data reception. Configures virtual channel, disables
 *              control data transfer, enables interrupts, initializes D-PHY,
 *              programs DMA address mapping, enables the Pattern Generator with
 *              RGB888 frame (320x16), and verifies DMA completion.
 */

typedef struct {
    unsigned int gdma_reg_base;
    unsigned int gdma_path;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Reads INT_ST_MAIN to clear pending interrupts, then writes all
 *              CSI-2 host interrupt mask registers (Steps 36-46).
 */
static void csi2_enable_interrupt(void)
{
    volatile unsigned int rd_data;

    LOGT("csi2_enable_interrupt: clearing pending interrupts");

    /* Step 36: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("Read INT_ST_MAIN = 0x%lx", (unsigned long)rd_data);

    /* Step 37: Write 0x0000000f to INT_MSK_PHY_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("Write INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 38: Write 0x00000003 to INT_MSK_PKT_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("Write INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 39: Write 0x000f000f to INT_MSK_PHY */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("Write INT_MSK_PHY = 0x000f000f");

    /* Step 40: Write 0x000f000f to INT_MSK_LINE */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("Write INT_MSK_LINE = 0x000f000f");

    /* Step 41: Write 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 42: Write 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 43: Write 0x0000ffff to INT_MSK_CRC_FRAME_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 44: Write 0x0000ffff to INT_MSK_PLD_CRC_FATAL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("Write INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 45: Write 0x0000ffff to INT_MSK_DATA_ID */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("Write INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 46: Write 0x0000ffff to INT_MSK_ECC_CORRECTED */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("Write INT_MSK_ECC_CORRECTED = 0x0000ffff");

    LOGT("csi2_enable_interrupt: all interrupt masks configured");
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Configures and enables the PPI Pattern Generator with RGB888
 *              frame parameters (Steps 25-28).
 */
static void csi2_ctrlr_pg_enable(void)
{
    LOGT("csi2_ctrlr_pg_enable: configuring pattern generator");

    /* Step 25: Write 0x10 to PPI_PG_PATTERN_VRES (16 lines) */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10U);
    LOGT("Write PPI_PG_PATTERN_VRES = 0x10");

    /* Step 26: Write 0x70140 to PPI_PG_PATTERN_HRES (320 pixels with config) */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140U);
    LOGT("Write PPI_PG_PATTERN_HRES = 0x70140");

    /* Step 27: Write 0xe401 to PPI_PG_CONFIG (RGB888 data type) */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401U);
    LOGT("Write PPI_PG_CONFIG = 0xe401");

    /* Step 28: Write 1 to PPI_PG_ENABLE */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1U);
    LOGT("Write PPI_PG_ENABLE = 0x1 (enabled)");

    LOGT("csi2_ctrlr_pg_enable: pattern generator enabled");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_test_pattern_generator. Computes virtual channel,
 *              disables control data, enables interrupts, initializes D-PHY,
 *              and polls PHY stop state.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    volatile unsigned int rd_data;
    unsigned int vcid;
    unsigned int vcid_unselected_path;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator_init: start");

    /* Step 2: Set int_pend = 1 (used internally, not stored in ctx) */
    /* Step 3: Set vcid = 3 */
    vcid = 3U;
    LOGT("vcid = %u", vcid);

    /* Step 4: Compute vcid_unselected_path */
    vcid_unselected_path = ((vcid + 1U) & 0xfU);
    LOGT("vcid_unselected_path = 0x%x", vcid_unselected_path);

    /* Step 5: Compute vcid_csi2_wrap_reg based on GDMA path */
#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) | (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) | (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) | (vcid << 4) |
                               (vcid_unselected_path << 8) | (vcid_unselected_path << 12);
    g_ctx.gdma_path = 1U;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) | (vcid_unselected_path << 4) |
                               (vcid << 8) | (vcid_unselected_path << 12);
    g_ctx.gdma_path = 2U;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) | (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) | (vcid << 12);
    g_ctx.gdma_path = 3U;
#else
    /* Default to GDMA0 path */
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) | (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) | (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#endif

    /* Step 6: Compute gdma_reg_base */
    g_ctx.gdma_reg_base = MIPI_CSI2_GDMA_REG_BASE + (g_ctx.gdma_path * 0x1000U);
    LOGT("gdma_path=%u gdma_reg_base=0x%lx vcid_csi2_wrap_reg=0x%lx",
         g_ctx.gdma_path,
         (unsigned long)g_ctx.gdma_reg_base,
         (unsigned long)g_ctx.vcid_csi2_wrap_reg);

    /* Step 7: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL = 0x%lx", (unsigned long)g_ctx.vcid_csi2_wrap_reg);

    /* Step 8: Write 0 to CONTROL_DATA to disable control data transfer */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("Write CONTROL_DATA = 0x0 (disabled)");

    /* Step 9: Call csi2_subsys_enable_interrupt (implemented as csi2_enable_interrupt) */
    csi2_enable_interrupt();

    /* Step 10: Initialize D-PHY */
    LOGT("Calling snps_phy_init()");
    snps_phy_init();
    LOGT("snps_phy_init() complete");

    /* Step 11-12: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("Polling PHY_STOPSTATE for 0x%lx", (unsigned long)PHY_STOPSTATE_EXPECTED);
    rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    timeout = MIPI_CSI2_POLL_TIMEOUT;

    while ((rd_data != PHY_STOPSTATE_EXPECTED) && (timeout > 0U)) {
        rd_data = readl_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (rd_data != PHY_STOPSTATE_EXPECTED) {
        LOGE("PHY_STOPSTATE timeout: expected=0x%lx actual=0x%lx",
             (unsigned long)PHY_STOPSTATE_EXPECTED,
             (unsigned long)rd_data);
        g_ctx.errors++;
        return -1;
    }

    LOGT("PHY_STOPSTATE = 0x%lx — all lanes in stop state", (unsigned long)rd_data);
    LOGT("mipi_csi2_test_pattern_generator_init: complete");

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for
 *              mipi_csi2_test_pattern_generator. Programs DMA address mapping,
 *              enables fractional divider, starts DMA channel 0, enables and
 *              disables the PPI Pattern Generator, and polls DMA completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    volatile unsigned int rd_data;
    unsigned int csi2_data_trnsfr_size;
    unsigned int dma_ch0_pc;
    unsigned int dma_dest_addr_incr_flag;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator_run: start");

    /* Step 13-14: Set frame parameters and compute DMA transfer size */
    /* hres=320, vres=16, valid_bits_per_pixel=24 */
    /* (320 * 24 / 8) = 960 bytes per line, 960 * 16 = 15360 bytes total */
    csi2_data_trnsfr_size = MIPI_CSI2_DATA_TRNSFR_SIZE;
    LOGT("csi2_data_trnsfr_size = %u bytes", csi2_data_trnsfr_size);

    /* Step 15: Write 0x100 to DMA_M0_ADDR_AR_CH0_DATA */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100U);
    LOGT("Write DMA_M0_ADDR_AR_CH0_DATA = 0x100");

    /* Step 16: Write 0x0 to DMA_M0_ADDR_AR_CH0_INSTRUCTION */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);
    LOGT("Write DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x0");

    /* Step 17: Write 0x0 to DMA_M0_ADDR_AW_CH0_DATA */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0U);
    LOGT("Write DMA_M0_ADDR_AW_CH0_DATA = 0x0");

    /* Step 18: Write 0x0 to DMA_M0_ADDR_AW_CH0_INSTRUCTION */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);
    LOGT("Write DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x0");

    /* Step 19: Write 0x1 to MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4 */
    writel_reg((uintptr_t)(MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_FRAC_DIV_OFFSET), 0x1U);
    LOGT("Write (RB_REG_BASE + 0xf4) = 0x1");

    /* Step 20: Set dma_ch0_pc */
    dma_ch0_pc = MIPI_CSI2_DMA_CH0_PC;
    LOGT("dma_ch0_pc = 0x%lx", (unsigned long)dma_ch0_pc);

    /* Step 21: Set dma_dest_addr_incr_flag based on FPS60 conditional */
#if defined(FPS60)
    dma_dest_addr_incr_flag = 1U;
#else
    dma_dest_addr_incr_flag = 0U;
#endif
    LOGT("dma_dest_addr_incr_flag = %u", dma_dest_addr_incr_flag);

    /* Step 22: Call dma_trnsfr_instn_preload_incr_addr */
    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, 0x0000U,
                                       GDMA_CSI2_DATA_DEST_ADDR2,
                                       csi2_data_trnsfr_size, 0U,
                                       dma_dest_addr_incr_flag);
    LOGT("DMA channel 0 preloaded");

    /* Step 23: Call DMAGO_CSI to start DMA channel 0 */
    DMAGO_CSI(0U, dma_ch0_pc, g_ctx.gdma_reg_base);
    LOGT("DMA channel 0 started");

    /* Step 24-29: Call csi2_ctrlr_pg_enable */
    csi2_ctrlr_pg_enable();

    /* Step 30: Call wait_on(100) */
    LOGT("Calling wait_on(100)");
    wait_on(100U);

    /* Step 31: Write 0 to PPI_PG_ENABLE to disable pattern generator */
    writel_reg((uintptr_t)MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0U);
    LOGT("Write PPI_PG_ENABLE = 0x0 (disabled)");

    /* Step 32-33: Poll DMA INTMIS until bit 0 is set */
    LOGT("Polling DMA INTMIS for channel 0 completion");
    rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET));
    timeout = MIPI_CSI2_POLL_TIMEOUT;

    while (((rd_data & 0x1U) == 0U) && (timeout > 0U)) {
        rd_data = readl_reg((uintptr_t)(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET));
        timeout--;
    }

    if ((rd_data & 0x1U) == 0U) {
        LOGE("DMA ch0 INTMIS timeout: rd_data=0x%lx", (unsigned long)rd_data);
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }

    LOGT("DMA channel 0 transfer complete (INTMIS = 0x%lx)", (unsigned long)rd_data);

    /* Step 34: Call wait_on(10000) */
    LOGT("Calling wait_on(10000)");
    wait_on(10000U);

    /* Step 35: DV finish(0) converted to PSV/FV status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_test_pattern_generator_run: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

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

    LOGT("mipi_csi2_test_pattern_generator teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
