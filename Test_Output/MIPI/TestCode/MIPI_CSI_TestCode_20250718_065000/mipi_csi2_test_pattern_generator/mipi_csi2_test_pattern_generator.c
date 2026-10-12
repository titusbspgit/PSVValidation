// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

// Global variable declarations (Meta Step 1)
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
int vcid_csi2_wrap_reg;
int gdma_path;
long int csi_ctrl_data;
int gdma_int_rsts, gdma_ch0_rsts, gdma_ch1_rsts;
extern int int_pend;
unsigned int tx_trnsfr_size;

// MANUAL_REVIEW: Meta source has 'extern int_pend;' missing type specifier 'int'.
// Preserved as 'extern int int_pend;' to produce valid C.


/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Enable the CSI-2 internal test pattern generator with
 *              configured resolution and pattern settings.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void csi2_ctrlr_pg_enable(void)
{
    // Meta Step 2a
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10);
    // Meta Step 2b
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140);
    // Meta Step 2c
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);
    // Meta Step 2d
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1);
}


/*
 * Function: csi2_enable_interrupt
 * Description: Clear pending CSI-2 interrupts by reading INT_ST_MAIN and
 *              enable all relevant interrupt masks.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void csi2_enable_interrupt(void)
{
    // Meta Step 4a
    int rd_data;

    // Meta Step 4b
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);

    // Meta Step 4c
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    // Meta Step 4d
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    // Meta Step 4e
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    // Meta Step 4f
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    // Meta Step 4g
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    // Meta Step 4h
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    // Meta Step 4i
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    // Meta Step 4j
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    // Meta Step 4k
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    // Meta Step 4l
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
}


/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: FV/PSV initialization entry point. Performs early setup.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator_init: start");

    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    LOGT("mipi_csi2_test_pattern_generator_init: complete");

    return 0;
}


/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: FV/PSV main testcase entry point. Implements the complete
 *              DV test_case() flow from Meta Step 3.
 * Parameters:
 *   cfg - pointer to test configuration structure
 *   out - pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    // Meta Step 3a: Local variable declarations
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

    // Suppress unused-variable warnings for Meta-declared but unused locals
    (void)rx_desc;
    (void)tx_desc;
    (void)single_axi_trnsfr_size;
    (void)num_beats;
    (void)dmago_instn_0;
    (void)dmago_instn_1;
    (void)instn;
    (void)total_loop_cnt;
    (void)lc1_iter;
    (void)lc0_iter;

    // Meta Step 3b
    int_pend = 1;

    // Meta Step 3c
    printf("start line\n");
    LOGT("mipi_csi2_test_pattern_generator_run: start line");

    // Meta Step 3d
    vcid = 3;

    // Meta Step 3e
    vcid_unselected_path = ((vcid + 1)&0xf);

    // Meta Step 3f: Conditional compilation for GDMA path selection
#ifdef GDMA3_PATH
    vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+vcid);
    gdma_path = 3;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid_unselected_path<<8)+(vcid<<4)+(vcid_unselected_path));
    gdma_path = 2;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = ((vcid_unselected_path<<12)+(vcid<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path));
    gdma_path = 1;
#else // GDMA0_PATH
    vcid_csi2_wrap_reg = ((vcid<<12)+(vcid_unselected_path<<8)+(vcid_unselected_path<<4)+(vcid_unselected_path));
    gdma_path = 0;
#endif

    // Meta Step 3g
    gdma_reg_base = 0xE6A00000 + ((gdma_path)*0x1000);

    // Meta Step 3h
    printf("vcid_csi2_wrap_reg=%0x\n", vcid_csi2_wrap_reg);
    LOGT("vcid_csi2_wrap_reg=0x%x", vcid_csi2_wrap_reg);

    // Meta Step 3i
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    // Meta Step 3j
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);

    // Meta Step 3k
    csi2_subsys_enable_interrupt();
    LOGT("CSI-2 subsystem interrupts enabled");

    // Meta Step 3l
    snps_phy_init();
    LOGT("SNPS D-PHY initialized");

    // Meta Step 3m
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);

    // Meta Step 3n: Poll PHY_STOPSTATE until 0x1000f
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY stop state reached: rd_data=0x%x", rd_data);

    // Meta Step 3o
    hres = 320;

    // Meta Step 3p
    vres = 16;

    // Meta Step 3q
    valid_bits_per_pixel = 24;

    // Meta Step 3r: Compute DMA transfer size with 8-byte alignment
    csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres)/8 )%8)? (((valid_bits_per_pixel * hres)/8 ) + 8 - (((valid_bits_per_pixel * hres)/8 )%8)) : ((valid_bits_per_pixel * hres)/8))*vres);
    LOGT("csi2_data_trnsfr_size=%lld", csi2_data_trnsfr_size);

    // Meta Step 3s
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100);

    // Meta Step 3t
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);

    // Meta Step 3u
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);

    // Meta Step 3v
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);

    // Meta Step 3w
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);
    LOGT("Fracdiv output enabled");

    // Meta Step 3x
    dma_ch0_pc = 0xE6000000;

    // Meta Step 3y
    dma_ch0_instn_preload_addr = dma_ch0_pc;
    (void)dma_ch0_instn_preload_addr;

    // Meta Step 3z: Conditional compilation for FPS60
#ifdef FPS60
    dma_dest_addr_incr_flag = 0;
#else
    dma_dest_addr_incr_flag = 1;
#endif

    // Meta Step 3aa
    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base, 0x00, 0xE6001000, csi2_data_trnsfr_size, 0, dma_dest_addr_incr_flag, 0);
    LOGT("DMA transfer instruction preloaded");

    // Meta Step 3ab
    DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0);
    LOGT("DMAGO issued for channel 0");

    // Meta Step 3ac
    csi2_ctrlr_pg_enable();
    LOGT("Pattern generator enabled");

    // Meta Step 3ad: wait_on(100) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 100; d++);

    // Meta Step 3ae
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0);
    LOGT("Pattern generator disabled");

    // Meta Step 3af
    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

    // Meta Step 3ag: Poll DMA interrupt masked status until bit 0 set
    while ((rd_data & 0x1) == 0) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("DMA transfer complete: rd_data=0x%x", rd_data);

    // Meta Step 3ah: wait_on(10000) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 10000; d++);

    // Meta Step 3ai: finish(0) - converted to FV/PSV completion
    LOGT("Test completed successfully");
    out->status = 0;

    return out->status;
}


/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: FV/PSV teardown entry point. Final cleanup and status.
 * Parameters:
 *   cfg - pointer to test configuration structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator_teardown: no additional cleanup required");

    return 0;
}
