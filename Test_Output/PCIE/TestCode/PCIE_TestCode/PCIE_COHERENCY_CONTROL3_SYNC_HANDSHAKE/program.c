// Author - AI Force 1.3.2. Date 19-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* ------------------------------------------------------------
 * Function: set_data
 * Purpose : Helper to set a bitfield [lsb:msb] in a 32-bit value
 * ------------------------------------------------------------ */
static inline unsigned int set_data(unsigned int val, unsigned int lsb, unsigned int msb, unsigned int field)
{
    unsigned int mask;
    if (msb < lsb) {
        return val; /* no-op if invalid range */
    }
    /* Build mask for the field width */
    if ((msb - lsb + 1u) >= 32u) {
        mask = 0xFFFFFFFFu;
    } else {
        mask = ((1u << (msb - lsb + 1u)) - 1u) << lsb;
    }
    val &= ~mask;                     /* clear field */
    val |= ((field << lsb) & mask);   /* set field */
    return val;
}

/* ------------------------------------------------------------
 * Function: program_coherency_fields
 * Purpose : Program coherency/cache fields on a given register
 * Notes   : Applies bitfields [11:14], [3:6] then [27:30], [19:22]
 * ------------------------------------------------------------ */
static void program_coherency_fields(unsigned int reg_addr)
{
    unsigned int rd_wr_data1;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Programming coherency fields at 0x%08X\n", reg_addr);
#endif

    /* First sequence: set [11:14] and [3:6] to 0xF, then write back */
    rd_wr_data1 = read_reg(reg_addr);
    rd_wr_data1 = set_data(rd_wr_data1, 11u, 14u, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 3u, 6u, 0xFu);
    write_reg(reg_addr, rd_wr_data1);

    /* Next, set [27:30] and [19:22] to 0xF, then write back */
    rd_wr_data1 = set_data(rd_wr_data1, 27u, 30u, 0xFu);
    rd_wr_data1 = set_data(rd_wr_data1, 19u, 22u, 0xFu);
    write_reg(reg_addr, rd_wr_data1);
}

/* ------------------------------------------------------------
 * Function: consolidate_coherency_fields
 * Purpose : Re-apply all four bitfields to 0xF and write back
 * ------------------------------------------------------------ */
static void consolidate_coherency_fields(unsigned int reg_addr)
{
    unsigned int v = read_reg(reg_addr);
    v = set_data(v, 11u, 14u, 0xFu);
    v = set_data(v, 3u, 6u, 0xFu);
    v = set_data(v, 27u, 30u, 0xFu);
    v = set_data(v, 19u, 22u, 0xFu);
    write_reg(reg_addr, v);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Consolidated coherency fields at 0x%08X => 0x%08X\n", reg_addr, v);
#endif
}

/* ------------------------------------------------------------
 * Function: main
 * Purpose : Execute Meta Test Steps / Procedure in exact order
 * ------------------------------------------------------------ */
