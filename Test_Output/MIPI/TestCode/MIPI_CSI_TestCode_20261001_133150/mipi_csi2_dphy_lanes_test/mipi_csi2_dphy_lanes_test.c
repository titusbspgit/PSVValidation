// Author - AI Force 2.3. 01-Oct-2026 13:31 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Description: Validates the MIPI CSI-2 receiver across all D-PHY lane
 *              configurations (4 lanes down to 1 lane) using DMA-based
 *              control and data packet transfers.
 */

/* Test context structure */
typedef struct {
    uint32_t gdma_reg_base;
    uint32_t vcid_csi2_wrap_reg;
    uint32_t gdma_path;
    uint32_t ch0_pc;
    uint32_t ch1_pc;
    unsigned int errors;
} mipi_csi2_dphy_ctx_t;

static mipi_csi2_dphy_ctx_t g_ctx;

/* Helper: Enable all CSI-2 host interrupts */
static void csi2_enable_interrupt(void)
{
    uint32_t rd_data;

    /* Step 3: Read INT_ST_MAIN to clear pending interrupts */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("CSI2 INT_ST_MAIN read to clear pending: 0x%x", rd_data);

    /* Step 4: Write 0x0000000f to INT_MSK_PHY_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000fU);
    LOGT("Wrote 0x0000000f to INT_MSK_PHY_FATAL");

    /* Step 5: Write 0x00000003 to INT_MSK_PKT_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003U);
    LOGT("Wrote 0x00000003 to INT_MSK_PKT_FATAL");

    /* Step 6: Write 0x000f000f to INT_MSK_PHY */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000fU);
    LOGT("Wrote 0x000f000f to INT_MSK_PHY");

    /* Step 7: Write 0x000f000f to INT_MSK_LINE */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000fU);
    LOGT("Wrote 0x000f000f to INT_MSK_LINE");

    /* Step 8: Write 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_BNDRY_FRAME_FATAL");

    /* Step 9: Write 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_SEQ_FRAME_FATAL");

    /* Step 10: Write 0x0000ffff to INT_MSK_CRC_FRAME_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_CRC_FRAME_FATAL");

    /* Step 11: Write 0x0000ffff to INT_MSK_PLD_CRC_FATAL */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_PLD_CRC_FATAL");

    /* Step 12: Write 0x0000ffff to INT_MSK_DATA_ID */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_DATA_ID");

    /* Step 13: Write 0x0000ffff to INT_MSK_ECC_CORRECTED */
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffffU);
    LOGT("Wrote 0x0000ffff to INT_MSK_ECC_CORRECTED");
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
    uint32_t vcid;
    uint32_t vcid_unselected_path;
    uint32_t rd_data;
    uint32_t timeout;

    (void)cfg;

    g_ctx = (mipi_csi2_dphy_ctx_t){0};

    LOGT("mipi_csi2_dphy_lanes_test init: starting D-PHY lane test initialization");

    /* Step 2: Enable all CSI-2 host interrupts */
    csi2_enable_interrupt();
    LOGT("CSI-2 host interrupts enabled");

    /* Step 15: Compute vcid_csi2_wrap_reg based on GDMA path */
    vcid = VC_ID;
    vcid_unselected_path = ((vcid + 1U) & 0xfU);

#if defined(GDMA0_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#elif defined(GDMA1_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 1U;
#elif defined(GDMA2_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 2U;
#elif defined(GDMA3_PATH)
    g_ctx.vcid_csi2_wrap_reg = (vcid_unselected_path << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid << 12);
    g_ctx.gdma_path = 3U;
#else
    g_ctx.vcid_csi2_wrap_reg = (vcid << 0) |
                               (vcid_unselected_path << 4) |
                               (vcid_unselected_path << 8) |
                               (vcid_unselected_path << 12);
    g_ctx.gdma_path = 0U;
