// Author - AI Force 2.3. 03-Jul-2025 18:12 IST
// (EMBENGG-SYSAPPS)

/*
 * pcie_mem_wr_rd_test.c
 *
 * Test Case : pcie_mem_wr_rd_test
 * Description: PCIe memory write and read operations to verify data integrity
 *              across the PCIe link. Initializes control register, performs x4
 *              link training, programs cache coherency, polls SII link status,
 *              reads Vendor ID, writes test data patterns to PCIe memory space,
 *              reads back and compares for data integrity, and polls for
 *              completion synchronization.
 */

#include "pcie_mem_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int data_wr;
unsigned int test_err;
unsigned int i;

/*
 * Function: pcie_mem_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe mem wr/rd test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Main testcase execution for pcie_mem_wr_rd_test. Performs link training,
 *              cache coherency programming, SII link status polling, Vendor ID read,
 *              memory write with test data patterns, memory read-back, data integrity
 *              comparison, system register writes, and completion synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe mem wr/rd test: %s\n", cfg->test_name);
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

    /* Step 7: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & SII_LINK_STATUS_MASK) != SII_LINK_STATUS_EXPECT)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 7: Polling SII0 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("Step 7: SII0 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 8: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & SII_LINK_STATUS_MASK) != SII_LINK_STATUS_EXPECT)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 8: Polling SII1 link status, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("Step 8: SII1 link-up confirmed, data_rd=0x%x\n", data_rd);

    /* Step 9: Under DM0_RC - Vendor ID read, command write, mem base program */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("Step 9: Vendor ID = 0x%x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 9: write_pcie_slv0_reg(0x4, 0x7) done\n");
        #endif

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 9: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() done\n");
        #endif

        wait_on(10);
    #endif

    /* Step 10: Memory Write Phase - write test data patterns to PCIe memory space */
    LOGI("Step 10: Memory Write Phase - writing test data patterns\n");
    for (i = 0; i < MEM_TEST_NUM_OFFSETS; i++)
    {
        data_wr = MEM_TEST_BASE_PATTERN ^ (i << 8) ^ (i * 0x11111111);

        write_pcie_slv0_reg(MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_wr);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 10: pcie_slv0 write offset=0x%x data=0x%x\n",
                 MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_wr);
        #endif

        write_pcie_slv1_reg(MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_wr);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 10: pcie_slv1 write offset=0x%x data=0x%x\n",
                 MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_wr);
        #endif
    }

    /* Step 11: Memory Read Phase - read back from the same addresses */
    LOGI("Step 11: Memory Read Phase - reading back data\n");
    for (i = 0; i < MEM_TEST_NUM_OFFSETS; i++)
    {
        data_wr = MEM_TEST_BASE_PATTERN ^ (i << 8) ^ (i * 0x11111111);

        /* Step 12: Data Comparison - pcie_slv0 */
        data_rd = read_pcie_slv0_reg(MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE));
        if (data_rd != data_wr)
        {
            LOGI("ERROR: Step 12: pcie_slv0 data mismatch at offset=0x%x read=0x%x expected=0x%x\n",
                 MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_rd, data_wr);
            test_err++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: Step 12: pcie_slv0 data match at offset=0x%x data=0x%x\n",
                     MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_rd);
            #endif
        }

        /* Step 12: Data Comparison - pcie_slv1 */
        data_rd = read_pcie_slv1_reg(MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE));
        if (data_rd != data_wr)
        {
            LOGI("ERROR: Step 12: pcie_slv1 data mismatch at offset=0x%x read=0x%x expected=0x%x\n",
                 MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_rd, data_wr);
            test_err++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: Step 12: pcie_slv1 data match at offset=0x%x data=0x%x\n",
                     MEM_TEST_BASE_OFFSET + (i * MEM_TEST_STRIDE), data_rd);
            #endif
        }
    }

    LOGI("Step 12: Data integrity verification complete, errors=%u\n", test_err);

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

    /* Step 14: Poll read_reg(0xE6004100) until 0x12345678 */
    data_rd = read_reg(COMPLETION_SYNC_REG);
    while (data_rd != COMPLETION_SYNC_VALUE)
    {
        #ifdef DEBUG_DISPLAY
            LOGI("Step 14: Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        #endif
        wait_on(5);
        data_rd = read_reg(COMPLETION_SYNC_REG);
    }
    LOGI("Step 14: Completion sync confirmed, data_rd=0x%x\n", data_rd);

    /* Step 15: finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs validation summary, cleanup, and final observation for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe mem wr/rd test: %s\n", cfg->test_name);

    return 0;
}
