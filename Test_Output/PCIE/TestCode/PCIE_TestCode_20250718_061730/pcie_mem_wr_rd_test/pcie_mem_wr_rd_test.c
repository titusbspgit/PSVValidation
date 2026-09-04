// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/* Global variables */
unsigned int data_rd;
unsigned int test_err;
int err1;
int err2;

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
    LOGI("[Test Init] PCIE memory write-read test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: pcie_mem_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_mem_wr_rd_test including link training,
 *              cache coherency programming, BAR programming, cache disable, and memory
 *              write-read verification for both RC and EP modes.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIE memory write-read test: %s\n", cfg->test_name);
    test_err = 0;
    err1 = 0;
    err2 = 0;

    /* Step 1: Initialize control register */
    LOGI("Step 1: Initialize control register at 0xE6004100 to 0x0\n");
    write_reg(0xE6004100, 0x0);

    /* Step 2: Link training based on compile-time defines */
    LOGI("Step 2: Perform PCIe link training in x4 configuration\n");
    #if defined(DM0_RC) || defined(DM0_EP)
        link_training_dm0_x4(4);
    #endif
    #if defined(DM1_RC) || defined(DM1_EP)
        link_training_dm1_x4(4);
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 bits [11:14]=0xf, [3:6]=0xf */
    LOGI("Step 3: Cache programming PCIE0 bits [11:14]=0xf, [3:6]=0xf\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0 bits [27:30]=0xf, [19:22]=0xf */
    LOGI("Step 4: Cache programming PCIE0 bits [27:30]=0xf, [19:22]=0xf\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat for PCIE1 */
    LOGI("Step 5: Cache programming PCIE1 all bit fields\n");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: Wait */
    LOGI("Step 6: wait_on(20)\n");
    wait_on(20);

    /* Step 7: Re-apply cache coherency for both PCIE0 and PCIE1 */
    LOGI("Step 7: Re-apply cache coherency for PCIE0 and PCIE1\n");
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

    /* Step 8: Poll SII0 under DM0 */
    #ifdef DM0
        LOGI("Step 8: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
        do {
            data_rd = read_sii0_reg(0xC0);
        } while ((data_rd & 0xD1) != 0xD1);
        #ifdef DEBUG_DISPLAY
            LOGI("SII0 link ready confirmed, data_rd=0x%08x\n", data_rd);
        #endif
    #endif

    /* Step 9: Poll SII1 under DM1 */
    #ifdef DM1
        LOGI("Step 9: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
        do {
            data_rd = read_sii1_reg(0xC0);
        } while ((data_rd & 0xD1) != 0xD1);
        #ifdef DEBUG_DISPLAY
            LOGI("SII1 link ready confirmed, data_rd=0x%08x\n", data_rd);
        #endif
    #endif

    /* Step 10: Under DM0_EP - long wait */
    #ifdef DM0_EP
        LOGI("Step 10: DM0_EP - wait_on(30000)\n");
        wait_on(30000);
    #endif

    /* Step 11: Under DM0_RC */
    #ifdef DM0_RC
        LOGI("Step 11: DM0_RC - Read Vendor ID, enable mem/bus master, BAR program\n");
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("Vendor ID = 0x%08x\n", data_rd);
        write_pcie_slv0_reg(0x4, 0x7);
        bar_program_dm0_x4();
        wait_on(10);
        mem_base_program_dm0_x4();
    #endif

    /* Step 12: Under DM1_RC */
    #ifdef DM1_RC
        LOGI("Step 12: DM1_RC - Read Vendor ID, enable mem/bus master, BAR program\n");
        data_rd = read_pcie_slv1_reg(0x0);
        LOGI("Vendor ID = 0x%08x\n", data_rd);
        write_pcie_slv1_reg(0x4, 0x7);
        bar_program_dm1_x4();
        wait_on(10);
        mem_base_program_dm1_x4();
    #endif

    /* Step 13: Under DM0_EP */
    #ifdef DM0_EP
        LOGI("Step 13: DM0_EP - BAR program EP mode\n");
        bar_program_dm0_EP_x4();
        wait_on(10);
        mem_base_program_dm0_x4();
    #endif

    /* Step 14: Under DM1_EP */
    #ifdef DM1_EP
        LOGI("Step 14: DM1_EP - BAR program EP mode\n");
        bar_program_dm1_EP_x4();
        wait_on(10);
        mem_base_program_dm1_x4();
    #endif

    /* Step 15: Non-secure protection */
    LOGI("Step 15: Call non_secure_prot_nic()\n");
    non_secure_prot_nic();

    /* Step 16: Synchronization signal */
    LOGI("Step 16: Write synchronization signal 0x11111111 to 0xE6004100\n");
    write_reg(0xE6004100, 0x11111111);

    /* Step 17: DISABLE_CACHE PROGRAMMING - PCIE0 */
    LOGI("Step 17: Disable cache PCIE0 bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0x0\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 18: DISABLE_CACHE PROGRAMMING - PCIE1 */
    LOGI("Step 18: Disable cache PCIE1 bits [11:14]=0xf, [3:6]=0xf, [27:30]=0xf, [19:22]=0x0\n");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 19: Final cache disable */
    LOGI("Step 19: Final cache disable bits [27:30]=0x0, [19:22]=0x0\n");
    wait_on(10);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 20: Wait */
    LOGI("Step 20: wait_on(30)\n");
    wait_on(30);

    /* Step 21: Memory write-read under DM0_RC */
    #ifdef DM0_RC
        LOGI("Step 21: DM0_RC - Memory write-read verification (3 addresses)\n");
        pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
        pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
        pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
    #endif

    /* Step 22: Memory write-read under DM1_RC */
    #ifdef DM1_RC
        LOGI("Step 22: DM1_RC - Memory write-read verification (3 addresses)\n");
        pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
        pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
        pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
    #endif

    /* Step 23: Memory write-read under DM0_EP (Bar1) */
    #ifdef DM0_EP
        LOGI("Step 23: DM0_EP - Memory write-read verification (5 BAR1 addresses)\n");
        pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a);
        pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a);
    #endif

    /* Step 24: Memory write-read under DM1_EP (Bar1) */
    #ifdef DM1_EP
        LOGI("Step 24: DM1_EP - Memory write-read verification (5 BAR1 addresses)\n");
        pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a);
        pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a);
    #endif

    /* Step 25: Wait */
    LOGI("Step 25: wait_on(10)\n");
    wait_on(10);

    /* Step 26: Poll synchronization register */
    LOGI("Step 26: Poll 0xE6004100 until value == 0x12345678\n");
    do {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    } while (data_rd != 0x12345678);
    LOGI("Synchronization handshake received: 0x%08x\n", data_rd);

    /* Step 27: Test complete */
    LOGI("Step 27: Test complete, calling finish(0)\n");
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
    LOGI("[TEARDOWN] PCIE memory write-read test: %s\n", cfg->test_name);

    return 0;
}
