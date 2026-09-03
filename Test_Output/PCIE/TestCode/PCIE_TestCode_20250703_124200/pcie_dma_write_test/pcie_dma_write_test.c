// Author - AI Force 2.3. 03-Jul-2025 18:12 IST
// (EMBENGG-SYSAPPS)

/*
 * pcie_dma_write_test.c
 *
 * Test Case : pcie_dma_write_test
 * Description: PCIe DMA write operation test. Initializes DMA engine,
 *              performs link training, programs cache coherency,
 *              polls SII link status, configures DMA channel registers,
 *              triggers DMA write, polls for completion, verifies data,
 *              and polls for final synchronization.
 */

#include "pcie_dma_write_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int test_err;

/*
 * Function: pcie_dma_write_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Main testcase execution for pcie_dma_write_test. Performs link training,
 *              cache coherency programming, SII link status polling, DMA channel
 *              configuration, DMA write trigger, DMA status polling, data verification,
 *              system register writes, and completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe DMA write test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 1: write_reg(0xE6004100, 0x0) done\n");
    #endif

    /* Step 2: Conditionally call link training based on defines */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
        #endif
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
        #endif
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 phase 1 */
    /* Read COHERENCY_CONTROL_3_OFF, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 3: PCIE0 cache prog phase1 [11:14]=0xf [3:6]=0xf done\n");
    #endif

    /* Step 4: PCIE0 phase 2 - set bits [27:30]=0xf, [19:22]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 4: PCIE0 cache prog phase2 [27:30]=0xf [19:22]=0xf done\n");
    #endif

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 5: PCIE1 cache prog phase1+phase2 done\n");
    #endif

    /* Step 6: Wait for cache programming to take effect */
    wait_on(20);

    /* Step 7: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & SII_LINK_STATUS_MASK) != SII_LINK_STATUS_EXPECT)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 7: Polling SII0 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("Step 7: SII0 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 8: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & SII_LINK_STATUS_MASK) != SII_LINK_STATUS_EXPECT)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 8: Polling SII1 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("Step 8: SII1 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 9: Configure DMA channel registers */
    /* Set DMA source address, destination address, transfer size, and control fields */
    write_reg(DMA_CH0_SRC_ADDR, DMA_CH0_SRC_ADDR_VAL);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: DMA_CH0_SRC_ADDR configured\n");
    #endif

    write_reg(DMA_CH0_DST_ADDR, DMA_CH0_DST_ADDR_VAL);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: DMA_CH0_DST_ADDR configured\n");
    #endif

    write_reg(DMA_CH0_XFER_SIZE, DMA_CH0_XFER_SIZE_VAL);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: DMA_CH0_XFER_SIZE configured\n");
    #endif

    write_reg(DMA_CH0_CTRL, DMA_CH0_CTRL_WR_VAL);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: DMA_CH0_CTRL configured for write direction\n");
    #endif

    /* Step 10: Trigger DMA write by writing to DMA doorbell register */
    write_reg(DMA_DOORBELL, DMA_DOORBELL_TRIGGER_VAL);
    LOGI("Step 10: DMA write triggered via DMA_DOORBELL\n");

    /* Step 11: Poll DMA status register until transfer complete */
    data_rd = read_reg(DMA_STATUS);
    while ((data_rd & DMA_STATUS_DONE_MASK) != DMA_STATUS_DONE_MASK)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 11: Polling DMA status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_reg(DMA_STATUS);
    }
    LOGI("Step 11: DMA transfer complete, DMA_STATUS=0x%x\n", data_rd);

    /* Step 12: Read back destination memory and verify data matches expected pattern */
    data_rd = read_reg(DMA_CH0_DST_ADDR_VAL);
    if (data_rd != DMA_EXPECTED_DATA_PATTERN)
    {
        LOGI("ERROR: Step 12: Data integrity check failed, read=0x%x expected=0x%x\n", data_rd, DMA_EXPECTED_DATA_PATTERN);
        test_err++;
    }
    else
    {
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: Step 12: Data integrity verified at destination, data=0x%x\n", data_rd);
        #endif
    }

    /* Step 13: Write system-level control registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 13: System-level control registers written with 0x1\n");
    #endif

    /* Step 14: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(COMPLETION_SYNC_REG);
    while (data_rd != COMPLETION_SYNC_VALUE)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 14: Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(5);
        data_rd = read_reg(COMPLETION_SYNC_REG);
    }
    LOGI("Step 14: Completion sync confirmed, data_rd=0x%x\n", data_rd);

    /* Step 15: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_dma_write_test_teardown
 * Description: Performs validation, cleanup, and final observation for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe DMA write test: %s\n", cfg->test_name);

    return 0;
}
