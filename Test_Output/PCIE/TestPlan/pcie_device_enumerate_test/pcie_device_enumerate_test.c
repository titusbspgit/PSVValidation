/*
 * pcie_device_enumerate_test.c
 *
 * Test Case : pcie_device_enumerate_test
 * Description: The testcase performs PCIe device enumeration across two
 *              controller instances (DM0 and DM1). It initializes the system,
 *              performs link training, configures cache coherency, polls SII
 *              link status, reads Vendor ID, enables IO/Memory/Bus Master,
 *              programs memory base addresses, configures system-level control
 *              registers, disables cache, performs BAR sizing and programming
 *              on both slave ports, and polls a synchronization register.
 */

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err;

int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("[Test Init] PCIe device enumerate test: %s", cfg->test_name);

    return 0;
}

int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    test_err = 0;

    LOGT("[Test Run] PCIe device enumerate test: %s", cfg->test_name);

    /* Step 1: Write 0x0 to 0xE6004100 to initialize the system */
    LOGT("Step 1: Initialize system - write 0x0 to 0xE6004100");
    write_reg(0xE6004100, 0x0);

    /* Step 2: Invoke link training based on compile-time defines */
    LOGT("Step 2: Invoke link training");
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

    /* Step 3: CACHE PROGRAMMING - PCIE0 coherency control phase 1 */
    LOGT("Step 3: Cache programming PCIE0 - set bits [11:14] and [3:6] to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0 coherency control phase 2 - set bits [27:30] and [19:22] to 0xF */
    LOGT("Step 4: Cache programming PCIE0 - set bits [27:30] and [19:22] to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 coherency control */
    LOGT("Step 5: Cache programming PCIE1 - set bits [11:14] and [3:6] to 0xF");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 5: Cache programming PCIE1 - set bits [27:30] and [19:22] to 0xF");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 6: Wait for coherency configuration to take effect */
    LOGT("Step 6: wait_on(20)");
    wait_on(20);

    /* Step 7: Read PCIE0 coherency control, set all four bit groups to 0xF, write back */
    LOGT("Step 7: Cache programming PCIE0 - set all four bit groups to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Repeat step 7 for PCIE1 coherency control */
    LOGT("Step 8: Cache programming PCIE1 - set all four bit groups to 0xF");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 9: Poll SII0 link status until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 9: Polling SII0 link status register 0xC0");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGT("Waiting for SII0 link up, data_rd=0x%x", data_rd);
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGT("SII0 link status ready: data_rd=0x%x", data_rd);

    /* Step 10: Configure non-secure protection settings */
    LOGT("Step 10: Call non_secure_prot_nic()");
    non_secure_prot_nic();

    /* Step 11: Poll SII1 link status until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 11: Polling SII1 link status register 0xC0");
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGT("Waiting for SII1 link up, data_rd=0x%x", data_rd);
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGT("SII1 link status ready: data_rd=0x%x", data_rd);

    #ifdef DM0_RC
    /* Step 12: Read Vendor ID from PCIe slave port 0 */
    LOGT("Step 12: Read Vendor ID from read_pcie_slv0_reg(0x0)");
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Vendor ID read: 0x%x", data_rd);

    /* Step 13: Write 0x7 to command register to enable IO, Memory, Bus Master */
    LOGT("Step 13: Write 0x7 to write_pcie_slv0_reg(0x4) - enable IO/Mem/BusMaster");
    write_pcie_slv0_reg(0x4, 0x7);

    /* Step 14: Program memory base addresses */
    LOGT("Step 14: Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4()");
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();

    /* Step 15: Wait */
    LOGT("Step 15: wait_on(10)");
    wait_on(10);

    /* Step 16: Write 0x1 to system-level control registers */
    LOGT("Step 16: Write 0x1 to system-level control registers");
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);

    /* Step 17: DISABLE_CACHE PROGRAMMING - PCIE0 set bits [19:22] and [27:30] to 0x0 */
    LOGT("Step 17: Disable cache PCIE0 - set bits [19:22] and [27:30] to 0x0");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 18: Repeat step 17 for PCIE1 */
    LOGT("Step 18: Disable cache PCIE1 - set bits [19:22] and [27:30] to 0x0");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 19: Wait */
    LOGT("Step 19: wait_on(10)");
    wait_on(10);

    /* Step 20: Final cache disable - both coherency control registers */
    LOGT("Step 20: Final cache disable PCIE0");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 20: Final cache disable PCIE1");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 21: Wait */
    LOGT("Step 21: wait_on(30)");
    wait_on(30);

    /* Step 22: BAR sizing on slave port 1 - write 0xFFFFFFFF and read back */
    LOGT("Step 22: BAR sizing on slave port 1");
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);

    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("SLV1 BAR sizing offset 0x10: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("SLV1 BAR sizing offset 0x14: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("SLV1 BAR sizing offset 0x18: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("SLV1 BAR sizing offset 0x1c: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("SLV1 BAR sizing offset 0x20: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("SLV1 BAR sizing offset 0x24: 0x%x", data_rd);

    /* Step 23: Program BAR values on slave port 1 and read back */
    LOGT("Step 23: Program BAR values on slave port 1");
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);

    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("SLV1 BAR program offset 0x10: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("SLV1 BAR program offset 0x14: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("SLV1 BAR program offset 0x18: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("SLV1 BAR program offset 0x1c: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("SLV1 BAR program offset 0x20: 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("SLV1 BAR program offset 0x24: 0x%x", data_rd);

    /* Step 24: Repeat BAR sizing and programming for slave port 0 */
    LOGT("Step 24: BAR sizing on slave port 0");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("SLV0 BAR sizing offset 0x10: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("SLV0 BAR sizing offset 0x14: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("SLV0 BAR sizing offset 0x18: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("SLV0 BAR sizing offset 0x1c: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("SLV0 BAR sizing offset 0x20: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("SLV0 BAR sizing offset 0x24: 0x%x", data_rd);

    LOGT("Step 24: Program BAR values on slave port 0");
    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("SLV0 BAR program offset 0x10: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("SLV0 BAR program offset 0x14: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("SLV0 BAR program offset 0x18: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("SLV0 BAR program offset 0x1c: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("SLV0 BAR program offset 0x20: 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("SLV0 BAR program offset 0x24: 0x%x", data_rd);

    /* Step 25: Wait */
    LOGT("Step 25: wait_on(10)");
    wait_on(10);

    /* Step 26: Poll 0xE6004100 until value equals 0x12345678 */
    LOGT("Step 26: Polling 0xE6004100 for synchronization value 0x12345678");
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        LOGT("Waiting for sync, data_rd=0x%x", data_rd);
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGT("Synchronization complete: data_rd=0x%x", data_rd);

    /* Step 27: Call finish(0) to end the test */
    LOGT("Step 27: Test complete - calling finish(0)");
    finish(0);
    #endif /* DM0_RC */

    return out->status = test_err;
}

int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("[TEARDOWN] PCIe device enumerate test: %s", cfg->test_name);

    /*
     * Validation / Acceptance Criteria:
     * 1. PCIe link training completes successfully on the applicable controller instance.
     * 2. SII interface 0 and SII interface 1 link status polling completes with the
     *    expected link-up bitmask condition satisfied.
     * 3. TYPE1_DEV_ID_VEND_ID_REG returns a valid Vendor ID confirming device presence.
     * 4. TYPE1_STATUS_COMMAND_REG is successfully written to enable IO space, memory
     *    space, and bus master.
     * 5. BAR sizing on both slave ports returns valid size information when all-ones
     *    are written to BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG,
     *    SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and
     *    PREF_MEM_LIMIT_PREF_MEM_BASE_REG.
     * 6. BAR address programming is verified by reading back the programmed values.
     * 7. The synchronization register polling completes with the expected completion value.
     * 8. The test terminates via finish(0) indicating success.
     */

    return 0;
}