#endif

    /* Step 17: Set gdma_reg_base */
    g_ctx.gdma_reg_base = 0xE6A00000U;
    LOGT("GDMA path=%u, gdma_reg_base=0x%x, vcid_csi2_wrap_reg=0x%x",
         g_ctx.gdma_path, g_ctx.gdma_reg_base, g_ctx.vcid_csi2_wrap_reg);

    /* Step 18: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Wrote 0x%x to VIRTUAL_CHANNEL", g_ctx.vcid_csi2_wrap_reg);

    /* Step 19: Write 1 to CONTROL_DATA to enable control data transfer */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("Wrote 1 to CONTROL_DATA (enabled)");

    /* Step 20: Write vcid_csi2_wrap_reg to VIRTUAL_CHANNEL again (repeated write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, g_ctx.vcid_csi2_wrap_reg);
    LOGT("Wrote 0x%x to VIRTUAL_CHANNEL (repeated)", g_ctx.vcid_csi2_wrap_reg);

    /* Step 21: Write 1 to CONTROL_DATA again (repeated write) */
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1U);
    LOGT("Wrote 1 to CONTROL_DATA (repeated)");

    /* Step 22: Initialize the D-PHY */
    snps_phy_init();
    LOGT("D-PHY initialized via snps_phy_init()");

    /* Steps 23-24: Poll PHY_STOPSTATE until all lanes in stop state (0x1000f) */
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
    LOGT("PHY_STOPSTATE initial read: 0x%x", rd_data);

    timeout = MIPI_CSI2_POLL_TIMEOUT;
    while ((rd_data != 0x1000fU) && (timeout > 0U)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
    }

    if (timeout == 0U) {
        LOGE("PHY_STOPSTATE polling timeout, rd_data=0x%x", rd_data);
        g_ctx.errors++;
        return -1;
    }
    LOGT("PHY_STOPSTATE reached 0x1000f, all lanes in stop state");

    /* Step 25: Set DMA program counter addresses */
    g_ctx.ch0_pc = 0xE6000000U;
    g_ctx.ch1_pc = 0xE6000500U;
    LOGT("ch0_pc=0x%x, ch1_pc=0x%x", g_ctx.ch0_pc, g_ctx.ch1_pc);

    LOGT("mipi_csi2_dphy_lanes_test init complete");
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
    int lane_num;
    uint32_t cntrl_pkt_cnt;
    uint32_t i;
    uint32_t ch0_preload_loc;
    uint32_t ch1_preload_loc;
    uint32_t rd_data;
    uint32_t csi_ctrl_data;
    uint32_t word_count;
    uint32_t csi_data_size;
    uint32_t timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_dphy_lanes_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_csi2_dphy_lanes_test run");

    /* Step 26: Outer for-loop: lane_num from 3 down to 0 */
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        LOGT("Lane configuration: lane_num=%d (N_LANES=%d)", lane_num, lane_num);

        /* Step 27: Write lane_num to N_LANES */
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, (uint32_t)lane_num);
        LOGT("Wrote %d to N_LANES", lane_num);

        /* Step 28: Compute control packet count */
        cntrl_pkt_cnt = (VRES * 3U) + 2U;

        /* Step 29: Write (lane_num + 1) to 0xa0243ffc to trigger CSI-2 sequence */
        write_reg(0xa0243ffcU, (uint32_t)(lane_num + 1));
        LOGT("Wrote %d to 0xa0243ffc to trigger CSI-2 sequence", lane_num + 1);

        /* Step 30: Inner for-loop: iterate over control packets */
        for (i = 0U; i < cntrl_pkt_cnt; i++) {

            /* Step 31: Set preload locations */
            ch0_preload_loc = g_ctx.ch0_pc;
            ch1_preload_loc = g_ctx.ch1_pc;

            /* Step 32: Enable DMA interrupts for both channels */
            write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3U);

            /* Step 33: Preload DMA transfer instructions for channel 0 */
            // MANUAL_REVIEW: dma_trnsfr_instn_preload() exact 5-argument signature preserved; implementation not in supplied source.
            dma_trnsfr_instn_preload(ch0_preload_loc, 0x8000U, GDMA_CTRL_DATA_DEST_ADDR2, 8U, 0U);

            /* Step 34: Start DMA channel 0 */
            // MANUAL_REVIEW: DMAGO_CSI() exact 3-argument signature preserved; implementation not in supplied source.
            DMAGO_CSI(g_ctx.gdma_reg_base, 0U, g_ctx.ch0_pc);

            /* Steps 35-36: Poll DMA INTMIS until bit 0 is set (ch0 completion) */
            rd_data = 0U;
            timeout = MIPI_CSI2_POLL_TIMEOUT;
            while (((rd_data & 0x1U) == 0U) && (timeout > 0U)) {
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                timeout--;
            }
            if (timeout == 0U) {
                LOGE("DMA ch0 poll timeout, lane_num=%d pkt=%u rd_data=0x%x",
                     lane_num, i, rd_data);
                g_ctx.errors++;
                out->status = -1;
                return out->status;
            }

            /* Step 37: Clear DMA channel 0 interrupt */
            write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1U);

            /* Step 38: Read debug status */
            rd_data = read_reg(g_ctx.gdma_reg_base + 0x28U);
            LOGT("DMA debug status: 0x%x", rd_data);

            /* Step 39: Read control data */
            csi_ctrl_data = read_reg(0xE6001000U);
            LOGT("csi_ctrl_data=0x%x", csi_ctrl_data);

            /* Step 40: Check if long packet */
            if ((csi_ctrl_data & 0x3fU) > 0xfU) {

                /* Step 41: Extract word_count from bits[21:6] */
                word_count = (csi_ctrl_data >> 6) & 0xFFFFU;

                /* Step 42: Compute csi_data_size with 8-byte alignment */
                csi_data_size = (word_count + 7U) & ~7U;
                LOGT("Long packet: word_count=%u, csi_data_size=%u",
                     word_count, csi_data_size);

                /* Step 43: Preload DMA transfer instructions for channel 1 */
                dma_trnsfr_instn_preload(ch1_preload_loc, 0x0000U, GDMA_CSI2_DATA_DEST_ADDR2, csi_data_size, 1U);

                /* Step 44: Start DMA channel 1 */
                DMAGO_CSI(g_ctx.gdma_reg_base, 1U, g_ctx.ch1_pc);

                /* Steps 45-46: Poll DMA INTMIS until bit 1 is set (ch1 completion) */
                rd_data = 0U;
                timeout = MIPI_CSI2_POLL_TIMEOUT;
                while (((rd_data & 0x2U) == 0U) && (timeout > 0U)) {
                    rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    timeout--;
                }
                if (timeout == 0U) {
                    LOGE("DMA ch1 poll timeout, lane_num=%d pkt=%u rd_data=0x%x",
                         lane_num, i, rd_data);
                    g_ctx.errors++;
                    out->status = -1;
                    return out->status;
                }

                /* Step 47: Read DMA INTMIS again */
                rd_data = read_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                LOGT("DMA INTMIS after ch1 complete: 0x%x", rd_data);

                /* Step 48: Clear DMA channel 1 interrupt */
                write_reg(g_ctx.gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2U);

            } /* End of long-packet conditional block (Step 49) */

        } /* End of inner for-loop (Step 50) */

        LOGT("Lane configuration %d complete", lane_num);

    } /* End of outer for-loop (Step 51) */

    /* Step 52: DV finish(0) converted to PSV/FV pass status */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to FV out->status PASS.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
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
