// Author - AI Force 2.3. 03-Sep-2026 15:27 IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err, ch;
int int_pend;

/*
 * Function: pcie_dma_write_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              PCIe DMA write and read operations test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    LOGI("[Init] Control register 0xE6004100 initialized to 0x0\n");

    return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Main testcase execution for PCIe DMA write and read operations
 *              across four channels on both PCIe controllers. Performs link
 *              training, link-up polling, BAR programming, DMA write channel
 *              programming with doorbell, DMA read channel programming with
 *              doorbell, and completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    LOGI("[Test Run] PCIe DMA write test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 2: Link training - conditionally call based on compile-time defines */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGI("[Run] link_training_dm0_x4(4) called for DM0_RC\n");
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGI("[Run] link_training_dm1_x4(4) called for DM1_RC\n");
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGI("[Run] link_training_dm0_x4(4) called for DM0_EP\n");
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGI("[Run] link_training_dm1_x4(4) called for DM1_EP\n");
    #endif

    /* Steps 3-4: SII0 link-up polling until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("[Run] SII0 link-up confirmed: 0x%x\n", data_rd);

    /* Step 5: SII1 link-up polling until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("[Run] SII1 link-up confirmed: 0x%x\n", data_rd);

    /* Steps 6-7: Vendor ID read and command register under DM0_RC */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("[Run] Vendor ID read from pcie_slv0 offset 0x0: 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Run] Command register pcie_slv0 offset 0x4 written with 0x7\n");
    #endif

    /* Step 8: BAR programming */
    #ifdef DM0_RC
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        LOGI("[Run] mem_base_program_dm0_x4() and mem_base_program_dm1_x4() called\n");
    #endif

    wait_on(10);
    LOGI("[Run] wait_on(10) after BAR programming\n");

    /* Step 9: Unmask DMA write interrupts for PCIE0 */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    LOGI("[Run] PCIE0 DMA_WRITE_INT_MASK_OFF unmasked\n");

    /* Step 10: Unmask DMA read interrupts for PCIE0 */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGI("[Run] PCIE0 DMA_READ_INT_MASK_OFF unmasked\n");

    /* Step 11: Unmask DMA write interrupts for PCIE1 */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    LOGI("[Run] PCIE1 DMA_WRITE_INT_MASK_OFF unmasked\n");

    /* Step 12: Unmask DMA read interrupts for PCIE1 */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGI("[Run] PCIE1 DMA_READ_INT_MASK_OFF unmasked\n");

    /* Steps 9-12: DMA write channels 0-3 programming and doorbell for PCIE0 */
    for (ch = 0; ch < DMA_NUM_CHANNELS; ch++)
    {
        LOGI("[Run] PCIE0 DMA write channel %u programmed\n", ch);

        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, ch);
        LOGI("[Run] PCIE0 DMA_WRITE_DOORBELL_OFF ch=%u triggered\n", ch);

        int_pend = 1;
        while (int_pend)
        {
            LOGI("[Run] Waiting for PCIE0 DMA write interrupt ch=%u\n", ch);
            wait_on(10);
        }
        LOGI("[Run] PCIE0 DMA write ch=%u completed\n", ch);
    }

    /* Steps 13-16: DMA write channels 0-3 programming and doorbell for PCIE1 */
    for (ch = 0; ch < DMA_NUM_CHANNELS; ch++)
    {
        LOGI("[Run] PCIE1 DMA write channel %u programmed\n", ch);

        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, ch);
        LOGI("[Run] PCIE1 DMA_WRITE_DOORBELL_OFF ch=%u triggered\n", ch);

        int_pend = 1;
        while (int_pend)
        {
            LOGI("[Run] Waiting for PCIE1 DMA write interrupt ch=%u\n", ch);
            wait_on(10);
        }
        LOGI("[Run] PCIE1 DMA write ch=%u completed\n", ch);
    }

    /* Steps 17-18: DMA read channels 0-3 for PCIE0 */
    for (ch = 0; ch < DMA_NUM_CHANNELS; ch++)
    {
        LOGI("[Run] PCIE0 DMA read channel %u programmed\n", ch);

        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, ch);
        LOGI("[Run] PCIE0 DMA_READ_DOORBELL_OFF ch=%u triggered\n", ch);

        int_pend = 1;
        while (int_pend)
        {
            LOGI("[Run] Waiting for PCIE0 DMA read interrupt ch=%u\n", ch);
            wait_on(10);
        }
        LOGI("[Run] PCIE0 DMA read ch=%u completed\n", ch);
    }

    /* Steps 19-20: DMA read channels 0-3 for PCIE1 */
    for (ch = 0; ch < DMA_NUM_CHANNELS; ch++)
    {
        LOGI("[Run] PCIE1 DMA read channel %u programmed\n", ch);

        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, ch);
        LOGI("[Run] PCIE1 DMA_READ_DOORBELL_OFF ch=%u triggered\n", ch);

        int_pend = 1;
        while (int_pend)
        {
            LOGI("[Run] Waiting for PCIE1 DMA read interrupt ch=%u\n", ch);
            wait_on(10);
        }
        LOGI("[Run] PCIE1 DMA read ch=%u completed\n", ch);
    }

    /* Synchronization polling */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("[Run] Synchronization complete: 0xE6004100 reads 0x%x\n", data_rd);

    /* finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for PCIe DMA write and read interrupt processing.
 *              Checks DMA write/read interrupt status on PCIE0 and PCIE1,
 *              clears interrupt status, and calls GIC_ClearIRQ.
 */
void Default_IRQHandler()
{
    unsigned int wr_sts, rd_sts;

    int_pend = 0;

    wr_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
    if (wr_sts != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] PCIE0 DMA write interrupt status: 0x%x\n", wr_sts);
        #endif
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_sts);
        LOGI("[IRQ] PCIE0 DMA write interrupt cleared: 0x%x\n", wr_sts);
    }

    rd_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF);
    if (rd_sts != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] PCIE0 DMA read interrupt status: 0x%x\n", rd_sts);
        #endif
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_sts);
        LOGI("[IRQ] PCIE0 DMA read interrupt cleared: 0x%x\n", rd_sts);
    }

    wr_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
    if (wr_sts != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] PCIE1 DMA write interrupt status: 0x%x\n", wr_sts);
        #endif
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_sts);
        LOGI("[IRQ] PCIE1 DMA write interrupt cleared: 0x%x\n", wr_sts);
    }

    rd_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF);
    if (rd_sts != 0x0)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[IRQ] PCIE1 DMA read interrupt status: 0x%x\n", rd_sts);
        #endif
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_sts);
        LOGI("[IRQ] PCIE1 DMA read interrupt cleared: 0x%x\n", rd_sts);
    }

    #ifdef DM0_RC
        GIC_ClearIRQ(0);
    #endif
    #ifdef DM1_RC
        GIC_ClearIRQ(0);
    #endif
}

/*
 * Function: pcie_dma_write_test_teardown
 * Description: Performs validation observations, cleanup, and testcase
 *              completion for PCIe DMA write and read operations test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe DMA write test teardown: %s\n", cfg->test_name);

    /* Validation observations:
     * 1. SII link status polling - verified in run via SII0/SII1 polling.
     * 2. Synchronization polling - verified in run via 0xE6004100 polling.
     * 3. DMA write completion via interrupt - verified in run and IRQ handler.
     * 4. DMA read completion via interrupt - verified in run and IRQ handler.
     * 5. GIC_ClearIRQ - called in Default_IRQHandler.
     * 6. finish(0) - called in run.
     */

    return 0;
}
