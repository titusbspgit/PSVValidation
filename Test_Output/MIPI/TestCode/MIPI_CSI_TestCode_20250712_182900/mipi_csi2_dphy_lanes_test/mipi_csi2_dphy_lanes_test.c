// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_dphy_lanes_test.h"
#include "test_define.inc"

// Testcase context
static unsigned int g_errors;

// PHY_STOPSTATE polling timeout
#define MIPI_CSI2_PHY_POLL_TIMEOUT 100000U
// DMA polling timeout
#define MIPI_CSI2_DMA_POLL_TIMEOUT 100000U

/*
 * Function: csi2_enable_interrupt
 * Description: Enable CSI-2 interrupts by reading INT_ST_MAIN to clear
 *              pending interrupts and writing all interrupt mask registers.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void csi2_enable_interrupt(void)
{
    int rd_data;

    // Step 1b: Read INT_ST_MAIN to clear pending interrupts
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_INT_ST_MAIN);
    LOGT("csi2_enable_interrupt: INT_ST_MAIN read = 0x%x", rd_data);

    // Step 1c: Write interrupt mask registers
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY_FATAL, 0x0000000f);

    // Step 1d
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PKT_FATAL, 0x00000003);

    // Step 1e
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PHY, 0x000f000f);

    // Step 1f
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_LINE, 0x000f000f);

    // Step 1g
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_BNDRY_FRAME_FATAL, 0x0000ffff);

    // Step 1h
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_SEQ_FRAME_FATAL, 0x0000ffff);

    // Step 1i
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_CRC_FRAME_FATAL, 0x0000ffff);

    // Step 1j
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_PLD_CRC_FATAL, 0x0000ffff);

    // Step 1k
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_DATA_ID, 0x0000ffff);

    // Step 1l
    write_reg(MIZAR_MIPI_CSI2_HOST_INT_MSK_ECC_CORRECTED, 0x0000ffff);
}

/*
 * Function: mipi_csi2_dphy_lanes_test_init
 * Description: Initialize testcase context, enable CSI-2 interrupts,
 *              configure virtual channel and control data registers,
 *              initialize D-PHY, and poll PHY_STOPSTATE.
 * Parameters:
 *   cfg - Pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_init(const TestsItem *cfg)
{
    int rd_data;
    unsigned int vcid_csi2_wrap_reg = 0;
    unsigned int gdma_path = 0;
    unsigned int timeout;

    (void)cfg;

    g_errors = 0;

    LOGT("mipi_csi2_dphy_lanes_test_init: START");

    // Step 1: Call csi2_enable_interrupt()
    csi2_enable_interrupt();

    // Step 2: Conditional GDMA path selection
#ifdef GDMA3_PATH
    vcid_csi2_wrap_reg = VC_ID;
    gdma_path = 3;
#elif defined(GDMA2_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 4);
    gdma_path = 2;
#elif defined(GDMA1_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 8);
    gdma_path = 1;
#elif defined(GDMA0_PATH)
    vcid_csi2_wrap_reg = (VC_ID << 12);
    LOGT("VC_ID=%d", VC_ID);
    gdma_path = 0;
#endif

    (void)gdma_path;

    // Step 3: gdma_reg_base is set globally in test_define.inc

    // Step 4: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    // Step 5: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);

    // Step 6: printf
    LOGT("vcid_csi2_wrap_reg=%0x", vcid_csi2_wrap_reg);

    // Step 7: write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg) (second time)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_VIRTUAL_CHANNEL, vcid_csi2_wrap_reg);

    // Step 8: write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1) (second time)
    write_reg(MIZAR_MIPI_CSI2_RB_REG_CONTROL_DATA, 1);

    // Step 9: Call snps_phy_init()
    snps_phy_init();

    // Step 10: rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE)
    rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);

    // Step 11: while(!(rd_data == 0x1000f)) poll PHY_STOPSTATE
    timeout = MIPI_CSI2_PHY_POLL_TIMEOUT;
    while (!(rd_data == 0x1000f)) {
        rd_data = read_reg(MIZAR_MIPI_CSI2_HOST_PHY_STOPSTATE);
        timeout--;
        if (timeout == 0U) {
            LOGE("PHY_STOPSTATE polling timeout, rd_data=0x%x", rd_data);
            g_errors++;
            return -1;
        }
    }

    LOGT("PHY_STOPSTATE reached 0x%x", rd_data);
    LOGT("mipi_csi2_dphy_lanes_test_init: DONE");

    return 0;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_run
 * Description: Main testcase execution. Iterate through lane configurations
 *              from 4 lanes (lane_num=3) down to 1 lane (lane_num=0).
 *              For each lane, perform DMA-based control and data packet
 *              transfers with interrupt-driven completion polling.
 * Parameters:
 *   cfg - Pointer to test configuration item
 *   out - Pointer to test output structure
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_run(const TestsItem *cfg, TestOutput *out)
{
    int rd_data;
    unsigned int ch0_pc;
    unsigned int ch1_pc;
    unsigned int ch0_preload_loc;
    unsigned int ch1_preload_loc;
    int cntrl_pkt_cnt;
    int csi_ctrl_data;
    long int word_count;
    long int csi_data_size;
    unsigned int timeout;
    int lane_num;
    int i;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_dphy_lanes_test_run: START");

    // Step 12: ch0_pc = 0xE6000000
    ch0_pc = 0xE6000000;

    // Step 13: ch1_pc = 0xE6000500
    ch1_pc = 0xE6000500;

    // Step 14: for(int lane_num=3; lane_num>=0; lane_num--)
    for (lane_num = 3; lane_num >= 0; lane_num--) {

        // Step 14a: write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num)
        write_reg(MIZAR_MIPI_CSI2_HOST_N_LANES, lane_num);
        LOGT("Configured N_LANES = %d", lane_num);

        // Step 14b: cntrl_pkt_cnt = ((VRES * 3) + 2)
        cntrl_pkt_cnt = ((VRES * 3) + 2);

        // Step 14c: printf("DEBUG: cntrl_pkt_cnt=%d\n", cntrl_pkt_cnt)
        LOGT("DEBUG: cntrl_pkt_cnt=%d", cntrl_pkt_cnt);

        // Step 14d: write_reg(0xa0243ffc, (lane_num+1))
        // DV debug marker write - converted to PSV logging per DV-to-FV rule
        LOGD("DV debug marker: trigger CSI-2 sequence with lane_count=%d", (lane_num + 1));

        // Step 14e: for(int i=0; i< cntrl_pkt_cnt; i++)
        for (i = 0; i < cntrl_pkt_cnt; i++) {

            // Step 14e-i: ch0_preload_loc = ch0_pc
            ch0_preload_loc = ch0_pc;

            // Step 14e-ii: ch1_preload_loc = ch1_pc
            ch1_preload_loc = ch1_pc;

            // Step 14e-iii: write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTEN_OFFSET, 0x3)
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTEN_OFFSET, 0x3);

            // Step 14e-iv: dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0)
            dma_trnsfr_instn_preload(ch0_preload_loc, gdma_reg_base, 0x8000, 0xE6001000, 8, 0);

            // Step 14e-v: DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0)
            DMAGO_CSI(gdma_reg_base, ch0_pc, 0x0);

            // Step 14e-vi: rd_data = 0
            rd_data = 0;

            // Step 14e-vii: while((rd_data & 0x1) == 0x0) poll DMA ch0 interrupt
            timeout = MIPI_CSI2_DMA_POLL_TIMEOUT;
            while ((rd_data & 0x1) == 0x0) {
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
                timeout--;
                if (timeout == 0U) {
                    LOGE("DMA ch0 polling timeout, rd_data=0x%x", rd_data);
                    g_errors++;
                    out->status = -1;
                    return out->status;
                }
            }

            // Step 14e-viii: write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1)
            write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x1);

            // Step 14e-ix: rd_data = read_reg(gdma_reg_base + 0x28)
            rd_data = read_reg(gdma_reg_base + 0x28);

            // Step 14e-x: printf("DEBUG : irq polling completed ch0; rd_data = %0x \n", rd_data)
            LOGT("DEBUG : irq polling completed ch0; rd_data = %0x", rd_data);

            // Step 14e-xi: csi_ctrl_data = read_reg(0xE6001000)
            csi_ctrl_data = read_reg(0xE6001000);

            // Step 14e-xii: printf("DEBUG: csi_ctrl_data=%0x\n", csi_ctrl_data)
            LOGT("DEBUG: csi_ctrl_data=%0x", csi_ctrl_data);

            // Step 14e-xiii: if((csi_ctrl_data & 0x3f) > 0xf)
            if ((csi_ctrl_data & 0x3f) > 0xf) {

                // Step 14e-xiii-a: word_count = ((csi_ctrl_data >> 6)&0xffff)
                word_count = ((csi_ctrl_data >> 6) & 0xffff);

                // Step 14e-xiii-b: csi_data_size = (word_count%8) ? (word_count/8 + 1)*8 : word_count
                csi_data_size = (word_count % 8) ? (word_count / 8 + 1) * 8 : word_count;

                // Step 14e-xiii-c: dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1)
                dma_trnsfr_instn_preload(ch1_preload_loc, gdma_reg_base, 0x0000, 0xE6002000, csi_data_size, 1);

                // Step 14e-xiii-d: DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1)
                DMAGO_CSI(gdma_reg_base, ch1_pc, 0x1);

                // Step 14e-xiii-e: rd_data = 0
                rd_data = 0;

                // Step 14e-xiii-f: while((rd_data & 0x2) == 0x0) poll DMA ch1 interrupt
                timeout = MIPI_CSI2_DMA_POLL_TIMEOUT;
                while ((rd_data & 0x2) == 0x0) {
                    rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);
                    LOGT("polling irq; irq_status_reg rd_data =%0x", rd_data);
                    timeout--;
                    if (timeout == 0U) {
                        LOGE("DMA ch1 polling timeout, rd_data=0x%x", rd_data);
                        g_errors++;
                        out->status = -1;
                        return out->status;
                    }
                }

                // Step 14e-xiii-g: rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET)
                rd_data = read_reg(gdma_reg_base + MIPI_CSI2_DMA_INTMIS_OFFSET);

                // Step 14e-xiii-h: write_reg(gdma_reg_base+MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2)
                write_reg(gdma_reg_base + MIPI_CSI2_DMA_INTCLR_OFFSET, 0x2);

                // Step 14e-xiii-i: printf("DEBUG : irq polling completed ch1; rd_data=%0x\n", rd_data)
                LOGT("DEBUG : irq polling completed ch1; rd_data=%0x", rd_data);
            }
        }

        LOGT("Lane configuration lane_num=%d completed", lane_num);
    }

    // Step 15: finish(0) -> FV/PSV completion
    out->status = (g_errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_dphy_lanes_test_run: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_errors);

    return out->status;
}

/*
 * Function: mipi_csi2_dphy_lanes_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - Pointer to test configuration item
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_dphy_lanes_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_dphy_lanes_test_teardown: errors=%u", g_errors);
    return g_errors == 0U ? 0 : -1;
}
