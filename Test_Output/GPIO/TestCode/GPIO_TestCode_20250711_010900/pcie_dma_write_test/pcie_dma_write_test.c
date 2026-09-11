// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_dma_write_test
 * Description: Validates PCIe DMA write and read operations across all four
 *   DMA channels (0-3) for both dual-mode controllers (DM0 and DM1).
 *   Includes link training, BAR/memory base setup, source memory preload,
 *   interrupt-driven DMA write/read with int_pend synchronization,
 *   and IRQ handler for DMA interrupt status clear.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_dma_test_ctx_t;

static pcie_dma_test_ctx_t g_ctx;

/* Volatile interrupt pending flag shared with Default_IRQHandler */
static volatile unsigned int int_pend;

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for PCIe DMA write and read interrupt processing.
 *   Reads DMA interrupt status, clears interrupts, clears GIC, sets int_pend=0.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    unsigned int dma_wr_int_sts;
    unsigned int dma_rd_int_sts;

#if defined(DM0_RC)
    /* Read DMA write interrupt status and mask */
    dma_wr_int_sts = readl_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & DMA_INT_STATUS_MASK;
    /* Read DMA read interrupt status and mask */
    dma_rd_int_sts = readl_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF) & DMA_INT_STATUS_MASK;

    /* Clear DMA write interrupt */
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
    /* Clear DMA read interrupt */
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);

    LOGT("IRQ DM0: wr_sts=0x%x rd_sts=0x%x", dma_wr_int_sts, dma_rd_int_sts);

    /* Clear GIC interrupt for DM0 */
    GIC_ClearIRQ(PCIE_DM0_GIC_IRQ);
#elif defined(DM1_RC)
    /* Read DMA write interrupt status and mask */
    dma_wr_int_sts = readl_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF) & DMA_INT_STATUS_MASK;
    /* Read DMA read interrupt status and mask */
    dma_rd_int_sts = readl_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF) & DMA_INT_STATUS_MASK;

    /* Clear DMA write interrupt */
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);
    /* Clear DMA read interrupt */
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);

    LOGT("IRQ DM1: wr_sts=0x%x rd_sts=0x%x", dma_wr_int_sts, dma_rd_int_sts);

    /* Clear GIC interrupt for DM1 */
    GIC_ClearIRQ(PCIE_DM1_GIC_IRQ);
#else
    dma_wr_int_sts = 0U;
    dma_rd_int_sts = 0U;
    (void)dma_wr_int_sts;
    (void)dma_rd_int_sts;
