// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/* Global variables */
unsigned int data_rd;
unsigned int test_err;
volatile int int_pend;
unsigned int len;
unsigned int src_addr0;
unsigned int wr_addr0;
unsigned int rd_addr0;
unsigned int dst_addr0;
unsigned int i;

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
    LOGI("[Test Init] PCIE DMA write test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Executes the main testcase flow for pcie_dma_write_test including link training,
 *              BAR programming, source data preload, GIC setup, DMA write channels 0-3,
 *              DMA read channels 0-3, and interrupt-driven completion.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIE DMA write test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize control register */
    LOGI("Step 1: Initialize control register at 0xE6004100 to 0x0\n");
    write_reg(0xE6004100, 0x0);

    /* Step 2: Link training based on compile-time defines */
    LOGI("Step 2: Perform PCIe link training in x4 configuration\n");
    #if defined(DM0_RC) || defined(DM0_EP)
        link_training_dm0_x4(4);
    #endif
    #if defined(DM1_RC) || defined(DM1_EP)
        link_training_dm1_x4(4);
    #endif

    /* Step 3: Under DM0_RC - Poll SII0 for link readiness */
    #ifdef DM0_RC
        LOGI("Step 3: DM0_RC - Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
        do {
            data_rd = read_sii0_reg(0xC0);
        } while ((data_rd & 0xD1) != 0xD1);
        #ifdef DEBUG_DISPLAY
            LOGI("SII0 link ready confirmed, data_rd=0x%08x\n", data_rd);
        #endif

        /* Step 4: Under DM0_RC - Read Vendor ID, enable mem/bus master, BAR program */
        LOGI("Step 4: DM0_RC - Read Vendor ID, enable mem/bus master, BAR program\n");
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("Vendor ID = 0x%08x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);

        bar_program_dm0_x4();
        wait_on(10);
        mem_base_program_dm0_x4();
    #endif

    /* Step 5: Under DM1_RC - Poll SII1 for link readiness */
    #ifdef DM1_RC
        LOGI("Step 5: DM1_RC - Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
        do {
            data_rd = read_sii1_reg(0xC0);
        } while ((data_rd & 0xD1) != 0xD1);
        #ifdef DEBUG_DISPLAY
            LOGI("SII1 link ready confirmed, data_rd=0x%08x\n", data_rd);
        #endif

        /* Step 6: Under DM1_RC - Read Vendor ID, enable mem/bus master, BAR program */
        LOGI("Step 6: DM1_RC - Read Vendor ID, enable mem/bus master, BAR program\n");
        data_rd = read_pcie_slv1_reg(0x0);
        LOGI("Vendor ID = 0x%08x\n", data_rd);

        write_pcie_slv1_reg(0x4, 0x7);

        bar_program_dm1_x4();
        wait_on(10);
        mem_base_program_dm1_x4();
    #endif

    /* Step 7: Configure non-secure protection */
    LOGI("Step 7: Call non_secure_prot_nic()\n");
    non_secure_prot_nic();

    /* Step 8: Poll synchronization register */
    LOGI("Step 8: Poll 0xE6004100 until value == 0x12345678\n");
    do {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    } while (data_rd != 0x12345678);
    LOGI("Synchronization handshake received: 0x%08x\n", data_rd);

    /* Step 9: Configure DMA transfer parameters */
    LOGI("Step 9: Configure DMA transfer parameters\n");
    len = 0x40;
    src_addr0 = 0xE6000000;
    #ifdef DM0_RC
        wr_addr0 = 0x01000000;
        rd_addr0 = 0x01000000;
        dst_addr0 = 0xE6001000;
    #endif
    #ifdef DM1_RC
        wr_addr0 = 0x01000000;
        rd_addr0 = 0x01000000;
        dst_addr0 = 0xE6001000;
    #endif

    /* Step 10: Preload source data */
    LOGI("Step 10: Preload source memory with known data patterns\n");
    for (i = 0; i < 128; i++) {
        write_reg(src_addr0 + (i * 4), 0xC0DEBEED);
    }
    for (i = 0; i < 128; i++) {
        write_reg(src_addr0 + 400 + (i * 4), 0xF00DDEAF);
    }

    /* Step 11: Configure GIC */
    LOGI("Step 11: Configure GIC and enable all IRQs\n");
    int_pend = 1;
    GIC_Set();
    GIC_EnableAllIRQ();

    /* Steps 12-16: DMA write and read channels under DM0_RC */
    #ifdef DM0_RC
        /* Step 12: Unmask DMA interrupts for DM0 */
        LOGI("Step 12: DM0_RC - Unmask DMA write and read interrupts\n");
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);

        /* Step 13: DMA Write Channel 0 */
        LOGI("Step 13: DMA Write Channel 0 - program and trigger\n");
        program_dma_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch0 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Write Channel 0 complete\n");
        #endif
        int_pend = 1;

        /* Step 14a: DMA Write Channel 1 */
        LOGI("Step 14a: DMA Write Channel 1 - program and trigger\n");
        program_dma_wch1(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch1 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Write Channel 1 complete\n");
        #endif
        int_pend = 1;

        /* Step 14b: DMA Write Channel 2 */
        LOGI("Step 14b: DMA Write Channel 2 - program and trigger\n");
        program_dma_wch2(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch2 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Write Channel 2 complete\n");
        #endif
        int_pend = 1;

        /* Step 14c: DMA Write Channel 3 */
        LOGI("Step 14c: DMA Write Channel 3 - program and trigger\n");
        program_dma_wch3(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch3 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Write Channel 3 complete\n");
        #endif
        int_pend = 1;

        /* Step 15: DMA Read Channel 0 */
        LOGI("Step 15: DMA Read Channel 0 - program and trigger\n");
        program_dma_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch0 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Read Channel 0 complete\n");
        #endif
        int_pend = 1;

        /* Step 16a: DMA Read Channel 1 */
        LOGI("Step 16a: DMA Read Channel 1 - program and trigger\n");
        program_dma_rch1(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch1 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Read Channel 1 complete\n");
        #endif
        int_pend = 1;

        /* Step 16b: DMA Read Channel 2 */
        LOGI("Step 16b: DMA Read Channel 2 - program and trigger\n");
        program_dma_rch2(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch2 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Read Channel 2 complete\n");
        #endif
        int_pend = 1;

        /* Step 16c: DMA Read Channel 3 */
        LOGI("Step 16c: DMA Read Channel 3 - program and trigger\n");
        program_dma_rch3(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch3 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DMA Read Channel 3 complete\n");
        #endif
        int_pend = 1;
    #endif /* DM0_RC */

    /* Step 17: DMA write and read channels under DM1_RC */
    #ifdef DM1_RC
        /* Unmask DMA interrupts for DM1 */
        LOGI("Step 17: DM1_RC - Unmask DMA write and read interrupts\n");
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);

        /* DMA Write Channel 0 for DM1 */
        LOGI("DM1_RC: DMA Write Channel 0 - program and trigger\n");
        program_dma1_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch0 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Write Channel 0 complete\n");
        #endif
        int_pend = 1;

        /* DMA Write Channel 1 for DM1 */
        LOGI("DM1_RC: DMA Write Channel 1 - program and trigger\n");
        program_dma1_wch1(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch1 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Write Channel 1 complete\n");
        #endif
        int_pend = 1;

        /* DMA Write Channel 2 for DM1 */
        LOGI("DM1_RC: DMA Write Channel 2 - program and trigger\n");
        program_dma1_wch2(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch2 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Write Channel 2 complete\n");
        #endif
        int_pend = 1;

        /* DMA Write Channel 3 for DM1 */
        LOGI("DM1_RC: DMA Write Channel 3 - program and trigger\n");
        program_dma1_wch3(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
        while (int_pend) {
            LOGI("Waiting for DMA Write Ch3 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Write Channel 3 complete\n");
        #endif
        int_pend = 1;

        /* DMA Read Channel 0 for DM1 */
        LOGI("DM1_RC: DMA Read Channel 0 - program and trigger\n");
        program_dma1_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch0 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Read Channel 0 complete\n");
        #endif
        int_pend = 1;

        /* DMA Read Channel 1 for DM1 */
        LOGI("DM1_RC: DMA Read Channel 1 - program and trigger\n");
        program_dma1_rch1(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch1 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Read Channel 1 complete\n");
        #endif
        int_pend = 1;

        /* DMA Read Channel 2 for DM1 */
        LOGI("DM1_RC: DMA Read Channel 2 - program and trigger\n");
        program_dma1_rch2(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch2 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Read Channel 2 complete\n");
        #endif
        int_pend = 1;

        /* DMA Read Channel 3 for DM1 */
        LOGI("DM1_RC: DMA Read Channel 3 - program and trigger\n");
        program_dma1_rch3(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
        while (int_pend) {
            LOGI("Waiting for DMA Read Ch3 interrupt\n");
            wait_on(10);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("SUCCESS: DM1 DMA Read Channel 3 complete\n");
        #endif
        int_pend = 1;
    #endif /* DM1_RC */

    /* Step 19: Final wait */
    LOGI("Step 19: wait_on(10)\n");
    wait_on(10);

    /* Step 20: Test complete */
    LOGI("Step 20: Test complete, calling finish(0)\n");
    finish(0);

    return out->status = test_err;
}

/*
 * Step 18: Default_IRQHandler
 * Description: Interrupt handler for DMA write/read completion.
 *              Reads DMA interrupt status, clears interrupts, clears GIC.
 */
void Default_IRQHandler()
{
    unsigned int dma_wr_int_sts = 0;
    unsigned int dma_rd_int_sts = 0;

    int_pend = 0;

    #ifdef DM0_RC
        dma_wr_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & 0x0000000F;
        dma_rd_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF) & 0x0000000F;

        #ifdef DEBUG_DISPLAY
            LOGI("DM0 IRQHandler: dma_wr_int_sts=0x%08x dma_rd_int_sts=0x%08x\n", dma_wr_int_sts, dma_rd_int_sts);
        #endif

        if (dma_wr_int_sts) {
            write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
        }
        if (dma_rd_int_sts) {
            write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);
        }
        GIC_ClearIRQ(0x20);
    #endif

    #ifdef DM1_RC
        dma_wr_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & 0x0000000F;
        dma_rd_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF) & 0x0000000F;

        #ifdef DEBUG_DISPLAY
            LOGI("DM1 IRQHandler: dma_wr_int_sts=0x%08x dma_rd_int_sts=0x%08x\n", dma_wr_int_sts, dma_rd_int_sts);
        #endif

        if (dma_wr_int_sts) {
            write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
        }
        if (dma_rd_int_sts) {
            write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);
        }
        GIC_ClearIRQ(0x23);
    #endif
}

/*
 * Function: pcie_dma_write_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIE DMA write test: %s\n", cfg->test_name);

    return 0;
}
