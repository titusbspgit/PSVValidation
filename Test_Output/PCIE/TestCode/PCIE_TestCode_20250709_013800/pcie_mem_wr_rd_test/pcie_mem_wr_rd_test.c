// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case: pcie_mem_wr_rd_test
 * Description: Validates PCIe memory write and read operations through the PCIe
 *   slave interfaces. Performs link training, cache enable/disable programming,
 *   BAR/memory base setup, and memory write/read verification under DM0_RC,
 *   DM1_RC, DM0_EP, and DM1_EP compile-time modes.
 */

/* Testcase context structure */
typedef struct {
    unsigned int errors;
} pcie_mem_wr_rd_test_ctx_t;

static pcie_mem_wr_rd_test_ctx_t g_ctx;

static void cache_program_enable_pcie0(void)
{
    unsigned int data_rd;
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

static void cache_program_enable_pcie1(void)
{
    unsigned int data_rd;
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

static void cache_program_enable_combined_pcie0(void)
{
    unsigned int data_rd;
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

static void cache_program_enable_combined_pcie1(void)
{
    unsigned int data_rd;
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
}

int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    unsigned int data_rd;
    unsigned int timeout;
    (void)cfg;
    g_ctx = (pcie_mem_wr_rd_test_ctx_t){0};
    LOGT("pcie_mem_wr_rd_test init: starting PCIe memory write/read test setup");
    write_reg(PCIE_SYNC_REG, 0x0);
    LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
    LOGT("PCIe link training DM0 x4 initiated");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
    LOGT("PCIe link training DM1 x4 initiated");
#endif
    cache_program_enable_pcie0();
    cache_program_enable_pcie1();
    wait_on(20);
    LOGT("wait_on(20) complete");
    cache_program_enable_combined_pcie0();
    cache_program_enable_combined_pcie1();
    data_rd = read_sii0_reg(0xC0);
    LOGT("Initial read_sii0_reg(0xC0) = 0x%x", data_rd);
#ifdef DM0
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling read_sii0_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("DM0 link status OK: read_sii0_reg(0xC0)=0x%x", data_rd);
    }
#endif
#ifdef DM1
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii1_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii1_reg(0xC0);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling read_sii1_reg(0xC0), data_rd=0x%x", data_rd);
        g_ctx.errors++;
    } else {
        LOGT("DM1 link status OK: read_sii1_reg(0xC0)=0x%x", data_rd);
    }
#endif
#ifdef DM0_EP
    wait_on(30000);
    LOGT("DM0_EP wait_on(30000) complete");
#endif
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(0x0);
    LOGT("DM0_RC Vendor ID from pcie_slv0 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv0_reg(0x4, 0x7);
    LOGT("DM0_RC command register written: pcie_slv0 reg 0x4 = 0x7");
    bar_program_dm0_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("DM0_RC BAR and memory base programming complete");
#endif
#ifdef DM1_RC
    data_rd = read_pcie_slv1_reg(0x0);
    LOGT("DM1_RC Vendor ID from pcie_slv1 reg 0x0 = 0x%x", data_rd);
    write_pcie_slv1_reg(0x4, 0x7);
    LOGT("DM1_RC command register written: pcie_slv1 reg 0x4 = 0x7");
    bar_program_dm1_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
    LOGT("DM1_RC BAR and memory base programming complete");
#endif
#ifdef DM0_EP
    bar_program_dm0_EP_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
    LOGT("DM0_EP BAR and memory base programming complete");
#endif
#ifdef DM1_EP
    bar_program_dm1_EP_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
    LOGT("DM1_EP BAR and memory base programming complete");
#endif
    non_secure_prot_nic();
    LOGT("non_secure_prot_nic() called");
    write_reg(PCIE_SYNC_REG, PCIE_SYNC_READY);
    LOGT("Wrote 0x%lx to sync register 0x%lx", (unsigned long)PCIE_SYNC_READY, (unsigned long)PCIE_SYNC_REG);
    LOGT("pcie_mem_wr_rd_test init complete");
    return 0;
}

int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int data_rd;
    unsigned int timeout;
    (void)cfg;
    if (out == 0) {
        LOGE("pcie_mem_wr_rd_test output pointer is NULL");
        return -1;
    }
    out->status = 0;
    LOGT("pcie_mem_wr_rd_test run: starting main execution");
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Cache disable programming for PCIE0 complete");
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Cache disable programming for PCIE1 complete");
    wait_on(10);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    LOGT("Combined cache disable for PCIE0 and PCIE1 complete");
    wait_on(30);
    LOGT("wait_on(30) complete");
#ifdef DM0_RC
    pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
    LOGT("DM0_RC pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5) complete");
    pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
    LOGT("DM0_RC pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6) complete");
    pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
    LOGT("DM0_RC pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7) complete");
#endif
#ifdef DM1_RC
    pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
    LOGT("DM1_RC pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5) complete");
    pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
    LOGT("DM1_RC pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6) complete");
    pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
    LOGT("DM1_RC pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5) complete");
#endif
#ifdef DM0_EP
    pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a);
    LOGT("DM0_EP pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a) complete");
    pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a);
    LOGT("DM0_EP pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a) complete");
    pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    LOGT("DM0_EP pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a) complete");
    pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    LOGT("DM0_EP pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a) complete");
    pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a);
    LOGT("DM0_EP pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a) complete");
#endif
#ifdef DM1_EP
    pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a);
    LOGT("DM1_EP pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a) complete");
    pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a);
    LOGT("DM1_EP pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a) complete");
    pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    LOGT("DM1_EP pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a) complete");
    pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    LOGT("DM1_EP pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a) complete");
    pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a);
    LOGT("DM1_EP pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a) complete");
#endif
    wait_on(10);
    LOGT("wait_on(10) complete after memory operations");
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_reg(PCIE_SYNC_REG);
    while ((data_rd != PCIE_SYNC_EXPECTED) && (timeout > 0U)) {
        wait_on(5);
        data_rd = read_reg(PCIE_SYNC_REG);
        timeout--;
    }
    if (timeout == 0U) {
        LOGE("Timeout polling sync register 0x%lx, data_rd=0x%x, expected=0x%x", (unsigned long)PCIE_SYNC_REG, data_rd, PCIE_SYNC_EXPECTED);
        g_ctx.errors++;
        out->status = -1;
    } else {
        LOGT("Sync register 0x%lx matched expected 0x%x", (unsigned long)PCIE_SYNC_REG, PCIE_SYNC_EXPECTED);
    }
    if (g_ctx.errors == 0U) {
        out->status = 0;
    }
    LOGT("Run complete: %s errors=%u", (out->status == 0) ? "PASS" : "FAIL", g_ctx.errors);
    return out->status;
}

int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGT("pcie_mem_wr_rd_test teardown: errors=%u", g_ctx.errors);
    if (g_ctx.errors != 0U) {
        LOGE("pcie_mem_wr_rd_test FAILED with %u errors", g_ctx.errors);
    } else {
        LOGT("pcie_mem_wr_rd_test PASSED");
    }
    return g_ctx.errors == 0U ? 0 : -1;
}
