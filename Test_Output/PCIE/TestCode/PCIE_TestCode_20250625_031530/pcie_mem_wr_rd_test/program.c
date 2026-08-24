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
 * Function Name  : pcie_mem_wr_rd_test_init
 * Description    : Initialize testcase configuration, control register,
 *                  and perform PCIe link training.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful initialization.
 ********************************************************************/
int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
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
 * Function Name  : pcie_mem_wr_rd_test_run
 * Description    : Execute testcase register configuration and stimulus.
 *                  Configures cache coherency, polls link-up, reads
 *                  Vendor ID, enables memory/IO/bus master, programs
 *                  BARs and memory base, performs memory write-read
 *                  operations, disables cache, and polls sync register.
 * Parameters     : cfg - Test configuration pointer,
 *                  out - Test output pointer.
 * Return Value   : Test execution status.
 ********************************************************************/
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
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
    /* Step 10: (DM0 block) Poll SII0 until link-up                 */
    /* ------------------------------------------------------------ */
#if defined(DM0_RC) || defined(DM0_EP)
    data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII0_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 10: SII0 link-up confirmed\n");
#endif
#endif

    /* ------------------------------------------------------------ */
    /* Step 11: (DM1 block) Poll SII1 until link-up                 */
    /* ------------------------------------------------------------ */
