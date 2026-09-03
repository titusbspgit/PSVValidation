// Author - AI Force 2.3. 03-Jul-2025 18:12 IST
// (EMBENGG-SYSAPPS)

/*
 * pcie_reg_wr_rd_test.c
 *
 * Test Case : pcie_reg_wr_rd_test
 * Description: PCIe register write and read operations to verify register-level
 *              data integrity. Initializes control register, performs x4 link
 *              training, programs cache coherency, polls SII link status,
 *              reads Vendor ID, writes known values to PCIe configuration and
 *              control registers, reads back and compares for data integrity,
 *              performs cache disable programming, and polls for completion
 *              synchronization.
 */

#include "pcie_reg_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int data_wr;
unsigned int test_err;

/*
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe reg wr/rd test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_reg_wr_rd_test_run
 * Description: Main testcase execution for pcie_reg_wr_rd_test. Performs link training,
 *              cache coherency programming, SII link status polling, Vendor ID read,
 *              register write with known values, register read-back, data integrity
 *              comparison, cache disable programming, system register writes, and
 *              completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe reg wr/rd test: %s\n", cfg->test_name);
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

    /* Step 19: Register Write Phase on pcie_slv1 */
    LOGI("Step 19: Register Write Phase on pcie_slv1\n");

    data_wr = REG_TEST_VAL_0;
    write_pcie_slv1_reg(0x10, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x10 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_1;
    write_pcie_slv1_reg(0x14, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x14 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_2;
    write_pcie_slv1_reg(0x18, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x18 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_3;
    write_pcie_slv1_reg(0x1c, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x1c data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_4;
    write_pcie_slv1_reg(0x20, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x20 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_5;
    write_pcie_slv1_reg(0x24, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 19: pcie_slv1 write offset=0x24 data=0x%x\n", data_wr);
    #endif

    /* Step 20: Register Read Phase on pcie_slv1 - read back and compare */
    LOGI("Step 20: Register Read Phase on pcie_slv1\n");

    data_rd = read_pcie_slv1_reg(0x10);
    if (data_rd != REG_TEST_VAL_0)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x10 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_0);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x10 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv1_reg(0x14);
    if (data_rd != REG_TEST_VAL_1)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x14 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_1);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x14 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv1_reg(0x18);
    if (data_rd != REG_TEST_VAL_2)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x18 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_2);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x18 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv1_reg(0x1c);
    if (data_rd != REG_TEST_VAL_3)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x1c read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_3);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x1c data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv1_reg(0x20);
    if (data_rd != REG_TEST_VAL_4)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x20 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_4);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x20 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv1_reg(0x24);
    if (data_rd != REG_TEST_VAL_5)
    {
        LOGI("ERROR: Step 20: pcie_slv1 reg mismatch at offset=0x24 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_5);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 20: pcie_slv1 reg match at offset=0x24 data=0x%x\n", data_rd);
    }
    #endif

    /* Step 21: Register Write Phase on pcie_slv0 */
    LOGI("Step 21: Register Write Phase on pcie_slv0\n");

    data_wr = REG_TEST_VAL_0;
    write_pcie_slv0_reg(0x10, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x10 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_1;
    write_pcie_slv0_reg(0x14, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x14 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_2;
    write_pcie_slv0_reg(0x18, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x18 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_3;
    write_pcie_slv0_reg(0x1c, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x1c data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_4;
    write_pcie_slv0_reg(0x20, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x20 data=0x%x\n", data_wr);
    #endif

    data_wr = REG_TEST_VAL_5;
    write_pcie_slv0_reg(0x24, data_wr);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 21: pcie_slv0 write offset=0x24 data=0x%x\n", data_wr);
    #endif

    /* Step 22: Register Read Phase on pcie_slv0 - read back and compare */
    LOGI("Step 22: Register Read Phase on pcie_slv0\n");

    data_rd = read_pcie_slv0_reg(0x10);
    if (data_rd != REG_TEST_VAL_0)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x10 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_0);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x10 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv0_reg(0x14);
    if (data_rd != REG_TEST_VAL_1)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x14 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_1);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x14 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv0_reg(0x18);
    if (data_rd != REG_TEST_VAL_2)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x18 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_2);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x18 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv0_reg(0x1c);
    if (data_rd != REG_TEST_VAL_3)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x1c read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_3);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x1c data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv0_reg(0x20);
    if (data_rd != REG_TEST_VAL_4)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x20 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_4);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x20 data=0x%x\n", data_rd);
    }
    #endif

    data_rd = read_pcie_slv0_reg(0x24);
    if (data_rd != REG_TEST_VAL_5)
    {
        LOGI("ERROR: Step 22: pcie_slv0 reg mismatch at offset=0x24 read=0x%x expected=0x%x\n", data_rd, REG_TEST_VAL_5);
        test_err++;
    }
    #ifdef DEBUG_DISPLAY
    else
    {
        LOGI("SUCCESS: Step 22: pcie_slv0 reg match at offset=0x24 data=0x%x\n", data_rd);
    }
    #endif

    LOGI("Step 22: Register data integrity verification complete, errors=%u\n", test_err);

    /* Step 23: Wait after register probing */
    wait_on(10);

    /* Step 24: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 24: Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("Step 24: Completion sync confirmed, data_rd=0x%x\n", data_rd);

    /* Step 25: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs validation summary, cleanup, and final observation for pcie_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe reg wr/rd test: %s\n", cfg->test_name);

    return 0;
}
