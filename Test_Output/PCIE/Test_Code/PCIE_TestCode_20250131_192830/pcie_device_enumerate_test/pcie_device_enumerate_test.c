/*
 * pcie_device_enumerate_test.c
 *
 * Test Case: pcie_device_enumerate_test
 *
 * Description:
 *   Performs PCIe device enumeration across two controller instances (DM0 and DM1).
 *   Initializes the system, invokes link training, programs cache coherency,
 *   polls SII link status on both interfaces, reads Vendor ID, enables IO/Memory/Bus Master,
 *   programs memory base addresses, configures system-level control registers,
 *   disables cache coherency, performs BAR sizing and address programming on both
 *   PCIe slave ports, and polls a synchronization register for completion.
 */

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

/* Test context for error tracking */
static unsigned int test_err;
static unsigned int data_rd;

/*
 * pcie_device_enumerate_test_init
 *   Initial setup for PCIe device enumeration test.
 */
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;

    test_err = 0;
    data_rd = 0;

    LOGT("[Test Init] PCIe device enumerate test");

    return 0;
}

/*
 * pcie_device_enumerate_test_run
 *   Main testcase execution: link training, cache programming, SII polling,
 *   enumeration, BAR sizing/programming, and synchronization polling.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    LOGT("[Test Run] PCIe device enumerate test starting");

    /* Step 1: Write 0x0 to 0xE6004100 to initialize the system */
    LOGT("Step 1: System initialization - write 0x0 to 0xE6004100");
    write_reg(0xE6004100, 0x0);

    /* Step 2: Invoke link training based on compile-time defines */
    LOGT("Step 2: PCIe link training");
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 first RMW: set bits [11:14] to 0xF and bits [3:6] to 0xF */
    LOGT("Step 3: Cache programming PCIE0 - set bits [11:14] and [3:6] to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 4: PCIE0 second RMW: set bits [27:30] to 0xF and bits [19:22] to 0xF */
    LOGT("Step 4: Cache programming PCIE0 - set bits [27:30] and [19:22] to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 5: Repeat steps 3-4 for PCIE1 */
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
    LOGT("Step 6: wait_on(20) for coherency config");
    wait_on(20);

    /* Step 7: PCIE0 - set all four bit groups [3:6], [11:14], [19:22], [27:30] to 0xF */
    LOGT("Step 7: Cache programming PCIE0 - set all four bit groups to 0xF");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 8: Repeat step 7 for PCIE1 */
    LOGT("Step 8: Cache programming PCIE1 - set all four bit groups to 0xF");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 9: Poll SII interface 0 link status until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 9: Polling SII0 link status register 0xC0");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii0_reg(0xC0);
    }
    LOGT("Step 9: SII0 link status ready, data_rd=0x%x", data_rd);

    /* Step 10: Configure non-secure protection settings */
    LOGT("Step 10: Calling non_secure_prot_nic()");
    non_secure_prot_nic();

    /* Step 11: Poll SII interface 1 link status until (data_rd & 0xD1) == 0xD1 */
    LOGT("Step 11: Polling SII1 link status register 0xC0");
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        data_rd = read_sii1_reg(0xC0);
    }
    LOGT("Step 11: SII1 link status ready, data_rd=0x%x", data_rd);

#ifdef DM0_RC
    /* Step 12: Read Vendor ID from TYPE1_DEV_ID_VEND_ID_REG on slave port 0 */
    LOGT("Step 12: Reading Vendor ID from pcie_slv0 offset 0x0");
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("Step 12: Vendor ID read = 0x%x", data_rd);

    /* Step 13: Write 0x7 to TYPE1_STATUS_COMMAND_REG to enable IO, Memory, Bus Master */
    LOGT("Step 13: Enabling IO/Memory/Bus Master on pcie_slv0 offset 0x4");
    write_pcie_slv0_reg(0x4, 0x7);
