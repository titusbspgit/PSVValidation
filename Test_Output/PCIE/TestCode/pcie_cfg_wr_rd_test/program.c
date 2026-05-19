// Author - AI Force 1.3.2. Date 19-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*==============================================================
 * Function: program_coherency_fields
 * Description: Apply coherency/cache settings on impacted DBI DSP
 *              COHERENCY_CONTROL_3 registers as per Meta Steps.
 *==============================================================*/
static void program_coherency_fields(void)
{
    for (int idx = 0; idx < 2; ++idx)
    {
        unsigned int addr = pcie_coh_ctrl_regs[idx];
        unsigned int val  = read_reg(addr);
#ifdef DEBUG_DISPLAY
        printf("[DBG] Initial COH3[reg%d]=0x%08X\n", idx, val);
#endif
        /* Set [11:14] = 0xF and [3:6] = 0xF */
        val &= ~(AW_CACHE_MASK | AR_CACHE_MASK);
        val |= ((0xFU << 11) | (0xFU << 3));
        write_reg(addr, val);
#ifdef DEBUG_DISPLAY
        printf("[DBG] Set AW/AR cache (11:14,3:6) to 0xF, COH3[reg%d]=0x%08X\n", idx, val);
#endif
        /* Then set [27:30] = 0xF and [19:22] = 0xF */
        val  = read_reg(addr);
        val &= ~(AW2_CACHE_MASK | AR2_CACHE_MASK);
        val |= ((0xFU << 27) | (0xFU << 19));
        write_reg(addr, val);
#ifdef DEBUG_DISPLAY
        printf("[DBG] Set upper fields (27:30,19:22) to 0xF, COH3[reg%d]=0x%08X\n", idx, val);
#endif
    }
}

/*==============================================================
 * Function: reapply_coherency_fields
 * Description: Re-apply all four bitfields to 0xF as per Meta Step 4.
 *==============================================================*/
static void reapply_coherency_fields(void)
{
    for (int idx = 0; idx < 2; ++idx)
    {
        unsigned int addr = pcie_coh_ctrl_regs[idx];
        unsigned int val  = read_reg(addr);
        val &= ~(AW_CACHE_MASK | AR_CACHE_MASK | AW2_CACHE_MASK | AR2_CACHE_MASK);
        val |= ((0xFU << 11) | (0xFU << 3) | (0xFU << 27) | (0xFU << 19));
        write_reg(addr, val);
#ifdef DEBUG_DISPLAY
        printf("[DBG] Reapply all fields to 0xF, COH3[reg%d]=0x%08X\n", idx, val);
#endif
    }
}

/*==============================================================
 * Function: cfg_seq_dm0
 * Description: DM0_RC configuration sequence per Meta Steps.
 *==============================================================*/
static void cfg_seq_dm0(void)
{
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
    printf("[DBG] DM0_RC: Starting configuration sequence.\n");
#endif
    mem_base_program_dm0_x4();
    wait_on(10);

    for (int i = 0; i < 10; ++i)
    {
        (void)read_pcie_slv0_reg((unsigned int)(i * 0x4));
#ifdef DEBUG_DISPLAY
        printf("[DBG] DM0_RC: Read cfg dword @0x%02X\n", i * 0x4);
#endif
    }

    write_pcie_slv0_reg(0x10, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x1C, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFFU);

    (void)read_pcie_slv0_reg(0x10);
    (void)read_pcie_slv0_reg(0x14);
    (void)read_pcie_slv0_reg(0x18);
    (void)read_pcie_slv0_reg(0x1C);
    (void)read_pcie_slv0_reg(0x20);
    (void)read_pcie_slv0_reg(0x24);

    write_pcie_slv0_reg(0x10, 0x00000000U);
    write_pcie_slv0_reg(0x14, 0x00000004U);
    write_pcie_slv0_reg(0x18, 0x20000000U);
    write_pcie_slv0_reg(0x1C, 0x40000000U);
    write_pcie_slv0_reg(0x20, 0x60000000U);
    write_pcie_slv0_reg(0x24, 0x80000000U);

    (void)read_pcie_slv0_reg(0x10);
    (void)read_pcie_slv0_reg(0x14);
    (void)read_pcie_slv0_reg(0x18);
    (void)read_pcie_slv0_reg(0x1C);
    (void)read_pcie_slv0_reg(0x20);
    (void)read_pcie_slv0_reg(0x24);

    write_pcie_slv0_reg(0x04, 0x00000007U); /* Enable Mem/IO/BusMaster */
#ifdef DEBUG_DISPLAY
    printf("[DBG] DM0_RC: Configuration sequence complete.\n");
#endif
#endif /* DM0_RC */
}

