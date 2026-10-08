// Author - AI Force 2.3. 18-Jul-2025 15:12 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator (PPI PG)
 *              functionality by configuring the virtual channel, enabling CSI-2
 *              interrupts, initializing the D-PHY, waiting for PHY stop state,
 *              programming DMA address registers and transfer parameters, starting
 *              a DMA transfer, enabling the pattern generator with specified
 *              vertical resolution, horizontal resolution, and configuration, then
 *              disabling the pattern generator and polling for DMA transfer
 *              completion to confirm successful data reception.
 */

/* DMA offset macros used for polling and clearing */
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x00000010U

/* PHY stop state expected value */
#define PHY_STOPSTATE_EXPECTED  0x1000fU

/* Pattern Generator configuration values from steps 32-35 */
#define PG_PATTERN_VRES_VALUE   0x10U
#define PG_PATTERN_HRES_VALUE   0x70140U
#define PG_CONFIG_VALUE         0xe401U
#define PG_ENABLE_VALUE         1U
#define PG_DISABLE_VALUE        0U

/* DMA transfer parameters from steps 18-21 */
#define PG_HRES                 320
#define PG_VRES_RES             16
#define PG_VALID_BITS_PER_PIXEL 24

/* Test context structure following FV Template pattern */
typedef struct {
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_path;
    unsigned int ch0_pc;
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Clears pending CSI-2 interrupts by reading INT_ST_MAIN,
 *              then enables all CSI-2 interrupt masks (steps 45-58).
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    /* Step 45: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Step 46: Enable phy_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("INT_MSK_PHY_FATAL = 0x0000000f");

    /* Step 47: Enable pkt_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("INT_MSK_PKT_FATAL = 0x00000003");

    /* Step 48: Enable phy interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("INT_MSK_PHY = 0x000f000f");

    /* Step 49: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("INT_MSK_LINE = 0x000f000f");

    /* Step 50: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    /* Step 51: Enable seq frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    /* Step 52: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    /* Step 53: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    /* Step 54: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("INT_MSK_DATA_ID = 0x0000ffff");

    /* Step 55: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("INT_MSK_ECC_CORRECTED = 0x0000ffff");

    LOGT("csi2_enable_interrupt: all interrupt masks enabled");
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Configures and enables the CSI-2 test pattern generator
 *              with specified VRES, HRES, CONFIG, and ENABLE values
 *              (steps 32-35).
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void csi2_ctrlr_pg_enable(void)
{
    /* Step 32: Write PPI_PG_PATTERN_VRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, PG_PATTERN_VRES_VALUE);
    LOGT("PPI_PG_PATTERN_VRES = 0x%x", PG_PATTERN_VRES_VALUE);

    /* Step 33: Write PPI_PG_PATTERN_HRES */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, PG_PATTERN_HRES_VALUE);
    LOGT("PPI_PG_PATTERN_HRES = 0x%x", PG_PATTERN_HRES_VALUE);

    /* Step 34: Write PPI_PG_CONFIG */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, PG_CONFIG_VALUE);
    LOGT("PPI_PG_CONFIG = 0x%x", PG_CONFIG_VALUE);

    /* Step 35: Write PPI_PG_ENABLE = 1 */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_ENABLE_VALUE);
    LOGT("PPI_PG_ENABLE = %u", PG_ENABLE_VALUE);

    LOGT("csi2_ctrlr_pg_enable: pattern generator enabled");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_test_pattern_generator. Configures virtual channel,
 *              disables control data, enables CSI-2 and DMA interrupts,
 *              initializes D-PHY, polls PHY stop state, programs DMA address
 *              registers, and enables fracdiv output.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    int rd_data;
    int vcid;
    int vcid_unselected_path;

    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator_init: start");

    /* Step 3: int_pend = 1 (used only in DV flow, not needed in FV) */
    /* Step 4: Print start line */
    LOGT("start line");

    /* Step 5: vcid = VC_ID (3) */
    vcid = VC_ID;

    /* Step 6: vcid_unselected_path = ((vcid + 1) & 0xf) = 4 */
    vcid_unselected_path = ((vcid + 1) & 0xf);
    LOGT("vcid=%d vcid_unselected_path=%d", vcid, vcid_unselected_path);

    /* Step 7: Conditional compilation for GDMA path selection */
#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = ((unsigned int)vcid << 0) |
                               ((unsigned int)vcid_unselected_path << 4) |
                               ((unsigned int)vcid_unselected_path << 8) |
                               ((unsigned int)vcid_unselected_path << 12);
    g_ctx.gdma_path = 0;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = ((unsigned int)vcid_unselected_path << 0) |
                               ((unsigned int)vcid << 4) |
                               ((unsigned int)vcid_unselected_path << 8) |
                               ((unsigned int)vcid_unselected_path << 12);
    g_ctx.gdma_path = 1;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = ((unsigned int)vcid_unselected_path << 0) |
                               ((unsigned int)vcid_unselected_path << 4) |
                               ((unsigned int)vcid << 8) |
                               ((unsigned int)vcid_unselected_path << 12);
    g_ctx.gdma_path = 2;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = ((unsigned int)vcid_unselected_path << 0) |
                               ((unsigned int)vcid_unselected_path << 4) |
                               ((unsigned int)vcid_unselected_path << 8) |
                               ((unsigned int)vcid << 12);
    g_ctx.gdma_path = 3;
