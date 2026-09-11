// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_mem_wr_rd_test
 * Description: Validates PCIe memory write and read operations through the
 *   PCIe slave interfaces for DM0_RC, DM1_RC, DM0_EP, and DM1_EP modes.
 *   Includes link training, cache enable/disable programming, link status
 *   polling, BAR/memory base setup, and memory write/read verification.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_mem_wr_rd_test_ctx_t;

static pcie_mem_wr_rd_test_ctx_t g_ctx;

/*
 * Function: pcie_cache_program_enable
 * Description: Performs cache enable programming on a coherency control register.
 *   Reads the register, sets specified bit fields using set_data(), writes back.
 * Parameters:
 *   reg_addr - Address of the coherency control register.
 * Returns:
 *   void
 */
static void pcie_cache_program_enable(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    writel_reg(reg_addr, data_rd);

    /* Read, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    writel_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_program_enable_combined
 * Description: Performs combined cache enable programming in a single read-modify-write.
 * Parameters:
 *   reg_addr - Address of the coherency control register.
 * Returns:
 *   void
 */
static void pcie_cache_program_enable_combined(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    writel_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_disable
 * Description: Performs cache disable programming on a coherency control register.
 *   Sets bits [11:14]=0xf, [3:6]=0xf first, then bits [27:30]=0xf, [19:22]=0x0.
 * Parameters:
 *   reg_addr - Address of the coherency control register.
 * Returns:
 *   void
 */
static void pcie_cache_disable(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    writel_reg(reg_addr, data_rd);

    /* Read, set bits [27:30]=0xf, [19:22]=0x0, write back */
    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    writel_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_disable_combined
 * Description: Performs combined cache disable with bits [27:30]=0x0, [19:22]=0x0.
 * Parameters:
 *   reg_addr - Address of the coherency control register.
 * Returns:
 *   void
 */
static void pcie_cache_disable_combined(unsigned long reg_addr)
{
    unsigned int data_rd;

    data_rd = readl_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    writel_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_mem_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (pcie_mem_wr_rd_test_ctx_t){0};

    LOGT("PCIe memory write/read test init");

    /* Step 1: Write 0x0 to synchronization register 0xE6004100 */
    writel_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int data_rd;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("PCIe mem wr/rd test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe memory write/read test run");

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

    /* Steps 3-5: Cache enable programming for PCIE0 and PCIE1 */
    LOGT("Cache enable programming: PCIE0 coherency control");
    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Cache enable programming: PCIE1 coherency control");
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Steps 7-8: Combined cache enable programming after wait */
    LOGT("Combined cache enable programming: PCIE0 coherency control");
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Combined cache enable programming: PCIE1 coherency control");
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 9: Read SII0 register 0xC0 */
    data_rd = read_sii0_reg(0xC0);
    LOGT("Initial read_sii0_reg(0xC0) = 0x%x", data_rd);

    /* Step 10: Under DM0, poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
#if defined(DM0)
    LOGT("Polling SII0 link status register 0xC0 for DM0");
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
#endif

    /* Step 11: Under DM1, poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
#if defined(DM1)
    LOGT("Polling SII1 link status register 0xC0 for DM1");
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
#endif

    /* Step 12: Under DM0_EP, wait_on(30000) */
#if defined(DM0_EP)
    LOGT("DM0_EP: wait_on(30000)");
    wait_on(30000);
#endif

    /* Step 13: Under DM0_RC, Vendor ID read, command enable, BAR and mem base programming */
#if defined(DM0_RC)
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("DM0_RC Vendor ID from pcie_slv0 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("DM0_RC: Wrote 0x7 to pcie_slv0 command register 0x4");
    LOGT("DM0_RC: Calling bar_program_dm0_x4()");
    bar_program_dm0_x4();
    wait_on(10);
    LOGT("DM0_RC: Calling mem_base_program_dm0_x4()");
    mem_base_program_dm0_x4();
#endif

    /* Step 14: Under DM1_RC, Vendor ID read, command enable, BAR and mem base programming */
#if defined(DM1_RC)
    data_rd = read_pcie_slv1_reg(0x0);
    LOGT("DM1_RC Vendor ID from pcie_slv1 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv1_reg(0x4, 0x7);
    LOGT("DM1_RC: Wrote 0x7 to pcie_slv1 command register 0x4");
    LOGT("DM1_RC: Calling bar_program_dm1_x4()");
    bar_program_dm1_x4();
    wait_on(10);
    LOGT("DM1_RC: Calling mem_base_program_dm1_x4()");
    mem_base_program_dm1_x4();
#endif

    /* Step 15: Under DM0_EP, EP-specific BAR and mem base programming */
#if defined(DM0_EP)
    LOGT("DM0_EP: Calling bar_program_dm0_EP_x4()");
    bar_program_dm0_EP_x4();
    wait_on(10);
    LOGT("DM0_EP: Calling mem_base_program_dm0_x4()");
    mem_base_program_dm0_x4();
#endif

    /* Step 16: Under DM1_EP, EP-specific BAR and mem base programming */
#if defined(DM1_EP)
    LOGT("DM1_EP: Calling bar_program_dm1_EP_x4()");
    bar_program_dm1_EP_x4();
    wait_on(10);
    LOGT("DM1_EP: Calling mem_base_program_dm1_x4()");
    mem_base_program_dm1_x4();
#endif

    /* Step 17: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    LOGT("Called non_secure_prot_nic()");

    /* Step 18: Write 0x11111111 to sync register to signal readiness */
    writel_reg(PCIE_SYNC_REG, PCIE_SYNC_READY_VAL);
    LOGT("Wrote 0x%x to sync register 0x%lx",
         PCIE_SYNC_READY_VAL, (unsigned long)PCIE_SYNC_REG);

    /* Steps 19-20: Cache disable for PCIE0 and PCIE1 */
    LOGT("Cache disable: PCIE0 coherency control");
    pcie_cache_disable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Cache disable: PCIE1 coherency control");
    pcie_cache_disable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 21: wait_on(10), then combined cache disable */
    wait_on(10);
    LOGT("Combined cache disable: PCIE0 and PCIE1");
    pcie_cache_disable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_disable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 22: wait_on(30) */
    wait_on(30);

    /* Step 23: Under DM0_RC, memory write/read operations */
#if defined(DM0_RC)
    LOGT("DM0_RC: pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5)");
    pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
    LOGT("DM0_RC: pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6)");
    pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
    LOGT("DM0_RC: pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7)");
    pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
#endif

    /* Step 24: Under DM1_RC, memory write/read operations */
#if defined(DM1_RC)
    LOGT("DM1_RC: pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5)");
    pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
    LOGT("DM1_RC: pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6)");
    pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
    LOGT("DM1_RC: pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5)");
    pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
#endif

    /* Step 25: Under DM0_EP, memory write/read operations */
#if defined(DM0_EP)
    LOGT("DM0_EP: pcie_slv0_mem_wr_rd at 5 addresses with data 0x5a5a5a5a");
    pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a);
#endif

    /* Step 26: Under DM1_EP, memory write/read operations */
#if defined(DM1_EP)
    LOGT("DM1_EP: pcie_slv1_mem_wr_rd at 5 addresses with data 0x5a5a5a5a");
    pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a);
#endif

    /* Step 27: wait_on(10) */
    wait_on(10);

    /* Step 28: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
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
        LOGT("Sync register 0xE6004100 = 0x%x, completion detected", data_rd);
    }

    /* Step 29: DV finish(0) converted to PSV/FV status reporting */
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
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("PCIe memory write/read test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
