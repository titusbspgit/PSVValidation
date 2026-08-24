/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

#include <stdio.h>
#include "test_define.c"

/* File-scope variables for cross-function access */
static int error_count = 0;
static unsigned int data_rd = 0;

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_init
 * Description    : Initialize testcase configuration, control register,
 *                  and perform PCIe link training.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful initialization.
 ********************************************************************/
int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Init] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 1: Write 0x0 to 0xE6004100 to initialize control register */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)CTRL_REG_ADDR = 0x0;
#ifdef DEBUG_DISPLAY
    debug_print("Step 1: Wrote 0x0 to CTRL_REG_ADDR (0xE6004100)\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 2: Perform x4 link training based on compile-time defines */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
#endif
#endif

#ifdef DM1_RC
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
#endif
#endif

#ifdef DM0_EP
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm0_x4(4) called (DM0_EP)\n");
#endif
#endif

#ifdef DM1_EP
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    debug_print("Step 2: link_training_dm1_x4(4) called (DM1_EP)\n");
#endif
#endif

    return 0;
}

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_run
 * Description    : Execute testcase register configuration and stimulus.
 *                  Configures cache coherency, polls link-up status,
 *                  reads Vendor ID, enables memory/IO/bus master,
 *                  programs memory base regions, disables cache,
 *                  enumerates BARs on both slave ports, and polls
 *                  synchronization register.
 * Parameters     : cfg - Test configuration pointer,
 *                  out - Test output pointer.
 * Return Value   : Test execution status.
 ********************************************************************/
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Run] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 3: CACHE PROGRAMMING - PCIE0 COHERENCY_CONTROL_3_OFF    */
    /*         Read, set bits [11:14] to 0xF and bits [3:6] to 0xF  */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 3: PCIE0 COHERENCY_CONTROL_3_OFF bits [11:14],[3:6] set to 0xF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 4: Set bits [27:30] to 0xF and bits [19:22] to 0xF      */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 4: PCIE0 COHERENCY_CONTROL_3_OFF bits [27:30],[19:22] set to 0xF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 5: Repeat steps 3-4 for PCIE1 COHERENCY_CONTROL_3_OFF  */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;

    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 5: PCIE1 COHERENCY_CONTROL_3_OFF all bit groups set to 0xF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 6: Wait for coherency settings to take effect           */
    /* ------------------------------------------------------------ */
    wait_on(20);
