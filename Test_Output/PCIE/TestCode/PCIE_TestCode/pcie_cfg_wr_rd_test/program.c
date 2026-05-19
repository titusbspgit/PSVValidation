// Author - AI Force 1.3.2. Date 19-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* ---------------------------------------------------------
 * Function: set_field
 * Purpose : Set bits [lsb:msb] in a 32-bit value to the given field
 * --------------------------------------------------------- */
static inline unsigned int set_field(unsigned int value, unsigned int lsb, unsigned int msb, unsigned int field)
{
    unsigned int width = (msb - lsb + 1u);
    unsigned int mask  = ((width >= 32u) ? 0xFFFFFFFFu : ((1u << width) - 1u)) << lsb;
    value &= ~mask;                           /* Clear the target field */
    value |= ((field & ((1u << width) - 1u)) << lsb); /* Set new field value */
    return value;
}

/* ---------------------------------------------------------
 * Function: test_case
 * Purpose : Execute PCIe config write/read and coherency programming
 * --------------------------------------------------------- */
void test_case(void)
{
    unsigned int data_rd = 0u;
    unsigned int rd_wr_data0 = 0u;
    unsigned int rd_wr_data1 = 0u;
    unsigned int i = 0u;
    int test_err = 0; /* No explicit comparison errors expected per spec */

    /* Step 1: Initialize sync MMIO register */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 1] write_reg(0xE6004100, 0x0)\n");
    #endif
    write_reg(0xE6004100, 0x0);

    /* Step 2: Link training calls based on build flags */
    #ifdef DM0_RC
    #ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm0_x4(4) for DM0_RC\n");
    #endif
    link_training_dm0_x4(4);
    #endif

    #ifdef DM1_RC
    #ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm1_x4(4) for DM1_RC\n");
    #endif
    link_training_dm1_x4(4);
    #endif

    #ifdef DM0_EP
    #ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm0_x4(4) for DM0_EP\n");
    #endif
    link_training_dm0_x4(4);
    #endif

    #ifdef DM1_EP
    #ifdef DEBUG_DISPLAY
    printf("[STEP 2] link_training_dm1_x4(4) for DM1_EP\n");
    #endif
    link_training_dm1_x4(4);
    #endif

    /* Step 3: Program coherency control fields on PCIe0 and PCIe1 */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 3] Program coherency fields on PCIe0\n");
    #endif
    rd_wr_data0 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data0 = set_field(rd_wr_data0, 11u, 14u, 0xFu); /* CFG_MSTR_AWCACHE_MODE */
    rd_wr_data0 = set_field(rd_wr_data0, 3u, 6u, 0xFu);   /* CFG_MSTR_ARCACHE_MODE */
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data0);

    rd_wr_data0 = set_field(rd_wr_data0, 27u, 30u, 0xFu); /* [27:30] */
    rd_wr_data0 = set_field(rd_wr_data0, 19u, 22u, 0xFu); /* [19:22] */
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data0);

    #ifdef DEBUG_DISPLAY
    printf("[STEP 3] Program coherency fields on PCIe1\n");
    #endif
    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_field(rd_wr_data1, 11u, 14u, 0xFu);
    rd_wr_data1 = set_field(rd_wr_data1, 3u, 6u, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    rd_wr_data1 = set_field(rd_wr_data1, 27u, 30u, 0xFu);
    rd_wr_data1 = set_field(rd_wr_data1, 19u, 22u, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 4: wait_on(20), then re-apply all four bitfields on both instances */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 4] wait_on(20) and re-apply coherency fields on both instances\n");
    #endif
    wait_on(20);

    rd_wr_data0 = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data0 = set_field(rd_wr_data0, 11u, 14u, 0xFu);
    rd_wr_data0 = set_field(rd_wr_data0, 3u, 6u, 0xFu);
    rd_wr_data0 = set_field(rd_wr_data0, 27u, 30u, 0xFu);
    rd_wr_data0 = set_field(rd_wr_data0, 19u, 22u, 0xFu);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data0);

    rd_wr_data1 = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    rd_wr_data1 = set_field(rd_wr_data1, 11u, 14u, 0xFu);
    rd_wr_data1 = set_field(rd_wr_data1, 3u, 6u, 0xFu);
    rd_wr_data1 = set_field(rd_wr_data1, 27u, 30u, 0xFu);
    rd_wr_data1 = set_field(rd_wr_data1, 19u, 22u, 0xFu);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1);

    /* Step 5: Poll SII status */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 5] Poll SII0 status at 0xC0 for mask 0xD1\n");
    #endif
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1u) != 0xD1u) {
        data_rd = read_sii0_reg(0xC0);
    }

    #ifdef DM1_RC
    #ifdef DEBUG_DISPLAY
    printf("[STEP 5] Poll SII1 status at 0xC0 for mask 0xD1 (DM1_RC)\n");
    #endif
    data_rd = read_sii1_reg(0xC0);
    while ((data_rd & 0xD1u) != 0xD1u) {
        data_rd = read_sii1_reg(0xC0);
    }
    #endif

    /* Step 6: Write handshake MMIO and wait */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 6] write_reg(0xE6004100, 0x11111111) and wait_on(15000)\n");
    #endif
    write_reg(0xE6004100, 0x11111111);
    wait_on(15000);

    /* Step 7: DM0_RC sequence */
    #ifdef DM0_RC
    #ifdef DEBUG_DISPLAY
    printf("[STEP 7] DM0_RC: mem_base_program_dm0_x4(); wait_on(10)\n");
    #endif
    mem_base_program_dm0_x4();
    wait_on(10);

    for (i = 0u; i < 10u; i++) {
        (void)read_pcie_slv0_reg(i * 0x4u);
    }

    /* Write 0xFFFFFFFF to BAR0..BAR5 offsets 0x10..0x24 */
    for (i = 0x10u; i <= 0x24u; i += 4u) {
        write_pcie_slv0_reg(i, 0xFFFFFFFFu);
    }
    /* Read back BAR0..BAR5 */
    for (i = 0x10u; i <= 0x24u; i += 4u) {
        (void)read_pcie_slv0_reg(i);
    }

    /* Program specific BAR values and read back */
    write_pcie_slv0_reg(0x10u, 0x00000000u);
    (void)read_pcie_slv0_reg(0x10u);

    write_pcie_slv0_reg(0x14u, 0x00000004u);
    (void)read_pcie_slv0_reg(0x14u);

    write_pcie_slv0_reg(0x18u, 0x20000000u);
    (void)read_pcie_slv0_reg(0x18u);

    write_pcie_slv0_reg(0x1Cu, 0x40000000u);
    (void)read_pcie_slv0_reg(0x1Cu);

    write_pcie_slv0_reg(0x20u, 0x60000000u);
    (void)read_pcie_slv0_reg(0x20u);

    write_pcie_slv0_reg(0x24u, 0x80000000u);
    (void)read_pcie_slv0_reg(0x24u);

    /* Enable Memory/IO/Bus Master: Command register at 0x4 */
    write_pcie_slv0_reg(0x4u, 0x7u);
    #endif /* DM0_RC */

    /* Step 8: DM1_RC sequence */
    #ifdef DM1_RC
    #ifdef DEBUG_DISPLAY
    printf("[STEP 8] DM1_RC: mem_base_program_dm1_x4(); wait_on(10)\n");
    #endif
    mem_base_program_dm1_x4();
    wait_on(10);

    for (i = 0u; i < 10u; i++) {
        (void)read_pcie_slv1_reg(i * 0x4u);
    }

    for (i = 0x10u; i <= 0x24u; i += 4u) {
        write_pcie_slv1_reg(i, 0xFFFFFFFFu);
    }
    for (i = 0x10u; i <= 0x24u; i += 4u) {
        (void)read_pcie_slv1_reg(i);
    }

    write_pcie_slv1_reg(0x10u, 0x00000000u);
    (void)read_pcie_slv1_reg(0x10u);

    write_pcie_slv1_reg(0x14u, 0x00000004u);
    (void)read_pcie_slv1_reg(0x14u);

    write_pcie_slv1_reg(0x18u, 0x20000000u);
    (void)read_pcie_slv1_reg(0x18u);

    write_pcie_slv1_reg(0x1Cu, 0x40000000u);
    (void)read_pcie_slv1_reg(0x1Cu);

    write_pcie_slv1_reg(0x20u, 0x60000000u);
    (void)read_pcie_slv1_reg(0x20u);

    write_pcie_slv1_reg(0x24u, 0x80000000u);
    (void)read_pcie_slv1_reg(0x24u);

    write_pcie_slv1_reg(0x4u, 0x7u);
    #endif /* DM1_RC */

    /* Step 9: Poll handshake completion at 0xE6004100 */
    #ifdef DEBUG_DISPLAY
    printf("[STEP 9] Poll read_reg(0xE6004100) until 0x12345678\n");
    #endif
    data_rd = read_reg(0xE6004100);
    while (data_rd != 0x12345678u) {
        wait_on(5);
        data_rd = read_reg(0xE6004100);
    }

    /* Step 10: Finish with PASS */
    finish(0);
}
