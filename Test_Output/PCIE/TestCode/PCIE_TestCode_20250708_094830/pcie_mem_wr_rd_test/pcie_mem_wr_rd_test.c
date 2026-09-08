// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_mem_wr_rd_test
 * Description: Validates PCIe memory write and read operations through the
 *              PCIe slave interfaces for DM0_RC, DM1_RC, DM0_EP, and DM1_EP
 *              modes. Includes link training, cache programming, link status
 *              polling, BAR/memory base programming, cache disable, memory
 *              write/read verification, and final synchronization polling.
 */

typedef struct {
    unsigned int errors;
} pcie_mem_wr_rd_test_ctx_t;

static pcie_mem_wr_rd_test_ctx_t g_ctx;

/*
 * Function: pcie_cache_program_enable
 * Description: Performs cache programming on a coherency control register
 *              by reading, setting specified bit fields, and writing back.
 * Parameters:
 *   reg_addr - Register address to program.
 * Returns:
 *   void
 */
static void pcie_cache_program_enable(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(reg_addr, data_rd);

    /* Read, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_program_enable_combined
 * Description: Performs combined cache programming setting all bit fields
 *              in a single read-modify-write sequence.
 * Parameters:
 *   reg_addr - Register address to program.
 * Returns:
 *   void
 */
static void pcie_cache_program_enable_combined(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_disable
 * Description: Performs cache disable programming by setting some bit fields
 *              and clearing others in the coherency control register.
 * Parameters:
 *   reg_addr - Register address to program.
 * Returns:
 *   void
 */
static void pcie_cache_disable(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(reg_addr, data_rd);

    /* Read, set bits [27:30]=0xf, [19:22]=0x0, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(reg_addr, data_rd);
}

/*
 * Function: pcie_cache_disable_combined
 * Description: Performs combined cache disable clearing bits [27:30] and [19:22].
 * Parameters:
 *   reg_addr - Register address to program.
 * Returns:
 *   void
 */
static void pcie_cache_disable_combined(unsigned long reg_addr)
{
    unsigned int data_rd;

    /* Read, set bits [27:30]=0x0, [19:22]=0x0, write back */
    data_rd = read_reg(reg_addr);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(reg_addr, data_rd);
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
    unsigned int data_rd;
    unsigned int timeout;

    (void)cfg;

    g_ctx = (pcie_mem_wr_rd_test_ctx_t){0};

    LOGT("PCIe memory write/read test init");

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

    /* Step 3-4: Cache enable programming for PCIE0 */
    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache enable programming for PCIE0 complete");

    /* Step 5: Cache enable programming for PCIE1 */
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache enable programming for PCIE1 complete");

    /* Step 6: Wait */
    wait_on(20);
    LOGT("wait_on(20) complete");

    /* Step 7: Combined cache enable programming for PCIE0 */
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Combined cache enable programming for PCIE0 complete");

    /* Step 8: Combined cache enable programming for PCIE1 */
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Combined cache enable programming for PCIE1 complete");

    /* Step 9: Read sii0 link status */
    data_rd = read_sii0_reg(0xC0);
    LOGT("read_sii0_reg(0xC0) = 0x%x", data_rd);

    /* Step 10: Under DM0 - poll link status */
#ifdef DM0
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
#endif

    /* Step 11: Under DM1 - poll link status */
#ifdef DM1
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
#endif

    /* Step 12: Under DM0_EP - additional wait */
#ifdef DM0_EP
    wait_on(30000);
    LOGT("wait_on(30000) for DM0_EP complete");
#endif

    /* Step 13: Under DM0_RC - Vendor ID, command register, BAR and memory base */
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID from pcie_slv0_reg(0x0) = 0x%x", data_rd);
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("write_pcie_slv0_reg(0x4, 0x7) - command register enabled");
    bar_program_dm0_x4();
    LOGT("bar_program_dm0_x4() called");
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("mem_base_program_dm0_x4() called");
#endif

    /* Step 14: Under DM1_RC - Vendor ID, command register, BAR and memory base */
#ifdef DM1_RC
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

    /* Step 15: Under DM0_EP - EP BAR and memory base */
#ifdef DM0_EP
    bar_program_dm0_EP_x4();
    LOGT("bar_program_dm0_EP_x4() called");
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("mem_base_program_dm0_x4() called");
#endif

    /* Step 16: Under DM1_EP - EP BAR and memory base */
#ifdef DM1_EP
    bar_program_dm1_EP_x4();
    LOGT("bar_program_dm1_EP_x4() called");
    wait_on(10);
    mem_base_program_dm1_x4();
    LOGT("mem_base_program_dm1_x4() called");
#endif

    /* Step 17: Configure NIC security */
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");

    /* Step 18: Signal readiness */
    write_reg(0xE6004100, 0x11111111);
    LOGT("write_reg(0xE6004100, 0x11111111) - readiness signaled");

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

    /* Step 19: Cache disable for PCIE0 */
    pcie_cache_disable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache disable for PCIE0 complete");

    /* Step 20: Cache disable for PCIE1 */
    pcie_cache_disable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache disable for PCIE1 complete");

    /* Step 21: Wait and combined cache disable for both PCIE0 and PCIE1 */
    wait_on(10);
    pcie_cache_disable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_disable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Combined cache disable for PCIE0 and PCIE1 complete");

    /* Step 22: Wait */
    wait_on(30);
    LOGT("wait_on(30) complete");

    /* Step 23: Under DM0_RC - memory write/read operations */
#ifdef DM0_RC
    pcie_slv0_mem_wr_rd(PCIE_DM0_RC_ADDR0, PCIE_DM0_RC_DATA0);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_DM0_RC_ADDR0, PCIE_DM0_RC_DATA0);
    pcie_slv0_mem_wr_rd(PCIE_DM0_RC_ADDR1, PCIE_DM0_RC_DATA1);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_DM0_RC_ADDR1, PCIE_DM0_RC_DATA1);
    pcie_slv0_mem_wr_rd(PCIE_DM0_RC_ADDR2, PCIE_DM0_RC_DATA2);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_DM0_RC_ADDR2, PCIE_DM0_RC_DATA2);
    LOGT("DM0_RC memory write/read operations complete");
#endif

    /* Step 24: Under DM1_RC - memory write/read operations */
#ifdef DM1_RC
    pcie_slv1_mem_wr_rd(PCIE_DM1_RC_ADDR0, PCIE_DM1_RC_DATA0);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_DM1_RC_ADDR0, PCIE_DM1_RC_DATA0);
    pcie_slv1_mem_wr_rd(PCIE_DM1_RC_ADDR1, PCIE_DM1_RC_DATA1);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_DM1_RC_ADDR1, PCIE_DM1_RC_DATA1);
    pcie_slv1_mem_wr_rd(PCIE_DM1_RC_ADDR2, PCIE_DM1_RC_DATA2);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_DM1_RC_ADDR2, PCIE_DM1_RC_DATA2);
    LOGT("DM1_RC memory write/read operations complete");
