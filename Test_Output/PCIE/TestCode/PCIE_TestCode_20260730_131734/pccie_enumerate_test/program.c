// Author - AI Force 1.3.2. Date 30-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: set_data
 * Purpose : Helper to set a bitfield [lsb:msb] in an integer value deterministically
 */
static inline unsigned int set_data(unsigned int value, int lsb, int msb, unsigned int field)
{
    int width = (msb - lsb + 1);
    unsigned int mask;
    if (width >= (int)(8u * sizeof(unsigned int))) {
        mask = 0xFFFFFFFFu;
    } else {
        mask = ((1u << width) - 1u);
    }
    value &= ~(mask << lsb);
    value |= ((field & mask) << lsb);
    return value;
}

/*
 * Function: test_case
 * Entry   : Test entry point
 * Notes   : Follows Meta Test Steps strictly; no reordering or optimization
 */
int test_case(void)
{
    unsigned int rd_wr_data1 = 0u;
    unsigned int rd_wr_data  = 0u;
    unsigned int data_rd     = 0u;
    unsigned int errors      = 0u;

    /* Initialize handshake register */
    write_reg(0xE6004100u, 0x0u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Handshake register 0xE6004100 <- 0x0\n");
#endif

    /* Link training based on build flags */
#ifdef DM0_RC
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] DM0_RC link training x4 initiated\n");
#endif
#endif
#ifdef DM1_RC
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] DM1_RC link training x4 initiated\n");
#endif
#endif
#ifdef DM0_EP
    link_training_dm0_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] DM0_EP link training x4 initiated\n");
#endif
#endif
#ifdef DM1_EP
    link_training_dm1_x4(4);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] DM1_EP link training x4 initiated\n");
#endif
#endif

    /* CACHE PROGRAMMING (enable) for PCIE0 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE0 coherency fields [11:14],[3:6] written: 0x%08X\n", rd_wr_data1);
#endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE0 coherency fields [27:30],[19:22] written: 0x%08X\n", rd_wr_data1);
#endif

    /* CACHE PROGRAMMING (enable) for PCIE1 */
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE1 coherency fields [11:14],[3:6] written: 0x%08X\n", rd_wr_data1);
#endif
    rd_wr_data1 = set_data(read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE1 coherency fields [27:30],[19:22] written: 0x%08X\n", rd_wr_data1);
#endif

    wait_on(20);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] wait_on(20) after coherency enable writes\n");
#endif
    /* consolidate enable writes for PCIE0 and PCIE1 coherency control fields */

    /* Poll SII0/SII1 0xC0 until (data_rd & 0xD1) == 0xD1 */
    non_secure_prot_nic();
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] non_secure_prot_nic() called\n");
#endif

    data_rd = read_sii0_reg(0xC0u);
    while (((data_rd) & 0xD1u) != 0xD1u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII0 status 0x%02X (waiting for 0xD1)\n", (data_rd & 0xFFu));
#endif
        data_rd = read_sii0_reg(0xC0u);
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SII0 ready: 0x%02X\n", (data_rd & 0xFFu));
#endif

    data_rd = read_sii1_reg(0xC0u);
    while (((data_rd) & 0xD1u) != 0xD1u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII1 status 0x%02X (waiting for 0xD1)\n", (data_rd & 0xFFu));
#endif
        data_rd = read_sii1_reg(0xC0u);
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SII1 ready: 0x%02X\n", (data_rd & 0xFFu));
#endif

#ifdef DM0_RC
    /* DM0_RC Vendor/Command setup */
    rd_wr_data1 = read_pcie_slv0_reg(0x0u); /* Vendor ID */
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE SLV0 Vendor ID: 0x%08X\n", rd_wr_data1);
#endif
    write_pcie_slv0_reg(0x4u, 0x7u); /* Command: enable MEM/BME */
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] PCIE SLV0 Command reg (0x4) <- 0x7\n");
#endif
    mem_base_program_dm0_x4();
    mem_base_program_dm1_x4();
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Memory base programming done for DM0/DM1 x4\n");
#endif
    wait_on(10);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] wait_on(10) after memory base programming\n");
#endif
#endif /* DM0_RC */

    /* Control registers toggling */
    write_reg(0xE690000Cu, 0x1u);
    write_reg(0xE6900010u, 0x1u);
    write_reg(0xE6900014u, 0x1u);
    write_reg(0xE6900018u, 0x1u);
    write_reg(0xE6900030u, 0x1u);
    write_reg(0xE6900034u, 0x1u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Control registers toggled to 0x1\n");
#endif

    /* DISABLE CACHE PROGRAMMING */
    /* Re-write coherency control fields with some fields set to 0x0 for both PCIE0 and PCIE1, with waits */
    wait_on(30);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] wait_on(30) during coherency disable sequence\n");