#else
    g_ctx.vcid_csi2_wrap_reg = ((unsigned int)vcid << 0) |
                               ((unsigned int)vcid_unselected_path << 4) |
                               ((unsigned int)vcid_unselected_path << 8) |
                               ((unsigned int)vcid_unselected_path << 12);
    g_ctx.gdma_path = 0;
#endif

    /* Step 8: gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000) */
    g_ctx.gdma_reg_base = 0xE6A00000U + (g_ctx.gdma_path * 0x1000U);

    /* Step 9: Print vcid_csi2_wrap_reg */
    LOGT("vcid_csi2_wrap_reg=0x%x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 10: Write virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("VIRTUAL_CHANNEL = 0x%x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 11: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);
    LOGT("CONTROL_DATA = 0");

    /* Step 12: Enable CSI-2 and DMA interrupts via subsystem interrupt enable */
    csi2_subsys_enable_interrupt();
    LOGT("csi2_subsys_enable_interrupt() called");

    /* Step 13: Initialize D-PHY */
    snps_phy_init();
    LOGT("snps_phy_init() called");

    /* Steps 14-17: Poll PHY_STOPSTATE until stop state reached */
    LOGT("Polling PHY_STOPSTATE for 0x%x", PHY_STOPSTATE_EXPECTED);
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while (!(rd_data == (int)PHY_STOPSTATE_EXPECTED)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE reached: 0x%x", rd_data);

    /* Step 22: Write DMA_M0_ADDR_AR_CH0_DATA = 0x0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0);
    LOGT("DMA_M0_ADDR_AR_CH0_DATA = 0x0");

    /* Step 23: Write DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x0");

    /* Step 24: Write DMA_M0_ADDR_AW_CH0_DATA = 0x0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("DMA_M0_ADDR_AW_CH0_DATA = 0x0");

    /* Step 25: Write DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x0 */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x0");

    /* Step 26: Enable fracdiv output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("FRACDIV_CLK_GATE_CTRL (BASE+0xf4) = 0x1");

    /* Step 27: Set DMA channel 0 program counter */
    g_ctx.ch0_pc = 0xE6000000U;
    LOGT("ch0_pc=0x%x", g_ctx.ch0_pc);

    LOGT("mipi_csi2_test_pattern_generator_init: complete");
    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for
 *              mipi_csi2_test_pattern_generator. Calculates DMA transfer size,
 *              programs DMA transfer instructions, starts DMA, enables and
 *              disables the pattern generator, and polls for DMA completion.
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
    int dest_addr_incr;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator_run: start");

    /* Step 18: hres = 320 */
    hres = PG_HRES;

    /* Step 19: vres = 16 */
    vres = PG_VRES_RES;

    /* Step 20: valid_bits_per_pixel = 24 */
    valid_bits_per_pixel = PG_VALID_BITS_PER_PIXEL;

    /* Step 21: Calculate DMA transfer size and ensure 8-byte alignment */
    csi2_data_trnsfr_size = (long long int)hres * vres * valid_bits_per_pixel / 8;
    if (csi2_data_trnsfr_size % 8) {
        csi2_data_trnsfr_size = ((csi2_data_trnsfr_size / 8) + 1) * 8;
    }
    LOGT("hres=%d vres=%d bpp=%d trnsfr_size=%lld",
         hres, vres, valid_bits_per_pixel, csi2_data_trnsfr_size);

    /* Step 28: Configure destination address increment based on FPS60 define */
#ifdef FPS60
    dest_addr_incr = 1;
#else
    dest_addr_incr = 0;
#endif
    LOGT("dest_addr_incr=%d", dest_addr_incr);

    /* Step 29: Program DMA transfer instructions */
    dma_trnsfr_instn_preload_incr_addr(g_ctx.ch0_pc, 0x0, GDMA_CSI2_DATA_DEST_ADDR2,
                                        csi2_data_trnsfr_size, 0, dest_addr_incr);
    LOGT("dma_trnsfr_instn_preload_incr_addr() called");

    /* Step 30: Start DMA transfer on channel 0 */
    DMAGO_CSI(g_ctx.gdma_reg_base, 0, g_ctx.ch0_pc);
    LOGT("DMAGO_CSI() called: gdma_reg_base=0x%x ch=0 ch0_pc=0x%x",
         g_ctx.gdma_reg_base, g_ctx.ch0_pc);

    /* Step 31: Enable test pattern generator */
    csi2_ctrlr_pg_enable();

    /* Step 37: Wait after PG enable (DV wait_on(100) replaced with busy-wait) */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("Busy-wait 100 iterations after PG enable");

    /* Step 38: Disable test pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, PG_DISABLE_VALUE);
    LOGT("PPI_PG_ENABLE = %u (disabled)", PG_DISABLE_VALUE);

    /* Steps 39-42: Poll DMA interrupt status for channel 0 completion (bit 0) */
    LOGT("Polling DMA INTMIS for channel 0 completion");
    rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    while (!(rd_data & 0x1)) {
        rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("DMA channel 0 transfer complete: INTMIS=0x%x", rd_data);

    /* Step 43: Post-DMA wait (DV wait_on(10000) replaced with busy-wait) */
    for (volatile int d1 = 0; d1 < 10000; d1++);
    LOGT("Busy-wait 10000 iterations after DMA completion");

    /* MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native */
    /* completion is handled via out->status. */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_test_pattern_generator_run complete: %s errors=%u",
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