#endif

    /* Step 25: Under DM0_EP - memory write/read operations */
#ifdef DM0_EP
    pcie_slv0_mem_wr_rd(PCIE_EP_ADDR0, PCIE_EP_DATA);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR0, PCIE_EP_DATA);
    pcie_slv0_mem_wr_rd(PCIE_EP_ADDR1, PCIE_EP_DATA);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR1, PCIE_EP_DATA);
    pcie_slv0_mem_wr_rd(PCIE_EP_ADDR2, PCIE_EP_DATA);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR2, PCIE_EP_DATA);
    pcie_slv0_mem_wr_rd(PCIE_EP_ADDR3, PCIE_EP_DATA);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR3, PCIE_EP_DATA);
    pcie_slv0_mem_wr_rd(PCIE_EP_ADDR4, PCIE_EP_DATA);
    LOGT("pcie_slv0_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR4, PCIE_EP_DATA);
    LOGT("DM0_EP memory write/read operations complete");
#endif

    /* Step 26: Under DM1_EP - memory write/read operations */
#ifdef DM1_EP
    pcie_slv1_mem_wr_rd(PCIE_EP_ADDR0, PCIE_EP_DATA);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR0, PCIE_EP_DATA);
    pcie_slv1_mem_wr_rd(PCIE_EP_ADDR1, PCIE_EP_DATA);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR1, PCIE_EP_DATA);
    pcie_slv1_mem_wr_rd(PCIE_EP_ADDR2, PCIE_EP_DATA);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR2, PCIE_EP_DATA);
    pcie_slv1_mem_wr_rd(PCIE_EP_ADDR3, PCIE_EP_DATA);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR3, PCIE_EP_DATA);
    pcie_slv1_mem_wr_rd(PCIE_EP_ADDR4, PCIE_EP_DATA);
    LOGT("pcie_slv1_mem_wr_rd(0x%x, 0x%x)", PCIE_EP_ADDR4, PCIE_EP_DATA);
    LOGT("DM1_EP memory write/read operations complete");
#endif

    /* Step 27: Wait */
    wait_on(10);
    LOGT("wait_on(10) complete");

    /* Step 28: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow (step 29).
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.
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
        LOGT("Sync register 0xE6004100 = 0x%x (expected 0x12345678)", data_rd);
    }

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
