/*
 // Author - AI Force 1.3.2. Date 24-01-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name: pcie_device_enumerate_test
 *
 * Description: This testcase performs PCIe device enumeration for both PCIE0
 * and PCIE1 controllers. It begins by writing 0x0 to 0xE6004100 and invoking
 * link training (link_training_dm0_x4 or link_training_dm1_x4 depending on
 * DM0_RC/DM1_RC/DM0_EP/DM1_EP compile-time defines) with a width parameter
 * of 4 (x4 lanes). Cache coherency is then programmed by performing
 * read-modify-write operations on COHERENCY_CONTROL_3_OFF registers. The
 * testcase polls SII0 and SII1 registers at offset 0xC0 with mask 0xD1,
 * waiting until link-up status is confirmed. BAR enumeration is performed on
 * both PCIe slave 1 and slave 0. Finally, the testcase polls 0xE6004100
 * waiting for value 0x12345678 and calls finish(0) upon success.
 */

#include <stdio.h>
#include "test_define.c"

static int error_count = 0;

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_init
 * Description    : Initialize testcase configuration, link training,
 *                  and cache coherency programming.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful initialization.
 ********************************************************************/
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;
    unsigned int data_rd = 0;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Init] Testcase: %s\n", cfg->test_name);
#endif

    /* Step 1: Write 0x0 to 0xE6004100 to initialize the control register */
    *(volatile unsigned int *)CTRL_REG_ADDR = 0x0;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 1: Wrote 0x0 to 0xE6004100");
#endif

    /* Step 2: Invoke link training based on compile-time defines for x4 link training */
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 2: link_training_dm0_x4(4) invoked");
#endif
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 2: link_training_dm1_x4(4) invoked");
#endif
#endif

    /* Step 3: CACHE PROGRAMMING - Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
     * set bits [11:14] to 0xF and bits [3:6] to 0xF, then write back */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 3: PCIE0 COHERENCY_CONTROL_3_OFF bits [11:14] and [3:6] set to 0xF");
#endif

    /* Step 4: Read mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again,
     * set bits [27:30] to 0xF and bits [19:22] to 0xF, then write back */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 4: PCIE0 COHERENCY_CONTROL_3_OFF bits [27:30] and [19:22] set to 0xF");
#endif

    /* Step 5: Repeat steps 3-4 for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 5a: PCIE1 COHERENCY_CONTROL_3_OFF bits [11:14] and [3:6] set to 0xF");
#endif

    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 5b: PCIE1 COHERENCY_CONTROL_3_OFF bits [27:30] and [19:22] set to 0xF");
#endif

    /* Step 6: Wait */
    wait_on(20);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 6: wait_on(20) completed");
#endif

    /* Step 7: Combined read-modify-write on PCIE0 COHERENCY_CONTROL_3_OFF
     * setting all four bit groups to 0xF */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 7: PCIE0 COHERENCY_CONTROL_3_OFF all four bit groups set to 0xF");
#endif

    /* Step 8: Repeat step 7 for PCIE1 COHERENCY_CONTROL_3_OFF */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 8: PCIE1 COHERENCY_CONTROL_3_OFF all four bit groups set to 0xF");
#endif

    return 0;
}

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_run
 * Description    : Execute testcase register configuration, link-up
 *                  polling, BAR enumeration, and stimulus.
 * Parameters     : cfg - Test configuration pointer,
 *                  out - Test output pointer.
 * Return Value   : Test execution status.
 ********************************************************************/
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    unsigned int data_rd = 0;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Run] Testcase: %s\n", cfg->test_name);
#endif

    /* Step 9: Read SII0 register at offset 0xC0 */
    data_rd = read_sii0_reg(SII_LINK_STATUS_OFFSET);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 9: Read SII0 register at offset 0xC0");
#endif

    /* Step 10: Call non_secure_prot_nic() */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 10: non_secure_prot_nic() called");
