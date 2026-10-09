// Author - AI Force 2.3. 10-Jul-2025 12:30 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * mipi_csi2_test_pattern_generator
 *
 * Test Case  : mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator (PG)
 *              functionality by configuring virtual channel, initializing D-PHY,
 *              programming DMA for data reception, enabling the pattern generator
 *              with specific vres/hres/config parameters, then disabling it and
 *              polling for DMA transfer completion to confirm successful data
 *              reception.
 */

/* Global variable from DV source */
static volatile int int_pend;

/* Testcase context structure */
typedef struct {
    unsigned int errors;
} mipi_csi2_tpg_ctx_t;

static mipi_csi2_tpg_ctx_t g_ctx;

/*
 * csi2_enable_interrupt
 * Clears pending interrupts and enables all CSI-2 interrupt masks.
 * This function is defined locally but invoked externally via
 * csi2_subsys_enable_interrupt().
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    LOGT("csi2_enable_interrupt: clearing pending interrupts and enabling masks");

    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("read INT_ST_MAIN = 0x%x", (unsigned int)rd_data);

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("write INT_MSK_PHY_FATAL = 0x0000000f");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);
    LOGT("write INT_MSK_PKT_FATAL = 0x00000003");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("write INT_MSK_PHY = 0x000f000f");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("write INT_MSK_LINE = 0x000f000f");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);
    LOGT("write INT_MSK_BNDRY_FRAME_FATAL = 0x0000ffff");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);
    LOGT("write INT_MSK_SEQ_FRAME_FATAL = 0x0000ffff");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);
    LOGT("write INT_MSK_CRC_FRAME_FATAL = 0x0000ffff");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);
    LOGT("write INT_MSK_PLD_CRC_FATAL = 0x0000ffff");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);
    LOGT("write INT_MSK_DATA_ID = 0x0000ffff");

    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
    LOGT("write INT_MSK_ECC_CORRECTED = 0x0000ffff");

    LOGT("csi2_enable_interrupt: all interrupt masks enabled");
}

/*
 * csi2_ctrlr_pg_enable
 * Configures and enables the internal test pattern generator.
 */
static void csi2_ctrlr_pg_enable(void)
{
    LOGT("csi2_ctrlr_pg_enable: configuring pattern generator");

    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, 0x10);
    LOGT("write PPI_PG_PATTERN_VRES = 0x10");

    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, 0x70140);
    LOGT("write PPI_PG_PATTERN_HRES = 0x70140");

    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, 0xe401);
    LOGT("write PPI_PG_CONFIG = 0xe401");

    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 1);
    LOGT("write PPI_PG_ENABLE = 1");

    LOGT("csi2_ctrlr_pg_enable: pattern generator enabled");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;
    g_ctx = (mipi_csi2_tpg_ctx_t){0};
    LOGT("mipi_csi2_test_pattern_generator init: testcase initialization");
    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    long long int csi2_data_trnsfr_size;
    int vcid_unselected_path, vcid;
    unsigned long int dma_ch0_pc, dma_ch0_instn_preload_addr;
    int dma_dest_addr_incr_flag;
    int valid_bits_per_pixel;
    unsigned int rd_data;
    int vres, hres;
    unsigned int vcid_csi2_wrap_reg;
    int gdma_path;
    unsigned long int gdma_reg_base;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator output pointer is NULL");
        return -1;
    }

    out->status = 0;
    LOGT("mipi_csi2_test_pattern_generator run: starting main testcase flow");

    int_pend = 1;
    LOGT("start line");

    vcid = 3;
    vcid_unselected_path = ((vcid + 1) & 0xf);
    vcid_csi2_wrap_reg = ((vcid << 12) + (vcid_unselected_path << 8) +
                          (vcid_unselected_path << 4) + (vcid_unselected_path));
    LOGT("vcid_csi2_wrap_reg=0x%x", vcid_csi2_wrap_reg);

    gdma_path = 0;
    gdma_reg_base = 0xE6A00000UL + ((unsigned long int)(gdma_path) * 0x1000UL);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("write VIRTUAL_CHANNEL = 0x%x", vcid_csi2_wrap_reg);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0);
    LOGT("write CONTROL_DATA = 0");

    csi2_subsys_enable_interrupt();
    snps_phy_init();

    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    }
    LOGT("PHY_STOPSTATE reached 0x%x", rd_data);

    hres = 320;
    vres = 16;
    valid_bits_per_pixel = 24;
    csi2_data_trnsfr_size = (((((valid_bits_per_pixel * hres) / 8) % 8) ?
        (((valid_bits_per_pixel * hres) / 8) + 8 -
         (((valid_bits_per_pixel * hres) / 8) % 8)) :
        ((valid_bits_per_pixel * hres) / 8)) * vres);
    LOGT("csi2_data_trnsfr_size = %lld bytes", csi2_data_trnsfr_size);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA, 0x100);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION, 0x0);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA, 0x0);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION, 0x0);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4, 0x1);

    dma_ch0_pc = 0xE6000000UL;
    dma_ch0_instn_preload_addr = dma_ch0_pc;
    dma_dest_addr_incr_flag = 1;

    dma_trnsfr_instn_preload_incr_addr(dma_ch0_pc, gdma_reg_base,
        0x00, 0xE6001000, csi2_data_trnsfr_size,
        0, dma_dest_addr_incr_flag, 0);

    DMAGO_CSI(gdma_reg_base, dma_ch0_pc, 0);

    csi2_ctrlr_pg_enable();

    for (volatile int d0 = 0; d0 < 100; d0++);

    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0);
    LOGT("PPI_PG_ENABLE disabled");

    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    while ((rd_data & 0x1) == 0) {
        rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("DMA ch0 transfer complete (INTMIS=0x%x)", rd_data);

    for (volatile int d1 = 0; d1 < 10000; d1++);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to FV out->status PASS reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase validation, cleanup, and final status handling.
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
