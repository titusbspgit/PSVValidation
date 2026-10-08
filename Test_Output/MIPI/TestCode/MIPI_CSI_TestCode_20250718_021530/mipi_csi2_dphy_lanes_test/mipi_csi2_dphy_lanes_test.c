// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case    : mipi_csi2_dphy_lanes_test
 * Description  : Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *                through all lane counts (4 lanes down to 1 lane). For each
 *                lane configuration, enables CSI-2 interrupt masks, configures
 *                virtual channel and control data registers, initializes the
 *                SNPS D-PHY, polls PHY_STOPSTATE, programs N_LANES, triggers
 *                CSI-2 sequence, and performs DMA-based control and image data
 *                transfers with interrupt-driven completion polling.
 */

typedef struct {
    unsigned int gdma_reg_base;
    unsigned int vcid_csi2_wrap_reg;
    int gdma_path;
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_csi2_dphy_lanes_test_ctx_t;

static mipi_csi2_dphy_lanes_test_ctx_t g_ctx;

/*
 * Function: csi2_enable_interrupt
 * Description: Enable all CSI-2 host interrupt masks by reading INT_ST_MAIN
 *              to clear pending interrupts, then writing 0xFFFFFFFF to all
 *              interrupt mask registers.
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    /* Step: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("Read INT_ST_MAIN to clear pending interrupts: 0x%x", rd_data);

    /* Step: Enable PHY Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PHY_FATAL: 0xFFFFFFFF");

    /* Step: Enable Packet Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PKT_FATAL: 0xFFFFFFFF");

    /* Step: Enable PHY interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PHY: 0xFFFFFFFF");

    /* Step: Enable Line interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0xFFFFFFFF);
    LOGT("Write INT_MSK_LINE: 0xFFFFFFFF");

    /* Step: Enable Boundary Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_BNDRY_FRAME_FATAL: 0xFFFFFFFF");

    /* Step: Enable Sequence Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_SEQ_FRAME_FATAL: 0xFFFFFFFF");

    /* Step: Enable CRC Frame Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_CRC_FRAME_FATAL: 0xFFFFFFFF");

    /* Step: Enable Payload CRC Fatal interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0xFFFFFFFF);
    LOGT("Write INT_MSK_PLD_CRC_FATAL: 0xFFFFFFFF");

    /* Step: Enable Data ID interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0xFFFFFFFF);
    LOGT("Write INT_MSK_DATA_ID: 0xFFFFFFFF");

    /* Step: Enable ECC Corrected interrupt mask */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0xFFFFFFFF);
    LOGT("Write INT_MSK_ECC_CORRECTED: 0xFFFFFFFF");
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_dphy_lanes_test_ctx_t){0};

    /* Step: GDMA Path Selection (Conditional Compilation) */
#if defined(GDMA0_PATH)
    g_ctx.gdma_path = 0;
    g_ctx.vcid_csi2_wrap_reg = (VC_ID & 0xf) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