#endif

    /* Step 14: Program memory base addresses for both controller instances */
    LOGT("Step 14: Memory base programming for DM0 and DM1");
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();

    /* Step 15: Wait for memory base programming to take effect */
    LOGT("Step 15: wait_on(10)");
    wait_on(10);

    /* Step 16: Write 0x1 to system-level control registers */
    LOGT("Step 16: Writing 0x1 to system-level control registers");
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);

    /* Step 17: DISABLE_CACHE PROGRAMMING - PCIE0: clear bits [19:22] and [27:30] to 0x0 */
    LOGT("Step 17: Disable cache PCIE0 - clear bits [19:22] and [27:30]");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 18: Repeat step 17 for PCIE1 */
    LOGT("Step 18: Disable cache PCIE1 - clear bits [19:22] and [27:30]");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 19: Wait for cache disable to take effect */
    LOGT("Step 19: wait_on(10)");
    wait_on(10);

    /* Step 20: Final cache disable - RMW both coherency control registers */
    LOGT("Step 20: Final cache disable PCIE0 - clear bits [19:22] and [27:30]");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    LOGT("Step 20: Final cache disable PCIE1 - clear bits [19:22] and [27:30]");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

    /* Step 21: Wait after final cache disable */
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
    LOGT("SLV1 BAR0 sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("SLV1 BAR1 sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("SLV1 SEC_LAT_TIMER sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("SLV1 SEC_STAT_IO sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("SLV1 MEM_LIMIT sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("SLV1 PREF_MEM sizing readback = 0x%x", data_rd);

    /* Step 23: Program BAR values on slave port 1 and read back */
    LOGT("Step 23: Program BAR values on slave port 1");
    write_pcie_slv1_reg(0x10, 0x0);
    write_pcie_slv1_reg(0x14, 0x4);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1c, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);

    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("SLV1 BAR0 programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("SLV1 BAR1 programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("SLV1 SEC_LAT_TIMER programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("SLV1 SEC_STAT_IO programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("SLV1 MEM_LIMIT programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("SLV1 PREF_MEM programmed readback = 0x%x", data_rd);

    /* Step 24: Repeat BAR sizing and programming on slave port 0 */
    LOGT("Step 24: BAR sizing on slave port 0");
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("SLV0 BAR0 sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("SLV0 BAR1 sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("SLV0 SEC_LAT_TIMER sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("SLV0 SEC_STAT_IO sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("SLV0 MEM_LIMIT sizing readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("SLV0 PREF_MEM sizing readback = 0x%x", data_rd);

    LOGT("Step 24: Program BAR values on slave port 0");
    write_pcie_slv0_reg(0x10, 0x0);
    write_pcie_slv0_reg(0x14, 0x4);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1c, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);

    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("SLV0 BAR0 programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("SLV0 BAR1 programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("SLV0 SEC_LAT_TIMER programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("SLV0 SEC_STAT_IO programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("SLV0 MEM_LIMIT programmed readback = 0x%x", data_rd);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("SLV0 PREF_MEM programmed readback = 0x%x", data_rd);

    /* Step 25: Wait after BAR programming */
    LOGT("Step 25: wait_on(10)");
    wait_on(10);

    /* Step 26: Poll synchronization register until value equals 0x12345678 */
    LOGT("Step 26: Polling 0xE6004100 for sync completion value 0x12345678");
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678)
    {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }
    LOGT("Step 26: Sync register polling complete, data_rd=0x%x", data_rd);

    /* Step 27: End the test successfully */
    LOGT("Step 27: Test complete - calling finish(0)");
    finish(0);

    return test_err;
}

/*
 * pcie_device_enumerate_test_teardown
 *   Teardown and final status reporting.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("[TEARDOWN] PCIe device enumerate test teardown");

    if (test_err != 0)
    {
        LOGE("PCIe device enumerate test completed with %u errors", test_err);
    }
    else
    {
        LOGT("PCIe device enumerate test completed successfully");
    }

    return (test_err == 0) ? 0 : -1;
}