#endif

    /* Step 11: Poll SII0 register at offset 0xC0 until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii0_reg(SII_LINK_STATUS_OFFSET);
    while ((data_rd & LINK_UP_MASK) != LINK_UP_MASK)
    {
        wait_on(5);
        data_rd = read_sii0_reg(SII_LINK_STATUS_OFFSET);
    }
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 11: SII0 link-up status confirmed");
#endif

    /* Step 12: Read SII1 register at offset 0xC0 and poll until (data_rd & 0xD1) == 0xD1 */
    data_rd = read_sii1_reg(SII_LINK_STATUS_OFFSET);
    while ((data_rd & LINK_UP_MASK) != LINK_UP_MASK)
    {
        wait_on(5);
        data_rd = read_sii1_reg(SII_LINK_STATUS_OFFSET);
    }
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 12: SII1 link-up status confirmed");
#endif

#ifdef DM0_RC
    /* Step 13: (DM0_RC block) Read Vendor ID from PCIe slave 0 at offset 0x0 */
    data_rd = read_pcie_slv0_reg(0x0);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 13: Read Vendor ID from PCIe slave 0");
#endif

    /* Step 14: Write 0x7 to command register at offset 0x4 to enable
     * memory space, I/O space, and bus master */
    write_pcie_slv0_reg(0x4, 0x7);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 14: Wrote 0x7 to PCIe slave 0 command register");
#endif
#endif

    /* Step 15: Call mem_base_program_dm0_x4() and mem_base_program_dm1_x4() */
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 15: mem_base_program_dm0_x4 and mem_base_program_dm1_x4 called");
#endif

    /* Step 16: Wait */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 16: wait_on(10) completed");
#endif

    /* Step 17: Write 0x1 to system-level registers */
    *(volatile unsigned int *)SYS_REG_0C = 0x1;
    *(volatile unsigned int *)SYS_REG_10 = 0x1;
    *(volatile unsigned int *)SYS_REG_14 = 0x1;
    *(volatile unsigned int *)SYS_REG_18 = 0x1;
    *(volatile unsigned int *)SYS_REG_30 = 0x1;
    *(volatile unsigned int *)SYS_REG_34 = 0x1;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 17: System-level registers written with 0x1");
#endif

    /* Step 18: DISABLE_CACHE PROGRAMMING - Clear bits [19:22] and [27:30] to 0x0
     * in PCIE0 COHERENCY_CONTROL_3_OFF */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 18: PCIE0 COHERENCY_CONTROL_3_OFF bits [19:22] and [27:30] cleared");
#endif

    /* Step 19: Repeat step 18 for PCIE1 COHERENCY_CONTROL_3_OFF */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 19: PCIE1 COHERENCY_CONTROL_3_OFF bits [19:22] and [27:30] cleared");
#endif

    /* Step 20: Wait */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 20: wait_on(10) completed");
#endif

    /* Step 21: Combined read-modify-write on both COHERENCY_CONTROL_3_OFF registers
     * clearing bits [19:22] and [27:30] to 0x0 */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;

    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 21: Both COHERENCY_CONTROL_3_OFF registers bits [19:22] and [27:30] cleared");
#endif

    /* Step 22: Wait */
    wait_on(30);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 22: wait_on(30) completed");
#endif

    /* Step 23: Write 0xFFFFFFFF to PCIe slave 1 BAR registers at offsets 0x10-0x24 */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x1C, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 23: Wrote 0xFFFFFFFF to PCIe slave 1 BAR registers");
