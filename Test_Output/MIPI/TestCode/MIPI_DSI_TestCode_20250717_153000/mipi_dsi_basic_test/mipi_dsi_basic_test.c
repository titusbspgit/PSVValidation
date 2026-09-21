// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_basic_test.h"
#include "test_define.inc"

/*
 * mipi_dsi_basic_test
 *
 * Description: Writes to a control register, conditionally invokes link training
 * functions based on compile-time defines, performs cache programming by
 * reading/modifying/writing coherency control registers, polls status registers,
 * programs memory bases, writes to hardcoded addresses, disables cache programming,
 * programs BAR registers, and polls for final completion value.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_dsi_basic_test_ctx_t;

static mipi_dsi_basic_test_ctx_t g_ctx;

/*
 * Function: mipi_dsi_basic_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_basic_test_ctx_t){0};

    LOGT("mipi_dsi_basic_test_init: Initialization complete");

    return 0;
}

/*
 * Function: mipi_dsi_basic_test_run
 * Description: Executes the main testcase flow for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint32_t data_rd;
    uint32_t timeout;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_basic_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_basic_test_run: Starting testcase execution");

    /* Step 1: Write 0x0 to hardcoded control register */
    // MANUAL_REVIEW: The hardcoded control register address is not provided in the Meta TestPlan JSON.
    // writel_reg(<HARDCODED_CTRL_REG_ADDR>, 0x0);
    LOGT("Step 1: Write 0x0 to hardcoded control register (address not provided)");

    /* Step 2: Conditionally call link training based on compile-time defines */
#if defined(DM0_RC)
    LOGT("Step 2: Calling link_training_dm0_x4(4) under DM0_RC");
    link_training_dm0_x4(4);
#elif defined(DM1_RC)
    LOGT("Step 2: Calling link_training_dm1_x4(4) under DM1_RC");
    link_training_dm1_x4(4);
#elif defined(DM0_EP)
    LOGT("Step 2: Calling link_training_dm0_x4(4) under DM0_EP");
    link_training_dm0_x4(4);
#elif defined(DM1_EP)
    LOGT("Step 2: Calling link_training_dm1_x4(4) under DM1_EP");
    link_training_dm1_x4(4);
#endif

    /* Step 3: Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, modify bit fields [11:14] and [3:6] with 0xF, write back */
    LOGT("Step 3: Cache programming - PCIE0 coherency control bit fields [11:14] and [3:6]");
    data_rd = readl_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    writel_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, modify bit fields [27:30] and [19:22] with 0xF, write back */
    LOGT("Step 4: Cache programming - PCIE0 coherency control bit fields [27:30] and [19:22]");
    data_rd = readl_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    writel_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF */
    LOGT("Step 5: Cache programming - PCIE1 coherency control bit fields [11:14] and [3:6]");
    data_rd = readl_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    writel_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 5: Cache programming - PCIE1 coherency control bit fields [27:30] and [19:22]");
    data_rd = readl_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    writel_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: wait_on(20) */
    LOGT("Step 6: wait_on(20)");
    wait_on(20);

    /* Step 7: Repeat combined bit field modifications for both coherency control registers */
    LOGT("Step 7: Repeat combined bit field modifications for PCIE0");
    data_rd = readl_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    writel_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 7: Repeat combined bit field modifications for PCIE1");
    data_rd = readl_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    writel_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Call non_secure_prot_nic() */
    LOGT("Step 8: Calling non_secure_prot_nic()");
    non_secure_prot_nic();

    /* Step 9: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 9: Polling read_sii0_reg(0xC0) for status bits 0xD1");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    do {
        data_rd = read_sii0_reg(0xC0);
        if ((data_rd & 0xD1U) == 0xD1U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Step 9: Timeout polling read_sii0_reg(0xC0) for 0xD1");
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("Step 9: read_sii0_reg(0xC0) status met: data_rd=0x%lx", (unsigned long)data_rd);
    }

    /* Step 10: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 10: Polling read_sii1_reg(0xC0) for status bits 0xD1");
    timeout = MIPI_DSI_POLL_TIMEOUT;
    do {
        data_rd = read_sii1_reg(0xC0);
        if ((data_rd & 0xD1U) == 0xD1U) {
            break;
        }
        timeout--;
    } while (timeout > 0U);
    if (timeout == 0U) {
        LOGE("Step 10: Timeout polling read_sii1_reg(0xC0) for 0xD1");
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("Step 10: read_sii1_reg(0xC0) status met: data_rd=0x%lx", (unsigned long)data_rd);
    }

    /* Step 11: Under DM0_RC: read vendor ID, write command register, program memory bases */
