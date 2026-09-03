// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              pcie_device_enumerate_test. Initializes the control register
 *              and performs link training based on compile-time controller mode.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    LOGI("[Init] Control register 0xE6004100 cleared to 0x0\n");

    /* Step 2: Conditionally perform link training */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGI("[Init] link_training_dm0_x4(4) called for DM0_RC\n");
    #endif

    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGI("[Init] link_training_dm1_x4(4) called for DM1_RC\n");
    #endif

    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGI("[Init] link_training_dm0_x4(4) called for DM0_EP\n");
    #endif

    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGI("[Init] link_training_dm1_x4(4) called for DM1_EP\n");
    #endif

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration and link
 *              training. Performs cache programming, SII link-up polling,
 *              Vendor ID read, memory base programming, system register
 *              configuration, cache disable, BAR probing, and completion
 *              synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    test_err = 0;
    LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);

    /* Step 3: CACHE PROGRAMMING - PCIE0 phase 1 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 phase1 bits [11:14],[3:6] set to 0xf\n");

    /* Step 4: CACHE PROGRAMMING - PCIE0 phase 2 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 phase2 bits [27:30],[19:22] set to 0xf\n");

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 phase1 bits [11:14],[3:6] set to 0xf\n");

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 phase2 bits [27:30],[19:22] set to 0xf\n");

    /* Step 6: Wait for cache programming to take effect */
    wait_on(20);
    LOGI("[Run] wait_on(20) after initial cache programming\n");

    /* Step 7: Second round cache programming - PCIE0 all fields */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 all cache fields set to 0xf\n");

    /* Step 8: Second round cache programming - PCIE1 all fields */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 all cache fields set to 0xf\n");

    /* Step 9: Read SII0 register and call non_secure_prot_nic */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    LOGI("[Run] SII0 reg 0xC0 initial read = 0x%x, non_secure_prot_nic() called\n", data_rd);

    /* Step 10: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    LOGI("[Run] Polling SII0 link status register 0xC0\n");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        wait_on(5);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("[Run] SII0 link-up confirmed: data_rd = 0x%x\n", data_rd);

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    LOGI("[Run] Polling SII1 link status register 0xC0\n");
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        wait_on(5);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("[Run] SII1 link-up confirmed: data_rd = 0x%x\n", data_rd);

    /* Step 12: Under DM0_RC - Vendor ID, command reg, mem base programming */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("[Run] Vendor ID read from pcie_slv0 offset 0x0 = 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Run] Command register pcie_slv0 offset 0x4 written with 0x7\n");

        mem_base_program_dm0_x4();
        LOGI("[Run] mem_base_program_dm0_x4() called\n");

        mem_base_program_dm1_x4();
        LOGI("[Run] mem_base_program_dm1_x4() called\n");

        wait_on(10);
    #endif

    /* Step 13: Write system-level control registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    LOGI("[Run] System-level registers 0xE690000C-0xE6900034 written with 0x1\n");

    /* Step 14: DISABLE_CACHE PROGRAMMING - PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 cache disable: bits [19:22],[27:30] cleared to 0x0\n");

    /* Step 15: DISABLE_CACHE PROGRAMMING - PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 cache disable: bits [19:22],[27:30] cleared to 0x0\n");

    /* Step 16: Wait for cache disable to take effect */
    wait_on(10);
    LOGI("[Run] wait_on(10) after cache disable\n");

    /* Step 17: Clear all cache fields - PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 all cache fields cleared\n");

    /* Step 17 continued: Clear all cache fields - PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 all cache fields cleared\n");

    /* Step 18: Wait after clearing cache fields */
    wait_on(30);
    LOGI("[Run] wait_on(30) after clearing all cache fields\n");

    /* Step 19: BAR probing on pcie_slv1 */
    LOGI("[Run] BAR probing on pcie_slv1\n");

    /* BAR0 (offset 0x10) */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR0 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);

    /* BAR1 (offset 0x14) */
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR1 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);

    /* BAR2 (offset 0x18) */
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR2 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);

    /* BAR3 (offset 0x1c) */
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR3 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);

    /* BAR4 (offset 0x20) */
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR4 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);

    /* BAR5 (offset 0x24) */
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR5 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);

    /* Step 20: BAR probing on pcie_slv0 */
    LOGI("[Run] BAR probing on pcie_slv0\n");

    /* BAR0 (offset 0x10) */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR0 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);

    /* BAR1 (offset 0x14) */
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR1 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);

    /* BAR2 (offset 0x18) */
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR2 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);

    /* BAR3 (offset 0x1c) */
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR3 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);

    /* BAR4 (offset 0x20) */
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR4 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);

    /* BAR5 (offset 0x24) */
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR5 size read = 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);

    /* Step 21: Wait after BAR probing */
    wait_on(10);
    LOGI("[Run] wait_on(10) after BAR probing\n");

    /* Step 22: Poll 0xE6004100 until it reads 0x12345678 */
    LOGI("[Run] Polling 0xE6004100 for completion value 0x12345678\n");
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("[Run] Completion synchronization achieved: data_rd = 0x%x\n", data_rd);

    /* Step 23: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation observation and testcase completion for
 *              pcie_device_enumerate_test. Reports final test status.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe device enumerate test teardown: %s\n", cfg->test_name);

    /* Validation: SII0 and SII1 link-up, Vendor ID read, BAR sizing,
     * BAR programming, completion synchronization, and finish(0)
     * were all performed in the run phase. */

    return 0;
}
