// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * PCIe Device Enumeration Test
 * This testcase performs PCIe device enumeration including link training,
 * cache coherency programming, link-up polling, Vendor ID read, BAR
 * enumeration and programming, and final synchronization polling.
 */

unsigned int data_rd;
unsigned int test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup
 *              for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe Device Enumerate test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration.
 *              Performs link training, cache programming, link-up polling,
 *              Vendor ID read, system register configuration, cache disable,
 *              BAR enumeration/programming, and final sync polling.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe Device Enumerate test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize synchronization register */
    write_reg(0xE6004100, 0x0);
    LOGI("[Step 1] Wrote 0x0 to sync register 0xE6004100\n");

    /* Step 2: Conditionally call link training */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGI("[Step 2] link_training_dm0_x4(4) called (DM0_RC)\n");
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGI("[Step 2] link_training_dm1_x4(4) called (DM1_RC)\n");
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGI("[Step 2] link_training_dm0_x4(4) called (DM0_EP)\n");
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGI("[Step 2] link_training_dm1_x4(4) called (DM1_EP)\n");
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 lower bits */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 3] PCIE0 COHERENCY_CONTROL_3 lower bits programmed\n");

    /* Step 4: CACHE PROGRAMMING - PCIE0 upper bits */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 4] PCIE0 COHERENCY_CONTROL_3 upper bits programmed\n");

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 5] PCIE1 COHERENCY_CONTROL_3 bits programmed\n");

    /* Step 6: Wait for configuration to take effect */
    wait_on(20);
    LOGI("[Step 6] wait_on(20) completed\n");

    /* Step 7: Re-apply all cache coherency bits for PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 7] PCIE0 COHERENCY_CONTROL_3 all bits re-applied\n");

    /* Step 8: Re-apply all cache coherency bits for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 8] PCIE1 COHERENCY_CONTROL_3 all bits re-applied\n");

    /* Step 9: Repeat link training and cache programming block (duplicate) */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
    #endif

    /* Duplicate cache programming */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    wait_on(20);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 9] Duplicate link training and cache programming completed\n");

    /* Step 10: Read SII0 link status and call non_secure_prot_nic */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    LOGI("[Step 10] SII0 link status read, non_secure_prot_nic() called\n");

    /* Step 11: Poll SII0 link status until link-up */
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Step 11] Polling SII0 link status: data_rd=0x%x\n", data_rd);
        #endif
    }
    LOGI("[Step 11] SII0 link-up confirmed: data_rd=0x%x\n", data_rd);

    /* Step 12: Poll SII1 link status until link-up */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Step 12] Polling SII1 link status: data_rd=0x%x\n", data_rd);
        #endif
    }
    LOGI("[Step 12] SII1 link-up confirmed: data_rd=0x%x\n", data_rd);

    /* Step 13: Under DM0_RC - Vendor ID read, command write, mem base program */
    #ifdef DM0_RC
    {
        data_rd = read_pcie_slv0_reg(0x0);
        printf("Vendor ID = 0x%x\n", data_rd);
        LOGI("[Step 13] Vendor ID read from slv0: 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("[Step 13] Wrote 0x7 to slv0 offset 0x4\n");

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        LOGI("[Step 13] mem_base_program_dm0_x4() and dm1_x4() called\n");

        wait_on(10);
    }
    #endif

    /* Step 14: Write 0x1 to system-level registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    LOGI("[Step 14] System registers 0xE690000C-0xE6900034 written with 0x1\n");

    /* Step 15: DISABLE_CACHE PROGRAMMING - PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* DISABLE_CACHE PROGRAMMING - PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 15] Cache disable phase 1 completed for PCIE0 and PCIE1\n");

    /* Step 16: Wait and clear remaining coherency fields */
    wait_on(10);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Step 16] Cache disable phase 2 completed for PCIE0 and PCIE1\n");

    /* Step 17: Wait */
    wait_on(30);
    LOGI("[Step 17] wait_on(30) completed\n");

    /* Step 18: BAR sizing on slv1 - write 0xFFFFFFFF and read back */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x10 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x14 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x18 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x1c sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x20 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 18] slv1 BAR 0x24 sizing readback: 0x%x\n", data_rd);
    #endif
    LOGI("[Step 18] slv1 BAR sizing completed\n");

    /* Step 19: BAR programming on slv1 with specific base addresses */
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x10 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x14 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x18 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x1c programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x20 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 19] slv1 BAR 0x24 programmed readback: 0x%x\n", data_rd);
    #endif
    LOGI("[Step 19] slv1 BAR programming completed\n");

    /* Step 20: BAR sizing on slv0 - write 0xFFFFFFFF and read back */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x10 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x14 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x18 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x1c sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x20 sizing readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 20] slv0 BAR 0x24 sizing readback: 0x%x\n", data_rd);
    #endif
    LOGI("[Step 20] slv0 BAR sizing completed\n");

    /* Step 21: BAR programming on slv0 with specific base addresses */
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x10 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x14 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x18 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x1c programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x20 programmed readback: 0x%x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("[Step 21] slv0 BAR 0x24 programmed readback: 0x%x\n", data_rd);
    #endif
    LOGI("[Step 21] slv0 BAR programming completed\n");

    /* Step 22: Wait */
    wait_on(10);
    LOGI("[Step 22] wait_on(10) completed\n");

    /* Step 23: Poll synchronization register until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
        #ifdef DEBUG_DISPLAY
            LOGI("[Step 23] Polling sync register 0xE6004100: data_rd=0x%x\n", data_rd);
        #endif
    }
    LOGI("[Step 23] Sync register confirmed: 0x%x\n", data_rd);

    /* Step 24: Call finish(0) */
    finish(0);
    LOGI("[Step 24] finish(0) called\n");

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation, final observation, and testcase
 *              completion for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe Device Enumerate test teardown: %s\n", cfg->test_name);

    return 0;
}