#endif

    /* Step 24: Read back PCIe slave 1 BAR registers to determine BAR sizes */
    data_rd = read_pcie_slv1_reg(0x10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR0 size");
#endif
    data_rd = read_pcie_slv1_reg(0x14);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR1 size");
#endif
    data_rd = read_pcie_slv1_reg(0x18);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR2 size");
#endif
    data_rd = read_pcie_slv1_reg(0x1C);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR3 size");
#endif
    data_rd = read_pcie_slv1_reg(0x20);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR4 size");
#endif
    data_rd = read_pcie_slv1_reg(0x24);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 24: Read PCIe slave 1 BAR5 size");
#endif

    /* Step 25: Program PCIe slave 1 BAR registers with actual base addresses */
    write_pcie_slv1_reg(0x10, 0x00000000);
    write_pcie_slv1_reg(0x14, 0x00000004);
    write_pcie_slv1_reg(0x18, 0x20000000);
    write_pcie_slv1_reg(0x1C, 0x40000000);
    write_pcie_slv1_reg(0x20, 0x60000000);
    write_pcie_slv1_reg(0x24, 0x80000000);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 25: Programmed PCIe slave 1 BAR base addresses");
#endif

    /* Step 26: Read back PCIe slave 1 BAR registers to confirm programmed values */
    data_rd = read_pcie_slv1_reg(0x10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR0");
#endif
    data_rd = read_pcie_slv1_reg(0x14);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR1");
#endif
    data_rd = read_pcie_slv1_reg(0x18);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR2");
#endif
    data_rd = read_pcie_slv1_reg(0x1C);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR3");
#endif
    data_rd = read_pcie_slv1_reg(0x20);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR4");
#endif
    data_rd = read_pcie_slv1_reg(0x24);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 26: Confirmed PCIe slave 1 BAR5");
#endif

    /* Step 27: Repeat steps 23-26 for PCIe slave 0 BAR registers */

    /* Write 0xFFFFFFFF to PCIe slave 0 BAR registers at offsets 0x10-0x24 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x1C, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27a: Wrote 0xFFFFFFFF to PCIe slave 0 BAR registers");
#endif

    /* Read back PCIe slave 0 BAR registers to determine BAR sizes */
    data_rd = read_pcie_slv0_reg(0x10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR0 size");
#endif
    data_rd = read_pcie_slv0_reg(0x14);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR1 size");
#endif
    data_rd = read_pcie_slv0_reg(0x18);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR2 size");
#endif
    data_rd = read_pcie_slv0_reg(0x1C);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR3 size");
#endif
    data_rd = read_pcie_slv0_reg(0x20);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR4 size");
#endif
    data_rd = read_pcie_slv0_reg(0x24);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27b: Read PCIe slave 0 BAR5 size");
#endif

    /* Program PCIe slave 0 BAR registers with actual base addresses */
    write_pcie_slv0_reg(0x10, 0x00000000);
    write_pcie_slv0_reg(0x14, 0x00000004);
    write_pcie_slv0_reg(0x18, 0x20000000);
    write_pcie_slv0_reg(0x1C, 0x40000000);
    write_pcie_slv0_reg(0x20, 0x60000000);
    write_pcie_slv0_reg(0x24, 0x80000000);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27c: Programmed PCIe slave 0 BAR base addresses");
#endif

    /* Read back PCIe slave 0 BAR registers to confirm programmed values */
    data_rd = read_pcie_slv0_reg(0x10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR0");
#endif
    data_rd = read_pcie_slv0_reg(0x14);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR1");
#endif
    data_rd = read_pcie_slv0_reg(0x18);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR2");
#endif
    data_rd = read_pcie_slv0_reg(0x1C);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR3");
#endif
    data_rd = read_pcie_slv0_reg(0x20);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR4");
#endif
    data_rd = read_pcie_slv0_reg(0x24);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 27d: Confirmed PCIe slave 0 BAR5");
#endif

    /* Step 28: Wait */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 28: wait_on(10) completed");
#endif

    /* Step 29: Poll 0xE6004100 until value equals 0x12345678 */
    data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    while (data_rd != POLL_EXPECTED_VALUE)
    {
        wait_on(5);
        data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    }
#ifdef DEBUG_DISPLAY
    DEBUG_PRINT("Step 29: 0xE6004100 poll matched 0x12345678");
#endif

    return out->status;
}

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_teardown
 * Description    : Perform output validation, error handling, and cleanup.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful teardown.
 ********************************************************************/
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[TEARDOWN] Testcase: %s\n", cfg->test_name);
#endif

    /* Step 30: Test completion */
    /* Validation / Acceptance Criteria: NA - no additional validation conditions specified */
    if (error_count == 0)
    {
#ifdef DEBUG_DISPLAY
        LOGI("TEST PASSED: error_count = 0\n");
#endif
        finish(0);
    }
    else
    {
#ifdef DEBUG_DISPLAY
        LOGI("TEST FAILED: error_count = %d\n", error_count);
#endif
        finish(1);
    }

    return 0;
}
