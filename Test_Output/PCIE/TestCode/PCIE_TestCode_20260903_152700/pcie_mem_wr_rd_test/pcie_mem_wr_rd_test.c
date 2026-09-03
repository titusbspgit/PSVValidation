// Author - AI Force 2.3. 03-Sep-2026 15:27 IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd, data_wr, test_err;

/*
 * Function: pcie_mem_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              PCIe memory write and read verification test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe mem wr rd test: %s\n", cfg->test_name);

    /* Step 1: Initialize control register */
    write_reg(0xE6004100, 0x0);
    LOGI("[Init] Control register 0xE6004100 initialized to 0x0\n");

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Main testcase execution for PCIe memory write and read
 *              verification through PCIe slave interfaces. Performs link
 *              training, cache programming, link-up polling, BAR programming,
 *              cache disable, memory write-read tests, and completion
 *              synchronization.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    LOGI("[Test Run] PCIe mem wr rd test: %s\n", cfg->test_name);
    test_err = 0;

    /* Step 2: Link training - conditionally call based on compile-time defines */
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

    /* Steps 3-4: CACHE PROGRAMMING - PCIE0 phase 1 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase1a programmed: 0x%x\n", data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase1b programmed: 0x%x\n", data_rd);

    /* Steps 5-6: Repeat cache programming for PCIE1 */
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

    /* Steps 7-8: Second round cache programming */
    wait_on(20);
    LOGI("[Run] wait_on(20) after initial cache programming\n");

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache phase2 programmed: 0x%x\n", data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache phase2 programmed: 0x%x\n", data_rd);

    /* Step 9: Poll SII0 link status */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
    }
    LOGI("[Run] SII0 link-up confirmed: 0x%x\n", data_rd);

    /* Step 10: Poll SII1 link status */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
    }
    LOGI("[Run] SII1 link-up confirmed: 0x%x\n", data_rd);

    /* Steps 11-12: Non-secure protection NIC call */
    non_secure_prot_nic();
    LOGI("[Run] non_secure_prot_nic() called\n");

    /* Steps 13-14: Under DM0_RC */
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

    /* Steps 15-16: Under DM1_RC */
    #ifdef DM1_RC
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

    /* Steps 17-18: DISABLE_CACHE PROGRAMMING - PCIE0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE0 COHERENCY_CONTROL_3_OFF cache disable programmed: 0x%x\n", data_rd);

    /* Steps 19-20: Repeat cache disable for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGI("[Run] PCIE1 COHERENCY_CONTROL_3_OFF cache disable programmed: 0x%x\n", data_rd);

    /* Step 21: Wait */
    wait_on(10);
    LOGI("[Run] wait_on(10) after cache disable\n");

    /* Steps 22-23: Clear all cache fields */
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

    wait_on(30);
    LOGI("[Run] wait_on(30) after clearing all cache fields\n");

    /* Steps 24-25: Memory write-read tests on pcie_slv0 */
    #ifdef DM0
        data_wr = 0xDEADBEEF;
        write_pcie_slv0_reg(0x0, data_wr);
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv0 mem write=0x%x read=0x%x\n", data_wr, data_rd);
        #endif
        if (data_rd != data_wr)
        {
            LOGI("ERROR: pcie_slv0 mem wr/rd mismatch: expected=0x%x actual=0x%x\n", data_wr, data_rd);
            test_err++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: pcie_slv0 mem wr/rd match: 0x%x\n", data_rd);
            #endif
        }
    #endif

    /* Steps 26-27: Memory write-read tests on pcie_slv1 */
    #ifdef DM1
        data_wr = 0xCAFEBABE;
        write_pcie_slv1_reg(0x0, data_wr);
        data_rd = read_pcie_slv1_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] pcie_slv1 mem write=0x%x read=0x%x\n", data_wr, data_rd);
        #endif
        if (data_rd != data_wr)
        {
            LOGI("ERROR: pcie_slv1 mem wr/rd mismatch: expected=0x%x actual=0x%x\n", data_wr, data_rd);
            test_err++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: pcie_slv1 mem wr/rd match: 0x%x\n", data_rd);
            #endif
        }
    #endif

    /* Step 28: Wait */
    wait_on(10);
    LOGI("[Run] wait_on(10) after memory write-read tests\n");

    /* Steps 29-30: Poll completion */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGI("[Run] Completion synchronization: 0xE6004100 reads 0x%x\n", data_rd);

    /* finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs validation observations, cleanup, and testcase
 *              completion for PCIe memory write and read verification test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe mem wr rd test teardown: %s\n", cfg->test_name);

    return 0;
}
