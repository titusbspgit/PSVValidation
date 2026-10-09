// Author - AI Force 2.3. 09-Jun-2025 21:10 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *   through lane counts from 4 lanes down to 1 lane. For each lane
 *   configuration, the test performs a complete CSI-2 data reception sequence
 *   including control packet DMA transfers and data packet DMA transfers.
 */

/* Forward declaration of helper function */
static void csi2_enable_interrupt(void);

/*
 * Function: csi2_enable_interrupt
 * Description: Enables all CSI-2 host interrupt masks by reading INT_ST_MAIN
 *   to clear pending interrupts, then writing enable values to all 10
 *   interrupt mask registers.
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    /* Step 6: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read to clear = 0x%x", rd_data);

    /* Step 7: Enable phy_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);

    /* Step 8: Enable pkt_fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);

    /* Step 9: Enable phy interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);

    /* Step 10: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);

    /* Step 11: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);

    /* Step 12: Enable seq frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);

    /* Step 13: Enable crc frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);

    /* Step 14: Enable pld crc fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);

    /* Step 15: Enable data_id interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);

    /* Step 16: Enable ecc corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);

    LOGT("csi2_enable_interrupt: all interrupt masks enabled");
}

static unsigned int g_errors;

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

    g_errors = 0U;

    LOGT("mipi_csi2_dphy_lanes_test_init: initialization start");

    /* Step 3: Print start line */
    LOGT("start line");

    /* Step 4: Enable all CSI-2 host interrupt masks */
    csi2_enable_interrupt();

    LOGT("mipi_csi2_dphy_lanes_test_init: initialization complete");

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
    int rd_data;
    int vcid_csi2_wrap_reg;
    int gdma_path;
    unsigned int gdma_reg_base;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    int cntrl_pkt_cnt;
    int csi_ctrl_data;
    int word_count;
    long long int csi_data_size;
    int lane_num;
    int i;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: execution start");

    /* Step 18: Conditional compilation - set vcid_csi2_wrap_reg based on GDMA path */
#ifdef GDMA0_PATH
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
#else
    /* Default GDMA0_PATH */
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
#endif

    /* Step 19: Set GDMA register base */
    gdma_reg_base = 0xE6A00000;

    /* Step 20: Write virtual channel register (first occurrence) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    /* Step 21: Enable control data transfer (first occurrence) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);

    /* Step 22: Debug print vcid_csi2_wrap_reg */
    LOGT("vcid_csi2_wrap_reg=0x%x", vcid_csi2_wrap_reg);

    /* Step 23: Write virtual channel register (second occurrence) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    /* Step 24: Enable control data transfer (second occurrence) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);

    /* Step 25: D-PHY initialization sequence (external function) */
    snps_phy_init();

    /* Step 26: First read of PHY_STOPSTATE */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("PHY_STOPSTATE initial read = 0x%x", rd_data);

    /* Step 27: Poll PHY_STOPSTATE until value equals 0x1000f */
    timeout = PSV_POLL_TIMEOUT;
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout <= 0) {
            LOGE("PHY_STOPSTATE poll timeout, rd_data=0x%x", rd_data);
            g_errors++;
            out->status = -1;
            return out->status;
        }
    }
    LOGT("PHY_STOPSTATE reached expected value 0x1000f");

    /* Step 28: Set ch0_pc */
    ch0_pc = 0xE6000000;

    /* Step 29: Set ch1_pc */
    ch1_pc = 0xE6000500;

    /* Step 30: Begin outer for loop - lane_num = 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        /* Step 31: Configure number of lanes in CSI-2 controller */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);
        LOGT("Configured N_LANES = %d", lane_num);

        /* Step 32: Compute cntrl_pkt_cnt */
        cntrl_pkt_cnt = ((VRES * 3) + 2);

        /* Step 33: Debug print cntrl_pkt_cnt */
        LOGT("DEBUG: cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

        /* Step 34: Trigger CSI-2 sequence for current lane count */
        write_reg(0xa0243ffc, (lane_num + 1));

        /* Step 35: Begin inner for loop - packet iteration */
        for (i = 0; i < cntrl_pkt_cnt; i++) {

            /* Step 36: Set ch0_preload_loc */
            ch0_preload_loc = ch0_pc;

            /* Step 37: Set ch1_preload_loc */
            ch1_preload_loc = ch1_pc;

            /* Step 38: Enable dma_irq[1] and dma_irq[0] */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);

            /* Step 39: Program DMA channel 0 control data transfer */
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0);

            /* Step 40: Start DMA channel 0 */
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);

            /* Step 41: Reset rd_data for poll */
            rd_data = 0;

            /* Step 42: Poll DMA interrupt status for channel 0 completion (bit 0) */
            timeout = PSV_POLL_TIMEOUT;
            while ((rd_data & 0x1) == 0x0) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                LOGT("polling irq; irq_status_reg rd_data =0x%x", rd_data);
                timeout--;
                if (timeout <= 0) {
                    LOGE("DMA ch0 poll timeout, rd_data=0x%x", rd_data);
                    g_errors++;
                    out->status = -1;
                    return out->status;
                }
            }

            /* Step 43: Clear DMA channel 0 interrupt */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            /* Step 44: Read DMA interrupt status register after clear */
            rd_data = read_reg(gdma_reg_base + 0x28);

            /* Step 45: Debug print irq polling completed ch0 */
            LOGT("DEBUG : irq polling completed ch0; rd_data = 0x%x", rd_data);

            /* Step 46: Read received control data */
            csi_ctrl_data = read_reg(0xE6001000);

            /* Step 47: Debug print csi_ctrl_data */
            LOGT("DEBUG: csi_ctrl_data=0x%x", csi_ctrl_data);

            /* Step 48: Check if data type field indicates a long packet */
            if ((csi_ctrl_data & 0x3f) > 0xf) {

                /* Step 49: Extract word count from bits[21:6] */
                word_count = ((csi_ctrl_data >> 6) & 0xffff);

                /* Step 50: Compute 8-byte aligned transfer size */
                csi_data_size = (word_count % 8) ? ((long long int)(word_count / 8 + 1) * 8) : (long long int)word_count;

                /* Step 51: Program DMA channel 1 data transfer */
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1);

                /* Step 52: Start DMA channel 1 */
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);

                /* Step 53: Reset rd_data for poll */
                rd_data = 0;

                /* Step 54: Poll DMA interrupt status for channel 1 completion (bit 1) */
                timeout = PSV_POLL_TIMEOUT;
                while ((rd_data & 0x2) == 0x0) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    LOGT("polling irq; irq_status_reg rd_data =0x%x", rd_data);
                    timeout--;
                    if (timeout <= 0) {
                        LOGE("DMA ch1 poll timeout, rd_data=0x%x", rd_data);
                        g_errors++;
                        out->status = -1;
                        return out->status;
                    }
                }

                /* Step 55: Additional read of DMA interrupt status after poll exit */
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                /* Step 56: Clear DMA channel 1 interrupt */
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);

                /* Step 57: Debug print irq polling completed ch1 */
                LOGT("DEBUG : irq polling completed ch1; rd_data=0x%x", rd_data);

            } /* Step 58: End of if block (data transfer branch) */

        } /* Step 59: End of inner for loop (packet iteration) */

    } /* Step 60: End of outer for loop (lane iteration) */

    /* Step 61: Testcase completion with pass status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status. finish() is not called in PSV/FV.
    out->status = (g_errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test_run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_errors);

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

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u", g_errors);

    return g_errors == 0U ? 0 : -1;
}
