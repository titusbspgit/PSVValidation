/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

#include <stdio.h>
#include "test_define.c"

/* File-scope variables for cross-function access */
static int error_count = 0;
static unsigned int data_rd = 0;
static unsigned int len = 0;
static unsigned int src_addr0 = 0;
static unsigned int dst_addr0 = 0;
static unsigned int dst_addr1 = 0;
static unsigned int dst_addr2 = 0;
static unsigned int dst_addr3 = 0;
static unsigned int wr_addr0 = 0;
static unsigned int wr_addr1 = 0;
static unsigned int wr_addr2 = 0;
static unsigned int wr_addr3 = 0;
static unsigned int rd_addr0 = 0;
static unsigned int rd_addr1 = 0;
static unsigned int rd_addr2 = 0;
static unsigned int rd_addr3 = 0;

/* Global interrupt pending flag */
volatile int int_pend = 1;

/********************************************************************
 * Function Name  : Default_IRQHandler
 * Description    : Handle DMA completion interrupts. Reads DMA write
 *                  and read interrupt status, clears interrupts, and
 *                  signals completion via int_pend flag.
 * Parameters     : None.
 * Return Value   : None.
 ********************************************************************/
void Default_IRQHandler(void)
{
    unsigned int dma_wr_int_sts = 0;
    unsigned int dma_rd_int_sts = 0;

    /* Set int_pend to 0 to signal completion */
    int_pend = 0;

#ifdef DM0_RC
    /* Read DMA write interrupt status for PCIE0 */
    dma_wr_int_sts = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF;
    dma_wr_int_sts = dma_wr_int_sts & DMA_INT_CHANNEL_MASK;
#ifdef DEBUG_DISPLAY
    debug_print("IRQ: PCIE0 DMA_WRITE_INT_STATUS = 0x%08X\n", dma_wr_int_sts);
#endif

    /* Read DMA read interrupt status for PCIE0 */
    dma_rd_int_sts = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF;
    dma_rd_int_sts = dma_rd_int_sts & DMA_INT_CHANNEL_MASK;
#ifdef DEBUG_DISPLAY
    debug_print("IRQ: PCIE0 DMA_READ_INT_STATUS = 0x%08X\n", dma_rd_int_sts);
#endif

    /* Clear DMA write interrupt for PCIE0 */
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF = dma_wr_int_sts;

    /* Clear DMA read interrupt for PCIE0 */
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF = dma_rd_int_sts;

    /* Clear GIC IRQ for PCIE0 DMA */
    GIC_ClearIRQ(GIC_IRQ_PCIE0);
#endif /* DM0_RC */

#ifdef DM1_RC
    /* Read DMA write interrupt status for PCIE1 */
    dma_wr_int_sts = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF;
    dma_wr_int_sts = dma_wr_int_sts & DMA_INT_CHANNEL_MASK;
#ifdef DEBUG_DISPLAY
    debug_print("IRQ: PCIE1 DMA_WRITE_INT_STATUS = 0x%08X\n", dma_wr_int_sts);
#endif

    /* Read DMA read interrupt status for PCIE1 */
    dma_rd_int_sts = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF;
    dma_rd_int_sts = dma_rd_int_sts & DMA_INT_CHANNEL_MASK;
#ifdef DEBUG_DISPLAY
    debug_print("IRQ: PCIE1 DMA_READ_INT_STATUS = 0x%08X\n", dma_rd_int_sts);
#endif

    /* Clear DMA write interrupt for PCIE1 */
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF = dma_wr_int_sts;

    /* Clear DMA read interrupt for PCIE1 */
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF = dma_rd_int_sts;

    /* Clear GIC IRQ for PCIE1 DMA */
    GIC_ClearIRQ(GIC_IRQ_PCIE1);
#endif /* DM1_RC */
}

/********************************************************************
 * Function Name  : pcie_dma_write_test_init
 * Description    : Initialize testcase configuration, control register,
 *                  and perform PCIe link training.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful initialization.
 ********************************************************************/