int main(void)
{
    unsigned int i;
    unsigned int data_rd;
    unsigned int err = 0u; /* Error counter (not used for gating per Meta) */

    /* 1) Initialize sync MMIO register */
#ifdef DEBUG_DISPLAY
    printf("[STEP 1] write_reg(0xE6004100, 0x0)\n");
#endif
    write_reg(0xE6004100u, 0x0u);

    /* 2) Conditional link training calls based on build flags */
#ifdef DEBUG_DISPLAY
    printf("[STEP 2] Link training based on role defines\n");
#endif
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

    /* 3) Program coherency settings for PCIe0 then PCIe1 */
#ifdef DEBUG_DISPLAY
    printf("[STEP 3] Program coherency fields for PCIe0 and PCIe1\n");
#endif
    /* PCIe0 */
    program_coherency_fields(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    /* PCIe1 */
    program_coherency_fields(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* 4) wait_on(20); then consolidate fields on both */
#ifdef DEBUG_DISPLAY
    printf("[STEP 4] wait_on(20) then consolidate coherency fields\n");
#endif
    wait_on(20);
    consolidate_coherency_fields(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    consolidate_coherency_fields(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);

    /* 5) Poll SII0 (and SII1 when DM1_RC) for readiness with mask 0xD1 */
#ifdef DEBUG_DISPLAY
    printf("[STEP 5] Poll SII status registers with mask 0xD1\n");
#endif
    data_rd = read_sii0_reg(0xC0u);
    while ((data_rd & 0xD1u) != 0xD1u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII0 status=0x%02X (waiting for 0xD1)\n", (unsigned)(data_rd & 0xFFu));
#endif
        data_rd = read_sii0_reg(0xC0u);
    }
#ifdef DM1_RC
    data_rd = read_sii1_reg(0xC0u);
    while ((data_rd & 0xD1u) != 0xD1u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII1 status=0x%02X (waiting for 0xD1)\n", (unsigned)(data_rd & 0xFFu));
#endif
        data_rd = read_sii1_reg(0xC0u);
    }
#endif

    /* 6) Handshake write to sync register and delay */
#ifdef DEBUG_DISPLAY
    printf("[STEP 6] write_reg(0xE6004100, 0x11111111) and wait_on(15000)\n");
#endif
    write_reg(0xE6004100u, 0x11111111u);
    wait_on(15000);

    /* 7) DM0_RC config space read/write sequence */
#ifdef DM0_RC
#ifdef DEBUG_DISPLAY
    printf("[STEP 7:DM0_RC] mem_base_program_dm0_x4, read first 10 dwords, program BARs, enable CMD\n");
#endif
    mem_base_program_dm0_x4();
    wait_on(10);
    for (i = 0u; i < 10u; i++) {
        (void)read_pcie_slv0_reg(i * 0x4u);
    }
    /* Write all BARs to 0xFFFFFFFF then read back */
    write_pcie_slv0_reg(0x10u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x14u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x18u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x1Cu, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x20u, 0xFFFFFFFFu);
    write_pcie_slv0_reg(0x24u, 0xFFFFFFFFu);

    (void)read_pcie_slv0_reg(0x10u);
    (void)read_pcie_slv0_reg(0x14u);
    (void)read_pcie_slv0_reg(0x18u);
    (void)read_pcie_slv0_reg(0x1Cu);
    (void)read_pcie_slv0_reg(0x20u);
    (void)read_pcie_slv0_reg(0x24u);

    /* Program specific BAR values then read back */
    write_pcie_slv0_reg(0x10u, 0x00000000u);
    write_pcie_slv0_reg(0x14u, 0x00000004u);
    write_pcie_slv0_reg(0x18u, 0x20000000u);
    write_pcie_slv0_reg(0x1Cu, 0x40000000u);
    write_pcie_slv0_reg(0x20u, 0x60000000u);
    write_pcie_slv0_reg(0x24u, 0x80000000u);

    (void)read_pcie_slv0_reg(0x10u);
    (void)read_pcie_slv0_reg(0x14u);
    (void)read_pcie_slv0_reg(0x18u);
    (void)read_pcie_slv0_reg(0x1Cu);
    (void)read_pcie_slv0_reg(0x20u);
    (void)read_pcie_slv0_reg(0x24u);

    /* Enable Memory/IO/Bus Master: Command register at 0x4 */
    write_pcie_slv0_reg(0x4u, 0x7u);
#endif

    /* 8) DM1_RC analogous sequence */
#ifdef DM1_RC
#ifdef DEBUG_DISPLAY
    printf("[STEP 8:DM1_RC] mem_base_program_dm1_x4, read first 10 dwords, program BARs, enable CMD\n");
#endif
    mem_base_program_dm1_x4();
    wait_on(10);
    for (i = 0u; i < 10u; i++) {
        (void)read_pcie_slv1_reg(i * 0x4u);
    }
    /* Write all BARs to 0xFFFFFFFF then read back */
    write_pcie_slv1_reg(0x10u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x14u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x18u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x1Cu, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x20u, 0xFFFFFFFFu);
    write_pcie_slv1_reg(0x24u, 0xFFFFFFFFu);

    (void)read_pcie_slv1_reg(0x10u);
    (void)read_pcie_slv1_reg(0x14u);
    (void)read_pcie_slv1_reg(0x18u);
    (void)read_pcie_slv1_reg(0x1Cu);
    (void)read_pcie_slv1_reg(0x20u);
    (void)read_pcie_slv1_reg(0x24u);

    /* Program specific BAR values then read back */
    write_pcie_slv1_reg(0x10u, 0x00000000u);
    write_pcie_slv1_reg(0x14u, 0x00000004u);
    write_pcie_slv1_reg(0x18u, 0x20000000u);
    write_pcie_slv1_reg(0x1Cu, 0x40000000u);
    write_pcie_slv1_reg(0x20u, 0x60000000u);
    write_pcie_slv1_reg(0x24u, 0x80000000u);

    (void)read_pcie_slv1_reg(0x10u);
    (void)read_pcie_slv1_reg(0x14u);
    (void)read_pcie_slv1_reg(0x18u);
    (void)read_pcie_slv1_reg(0x1Cu);
    (void)read_pcie_slv1_reg(0x20u);
    (void)read_pcie_slv1_reg(0x24u);

    /* Enable Memory/IO/Bus Master: Command register at 0x4 */
    write_pcie_slv1_reg(0x4u, 0x7u);
#endif

    /* 9) Poll 0xE6004100 for completion value 0x12345678 with periodic wait_on(5) */
#ifdef DEBUG_DISPLAY
    printf("[STEP 9] Poll 0xE6004100 for 0x12345678 with periodic wait_on(5)\n");
#endif
    data_rd = read_reg(0xE6004100u);
    while (data_rd != 0x12345678u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Sync MMIO=0x%08X (waiting for 0x12345678)\n", data_rd);
#endif
        wait_on(5);
        data_rd = read_reg(0xE6004100u);
    }

    /* 10) Finish with PASS */
#ifdef DEBUG_DISPLAY
    printf("[STEP 10] finish(0)\n");
#endif
    finish(0);

    /* Unreachable, but retain structure */
    return 0;
}
