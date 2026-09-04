// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/* Global variables */
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
    LOGI("[Test Init] PCIE device enumerate test: %s\n", cfg->test_name);

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
    LOGI("[Test Run] PCIE device enumerate test: %s\n", cfg->test_name);
    test_err = 0;

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

    /* Step 3: CACHE PROGRAMMING - PCIE0 coherency control bits [11:14]=0xf, [3:6]=0xf */
    LOGI("Step 3: Cache programming PCIE0 bits [11:14]=0xf, [3:6]=0xf\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0 coherency control bits [27:30]=0xf, [19:22]=0xf */
    LOGI("Step 4: Cache programming PCIE0 bits [27:30]=0xf, [19:22]=0xf\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    LOGI("Step 5: Cache programming PCIE1 all bit fields\n");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: Wait for coherency settings to take effect */
    LOGI("Step 6: wait_on(20)\n");
    wait_on(20);

    /* Step 7: Re-apply cache coherency programming for both PCIE0 and PCIE1 */
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

    /* Step 8: Poll SII0 for link readiness */
    LOGI("Step 8: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
    do {
        data_rd = read_sii0_reg(0xC0);
    } while ((data_rd & 0xD1) != 0xD1);
    #ifdef DEBUG_DISPLAY
        LOGI("SII0 link ready confirmed, data_rd=0x%08x\n", data_rd);
    #endif

    /* Step 9: Configure non-secure protection */
    LOGI("Step 9: Call non_secure_prot_nic()\n");
    non_secure_prot_nic();

    /* Step 10: Poll SII1 for link readiness */
    LOGI("Step 10: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1\n");
    do {
        data_rd = read_sii1_reg(0xC0);
    } while ((data_rd & 0xD1) != 0xD1);
    #ifdef DEBUG_DISPLAY
        LOGI("SII1 link ready confirmed, data_rd=0x%08x\n", data_rd);
    #endif

    /* Step 11: Under DM0_RC - Read Vendor ID, enable memory/bus master, program memory base */
    #ifdef DM0_RC
        LOGI("Step 11: DM0_RC - Read Vendor ID, enable mem/bus master, program mem base\n");
        data_rd = read_pcie_slv0_reg(0x0);
        LOGI("Vendor ID = 0x%08x\n", data_rd);

        write_pcie_slv0_reg(0x4, 0x7);

        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        wait_on(10);
    #endif

    /* Step 12: Write 0x1 to system-level control registers */
    LOGI("Step 12: Enable system-level control registers\n");
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);

    /* Step 13: DISABLE_CACHE PROGRAMMING - PCIE0 */
    LOGI("Step 13: Disable cache programming PCIE0\n");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 14: DISABLE_CACHE PROGRAMMING - PCIE1 */
    LOGI("Step 14: Disable cache programming PCIE1\n");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 15: Combined cache disable for both PCIE0 and PCIE1 */
    LOGI("Step 15: Combined cache disable bits [27:30]=0x0, [19:22]=0x0\n");
    wait_on(10);

    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 16: Wait for cache disable to take effect */
    LOGI("Step 16: wait_on(30)\n");
    wait_on(30);

    /* Step 17: BAR enumeration - Write 0xFFFFFFFF to slave port 1 offsets 0x10-0x24 */
    LOGI("Step 17: Write 0xFFFFFFFF to slave port 1 BAR offsets\n");
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);

    /* Step 18: Read back from slave port 1 */
    LOGI("Step 18: Read back slave port 1 BAR offsets\n");
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x10 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x14 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x18 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x1c = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x20 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x24 = 0x%08x\n", data_rd);
    #endif

    /* Step 19: Write specific base addresses to slave port 1 */
    LOGI("Step 19: Write specific base addresses to slave port 1\n");
    write_pcie_slv1_reg(0x10, 0x00000000);
    write_pcie_slv1_reg(0x14, 0x00000004);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);

    /* Step 20: Read back from slave port 1 */
    LOGI("Step 20: Read back slave port 1 programmed addresses\n");
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x10 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x14 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x18 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x1c = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x20 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV1 offset 0x24 = 0x%08x\n", data_rd);
    #endif

    /* Step 21: Repeat BAR enumeration on slave port 0 */
    LOGI("Step 21: BAR enumeration on slave port 0\n");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);

    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x10 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x14 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x18 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x1c = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x20 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x24 = 0x%08x\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x10, 0x00000000);
    write_pcie_slv0_reg(0x14, 0x00000004);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);

    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x10 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x14 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x18 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x1c = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x20 = 0x%08x\n", data_rd);
    #endif
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("SLV0 offset 0x24 = 0x%08x\n", data_rd);
    #endif

    /* Step 22: Wait */
    LOGI("Step 22: wait_on(10)\n");
    wait_on(10);

    /* Step 23: Poll synchronization register for completion handshake */
    LOGI("Step 23: Poll 0xE6004100 until value == 0x12345678\n");
    do {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    } while (data_rd != 0x12345678);
    LOGI("Synchronization handshake received: 0x%08x\n", data_rd);

    /* Step 24: Test complete */
    LOGI("Step 24: Test complete, calling finish(0)\n");
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
    LOGI("[TEARDOWN] PCIE device enumerate test: %s\n", cfg->test_name);

    return 0;
}