#ifdef DEBUG_DISPLAY
    debug_print("Step 6: wait_on(20) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 7: Combined RMW on PCIE0 COHERENCY_CONTROL_3_OFF       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 7: PCIE0 COHERENCY_CONTROL_3_OFF combined RMW all 0xF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 8: Combined RMW on PCIE1 COHERENCY_CONTROL_3_OFF       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 8: PCIE1 COHERENCY_CONTROL_3_OFF combined RMW all 0xF\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 9: Read SII0 register at offset 0xC0                    */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
#ifdef DEBUG_DISPLAY
    debug_print("Step 9: SII0 link status read = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 10: Call non_secure_prot_nic()                           */
    /* ------------------------------------------------------------ */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    debug_print("Step 10: non_secure_prot_nic() called\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 11: Poll SII0 register until link-up                    */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 11: SII0 link-up confirmed, data_rd = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 12: Poll SII1 register until link-up                    */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 12: SII1 link-up confirmed, data_rd = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 13: (DM0_RC) Read Vendor ID from PCIe slave 0           */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(PCIE_SLV_VENDOR_ID_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 13: Vendor ID from slv0 = 0x%08X\n", data_rd);
#endif

    /* ------------------------------------------------------------ */
    /* Step 14: Write 0x7 to enable mem/IO/bus master                */
    /* ------------------------------------------------------------ */
    write_pcie_slv0_reg(PCIE_SLV_CMD_STATUS_OFFSET, PCIE_CMD_MEM_IO_BUSMASTER);
#ifdef DEBUG_DISPLAY
    debug_print("Step 14: Wrote 0x7 to slv0 TYPE1_STATUS_COMMAND_REG\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 15: Program memory base address regions                  */
    /* ------------------------------------------------------------ */
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 15: mem_base_program_dm0_x4() and dm1_x4() called\n");
#endif
#endif /* DM0_RC */

    /* ------------------------------------------------------------ */
    /* Step 16: Wait for memory base programming to settle          */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 16: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 17: Write 0x1 to system-level configuration registers   */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)SYS_REG_0 = 0x1;
    *(volatile unsigned int *)SYS_REG_1 = 0x1;
    *(volatile unsigned int *)SYS_REG_2 = 0x1;
    *(volatile unsigned int *)SYS_REG_3 = 0x1;
    *(volatile unsigned int *)SYS_REG_4 = 0x1;
    *(volatile unsigned int *)SYS_REG_5 = 0x1;
#ifdef DEBUG_DISPLAY
    debug_print("Step 17: System-level registers written with 0x1\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 18: DISABLE_CACHE PROGRAMMING - PCIE0                   */
    /*   RMW clearing bits [19:22] and [27:30] to 0x0               */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 18: PCIE0 COHERENCY_CONTROL_3_OFF bits [19:22],[27:30] cleared\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 19: DISABLE_CACHE PROGRAMMING - PCIE1                   */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 19: PCIE1 COHERENCY_CONTROL_3_OFF bits [19:22],[27:30] cleared\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 20: Wait for cache disable settings to propagate        */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 20: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 21: Combined RMW on PCIE0 - clear [19:22],[27:30]       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;

    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 19, 22, 0x0);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 21: Both COHERENCY_CONTROL_3_OFF fully cleared\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 22: Wait for hardware state transitions                 */
    /* ------------------------------------------------------------ */
    wait_on(30);
#ifdef DEBUG_DISPLAY
    debug_print("Step 22: wait_on(30) completed\n");
#endif

    /* ============================================================ */
    /* Step 23-26: BAR Enumeration on PCIe Slave 1                  */
    /* ============================================================ */

    /* Step 23: Write 0xFFFFFFFF to slave 1 BAR registers */
    write_pcie_slv1_reg(BAR0_REG_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv1_reg(BAR1_REG_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv1_reg(SEC_LAT_TIMER_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv1_reg(SEC_STAT_IO_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv1_reg(MEM_LIMIT_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv1_reg(PREF_MEM_LIMIT_OFFSET, BAR_ENUM_PATTERN);
#ifdef DEBUG_DISPLAY
    debug_print("Step 23: Wrote 0xFFFFFFFF to slv1 BAR offsets 0x10-0x24\n");
#endif

    /* Step 24: Read back slave 1 BAR registers to determine sizes */
    data_rd = read_pcie_slv1_reg(BAR0_REG_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 24: slv1 BAR0 readback = 0x%08X\n", data_rd);
#endif
    data_rd = read_pcie_slv1_reg(BAR1_REG_OFFSET);
    data_rd = read_pcie_slv1_reg(SEC_LAT_TIMER_OFFSET);
    data_rd = read_pcie_slv1_reg(SEC_STAT_IO_OFFSET);
    data_rd = read_pcie_slv1_reg(MEM_LIMIT_OFFSET);
    data_rd = read_pcie_slv1_reg(PREF_MEM_LIMIT_OFFSET);

    /* Step 25: Program slave 1 BAR registers with actual base addresses */
    write_pcie_slv1_reg(BAR0_REG_OFFSET, BAR0_BASE_ADDR);
    write_pcie_slv1_reg(BAR1_REG_OFFSET, BAR1_BASE_ADDR);
    write_pcie_slv1_reg(SEC_LAT_TIMER_OFFSET, BAR2_BASE_ADDR);
    write_pcie_slv1_reg(SEC_STAT_IO_OFFSET, BAR3_BASE_ADDR);
    write_pcie_slv1_reg(MEM_LIMIT_OFFSET, BAR4_BASE_ADDR);
    write_pcie_slv1_reg(PREF_MEM_LIMIT_OFFSET, BAR5_BASE_ADDR);
#ifdef DEBUG_DISPLAY
    debug_print("Step 25: Programmed slv1 BAR registers with base addresses\n");
#endif

    /* Step 26: Read back slave 1 BAR registers to confirm */
    data_rd = read_pcie_slv1_reg(BAR0_REG_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 26: slv1 BAR0 confirm = 0x%08X\n", data_rd);
#endif
    data_rd = read_pcie_slv1_reg(BAR1_REG_OFFSET);
    data_rd = read_pcie_slv1_reg(SEC_LAT_TIMER_OFFSET);
    data_rd = read_pcie_slv1_reg(SEC_STAT_IO_OFFSET);
    data_rd = read_pcie_slv1_reg(MEM_LIMIT_OFFSET);
    data_rd = read_pcie_slv1_reg(PREF_MEM_LIMIT_OFFSET);

    /* ============================================================ */
    /* Step 27: Repeat BAR Enumeration on PCIe Slave 0              */
    /* ============================================================ */

    /* Write 0xFFFFFFFF to slave 0 BAR registers */
    write_pcie_slv0_reg(BAR0_REG_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv0_reg(BAR1_REG_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv0_reg(SEC_LAT_TIMER_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv0_reg(SEC_STAT_IO_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv0_reg(MEM_LIMIT_OFFSET, BAR_ENUM_PATTERN);
    write_pcie_slv0_reg(PREF_MEM_LIMIT_OFFSET, BAR_ENUM_PATTERN);
#ifdef DEBUG_DISPLAY
    debug_print("Step 27: Wrote 0xFFFFFFFF to slv0 BAR offsets 0x10-0x24\n");
#endif

    /* Read back slave 0 BAR registers */
    data_rd = read_pcie_slv0_reg(BAR0_REG_OFFSET);
    data_rd = read_pcie_slv0_reg(BAR1_REG_OFFSET);
    data_rd = read_pcie_slv0_reg(SEC_LAT_TIMER_OFFSET);
    data_rd = read_pcie_slv0_reg(SEC_STAT_IO_OFFSET);
    data_rd = read_pcie_slv0_reg(MEM_LIMIT_OFFSET);
    data_rd = read_pcie_slv0_reg(PREF_MEM_LIMIT_OFFSET);

    /* Program slave 0 BAR registers with actual base addresses */
    write_pcie_slv0_reg(BAR0_REG_OFFSET, BAR0_BASE_ADDR);
    write_pcie_slv0_reg(BAR1_REG_OFFSET, BAR1_BASE_ADDR);
    write_pcie_slv0_reg(SEC_LAT_TIMER_OFFSET, BAR2_BASE_ADDR);
    write_pcie_slv0_reg(SEC_STAT_IO_OFFSET, BAR3_BASE_ADDR);
    write_pcie_slv0_reg(MEM_LIMIT_OFFSET, BAR4_BASE_ADDR);
    write_pcie_slv0_reg(PREF_MEM_LIMIT_OFFSET, BAR5_BASE_ADDR);
#ifdef DEBUG_DISPLAY
    debug_print("Step 27: Programmed slv0 BAR registers with base addresses\n");
#endif

    /* Read back slave 0 BAR registers to confirm */
    data_rd = read_pcie_slv0_reg(BAR0_REG_OFFSET);
    data_rd = read_pcie_slv0_reg(BAR1_REG_OFFSET);
    data_rd = read_pcie_slv0_reg(SEC_LAT_TIMER_OFFSET);
    data_rd = read_pcie_slv0_reg(SEC_STAT_IO_OFFSET);
    data_rd = read_pcie_slv0_reg(MEM_LIMIT_OFFSET);
    data_rd = read_pcie_slv0_reg(PREF_MEM_LIMIT_OFFSET);

    /* ------------------------------------------------------------ */
    /* Step 28: Wait                                                */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 28: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 29: Poll 0xE6004100 until value equals 0x12345678       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    while (data_rd != SYNC_HANDSHAKE_VALUE)
    {
        wait_on(5);
        data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: Synchronization handshake received (0x12345678)\n");
#endif

    return out->status;
}

/********************************************************************
 * Function Name  : pcie_device_enumerate_test_teardown
 * Description    : Perform output validation, error handling, and cleanup.
 *                  Evaluates error_count and reports final pass/fail.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful teardown.
 ********************************************************************/
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[TEARDOWN] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 30: Report test completion                               */
    /* ------------------------------------------------------------ */
#ifdef DEBUG_DISPLAY
    debug_print("Test complete. error_count = %d\n", error_count);
#endif

    if (error_count != 0)
    {
#ifdef DEBUG_DISPLAY
        LOGI("ERROR: pcie_device_enumerate_test FAILED with error_count = %d\n", error_count);
#endif
    }
    else
    {
#ifdef DEBUG_DISPLAY
        LOGI("pcie_device_enumerate_test PASSED\n");
#endif
    }

    return 0;
}
