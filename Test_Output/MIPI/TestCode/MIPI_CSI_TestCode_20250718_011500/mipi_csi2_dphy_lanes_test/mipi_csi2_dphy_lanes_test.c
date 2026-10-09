// Author - AI Force 2.3. 18-Jul-2025 01:15 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Description: Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *              through lane counts from 4 lanes down to 1 lane. For each lane
 *              configuration, the test performs a complete CSI-2 data reception
 *              sequence including control packet DMA transfers and data packet
 *              DMA transfers with interrupt-driven completion polling.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/* Helper function: enable all CSI-2 host interrupt masks */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    /* Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: read INT_ST_MAIN=0x%x to clear pending interrupts", rd_data);

    /* Enable PHY_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);
    LOGT("write INT_MSK_PHY_FATAL=0x0000000f");

    /* Enable PKT_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x0000000f);
    LOGT("write INT_MSK_PKT_FATAL=0x0000000f");

    /* Enable PHY interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);
    LOGT("write INT_MSK_PHY=0x000f000f");

    /* Enable LINE interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);
    LOGT("write INT_MSK_LINE=0x000f000f");

    /* Enable BNDRY_FRAME_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x000f000f);
    LOGT("write INT_MSK_BNDRY_FRAME_FATAL=0x000f000f");

    /* Enable SEQ_FRAME_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x000f000f);
    LOGT("write INT_MSK_SEQ_FRAME_FATAL=0x000f000f");

    /* Enable CRC_FRAME_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x000f000f);
    LOGT("write INT_MSK_CRC_FRAME_FATAL=0x000f000f");

    /* Enable PLD_CRC_FATAL interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x000f000f);
    LOGT("write INT_MSK_PLD_CRC_FATAL=0x000f000f");

    /* Enable DATA_ID interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x000f000f);
    LOGT("write INT_MSK_DATA_ID=0x000f000f");

    /* Enable ECC_CORRECTED interrupts */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x000f000f);
    LOGT("write INT_MSK_ECC_CORRECTED=0x000f000f");
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

    LOGT("mipi_csi2_dphy_lanes_test init: D-PHY lane configuration test");

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
    long long int rx_desc;
    long long int tx_desc;
    long long int gdma_tx_trnsfr_size;
    long long int gdma_trnsfr_size;
    int cntrl_pkt_cnt;
    int lane_num;
    int rd_data;
    int data_type;
    int word_count;
    int csi_data_size;
    int pkt;
    int timeout;

    (void)cfg;
    (void)tx_desc;
    (void)gdma_tx_trnsfr_size;
    (void)gdma_trnsfr_size;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("start line");

    /* Step 1: Enable all CSI-2 host interrupt masks */
    LOGT("Step 1: Enabling all CSI-2 host interrupt masks");
    csi2_enable_interrupt();

    /* Step 2: Configure virtual channel register with VC_ID */
    LOGT("Step 2: Configuring virtual channel register with VC_ID=%d", VC_ID);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, VC_ID);

    /* Step 3: Enable control data transfer */
    LOGT("Step 3: Enabling control data transfer");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1);

    /* Step 4: Repeat virtual channel and control data configuration */
    LOGT("Step 4: Repeating virtual channel and control data configuration");
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, VC_ID);
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1);

    /* Step 5: Perform D-PHY initialization sequence */
    LOGT("Step 5: Performing D-PHY initialization via snps_phy_init()");
    snps_phy_init();

    /* Step 6: Poll PHY_STOPSTATE until D-PHY enters stop state (expected 0x1000f) */
    LOGT("Step 6: Polling PHY_STOPSTATE for expected value 0x1000f");
    timeout = MIPI_CSI2_POLL_TIMEOUT;
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    } while ((rd_data != 0x1000f) && (timeout > 0));

    if (timeout <= 0) {
        LOGE("PHY_STOPSTATE poll timeout: rd_data=0x%x expected=0x1000f", rd_data);
        g_ctx.errors++;
        out->status = -1;
        return out->status;
    }
    LOGT("PHY_STOPSTATE=0x%x, D-PHY entered stop state", rd_data);
    g_ctx.checks_passed++;
    g_ctx.checks_total++;

    /* Step 7: Iterate lane configurations from 4 lanes down to 1 lane */
    cntrl_pkt_cnt = ((VRES * 3) + 2);
    LOGT("Step 7: Starting lane iteration, cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

    for (lane_num = 3; lane_num >= 0; lane_num--) {

        /* Step 7a: Write N_LANES register with current lane count */
        LOGT("Lane config: lane_num=%d (lane count=%d)", lane_num, lane_num + 1);
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);

        /* Step 7b: Trigger CSI-2 sequence for current lane configuration */
        LOGT("Triggering CSI-2 sequence: writing %d to 0xa0243ffc", lane_num + 1);
        write_reg(0xa0243ffc, (lane_num + 1));

        /* Step 7c: Loop through control/data packets */
        for (pkt = 0; pkt < cntrl_pkt_cnt; pkt++) {

            /* Step 7c-i: Enable DMA interrupts for both channels */
            write_reg(MIPI_CSI2_DMA_INTCLR_OFFSET, 0x3);

            /* Step 7c-ii: Program DMA channel 0 for control data transfer (8 bytes) */
            dma_trnsfr_instn_preload(0, 0x8000, 0xE6001000, 8);
            DMAGO_CSI(0);
            LOGT("DMA ch0 started: src=0x8000 dst=0xE6001000 size=8 (lane=%d pkt=%d)",
                 lane_num + 1, pkt);

            /* Step 7c-iii: Poll DMA interrupt status for channel 0 completion */
            timeout = MIPI_CSI2_POLL_TIMEOUT;
            do {
                rd_data = read_reg(MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
            } while (((rd_data & 0x1) == 0) && (timeout > 0));

            if (timeout <= 0) {
                LOGE("DMA ch0 poll timeout: lane=%d pkt=%d INTMIS=0x%x",
                     lane_num + 1, pkt, rd_data);
                g_ctx.errors++;
                out->status = -1;
                return out->status;
            }
            g_ctx.checks_passed++;
            g_ctx.checks_total++;

            /* Step 7c-iv: Clear channel 0 DMA interrupt */
            write_reg(MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            /* Step 7c-v: Read received control data */
            rx_desc = *((volatile long long int *)0xE6001000);
            data_type = (int)(rx_desc & 0x3f);
            LOGT("Control data read: rx_desc=0x%llx data_type=0x%x",
                 (unsigned long long)rx_desc, data_type);

            /* Step 7c-vi: If long packet, extract word count and do data transfer */
            if (data_type > 0xf) {
                word_count = (int)((rx_desc >> 6) & 0xffff);
                csi_data_size = (word_count + 7) & ~7; /* 8-byte aligned */
                LOGT("Long packet: word_count=%d csi_data_size=%d",
                     word_count, csi_data_size);

                /* Program DMA channel 1 for data transfer */
                dma_trnsfr_instn_preload(1, 0x0000, 0xE6002000, csi_data_size);
                DMAGO_CSI(1);
                LOGT("DMA ch1 started: src=0x0000 dst=0xE6002000 size=%d", csi_data_size);

                /* Step 7c-vii: Poll DMA interrupt status for channel 1 completion */
                timeout = MIPI_CSI2_POLL_TIMEOUT;
                do {
                    rd_data = read_reg(MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                } while (((rd_data & 0x2) == 0) && (timeout > 0));

                if (timeout <= 0) {
                    LOGE("DMA ch1 poll timeout: lane=%d pkt=%d INTMIS=0x%x",
                         lane_num + 1, pkt, rd_data);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }
                g_ctx.checks_passed++;
                g_ctx.checks_total++;

                /* Step 7c-viii: Clear channel 1 DMA interrupt */
                write_reg(MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);
            }
        }
        LOGT("%d-lane configuration complete", lane_num + 1);
    }

    /* Step 8: Complete test with pass status */
    // MANUAL_REVIEW: DV finish(0) converted to PSV/FV out->status based completion.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

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

    g_ctx.checks_failed = g_ctx.errors;

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u checks_passed=%u checks_failed=%u checks_total=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_failed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
