// Author - AI Force 2.3. 03-Sep-2026 15:27 IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              PCIe device enumeration and link training test.
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
    LOGI("[Init] Control register 0xE6004100 initialized to 0x0\n");

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration and link
 *              training. Performs link training, cache programming, SII polling,
 *              Vendor ID read, system register configuration, cache disable,
 *              BAR probing, and completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 2: Conditionally call link training based on compile-time defines */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGI("[Run] link_training_dm0_x4(4) called for DM0_RC\n");
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGI("[Run] link_training_dm1_x4(4) called for DM1_RC\n");
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGI("[Run] link_training_dm0_x4(4) called for DM0_EP\n");
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGI("[Run] link_training_dm1_x4(4) called for DM1_EP\n");
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 phase 1 */
    /* Read COHERENCY_CONTROL_3_OFF, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase1a programmed: 0x%x\n", data_rd);

    /* Step 4: Read COHERENCY_CONTROL_3_OFF again, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase1b programmed: 0x%x\n", data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache phase1a programmed: 0x%x\n", data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache phase1b programmed: 0x%x\n", data_rd);

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
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase2 programmed: 0x%x\n", data_rd);

    /* Step 8: Repeat for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache phase2 programmed: 0x%x\n", data_rd);

    /* Step 9: Read SII0 register and call non_secure_prot_nic() */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    LOGI("[Run] SII0 initial read: 0x%x, non_secure_prot_nic() called\n", data_rd);

    /* Step 10: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("[Run] SII0 link-up confirmed: 0x%x\n", data_rd);

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("[Run] SII1 link-up confirmed: 0x%x\n", data_rd);

    /* Step 12: Under DM0_RC - Vendor ID read, command register, mem base programming */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("[Run] Vendor ID read from pcie_slv0 offset 0x0: 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Run] Command register pcie_slv0 offset 0x4 written with 0x7\n");

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        LOGI("[Run] mem_base_program_dm0_x4() and mem_base_program_dm1_x4() called\n");

        wait_on(10);
        LOGI("[Run] wait_on(10) after mem base programming\n");
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
    /* Set fields [19:22] and [27:30] to 0x0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache disable programmed: 0x%x\n", data_rd);

    /* Step 15: Repeat cache disable for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache disable programmed: 0x%x\n", data_rd);

    /* Step 16: Wait for cache disable to take effect */
    wait_on(10);
    LOGI("[Run] wait_on(10) after cache disable\n");

    /* Step 17: Clear all cache fields */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF all cache fields cleared: 0x%x\n", data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF all cache fields cleared: 0x%x\n", data_rd);

    /* Step 18: Wait after clearing cache fields */
    wait_on(30);
    LOGI("[Run] wait_on(30) after clearing all cache fields\n");

    /* Step 19: BAR probing on pcie_slv1 */
    /* Write 0xFFFFFFFF to BAR offsets, read back for sizing, then write final values */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x10 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x14 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x18 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x1c sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x20 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR offset 0x24 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);

    LOGI("[Run] BAR probing on pcie_slv1 complete\n");

    /* Step 20: BAR probing on pcie_slv0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x10 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x14 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x18 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x1c sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x20 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR offset 0x24 sizing read: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);

    LOGI("[Run] BAR probing on pcie_slv0 complete\n");

    /* Step 21: Wait after BAR probing */
    wait_on(10);
    LOGI("[Run] wait_on(10) after BAR probing\n");

    /* Step 22: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("[Run] Completion synchronization: 0xE6004100 reads 0x%x\n", data_rd);

    /* Step 23: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation observations, cleanup, and testcase
 *              completion for PCIe device enumeration and link training test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe device enumerate test teardown: %s\n", cfg->test_name);

    /* Validation observations:
     * 1. SII0 link status polling until (data_rd & 0xD1) == 0xD1 - verified in run.
     * 2. SII1 link status polling - verified in run.
     * 3. Vendor ID read - performed in run under DM0_RC.
     * 4. BAR sizing - performed in run on pcie_slv1 and pcie_slv0.
     * 5. BAR programming - performed in run on pcie_slv1 and pcie_slv0.
     * 6. Completion synchronization - polling 0xE6004100 for 0x12345678 in run.
     * 7. finish(0) - called in run.
     */

    return 0;
}
