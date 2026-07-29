// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

/*
 * Test Case: pcie_enumerate_test
 * Description: Converted directly from Meta Test Steps / Procedure without reordering or optimization.
 */

#include "test_define.c"

/* --------------------------------------------------------------------------
 * Helper: set_data
 * Set bits in range [start:end] (inclusive) of 'src' to 'val'. No side effects.
 * -------------------------------------------------------------------------- */
static inline uint32_t set_data(uint32_t src, int start, int end, uint32_t val)
{
    /* compute mask for inclusive bit range */
    const int lsb = start;
    const int msb = end;
    const uint32_t width = (uint32_t)(msb - lsb + 1);
    const uint32_t field_mask = (width >= 32u) ? 0xFFFFFFFFu : (((1u << width) - 1u) << lsb);
    const uint32_t val_masked = (width >= 32u) ? val : ((val & ((1u << width) - 1u)) << lsb);
    return (src & ~field_mask) | val_masked;
}

/* --------------------------------------------------------------------------
 * Entry Point
 * Implements the exact Meta Test Steps with debug logs and validation.
 * -------------------------------------------------------------------------- */
int test_case(void)
{
    int error_count = 0;          /* error counter */
    uint32_t rd_wr_data1 = 0u;    /* temp data for RMW */
    uint32_t data_rd = 0u;        /* temp data for reads */

#ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Start\n");
#endif

    /* 1) write_reg(0xE6004100, 0x0); */
#ifdef DEBUG_DISPLAY
    printf("Step 1: Clear handshake register 0xE6004100 to 0x0\n");
#endif
    write_reg(0xE6004100u, 0x0u);

    /* 2) Conditional link training based on build flags */
#ifdef DEBUG_DISPLAY
    printf("Step 2: Link training based on build flags\n");
#endif
#if defined(DM0_RC)
    link_training_dm0_x4(4);
#elif defined(DM1_RC)
    link_training_dm1_x4(4);
#elif defined(DM0_EP)
    link_training_dm0_x4(4);
#elif defined(DM1_EP)
    link_training_dm1_x4(4);
#else
    /* No link training mode defined at build-time; proceed without invoking training */
#endif

    /* 3) DBI coherency programming round 1 (PCIE0) */
#ifdef DEBUG_DISPLAY
    printf("Step 3: DBI coherency programming round 1 (PCIE0)\n");
#endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 4) DBI coherency programming round 1 (PCIE1) */
#ifdef DEBUG_DISPLAY
    printf("Step 4: DBI coherency programming round 1 (PCIE1)\n");
#endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 5) wait_on(20); */
#ifdef DEBUG_DISPLAY
    printf("Step 5: wait_on(20)\n");
#endif
    wait_on(20);

    /* 5->6) DBI coherency programming round 2 (PCIE0) */
#ifdef DEBUG_DISPLAY
    printf("Step 6: DBI coherency programming round 2 (PCIE0)\n");
#endif
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 6) DBI coherency programming round 2 (PCIE1) */
#ifdef DEBUG_DISPLAY
    printf("Step 6: DBI coherency programming round 2 (PCIE1)\n");
#endif
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 7) Poll SII0 until link-up condition met */
#ifdef DEBUG_DISPLAY
    printf("Step 7: Poll SII0 status at 0xC0 until (val & 0xD1) == 0xD1\n");
#endif
    data_rd = read_sii0_reg(0xC0u);
    while ((data_rd & 0xD1u) != 0xD1u) {
        data_rd = read_sii0_reg(0xC0u);
    }

    /* 8) Poll SII1 until link-up condition met */
#ifdef DEBUG_DISPLAY
    printf("Step 8: Poll SII1 status at 0xC0 until (val & 0xD1) == 0xD1\n");
#endif
    data_rd = read_sii1_reg(0xC0u);
    while ((data_rd & 0xD1u) != 0xD1u) {
        data_rd = read_sii1_reg(0xC0u);
    }

    /* 9) non_secure_prot_nic(); */
#ifdef DEBUG_DISPLAY
    printf("Step 9: Configure non-secure protection on NIC\n");
#endif
    non_secure_prot_nic();

    /* 10) If DM0_RC: Vendor ID read and SLV0 programming */
#ifdef DEBUG_DISPLAY
    printf("Step 10: Conditional DM0_RC vendor ID read and SLV0 programming\n");
#endif
#if defined(DM0_RC)
    rd_wr_data1 = read_pcie_slv0_reg(0x0u);
#ifdef DEBUG_DISPLAY
    printf("  DM0_RC: SLV0 VENDOR ID = 0x%08X\n", rd_wr_data1);
#endif
    write_pcie_slv0_reg(0x4u, 0x7u);
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    wait_on(10);
#endif

    /* 11) Write system control enables */
#ifdef DEBUG_DISPLAY
    printf("Step 11: System control enables writes\n");
#endif
    write_reg(0xE690000Cu, 0x1u);
    write_reg(0xE6900010u, 0x1u);
    write_reg(0xE6900014u, 0x1u);
    write_reg(0xE6900018u, 0x1u);
    write_reg(0xE6900030u, 0x1u);
    write_reg(0xE6900034u, 0x1u);

    /* 12) Disable cache programming phase 1 (PCIE0, PCIE1) */
#ifdef DEBUG_DISPLAY
    printf("Step 12: Disable cache programming phase 1 (PCIE0)\n");
