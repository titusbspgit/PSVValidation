// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: test_case
 * --------------------------------------------
 * Entry point for the PCIe enumerate and validation test.
 * Translates the Meta Test Steps to executable logic.
 */
int test_case(void)
{
    int g_error_count = 0;           /* Error counter for validation failures */
    unsigned int data_rd = 0;        /* General purpose read variable */
    unsigned int rd_wr_data1 = 0;    /* Read-modify-write scratch */

    /* Step 1: Clear handshake/scratch register */
    write_reg(0xE6004100, 0x0);
#ifdef DEBUG_DISPLAY
    printf("[STEP 1] write_reg(0xE6004100, 0x0)\n");
#endif

    /* Step 2: Conditional link training */
#if defined(DM0_RC)
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm0_x4(4) for DM0_RC\n");
#endif
#elif defined(DM1_RC)
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm1_x4(4) for DM1_RC\n");
#endif
#elif defined(DM0_EP)
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm0_x4(4) for DM0_EP\n");
#endif
#elif defined(DM1_EP)
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm1_x4(4) for DM1_EP\n");
#endif
#endif

    /* Step 3: DBI coherency programming round 1 (PCIE0) */
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 3] PCIE0 RMW: set [11:14],[3:6] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 3] PCIE0 RMW: set [27:30],[19:22] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    /* Step 4: DBI coherency programming round 1 (PCIE1) */
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 4] PCIE1 RMW: set [11:14],[3:6] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 4] PCIE1 RMW: set [27:30],[19:22] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    /* Step 5: wait_on(20) */
    wait_on(20);
#ifdef DEBUG_DISPLAY
    printf("[STEP 5] wait_on(20)\n");
#endif

    /* Step 5/6 continuation: DBI coherency programming round 2 */
    /* PCIE0 */
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 5] PCIE0 RMW round2 all fields to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    /* PCIE1 */
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 6] PCIE1 RMW round2 [11:14],[3:6] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 6] PCIE1 RMW round2 [27:30],[19:22] to 0xF -> 0x%08X\n", rd_wr_data1);
#endif

    /* Step 7: Poll SII0 status */
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1U) != 0xD1U) {
#ifdef DEBUG_DISPLAY
        printf("[STEP 7] SII0 wait: 0x%08X\n", data_rd);
#endif
        data_rd = read_sii0_reg(0xC0);
    }

    /* Step 8: Poll SII1 status */
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1U) != 0xD1U) {
#ifdef DEBUG_DISPLAY
        printf("[STEP 8] SII1 wait: 0x%08X\n", data_rd);
#endif
        data_rd = read_sii1_reg(0xC0);
    }

    /* Step 9: NIC non-secure protection */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    printf("[STEP 9] non_secure_prot_nic()\n");
#endif

    /* Step 10: RC-only vendor and memory base programming */
#if defined(DM0_RC)
    rd_wr_data1 = read_pcie_slv0_reg(0x0);
#ifdef DEBUG_DISPLAY
    printf("[STEP 10] Vendor ID SLV0[0x0]=0x%08X\n", rd_wr_data1);
#endif
    write_pcie_slv0_reg(0x4, 0x7);
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    wait_on(10);
#ifdef DEBUG_DISPLAY
    printf("[STEP 10] write_pcie_slv0_reg(0x4,0x7), mem_base_program_dm0_x4(), mem_base_program_dm1_x4(), wait_on(10)\n");
#endif
#endif

    /* Step 11: System control enables */
    write_reg(0xE690000C, 0x1);
    write_reg(0xE6900010, 0x1);
    write_reg(0xE6900014, 0x1);
    write_reg(0xE6900018, 0x1);
    write_reg(0xE6900030, 0x1);
    write_reg(0xE6900034, 0x1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 11] System enable registers written\n");
#endif

    /* Step 12: Disable cache programming phase 1 */
    /* PCIE0 */
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* PCIE1 */
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 12] Disable cache phase1 programmed on PCIE0/1\n");
#endif

    /* Step 13: wait_on(10), phase 2 */
    wait_on(10);

    /* PCIE0 phase 2 */
    rd_wr_data1 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* PCIE1 phase 2 */
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_data(rd_wr_data1, 11, 14, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF);
    rd_wr_data1 = set_data(rd_wr_data1, 27, 30, 0x0);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[STEP 13] Disable cache phase2 programmed on PCIE0/1\n");
#endif

    /* Step 14: wait_on(30) */
    wait_on(30);
