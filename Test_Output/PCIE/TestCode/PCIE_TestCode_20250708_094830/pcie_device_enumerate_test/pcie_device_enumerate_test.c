// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_device_enumerate_test
 * Description: Performs PCIe device enumeration including link training,
 *              cache programming, link status polling, vendor ID read,
 *              command register enable, memory base programming, system
 *              register writes, cache disable, BAR sizing and assignment,
 *              and final synchronization polling.
 */

typedef struct {
    unsigned int errors;
} pcie_enum_test_ctx_t;

static pcie_enum_test_ctx_t g_ctx;

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
 * Description: Performs cache disable programming by clearing specified
 *              bit fields in the coherency control register.
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

    /* Step 3-4: Cache programming for PCIE0 */
    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache programming enable for PCIE0 complete");

    /* Step 5: Cache programming for PCIE1 */
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Cache programming enable for PCIE1 complete");

    /* Step 6: Wait */
    wait_on(20);
    LOGT("wait_on(20) complete");

    /* Step 7: Combined cache programming for PCIE0 */
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Combined cache programming enable for PCIE0 complete");

    /* Step 8: Combined cache programming for PCIE1 */
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Combined cache programming enable for PCIE1 complete");

    /* Step 9: Repeat link training and cache programming (duplicate block in source) */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
    LOGT("link_training_dm0_x4(4) called (repeat)");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
    LOGT("link_training_dm1_x4(4) called (repeat)");
#endif
    pcie_cache_program_enable(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_program_enable(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    wait_on(20);
    pcie_cache_program_enable_combined(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    pcie_cache_program_enable_combined(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    LOGT("Repeat link training and cache programming complete");

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
        LOGE("PCIe device enumerate test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe device enumeration run");

    /* Step 10: Read sii0 link status */
    data_rd = read_sii0_reg(0xC0);
    LOGT("read_sii0_reg(0xC0) = 0x%x", data_rd);

    /* Step 11: Call non_secure_prot_nic */
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");

    /* Step 12: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sii0_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("sii0_reg(0xC0) link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 13: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sii1_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("sii1_reg(0xC0) link status OK: data_rd=0x%x", data_rd);
    }

    /* Step 14: Under DM0_RC, read Vendor ID */
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID from pcie_slv0_reg(0x0) = 0x%x", data_rd);

    /* Step 15: Enable command register */
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("write_pcie_slv0_reg(0x4, 0x7) - command register enabled");

    /* Step 16: Memory base programming */
    mem_base_program_dm0_x4();
    LOGT("mem_base_program_dm0_x4() called");
    mem_base_program_dm1_x4();
    LOGT("mem_base_program_dm1_x4() called");
#endif

    /* Step 17: Wait */
    wait_on(10);
    LOGT("wait_on(10) complete");

    /* Step 18: Write 0x1 to system-level registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    LOGT("System-level registers 0xE690000C-0xE6900034 written with 0x1");

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

    /* Step 23: BAR sizing - write 0xFFFFFFFF to pcie_slv1 BAR registers */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    LOGT("pcie_slv1 BAR registers written with 0xFFFFFFFF for sizing");

    /* Step 24: Read back pcie_slv1 BAR registers */
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 size readback = 0x%x", data_rd);

    /* Step 25: Write specific address values to pcie_slv1 BAR registers */
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);
    LOGT("pcie_slv1 BAR registers programmed with address values");

    /* Step 26: Read back pcie_slv1 BAR registers after assignment */
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("pcie_slv1 BAR0 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("pcie_slv1 BAR1 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("pcie_slv1 BAR2 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("pcie_slv1 BAR3 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("pcie_slv1 BAR4 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("pcie_slv1 BAR5 assigned readback = 0x%x", data_rd);

    /* Step 27: Repeat BAR sizing and assignment for pcie_slv0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    LOGT("pcie_slv0 BAR registers written with 0xFFFFFFFF for sizing");

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 size readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 size readback = 0x%x", data_rd);

    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);
    LOGT("pcie_slv0 BAR registers programmed with address values");

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("pcie_slv0 BAR0 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("pcie_slv0 BAR1 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("pcie_slv0 BAR2 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("pcie_slv0 BAR3 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("pcie_slv0 BAR4 assigned readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("pcie_slv0 BAR5 assigned readback = 0x%x", data_rd);

    /* Step 28: Wait */
    wait_on(10);
    LOGT("wait_on(10) complete");

    /* Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow (step 30).
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_reg(0xE6004100);
    while ((data_rd != 0x12345678) && (timeout > 0U)) {
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
