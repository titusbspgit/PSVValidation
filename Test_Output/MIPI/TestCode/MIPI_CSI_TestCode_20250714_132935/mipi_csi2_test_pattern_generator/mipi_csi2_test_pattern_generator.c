// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

// Meta Step 1: Global variable declarations
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
int vcid_csi2_wrap_reg;
int gdma_path;
long int csi_ctrl_data;
int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;
extern int int_pend; // MANUAL_REVIEW: Meta has 'extern int_pend' missing type specifier; preserved as 'extern int int_pend'
unsigned int tx_trnsfr_size;

// Testcase error tracking (FV status path)
static unsigned int g_errors = 0;

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Configures and enables the MIPI CSI-2 internal test pattern
 *              generator with vertical resolution 16, horizontal resolution
 *              320 pixels (24bpp), and enables the pattern output.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void csi2_ctrlr_pg_enable(void)
{
    // Step 2a: Set pattern generator vertical resolution to 16 lines
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10);
    LOGT("csi2_ctrlr_pg_enable: write_reg(PPI_PG_PATTERN_VRES, 0x10)");

    // Step 2b: Set pattern generator horizontal resolution (320 pixels, 24bpp)
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140);
    LOGT("csi2_ctrlr_pg_enable: write_reg(PPI_PG_PATTERN_HRES, 0x70140)");

    // Step 2c: Configure pattern generator settings
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);
    LOGT("csi2_ctrlr_pg_enable: write_reg(PPI_PG_CONFIG, 0xe401)");

    // Step 2d: Enable the pattern generator
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1);
    LOGT("csi2_ctrlr_pg_enable: write_reg(PPI_PG_ENABLE, 1) - Pattern Generator ENABLED");
}