#endif

    /* Step 23: Signal DMA completion to main flow */
    int_pend = 0U;
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
    (void)cfg;

    g_ctx = (pcie_dma_test_ctx_t){0};
    int_pend = 0U;

    LOGT("PCIe DMA write test init");

    /* Step 1: Write 0x0 to synchronization register 0xE6004100 */
    writel_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

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
    unsigned int data_rd;
    unsigned int timeout;
    unsigned int i;
    unsigned int len;
    unsigned long src_addr0;
    unsigned long wr_addr0;
    unsigned long wr_addr1;
    unsigned long wr_addr2;
    unsigned long wr_addr3;
    unsigned long rd_addr0;
    unsigned long rd_addr1;
    unsigned long rd_addr2;
    unsigned long rd_addr3;
    unsigned long dst_addr0;
    unsigned long dst_addr1;
    unsigned long dst_addr2;
    unsigned long dst_addr3;
    volatile unsigned int *src_ptr;

    (void)cfg;

    if (out == 0) {
        LOGE("PCIe DMA write test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe DMA write test run");

    /* Step 2: Conditional link training based on compile-time defines */
#if defined(DM0_RC)
    LOGT("Calling link_training_dm0_x4(4) for DM0_RC");
    link_training_dm0_x4(4);
#elif defined(DM1_RC)
    LOGT("Calling link_training_dm1_x4(4) for DM1_RC");
    link_training_dm1_x4(4);
#elif defined(DM0_EP)
    LOGT("Calling link_training_dm0_x4(4) for DM0_EP");
    link_training_dm0_x4(4);
#elif defined(DM1_EP)
    LOGT("Calling link_training_dm1_x4(4) for DM1_EP");
    link_training_dm1_x4(4);
#endif

    /* Steps 3-5: DM0_RC link setup, BAR and memory base programming */
#if defined(DM0_RC)
    /* Step 3: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    LOGT("Polling SII0 link status register 0xC0");
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling SII0 reg 0xC0, data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("SII0 link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 4: Read Vendor ID and enable command register */
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID from pcie_slv0 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("Wrote 0x7 to pcie_slv0 command register 0x4");

    /* Step 5: BAR and memory base programming for DM0 */
    LOGT("Calling bar_program_dm0_x4()");
    bar_program_dm0_x4();
    wait_on(10);
    LOGT("Calling mem_base_program_dm0_x4()");
    mem_base_program_dm0_x4();
#endif

    /* Steps 6-7: DM1_RC link setup, BAR and memory base programming */
#if defined(DM1_RC)
    /* Step 6: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    LOGT("Polling SII1 link status register 0xC0");
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling SII1 reg 0xC0, data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("SII1 link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 7: Read Vendor ID and enable command register for DM1 */
    data_rd = read_pcie_slv1_reg(0x0);
    LOGT("Vendor ID from pcie_slv1 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv1_reg(0x4, 0x7);
    LOGT("Wrote 0x7 to pcie_slv1 command register 0x4");

    LOGT("Calling bar_program_dm1_x4()");
    bar_program_dm1_x4();
    wait_on(10);
    LOGT("Calling mem_base_program_dm1_x4()");
    mem_base_program_dm1_x4();
#endif

    /* Step 8: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    LOGT("Called non_secure_prot_nic()");

    /* Step 9: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    LOGT("Polling sync register 0xE6004100 for completion value 0x12345678");
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = readl_reg(PCIE_SYNC_REG);
    while ((data_rd != PCIE_SYNC_COMPLETE_VAL) && (timeout > 0U)) {
        wait_on(5);
        data_rd = readl_reg(PCIE_SYNC_REG);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sync register 0xE6004100, data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("Sync register 0xE6004100 = 0x%x, synchronization complete", data_rd);
    }

    /* Step 10: Configure DMA addresses and length */
    len = DMA_TRANSFER_LEN;
    src_addr0 = DMA_SRC_ADDR0;

#if defined(DM0_RC)
    wr_addr0 = DM0_WR_ADDR0;
    wr_addr1 = DM0_WR_ADDR1;
    wr_addr2 = DM0_WR_ADDR2;
    wr_addr3 = DM0_WR_ADDR3;
    rd_addr0 = DM0_RD_ADDR0;
    rd_addr1 = DM0_RD_ADDR1;
    rd_addr2 = DM0_RD_ADDR2;
    rd_addr3 = DM0_RD_ADDR3;
#elif defined(DM1_RC)
    wr_addr0 = DM1_WR_ADDR0;
    wr_addr1 = DM1_WR_ADDR1;
    wr_addr2 = DM1_WR_ADDR2;
    wr_addr3 = DM1_WR_ADDR3;
    rd_addr0 = DM1_RD_ADDR0;
    rd_addr1 = DM1_RD_ADDR1;
    rd_addr2 = DM1_RD_ADDR2;
    rd_addr3 = DM1_RD_ADDR3;
#else
    wr_addr0 = 0UL;
    wr_addr1 = 0UL;
    wr_addr2 = 0UL;
    wr_addr3 = 0UL;
    rd_addr0 = 0UL;
    rd_addr1 = 0UL;
    rd_addr2 = 0UL;
    rd_addr3 = 0UL;
#endif

    dst_addr0 = DMA_DST_ADDR0;
    dst_addr1 = DMA_DST_ADDR1;
    dst_addr2 = DMA_DST_ADDR2;
    dst_addr3 = DMA_DST_ADDR3;

    LOGT("DMA config: len=0x%x src_addr0=0x%lx", len, src_addr0);

    /* Step 11: Preload source memory with test patterns */
    LOGT("Preloading source memory at 0x%lx", src_addr0);
    src_ptr = (volatile unsigned int *)src_addr0;
    for (i = 0U; i < DMA_PRELOAD_WORD_COUNT; i++) {
        src_ptr[i] = DMA_PATTERN1;
    }
    for (i = 0U; i < DMA_PRELOAD_WORD_COUNT; i++) {
        src_ptr[DMA_PRELOAD_OFFSET_WORDS + i] = DMA_PATTERN2;
    }
    LOGT("Source memory preloaded: 128 words of 0x%x + 128 words of 0x%x",
         DMA_PATTERN1, DMA_PATTERN2);

    /* Step 12: Set int_pend=1, enable GIC */
    int_pend = 1U;
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("GIC configured and all IRQs enabled");

    /* Steps 13-17: DM0_RC DMA write and read channels 0-3 */
#if defined(DM0_RC)
    /* Step 13: Unmask DMA interrupts for DM0 */
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM0 DMA interrupts unmasked");

    /* Step 14: DMA write channel 0 */
    LOGT("Programming DMA write channel 0");
    program_dma_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DMA write ch0 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA write ch0 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    /* Step 15: DMA write channels 1-3 */
    LOGT("Programming DMA write channel 1");
    program_dma_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DMA write ch1 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA write ch1 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DMA write channel 2");
    program_dma_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DMA write ch2 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA write ch2 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DMA write channel 3");
    program_dma_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DMA write ch3 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA write ch3 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    /* Step 16: DMA read channel 0 */
    LOGT("Programming DMA read channel 0");
    program_dma_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DMA read ch0 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA read ch0 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    /* Step 17: DMA read channels 1-3 */
    LOGT("Programming DMA read channel 1");
    program_dma_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DMA read ch1 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA read ch1 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DMA read channel 2");
    program_dma_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DMA read ch2 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA read ch2 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DMA read channel 3");
    program_dma_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    writel_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DMA read ch3 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DMA read ch3 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;
#endif

    /* Steps 18-20: DM1_RC DMA write and read channels 0-3 */
#if defined(DM1_RC)
    /* Step 18: Unmask DMA interrupts for DM1 */
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM1 DMA interrupts unmasked");

    /* Step 19: DMA write channels 0-3 for DM1 */
    LOGT("Programming DM1 DMA write channel 0");
    program_dma1_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA write ch0 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA write ch0 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA write channel 1");
    program_dma1_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA write ch1 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA write ch1 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA write channel 2");
    program_dma1_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA write ch2 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA write ch2 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA write channel 3");
    program_dma1_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA write ch3 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA write ch3 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    /* Step 20: DMA read channels 0-3 for DM1 */
    LOGT("Programming DM1 DMA read channel 0");
    program_dma1_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA read ch0 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA read ch0 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA read channel 1");
    program_dma1_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA read ch1 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA read ch1 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA read channel 2");
    program_dma1_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA read ch2 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA read ch2 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;

    LOGT("Programming DM1 DMA read channel 3");
    program_dma1_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    writel_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA read ch3 doorbell triggered");
    timeout = PCIE_POLL_TIMEOUT;
    while ((int_pend != 0U) && (timeout > 0U)) {
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout waiting for DM1 DMA read ch3 completion");
        g_ctx.errors++;
        out->status = -1;
    }
    int_pend = 1U;
#endif

    /* Step 24: Final wait and status reporting */
    wait_on(10);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to PSV/FV-native out->status completion.
    if (g_ctx.errors == 0U) {
        out->status = 0;
    }

    g_ctx.checks_failed = g_ctx.errors;

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

    LOGT("PCIe DMA write test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