int pcie_dma_write_test_init(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Init] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 1: Write 0x0 to 0xE6004100 to initialize control register */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)CTRL_REG_ADDR = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 1: Wrote 0x0 to CTRL_REG_ADDR (0xE6004100)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 2: Perform x4 link training based on compile-time defines */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
#endif
#endif

#ifdef DM1_RC
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
#endif
#endif

#ifdef DM0_EP
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm0_x4(4) called (DM0_EP)\n");
#endif
#endif

#ifdef DM1_EP
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm1_x4(4) called (DM1_EP)\n");
#endif
#endif

    return 0;
}

/********************************************************************
 * Function Name  : pcie_dma_write_test_run
 * Description    : Execute testcase register configuration and stimulus.
 *                  Polls link-up, reads Vendor ID, enables memory/IO/bus
 *                  master, programs BARs and memory base, preloads source
 *                  memory, initializes GIC, unmasks DMA interrupts, and
 *                  triggers all 4 DMA write and read channels via doorbell
 *                  registers with interrupt-driven completion handshaking.
 * Parameters     : cfg - Test configuration pointer,
 *                  out - Test output pointer.
 * Return Value   : Test execution status.
 ********************************************************************/
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    int i = 0;
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Run] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 3: (DM0_RC) Poll SII0 link status until link-up        */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 3: SII0 link-up confirmed, data_rd = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 4: (DM0_RC) Read Vendor ID from PCIe slave 0            */
    /* ------------------------------------------------------------ */
    data_rd = read_pcie_slv0_reg(PCIE_SLV_VENDOR_ID_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 4: Vendor ID from slv0 = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 5: (DM0_RC) Write 0x7 to enable mem/IO/bus master       */
    /* ------------------------------------------------------------ */
    write_pcie_slv0_reg(PCIE_SLV_CMD_STATUS_OFFSET, PCIE_CMD_MEM_IO_BUSMASTER);
#ifdef DEBUG_DISPLAY
    debug_print("Step 5: Wrote 0x7 to slv0 TYPE1_STATUS_COMMAND_REG\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 6: (DM0_RC) Program BARs                                */
    /* ------------------------------------------------------------ */
    bar_program_dm0_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 6: bar_program_dm0_x4() called\n");
#endif
#endif /* DM0_RC */

    /* ------------------------------------------------------------ */
    /* Step 7: Wait for BAR programming to settle                   */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 7: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 8: (DM0_RC) Program memory base regions                 */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    mem_base_program_dm0_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 8: mem_base_program_dm0_x4() called\n");
#endif
#endif /* DM0_RC */

    /* ------------------------------------------------------------ */
    /* Steps 9-12: (DM1_RC) Link-up, Vendor ID, cmd, BAR, mem base */
    /* ------------------------------------------------------------ */
#ifdef DM1_RC
    /* Step 9: Poll SII1 link status until link-up */
    data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 9: SII1 link-up confirmed, data_rd = 0x%08X\n", data_rd);
#endif

    /* Step 10: Read Vendor ID from PCIe slave 1 */
    data_rd = read_pcie_slv1_reg(PCIE_SLV_VENDOR_ID_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 10: Vendor ID from slv1 = 0x%08X\n", data_rd);
#endif

    /* Step 11: Write 0x7 to enable mem/IO/bus master on slv1 */
    write_pcie_slv1_reg(PCIE_SLV_CMD_STATUS_OFFSET, PCIE_CMD_MEM_IO_BUSMASTER);
#ifdef DEBUG_DISPLAY
    debug_print("Step 11: Wrote 0x7 to slv1 TYPE1_STATUS_COMMAND_REG\n");
#endif

    /* Step 12: Program BARs and memory base for DM1 */
    bar_program_dm1_x4();
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 12: bar_program_dm1_x4() and mem_base_program_dm1_x4() called\n");
#endif
#endif /* DM1_RC */

    /* ------------------------------------------------------------ */
    /* Step 13: Configure non-secure protection settings            */
    /* ------------------------------------------------------------ */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    debug_print("Step 13: non_secure_prot_nic() called\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 14: Poll 0xE6004100 until value equals 0x12345678       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    while (data_rd != SYNC_HANDSHAKE_VALUE)
    {
        wait_on(5);
        data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 14: Synchronization handshake received (0x12345678)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 15: Set DMA transfer parameters                         */
    /* ------------------------------------------------------------ */
    len = DMA_TRANSFER_LEN;
    src_addr0 = SRC_ADDR0;
    dst_addr0 = DST_ADDR0;
    dst_addr1 = DST_ADDR1;
    dst_addr2 = DST_ADDR2;
    dst_addr3 = DST_ADDR3;

#ifdef DM0_RC
    wr_addr0 = DM0_WR_ADDR0;
    wr_addr1 = DM0_WR_ADDR1;
    wr_addr2 = DM0_WR_ADDR2;
    wr_addr3 = DM0_WR_ADDR3;
    rd_addr0 = DM0_RD_ADDR0;
    rd_addr1 = DM0_RD_ADDR1;
    rd_addr2 = DM0_RD_ADDR2;
    rd_addr3 = DM0_RD_ADDR3;
#endif

#ifdef DM1_RC
    wr_addr0 = DM1_WR_ADDR0;
    wr_addr1 = DM1_WR_ADDR1;
    wr_addr2 = DM1_WR_ADDR2;
    wr_addr3 = DM1_WR_ADDR3;
    rd_addr0 = DM1_RD_ADDR0;
    rd_addr1 = DM1_RD_ADDR1;
    rd_addr2 = DM1_RD_ADDR2;
    rd_addr3 = DM1_RD_ADDR3;
#endif

#ifdef DEBUG_DISPLAY
    debug_print("Step 15: DMA parameters set, len=0x%X, src=0x%08X\n", len, src_addr0);
#endif

    /* ------------------------------------------------------------ */
    /* Step 16: Preload source memory with known data patterns      */
    /* ------------------------------------------------------------ */
    for (i = 0; i < 128; i++)
    {
        *(volatile unsigned int *)(src_addr0 + (i * 4)) = SRC_PATTERN_0;
    }
    for (i = 0; i < 128; i++)
    {
        *(volatile unsigned int *)(src_addr0 + 400 + (i * 4)) = SRC_PATTERN_1;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 16: Source memory preloaded with 0xC0DEBEED and 0xF00DDEAF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 17: Initialize GIC and enable all IRQs                  */
    /* ------------------------------------------------------------ */
    int_pend = 1;
    GIC_Set();
    GIC_EnableAllIRQ();
#ifdef DEBUG_DISPLAY
    debug_print("Step 17: GIC initialized and all IRQs enabled\n");
#endif

    /* ============================================================ */
    /* DM0_RC: DMA Write and Read Operations on PCIE0               */
    /* ============================================================ */
#ifdef DM0_RC

    /* ------------------------------------------------------------ */
    /* Step 18: Unmask DMA write interrupts for PCIE0               */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 18: PCIE0 DMA_WRITE_INT_MASK_OFF cleared (unmasked)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 19: Unmask DMA read interrupts for PCIE0                */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 19: PCIE0 DMA_READ_INT_MASK_OFF cleared (unmasked)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 20: DMA Write Channel 0                                 */
    /* ------------------------------------------------------------ */
    program_dma_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 20: DMA write ch0 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 20: DMA write ch0 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 21: DMA Write Channel 1                                 */
    /* ------------------------------------------------------------ */
    program_dma_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x1;
#ifdef DEBUG_DISPLAY
    debug_print("Step 21: DMA write ch1 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 21: DMA write ch1 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 22: DMA Write Channel 2                                 */
    /* ------------------------------------------------------------ */
    program_dma_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x2;
#ifdef DEBUG_DISPLAY
    debug_print("Step 22: DMA write ch2 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 22: DMA write ch2 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 23: DMA Write Channel 3                                 */
    /* ------------------------------------------------------------ */
    program_dma_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x3;
#ifdef DEBUG_DISPLAY
    debug_print("Step 23: DMA write ch3 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 23: DMA write ch3 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 24: DMA Read Channel 0                                  */
    /* ------------------------------------------------------------ */
    program_dma_rch0(0x0, dst_addr0, 0x0, rd_addr0, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 24: DMA read ch0 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 24: DMA read ch0 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 25: DMA Read Channel 1                                  */
    /* ------------------------------------------------------------ */
    program_dma_rch1(0x0, dst_addr1, 0x0, rd_addr1, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x1;
#ifdef DEBUG_DISPLAY
    debug_print("Step 25: DMA read ch1 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 25: DMA read ch1 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 26: DMA Read Channel 2                                  */
    /* ------------------------------------------------------------ */
    program_dma_rch2(0x0, dst_addr2, 0x0, rd_addr2, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x2;
#ifdef DEBUG_DISPLAY
    debug_print("Step 26: DMA read ch2 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 26: DMA read ch2 completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 27: DMA Read Channel 3                                  */
    /* ------------------------------------------------------------ */
    program_dma_rch3(0x0, dst_addr3, 0x0, rd_addr3, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x3;
#ifdef DEBUG_DISPLAY
    debug_print("Step 27: DMA read ch3 triggered via DOORBELL\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 27: DMA read ch3 completed\n");
#endif

#endif /* DM0_RC */

    /* ============================================================ */
    /* DM1_RC: DMA Write and Read Operations on PCIE1               */
    /* ============================================================ */
#ifdef DM1_RC

    /* ------------------------------------------------------------ */
    /* Step 28: Unmask DMA write and read interrupts for PCIE1      */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF = 0x0;
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 28: PCIE1 DMA write/read INT_MASK cleared (unmasked)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 29: DMA Write Channels 0-3 on PCIE1                     */
    /* ------------------------------------------------------------ */
    /* DMA Write Channel 0 */
    program_dma1_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: PCIE1 DMA write ch0 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Write Channel 1 */
    program_dma1_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x1;
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: PCIE1 DMA write ch1 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Write Channel 2 */
    program_dma1_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x2;
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: PCIE1 DMA write ch2 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Write Channel 3 */
    program_dma1_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF = 0x3;
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: PCIE1 DMA write ch3 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* ------------------------------------------------------------ */
    /* Step 30: DMA Read Channels 0-3 on PCIE1                      */
    /* ------------------------------------------------------------ */
    /* DMA Read Channel 0 */
    program_dma1_rch0(0x0, dst_addr0, 0x0, rd_addr0, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 30: PCIE1 DMA read ch0 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Read Channel 1 */
    program_dma1_rch1(0x0, dst_addr1, 0x0, rd_addr1, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x1;
#ifdef DEBUG_DISPLAY
    debug_print("Step 30: PCIE1 DMA read ch1 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Read Channel 2 */
    program_dma1_rch2(0x0, dst_addr2, 0x0, rd_addr2, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x2;
#ifdef DEBUG_DISPLAY
    debug_print("Step 30: PCIE1 DMA read ch2 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

    /* DMA Read Channel 3 */
    program_dma1_rch3(0x0, dst_addr3, 0x0, rd_addr3, 0x0, len);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF = 0x3;
#ifdef DEBUG_DISPLAY
    debug_print("Step 30: PCIE1 DMA read ch3 triggered\n");
#endif
    while (int_pend) { wait_on(10); }
    int_pend = 1;
    wait_on(10);

#endif /* DM1_RC */

    /* ------------------------------------------------------------ */
    /* Step 32: Final wait                                          */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 32: wait_on(10) completed\n");
#endif

    return out->status;
}

/********************************************************************
 * Function Name  : pcie_dma_write_test_teardown
 * Description    : Perform output validation, error handling, and cleanup.
 *                  Evaluates error_count and reports final pass/fail.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful teardown.
 ********************************************************************/
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[TEARDOWN] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 33: Report test completion                               */
    /* ------------------------------------------------------------ */
#ifdef DEBUG_DISPLAY
    debug_print("Test complete. error_count = %d\n", error_count);
#endif

    if (error_count != 0)
    {
#ifdef DEBUG_DISPLAY
        LOGI("ERROR: pcie_dma_write_test FAILED with error_count = %d\n", error_count);
#endif
    }
    else
    {
#ifdef DEBUG_DISPLAY
        LOGI("pcie_dma_write_test PASSED\n");
#endif
    }

    return 0;
}
