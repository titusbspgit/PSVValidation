// Author - AI Force 2.3. 07-Jul-2025 17:08 IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*
 * PCIe Device Enumerate Test
 * Description: This testcase performs PCIe device enumeration including link
 * training, cache programming, link status polling, Vendor ID read, command
 * register enable, memory base programming, system register writes, cache
 * disable, BAR sizing and assignment, and final synchronization polling.
 */

unsigned int data_rd;
unsigned int test_err;

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
    LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

    test_err = 0;

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
    (void)cfg;
    LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);

    /* Step 1: Initialize synchronization register */
    write_reg(0xE6004100, 0x0);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 1: write_reg(0xE6004100, 0x0) done\n");
    #endif

    /* Step 2: Conditional link training */
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
    #ifdef DEBUG_DISPLAY
        LOGI("Step 2: Link training complete\n");
    #endif

    /* Step 3: Cache programming PCIE0 - bits [11:14]=0xf, [3:6]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: Cache programming PCIE0 - bits [27:30]=0xf, [19:22]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat cache programming for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("Steps 3-5: Cache programming (first pass) complete\n");
    #endif

    /* Step 6: wait_on(20) */
    wait_on(20);

    /* Step 7: Combined cache programming PCIE0 - all bits */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Combined cache programming PCIE1 - all bits */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("Steps 7-8: Cache programming (second pass) complete\n");
    #endif

    /* Step 9: Repeat link training and cache programming sequence (duplicate block) */
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

    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: Duplicate link training and cache programming complete\n");
    #endif

    /* Step 10: Read read_sii0_reg(0xC0) into data_rd */
    data_rd = read_sii0_reg(0xC0);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 10: read_sii0_reg(0xC0) = 0x%x\n", data_rd);
    #endif

    /* Step 11: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    #ifdef DEBUG_DISPLAY
        LOGI("Step 11: non_secure_prot_nic() called\n");
    #endif

    /* Step 12: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGI("Polling sii0_reg(0xC0), data_rd=0x%x\n", data_rd);
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    #ifdef DEBUG_DISPLAY
        LOGI("Step 12: sii0_reg(0xC0) poll complete, data_rd=0x%x\n", data_rd);
    #endif

    /* Step 13: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGI("Polling sii1_reg(0xC0), data_rd=0x%x\n", data_rd);
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    #ifdef DEBUG_DISPLAY
        LOGI("Step 13: sii1_reg(0xC0) poll complete, data_rd=0x%x\n", data_rd);
    #endif

    /* Step 14: Under DM0_RC - read Vendor ID */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 14: Vendor ID = 0x%x\n", data_rd);
        #endif
    #endif

    /* Step 15: Write command register */
    #ifdef DM0_RC
        write_pcie_slv0_reg(0x4, 0x7);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 15: write_pcie_slv0_reg(0x4, 0x7) done\n");
        #endif
    #endif

    /* Step 16: Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4() */
    #ifdef DM0_RC
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 16: mem_base_program_dm0_x4 and dm1_x4 called\n");
        #endif
    #endif

    /* Step 17: wait_on(10) */
    wait_on(10);

    /* Step 18: Write 0x1 to system-level registers */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 18: System-level registers written with 0x1\n");
    #endif

    /* Step 19: Cache disable PCIE0 - bits [11:14]=0xf, [3:6]=0xf, then [27:30]=0xf, [19:22]=0x0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: Cache disable PCIE0 complete\n");
    #endif

    /* Step 20: Cache disable PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("Step 20: Cache disable PCIE1 complete\n");
    #endif

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

    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: Combined cache disable complete\n");
    #endif

    /* Step 22: wait_on(30) */
    wait_on(30);

    /* Step 23: Write 0xFFFFFFFF to pcie_slv1 BAR registers (BAR sizing) */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 23: pcie_slv1 BAR sizing writes complete\n");
    #endif

    /* Step 24: Read back pcie_slv1 BAR registers */
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR0 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR1 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR2 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR3 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR4 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 24: pcie_slv1 BAR5 read = 0x%x\n", data_rd);
    #endif

    /* Step 25: Write specific address values to pcie_slv1 BAR registers */
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 25: pcie_slv1 BAR assignment writes complete\n");
    #endif

    /* Step 26: Read back pcie_slv1 BAR registers after assignment */
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR0 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR1 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR2 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR3 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR4 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 26: pcie_slv1 BAR5 read = 0x%x\n", data_rd);
    #endif

    /* Step 27: Repeat BAR sizing and assignment for pcie_slv0 */
    /* Step 27a: Write 0xFFFFFFFF to pcie_slv0 BAR registers (BAR sizing) */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27a: pcie_slv0 BAR sizing writes complete\n");
    #endif

    /* Step 27b: Read back pcie_slv0 BAR registers */
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR0 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR1 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR2 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR3 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR4 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27b: pcie_slv0 BAR5 read = 0x%x\n", data_rd);
    #endif

    /* Step 27c: Write specific address values to pcie_slv0 BAR registers */
    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27c: pcie_slv0 BAR assignment writes complete\n");
    #endif

    /* Step 27d: Read back pcie_slv0 BAR registers after assignment */
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR0 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR1 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR2 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR3 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR4 read = 0x%x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 27d: pcie_slv0 BAR5 read = 0x%x\n", data_rd);
    #endif

    /* Step 28: wait_on(10) */
    wait_on(10);

    /* Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        LOGI("Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    #ifdef DEBUG_DISPLAY
        LOGI("Step 29: Poll complete, 0xE6004100 = 0x%x\n", data_rd);
    #endif

    /* Step 30: Call finish(0) */
    finish(0);

    return out->status = test_err;
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
    LOGI("[TEARDOWN] PCIe device enumerate test: %s\n", cfg->test_name);

    return 0;
}
