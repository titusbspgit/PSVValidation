// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

// Global variable declarations (from Meta)
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
int vcid_csi2_wrap_reg;
int gdma_path;
long int csi_ctrl_data;
int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;
extern int int_pend;
unsigned int tx_trnsfr_size;

/*
 * Function: csi2_enable_interrupt
 * Description: Clears pending interrupts by reading INT_ST_MAIN, then
 *              enables all CSI-2 host interrupt masks.
 *              Defined locally but NOT called from the main run flow.
 *              test_case() calls csi2_subsys_enable_interrupt() instead.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    // L1. Read INT_ST_MAIN to clear pending interrupts
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%08x", rd_data);

    // L2. Enable phy_fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("csi2_enable_interrupt: INT_MSK_PHY_FATAL = 0x0000000f");

    // L3. Enable pkt_fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("csi2_enable_interrupt: INT_MSK_PKT_FATAL = 0x00000003");

    // L4. Enable phy interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("csi2_enable_interrupt: INT_MSK_PHY = 0x000f000f");

    // L5. Enable line interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("csi2_enable_interrupt: INT_MSK_LINE = 0x000f000f");

    // L6. Enable boundary frame fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    // L7. Enable seq frame fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    // L8. Enable CRC frame fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    // L9. Enable payload CRC fatal interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    // L10. Enable data ID interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_DATA_ID = 0x0000ffff");

    // L11. Enable ECC corrected interrupts
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("csi2_enable_interrupt: INT_MSK_ECC_CORRECTED = 0x0000ffff");
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Enables the CSI-2 internal test pattern generator by
 *              configuring vertical resolution, horizontal resolution,
 *              pattern configuration, and PG enable registers.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void csi2_ctrlr_pg_enable(void)
{
    // Step 28a. Set PG vertical resolution to 0x10 (16 lines)
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10);
    LOGT("csi2_ctrlr_pg_enable: PPI_PG_PATTERN_VRES = 0x10");

    // Step 28b. Set PG horizontal resolution to 0x70140
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140);
    LOGT("csi2_ctrlr_pg_enable: PPI_PG_PATTERN_HRES = 0x00070140");

    // Step 28c. Set PG configuration to 0xe401
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);
    LOGT("csi2_ctrlr_pg_enable: PPI_PG_CONFIG = 0x0000e401");

    // Step 28d. Enable pattern generator
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1);
    LOGT("csi2_ctrlr_pg_enable: PPI_PG_ENABLE = 1 (Pattern Generator ENABLED)");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Initializes testcase context and global state.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator init: begin");

    // Reset global state
    data_rd = 0;
    data_wr = 0;
    def_fail_cnt = 0;
    wr_fail_cnt = 0;
    vcid_csi2_wrap_reg = 0;
    gdma_path = 0;
    csi_ctrl_data = 0;
    gdma_int_rsts = 0;
    gdma_ch0_rsts = 0;
    gdma_ch1_rsts = 0;
    tx_trnsfr_size = 0;

    LOGT("mipi_csi2_test_pattern_generator init: complete");

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Main testcase execution. Configures virtual channel routing,
 *              initializes D-PHY, programs DMA, enables pattern generator,
 *              and validates DMA transfer completion.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput result structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rd_data;
    int vcid;
    int vcid_unselected_path;
    unsigned int gdma_reg_base;
    int hres, vres, valid_bits_per_pixel;
    unsigned int csi2_data_trnsfr_size;
    unsigned int dma_ch0_pc;
    unsigned int dma_ch0_instn_preload_addr;
    int dma_dest_addr_incr_flag;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator run: begin");

    // Step 1. Set global variable int_pend = 1
    int_pend = 1;
    LOGT("Step 1: int_pend = 1");

    // Step 2. printf("start line\n")
    printf("start line\n");

    // Step 3. Set local variable vcid = 3
    vcid = 3;
    LOGT("Step 3: vcid = %d", vcid);

    // Step 4. Compute vcid_unselected_path
    vcid_unselected_path = ((vcid + 1) & 0xf);
    LOGT("Step 4: vcid_unselected_path = 0x%x", vcid_unselected_path);

    // Step 5. Conditional compilation for GDMA path selection
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid_unselected_path << 8) + (vcid_unselected_path << 4) + vcid);
    gdma_path = 3;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid_unselected_path << 8) + (vcid << 4) + (vcid_unselected_path));
    gdma_path = 2;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid << 8) + (vcid_unselected_path << 4) + (vcid_unselected_path));
    gdma_path = 1;
#else
    vcid_csi2_wrap_reg = ((vcid << 12) + (vcid_unselected_path << 8) + (vcid_unselected_path << 4) + (vcid_unselected_path));
    gdma_path = 0;
#endif

    // Step 6. Compute gdma_reg_base
    gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000);
    LOGT("Step 6: gdma_path = %d, gdma_reg_base = 0x%08x", gdma_path, gdma_reg_base);

    // Step 7. printf vcid_csi2_wrap_reg
    printf("vcid_csi2_wrap_reg=%0x\n", vcid_csi2_wrap_reg);

    // Step 8. Write CSI-2 virtual channel register
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("Step 8: write_reg(VIRTUAL_CHANNEL, 0x%08x)", vcid_csi2_wrap_reg);

    // Step 9. Disable control data transfer
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);
    LOGT("Step 9: write_reg(CONTROL_DATA, 0x0)");

    // Step 10. Enable CSI-2 and DMA interrupts (external function)
    LOGT("Step 10: Calling csi2_subsys_enable_interrupt()");
    csi2_subsys_enable_interrupt();
    LOGT("Step 10: csi2_subsys_enable_interrupt() done");

    // Step 11. D-PHY initialization (external function)
    LOGT("Step 11: Calling snps_phy_init()");
    snps_phy_init();
    LOGT("Step 11: snps_phy_init() done");

    // Step 12. Read PHY stop state register
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("Step 12: PHY_STOPSTATE initial read = 0x%08x", rd_data);

    // Step 13. Poll PHY_STOPSTATE until value equals 0x1000f
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("Step 13: PHY_STOPSTATE = 0x%08x (all lanes in stop state)", rd_data);

    // Step 14. Set hres = 320
    hres = 320;

    // Step 15. Set vres = 16
    vres = 16;

    // Step 16. Set valid_bits_per_pixel = 24
    valid_bits_per_pixel = 24;
    LOGT("Steps 14-16: hres=%d, vres=%d, bpp=%d", hres, vres, valid_bits_per_pixel);

    // Step 17. Compute DMA transfer size
    csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres) / 8) % 8) ?
        (((valid_bits_per_pixel * hres) / 8) + 8 - (((valid_bits_per_pixel * hres) / 8) % 8)) :
        ((valid_bits_per_pixel * hres) / 8)) * vres);
    LOGT("Step 17: csi2_data_trnsfr_size = %u bytes", csi2_data_trnsfr_size);

    // Step 18. Program higher order ARM DMA AR channel 0 data address bits
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100);
    LOGT("Step 18: write_reg(DMA_M0_ADDR_AR_CH0_DATA, 0x100)");

    // Step 19. Program higher order ARM DMA AR channel 0 instruction address bits
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("Step 19: write_reg(DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0)");

    // Step 20. Program higher order ARM DMA AW channel 0 data address bits
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("Step 20: write_reg(DMA_M0_ADDR_AW_CH0_DATA, 0x0)");

    // Step 21. Program higher order ARM DMA AW channel 0 instruction address bits
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("Step 21: write_reg(DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0)");

    // Step 22. Enable sending fracdiv output to CSI2 subsystem
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("Step 22: write_reg(RB_REG_BASE + 0xf4, 0x1)");

    // Step 23. Set dma_ch0_pc
    dma_ch0_pc = 0xE6000000;

    // Step 24. Set dma_ch0_instn_preload_addr = dma_ch0_pc
    dma_ch0_instn_preload_addr = dma_ch0_pc;
    LOGT("Steps 23-24: dma_ch0_pc = 0x%08x", dma_ch0_pc);

    // Step 25. Conditional compilation for FPS60
#if defined(FPS60)
    dma_dest_addr_incr_flag = 0;
#else
    dma_dest_addr_incr_flag = 1;
#endif
    LOGT("Step 25: dma_dest_addr_incr_flag = %d", dma_dest_addr_incr_flag);

    // Step 26. Preload DMA transfer instructions (external function)
    LOGT("Step 26: Calling dma_trnsfr_instn_preload_incr_addr(pc=0x%08x, base=0x%08x, src=0x00, dst=0xE6001000, size=%u, src_incr=0, dst_incr=%d, irq=0)",
         dma_ch0_pc, gdma_reg_base, csi2_data_trnsfr_size, dma_dest_addr_incr_flag);
    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000,
        csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0);
    LOGT("Step 26: dma_trnsfr_instn_preload_incr_addr() done");

    // Step 27. Issue DMAGO command for channel 0 (external function)
    LOGT("Step 27: Calling DMAGO_CSI(base=0x%08x, pc=0x%08x, ch=0)", gdma_reg_base, dma_ch0_pc);
    DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0);
    LOGT("Step 27: DMAGO_CSI() done");

    // Step 28. Enable pattern generator (internal function)
    LOGT("Step 28: Calling csi2_ctrlr_pg_enable()");
    csi2_ctrlr_pg_enable();
    LOGT("Step 28: csi2_ctrlr_pg_enable() done");

    // Step 29. Wait 100 cycles (PSV busy-wait adaptation)
    LOGT("Step 29: Waiting 100 cycles");
    for (volatile int d = 0; d < 100; d++);
    LOGT("Step 29: Wait complete");

    // Step 30. Disable pattern generator
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0);
    LOGT("Step 30: write_reg(PPI_PG_ENABLE, 0) - Pattern Generator DISABLED");

    // Step 31. Read DMA interrupt masked status register
    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    LOGT("Step 31: DMA INTMIS initial read = 0x%08x", rd_data);

    // Step 32. Poll DMA interrupt masked status until bit 0 is set
    while ((rd_data & 0x1) == 0) {
#if defined(DEBUG)
        printf("Polling dma_irq[0]=%0d\n", rd_data);
#endif
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("Step 32: DMA INTMIS = 0x%08x - DMA transfer complete", rd_data);

    // Step 33. Wait 10000 cycles (PSV busy-wait adaptation)
    LOGT("Step 33: Waiting 10000 cycles");
    for (volatile int d = 0; d < 10000; d++);
    LOGT("Step 33: Wait complete");

    // Step 34. finish(0) replaced by FV/PSV status return
    // DV: finish(0) - test pass
    out->status = 0;
    LOGT("Step 34: Test complete - PASS (out->status = 0)");

    LOGT("mipi_csi2_test_pattern_generator run: complete");

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator teardown: no additional cleanup required");

    return 0;
}
