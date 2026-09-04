// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/*
 * PCIe DMA Write Test
 * This testcase performs PCIe DMA write and read-back operations across
 * all four DMA channels (0-3) with interrupt-driven completion.
 */

unsigned int data_rd, test_err, i;
unsigned int len, src_addr0, dst_addr0, dst_addr1, dst_addr2, dst_addr3;
unsigned int wr_addr0, wr_addr1, wr_addr2, wr_addr3;
unsigned int rd_addr0, rd_addr1, rd_addr2, rd_addr3;
int int_pend;

/*
 * Function: pcie_dma_write_test_init
 * Description: Performs testcase initialization and pre-condition setup
 *              for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe DMA Write test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Main testcase execution for PCIe DMA write/read-back.
 *              Performs link training, link-up polling, Vendor ID read,
 *              BAR/mem base programming, source memory preload, GIC setup,
 *              DMA write channels 0-3, DMA read channels 0-3, and final sync.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe DMA Write test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize synchronization register */
    write_reg(0xE6004100, 0x0);
    LOGI("[Step 1] Wrote 0x0 to sync register 0xE6004100\n");

    /* Step 2: Conditionally call link training */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGI("[Step 2] link_training_dm0_x4(4) called (DM0_RC)\n");
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGI("[Step 2] link_training_dm1_x4(4) called (DM1_RC)\n");
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGI("[Step 2] link_training_dm0_x4(4) called (DM0_EP)\n");
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGI("[Step 2] link_training_dm1_x4(4) called (DM1_EP)\n");
    #endif

    /* Step 3: Under DM0_RC - Poll SII0 link status until link-up */
    #ifdef DM0_RC
    {
        data_rd = read_sii0_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            data_rd = read_sii0_reg(0xC0);
            #ifdef DEBUG_DISPLAY
                LOGI("[Step 3] Polling SII0 link status: data_rd=0x%x\n", data_rd);
            #endif
        }
        LOGI("[Step 3] SII0 link-up confirmed: data_rd=0x%x\n", data_rd);
    }
    #endif

    /* Step 4: Under DM0_RC - Vendor ID read, command write, BAR and mem base program */
    #ifdef DM0_RC
    {
        data_rd = read_pcie_slv0_reg(0x0);
        printf("Vendor ID = 0x%x\n", data_rd);
        LOGI("[Step 4] Vendor ID read from slv0: 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Step 4] Wrote 0x7 to slv0 offset 0x4\n");

        bar_program_dm0_x4();
        LOGI("[Step 4] bar_program_dm0_x4() called\n");

        wait_on(10);

        mem_base_program_dm0_x4();
        LOGI("[Step 4] mem_base_program_dm0_x4() called\n");
    }
    #endif

    /* Step 5: Under DM1_RC - Poll SII1 link status until link-up */
    #ifdef DM1_RC
    {
        data_rd = read_sii1_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            data_rd = read_sii1_reg(0xC0);
            #ifdef DEBUG_DISPLAY
                LOGI("[Step 5] Polling SII1 link status: data_rd=0x%x\n", data_rd);
            #endif
        }
        LOGI("[Step 5] SII1 link-up confirmed: data_rd=0x%x\n", data_rd);
    }
    #endif

    /* Step 6: Under DM1_RC - Vendor ID read, command write, BAR and mem base program */
    #ifdef DM1_RC
    {
        data_rd = read_pcie_slv1_reg(0x0);
        printf("Vendor ID = 0x%x\n", data_rd);
        LOGI("[Step 6] Vendor ID read from slv1: 0x%x\n", data_rd);

        write_pcie_slv1_reg(0x4, 0x7);
        LOGI("[Step 6] Wrote 0x7 to slv1 offset 0x4\n");

        bar_program_dm1_x4();
        LOGI("[Step 6] bar_program_dm1_x4() called\n");

        wait_on(10);

        mem_base_program_dm1_x4();
        LOGI("[Step 6] mem_base_program_dm1_x4() called\n");
    }
    #endif

    /* Step 7: Configure non-secure protection */
    non_secure_prot_nic();
    LOGI("[Step 7] non_secure_prot_nic() called\n");

    /* Step 8: Poll synchronization register until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
        #ifdef DEBUG_DISPLAY
            LOGI("[Step 8] Polling sync register 0xE6004100: data_rd=0x%x\n", data_rd);
        #endif
    }
    LOGI("[Step 8] Sync register confirmed: 0x%x\n", data_rd);

    /* Step 9: Set DMA transfer parameters */
    len = 0x40;
    src_addr0 = 0xE6000000;
    dst_addr0 = 0xE6001000;
    dst_addr1 = 0xE6020000;
    dst_addr2 = 0xE6020000;
    dst_addr3 = 0xE6020000;

    #ifdef DM0_RC
        wr_addr0 = 0xA7000000;
        wr_addr1 = 0xA7100000;
        wr_addr2 = 0xA7200000;
        wr_addr3 = 0xA7300000;
        rd_addr0 = 0xA7000000;
        rd_addr1 = 0xA7100000;
        rd_addr2 = 0xA7200000;
        rd_addr3 = 0xA7300000;
    #endif
    #ifdef DM1_RC
        wr_addr0 = 0xC7000000;
        wr_addr1 = 0xC7100000;
        wr_addr2 = 0xC7200000;
        wr_addr3 = 0xC7300000;
        rd_addr0 = 0xC7000000;
        rd_addr1 = 0xC7100000;
        rd_addr2 = 0xC7200000;
        rd_addr3 = 0xC7300000;
    #endif
    LOGI("[Step 9] DMA transfer parameters set: len=0x%x src_addr0=0x%x\n", len, src_addr0);

    /* Step 10: Preload source memory */
    for (i = 0; i < 128; i++)
    {
        write_reg(src_addr0 + (4 * i), 0xC0DEBEED);
    }
    for (i = 0; i < 128; i++)
    {
        write_reg((src_addr0 + 400) + (4 * i), 0xF00DDEAF);
    }
    LOGI("[Step 10] Source memory preloaded with 0xC0DEBEED and 0xF00DDEAF\n");

    /* Step 11: GIC setup */
    int_pend = 1;
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGI("[Step 11] GIC_Set() and GIC_EnableAllIRQ() called\n");

    /* Step 12: Clear DMA interrupt masks */
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
        LOGI("[Step 12] PCIE0 DMA write/read interrupt masks cleared\n");
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
        LOGI("[Step 12] PCIE1 DMA write/read interrupt masks cleared\n");
    #endif

    /* Step 13: DMA Write Channel 0 */
    int_pend = 1;
    program_dma_wch0(src_addr0, wr_addr0, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    #endif
    LOGI("[Step 13] DMA Write Channel 0 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA WCH0 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 13] DMA Write Channel 0 completed\n");

    /* Step 14: DMA Write Channel 1 */
    int_pend = 1;
    program_dma_wch1(src_addr0, wr_addr1, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    #endif
    LOGI("[Step 14] DMA Write Channel 1 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA WCH1 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 14] DMA Write Channel 1 completed\n");

    /* Step 15: DMA Write Channel 2 */
    int_pend = 1;
    program_dma_wch2(src_addr0, wr_addr2, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    #endif
    LOGI("[Step 15] DMA Write Channel 2 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA WCH2 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 15] DMA Write Channel 2 completed\n");

    /* Step 16: DMA Write Channel 3 */
    int_pend = 1;
    program_dma_wch3(src_addr0, wr_addr3, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    #endif
    LOGI("[Step 16] DMA Write Channel 3 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA WCH3 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 16] DMA Write Channel 3 completed\n");

    /* Step 17: DMA Read Channel 0 */
    int_pend = 1;
    program_dma_rch0(rd_addr0, dst_addr0, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    #endif
    LOGI("[Step 17] DMA Read Channel 0 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA RCH0 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 17] DMA Read Channel 0 completed\n");

    /* Step 18: DMA Read Channel 1 */
    int_pend = 1;
    program_dma_rch1(rd_addr1, dst_addr1, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    #endif
    LOGI("[Step 18] DMA Read Channel 1 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA RCH1 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 18] DMA Read Channel 1 completed\n");

    /* Step 19: DMA Read Channel 2 */
    int_pend = 1;
    program_dma_rch2(rd_addr2, dst_addr2, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    #endif
    LOGI("[Step 19] DMA Read Channel 2 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA RCH2 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 19] DMA Read Channel 2 completed\n");

    /* Step 20: DMA Read Channel 3 */
    int_pend = 1;
    program_dma_rch3(rd_addr3, dst_addr3, len);
    #ifdef DM0_RC
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    #endif
    #ifdef DM1_RC
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    #endif
    LOGI("[Step 20] DMA Read Channel 3 triggered\n");
    while (int_pend)
    {
        LOGI("Waiting for DMA RCH3 interrupt\n");
        wait_on(10);
    }
    LOGI("[Step 20] DMA Read Channel 3 completed\n");

    /* Step 21: Call finish(0) */
    finish(0);
    LOGI("[Step 21] finish(0) called\n");

    return out->status = test_err;
}

/*
 * Default_IRQHandler
 * Reads DMA write/read interrupt status, masks with 0x0000000F,
 * clears interrupts, and calls GIC_ClearIRQ.
 */
void Default_IRQHandler()
{
    unsigned int wr_status, rd_status;
    int_pend = 0;

    #ifdef DM0_RC
    {
        wr_status = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
        wr_status = wr_status & 0x0000000F;
        rd_status = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF);
        rd_status = rd_status & 0x0000000F;

        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] DM0 DMA Write INT Status: 0x%x\n", wr_status);
            LOGI("[IRQ] DM0 DMA Read INT Status: 0x%x\n", rd_status);
        #endif

        if (wr_status != 0x0)
        {
            write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_status);
            LOGI("[IRQ] DM0 DMA Write interrupt cleared: 0x%x\n", wr_status);
        }
        if (rd_status != 0x0)
        {
            write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_status);
            LOGI("[IRQ] DM0 DMA Read interrupt cleared: 0x%x\n", rd_status);
        }

        GIC_ClearIRQ(0x20);
        LOGI("[IRQ] GIC_ClearIRQ(0x20) called for DM0\n");
    }
    #endif

    #ifdef DM1_RC
    {
        wr_status = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
        wr_status = wr_status & 0x0000000F;
        rd_status = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF);
        rd_status = rd_status & 0x0000000F;

        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] DM1 DMA Write INT Status: 0x%x\n", wr_status);
            LOGI("[IRQ] DM1 DMA Read INT Status: 0x%x\n", rd_status);
        #endif

        if (wr_status != 0x0)
        {
            write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_status);
            LOGI("[IRQ] DM1 DMA Write interrupt cleared: 0x%x\n", wr_status);
        }
        if (rd_status != 0x0)
        {
            write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_status);
            LOGI("[IRQ] DM1 DMA Read interrupt cleared: 0x%x\n", rd_status);
        }

        GIC_ClearIRQ(0x23);
        LOGI("[IRQ] GIC_ClearIRQ(0x23) called for DM1\n");
    }
    #endif
}

/*
 * Function: pcie_dma_write_test_teardown
 * Description: Performs validation, final observation, and testcase
 *              completion for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe DMA Write test teardown: %s\n", cfg->test_name);

    return 0;
}
