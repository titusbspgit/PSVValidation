// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * mipi_csi2_dphy_lanes_test
 * Validates MIPI CSI-2 DPHY lane configuration by iterating through all
 * supported lane counts (4 down to 1). Enables CSI-2 interrupts, configures
 * virtual channel and control data, initializes D-PHY, polls PHY stop state,
 * and for each lane count performs DMA-based control and data packet transfers.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * csi2_enable_interrupt
 * Reads INT_ST_MAIN to clear pending interrupts, then writes all
 * interrupt mask registers to enable CSI-2 interrupts.
 */
static void csi2_enable_interrupt(void)
{
    /* Step 2: Read INT_ST_MAIN to clear pending interrupts */
    (void)read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("CSI2: INT_ST_MAIN read to clear pending interrupts");

    /* Step 3: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);

    /* Step 4: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);

    /* Step 5: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);

    /* Step 6: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);

    /* Step 7: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);

    /* Step 8: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);

    /* Step 9: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);

    /* Step 10: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);

    /* Step 11: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);

    /* Step 12: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);

    LOGT("CSI2: All interrupt masks configured");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test init: starting DPHY lanes test");

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rd_data;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int gdma_path;
    unsigned int gdma_reg_base;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    unsigned int csi_ctrl_data;
    unsigned int word_count;
    unsigned int csi_data_size;
    unsigned int cntrl_pkt_cnt;
    int lane_num;
    unsigned int i;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test run: begin");

    /* Step 1: Call csi2_enable_interrupt() */
    csi2_enable_interrupt();

    /* Step 14: Compute vcid_csi2_wrap_reg based on compile-time GDMA path define */
#if defined(GDMA3_PATH)
    vcid_csi2_wrap_reg = VC_ID;
    gdma_path = 3;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 4);
    gdma_path = 2;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 8);
    gdma_path = 1;
#else /* GDMA0_PATH */
    vcid_csi2_wrap_reg = (VC_ID << 12);
    gdma_path = 0;
#endif
    (void)gdma_path;
    LOGT("CSI2: vcid_csi2_wrap_reg=0x%x gdma_path=%u", vcid_csi2_wrap_reg, gdma_path);

    /* Step 15: Set gdma_reg_base */
    gdma_reg_base = 0xE6A00000U;
    LOGT("CSI2: gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 16: Configure virtual channel */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("CSI2: Virtual channel configured with 0x%x", vcid_csi2_wrap_reg);

    /* Step 17: Enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);
    LOGT("CSI2: Control data transfer enabled");

    /* Step 18: Repeated write to virtual channel */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    /* Step 19: Repeated write to control data */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);

    /* Step 20: Initialize the D-PHY */
    snps_phy_init();
    LOGT("CSI2: D-PHY initialized via snps_phy_init()");

    /* Step 21: Read PHY stop state */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);

    /* Step 22: Poll until PHY enters stop state (value equals 0x1000f) */
    timeout = PHY_STOPSTATE_TIMEOUT;
    while (rd_data != 0x1000fU) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout == 0U) {
            LOGE("CSI2: PHY_STOPSTATE poll timeout, rd_data=0x%x", rd_data);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("CSI2: PHY entered stop state, rd_data=0x%x", rd_data);

    /* Step 23: Set DMA program counter addresses */
    ch0_pc = 0xE6000000U;
    ch1_pc = 0xE6000500U;

    /* Step 24: Outer loop - iterate lane_num from 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        /* Step 25: Write lane count to N_LANES register */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("CSI2: N_LANES set to %d (lane count = %d)", lane_num, lane_num + 1);

        /* Step 26: Compute control packet count */
        cntrl_pkt_cnt = (VRES * 3U) + 2U;

        /* Step 27: Trigger CSI-2 sequence for current lane count */
        write_reg(0xa0243ffcU, (unsigned int)(lane_num + 1));
        LOGT("CSI2: Trigger written to 0xa0243ffc with value %d", lane_num + 1);

        /* Step 28: Inner loop - iterate through all packets */
        for (i = 0U; i < cntrl_pkt_cnt; i++) {

            /* Step 29: Set preload locations */
            ch0_preload_loc = ch0_pc;
            ch1_preload_loc = ch1_pc;

            /* Step 30: Enable DMA IRQ[1] and IRQ[0] */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);

            /* Step 31: Preload control data DMA transfer instructions for channel 0 */
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000U, 0xE6001000U, 8U, 0U);

            /* Step 32: Start DMA channel 0 */
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0U);

            /* Step 33-34: Poll DMA interrupt status for channel 0 completion */
            rd_data = 0U;
            timeout = DMA_POLL_TIMEOUT;
            while ((rd_data & 0x1U) == 0x0U) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
                if (timeout == 0U) {
                    LOGE("CSI2: DMA ch0 poll timeout, lane_num=%d pkt=%u", lane_num, i);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }
            }

            /* Step 35: Clear DMA IRQ for channel 0 */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Step 36: Read DMA status */
            rd_data = read_reg(gdma_reg_base + 0x28U);

            /* Step 37: Read the transferred control data */
            csi_ctrl_data = read_reg(0xE6001000U);

            /* Step 38: Check if data type indicates a long packet */
            if ((csi_ctrl_data & 0x3fU) > 0xfU) {

                /* Step 39: Extract word_count from bits [21:6] */
                word_count = (csi_ctrl_data >> 6) & 0xffffU;

                /* Step 40: Compute csi_data_size aligned to 8 bytes */
                if ((word_count % 8U) != 0U) {
                    csi_data_size = ((word_count / 8U) + 1U) * 8U;
                } else {
                    csi_data_size = word_count;
                }

                /* Step 41: Preload data DMA transfer instructions for channel 1 */
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000U, 0xE6002000U, csi_data_size, 1U);

                /* Step 42: Start DMA channel 1 */
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1U);

                /* Step 43-44: Poll DMA interrupt status for channel 1 completion */
                rd_data = 0U;
                timeout = DMA_POLL_TIMEOUT;
                while ((rd_data & 0x2U) == 0x0U) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                    if (timeout == 0U) {
                        LOGE("CSI2: DMA ch1 poll timeout, lane_num=%d pkt=%u", lane_num, i);
                        g_ctx.errors++;
                        out->status = -1;
                        return out->status;
                    }
                }

                /* Step 45: Additional read after poll exit */
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Step 46: Clear DMA IRQ for channel 1 */
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);

            } /* Step 47: End of long packet handling */

        } /* Step 48: End of inner loop */

        LOGT("CSI2: Lane %d processing complete", lane_num + 1);

    } /* Step 49: End of outer loop */

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV completion is handled via out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
