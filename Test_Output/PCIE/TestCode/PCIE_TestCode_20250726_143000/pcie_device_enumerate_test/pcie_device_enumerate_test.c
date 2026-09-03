// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

/* PCIe Device Enumerate Test
 * Description: This testcase performs PCIe device enumeration and link training.
 * It initializes control registers, performs link training, cache programming,
 * SII polling, Vendor ID read, BAR probing, and completion synchronization.
 */

unsigned int data_rd, test_err, rdata;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration including link training,
 *   cache programming, SII polling, Vendor ID read, system register writes, cache disable,
 *   BAR probing, and completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    LOGI("Step 1: write_reg(0xE6004100, 0x0) done\n");

    /* Step 2: Conditionally call link training based on defines */
    #ifdef DM0_RC
    link_training_dm0_x4(4);
    LOGI("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
    #endif
    #ifdef DM1_RC
    link_training_dm1_x4(4);
    LOGI("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
    #endif
    #ifdef DM0_EP
    link_training_dm0_x4(4);
    LOGI("Step 2: link_training_dm0_x4(4) called (DM0_EP)\n");
    #endif
    #ifdef DM1_EP
    link_training_dm1_x4(4);
    LOGI("Step 2: link_training_dm1_x4(4) called (DM1_EP)\n");
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 phase 1 */
    rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0xf);
    rdata = set_data(rdata, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 3: PCIE0 cache programming phase 1 done\n");

    /* Step 4: CACHE PROGRAMMING - PCIE0 phase 2 */
    rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 27, 30, 0xf);
    rdata = set_data(rdata, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 4: PCIE0 cache programming phase 2 done\n");

    /* Step 5: CACHE PROGRAMMING - PCIE1 phase 1 */
    rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0xf);
    rdata = set_data(rdata, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 5: PCIE1 cache programming phase 1 done\n");

    /* Step 5 continued: CACHE PROGRAMMING - PCIE1 phase 2 */
    rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 27, 30, 0xf);
    rdata = set_data(rdata, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 5: PCIE1 cache programming phase 2 done\n");

    /* Step 6: wait_on(20) */
    wait_on(20);
    LOGI("Step 6: wait_on(20) done\n");

    /* Step 7: CACHE PROGRAMMING - PCIE0 all fields */
    rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0xf);
    rdata = set_data(rdata, 3, 6, 0xf);
    rdata = set_data(rdata, 27, 30, 0xf);
    rdata = set_data(rdata, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 7: PCIE0 cache programming all fields done\n");

    /* Step 8: CACHE PROGRAMMING - PCIE1 all fields */
    rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0xf);
    rdata = set_data(rdata, 3, 6, 0xf);
    rdata = set_data(rdata, 27, 30, 0xf);
    rdata = set_data(rdata, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 8: PCIE1 cache programming all fields done\n");

    /* Step 9: Read SII0 register and call non_secure_prot_nic */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    LOGI("Step 9: read_sii0_reg(0xC0) and non_secure_prot_nic() done\n");

    /* Step 10: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    LOGI("Step 10: Polling SII0 link status\n");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
        LOGI("SII0 polling: data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("Step 10: SII0 link-up status achieved\n");

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    LOGI("Step 11: Polling SII1 link status\n");
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
        LOGI("SII1 polling: data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("Step 11: SII1 link-up status achieved\n");

    /* Step 12: Under DM0_RC - Vendor ID read, command write, mem base program */
    #ifdef DM0_RC
    {
        unsigned int vendor_id;
        vendor_id = read_pcie_slv0_reg(0x0);
        LOGI("Step 12: Vendor ID = 0x%x\n", vendor_id);

        write_pcie_slv0_reg(0x4, 0x7);
        LOGI("Step 12: write_pcie_slv0_reg(0x4, 0x7) done\n");

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        LOGI("Step 12: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() done\n");

        wait_on(10);
    }
    #endif

    /* Step 13: Write system-level registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    LOGI("Step 13: System-level registers written with 0x1\n");

    /* Step 14: DISABLE_CACHE PROGRAMMING - PCIE0 */
    rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 19, 22, 0x0);
    rdata = set_data(rdata, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 14: PCIE0 cache disable programming done\n");

    /* Step 15: DISABLE_CACHE PROGRAMMING - PCIE1 */
    rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 19, 22, 0x0);
    rdata = set_data(rdata, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 15: PCIE1 cache disable programming done\n");

    /* Step 16: wait_on(10) */
    wait_on(10);

    /* Step 17: Clear all cache fields */
    rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0x0);
    rdata = set_data(rdata, 3, 6, 0x0);
    rdata = set_data(rdata, 27, 30, 0x0);
    rdata = set_data(rdata, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);

    rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rdata = set_data(rdata, 11, 14, 0x0);
    rdata = set_data(rdata, 3, 6, 0x0);
    rdata = set_data(rdata, 27, 30, 0x0);
    rdata = set_data(rdata, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
    LOGI("Step 17: All cache fields cleared\n");

    /* Step 18: wait_on(30) */
    wait_on(30);

    /* Step 19: BAR probing on pcie_slv1 */
    LOGI("Step 19: BAR probing on pcie_slv1\n");
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR0 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x10, 0x0);
    rdata = read_pcie_slv1_reg(0x10);

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR1 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x14, 0x4);
    rdata = read_pcie_slv1_reg(0x14);

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR2 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x18, 0x20000000);
    rdata = read_pcie_slv1_reg(0x18);

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR3 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x1c, 0x40000000);
    rdata = read_pcie_slv1_reg(0x1c);

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR4 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x20, 0x60000000);
    rdata = read_pcie_slv1_reg(0x20);

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    rdata = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv1 BAR5 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv1_reg(0x24, 0x80000000);
    rdata = read_pcie_slv1_reg(0x24);

    /* Step 20: BAR probing on pcie_slv0 */
    LOGI("Step 20: BAR probing on pcie_slv0\n");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR0 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x10, 0x0);
    rdata = read_pcie_slv0_reg(0x10);

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR1 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x14, 0x4);
    rdata = read_pcie_slv0_reg(0x14);

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR2 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x18, 0x20000000);
    rdata = read_pcie_slv0_reg(0x18);

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR3 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x1c, 0x40000000);
    rdata = read_pcie_slv0_reg(0x1c);

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR4 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x20, 0x60000000);
    rdata = read_pcie_slv0_reg(0x20);

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    rdata = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
    LOGI("pcie_slv0 BAR5 sizing: 0x%x\n", rdata);
    #endif
    write_pcie_slv0_reg(0x24, 0x80000000);
    rdata = read_pcie_slv0_reg(0x24);

    /* Step 21: wait_on(10) */
    wait_on(10);

    /* Step 22: Poll read_reg(0xE6004100) until 0x12345678 */
    LOGI("Step 22: Polling 0xE6004100 for completion synchronization\n");
    rdata = read_reg(0xE6004100);
    while (rdata != 0x12345678)
    {
        wait_on(5);
        rdata = read_reg(0xE6004100);
    }
    LOGI("Step 22: Completion synchronization achieved (0x12345678)\n");

    /* Step 23: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs teardown and final observation for pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe device enumerate test teardown: %s\n", cfg->test_name);

    return 0;
}
