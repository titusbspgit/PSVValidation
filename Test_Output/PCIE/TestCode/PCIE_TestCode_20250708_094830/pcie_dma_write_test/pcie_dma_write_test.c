// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_dma_write_test
 * Description: Validates PCIe DMA write and read operations across all four
 *              DMA channels (0-3) for both dual-mode controllers (DM0 and DM1).
 *              Includes link training, link status polling, BAR/memory base
 *              programming, synchronization polling, source memory preload,
 *              GIC/interrupt setup, DMA write/read channel programming with
 *              doorbell trigger and interrupt-driven completion.
 */

typedef struct {
    unsigned int errors;
} pcie_dma_test_ctx_t;

static pcie_dma_test_ctx_t g_ctx;

/* Volatile interrupt pending flag shared with IRQ handler */
static volatile unsigned int int_pend;

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for PCIe DMA write/read interrupt clearing.
 *              Reads DMA interrupt status, clears interrupts, clears GIC,
 *              and signals completion via int_pend.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    unsigned int dma_wr_int_sts;
    unsigned int dma_rd_int_sts;

#ifdef DM0_RC
    /* Read DMA write interrupt status */
    dma_wr_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
    dma_wr_int_sts = dma_wr_int_sts & DMA_INT_STATUS_MASK;

    /* Read DMA read interrupt status */
    dma_rd_int_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF);
    dma_rd_int_sts = dma_rd_int_sts & DMA_INT_STATUS_MASK;

    /* Clear DMA write interrupt */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);

    /* Clear DMA read interrupt */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);

    /* Clear GIC interrupt for DM0 */
    GIC_ClearIRQ(PCIE_DM0_GIC_IRQ);
    LOGT("IRQ DM0: wr_sts=0x%x rd_sts=0x%x", dma_wr_int_sts, dma_rd_int_sts);
#endif

#ifdef DM1_RC
    /* Read DMA write interrupt status */
    dma_wr_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
    dma_wr_int_sts = dma_wr_int_sts & DMA_INT_STATUS_MASK;

    /* Read DMA read interrupt status */
    dma_rd_int_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF);
    dma_rd_int_sts = dma_rd_int_sts & DMA_INT_STATUS_MASK;

    /* Clear DMA write interrupt */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, dma_wr_int_sts);

    /* Clear DMA read interrupt */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, dma_rd_int_sts);

    /* Clear GIC interrupt for DM1 */
    GIC_ClearIRQ(PCIE_DM1_GIC_IRQ);
    LOGT("IRQ DM1: wr_sts=0x%x rd_sts=0x%x", dma_wr_int_sts, dma_rd_int_sts);
#endif

    /* Signal DMA completion */
    int_pend = 0;
}

/*
 * Function: pcie_dma_wait_int_pend
 * Description: Waits for int_pend to be cleared by the IRQ handler with timeout.
 * Parameters:
 *   None.
 * Returns:
 *   0 on success, -1 on timeout.
 */