#if defined(DM0_RC)
    LOGT("Step 11: DM0_RC - Reading vendor ID via read_pcie_slv0_reg(0x0)");
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Step 11: Vendor ID read: 0x%lx", (unsigned long)data_rd);

    // MANUAL_REVIEW: Command register write address and value not fully specified in Meta TestPlan JSON.
    // write_pcie_slv0_reg(<CMD_REG_OFFSET>, <CMD_VALUE>);

    LOGT("Step 11: Calling mem_base_program_dm0_x4()");
    mem_base_program_dm0_x4();

    LOGT("Step 11: Calling mem_base_program_dm1_x4()");
    mem_base_program_dm1_x4();

    LOGT("Step 11: wait_on(10)");
    wait_on(10);
#endif

    /* Step 12: Write to six hardcoded addresses */
    // MANUAL_REVIEW: The six hardcoded addresses and their values are not provided in the Meta TestPlan JSON.
    LOGT("Step 12: Write to six hardcoded addresses (addresses not provided)");

    /* Step 13: Disable cache by clearing bit fields [27:30] and [19:22] to 0x0 in both coherency control registers */
    LOGT("Step 13: Disable cache - PCIE0 clearing bit fields [27:30] and [19:22]");
    data_rd = readl_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    writel_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 13: Disable cache - PCIE1 clearing bit fields [27:30] and [19:22]");
    data_rd = readl_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    writel_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 14: wait_on(10), repeat disable cache with combined fields */
    LOGT("Step 14: wait_on(10)");
    wait_on(10);

    LOGT("Step 14: Repeat disable cache with combined fields for PCIE0");
    data_rd = readl_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    writel_reg(SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 14: Repeat disable cache with combined fields for PCIE1");
    data_rd = readl_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    writel_reg(SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 15: wait_on(30) */
    LOGT("Step 15: wait_on(30)");
    wait_on(30);

    /* Step 16: Write 0xFFFFFFFF to BAR registers via write_pcie_slv1_reg, read back, reprogram, read back */
    LOGT("Step 16: BAR programming via write_pcie_slv1_reg");
    // MANUAL_REVIEW: BAR register offsets and reprogram values are not fully specified in the Meta TestPlan JSON.
    // write_pcie_slv1_reg(<BAR_OFFSET>, 0xFFFFFFFF);
    // data_rd = read_pcie_slv1_reg(<BAR_OFFSET>);
    // write_pcie_slv1_reg(<BAR_OFFSET>, <SPECIFIC_VALUE>);
    // data_rd = read_pcie_slv1_reg(<BAR_OFFSET>);

    /* Step 17: Repeat BAR programming for pcie_slv0 */
    LOGT("Step 17: BAR programming via write_pcie_slv0_reg");
    // MANUAL_REVIEW: BAR register offsets and reprogram values are not fully specified in the Meta TestPlan JSON.
    // write_pcie_slv0_reg(<BAR_OFFSET>, 0xFFFFFFFF);
    // data_rd = read_pcie_slv0_reg(<BAR_OFFSET>);
    // write_pcie_slv0_reg(<BAR_OFFSET>, <SPECIFIC_VALUE>);
    // data_rd = read_pcie_slv0_reg(<BAR_OFFSET>);

    /* Step 18: wait_on(10) */
    LOGT("Step 18: wait_on(10)");
    wait_on(10);

    /* Step 19: Poll hardcoded register until value equals 0x12345678 */
    LOGT("Step 19: Polling hardcoded register for value 0x12345678");
    // MANUAL_REVIEW: The hardcoded register address for final poll is not provided in the Meta TestPlan JSON.
    // timeout = MIPI_DSI_POLL_TIMEOUT;
    // do {
    //     data_rd = readl_reg(<HARDCODED_POLL_ADDR>);
    //     if (data_rd == 0x12345678U) {
    //         break;
    //     }
    //     timeout--;
    // } while (timeout > 0U);
    // if (timeout == 0U) {
    //     LOGE("Step 19: Timeout polling for 0x12345678");
    //     g_ctx.errors++;
    //     out->status = -1;
    // }

    /* Step 20: finish(0) - converted to PSV/FV-native completion */
    // MANUAL_REVIEW: DV finish(0) was present in the source flow. Converted to FV-native out->status reporting.

    g_ctx.checks_failed = g_ctx.errors;

    if (out->status != -1) {
        out->status = (g_ctx.errors == 0U) ? 0 : -1;
    }

    LOGT("mipi_dsi_basic_test_run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_dsi_basic_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_dsi_basic_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_basic_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_basic_test_teardown: errors=%u checks_passed=%u checks_failed=%u",
         g_ctx.errors, g_ctx.checks_passed, g_ctx.checks_failed);

    /* Validation: BAR register readbacks were performed but no explicit comparison */
    /* or assertion is present in the source code for those values. */

    LOGT("mipi_dsi_basic_test_teardown: cleanup complete");

    return (g_ctx.errors == 0U) ? 0 : -1;
}