#ifdef DEBUG_DISPLAY
    printf("[STEP 14] wait_on(30)\n");
#endif

    /* Step 15: SLV1 BARs to 0xFFFFFFFF and verify */
    write_pcie_slv1_reg(0x10, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x14, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x18, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x1C, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x20, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x24, 0xFFFFFFFFU);

    if (read_pcie_slv1_reg(0x10) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv1_reg(0x14) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv1_reg(0x18) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv1_reg(0x1C) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv1_reg(0x20) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv1_reg(0x24) != 0xFFFFFFFFU) { g_error_count++; }
#ifdef DEBUG_DISPLAY
    printf("[STEP 15] SLV1 BARs verified to 0xFFFFFFFF, errors=%d\n", g_error_count);
#endif

    /* Step 16: Program SLV1 BARs to target values and verify */
    write_pcie_slv1_reg(0x10, 0x00000000U);
    write_pcie_slv1_reg(0x14, 0x00000004U);
    write_pcie_slv1_reg(0x18, 0x20000000U);
    write_pcie_slv1_reg(0x1C, 0x40000000U);
    write_pcie_slv1_reg(0x20, 0x60000000U);
    write_pcie_slv1_reg(0x24, 0x80000000U);

    if (read_pcie_slv1_reg(0x10) != 0x00000000U) { g_error_count++; }
    if (read_pcie_slv1_reg(0x14) != 0x00000004U) { g_error_count++; }
    if (read_pcie_slv1_reg(0x18) != 0x20000000U) { g_error_count++; }
    if (read_pcie_slv1_reg(0x1C) != 0x40000000U) { g_error_count++; }
    if (read_pcie_slv1_reg(0x20) != 0x60000000U) { g_error_count++; }
    if (read_pcie_slv1_reg(0x24) != 0x80000000U) { g_error_count++; }
#ifdef DEBUG_DISPLAY
    printf("[STEP 16] SLV1 BARs verified to target values, errors=%d\n", g_error_count);
#endif

    /* Step 17: Repeat BAR sequence for SLV0 */
    write_pcie_slv0_reg(0x10, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x14, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x18, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x1C, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x20, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x24, 0xFFFFFFFFU);

    if (read_pcie_slv0_reg(0x10) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv0_reg(0x14) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv0_reg(0x18) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv0_reg(0x1C) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv0_reg(0x20) != 0xFFFFFFFFU) { g_error_count++; }
    if (read_pcie_slv0_reg(0x24) != 0xFFFFFFFFU) { g_error_count++; }
#ifdef DEBUG_DISPLAY
    printf("[STEP 17] SLV0 BARs verified to 0xFFFFFFFF, errors=%d\n", g_error_count);
#endif

    write_pcie_slv0_reg(0x10, 0x00000000U);
    write_pcie_slv0_reg(0x14, 0x00000004U);
    write_pcie_slv0_reg(0x18, 0x20000000U);
    write_pcie_slv0_reg(0x1C, 0x40000000U);
    write_pcie_slv0_reg(0x20, 0x60000000U);
    write_pcie_slv0_reg(0x24, 0x80000000U);

    if (read_pcie_slv0_reg(0x10) != 0x00000000U) { g_error_count++; }
    if (read_pcie_slv0_reg(0x14) != 0x00000004U) { g_error_count++; }
    if (read_pcie_slv0_reg(0x18) != 0x20000000U) { g_error_count++; }
    if (read_pcie_slv0_reg(0x1C) != 0x40000000U) { g_error_count++; }
    if (read_pcie_slv0_reg(0x20) != 0x60000000U) { g_error_count++; }
    if (read_pcie_slv0_reg(0x24) != 0x80000000U) { g_error_count++; }
#ifdef DEBUG_DISPLAY
    printf("[STEP 17] SLV0 BARs verified to target values, errors=%d\n", g_error_count);
#endif

    /* Step 18: wait_on(10); then poll for completion signature */
    wait_on(10);
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678U) {
#ifdef DEBUG_DISPLAY
        printf("[STEP 18] Waiting for 0x12345678, got 0x%08X\n", data_rd);
#endif
        data_rd = read_reg(0xE6004100);
    }
#ifdef DEBUG_DISPLAY
    printf("[STEP 18] Completion signature observed: 0x%08X\n", data_rd);
#endif

    /* Step 19: Finish with PASS/FAIL based on errors */
    if (g_error_count == 0) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] FAIL, errors=%d\n", g_error_count);
#endif
        finish(1);
    }

    return 0;
}
