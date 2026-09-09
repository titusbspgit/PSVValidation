// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/*
 * Test Case: pcie_dma_write_test
 * Description: Validates PCIe DMA write and read operations across all four DMA
 *   channels (0-3) for both dual-mode controllers (DM0 and DM1). Includes link
 *   training, BAR/memory base programming, source memory preload, DMA channel
 *   programming with doorbell trigger, interrupt-driven completion via int_pend,
 *   and IRQ handler for DMA interrupt status read/clear.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
} pcie_dma_test_ctx_t;

static pcie_dma_test_ctx_t g_ctx;

/* Volatile interrupt pending flag shared with IRQ handler */
static volatile unsigned int int_pend;

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for DMA write/read interrupt status read, clear, and
 *   GIC interrupt clear. Sets int_pend=0 to signal DMA completion.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
void Default_IRQHandler(void)
{
    unsigned int dma_wr_int_sts;
    unsigned int dma_rd_int_sts;

    /* Step 21: Under DM0_RC: read and clear DMA interrupt status */
#ifdef DM0_RC
    dma_wr_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & 0x0000000FU;
    dma_rd_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF) & 0x0000000FU;

    if (dma_wr_int_sts != 0U) {
        write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
        LOGT("DM0 DMA write interrupt cleared: sts=0x%x", dma_wr_int_sts);
    }
    if (dma_rd_int_sts != 0U) {
        write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);
        LOGT("DM0 DMA read interrupt cleared: sts=0x%x", dma_rd_int_sts);
    }

    GIC_ClearIRQ(0x20);
#endif /* DM0_RC */

    /* Step 22: Under DM1_RC: read and clear DMA interrupt status */
#ifdef DM1_RC
    dma_wr_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & 0x0000000FU;
    dma_rd_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF) & 0x0000000FU;

    if (dma_wr_int_sts != 0U) {
        write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
        LOGT("DM1 DMA write interrupt cleared: sts=0x%x", dma_wr_int_sts);
    }
    if (dma_rd_int_sts != 0U) {
        write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);
        LOGT("DM1 DMA read interrupt cleared: sts=0x%x", dma_rd_int_sts);
    }

    GIC_ClearIRQ(0x23);
#endif /* DM1_RC */

    /* Step 23: Signal DMA completion */
    int_pend = 0U;
}

/*
 * Helper: wait_int_pend
 * Description: Polls int_pend flag with timeout. Returns 0 on success, -1 on timeout.
 */
static int wait_int_pend(void)
{
    unsigned int timeout = PCIE_DMA_POLL_TIMEOUT;

    while ((int_pend != 0U) && (timeout > 0U)) {
        wait_on(1);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA interrupt completion");
        g_ctx.errors++;
        return -1;
    }
    return 0;
}

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
    unsigned int data_rd;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (pcie_dma_test_ctx_t){0};
    int_pend = 0U;

    LOGT("pcie_dma_write_test init: starting PCIe DMA write/read test setup");

    /* Step 1: Write 0x0 to 0xE6004100 to initialize synchronization register */
    write_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

    /* Step 2: Conditionally call link training based on compile-time defines */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
    LOGT("PCIe link training DM0 x4 initiated");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
    LOGT("PCIe link training DM1 x4 initiated");
#endif

    /* Step 3: Under DM0_RC: poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
#ifdef DM0_RC
    timeout = PCIE_DMA_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling read_sii0_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("DM0 link status OK: read_sii0_reg(0xC0)=0x%x", data_rd);
    }

    /* Step 4: Read Vendor ID, enable command register */
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("DM0 Vendor ID from pcie_slv0 reg 0x0 = 0x%x", data_rd);

    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("DM0 command register written: pcie_slv0 reg 0x4 = 0x7");

    /* Step 5: BAR program, wait, memory base program for DM0 */
    bar_program_dm0_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("DM0 BAR and memory base programming complete");
#endif /* DM0_RC */

    /* Step 6: Under DM1_RC: poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
#ifdef DM1_RC
    timeout = PCIE_DMA_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling read_sii1_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("DM1 link status OK: read_sii1_reg(0xC0)=0x%x", data_rd);
    }

    /* Step 7: Read Vendor ID, enable command register, BAR/mem base for DM1 */
    data_rd = read_pcie_slv1_reg(0x0);
    LOGT("DM1 Vendor ID from pcie_slv1 reg 0x0 = 0x%x", data_rd);

    write_pcie_slv1_reg(0x4, 0x7);
    LOGT("DM1 command register written: pcie_slv1 reg 0x4 = 0x7");

    bar_program_dm1_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
    LOGT("DM1 BAR and memory base programming complete");
#endif /* DM1_RC */

    /* Step 8: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");

    /* Step 9: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    timeout = PCIE_DMA_POLL_TIMEOUT;
    data_rd = read_reg(PCIE_SYNC_REG);
    while ((data_rd != PCIE_SYNC_EXPECTED) && (timeout > 0U)) {
        wait_on(5);
        data_rd = read_reg(PCIE_SYNC_REG);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sync register 0x%lx, data_rd=0x%x, expected=0x%x",
             (unsigned long)PCIE_SYNC_REG, data_rd, PCIE_SYNC_EXPECTED);
        g_ctx.errors++;
    } else {
        LOGT("Sync register matched: 0x%x", data_rd);
    }

    LOGT("pcie_dma_write_test init complete");

    return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Executes the main testcase flow for pcie_dma_write_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int i;
    unsigned int len;
    unsigned int src_addr0;
    unsigned int wr_addr0, wr_addr1, wr_addr2, wr_addr3;
    unsigned int rd_addr0, rd_addr1, rd_addr2, rd_addr3;
    unsigned int dst_addr0, dst_addr1, dst_addr2, dst_addr3;
    volatile unsigned int *src_ptr;

    (void)cfg;

    if (out == 0) {
        LOGE("pcie_dma_write_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("pcie_dma_write_test run: starting DMA operations");

    /* Step 10: Configure DMA transfer parameters */
    len = PCIE_DMA_XFER_LEN;
    src_addr0 = PCIE_DMA_SRC_ADDR0;

