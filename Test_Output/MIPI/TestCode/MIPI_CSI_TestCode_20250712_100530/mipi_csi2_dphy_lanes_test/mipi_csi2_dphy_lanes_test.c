// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Testcase: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *   through lane counts from 4 lanes down to 1 lane. Enables all CSI-2 host
 *   interrupts, configures virtual channel, initializes D-PHY, polls
 *   PHY_STOPSTATE, and performs DMA-based control and data packet transfers
 *   for each lane configuration.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Reads INT_ST_MAIN to clear pending interrupts, then writes
 *   enable masks to all CSI-2 host interrupt mask registers.
 */
static void csi2_enable_interrupt(void)
{
    unsigned int rd_data;

    /* Step 3: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    /* Step 4: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);

    /* Step 5: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);

    /* Step 6: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);

    /* Step 7: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);

    /* Step 8: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);

    /* Step 9: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);

    /* Step 10: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);

    /* Step 11: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);

    /* Step 12: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);

    /* Step 13: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);

    LOGT("csi2_enable_interrupt: All interrupt masks configured");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *   mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test_init: Initialization start");

    /* Step 2: Enable CSI-2 host interrupts */
    csi2_enable_interrupt();

    /* Step 15-16: Select GDMA path and set gdma_reg_base */
    /* MANUAL_REVIEW: GDMA path selection is conditional compilation in DV source. */
    /* Using GDMA0_PATH default. Adjust vcid_csi2_wrap_reg for target GDMA path. */

    LOGT("mipi_csi2_dphy_lanes_test_init: gdma_reg_base = 0x%lx",
         (unsigned long)GDMA_REG_BASE);

    /* Step 17: Write virtual channel register (first write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, VCID_CSI2_WRAP_REG);
    LOGT("mipi_csi2_dphy_lanes_test_init: VIRTUAL_CHANNEL = 0x%x", VCID_CSI2_WRAP_REG);

    /* Step 18: Enable control data transfer (first write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);

    /* Step 19: Write virtual channel register (second write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, VCID_CSI2_WRAP_REG);

    /* Step 20: Enable control data transfer (second write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);

    /* Step 21: D-PHY initialization */
    snps_phy_init();
    LOGT("mipi_csi2_dphy_lanes_test_init: snps_phy_init() called");

    /* Step 22-23: Poll PHY_STOPSTATE until 0x1000f */
    {
        unsigned int rd_data;
        unsigned int timeout = PHY_STOPSTATE_TIMEOUT;

        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        LOGT("mipi_csi2_dphy_lanes_test_init: PHY_STOPSTATE initial = 0x%x", rd_data);

        while (rd_data != 0x1000fU) {
            timeout--;
            if (timeout == 0U) {
                LOGE("mipi_csi2_dphy_lanes_test_init: PHY_STOPSTATE timeout, rd_data=0x%x", rd_data);
                g_ctx.errors++;
                return -1;
            }
            rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        }
        LOGT("mipi_csi2_dphy_lanes_test_init: PHY_STOPSTATE = 0x%x (stop-state confirmed)", rd_data);
    }

    LOGT("mipi_csi2_dphy_lanes_test_init: Initialization complete");
    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 *   Iterates lane_num from 3 down to 0, configures N_LANES, triggers CSI-2
 *   sequence, and performs DMA-based control/data packet transfers.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    int lane_num;
    unsigned int cntrl_pkt_cnt;
    unsigned int pkt;
    unsigned int rd_data;
    unsigned int csi_ctrl_data;
    unsigned int word_count;
    unsigned int csi_data_size;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: Starting lane iteration");

    /* Step 24: Set DMA program counter addresses */
    /* ch0_pc = 0xE6000000, ch1_pc = 0xE6000500 */
    /* These are used internally by dma_trnsfr_instn_preload / DMAGO_CSI */

    /* Step 25: Outer for loop - lane_num from 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        /* Step 26: Write lane count to N_LANES */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("mipi_csi2_dphy_lanes_test_run: N_LANES = %d (lane_count = %d)",
             lane_num, lane_num + 1);

        /* Step 27: Compute control packet count */
        cntrl_pkt_cnt = (VRES * 3U) + 2U;

        /* Step 28: Trigger CSI-2 sequence */
        write_reg(0xa0243ffcU, (unsigned int)(lane_num + 1));
        LOGT("mipi_csi2_dphy_lanes_test_run: Trigger CSI-2 seq, wrote %d to 0xa0243ffc",
             lane_num + 1);

        /* Steps 29-51: Inner loop for DMA transfers and completion polling */
        for (pkt = 0U; pkt < cntrl_pkt_cnt; pkt++) {

            /* Enable DMA interrupts for channels 0 and 1 */
            write_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);

            /* DMA channel 0: preload transfer instruction for control data */
            dma_trnsfr_instn_preload(
                /*ch_num*/ 0,
                /*src_addr*/ 0x8000U,
                /*dest_addr*/ 0xE6001000U,
                /*trnsfr_size*/ 8U,
                /*irq_num*/ 0
            );

            /* Start DMA channel 0 */
            DMAGO_CSI(/*ch_num*/ 0);

            /* Poll DMA channel 0 completion: INTMIS bit 0 */
            timeout = DMA_POLL_TIMEOUT;
            rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
            while ((rd_data & 0x1U) == 0x0U) {
                timeout--;
                if (timeout == 0U) {
                    LOGE("mipi_csi2_dphy_lanes_test_run: DMA ch0 timeout, lane=%d pkt=%u",
                         lane_num, pkt);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }
                rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
            }

            /* Clear DMA channel 0 interrupt */
            write_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Read debug status */
            rd_data = read_reg(GDMA_REG_BASE + 0x28U);
            LOGT("mipi_csi2_dphy_lanes_test_run: DMA debug status = 0x%x", rd_data);

            /* Read CSI control data from destination */
            csi_ctrl_data = read_reg(0xE6001000U);
            LOGT("mipi_csi2_dphy_lanes_test_run: csi_ctrl_data = 0x%x", csi_ctrl_data);

            /* Long packet detection: (csi_ctrl_data & 0x3f) > 0xf */
            if ((csi_ctrl_data & 0x3fU) > 0xfU) {

                /* Extract word_count from bits [21:6] */
                word_count = (csi_ctrl_data >> 6) & 0xFFFFU;

                /* Calculate csi_data_size aligned to 8 bytes */
                csi_data_size = (word_count + 7U) & ~7U;
                LOGT("mipi_csi2_dphy_lanes_test_run: Long pkt: word_count=%u csi_data_size=%u",
                     word_count, csi_data_size);

                /* DMA channel 1: preload transfer instruction for CSI data */
                dma_trnsfr_instn_preload(
                    /*ch_num*/ 1,
                    /*src_addr*/ 0x0000U,
                    /*dest_addr*/ 0xE6002000U,
                    /*trnsfr_size*/ csi_data_size,
                    /*irq_num*/ 1
                );

                /* Start DMA channel 1 */
                DMAGO_CSI(/*ch_num*/ 1);

                /* Poll DMA channel 1 completion: INTMIS bit 1 */
                timeout = DMA_POLL_TIMEOUT;
                rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
                while ((rd_data & 0x2U) == 0x0U) {
                    timeout--;
                    if (timeout == 0U) {
                        LOGE("mipi_csi2_dphy_lanes_test_run: DMA ch1 timeout, lane=%d pkt=%u",
                             lane_num, pkt);
                        g_ctx.errors++;
                        out->status = -1;
                        return out->status;
                    }
                    rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);
                }

                /* Read INTMIS again (as per DV source) */
                rd_data = read_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Clear DMA channel 1 interrupt */
                write_reg(GDMA_REG_BASE + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);
            }
        }

        LOGT("mipi_csi2_dphy_lanes_test_run: Lane %d complete (%u packets)",
             lane_num, cntrl_pkt_cnt);
    }

    /* DV finish(0) converted to PSV/FV status reporting */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test_run: Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *   for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test_teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