/*
 * Function: csi2_enable_interrupt
 * Description: Clears pending CSI-2 interrupts by reading INT_ST_MAIN, then
 *              enables all relevant CSI-2 interrupt masks.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void csi2_enable_interrupt(void)
{
    // Step 4a: Local variable declaration
    int rd_data;

    // Step 4b: Read INT_ST_MAIN to clear any pending interrupts
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: read_reg(INT_ST_MAIN) = 0x%08x (clear pending)", rd_data);

    // Step 4c: Enable PHY fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_PHY_FATAL, 0x0000000f)");

    // Step 4d: Enable packet fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_PKT_FATAL, 0x00000003)");

    // Step 4e: Enable PHY interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_PHY, 0x000f000f)");

    // Step 4f: Enable line interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_LINE, 0x000f000f)");

    // Step 4g: Enable boundary frame fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff)");

    // Step 4h: Enable sequence frame fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff)");

    // Step 4i: Enable CRC frame fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_CRC_FRAME_FATAL, 0x0000ffff)");

    // Step 4j: Enable payload CRC fatal interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_PLD_CRC_FATAL, 0x0000ffff)");

    // Step 4k: Enable data ID interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_DATA_ID, 0x0000ffff)");

    // Step 4l: Enable ECC corrected interrupt mask
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("csi2_enable_interrupt: write_reg(INT_MSK_ECC_CORRECTED, 0x0000ffff)");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: FV/PSV initialization entry point. Performs initial setup
 *              and preconditions for the MIPI CSI-2 test pattern generator test.
 * Parameters:
 *   cfg - Pointer to the test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    g_errors = 0;
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    LOGT("mipi_csi2_test_pattern_generator_init: Initialization complete");

    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: FV/PSV main run entry point. Executes the complete MIPI CSI-2
 *              test pattern generator testcase flow derived from Meta test_case().
 * Parameters:
 *   cfg - Pointer to the test configuration item
 *   out - Pointer to the test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    // Step 3a: Local variable declarations
    long long int rx_desc, tx_desc;
    long long int csi2_data_trnsfr_size, single_axi_trnsfr_size;
    int vcid_unselected_path, vcid;
    int num_beats;
    unsigned long int dma_ch0_pc, dma_ch0_instn_preload_addr;
    unsigned int dmago_instn_0, dmago_instn_1, instn;
    int total_loop_cnt, lc1_iter, lc0_iter;
    int dma_dest_addr_incr_flag;
    int valid_bits_per_pixel;
    unsigned int rd_data;
    int vres, hres;
    volatile unsigned int timeout_cnt;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator_run: Starting test execution");

    // Step 3b: Set interrupt pending flag
    int_pend = 1;
    LOGT("run: int_pend = 1");

    // Step 3c: Print start line
    printf("start line\n");

    // Step 3d: Set virtual channel ID
    vcid = 3;
    LOGT("run: vcid = %d", vcid);

    // Step 3e: Compute unselected path value
    vcid_unselected_path = ((vcid + 1) & 0xf);
    LOGT("run: vcid_unselected_path = 0x%x", vcid_unselected_path);

    // Step 3f: Conditional compilation for GDMA path selection
#ifdef GDMA3_PATH
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid_unselected_path << 8) + (vcid_unselected_path << 4) + vcid);
    gdma_path = 3;
    LOGT("run: GDMA3_PATH selected, gdma_path = 3");
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid_unselected_path << 8) + (vcid << 4) + (vcid_unselected_path));
    gdma_path = 2;
    LOGT("run: GDMA2_PATH selected, gdma_path = 2");
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path << 12) + (vcid << 8) + (vcid_unselected_path << 4) + (vcid_unselected_path));
    gdma_path = 1;
    LOGT("run: GDMA1_PATH selected, gdma_path = 1");
#else
    // GDMA0_PATH (default)
    vcid_csi2_wrap_reg = ((vcid << 12) + (vcid_unselected_path << 8) + (vcid_unselected_path << 4) + (vcid_unselected_path));
    gdma_path = 0;
    LOGT("run: GDMA0_PATH selected (default), gdma_path = 0");
#endif

    // Step 3g: Compute gdma_reg_base address from GDMA path index
    gdma_reg_base = 0xE6A00000 + ((gdma_path) * 0x1000);
    LOGT("run: gdma_reg_base = 0x%08lx", gdma_reg_base);

    // Step 3h: Print virtual channel wrapper register value
    printf("vcid_csi2_wrap_reg=%0x\n", vcid_csi2_wrap_reg);

    // Step 3i: Write virtual channel wrapper register value
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("run: write_reg(VIRTUAL_CHANNEL, 0x%08x)", vcid_csi2_wrap_reg);

    // Step 3j: Disable control data transfer
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);
    LOGT("run: write_reg(CONTROL_DATA, 0x0) - Control data transfer disabled");

    // Step 3k: Enable CSI-2 subsystem interrupts
    LOGT("run: Calling csi2_subsys_enable_interrupt()");
    csi2_subsys_enable_interrupt();
    LOGT("run: CSI-2 subsystem interrupts enabled");

    // Step 3l: Initialize SNPS D-PHY
    LOGT("run: Calling snps_phy_init()");
    snps_phy_init();
    LOGT("run: SNPS D-PHY initialization complete");

    // Step 3m: Read PHY_STOPSTATE register
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("run: read_reg(PHY_STOPSTATE) = 0x%08x", rd_data);

    // Step 3n: Poll PHY_STOPSTATE until D-PHY enters stop state (0x1000f)
    LOGT("run: Polling PHY_STOPSTATE for 0x1000f");
    timeout_cnt = 0;
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout_cnt++;
        if (timeout_cnt > 100000U) {
            LOGE("run: TIMEOUT polling PHY_STOPSTATE for 0x1000f, last rd_data=0x%08x", rd_data);
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("run: PHY_STOPSTATE = 0x%08x - D-PHY in stop state", rd_data);

    // Step 3o: Set horizontal resolution
    hres = 320;
    LOGT("run: hres = %d", hres);

    // Step 3p: Set vertical resolution
    vres = 16;
    LOGT("run: vres = %d", vres);

    // Step 3q: Set valid bits per pixel
    valid_bits_per_pixel = 24;
    LOGT("run: valid_bits_per_pixel = %d", valid_bits_per_pixel);

    // Step 3r: Compute DMA transfer size with 8-byte alignment
    csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres) / 8) % 8) ? (((valid_bits_per_pixel * hres) / 8) + 8 - (((valid_bits_per_pixel * hres) / 8) % 8)) : ((valid_bits_per_pixel * hres) / 8)) * vres);
    LOGT("run: csi2_data_trnsfr_size = 0x%llx", csi2_data_trnsfr_size);

    // Step 3s: Program DMA higher-order address register - AR CH0 Data
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100);
    LOGT("run: write_reg(DMA_M0_ADDR_AR_CH0_DATA, 0x100)");

    // Step 3t: Program DMA higher-order address register - AR CH0 Instruction
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    LOGT("run: write_reg(DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0)");

    // Step 3u: Program DMA higher-order address register - AW CH0 Data
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    LOGT("run: write_reg(DMA_M0_ADDR_AW_CH0_DATA, 0x0)");

    // Step 3v: Program DMA higher-order address register - AW CH0 Instruction
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    LOGT("run: write_reg(DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0)");

    // Step 3w: Enable fracdiv output to CSI-2 subsystem
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("run: write_reg(BASE+0xf4, 0x1) - Fracdiv output enabled");

    // Step 3x: Set DMA channel 0 program counter
    dma_ch0_pc = 0xE6000000;
    LOGT("run: dma_ch0_pc = 0x%08lx", dma_ch0_pc);

    // Step 3y: Set DMA channel 0 instruction preload address
    dma_ch0_instn_preload_addr = dma_ch0_pc;
    LOGT("run: dma_ch0_instn_preload_addr = 0x%08lx", dma_ch0_instn_preload_addr);

    // Step 3z: Conditional compilation for FPS60 - DMA dest addr increment flag
#ifdef FPS60
    dma_dest_addr_incr_flag = 0;
    LOGT("run: FPS60 defined: dma_dest_addr_incr_flag = 0");
#else
    dma_dest_addr_incr_flag = 1;
    LOGT("run: FPS60 not defined: dma_dest_addr_incr_flag = 1");
#endif

    // Step 3aa: Configure DMA transfer instruction preload with addresses and size
    LOGT("run: Calling dma_trnsfr_instn_preload_incr_addr()");
    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0);
    LOGT("run: DMA transfer instruction preload configured");

    // Step 3ab: Issue DMAGO command for channel 0
    LOGT("run: Issuing DMAGO_CSI()");
    DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0);
    LOGT("run: DMAGO command issued for channel 0");

    // Step 3ac: Enable the pattern generator
    LOGT("run: Calling csi2_ctrlr_pg_enable()");
    csi2_ctrlr_pg_enable();
    LOGT("run: Pattern generator enabled");

    // Step 3ad: Wait 100 cycles (DV wait_on(100) adapted to busy-wait)
    LOGT("run: Waiting 100 cycles");
    for (volatile int d = 0; d < 100; d++);
    LOGT("run: Wait 100 cycles complete");

    // Step 3ae: Disable the pattern generator
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0);
    LOGT("run: write_reg(PPI_PG_ENABLE, 0) - Pattern Generator DISABLED");

    // Step 3af: Read DMA interrupt masked status register
    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    LOGT("run: read_reg(DMA_INTMIS) = 0x%08x", rd_data);

    // Step 3ag: Poll DMA interrupt masked status until bit 0 is set (transfer complete)
    LOGT("run: Polling DMA INTMIS for bit 0 (transfer complete)");
    timeout_cnt = 0;
    while ((rd_data & 0x1) == 0) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
        timeout_cnt++;
        if (timeout_cnt > 100000U) {
            LOGE("run: TIMEOUT polling DMA INTMIS for bit 0, last rd_data=0x%08x", rd_data);
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("run: DMA INTMIS bit 0 set = 0x%08x - DMA transfer complete", rd_data);

    // Step 3ah: Wait 10000 cycles (DV wait_on(10000) adapted to busy-wait)
    LOGT("run: Waiting 10000 cycles");
    for (volatile int d = 0; d < 10000; d++);
    LOGT("run: Wait 10000 cycles complete");

    // Step 3ai: DV finish(0) converted to FV/PSV completion
    LOGT("run: Test execution complete (DV finish(0) converted to FV status)");

    out->status = (g_errors == 0U) ? 0 : -1;

    LOGT("run: %s errors=%u", (out->status == 0) ? "PASS" : "FAIL", g_errors);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: FV/PSV teardown entry point. Final cleanup and status reporting.
 * Parameters:
 *   cfg - Pointer to the test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator_teardown: no additional cleanup required");
    return g_errors == 0U ? 0 : -1;
}