#ifdef DM0_RC
    wr_addr0 = 0xA7000000U;
    wr_addr1 = 0xA7100000U;
    wr_addr2 = 0xA7200000U;
    wr_addr3 = 0xA7300000U;
    rd_addr0 = 0xA7000000U;
    rd_addr1 = 0xA7100000U;
    rd_addr2 = 0xA7200000U;
    rd_addr3 = 0xA7300000U;
#endif /* DM0_RC */

#ifdef DM1_RC
    wr_addr0 = 0xC7000000U;
    wr_addr1 = 0xC7100000U;
    wr_addr2 = 0xC7200000U;
    wr_addr3 = 0xC7300000U;
    rd_addr0 = 0xC7000000U;
    rd_addr1 = 0xC7100000U;
    rd_addr2 = 0xC7200000U;
    rd_addr3 = 0xC7300000U;
#endif /* DM1_RC */

    dst_addr0 = PCIE_DMA_DST_ADDR0;
    dst_addr1 = PCIE_DMA_DST_ADDR1;
    dst_addr2 = PCIE_DMA_DST_ADDR2;
    dst_addr3 = PCIE_DMA_DST_ADDR3;

    LOGT("DMA params: len=0x%x src_addr0=0x%x", len, src_addr0);

    /* Step 11: Preload source memory with test patterns */
    src_ptr = (volatile unsigned int *)(uintptr_t)src_addr0;
    for (i = 0U; i < PCIE_DMA_PRELOAD_WORDS; i++) {
        src_ptr[i] = PCIE_DMA_PATTERN1;
    }
    for (i = 0U; i < PCIE_DMA_PRELOAD_WORDS; i++) {
        src_ptr[PCIE_DMA_PRELOAD_OFFSET_WORDS + i] = PCIE_DMA_PATTERN2;
    }
    LOGT("Source memory preloaded: 128 words of 0x%x, 128 words of 0x%x",
         PCIE_DMA_PATTERN1, PCIE_DMA_PATTERN2);

    /* Step 12: Set int_pend=1, enable GIC */
    int_pend = 1U;
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("GIC configured and all IRQs enabled");

    /* Steps 13-17: DMA write and read channels under DM0_RC */
#ifdef DM0_RC
    /* Step 13: Unmask DMA interrupts for DM0 */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM0 DMA write/read interrupt masks cleared");

    /* Step 14: DMA write channel 0 */
    program_dma_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DM0 DMA write ch0 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* Step 15: DMA write channel 1 */
    program_dma_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DM0 DMA write ch1 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA write channel 2 */
    program_dma_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DM0 DMA write ch2 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA write channel 3 */
    program_dma_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DM0 DMA write ch3 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* Step 16: DMA read channel 0 */
    program_dma_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DM0 DMA read ch0 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* Step 17: DMA read channel 1 */
    program_dma_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DM0 DMA read ch1 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA read channel 2 */
    program_dma_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DM0 DMA read ch2 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA read channel 3 */
    program_dma_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DM0 DMA read ch3 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    LOGT("DM0 DMA write/read all channels complete");
#endif /* DM0_RC */

    /* Steps 18-20: DMA write and read channels under DM1_RC */
#ifdef DM1_RC
    /* Step 18: Unmask DMA interrupts for DM1 */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM1 DMA write/read interrupt masks cleared");

    /* Step 19: DMA write channel 0 */
    program_dma1_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA write ch0 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA write channel 1 */
    program_dma1_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA write ch1 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA write channel 2 */
    program_dma1_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA write ch2 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA write channel 3 */
    program_dma1_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA write ch3 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* Step 20: DMA read channel 0 */
    program_dma1_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA read ch0 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA read channel 1 */
    program_dma1_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA read ch1 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA read channel 2 */
    program_dma1_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA read ch2 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    /* DMA read channel 3 */
    program_dma1_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA read ch3 doorbell triggered");
    (void)wait_int_pend();
    int_pend = 1U;

    LOGT("DM1 DMA write/read all channels complete");
#endif /* DM1_RC */

    /* Step 24: Final wait and status */
    wait_on(10);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to out->status based PASS/FAIL reporting.
    if (g_ctx.errors == 0U) {
        out->status = 0;
    } else {
        out->status = -1;
    }

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
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

    LOGT("pcie_dma_write_test teardown: errors=%u", g_ctx.errors);

    /* Validation summary */
    // Validation criteria 1: Link status polling was performed in init phase.
    // Validation criteria 2: Vendor ID was read and logged in init phase.
    // Validation criteria 3: Sync register poll was performed in init phase.
    // Validation criteria 4-5: DMA write/read channels 0-3 confirmed via int_pend mechanism in run phase.
    // Validation criteria 6-7: IRQ handler reads/clears DMA interrupt status and GIC interrupt.
    // Validation criteria 8: finish(0) converted to PSV/FV-native status.

    if (g_ctx.errors != 0U) {
        LOGE("pcie_dma_write_test FAILED with %u errors", g_ctx.errors);
    } else {
        LOGT("pcie_dma_write_test PASSED");
    }

    return g_ctx.errors == 0U ? 0 : -1;
}
