// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/*============================================================================
 * Test Case  : pcie_device_enumerate_test
 * Description: PCIe device enumeration test. Performs link training, cache
 *              coherency programming, link-up polling, Vendor ID read, bus
 *              master enable, memory base programming, system register config,
 *              cache disable, BAR enumeration and programming on both slave
 *              ports, and final synchronization polling.
 *============================================================================*/

volatile unsigned int data_rd = 0;
unsigned int test_err;

/*
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe Device Enumerate test: %s\n", cfg->test_name);
    LOGT("pcie_device_enumerate_test init");

    return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for pcie_device_enumerate_test.
 *              Performs link training, cache programming, link-up polling,
 *              Vendor ID read, bus master enable, memory base programming,
 *              system register configuration, cache disable, BAR enumeration
 *              and programming, and synchronization polling.
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

    /* Step 1: Initialize synchronization register by clearing it */
    LOGT("Step 1: Write 0x0 to 0xE6004100 to initialize sync register");
    write_reg(0xE6004100, 0x0);

    /* Step 2: Conditionally perform PCIe link training */
    LOGT("Step 2: Perform PCIe link training");
    #ifdef DM0_RC
        link_training_dm0_x4(4);
        LOGT("link_training_dm0_x4(4) called (DM0 RC mode)");
    #endif
    #ifdef DM1_RC
        link_training_dm1_x4(4);
        LOGT("link_training_dm1_x4(4) called (DM1 RC mode)");
    #endif
    #ifdef DM0_EP
        link_training_dm0_x4(4);
        LOGT("link_training_dm0_x4(4) called (DM0 EP mode)");
    #endif
    #ifdef DM1_EP
        link_training_dm1_x4(4);
        LOGT("link_training_dm1_x4(4) called (DM1 EP mode)");
    #endif

    /* Step 3: CACHE PROGRAMMING - Enable cache coherency for PCIE0 */
    LOGT("Step 3: Program COHERENCY_CONTROL_3_OFF for PCIE0 bits [11:14],[3:6]");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0 bits [27:30],[19:22] */
    LOGT("Step 4: Program COHERENCY_CONTROL_3_OFF for PCIE0 bits [27:30],[19:22]");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
    LOGT("Step 5: Program COHERENCY_CONTROL_3_OFF for PCIE1 bits [11:14],[3:6]");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 5: Program COHERENCY_CONTROL_3_OFF for PCIE1 bits [27:30],[19:22]");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: wait_on(20) */
    LOGT("Step 6: wait_on(20)");
    wait_on(20);

    /* Step 7: Re-apply cache coherency for PCIE0 all bits */
    LOGT("Step 7: Re-apply COHERENCY_CONTROL_3_OFF for PCIE0 all bits");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Re-apply cache coherency for PCIE1 all bits */
    LOGT("Step 8: Re-apply COHERENCY_CONTROL_3_OFF for PCIE1 all bits");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 9: Repeat link training and cache programming block (duplicate) */
    LOGT("Step 9: Duplicate link training and cache programming block");
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
    LOGT("Step 9: Duplicate block completed");

    /* Step 10: Read SII0 link status and call non_secure_prot_nic() */
    LOGT("Step 10: Read SII0 link status, call non_secure_prot_nic()");
    data_rd = read_sii0_reg(0xC0);
    non_secure_prot_nic();

    /* Step 11: Poll SII0 for link-up */
    LOGT("Step 11: Poll SII0 link status until (data_rd & 0xD1) == 0xD1");
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
    }
    LOGT("SII0 link-up confirmed");

    /* Step 12: Poll SII1 for link-up */
    LOGT("Step 12: Poll SII1 link status until (data_rd & 0xD1) == 0xD1");
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
    }
    LOGT("SII1 link-up confirmed");

    /* Step 13: Under DM0_RC - Read Vendor ID, enable bus master, program mem base */
    #ifdef DM0_RC
        LOGT("Step 13: Read Vendor ID from slv0 offset 0x0");
        data_rd = read_pcie_slv0_reg(0x0);
        #ifdef DEBUG_DISPLAY
            LOGI("Vendor ID read from slv0: 0x%08X\n", data_rd);
        #endif

        LOGT("Step 13: Write 0x7 to slv0 offset 0x4 (bus master + mem space)");
        write_pcie_slv0_reg(0x4, 0x7);

        LOGT("Step 13: Program memory base addresses for DM0 and DM1");
        mem_base_program_dm0_x4();
        mem_base_program_dm1_x4();
        wait_on(10);

        /* Step 14: Enable system-level configuration registers */
        LOGT("Step 14: Write 0x1 to system registers 0xE690000C-0xE6900034");
        write_reg(0xE690000C, 0x1);
        write_reg(0xE6900010, 0x1);
        write_reg(0xE6900014, 0x1);
        write_reg(0xE6900018, 0x1);
        write_reg(0xE6900030, 0x1);
        write_reg(0xE6900034, 0x1);
    #endif

    /* Step 15: DISABLE CACHE PROGRAMMING - PCIE0 */
    LOGT("Step 15: Disable cache - PCIE0 bits [11:14],[3:6] write back");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 15: Disable cache - PCIE0 bits [27:30]=0xf,[19:22]=0x0");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 15: DISABLE CACHE PROGRAMMING - PCIE1 */
    LOGT("Step 15: Disable cache - PCIE1 bits [11:14],[3:6] write back");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 15: Disable cache - PCIE1 bits [27:30]=0xf,[19:22]=0x0");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 16: wait_on(10), then clear remaining coherency fields */
    LOGT("Step 16: wait_on(10) then clear remaining coherency fields");
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
    LOGT("Cache coherency fully disabled for PCIE0 and PCIE1");

    /* Step 17: wait_on(30) */
    LOGT("Step 17: wait_on(30)");
    wait_on(30);

    /* Step 18: BAR enumeration on slave port 1 (write 0xFFFFFFFF, read back) */
    LOGT("Step 18: BAR enumeration on slv1 - write 0xFFFFFFFF to offsets 0x10-0x24");
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x10 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x14 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x18 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x1c sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x20 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x24 sizing readback = 0x%08X\n", data_rd);
    #endif

    /* Step 19: BAR programming on slave port 1 with specific base addresses */
    LOGT("Step 19: Program BAR registers on slv1 with base addresses");
    write_pcie_slv1_reg(0x10, 0x00000000);
    data_rd = read_pcie_slv1_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x10 programmed=0x00000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x14, 0x00000004);
    data_rd = read_pcie_slv1_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x14 programmed=0x00000004 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x18 programmed=0x20000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x1c programmed=0x40000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x20 programmed=0x60000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("slv1 offset 0x24 programmed=0x80000000 readback=0x%08X\n", data_rd);
    #endif

    /* Step 20: BAR enumeration on slave port 0 (write 0xFFFFFFFF, read back) */
    LOGT("Step 20: BAR enumeration on slv0 - write 0xFFFFFFFF to offsets 0x10-0x24");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x10 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x14 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x18 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x1c sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x20 sizing readback = 0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x24 sizing readback = 0x%08X\n", data_rd);
    #endif

    /* Step 21: BAR programming on slave port 0 with specific base addresses */
    LOGT("Step 21: Program BAR registers on slv0 with base addresses");
    write_pcie_slv0_reg(0x10, 0x00000000);
    data_rd = read_pcie_slv0_reg(0x10);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x10 programmed=0x00000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x14, 0x00000004);
    data_rd = read_pcie_slv0_reg(0x14);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x14 programmed=0x00000004 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x18 programmed=0x20000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x1c programmed=0x40000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x20 programmed=0x60000000 readback=0x%08X\n", data_rd);
    #endif

    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);
    #ifdef DEBUG_DISPLAY
        LOGI("slv0 offset 0x24 programmed=0x80000000 readback=0x%08X\n", data_rd);
    #endif

    /* Step 22: wait_on(10) */
    LOGT("Step 22: wait_on(10)");
    wait_on(10);

    /* Step 23: Poll synchronization register for completion */
    LOGT("Step 23: Poll 0xE6004100 until value equals 0x12345678");
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGT("Synchronization register = 0x12345678, enumeration complete");

    /* Step 24: Call finish(0) */
    LOGT("Step 24: Enumeration successful, calling finish(0)");
    finish(0);

    return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation, cleanup, and final observation for
 *              pcie_device_enumerate_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe Device Enumerate teardown: %s\n", cfg->test_name);
    LOGT("pcie_device_enumerate_test teardown: no additional cleanup required");

    return 0;
}