static int pcie_dma_wait_int_pend(void)
{
    unsigned int timeout = PCIE_POLL_TIMEOUT;

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
    int_pend = 0;

    LOGT("PCIe DMA write test init");

    /* Step 1: Initialize synchronization register */
    write_reg(0xE6004100, 0x0);
    LOGT("write_reg(0xE6004100, 0x0) - sync register initialized");

    /* Step 2: Conditionally call link training based on compile-time defines */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
    LOGT("link_training_dm0_x4(4) called");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
    LOGT("link_training_dm1_x4(4) called");
#endif

    /* Step 3: Under DM0_RC - poll link status */
#ifdef DM0_RC
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & PCIE_LINK_STATUS_MASK) != PCIE_LINK_STATUS_EXPECTED) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sii0_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("sii0_reg(0xC0) link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 4: Read Vendor ID and enable command register */
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID from pcie_slv0_reg(0x0) = 0x%x", data_rd);
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("write_pcie_slv0_reg(0x4, 0x7) - command register enabled");

    /* Step 5: BAR and memory base programming for DM0 */
    bar_program_dm0_x4();
    LOGT("bar_program_dm0_x4() called");
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("mem_base_program_dm0_x4() called");
#endif

    /* Step 6: Under DM1_RC - poll link status */
#ifdef DM1_RC
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & PCIE_LINK_STATUS_MASK) != PCIE_LINK_STATUS_EXPECTED) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sii1_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("sii1_reg(0xC0) link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 7: Read Vendor ID, enable command register, BAR and memory base for DM1 */
    data_rd = read_pcie_slv1_reg(0x0);
    LOGT("Vendor ID from pcie_slv1_reg(0x0) = 0x%x", data_rd);
    write_pcie_slv1_reg(0x4, 0x7);
    LOGT("write_pcie_slv1_reg(0x4, 0x7) - command register enabled");
    bar_program_dm1_x4();
    LOGT("bar_program_dm1_x4() called");
    wait_on(10);
    mem_base_program_dm1_x4();
    LOGT("mem_base_program_dm1_x4() called");
#endif

    /* Step 8: Call non_secure_prot_nic */
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");

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
    volatile unsigned int *src_ptr;
    unsigned int len;
    unsigned int src_addr0;
    unsigned int wr_addr0, wr_addr1, wr_addr2, wr_addr3;
    unsigned int rd_addr0, rd_addr1, rd_addr2, rd_addr3;
    unsigned int dst_addr0, dst_addr1, dst_addr2, dst_addr3;

    (void)cfg;

    if (out == 0) {
        LOGE("PCIe DMA write test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe DMA write test run");

    /* Step 9: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_reg(0xE6004100);
    while ((data_rd != PCIE_SYNC_EXPECTED) && (timeout > 0U)) {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling 0xE6004100 for 0x12345678, data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("Sync register 0xE6004100 = 0x%x", data_rd);
    }

    /* Step 10: Configure DMA addresses and length */
    len = PCIE_DMA_XFER_LEN;
    src_addr0 = PCIE_SRC_ADDR0;

#ifdef DM0_RC
    wr_addr0 = PCIE_DM0_WR_ADDR0;
    wr_addr1 = PCIE_DM0_WR_ADDR1;
    wr_addr2 = PCIE_DM0_WR_ADDR2;
    wr_addr3 = PCIE_DM0_WR_ADDR3;
    rd_addr0 = PCIE_DM0_RD_ADDR0;
    rd_addr1 = PCIE_DM0_RD_ADDR1;
    rd_addr2 = PCIE_DM0_RD_ADDR2;
    rd_addr3 = PCIE_DM0_RD_ADDR3;
#endif
#ifdef DM1_RC
    wr_addr0 = PCIE_DM1_WR_ADDR0;
    wr_addr1 = PCIE_DM1_WR_ADDR1;
    wr_addr2 = PCIE_DM1_WR_ADDR2;
    wr_addr3 = PCIE_DM1_WR_ADDR3;
    rd_addr0 = PCIE_DM1_RD_ADDR0;
    rd_addr1 = PCIE_DM1_RD_ADDR1;
    rd_addr2 = PCIE_DM1_RD_ADDR2;
    rd_addr3 = PCIE_DM1_RD_ADDR3;
#endif

    dst_addr0 = PCIE_DST_ADDR0;
    dst_addr1 = PCIE_DST_ADDR1;
    dst_addr2 = PCIE_DST_ADDR2;
    dst_addr3 = PCIE_DST_ADDR3;

    LOGT("DMA config: len=0x%x src_addr0=0x%x", len, src_addr0);

    /* Step 11: Preload source memory */
    src_ptr = (volatile unsigned int *)(uintptr_t)src_addr0;
    for (i = 0U; i < PCIE_DMA_PRELOAD_WORDS; i++) {
        src_ptr[i] = PCIE_DMA_PATTERN0;
    }
    for (i = 0U; i < PCIE_DMA_PRELOAD_WORDS; i++) {
        src_ptr[PCIE_DMA_PRELOAD_OFFSET_WORDS + i] = PCIE_DMA_PATTERN1;
    }
    LOGT("Source memory preloaded: 128 words of 0x%x, 128 words of 0x%x",
         PCIE_DMA_PATTERN0, PCIE_DMA_PATTERN1);

    /* Step 12: Set int_pend, enable GIC */
    int_pend = 1;
    GIC_Set();
    GIC_EnableAllIRQ();
    LOGT("GIC_Set() and GIC_EnableAllIRQ() called");

    /* ===== DM0_RC DMA operations ===== */
#ifdef DM0_RC
    /* Step 13: Unmask DMA interrupts for DM0 */
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM0 DMA interrupts unmasked");

    /* Step 14: DMA write channel 0 */
    program_dma_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DM0 DMA write ch0 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* Step 15: DMA write channel 1 */
    program_dma_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DM0 DMA write ch1 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA write channel 2 */
    program_dma_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DM0 DMA write ch2 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA write channel 3 */
    program_dma_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DM0 DMA write ch3 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* Step 16: DMA read channel 0 */
    program_dma_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DM0 DMA read ch0 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* Step 17: DMA read channel 1 */
    program_dma_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DM0 DMA read ch1 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA read channel 2 */
    program_dma_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DM0 DMA read ch2 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA read channel 3 */
    program_dma_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DM0 DMA read ch3 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;
    LOGT("DM0 DMA write/read channels 0-3 complete");
#endif

    /* ===== DM1_RC DMA operations ===== */
#ifdef DM1_RC
    /* Step 18: Unmask DMA interrupts for DM1 */
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
    LOGT("DM1 DMA interrupts unmasked");

    /* Step 19: DMA write channel 0 */
    program_dma1_wch0(0x0, src_addr0, 0x0, wr_addr0, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA write ch0 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA write channel 1 */
    program_dma1_wch1(0x0, src_addr0, 0x0, wr_addr1, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA write ch1 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA write channel 2 */
    program_dma1_wch2(0x0, src_addr0, 0x0, wr_addr2, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA write ch2 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA write channel 3 */
    program_dma1_wch3(0x0, src_addr0, 0x0, wr_addr3, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA write ch3 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* Step 20: DMA read channel 0 */
    program_dma1_rch0(0x0, rd_addr0, 0x0, dst_addr0, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    LOGT("DM1 DMA read ch0 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA read channel 1 */
    program_dma1_rch1(0x0, rd_addr1, 0x0, dst_addr1, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
    LOGT("DM1 DMA read ch1 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA read channel 2 */
    program_dma1_rch2(0x0, rd_addr2, 0x0, dst_addr2, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
    LOGT("DM1 DMA read ch2 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;

    /* DMA read channel 3 */
    program_dma1_rch3(0x0, rd_addr3, 0x0, dst_addr3, 0x0, len);
    write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
    LOGT("DM1 DMA read ch3 triggered");
    if (pcie_dma_wait_int_pend() != 0) {
        out->status = -1;
    }
    int_pend = 1;
    LOGT("DM1 DMA write/read channels 0-3 complete");
#endif

    /* Step 24: Final wait */
    wait_on(10);

    // MANUAL_REVIEW: DV finish(0) was present in the source flow (step 24).
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.

    /* Final status */
    if (g_ctx.errors > 0U) {
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

    LOGT("PCIe DMA write test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