#endif
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0u);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

#ifdef DEBUG_DISPLAY
    printf("Step 12: Disable cache programming phase 1 (PCIE1)\n");
#endif
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0u);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 13) wait_on(10); then phase 2 (PCIE0, PCIE1) */
#ifdef DEBUG_DISPLAY
    printf("Step 13: wait_on(10) then disable cache programming phase 2 (PCIE0/PCIE1)\n");
#endif
    wait_on(10);

    /* PCIE0 */
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0u);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0u);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* PCIE1 */
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0u);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0u);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* 14) wait_on(30); */
#ifdef DEBUG_DISPLAY
    printf("Step 14: wait_on(30)\n");
#endif
    wait_on(30);

    /* 15) SLV1 BARs: write FFFFFFFF then read back */
#ifdef DEBUG_DISPLAY
    printf("Step 15: SLV1 BARs write 0xFFFFFFFF and read-back verify\n");
#endif
    write_pcie_slv1_reg(0x10u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x10u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv1_reg(0x14u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x14u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv1_reg(0x18u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x18u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv1_reg(0x1Cu, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x1Cu);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv1_reg(0x20u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x20u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv1_reg(0x24u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv1_reg(0x24u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    /* 16) Program SLV1 BARs to target values and read back */
#ifdef DEBUG_DISPLAY
    printf("Step 16: SLV1 BARs program to target values and read-back verify\n");
#endif
    write_pcie_slv1_reg(0x10u, 0x00000000u);
    data_rd = read_pcie_slv1_reg(0x10u);
    if (data_rd != 0x00000000u) { error_count++; }

    write_pcie_slv1_reg(0x14u, 0x00000004u);
    data_rd = read_pcie_slv1_reg(0x14u);
    if (data_rd != 0x00000004u) { error_count++; }

    write_pcie_slv1_reg(0x18u, 0x20000000u);
    data_rd = read_pcie_slv1_reg(0x18u);
    if (data_rd != 0x20000000u) { error_count++; }

    write_pcie_slv1_reg(0x1Cu, 0x40000000u);
    data_rd = read_pcie_slv1_reg(0x1Cu);
    if (data_rd != 0x40000000u) { error_count++; }

    write_pcie_slv1_reg(0x20u, 0x60000000u);
    data_rd = read_pcie_slv1_reg(0x20u);
    if (data_rd != 0x60000000u) { error_count++; }

    write_pcie_slv1_reg(0x24u, 0x80000000u);
    data_rd = read_pcie_slv1_reg(0x24u);
    if (data_rd != 0x80000000u) { error_count++; }

    /* 17) Repeat BAR sequence for SLV0 */
#ifdef DEBUG_DISPLAY
    printf("Step 17: SLV0 BARs write 0xFFFFFFFF and read-back verify\n");
#endif
    write_pcie_slv0_reg(0x10u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x10u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv0_reg(0x14u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x14u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv0_reg(0x18u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x18u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv0_reg(0x1Cu, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x1Cu);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv0_reg(0x20u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x20u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

    write_pcie_slv0_reg(0x24u, 0xFFFFFFFFu);
    data_rd = read_pcie_slv0_reg(0x24u);
    if (data_rd != 0xFFFFFFFFu) { error_count++; }

#ifdef DEBUG_DISPLAY
    printf("Step 17: SLV0 BARs program to target values and read-back verify\n");
#endif
    write_pcie_slv0_reg(0x10u, 0x00000000u);
    data_rd = read_pcie_slv0_reg(0x10u);
    if (data_rd != 0x00000000u) { error_count++; }

    write_pcie_slv0_reg(0x14u, 0x00000004u);
    data_rd = read_pcie_slv0_reg(0x14u);
    if (data_rd != 0x00000004u) { error_count++; }

    write_pcie_slv0_reg(0x18u, 0x20000000u);
    data_rd = read_pcie_slv0_reg(0x18u);
    if (data_rd != 0x20000000u) { error_count++; }

    write_pcie_slv0_reg(0x1Cu, 0x40000000u);
    data_rd = read_pcie_slv0_reg(0x1Cu);
    if (data_rd != 0x40000000u) { error_count++; }

    write_pcie_slv0_reg(0x20u, 0x60000000u);
    data_rd = read_pcie_slv0_reg(0x20u);
    if (data_rd != 0x60000000u) { error_count++; }

    write_pcie_slv0_reg(0x24u, 0x80000000u);
    data_rd = read_pcie_slv0_reg(0x24u);
    if (data_rd != 0x80000000u) { error_count++; }

    /* 18) wait_on(10); then poll read_reg(0xE6004100) until it equals 0x12345678 */
#ifdef DEBUG_DISPLAY
    printf("Step 18: wait_on(10) then poll handshake register for completion (0x12345678)\n");
#endif
    wait_on(10);
    data_rd = read_reg(0xE6004100u);
    while (data_rd != 0x12345678u) {
        data_rd = read_reg(0xE6004100u);
    }

    /* 19) Test completion: PASS if no errors, else FAIL */
#ifdef DEBUG_DISPLAY
    printf("Step 19: Test completion with error_count=%d\n", error_count);
#endif
    if (error_count == 0) {
        finish(0);  /* PASS */
    } else {
        finish(1);  /* FAIL */
    }

    return 0; /* Unreachable due to finish(), but provided for completeness */
}
