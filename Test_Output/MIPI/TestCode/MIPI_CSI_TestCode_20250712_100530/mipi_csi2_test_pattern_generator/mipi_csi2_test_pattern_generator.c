// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_test_pattern_generator.h"
#include "test_define.inc"

/*
 * Testcase: mipi_csi2_test_pattern_generator
 * Description: Validates the MIPI CSI-2 internal test pattern generator
 *   functionality. Configures the CSI-2 subsystem to receive data generated
 *   by the host controller's built-in pattern generator and transfers the
 *   received data via DMA.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
} mipi_csi2_test_pattern_generator_ctx_t;

static mipi_csi2_test_pattern_generator_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Reads INT_ST_MAIN to clear pending interrupts, then writes
 *   enable masks to all CSI-2 host interrupt mask registers.
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Step 10: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Step 11: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);

    /* Step 12: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);

    /* Step 13: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);

    /* Step 14: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);

    /* Step 15: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);

    /* Step 16: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);

    /* Step 17: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);

    /* Step 18: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);

    /* Step 19: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);

    /* Step 20: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);

    LOGT("csi2_enable_interrupt: All interrupt masks configured");
}

/*
 * Function: csi2_subsys_enable_interrupt
 * Description: Wrapper that calls csi2_enable_interrupt() to enable all
 *   CSI-2 subsystem interrupts.
 */
static void csi2_subsys_enable_interrupt(void)
{
    csi2_enable_interrupt();
}

/*
 * Function: csi2_ctrlr_pg_enable
 * Description: Writes pattern generator configuration registers to enable
 *   the internal test pattern generator.
 */
static void csi2_ctrlr_pg_enable(void)
{
    /* Step 37: Write vertical resolution to pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_VRES, PG_VRES);
    LOGT("csi2_ctrlr_pg_enable: PG_PATTERN_VRES = %u", PG_VRES);

    /* Step 38: Write horizontal resolution to pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_PATTERN_HRES, PG_HRES);
    LOGT("csi2_ctrlr_pg_enable: PG_PATTERN_HRES = %u", PG_HRES);

    /* Step 39-41: Write pattern generator config */
    /* MANUAL_REVIEW: PG_CONFIG value depends on data_type and vc_id encoding. */
    /* Using (DATA_TYPE | (VCID << 6)) as a reasonable encoding from DV source. */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_CONFIG, PG_CONFIG_VAL);
    LOGT("csi2_ctrlr_pg_enable: PG_CONFIG = 0x%x", PG_CONFIG_VAL);

    /* Step 42: Enable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x1U);
    LOGT("csi2_ctrlr_pg_enable: Pattern generator enabled");
}

/*
 * Function: mipi_csi2_test_pattern_generator_init
 * Description: Performs testcase initialization and pre-condition setup for
 *   mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_test_pattern_generator_ctx_t){0};

    LOGT("mipi_csi2_test_pattern_generator_init: Initialization start");

    /* Step 2: Set int_pend = 1 */
    /* int_pend is a local DV variable, used for flow control */
    unsigned int int_pend = 1U;
    (void)int_pend;

    /* Step 3: Set vcid = 3 */
    /* vcid is used to compute vcid_csi2_wrap_reg, handled via macros */

    /* Step 4: Compute vcid_unselected_path = ((vcid + 1) & 0xf) */
    /* Handled via VCID_UNSELECTED_PATH macro in test_define.inc */

    /* Step 5-6: Conditional compilation selects GDMA path */
    /* MANUAL_REVIEW: GDMA path selection is conditional compilation in DV source. */
    /* Using GDMA0_PATH default. gdma_reg_base computed in test_define.inc. */
    LOGT("mipi_csi2_test_pattern_generator_init: gdma_reg_base = 0x%lx",
         (unsigned long)GDMA_REG_BASE);

    /* Step 7: Write virtual channel register */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, VCID_CSI2_WRAP_REG);
    LOGT("mipi_csi2_test_pattern_generator_init: VIRTUAL_CHANNEL = 0x%x",
         VCID_CSI2_WRAP_REG);

    /* Step 8: Disable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0U);
    LOGT("mipi_csi2_test_pattern_generator_init: CONTROL_DATA = 0 (disabled)");

    /* Step 9: Enable CSI-2 subsystem interrupts */
    csi2_subsys_enable_interrupt();

    /* Step 22: D-PHY initialization */
    snps_phy_init();
    LOGT("mipi_csi2_test_pattern_generator_init: snps_phy_init() called");

    /* Steps 23-24: Poll PHY_STOPSTATE until 0x1000f */
    {
        unsigned int rd_data;
        unsigned int timeout = PHY_STOPSTATE_TIMEOUT;

        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        LOGT("mipi_csi2_test_pattern_generator_init: PHY_STOPSTATE initial = 0x%x",
             rd_data);

        while (rd_data != 0x1000fU) {
            timeout--;
            if (timeout == 0U) {
                LOGE("mipi_csi2_test_pattern_generator_init: PHY_STOPSTATE timeout, rd_data=0x%x",
                     rd_data);
                g_ctx.errors++;
                return -1;
            }
            rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        }
        LOGT("mipi_csi2_test_pattern_generator_init: PHY_STOPSTATE = 0x%x (stop-state confirmed)",
             rd_data);
    }

    LOGT("mipi_csi2_test_pattern_generator_init: Initialization complete");
    return 0;
}

