/*
 * pcie_device_enumerate_test.c
 *
 * Test Case : pcie_device_enumerate_test
 * Description: PCIe device enumeration and link training.
 *              Initializes control register, performs x4 link training,
 *              programs cache coherency, polls SII link status,
 *              reads Vendor ID, programs BARs, and polls for completion.
 */

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int test_err;

int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

    return 0;
}

int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 1: write_reg(0xE6004100, 0x0) done\n");
    #endif

    /* Step 2: Conditionally call link training based on defines */
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
        #endif
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
        #endif
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm0_x4(4) called (DM0_EP)\n");
        #endif
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 2: link_training_dm1_x4(4) called (DM1_EP)\n");
        #endif
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 phase 1 */
    /* Read COHERENCY_CONTROL_3_OFF, set bits [11:14]=0xf, [3:6]=0xf, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 3: PCIE0 cache prog phase1 [11:14]=0xf [3:6]=0xf done\n");
    #endif

    /* Step 4: PCIE0 phase 2 - set bits [27:30]=0xf, [19:22]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 4: PCIE0 cache prog phase2 [27:30]=0xf [19:22]=0xf done\n");
    #endif

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 5: PCIE1 cache prog phase1+phase2 done\n");
    #endif

    /* Step 6: Wait for cache programming to take effect */
    wait_on(20);

    /* Step 7: PCIE0 - set all cache fields [11:14]=0xf [3:6]=0xf [27:30]=0xf [19:22]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 7: PCIE0 cache prog all fields done\n");
    #endif

    /* Step 8: Repeat for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 8: PCIE1 cache prog all fields done\n");
    #endif

    /* Step 9: Read SII0 register and call non_secure_prot_nic */
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: read_sii0_reg(0xC0)=0x%x, non_secure_prot_nic() called\n", data_rd);
    #endif

    /* Step 10: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 10: Polling SII0 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("Step 10: SII0 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 11: Polling SII1 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("Step 11: SII1 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 12: Under DM0_RC - Vendor ID read, command write, mem base program */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("Step 12: Vendor ID = 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 12: write_pcie_slv0_reg(0x4, 0x7) done\n");
        #endif

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 12: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() done\n");
        #endif

        wait_on(10);
    #endif

    /* Step 13: Write system-level control registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 13: System-level control registers written with 0x1\n");
    #endif

    /* Step 14: DISABLE_CACHE PROGRAMMING - PCIE0 */
    /* Set fields [19:22]=0x0 and [27:30]=0x0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 14: PCIE0 cache disable [19:22]=0x0 [27:30]=0x0 done\n");
    #endif

    /* Step 15: Repeat cache disable for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 15: PCIE1 cache disable [19:22]=0x0 [27:30]=0x0 done\n");
    #endif

    /* Step 16: Wait for cache disable to take effect */
    wait_on(10);

    /* Step 17: Clear all cache fields for PCIE0 and PCIE1 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0x0);
    data_rd = set_data(data_rd, 3, 6, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 17: All cache fields cleared for PCIE0 and PCIE1\n");
    #endif

    /* Step 18: Wait after clearing cache fields */
    wait_on(30);

    /* Step 19: BAR probing on pcie_slv1 */
    /* Write 0xFFFFFFFF to BAR offsets, read back, then write final values, read back */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR0 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR1 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR2 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR3 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR4 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 BAR5 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);

    /* Step 20: BAR probing on pcie_slv0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR0 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR1 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR2 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR3 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR4 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: pcie_slv0 BAR5 sizing read=0x%x\n", data_rd);
    #endif
    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);

    /* Step 21: Wait after BAR probing */
    wait_on(10);

    /* Step 22: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 22: Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("Step 22: Completion sync confirmed, data_rd=0x%x\n", data_rd);

    /* Step 23: finish(0) */
    finish(0);

    return out->status = test_err;
}

int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe device enumerate test: %s\n", cfg->test_name);

    return 0;
}
