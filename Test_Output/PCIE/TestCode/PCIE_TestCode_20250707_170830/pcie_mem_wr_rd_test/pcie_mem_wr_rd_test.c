// Author - AI Force 2.3. 07-Jul-2025 17:08 IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * PCIe Memory Write Read Test
 * Description: This testcase validates PCIe memory write and read operations
 * through the PCIe slave interfaces. It performs link training, cache
 * programming, link status polling, BAR programming, memory base programming,
 * cache disable, memory write/read operations, and final synchronization polling.
 */

unsigned int data_rd;
unsigned int test_err;

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
    LOGI("[Test Init] PCIe mem wr rd test: %s\n", cfg->test_name);

    test_err = 0;

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIe mem wr rd test: %s\n", cfg->test_name);

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

    /* Step 3: Cache enable programming PCIE0 - bits [11:14]=0xf, [3:6]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: Cache enable programming PCIE0 - bits [27:30]=0xf, [19:22]=0xf */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat cache enable programming for PCIE1 */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    #ifdef DEBUG_DISPLAY
        LOGI("Steps 3-5: Cache enable programming (first pass) complete\n");
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

    /* Step 9: Read read_sii0_reg(0xC0) into data_rd */
    data_rd = read_sii0_reg(0xC0);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 9: read_sii0_reg(0xC0) = 0x%x\n", data_rd);
    #endif

    /* Step 10: Under DM0 - poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    #ifdef DM0
        data_rd = read_sii0_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            LOGI("Polling sii0_reg(0xC0), data_rd=0x%x\n", data_rd);
            wait_on(10);
            data_rd = read_sii0_reg(0xC0);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("Step 10: sii0_reg(0xC0) poll complete, data_rd=0x%x\n", data_rd);
        #endif
    #endif

    /* Step 11: Under DM1 - poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    #ifdef DM1
        data_rd = read_sii1_reg(0xC0);
        while ((data_rd & 0xD1) != 0xD1)
        {
            LOGI("Polling sii1_reg(0xC0), data_rd=0x%x\n", data_rd);
            wait_on(10);
            data_rd = read_sii1_reg(0xC0);
        }
        #ifdef DEBUG_DISPLAY
            LOGI("Step 11: sii1_reg(0xC0) poll complete, data_rd=0x%x\n", data_rd);
        #endif
    #endif

    /* Step 12: Under DM0_EP - wait_on(30000) */
    #ifdef DM0_EP
        wait_on(30000);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 12: wait_on(30000) done for DM0_EP\n");
        #endif
    #endif

    /* Step 13: Under DM0_RC - Vendor ID, command register, BAR and mem base programming */
    #ifdef DM0_RC
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 13: DM0_RC Vendor ID = 0x%x\n", data_rd);
        #endif
        write_pcie_slv0_reg(0x4, 0x7);
        bar_program_dm0_x4();
        wait_on(10);
        mem_base_program_dm0_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 13: DM0_RC BAR and mem base programming complete\n");
        #endif
    #endif

    /* Step 14: Under DM1_RC - Vendor ID, command register, BAR and mem base programming */
    #ifdef DM1_RC
        data_rd = read_pcie_slv1_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 14: DM1_RC Vendor ID = 0x%x\n", data_rd);
        #endif
        write_pcie_slv1_reg(0x4, 0x7);
        bar_program_dm1_x4();
        wait_on(10);
        mem_base_program_dm1_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 14: DM1_RC BAR and mem base programming complete\n");
        #endif
    #endif

    /* Step 15: Under DM0_EP - EP BAR programming and mem base programming */
    #ifdef DM0_EP
        bar_program_dm0_EP_x4();
        wait_on(10);
        mem_base_program_dm0_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 15: DM0_EP BAR and mem base programming complete\n");
        #endif
    #endif

    /* Step 16: Under DM1_EP - EP BAR programming and mem base programming */
    #ifdef DM1_EP
        bar_program_dm1_EP_x4();
        wait_on(10);
        mem_base_program_dm1_x4();
        #ifdef DEBUG_DISPLAY
            LOGI("Step 16: DM1_EP BAR and mem base programming complete\n");
        #endif
    #endif

    /* Step 17: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    #ifdef DEBUG_DISPLAY
        LOGI("Step 17: non_secure_prot_nic() called\n");
    #endif

    /* Step 18: Signal readiness */
    write_reg(0xE6004100, 0x11111111);
    #ifdef DEBUG_DISPLAY
        LOGI("Step 18: write_reg(0xE6004100, 0x11111111) done\n");
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

    /* Step 23: Under DM0_RC - memory write/read operations */
    #ifdef DM0_RC
        pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 23: pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5) done\n");
        #endif
        pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 23: pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6) done\n");
        #endif
        pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 23: pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7) done\n");
        #endif
    #endif

    /* Step 24: Under DM1_RC - memory write/read operations */
    #ifdef DM1_RC
        pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 24: pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5) done\n");
        #endif
        pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 24: pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6) done\n");
        #endif
        pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 24: pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5) done\n");
        #endif
    #endif

    /* Step 25: Under DM0_EP - memory write/read operations at five addresses */
    #ifdef DM0_EP
        pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 25: DM0_EP pcie_slv0_mem_wr_rd at 5 addresses complete\n");
        #endif
    #endif

    /* Step 26: Under DM1_EP - memory write/read operations at five addresses */
    #ifdef DM1_EP
        pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a);
        #ifdef DEBUG_DISPLAY
            LOGI("Step 26: DM1_EP pcie_slv1_mem_wr_rd at 5 addresses complete\n");
        #endif
    #endif

    /* Step 27: wait_on(10) */
    wait_on(10);

    /* Step 28: Poll read_reg(0xE6004100) until data_rd == 0x12345678 */
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        LOGI("Polling 0xE6004100, data_rd=0x%x\n", data_rd);
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    #ifdef DEBUG_DISPLAY
        LOGI("Step 28: Poll complete, 0xE6004100 = 0x%x\n", data_rd);
    #endif

    /* Step 29: Call finish(0) */
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_mem_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe mem wr rd test: %s\n", cfg->test_name);

    return 0;
}