/*
 * Function: mipi_csi2_test_pattern_generator_run
 * Description: Executes the main testcase flow for
 *   mipi_csi2_test_pattern_generator. Programs DMA address registers, enables
 *   fracdiv output, configures and starts DMA, enables/disables pattern
 *   generator, and polls DMA completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rd_data;
    unsigned int timeout;
    unsigned int csi2_data_trnsfr_size;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_test_pattern_generator_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_test_pattern_generator_run: Starting test pattern generator flow");

    /* Steps 25-26: Compute csi2_data_trnsfr_size */
    /* total bytes = hres * vres * (valid_bits_per_pixel / 8), aligned to 8 bytes */
    {
        unsigned int total_bytes = PG_HRES * PG_VRES * (VALID_BITS_PER_PIXEL / 8U);
        csi2_data_trnsfr_size = (total_bytes + 7U) & ~7U;
    }
    LOGT("mipi_csi2_test_pattern_generator_run: csi2_data_trnsfr_size = %u",
         csi2_data_trnsfr_size);

    /* Steps 27-30: Program DMA address mapping registers */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_DATA,
              GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("mipi_csi2_test_pattern_generator_run: DMA_M0_ADDR_AR_CH0_DATA = 0x%x",
         GDMA_CSI2_DATA_DEST_ADDR2);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AR_CH0_INSTRUCTION,
              GDMA_CTRL_DATA_DEST_ADDR2);
    LOGT("mipi_csi2_test_pattern_generator_run: DMA_M0_ADDR_AR_CH0_INSTRUCTION = 0x%x",
         GDMA_CTRL_DATA_DEST_ADDR2);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_DATA,
              GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("mipi_csi2_test_pattern_generator_run: DMA_M0_ADDR_AW_CH0_DATA = 0x%x",
         GDMA_CSI2_DATA_DEST_ADDR2);

    write_reg(MIZAR_MIPI_CSI2_RB_REG_DMA_M0_ADDR_AW_CH0_INSTRUCTION,
              GDMA_CTRL_DATA_DEST_ADDR2);
    LOGT("mipi_csi2_test_pattern_generator_run: DMA_M0_ADDR_AW_CH0_INSTRUCTION = 0x%x",
         GDMA_CTRL_DATA_DEST_ADDR2);

    /* Step 31: Enable sending fracdiv output to CSI-2 subsystem */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_BASE + 0xf4U, 0x1U);
    LOGT("mipi_csi2_test_pattern_generator_run: fracdiv output enabled (RB_REG_BASE+0xf4 = 0x1)");

    /* Steps 32-36: Configure and start DMA channel 0 */
    /* Enable DMA interrupts */
    write_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTEN_OFFSET, 0x1U);

    /* DMA channel 0: preload transfer instruction for pattern data */
    dma_trnsfr_instn_preload(
        /*ch_num*/ 0,
        /*src_addr*/ 0x0000U,
        /*dest_addr*/ GDMA_CSI2_DATA_DEST_ADDR2,
        /*trnsfr_size*/ csi2_data_trnsfr_size,
        /*irq_num*/ 0
    );

    /* Start DMA channel 0 */
    DMAGO_CSI(/*ch_num*/ 0);
    LOGT("mipi_csi2_test_pattern_generator_run: DMA channel 0 started");

    /* Steps 37-42: Enable pattern generator */
    csi2_ctrlr_pg_enable();

    /* Step 43: wait_on(100) replaced with busy-wait loop */
    for (volatile int d0 = 0; d0 < 100; d0++);
    LOGT("mipi_csi2_test_pattern_generator_run: wait_on(100) delay complete");

    /* Step 44: Disable pattern generator */
    write_reg(MIZAR_MIPI_CSI2_HOST_PPI_PG_ENABLE, 0x0U);
    LOGT("mipi_csi2_test_pattern_generator_run: Pattern generator disabled");

    /* Steps 45-46: Poll DMA channel 0 completion: INTMIS bit 0 */
    timeout = DMA_POLL_TIMEOUT;
    rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
    while ((rd_data & 0x1U) == 0x0U) {
        timeout--;
        if (timeout == 0U) {
            LOGE("mipi_csi2_test_pattern_generator_run: DMA ch0 completion timeout");
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
        rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
    }
    LOGT("mipi_csi2_test_pattern_generator_run: DMA ch0 transfer complete, INTMIS=0x%x",
         rd_data);

    /* Clear DMA channel 0 interrupt */
    write_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

    /* Step 47: wait_on(10000) replaced with busy-wait loop */
    for (volatile int d1 = 0; d1 < 10000; d1++);
    LOGT("mipi_csi2_test_pattern_generator_run: wait_on(10000) delay complete");

    /* Step 48: DV finish(0) converted to PSV/FV status reporting */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_test_pattern_generator_run: Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_test_pattern_generator_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *   for mipi_csi2_test_pattern_generator.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_test_pattern_generator_teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
