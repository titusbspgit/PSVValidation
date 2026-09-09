// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * Test Case: pcie_device_enumerate_test
 * Description: This testcase performs PCIe device enumeration. It writes 0x0 to
 *   0xE6004100, conditionally invokes link training based on DM0_RC/DM1_RC/DM0_EP/DM1_EP
 *   compile-time defines, performs cache programming via read-modify-write of coherency
 *   control registers, polls link status, reads Vendor ID, programs BARs, and polls
 *   for final synchronization.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
} pcie_enum_test_ctx_t;

static pcie_enum_test_ctx_t g_ctx;

/*
 * Helper: cache_program_enable_pcie0
 * Description: Read-modify-write mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF
 *   to set cache enable bits.
 */
static void cache_program_enable_pcie0(void)
{
    unsigned int data_rd;

    /* Step 3: Read PCIE0 coherency control, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: Read PCIE0 coherency control again, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

/*
 * Helper: cache_program_enable_pcie1
 * Description: Read-modify-write mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
 *   to set cache enable bits.
 */
static void cache_program_enable_pcie1(void)
{
    unsigned int data_rd;

    /* Step 5: Read PCIE1 coherency control, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Read PCIE1 coherency control again, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

/*
 * Helper: cache_program_enable_combined_pcie0
 * Description: Read-modify-write mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF
 *   to set all cache enable bits in a single pass.
 */
static void cache_program_enable_combined_pcie0(void)
{
    unsigned int data_rd;

    /* Step 7: Read PCIE0, set bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

/*
 * Helper: cache_program_enable_combined_pcie1
 * Description: Read-modify-write mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
 *   to set all cache enable bits in a single pass.
 */
static void cache_program_enable_combined_pcie1(void)
{
    unsigned int data_rd;

    /* Step 8: Read PCIE1, set bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

/*
 * Helper: link_training_and_cache_program
 * Description: Performs conditional link training and cache programming sequence.
 */
static void link_training_and_cache_program(void)
{
    /* Step 2: Conditionally call link training based on compile-time defines */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
    LOGT("PCIe link training DM0 x4 initiated");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
    LOGT("PCIe link training DM1 x4 initiated");
#endif

    /* Steps 3-4: Cache programming for PCIE0 */
    cache_program_enable_pcie0();

    /* Step 5: Cache programming for PCIE1 */
    cache_program_enable_pcie1();
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

    LOGT("pcie_device_enumerate_test init: starting PCIe device enumeration setup");

    /* Step 1: Write 0x0 to 0xE6004100 to initialize synchronization register */
    write_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

    /* Steps 2-5: First link training and cache programming sequence */
    link_training_and_cache_program();

    /* Step 6: wait_on(20) */
    wait_on(20);
    LOGT("wait_on(20) complete");

    /* Steps 7-8: Combined cache programming after wait */
    cache_program_enable_combined_pcie0();
    cache_program_enable_combined_pcie1();

    /* Step 9: Repeat link training and cache programming sequence (duplicate block in source) */
    link_training_and_cache_program();

    /* Step 6 repeat: wait_on(20) */
    wait_on(20);
    LOGT("Repeated wait_on(20) complete");

    /* Steps 7-8 repeat: Combined cache programming after wait */
    cache_program_enable_combined_pcie0();
    cache_program_enable_combined_pcie1();

    LOGT("pcie_device_enumerate_test init complete");

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
    unsigned int i;

    (void)cfg;

    if (out == 0) {
        LOGE("pcie_device_enumerate_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("pcie_device_enumerate_test run: starting main execution");

    /* Step 10: Read read_sii0_reg(0xC0) into data_rd */
    data_rd = read_sii0_reg(0xC0);
    LOGT("Initial read_sii0_reg(0xC0) = 0x%x", data_rd);

    /* Step 11: Call non_secure_prot_nic() */
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
        LOGE("Timeout polling read_sii0_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("read_sii0_reg(0xC0) link status OK: 0x%x", data_rd);
    }

    /* Step 13: Read read_sii1_reg(0xC0) and poll until (data_rd & 0xD1) == 0xD1 */
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling read_sii1_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("read_sii1_reg(0xC0) link status OK: 0x%x", data_rd);
    }

    /* Step 14: Under DM0_RC, read Vendor ID from pcie_slv0 offset 0x0 */
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID read from pcie_slv0 reg 0x0 = 0x%x", data_rd);

    /* Step 15: Write command register via write_pcie_slv0_reg(0x4, 0x7) */
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("Command register written: pcie_slv0 reg 0x4 = 0x7");

    /* Step 16: Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4() */
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    LOGT("Memory base programming for dm0 and dm1 complete");
#endif /* DM0_RC */

    /* Step 17: wait_on(10) */
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
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Cache disable programming for PCIE0 complete");

    /* Step 20: Cache disable for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Cache disable programming for PCIE1 complete");

    /* Step 21: wait_on(10), then combined cache disable for both PCIE0 and PCIE1 */
    wait_on(10);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Combined cache disable for PCIE0 and PCIE1 complete");

    /* Step 22: wait_on(30) */
    wait_on(30);
    LOGT("wait_on(30) complete");

    /* Step 23: Write 0xFFFFFFFF to pcie_slv1 BAR registers for BAR sizing */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        write_pcie_slv1_reg(pcie_bar_offsets[i], 0xFFFFFFFF);
    }
    LOGT("BAR sizing: wrote 0xFFFFFFFF to pcie_slv1 offsets 0x10-0x24");

    /* Step 24: Read back pcie_slv1 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        data_rd = read_pcie_slv1_reg(pcie_bar_offsets[i]);
        LOGT("pcie_slv1 BAR[%u] offset=0x%x sizing readback=0x%x", i, pcie_bar_offsets[i], data_rd);
    }

    /* Step 25: Write specific address values to pcie_slv1 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        write_pcie_slv1_reg(pcie_bar_offsets[i], pcie_bar_values[i]);
    }
    LOGT("BAR assignment: wrote address values to pcie_slv1 offsets 0x10-0x24");

    /* Step 26: Read back pcie_slv1 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        data_rd = read_pcie_slv1_reg(pcie_bar_offsets[i]);
        LOGT("pcie_slv1 BAR[%u] offset=0x%x assignment readback=0x%x", i, pcie_bar_offsets[i], data_rd);
    }

    /* Step 27: Repeat BAR sizing and assignment for pcie_slv0 */
    /* Write 0xFFFFFFFF to pcie_slv0 BAR registers for BAR sizing */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        write_pcie_slv0_reg(pcie_bar_offsets[i], 0xFFFFFFFF);
    }
    LOGT("BAR sizing: wrote 0xFFFFFFFF to pcie_slv0 offsets 0x10-0x24");

    /* Read back pcie_slv0 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        data_rd = read_pcie_slv0_reg(pcie_bar_offsets[i]);
        LOGT("pcie_slv0 BAR[%u] offset=0x%x sizing readback=0x%x", i, pcie_bar_offsets[i], data_rd);
    }

    /* Write specific address values to pcie_slv0 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        write_pcie_slv0_reg(pcie_bar_offsets[i], pcie_bar_values[i]);
    }
    LOGT("BAR assignment: wrote address values to pcie_slv0 offsets 0x10-0x24");

    /* Read back pcie_slv0 BAR registers */
    for (i = 0U; i < PCIE_BAR_COUNT; i++) {
        data_rd = read_pcie_slv0_reg(pcie_bar_offsets[i]);
        LOGT("pcie_slv0 BAR[%u] offset=0x%x assignment readback=0x%x", i, pcie_bar_offsets[i], data_rd);
    }

    /* Step 28: wait_on(10) */
    wait_on(10);
    LOGT("wait_on(10) complete after BAR programming");

    /* Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    timeout = PCIE_POLL_TIMEOUT;
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
        out->status = -1;
    } else {
        LOGT("Sync register 0x%lx matched expected 0x%x",
             (unsigned long)PCIE_SYNC_REG, PCIE_SYNC_EXPECTED);
    }

    /* Step 30: finish(0) - converted to PSV/FV-native status reporting */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to out->status based PASS/FAIL reporting.
    if (g_ctx.errors == 0U) {
        out->status = 0;
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

    LOGT("pcie_device_enumerate_test teardown: errors=%u", g_ctx.errors);

    /* Validation summary */
    // Validation criteria 1-2: Link status polling was performed in run phase.
    // Validation criteria 3: Vendor ID was read and logged in run phase under DM0_RC.
    // Validation criteria 4-5: BAR sizing and assignment were performed and read back in run phase.
    // Validation criteria 6: Final sync register poll was performed in run phase.
    // Validation criteria 7: finish(0) converted to PSV/FV-native status.

    if (g_ctx.errors != 0U) {
        LOGE("pcie_device_enumerate_test FAILED with %u errors", g_ctx.errors);
    } else {
        LOGT("pcie_device_enumerate_test PASSED");
    }

    return g_ctx.errors == 0U ? 0 : -1;
}