#if defined(DM1_RC) || defined(DM1_EP)
    data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    while ((data_rd & SII_LINK_UP_MASK) != SII_LINK_UP_MASK)
    {
        data_rd = *(volatile unsigned int *)SII1_LINK_STATUS_REG;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 11: SII1 link-up confirmed\n");
#endif
#endif

    /* ------------------------------------------------------------ */
    /* Step 12: (DM0_EP) Wait for EP readiness                      */
    /* ------------------------------------------------------------ */
#ifdef DM0_EP
    wait_on(30000);
#ifdef DEBUG_DISPLAY
    debug_print("Step 12: wait_on(30000) for DM0_EP\n");
#endif
#endif

    /* ------------------------------------------------------------ */
    /* Step 13-15: (DM0_RC) Vendor ID, cmd, BAR, mem base           */
    /* ------------------------------------------------------------ */
#ifdef DM0_RC
    /* Step 13: Read Vendor ID from PCIe slave 0 */
    data_rd = read_pcie_slv0_reg(PCIE_SLV_VENDOR_ID_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 13: Vendor ID from slv0 = 0x%08X\n", data_rd);
#endif

    /* Step 14: Write 0x7 to enable mem/IO/bus master */
    write_pcie_slv0_reg(PCIE_SLV_CMD_STATUS_OFFSET, PCIE_CMD_MEM_IO_BUSMASTER);
#ifdef DEBUG_DISPLAY
    debug_print("Step 14: Wrote 0x7 to slv0 TYPE1_STATUS_COMMAND_REG\n");
#endif

    /* Step 15: Program BARs and memory base */
    bar_program_dm0_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 15: bar_program_dm0_x4() and mem_base_program_dm0_x4() called\n");
#endif
#endif /* DM0_RC */

    /* ------------------------------------------------------------ */
    /* Step 16: (DM1_RC) Vendor ID, cmd, BAR, mem base              */
    /* ------------------------------------------------------------ */
#ifdef DM1_RC
    data_rd = read_pcie_slv1_reg(PCIE_SLV_VENDOR_ID_OFFSET);
#ifdef DEBUG_DISPLAY
    debug_print("Step 16: Vendor ID from slv1 = 0x%08X\n", data_rd);
#endif
    write_pcie_slv1_reg(PCIE_SLV_CMD_STATUS_OFFSET, PCIE_CMD_MEM_IO_BUSMASTER);
    bar_program_dm1_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 16: DM1_RC BAR and mem base programmed\n");
#endif
#endif /* DM1_RC */

    /* ------------------------------------------------------------ */
    /* Step 17: (DM0_EP) EP BAR and memory base programming         */
    /* ------------------------------------------------------------ */
#ifdef DM0_EP
    bar_program_dm0_EP_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 17: DM0_EP BAR and mem base programmed\n");
#endif
#endif /* DM0_EP */

    /* ------------------------------------------------------------ */
    /* Step 18: (DM1_EP) EP BAR and memory base programming         */
    /* ------------------------------------------------------------ */
#ifdef DM1_EP
    bar_program_dm1_EP_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    debug_print("Step 18: DM1_EP BAR and mem base programmed\n");
#endif
#endif /* DM1_EP */

    /* ------------------------------------------------------------ */
    /* Step 19: Configure non-secure protection settings            */
    /* ------------------------------------------------------------ */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    debug_print("Step 19: non_secure_prot_nic() called\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 20: Write synchronization value to control register     */
    /* ------------------------------------------------------------ */
    *(volatile unsigned int *)CTRL_REG_ADDR = 0x11111111;
#ifdef DEBUG_DISPLAY
    debug_print("Step 20: Wrote 0x11111111 to CTRL_REG_ADDR\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 21-22: DISABLE_CACHE PROGRAMMING - PCIE0                */
    /*   Set bits [11:14] to 0xF, [3:6] to 0xF,                    */
    /*   then [27:30] to 0xF, [19:22] to 0x0                       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;

    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 21-22: PCIE0 cache partially disabled\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 23: Repeat for PCIE1 COHERENCY_CONTROL_3_OFF            */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;

    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 27, 30, 0xF);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 23: PCIE1 cache partially disabled\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 24: Wait for cache disable settings to propagate        */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 24: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 25: Combined RMW on PCIE0 - clear [19:22],[27:30]       */
    /*          keep [3:6],[11:14] at 0xF                            */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    *(volatile unsigned int *)mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 25: PCIE0 COHERENCY_CONTROL_3_OFF final cache disable\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 26: Combined RMW on PCIE1 - clear [19:22],[27:30]       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
    data_rd = set_data(data_rd, 11, 14, 0xF);
    data_rd = set_data(data_rd, 3, 6, 0xF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    *(volatile unsigned int *)mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF = data_rd;
#ifdef DEBUG_DISPLAY
    debug_print("Step 26: PCIE1 COHERENCY_CONTROL_3_OFF final cache disable\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 27: Wait for hardware state transitions                 */
    /* ------------------------------------------------------------ */
    wait_on(30);
#ifdef DEBUG_DISPLAY
    debug_print("Step 27: wait_on(30) completed\n");
#endif

    /* ============================================================ */
    /* Step 28: (DM0_RC) Memory write-read operations via slv0      */
    /* ============================================================ */
#ifdef DM0_RC
    pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
#ifdef DEBUG_DISPLAY
    debug_print("Step 28: slv0 mem_wr_rd(0x01040000, 0xa5a5a5a5)\n");
#endif
    pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
#ifdef DEBUG_DISPLAY
    debug_print("Step 28: slv0 mem_wr_rd(0x01000020, 0xa6a6a6a6)\n");
#endif
    pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
#ifdef DEBUG_DISPLAY
    debug_print("Step 28: slv0 mem_wr_rd(0x01004000, 0xa7a7a7a7)\n");
#endif
#endif /* DM0_RC */

    /* ============================================================ */
    /* Step 29: (DM1_RC) Memory write-read operations via slv1      */
    /* ============================================================ */
#ifdef DM1_RC
    pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: slv1 mem_wr_rd(0x01040000, 0xb5b5b5b5)\n");
#endif
    pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: slv1 mem_wr_rd(0x01000020, 0xb5b5b6b6)\n");
#endif
    pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
#ifdef DEBUG_DISPLAY
    debug_print("Step 29: slv1 mem_wr_rd(0x01004000, 0xb7b7b5b5)\n");
#endif
#endif /* DM1_RC */

    /* ============================================================ */
    /* Step 30: (DM0_EP) Memory write-read operations via slv0      */
    /*          Five operations targeting different BAR regions      */
    /* ============================================================ */
#ifdef DM0_EP
    pcie_slv0_mem_wr_rd(0x01040000, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x01000020, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x01004000, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x01008000, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x0100C000, 0x5a5a5a5a);
#ifdef DEBUG_DISPLAY
    debug_print("Step 30: DM0_EP 5 mem_wr_rd operations completed\n");
#endif
#endif /* DM0_EP */

    /* ============================================================ */
    /* Step 31: (DM1_EP) Memory write-read operations via slv1      */
    /*          Five operations targeting different BAR regions      */
    /* ============================================================ */
#ifdef DM1_EP
    pcie_slv1_mem_wr_rd(0x01040000, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x01000020, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x01004000, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x01008000, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x0100C000, 0x5a5a5a5a);
#ifdef DEBUG_DISPLAY
    debug_print("Step 31: DM1_EP 5 mem_wr_rd operations completed\n");
#endif
#endif /* DM1_EP */

    /* ------------------------------------------------------------ */
    /* Step 32: Wait before final polling                           */
    /* ------------------------------------------------------------ */
    wait_on(10);
#ifdef DEBUG_DISPLAY
    debug_print("Step 32: wait_on(10) completed\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 33: Poll 0xE6004100 until value equals 0x12345678       */
    /* ------------------------------------------------------------ */
    data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    while (data_rd != SYNC_HANDSHAKE_VALUE)
    {
        wait_on(5);
        data_rd = *(volatile unsigned int *)CTRL_REG_ADDR;
    }
#ifdef DEBUG_DISPLAY
    debug_print("Step 33: Synchronization handshake received (0x12345678)\n");
#endif

    return out->status;
}

/********************************************************************
 * Function Name  : pcie_mem_wr_rd_test_teardown
 * Description    : Perform output validation, error handling, and cleanup.
 *                  Evaluates error_count and reports final pass/fail.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful teardown.
 ********************************************************************/
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[TEARDOWN] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 34: Report test completion                               */
    /* ------------------------------------------------------------ */
#ifdef DEBUG_DISPLAY
    debug_print("Test complete. error_count = %d\n", error_count);
#endif

    if (error_count != 0)
    {
#ifdef DEBUG_DISPLAY
        LOGI("ERROR: pcie_mem_wr_rd_test FAILED with error_count = %d\n", error_count);
#endif
    }
    else
    {
#ifdef DEBUG_DISPLAY
        LOGI("pcie_mem_wr_rd_test PASSED\n");
#endif
    }

    return 0;
}
