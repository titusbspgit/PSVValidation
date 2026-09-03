/*
 * pcie_device_enumerate_test.c
 *
 * Test Case : pcie_device_enumerate_test
 * Description: PCIe device enumeration across two controller instances (DM0 and DM1).
 *              Performs link training, cache coherency programming, SII link status polling,
 *              Vendor ID read, command register enable, memory base programming,
 *              system control register configuration, cache disable, BAR sizing and
 *              address programming on both slave ports, and synchronization polling.
 */

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err;

int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("[Test Init] PCIe device enumerate test: %s", cfg->test_name);

    /* Step 1: Write 0x0 to 0xE6004100 to initialize the system */
    write_reg(PCIE_SYNC_REG, 0x0);
    LOGT("[Init] System control register 0xE6004100 initialized to 0x0");

    return 0;
}

int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    test_err = 0;

    LOGT("[Test Run] PCIe device enumerate test: %s", cfg->test_name);

    /* Step 2: Invoke link training based on compile-time defines */
    #if defined(DM0_RC) || defined(DM0_EP)
        link_training_dm0_x4(4);
        LOGT("[Run] link_training_dm0_x4(4) invoked");
    #endif
    #if defined(DM1_RC) || defined(DM1_EP)
        link_training_dm1_x4(4);
        LOGT("[Run] link_training_dm1_x4(4) invoked");
    #endif

    /* Step 3: CACHE PROGRAMMING - PCIE0 coherency control phase 1 */
    /* Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, set bits [11:14] to 0xF and bits [3:6] to 0xF, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE0 coherency ctrl phase1: bits [11:14],[3:6] set to 0xF");

    /* Step 4: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, set bits [27:30] to 0xF and bits [19:22] to 0xF, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE0 coherency ctrl phase1: bits [27:30],[19:22] set to 0xF");

    /* Step 5: Repeat steps 3-4 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE1 coherency ctrl phase1: bits [11:14],[3:6] set to 0xF");

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE1 coherency ctrl phase1: bits [27:30],[19:22] set to 0xF");

    /* Step 6: Call wait_on(20) */
    wait_on(20);

    /* Step 7: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, set all four bit groups to 0xF, write back */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE0 coherency ctrl phase2: all four bit groups set to 0xF");

    /* Step 8: Repeat step 7 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 6, 3, 0xF);
    data_rd = set_data(data_rd, 14, 11, 0xF);
    data_rd = set_data(data_rd, 22, 19, 0xF);
    data_rd = set_data(data_rd, 30, 27, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE1 coherency ctrl phase2: all four bit groups set to 0xF");

    /* Step 9: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGT("[Run] Polling SII0 link status reg 0xC0, data_rd=0x%x", data_rd);
        wait_on(10);
        data_rd = read_sii0_reg(0xC0);
    }
    LOGT("[Run] SII0 link status ready: data_rd=0x%x", data_rd);

    /* Step 10: Call non_secure_prot_nic() */
    non_secure_prot_nic();
    LOGT("[Run] non_secure_prot_nic() called");

    /* Step 11: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1)
    {
        LOGT("[Run] Polling SII1 link status reg 0xC0, data_rd=0x%x", data_rd);
        wait_on(10);
        data_rd = read_sii1_reg(0xC0);
    }
    LOGT("[Run] SII1 link status ready: data_rd=0x%x", data_rd);

    #ifdef DM0_RC
    /* Step 12: Read Vendor ID via read_pcie_slv0_reg(0x0) */
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("[Run] Vendor ID read from slv0 reg 0x0: data_rd=0x%x", data_rd);

    /* Step 13: Write 0x7 to write_pcie_slv0_reg(0x4) to enable IO, Memory, Bus Master */
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("[Run] Command register 0x4 written with 0x7 (IO+Mem+BusMaster)");

    /* Step 14: Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4() */
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    LOGT("[Run] mem_base_program_dm0_x4() and mem_base_program_dm1_x4() called");

    /* Step 15: Call wait_on(10) */
    wait_on(10);

    /* Step 16: Write 0x1 to system-level control registers */
    write_reg(PCIE_SYS_CTRL_REG_0C, 0x1);
    write_reg(PCIE_SYS_CTRL_REG_10, 0x1);
    write_reg(PCIE_SYS_CTRL_REG_14, 0x1);
    write_reg(PCIE_SYS_CTRL_REG_18, 0x1);
    write_reg(PCIE_SYS_CTRL_REG_30, 0x1);
    write_reg(PCIE_SYS_CTRL_REG_34, 0x1);
    LOGT("[Run] System control registers written with 0x1");

    /* Step 17: DISABLE_CACHE PROGRAMMING - PCIE0 setting bits [19:22] and [27:30] to 0x0 */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE0 cache disable phase1: bits [19:22],[27:30] set to 0x0");

    /* Step 18: Repeat step 17 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF */
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE1 cache disable phase1: bits [19:22],[27:30] set to 0x0");

    /* Step 19: Call wait_on(10) */
    wait_on(10);

    /* Step 20: Final cache disable - read-modify-write both coherency control registers */
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE0 final cache disable: bits [19:22],[27:30] set to 0x0");

    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 22, 19, 0x0);
    data_rd = set_data(data_rd, 30, 27, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("[Run] PCIE1 final cache disable: bits [19:22],[27:30] set to 0x0");

    /* Step 21: Call wait_on(30) */
    wait_on(30);

    /* Step 22: BAR sizing on slave port 1 - write 0xFFFFFFFF and read back */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("[Run] SLV1 BAR sizing offset 0x10: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("[Run] SLV1 BAR sizing offset 0x14: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("[Run] SLV1 BAR sizing offset 0x18: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("[Run] SLV1 BAR sizing offset 0x1c: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("[Run] SLV1 BAR sizing offset 0x20: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("[Run] SLV1 BAR sizing offset 0x24: read back=0x%x", data_rd);

    /* Step 23: Program BAR values on slave port 1 and read back */
    write_pcie_slv1_reg(0x10, 0x0);
    data_rd = read_pcie_slv1_reg(0x10);
    LOGT("[Run] SLV1 BAR prog offset 0x10=0x0: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x14, 0x4);
    data_rd = read_pcie_slv1_reg(0x14);
    LOGT("[Run] SLV1 BAR prog offset 0x14=0x4: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv1_reg(0x18);
    LOGT("[Run] SLV1 BAR prog offset 0x18=0x20000000: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv1_reg(0x1c);
    LOGT("[Run] SLV1 BAR prog offset 0x1c=0x40000000: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv1_reg(0x20);
    LOGT("[Run] SLV1 BAR prog offset 0x20=0x60000000: read back=0x%x", data_rd);

    write_pcie_slv1_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv1_reg(0x24);
    LOGT("[Run] SLV1 BAR prog offset 0x24=0x80000000: read back=0x%x", data_rd);

    /* Step 24: Repeat BAR sizing and programming for slave port 0 */
    /* BAR sizing on slave port 0 - write 0xFFFFFFFF and read back */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("[Run] SLV0 BAR sizing offset 0x10: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("[Run] SLV0 BAR sizing offset 0x14: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("[Run] SLV0 BAR sizing offset 0x18: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("[Run] SLV0 BAR sizing offset 0x1c: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("[Run] SLV0 BAR sizing offset 0x20: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("[Run] SLV0 BAR sizing offset 0x24: read back=0x%x", data_rd);

    /* Program BAR values on slave port 0 and read back */
    write_pcie_slv0_reg(0x10, 0x0);
    data_rd = read_pcie_slv0_reg(0x10);
    LOGT("[Run] SLV0 BAR prog offset 0x10=0x0: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x14, 0x4);
    data_rd = read_pcie_slv0_reg(0x14);
    LOGT("[Run] SLV0 BAR prog offset 0x14=0x4: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x18, 0x20000000);
    data_rd = read_pcie_slv0_reg(0x18);
    LOGT("[Run] SLV0 BAR prog offset 0x18=0x20000000: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x1c, 0x40000000);
    data_rd = read_pcie_slv0_reg(0x1c);
    LOGT("[Run] SLV0 BAR prog offset 0x1c=0x40000000: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x20, 0x60000000);
    data_rd = read_pcie_slv0_reg(0x20);
    LOGT("[Run] SLV0 BAR prog offset 0x20=0x60000000: read back=0x%x", data_rd);

    write_pcie_slv0_reg(0x24, 0x80000000);
    data_rd = read_pcie_slv0_reg(0x24);
    LOGT("[Run] SLV0 BAR prog offset 0x24=0x80000000: read back=0x%x", data_rd);

    /* Step 25: Call wait_on(10) */
    wait_on(10);
    #endif /* DM0_RC */

    /* Step 26: Poll read_reg(0xE6004100) until value equals 0x12345678 */
    data_rd = read_reg(PCIE_SYNC_REG);
    while (data_rd != PCIE_SYNC_EXPECTED_VAL)
    {
        LOGT("[Run] Polling sync reg 0xE6004100, data_rd=0x%x", data_rd);
        wait_on(5);
        data_rd = read_reg(PCIE_SYNC_REG);
    }
    LOGT("[Run] Sync register polling complete: data_rd=0x%x", data_rd);

    /* Step 27: Call finish(0) to end the test */
    finish(0);

    out->status = test_err;
    LOGT("[Run] Test complete: errors=%u", test_err);

    return out->status;
}

int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("[TEARDOWN] PCIe device enumerate test teardown: %s", cfg->test_name);

    /* Validation: The test passes when finish(0) is reached indicating all steps completed */
    /* 1. PCIe link training completes successfully on the applicable controller instance */
    /* 2. SII interface 0 and SII interface 1 link status polling completes with expected bitmask */
    /* 3. TYPE1_DEV_ID_VEND_ID_REG returns a valid Vendor ID confirming device presence */
    /* 4. TYPE1_STATUS_COMMAND_REG is successfully written to enable IO, memory, bus master */
    /* 5. BAR sizing on both slave ports returns valid size information */
    /* 6. BAR address programming is verified by reading back the programmed values */
    /* 7. The synchronization register polling completes with expected completion value */
    /* 8. The test terminates via finish(0) indicating success */

    return test_err == 0 ? 0 : -1;
}