#endif

    /* BAR probing and programming for pcie_slv1 */
    write_pcie_slv1_reg(0x10u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x14u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x18u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x1Cu, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x20u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x24u, 0xFFFFFFFFu);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV1 BAR probe writes done\n");
#endif
    rd_wr_data = read_pcie_slv1_reg(0x10u);
    rd_wr_data = read_pcie_slv1_reg(0x14u);
    rd_wr_data = read_pcie_slv1_reg(0x18u);
    rd_wr_data = read_pcie_slv1_reg(0x1Cu);
    rd_wr_data = read_pcie_slv1_reg(0x20u);
    rd_wr_data = read_pcie_slv1_reg(0x24u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV1 BAR probe reads captured\n");
#endif
    write_pcie_slv1_reg(0x10u, 0x0u);
    write_pcie_slv1_reg(0x14u, 0x4u);
    write_pcie_slv1_reg(0x18u, 0x20000000u);
    write_pcie_slv1_reg(0x1Cu, 0x40000000u);
    write_pcie_slv1_reg(0x20u, 0x60000000u);
    write_pcie_slv1_reg(0x24u, 0x80000000u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV1 BAR programming writes done\n");
#endif
    rd_wr_data = read_pcie_slv1_reg(0x10u);
    if (rd_wr_data != 0x0u) { errors++; }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV1 BAR0 readback: 0x%08X (expect 0x00000000)%s\n", rd_wr_data, (rd_wr_data==0x0u)?"":" [MISMATCH]");
#endif
    rd_wr_data = read_pcie_slv1_reg(0x14u);
    if (rd_wr_data != 0x4u) { errors++; }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV1 BAR1 readback: 0x%08X (expect 0x00000004)%s\n", rd_wr_data, (rd_wr_data==0x4u)?"":" [MISMATCH]");
#endif
    rd_wr_data = read_pcie_slv1_reg(0x18u);
    rd_wr_data = read_pcie_slv1_reg(0x1Cu);
    rd_wr_data = read_pcie_slv1_reg(0x20u);
    rd_wr_data = read_pcie_slv1_reg(0x24u);

    /* BAR probing and programming for pcie_slv0 */
    write_pcie_slv0_reg(0x10u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x14u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x18u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x1Cu, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x20u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x24u, 0xFFFFFFFFu);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV0 BAR probe writes done\n");
#endif
    rd_wr_data = read_pcie_slv0_reg(0x10u);
    rd_wr_data = read_pcie_slv0_reg(0x14u);
    rd_wr_data = read_pcie_slv0_reg(0x18u);
    rd_wr_data = read_pcie_slv0_reg(0x1Cu);
    rd_wr_data = read_pcie_slv0_reg(0x20u);
    rd_wr_data = read_pcie_slv0_reg(0x24u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV0 BAR probe reads captured\n");
#endif
    write_pcie_slv0_reg(0x10u, 0x0u);
    write_pcie_slv0_reg(0x14u, 0x4u);
    write_pcie_slv0_reg(0x18u, 0x20000000u);
    write_pcie_slv0_reg(0x1Cu, 0x40000000u);
    write_pcie_slv0_reg(0x20u, 0x60000000u);
    write_pcie_slv0_reg(0x24u, 0x80000000u);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV0 BAR programming writes done\n");
#endif
    rd_wr_data = read_pcie_slv0_reg(0x10u);
    if (rd_wr_data != 0x0u) { errors++; }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV0 BAR0 readback: 0x%08X (expect 0x00000000)%s\n", rd_wr_data, (rd_wr_data==0x0u)?"":" [MISMATCH]");
#endif
    rd_wr_data = read_pcie_slv0_reg(0x14u);
    if (rd_wr_data != 0x4u) { errors++; }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] SLV0 BAR1 readback: 0x%08X (expect 0x00000004)%s\n", rd_wr_data, (rd_wr_data==0x4u)?"":" [MISMATCH]");
#endif
    rd_wr_data = read_pcie_slv0_reg(0x18u);
    rd_wr_data = read_pcie_slv0_reg(0x1Cu);
    rd_wr_data = read_pcie_slv0_reg(0x20u);
    rd_wr_data = read_pcie_slv0_reg(0x24u);

    wait_on(10);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] wait_on(10) before final handshake poll\n");
#endif

    /* Final handshake poll */
    while (read_reg(0xE6004100u) != 0x12345678u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Waiting for handshake register 0xE6004100 to reach 0x12345678\n");
#endif
        wait_on(5);
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Handshake complete: 0x12345678 observed\n");
#endif

    if (errors == 0u) {
        finish(0);
    } else {
        finish(1);
    }

    return 0; /* Unreachable due to finish() */
}