/*==============================================================
 * Function: cfg_seq_dm1
 * Description: DM1_RC configuration sequence per Meta Steps.
 *==============================================================*/
static void cfg_seq_dm1(void)
{
#ifdef DM1_RC
#ifdef DEBUG_DISPLAY
    printf("[DBG] DM1_RC: Starting configuration sequence.\n");
#endif
    mem_base_program_dm1_x4();
    wait_on(10);

    for (int i = 0; i < 10; ++i)
    {
        (void)read_pcie_slv1_reg((unsigned int)(i * 0x4));
#ifdef DEBUG_DISPLAY
        printf("[DBG] DM1_RC: Read cfg dword @0x%02X\n", i * 0x4);
#endif
    }

    write_pcie_slv1_reg(0x10, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x1C, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFFU);

    (void)read_pcie_slv1_reg(0x10);
    (void)read_pcie_slv1_reg(0x14);
    (void)read_pcie_slv1_reg(0x18);
    (void)read_pcie_slv1_reg(0x1C);
    (void)read_pcie_slv1_reg(0x20);
    (void)read_pcie_slv1_reg(0x24);

    write_pcie_slv1_reg(0x10, 0x00000000U);
    write_pcie_slv1_reg(0x14, 0x00000004U);
    write_pcie_slv1_reg(0x18, 0x20000000U);
    write_pcie_slv1_reg(0x1C, 0x40000000U);
    write_pcie_slv1_reg(0x20, 0x60000000U);
    write_pcie_slv1_reg(0x24, 0x80000000U);

    (void)read_pcie_slv1_reg(0x10);
    (void)read_pcie_slv1_reg(0x14);
    (void)read_pcie_slv1_reg(0x18);
    (void)read_pcie_slv1_reg(0x1C);
    (void)read_pcie_slv1_reg(0x20);
    (void)read_pcie_slv1_reg(0x24);

    write_pcie_slv1_reg(0x04, 0x00000007U); /* Enable Mem/IO/BusMaster */
#ifdef DEBUG_DISPLAY
    printf("[DBG] DM1_RC: Configuration sequence complete.\n");
#endif
#endif /* DM1_RC */
}

/*==============================================================
 * Function: test_case
 * Description: Implements Meta Test Steps for pcie_cfg_wr_rd_test.
 *==============================================================*/
static void test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] Start test: pcie_cfg_wr_rd_test\n");
#endif
    /* 1) Initialize sync register to 0 */
    write_reg(SYNC_REG_ADDR, 0x00000000U);

    /* 2) Link training based on role */
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

    /* 3) Program coherency/cache fields on DBI DSP COH3 for PCIe0 and PCIe1 */
    program_coherency_fields();

    /* 4) Wait then re-apply all four fields */
    wait_on(20);
    reapply_coherency_fields();

    /* 5) Poll SII status until ready */
    unsigned int data_rd = read_sii0_reg(SII_STATUS_OFF);
    while ((data_rd & READY_MASK) != READY_VAL)
    {
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII0 status=0x%08X (waiting for mask 0x%08X==0x%08X)\n", data_rd, READY_MASK, READY_VAL);
#endif
        data_rd = read_sii0_reg(SII_STATUS_OFF);
    }

#ifdef DM1_RC
    unsigned int data_rd1 = read_sii1_reg(SII_STATUS_OFF);
    while ((data_rd1 & READY_MASK) != READY_VAL)
    {
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII1 status=0x%08X (waiting for mask 0x%08X==0x%08X)\n", data_rd1, READY_MASK, READY_VAL);
#endif
        data_rd1 = read_sii1_reg(SII_STATUS_OFF);
    }
#endif

    /* 6) Handshake start */
    write_reg(SYNC_REG_ADDR, HANDSHAKE_START);
    wait_on(15000);

    /* 7/8) Perform RC-specific configuration sequences */
    cfg_seq_dm0();
    cfg_seq_dm1();

    /* 9) Poll for handshake completion */
    unsigned int hs = read_reg(SYNC_REG_ADDR);
    while (hs != HANDSHAKE_DONE)
    {
#ifdef DEBUG_DISPLAY
        printf("[DBG] Waiting for handshake completion: SYNC=0x%08X (expect 0x%08X)\n", hs, HANDSHAKE_DONE);
#endif
        wait_on(5);
        hs = read_reg(SYNC_REG_ADDR);
    }

#ifdef DEBUG_DISPLAY
    printf("[DBG] Handshake complete. Test PASS.\n");
#endif
    /* 10) Finish with PASS */
    finish(0);
}

/*==============================================================
 * Function: main
 * Description: Entry point; invokes test_case().
 *==============================================================*/
int main(void)
{
    test_case();
    return 0; /* finish() should terminate; return to satisfy signature */
}
