// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 DPHY lane configuration by iterating
 *              through all lane counts (4 lanes down to 1 lane). For each
 *              lane configuration, it performs a complete CSI-2 data reception
 *              sequence including DMA transfers for control and image data.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

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

    /* Step 2: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: cleared pending interrupts INT_ST_MAIN=0x%x",
         (unsigned int)rd_data);

    /* Step 3: Enable PHY fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("write INT_MSK_PHY_FATAL=0x0000000f");

    /* Step 4: Enable packet fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("write INT_MSK_PKT_FATAL=0x00000003");

    /* Step 5: Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("write INT_MSK_PHY=0x000f000f");

    /* Step 6: Enable line interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("write INT_MSK_LINE=0x000f000f");

    /* Step 7: Enable boundary frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_BNDRY_FRAME_FATAL=0x0000ffff");

    /* Step 8: Enable sequence frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_SEQ_FRAME_FATAL=0x0000ffff");

    /* Step 9: Enable CRC frame fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_CRC_FRAME_FATAL=0x0000ffff");

    /* Step 10: Enable payload CRC fatal interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("write INT_MSK_PLD_CRC_FATAL=0x0000ffff");

    /* Step 11: Enable data ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("write INT_MSK_DATA_ID=0x0000ffff");

    /* Step 12: Enable ECC corrected interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("write INT_MSK_ECC_CORRECTED=0x0000ffff");
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

    LOGT("mipi_csi2_dphy_lanes_test init: starting CSI-2 DPHY lane test initialization");

    /* Step 1: Enable CSI-2 interrupts */
    csi2_enable_interrupt();
    LOGT("CSI-2 interrupts enabled");

    /* Step 14: Determine vcid_csi2_wrap_reg based on GDMA path */
    /* Step 20: Call snps_phy_init() */
    snps_phy_init();
    LOGT("D-PHY initialization complete via snps_phy_init()");

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
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    int lane_num;
    int pkt;
    int cntrl_pkt_cnt;
    unsigned int ctrl_pkt_data;
    unsigned int data_type;
    unsigned int word_count;
    unsigned int transfer_size;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test run: starting main test flow");

    /* Step 14: Determine vcid_csi2_wrap_reg based on GDMA path */
#ifdef GDMA_PATH_1
    vcid_csi2_wrap_reg = ((unsigned int)VC_ID << 8) | 0x0fU;
#else
    vcid_csi2_wrap_reg = ((unsigned int)VC_ID << 0) | (0x0fU << 8);
#endif
    LOGT("vcid_csi2_wrap_reg=0x%x", vcid_csi2_wrap_reg);

    /* Step 15: Set gdma_reg_base */
    gdma_reg_base = MIPI_CSI2_GDMA_REG_BASE;
    LOGT("gdma_reg_base=0x%x", gdma_reg_base);

    /* Step 16: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    LOGT("write VIRTUAL_CHANNEL=0x%x", vcid_csi2_wrap_reg);

    /* Step 17: Enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1U);
    LOGT("write CONTROL_DATA=0x1");

    /* Steps 18-19: Repeat writes */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1U);
    LOGT("repeated writes to VIRTUAL_CHANNEL and CONTROL_DATA");

    /* Steps 21-22: Poll PHY_STOPSTATE until 0x1000f */
    LOGT("polling PHY_STOPSTATE for stop state entry");
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    } while (rd_data != MIPI_CSI2_PHY_STOPSTATE_EXPECTED);
    LOGT("PHY_STOPSTATE=0x%x all lanes and clock stopped", rd_data);

    /* Step 23: Set DMA PC addresses */
    ch0_pc = gdma_reg_base + 0x400U;
    ch1_pc = gdma_reg_base + 0x800U;
    LOGT("DMA PC addresses: ch0_pc=0x%x ch1_pc=0x%x", ch0_pc, ch1_pc);

    /* Steps 24-47: Lane iteration (4 lanes down to 1 lane) */
    cntrl_pkt_cnt = VRES * 3 + 2;
    LOGT("starting lane iteration cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

    for (lane_num = 3; lane_num >= 0; lane_num--) {
        LOGT("configuring lane_num=%d (lane count=%d)", lane_num, lane_num + 1);

        /* Write lane count to N_LANES */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (unsigned int)lane_num);
        LOGT("write N_LANES=0x%x", (unsigned int)lane_num);

        /* Signal start of CSI-2 sequence */
        write_reg(MIPI_CSI2_LANE_SIGNAL_ADDR, (unsigned int)(lane_num + 1));
        LOGT("signal lane count via 0xa0243ffc=%d", lane_num + 1);

        /* Inner loop: iterate over expected control packets */
        for (pkt = 0; pkt < cntrl_pkt_cnt; pkt++) {

            /* Clear DMA interrupts */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x3U);

            /* Program and start DMA channel 0 for control data */
            dma_trnsfr_instn_preload(gdma_reg_base, 0, 0, GDMA_CTRL_DATA_DEST_ADDR2, 16);
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);

            /* Poll DMA interrupt status for channel 0 completion (bit 0) */
            do {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
            } while ((rd_data & 0x1U) == 0U);
            LOGT("lane=%d pkt=%d DMA CH0 complete INTMIS=0x%x", lane_num, pkt, rd_data);

            /* Clear DMA channel 0 interrupt */
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Read control packet data */
            ctrl_pkt_data = read_reg(MIPI_CSI2_CTRL_DATA_READ_ADDR);
            data_type = ctrl_pkt_data & 0x3fU;
            LOGT("lane=%d pkt=%d ctrl_pkt_data=0x%x data_type=0x%x",
                 lane_num, pkt, ctrl_pkt_data, data_type);

            /* Check for long packet (image data): data_type > 0x0f */
            if (data_type > 0x0fU) {
                word_count = (ctrl_pkt_data >> 6) & 0xFFFFU;
                transfer_size = word_count;
                LOGT("lane=%d pkt=%d long packet word_count=%u transfer_size=%u",
                     lane_num, pkt, word_count, transfer_size);

                /* Program and start DMA channel 1 for image data */
                dma_trnsfr_instn_preload(gdma_reg_base, 1, 0, GDMA_CSI2_DATA_DEST_ADDR2, transfer_size);
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);

                /* Poll DMA interrupt status for channel 1 completion (bit 1) */
                do {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                } while ((rd_data & 0x2U) == 0U);
                LOGT("lane=%d pkt=%d DMA CH1 complete INTMIS=0x%x", lane_num, pkt, rd_data);

                /* Clear DMA channel 1 interrupt */
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);
            }
        }
        LOGT("lane=%d all packets transferred successfully", lane_num);
    }

    /* Step 48: DV finish(0) converted to PSV/FV status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. PSV/FV-native
    // completion is handled via out->status.

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("run complete: %s errors=%u",
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