#elif defined(GDMA1_PATH)
    g_ctx.gdma_path = 1;
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf)) | ((VC_ID & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
#elif defined(GDMA2_PATH)
    g_ctx.gdma_path = 2;
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf)) | (((VC_ID + 1) & 0xf) << 4) |
                               ((VC_ID & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
#elif defined(GDMA3_PATH)
    g_ctx.gdma_path = 3;
    g_ctx.vcid_csi2_wrap_reg = (((VC_ID + 1) & 0xf)) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | ((VC_ID & 0xf) << 12);
#else
    g_ctx.gdma_path = 0;
    g_ctx.vcid_csi2_wrap_reg = (VC_ID & 0xf) | (((VC_ID + 1) & 0xf) << 4) |
                               (((VC_ID + 1) & 0xf) << 8) | (((VC_ID + 1) & 0xf) << 12);
#endif

    g_ctx.gdma_reg_base = 0xE6A00000;

    LOGT("mipi_csi2_dphy_lanes_test init: gdma_path=%d gdma_reg_base=0x%08x vcid_csi2_wrap_reg=0x%08x",
         g_ctx.gdma_path, g_ctx.gdma_reg_base, g_ctx.vcid_csi2_wrap_reg);

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Executes the main testcase flow for mipi_csi2_dphy_lanes_test.
 *              Enables CSI-2 interrupts, configures virtual channel and control
 *              data registers, initializes D-PHY, polls PHY stop state, then
 *              iterates lane counts (4 down to 1) performing DMA-based control
 *              and image data transfers with interrupt-driven completion polling.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    long long int gdma_tx_trnsfr_size;
    long long int gdma_trnsfr_size;
    int cntrl_pkt_cnt;
    int rd_data;
    int csi_ctrl_data;
    int word_count;
    int csi_data_size;
    int data_type;
    int lane_count;
    int pkt;
    int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_dphy_lanes_test run");

    /* Step 3: Print start line */
    LOGT("start line");

    /* Step 4: Enable all CSI-2 host interrupt masks */
    LOGT("=== Enabling CSI-2 Host Interrupt Masks ===");
    csi2_enable_interrupt();

    /* Step 20: Configure virtual channel register (first write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL: 0x%08x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 21: Configure control data register (first write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1);
    LOGT("Write CONTROL_DATA: 0x1 (enable)");

    /* Step 22: Configure virtual channel register (second write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Write VIRTUAL_CHANNEL (repeat): 0x%08x", g_ctx.vcid_csi2_wrap_reg);

    /* Step 23: Configure control data register (second write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 0x1);
    LOGT("Write CONTROL_DATA (repeat): 0x1 (enable)");

    /* Step 25: Initialize the SNPS D-PHY */
    LOGT("=== Initializing SNPS D-PHY ===");
    snps_phy_init();
    LOGT("SNPS D-PHY initialization complete");

    /* Step 26-27: Poll PHY_STOPSTATE until D-PHY enters stop state */
    LOGT("Polling PHY_STOPSTATE for stop state (expected: 0x%x)...", PHY_STOPSTATE_EXPECTED);
    timeout = MIPI_CSI2_TIMEOUT_COUNT;
    do {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout <= 0) {
            LOGE("PHY_STOPSTATE polling timeout: rd_data=0x%x expected=0x%x", rd_data, PHY_STOPSTATE_EXPECTED);
            g_ctx.errors++;
            out->status = -1;
            return out->status;
        }
    } while (rd_data != PHY_STOPSTATE_EXPECTED);
    LOGT("PHY_STOPSTATE achieved: 0x%x", rd_data);
    g_ctx.checks_total++;
    g_ctx.checks_passed++;

    /* Step 28: Set DMA program counter address for channel 0 */
    dma_trnsfr_instn_preload(0, GDMA_CTRL_DATA_DEST_ADDR2);
    LOGT("DMA PC address set for channel 0: 0x%08x", GDMA_CTRL_DATA_DEST_ADDR2);

    /* Step 29: Set DMA program counter address for channel 1 */
    dma_trnsfr_instn_preload(1, GDMA_CSI2_DATA_DEST_ADDR2);
    LOGT("DMA PC address set for channel 1: 0x%08x", GDMA_CSI2_DATA_DEST_ADDR2);

    /* Step 30-60: Iterate through lane configurations (4 lanes down to 1 lane) */
    for (lane_count = 4; lane_count >= 1; lane_count--) {
        LOGT("=== Lane Configuration: %d lane(s) ===", lane_count);

        /* Step: Write lane count to N_LANES register */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_count - 1);
        LOGT("Write N_LANES: %d (value: %d)", lane_count, lane_count - 1);

        /* Step: Calculate control packet count based on VRES */
        cntrl_pkt_cnt = VRES * 3 + 2;
        LOGT("Control packet count: %d (VRES=%d)", cntrl_pkt_cnt, VRES);

        /* Step: Trigger CSI-2 sequence */
        write_reg(CSI2_SEQ_TRIGGER_ADDR, lane_count);
        LOGT("CSI-2 sequence triggered with lane_count: %d", lane_count);

        /* Step: Process each control packet in the frame */
        for (pkt = 0; pkt < cntrl_pkt_cnt; pkt++) {

            /* Step: Enable DMA interrupts for both channels */
            write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);

            /* Step: Program DMA channel 0 for control packet transfer (8 bytes) */
            gdma_trnsfr_size = 8;
            dma_trnsfr_instn_preload(0, GDMA_CTRL_DATA_DEST_ADDR2);

            /* Step: Start DMA channel 0 */
            DMAGO_CSI(0, CSI_CTRL_SRC_ADDR, CSI_CTRL_DATA_ADDR, gdma_trnsfr_size);

            /* Step: Poll DMA interrupt status for channel 0 completion */
            timeout = MIPI_CSI2_TIMEOUT_COUNT;
            do {
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
                if (timeout <= 0) {
                    LOGE("DMA CH0 polling timeout at lane=%d pkt=%d intmis=0x%x",
                         lane_count, pkt, rd_data);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }
            } while ((rd_data & 0x1) == 0x0);
            LOGT("DMA CH0 transfer complete for packet %d (intmis: 0x%x)", pkt, rd_data);
            g_ctx.checks_total++;
            g_ctx.checks_passed++;

            /* Step: Clear DMA channel 0 interrupt */
            write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            /* Step: Read CSI control data from destination address */
            csi_ctrl_data = read_reg(CSI_CTRL_DATA_ADDR);
            data_type = csi_ctrl_data & 0x3f;
            LOGT("CSI control data: 0x%08x, data_type: 0x%x", csi_ctrl_data, data_type);

            /* Step: If image data, perform CSI data transfer on channel 1 */
            if (data_type > 0xf) {
                word_count = ((csi_ctrl_data >> 6) & 0xffff);
                csi_data_size = (word_count % 8) ? ((word_count / 8 + 1) * 8) : word_count;
                LOGT("Image data detected: word_count=%d, csi_data_size=%d (aligned)",
                     word_count, csi_data_size);

                /* Step: Program DMA channel 1 for CSI data transfer */
                gdma_tx_trnsfr_size = csi_data_size;
                DMAGO_CSI(1, CSI_DATA_SRC_ADDR, CSI_DATA_DEST_ADDR, gdma_tx_trnsfr_size);

                /* Step: Poll DMA interrupt status for channel 1 completion */
                timeout = MIPI_CSI2_TIMEOUT_COUNT;
                do {
                    rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                    if (timeout <= 0) {
                        LOGE("DMA CH1 polling timeout at lane=%d pkt=%d intmis=0x%x",
                             lane_count, pkt, rd_data);
                        g_ctx.errors++;
                        out->status = -1;
                        return out->status;
                    }
                } while ((rd_data & 0x2) == 0x0);
                LOGT("DMA CH1 transfer complete for packet %d (intmis: 0x%x)", pkt, rd_data);
                g_ctx.checks_total++;
                g_ctx.checks_passed++;

                /* Step: Clear DMA channel 1 interrupt */
                write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);
            }
        }
        LOGT("Lane configuration %d complete: all %d packets processed", lane_count, cntrl_pkt_cnt);
    }

    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for mipi_csi2_dphy_lanes_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test teardown: errors=%u checks_total=%u checks_passed=%u checks_failed=%u",
         g_ctx.errors, g_ctx.checks_total, g_ctx.checks_passed, g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
