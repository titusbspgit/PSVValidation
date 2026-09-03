// Author - AI Force 2.3. 03-Jul-2025 15:30 IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              pcie_device_enumerate_test. Clears the system control register
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

    /* Step 1: Initialize control register by clearing to zero */
    write_reg(0xE6004100, 0x0);
    LOGI("[Init] Control register 0xE6004100 cleared to 0x0\n");

    /* Step 2: Conditionally call link training based on DM0_RC/DM1_RC/DM0_EP/DM1_EP */
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
    /* Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 phase1 written: 0x%x\n", data_rd);
    #endif

    /* Step 4: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, set bits [27:30]=0xf, [19:22]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 phase2 written: 0x%x\n", data_rd);
    #endif

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 phase1 written: 0x%x\n", data_rd);
    #endif

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 phase2 written: 0x%x\n", data_rd);
    #endif

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Step 7: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, set all cache bits, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE0 COHERENCY_CONTROL_3 all cache bits set: 0x%x\n", data_rd);
    #endif

    /* Step 8: Repeat for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE1 COHERENCY_CONTROL_3 all cache bits set: 0x%x\n", data_rd);
    #endif

    /* Step 9: Read SII0 register at offset 0xC0, call non_secure_prot_nic() */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    LOGI("[Run] SII0 reg 0xC0 initial read: 0x%x\n", data_rd);

    /* Step 10: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Waiting for SII0 link-up, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("[Run] SII0 link-up confirmed: 0x%x\n", data_rd);

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Waiting for SII1 link-up, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("[Run] SII1 link-up confirmed: 0x%x\n", data_rd);

    /* Step 12: Under DM0_RC - Vendor ID read, command register write, memory base programming */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("[Run] Vendor ID read from pcie_slv0 offset 0x0: 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Run] Command register pcie_slv0 offset 0x4 written with 0x7\n");

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        LOGI("[Run] Memory base programming complete for DM0 and DM1\n");

        wait_on(10);
    #endif

    /* Step 13: Write system-level control registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    LOGI("[Run] System-level control registers written with 0x1\n");

    /* Step 14: DISABLE_CACHE PROGRAMMING - PCIE0 */
    /* Read COHERENCY_CONTROL_3_OFF, set fields [19:22]=0x0, [27:30]=0x0, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE0 cache disable phase1: 0x%x\n", data_rd);
    #endif

    /* Step 15: Repeat for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] PCIE1 cache disable phase1: 0x%x\n", data_rd);
    #endif

    /* Step 16: wait_on(10) */
    wait_on(10);

    /* Step 17: Clear all cache fields for PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Clear all cache fields for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] All cache fields cleared for PCIE0 and PCIE1\n");

    /* Step 18: wait_on(30) */
    wait_on(30);

    /* Step 19: BAR probing on pcie_slv1 */
    /* Write 0xFFFFFFFF to BAR offsets, read back size, write final values, read back */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x10 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x14 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x18 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x1c size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x20 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv1 BAR 0x24 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGI("[Run] BAR probing on pcie_slv1 complete\n");

    /* Step 20: BAR probing on pcie_slv0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x10 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x14 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x18 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x1c size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x20 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Run] pcie_slv0 BAR 0x24 size: 0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGI("[Run] BAR probing on pcie_slv0 complete\n");

    /* Step 21: wait_on(10) */
    wait_on(10);

    /* Step 22: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Polling 0xE6004100, current=0x%x\n", data_rd);
        #endif
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("[Run] Synchronization register 0xE6004100 matched 0x12345678\n");

    /* Step 23: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation observation and testcase teardown for
 *              pcie_device_enumerate_test. Reports final status.
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
     * 4. BAR sizing - performed in run on pcie_slv0 and pcie_slv1.
     * 5. BAR programming - performed in run on pcie_slv0 and pcie_slv1.
     * 6. Completion synchronization - polling 0xE6004100 until 0x12345678 in run.
     * 7. finish(0) - called in run.
     */

    return 0;
}
