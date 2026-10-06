// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator (PPI PG)
 *              and verifies data reception through the DMA path. Configures virtual
 *              channel, disables control data, enables interrupts, initializes D-PHY,
 *              programs DMA, enables pattern generator, and verifies DMA completion.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Clears pending CSI-2 interrupts and enables all interrupt masks.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Step 7: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: cleared pending interrupts INT_ST_MAIN=0x%x",
         (unsigned int)rd_data);

    /* Step 8: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("write INT_MSK_PHY_FATAL=0x0000000f");

    /* Step 9: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("write INT_MSK_PKT_FATAL=0x00000003");

    /* Step 10: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("write INT_MSK_PHY=0x000f000f");

    /* Step 11: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("write INT_MSK_LINE=0x000f000f");

    /* Step 12: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_BNDRY_FRAME_FATAL=0x0000ffff");

    /* Step 13: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_SEQ_FRAME_FATAL=0x0000ffff");

    /* Step 14: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_CRC_FRAME_FATAL=0x0000ffff");

    /* Step 15: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_PLD_CRC_FATAL=0x0000ffff");

    /* Step 16: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("write INT_MSK_DATA_ID=0x0000ffff");

    /* Step 17: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("write INT_MSK_ECC_CORRECTED=0x0000ffff");
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Configures and enables the PPI test pattern generator.
 * Parameters:
 *   vres   - Vertical resolution.
 *   hres   - Horizontal resolution.
 *   config - Pattern generator configuration value.
 * Returns:
 *   void.
 */
static void csi2_ctrlr_pg_enable(unsigned int vres, unsigned int hres, unsigned int config)
{
    LOGT("csi2_ctrlr_pg_enable: configuring pattern generator");

    /* Step 34: Set vertical resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, vres);
    LOGT("write PPI_PG_PATTERN_VRES=0x%x (%u)", vres, vres);

    /* Step 35: Set horizontal resolution */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, hres);
    LOGT("write PPI_PG_PATTERN_HRES=0x%x (%u)", hres, hres);

    /* Step 36: Set pattern configuration */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, config);
    LOGT("write PPI_PG_CONFIG=0x%x", config);

    /* Step 37: Enable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1U);
    LOGT("write PPI_PG_ENABLE=0x1 (enabled)");
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

    LOGT("mipi_csi2_test_pattern_generator init: starting test pattern generator initialization");

    /* Steps 7-19: Enable CSI-2 interrupts */
    csi2_enable_interrupt();
    LOGT("CSI-2 interrupts enabled");

    /* Step 20: D-PHY initialization */
    snps_phy_init();
    LOGT("D-PHY initialization complete via snps_phy_init()");

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
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int ch0_pc;
    unsigned int transfer_size;
    unsigned int int_pend;
    unsigned int vcid;
    unsigned int vcid_unselected_path;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator run: starting main test flow");

    /* Step 1: Set int_pend */
    int_pend = 1U;
    LOGT("int_pend=%u", int_pend);

    /* Step 2: Set vcid */
    vcid = 3U;
    LOGT("vcid=%u", vcid);

    /* Step 3: Determine vcid_csi2_wrap_reg based on GDMA path */
#ifdef GDMA_PATH_1
    vcid_unselected_path = 0x0fU;
    vcid_csi2_wrap_reg = (vcid << 8) | vcid_unselected_path;
#else
    vcid_unselected_path = 0x0fU << 8;
    vcid_csi2_wrap_reg = vcid | vcid_unselected_path;
#endif
    LOGT("vcid_csi2_wrap_reg=0x%x", vcid_csi2_wrap_reg);

    /* Step 4: Calculate gdma_reg_base */
    gdma_reg_base = MIPI_CSI2_GDMA_REG_BASE;
    LOGT("gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 5: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("write VIRTUAL_CHANNEL=0x%x", vcid_csi2_wrap_reg);

    /* Step 6: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x0U);
    LOGT("write CONTROL_DATA=0x0 (disabled)");

    /* Steps 21-22: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("polling PHY_STOPSTATE for stop state entry");
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    } while (rd_data != MIPI_CSI2_PHY_STOPSTATE_EXPECTED);
    LOGT("PHY_STOPSTATE=0x%x all lanes and clock stopped", rd_data);

    /* Steps 23-24: Calculate transfer size */
    /* transfer_size = (hres * vres * valid_bits_per_pixel) / 8 with 8-byte alignment */
    transfer_size = (MIPI_CSI2_PG_HRES * MIPI_CSI2_PG_VRES * MIPI_CSI2_PG_VALID_BITS_PER_PIXEL) / 8U;
    transfer_size = ((transfer_size + 7U) / 8U) * 8U;
    LOGT("calculated transfer_size=%u bytes (8-byte aligned)", transfer_size);

    /* Step 25: Program DMA higher-order AXI address register AR CH0 DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x0U);
    LOGT("write DMA_M0_ADDR_AR_CH0_DATA=0x0");

    /* Step 26: Program DMA higher-order AXI address register AR CH0 INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0U);
    LOGT("write DMA_M0_ADDR_AR_CH0_INSTRUCTION=0x0");

    /* Step 27: Program DMA higher-order AXI address register AW CH0 DATA */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("write DMA_M0_ADDR_AW_CH0_DATA=0x%x", (unsigned int)GDMA_CSI2_DATA_DEST_ADDR2);

    /* Step 28: Program DMA higher-order AXI address register AW CH0 INSTRUCTION */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0U);
    LOGT("write DMA_M0_ADDR_AW_CH0_INSTRUCTION=0x0");

    /* Step 29: Enable fractional divider output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + MIPI_CSI2_ENABLECLKGATING_CSIPHY_OFFSET, 0x1U);
    LOGT("write enableclkgating_csiphy (BASE+0xf4)=0x1");

    /* Steps 30-33: Program and start DMA channel 0 */
    ch0_pc = gdma_reg_base + 0x400U;
    LOGT("DMA PC address: ch0_pc=0x%x", ch0_pc);

    dma_trnsfr_instn_preload_incr_addr(gdma_reg_base, 0, 0, GDMA_CSI2_DATA_DEST_ADDR2, transfer_size);
    DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);
    LOGT("DMA channel 0 started ch0_pc=0x%x size=%u", ch0_pc, transfer_size);

    /* Steps 34-39: Enable test pattern generator */
    LOGT("enabling test pattern generator");
    csi2_ctrlr_pg_enable(MIPI_CSI2_PG_VRES, MIPI_CSI2_PG_HRES, MIPI_CSI2_PG_CONFIG_VALUE);
    LOGT("pattern generator enabled");

    /* Step 40: Wait for pattern generation (DV wait_on(100) replaced with busy-wait) */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("waited 100 cycles for pattern generation");

    /* Step 41: Disable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0U);
    LOGT("write PPI_PG_ENABLE=0x0 (disabled)");

    /* Steps 42-43: Poll DMA interrupt status for CH0 completion (bit 0) */
    LOGT("polling DMA interrupt status for CH0 completion");
    do {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    } while ((rd_data & 0x1U) == 0U);
    LOGT("DMA CH0 transfer complete INTMIS=0x%x", rd_data);

    /* Steps 44-45: Final settling wait (DV wait_on(10000) replaced with busy-wait) */
    LOGT("final settling wait 10000 cycles");
    for (volatile int d1 = 0; d1 < 10000; d1++);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("run complete: %s errors=%u",
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
