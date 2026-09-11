// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_device_enumerate_test
 * Description: Performs PCIe device enumeration including link training,
 *   cache programming, link status polling, vendor ID read, command register
 *   enable, memory base programming, system register writes, cache disable,
 *   BAR sizing, BAR assignment, and final synchronization polling.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_enum_test_ctx_t;

static pcie_enum_test_ctx_t g_ctx;

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
 *   Sets bits [19:22]=0x0 and [27:30]=0xf first pass, then [27:30]=0x0, [19:22]=0x0.
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
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (pcie_enum_test_ctx_t){0};

    LOGT("PCIe device enumerate test init");

    /* Step 1: Write 0x0 to synchronization register 0xE6004100 */
    writel_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Executes the main testcase flow for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int data_rd;
    unsigned int timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("PCIe enumerate test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe device enumerate test run");

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

    /* Steps 3-5: Cache programming for PCIE0 and PCIE1 */
    LOGT("Cache programming: PCIE0 coherency control");
    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Cache programming: PCIE1 coherency control");
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Steps 7-8: Combined cache programming after wait */
    LOGT("Combined cache programming: PCIE0 coherency control");
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    LOGT("Combined cache programming: PCIE1 coherency control");
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 9: Repeat link training and cache programming sequence */
#if defined(DM0_RC)
    link_training_dm0_x4(4);
#elif defined(DM1_RC)
    link_training_dm1_x4(4);
#elif defined(DM0_EP)
    link_training_dm0_x4(4);
#elif defined(DM1_EP)
    link_training_dm1_x4(4);
#endif

    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    wait_on(20);
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* Step 10: Read SII0 register 0xC0 */
    data_rd = read_sii0_reg(0xC0);
    LOGT("Initial read_sii0_reg(0xC0) = 0x%x", data_rd);

    /* Step 11: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    LOGT("Called non_secure_prot_nic()");

    /* Step 12: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
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

    /* Step 13: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
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

    /* Step 14: Under DM0_RC, read Vendor ID */
#if defined(DM0_RC)
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID from pcie_slv0 reg 0x0 = 0x%x", data_rd);

    /* Step 15: Write command register */
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("Wrote 0x7 to pcie_slv0 command register 0x4");

    /* Step 16: Memory base programming */
    LOGT("Calling mem_base_program_dm0_x4()");
    mem_base_program_dm0_x4();
    LOGT("Calling mem_base_program_dm1_x4()");
    mem_base_program_dm1_x4();
#endif

    /* Step 17: wait_on(10) */
    wait_on(10);

    /* Step 18: Write 0x1 to system-level registers */
    LOGT("Writing 0x1 to system-level registers");
    writel_reg(0xE690000C, 0x1);
    writel_reg(0xE6900010, 0x1);
    writel_reg(0xE6900014, 0x1);
    writel_reg(0xE6900018, 0x1);
    writel_reg(0xE6900030, 0x1);
    writel_reg(0xE6900034, 0x1);

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

    /* Steps 23-24: BAR sizing on pcie_slv1 - write 0xFFFFFFFF and read back */
    LOGT("BAR sizing: pcie_slv1 write 0xFFFFFFFF to offsets 0x10-0x24");
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);

    LOGT("BAR sizing: pcie_slv1 read back offsets 0x10-0x24");
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 size = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 size = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 size = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 size = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 size = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 size = 0x%x", data_rd);

    /* Steps 25-26: BAR assignment on pcie_slv1 - write specific values and read back */
    LOGT("BAR assignment: pcie_slv1 write specific address values");
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);

    LOGT("BAR assignment: pcie_slv1 read back offsets 0x10-0x24");
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 = 0x%x", data_rd);

    /* Step 27: Repeat BAR sizing and assignment for pcie_slv0 */
    LOGT("BAR sizing: pcie_slv0 write 0xFFFFFFFF to offsets 0x10-0x24");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);

    LOGT("BAR sizing: pcie_slv0 read back offsets 0x10-0x24");
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 size = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 size = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 size = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 size = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 size = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 size = 0x%x", data_rd);

    LOGT("BAR assignment: pcie_slv0 write specific address values");
    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);

    LOGT("BAR assignment: pcie_slv0 read back offsets 0x10-0x24");
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 = 0x%x", data_rd);

    /* Step 28: wait_on(10) */
    wait_on(10);

    /* Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
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

    /* Step 30: DV finish(0) converted to PSV/FV status reporting */
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
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("PCIe device enumerate test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
