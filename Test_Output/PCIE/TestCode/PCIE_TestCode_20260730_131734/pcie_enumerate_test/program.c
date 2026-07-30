// Author - AI Force 1.3.2. Date 30-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase Entry Point
 * Converts Meta Test Steps / Procedure to executable C code without reordering or optimization.
 */
int test_case(void)
{
    unsigned int errors = 0U;            // Error counter for validation failures
    unsigned int rd_wr_data1 = 0U;       // Scratch register/value variable
    unsigned int data_rd = 0U;           // Scratch for reads / polling
    unsigned int tmp = 0U;               // Temporary variable for readbacks

    // Step 1: Initialize handshake register
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] write_reg(0xE6004100, 0x0)\n");
    #endif
    write_reg(0xE6004100U, 0x0U);

    // Step 2: Board-specific link training (width x4) under build flags
    #ifdef DM0_RC
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] DM0_RC: link_training_dm0_x4(4)\n");
    #endif
    link_training_dm0_x4(4);
    #endif

    #ifdef DM1_RC
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] DM1_RC: link_training_dm1_x4(4)\n");
    #endif
    link_training_dm1_x4(4);
    #endif

    #ifdef DM0_EP
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] DM0_EP: link_training_dm0_x4(4)\n");
    #endif
    link_training_dm0_x4(4);
    #endif

    #ifdef DM1_EP
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] DM1_EP: link_training_dm1_x4(4)\n");
    #endif
    link_training_dm1_x4(4);
    #endif

    // Step 3: CACHE/COHERENCY PROGRAMMING (enable) for PCIE0
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Enable coherency fields for PCIE0\n");
    #endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFU);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFU);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFU);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFU);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    // Step 4: CACHE/COHERENCY PROGRAMMING (enable) for PCIE1
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Enable coherency fields for PCIE1\n");
    #endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFU);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFU);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFU);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFU);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    // Interleave wait as specified
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] wait_on(20) after coherency enable writes\n");
    #endif
    wait_on(20);

    // Step 5: Poll SII0/SII1 status after non-secure NIC programming
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] non_secure_prot_nic(); then poll SII0/SII1 at 0xC0 for (data & 0xD1) == 0xD1\n");
    #endif
    non_secure_prot_nic();

    data_rd = read_sii0_reg(0xC0U);
    while (((data_rd) & 0xD1U) != 0xD1U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test] Polling SII0: data=0x%08X\n", data_rd);
        #endif
        data_rd = read_sii0_reg(0xC0U);
    }

    data_rd = read_sii1_reg(0xC0U);
    while (((data_rd) & 0xD1U) != 0xD1U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test] Polling SII1: data=0x%08X\n", data_rd);
        #endif
        data_rd = read_sii1_reg(0xC0U);
    }

    // Step 6: DM0_RC specific configuration and memory base programming
    #ifdef DM0_RC
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] DM0_RC: Read Vendor ID, enable Command, program memory base, wait_on(10)\n");
    #endif
    rd_wr_data1 = read_pcie_slv0_reg(0x0U);   // Vendor ID read
    (void)rd_wr_data1; // value not validated per Meta Acceptance
    write_pcie_slv0_reg(0x4U, 0x7U);          // Command: enable MEM/BME
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
    wait_on(10);
    #endif

    // Step 7: Toggle specified control registers with 0x1
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Toggle control regs with 0x1 (0xE690000C..0xE6900034)\n");
    #endif
    write_reg(0xE690000CU, 0x1U);
    write_reg(0xE6900010U, 0x1U);
    write_reg(0xE6900014U, 0x1U);
    write_reg(0xE6900018U, 0x1U);
    write_reg(0xE6900030U, 0x1U);
    write_reg(0xE6900034U, 0x1U);

    // Step 8: DISABLE CACHE PROGRAMMING (details not specified in Meta; honor the wait only)
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Disable coherency programming (fields unspecified), then wait_on(30)\n");
    #endif
    wait_on(30);

    // Step 9: BAR probing and programming for pcie_slv1
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] BAR probe for pcie_slv1: write 0xFFFFFFFF to 0x10..0x24, then read back\n");
    #endif
    write_pcie_slv1_reg(0x10U, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x14U, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x18U, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x1CU, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x20U, 0xFFFFFFFFU);
    write_pcie_slv1_reg(0x24U, 0xFFFFFFFFU);

    tmp = read_pcie_slv1_reg(0x10U);
    tmp = read_pcie_slv1_reg(0x14U);
    tmp = read_pcie_slv1_reg(0x18U);
    tmp = read_pcie_slv1_reg(0x1CU);
    tmp = read_pcie_slv1_reg(0x20U);
    tmp = read_pcie_slv1_reg(0x24U);
    (void)tmp; // probe values are not validated per Meta Acceptance

    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Program BARs for pcie_slv1 and read back\n");
    #endif
    write_pcie_slv1_reg(0x10U, 0x0U);
    write_pcie_slv1_reg(0x14U, 0x4U);
    write_pcie_slv1_reg(0x18U, 0x20000000U);
    write_pcie_slv1_reg(0x1CU, 0x40000000U);
    write_pcie_slv1_reg(0x20U, 0x60000000U);
    write_pcie_slv1_reg(0x24U, 0x80000000U);

    // Readback and validate 0x10 and 0x14 as per Acceptance Criteria
    data_rd = read_pcie_slv1_reg(0x10U);
    if (data_rd != 0x0U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test][ERROR] pcie_slv1 BAR0 (0x10) readback 0x%08X != 0x00000000\n", data_rd);
        #endif
        errors++;
    }
    data_rd = read_pcie_slv1_reg(0x14U);
    if (data_rd != 0x4U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test][ERROR] pcie_slv1 BAR1 (0x14) readback 0x%08X != 0x00000004\n", data_rd);
        #endif
        errors++;
    }

    // Step 10: Repeat BAR probing and programming for pcie_slv0, then validate
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] BAR probe for pcie_slv0: write 0xFFFFFFFF to 0x10..0x24, then read back\n");
    #endif
    write_pcie_slv0_reg(0x10U, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x14U, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x18U, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x1CU, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x20U, 0xFFFFFFFFU);
    write_pcie_slv0_reg(0x24U, 0xFFFFFFFFU);

    tmp = read_pcie_slv0_reg(0x10U);
    tmp = read_pcie_slv0_reg(0x14U);
    tmp = read_pcie_slv0_reg(0x18U);
    tmp = read_pcie_slv0_reg(0x1CU);
    tmp = read_pcie_slv0_reg(0x20U);
    tmp = read_pcie_slv0_reg(0x24U);
    (void)tmp; // probe values are not validated per Meta Acceptance

    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Program BARs for pcie_slv0 and read back\n");
    #endif
    write_pcie_slv0_reg(0x10U, 0x0U);
    write_pcie_slv0_reg(0x14U, 0x4U);
    write_pcie_slv0_reg(0x18U, 0x20000000U);
    write_pcie_slv0_reg(0x1CU, 0x40000000U);
    write_pcie_slv0_reg(0x20U, 0x60000000U);
    write_pcie_slv0_reg(0x24U, 0x80000000U);

    // Readback and validate 0x10 and 0x14 as per Acceptance Criteria
    data_rd = read_pcie_slv0_reg(0x10U);
    if (data_rd != 0x0U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test][ERROR] pcie_slv0 BAR0 (0x10) readback 0x%08X != 0x00000000\n", data_rd);
        #endif
        errors++;
    }
    data_rd = read_pcie_slv0_reg(0x14U);
    if (data_rd != 0x4U) {
        #ifdef DEBUG_DISPLAY
        printf("[pcie_enumerate_test][ERROR] pcie_slv0 BAR1 (0x14) readback 0x%08X != 0x00000004\n", data_rd);
        #endif
        errors++;
    }

    // Step 11: Wait before final handshake poll
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] wait_on(10) before final handshake poll\n");
    #endif
    wait_on(10);

    // Step 12: Final handshake poll until sentinel value observed
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Poll 0xE6004100 until 0x12345678\n");
    #endif
    while (read_reg(0xE6004100U) != 0x12345678U) {
        wait_on(5);
    }

    // Termination according to validation result
    #ifdef DEBUG_DISPLAY
    printf("[pcie_enumerate_test] Test completed with errors=%u\n", errors);
    #endif
    if (errors > 0U) {
        finish(1);
    } else {
        finish(0);
    }
}
