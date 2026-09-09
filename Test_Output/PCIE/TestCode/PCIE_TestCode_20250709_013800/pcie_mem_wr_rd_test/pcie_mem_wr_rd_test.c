// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case: pcie_mem_wr_rd_test
 */

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
#if defined(DM0_RC) || defined(DM0_EP)
    link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
    link_training_dm1_x4(4);
#endif
    cache_program_enable_pcie0();
    cache_program_enable_pcie1();
    wait_on(20);
    cache_program_enable_combined_pcie0();
    cache_program_enable_combined_pcie1();
#ifdef DM0
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_sii0_reg(0xC0);
    while (((data_rd & 0xD1) != 0xD1) && (timeout > 0U)) {
        wait_on(1);
        data_rd = read_sii0_reg(0xC0);
        timeout--;
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
#endif
#ifdef DM0_EP
    wait_on(30000);
#endif
#ifdef DM0_RC
    data_rd = read_pcie_slv0_reg(0x0);
    write_pcie_slv0_reg(0x4, 0x7);
    bar_program_dm0_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
#endif
#ifdef DM1_RC
    data_rd = read_pcie_slv1_reg(0x0);
    write_pcie_slv1_reg(0x4, 0x7);
    bar_program_dm1_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
#endif
#ifdef DM0_EP
    bar_program_dm0_EP_x4();
    wait_on(10);
    mem_base_program_dm0_x4();
#endif
#ifdef DM1_EP
    bar_program_dm1_EP_x4();
    wait_on(10);
    mem_base_program_dm1_x4();
#endif
    non_secure_prot_nic();
    write_reg(PCIE_SYNC_REG, PCIE_SYNC_READY);
    return 0;
}

int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int data_rd;
    unsigned int timeout;
    (void)cfg;
    if (out == 0) { return -1; }
    out->status = 0;
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 11, 14, 0xf);
    data_rd = set_data(data_rd, 3, 6, 0xf);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0xf);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    wait_on(10);
    data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
    data_rd = set_data(data_rd, 27, 30, 0x0);
    data_rd = set_data(data_rd, 19, 22, 0x0);
    write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
    wait_on(30);
#ifdef DM0_RC
    pcie_slv0_mem_wr_rd(0x01040000, 0xa5a5a5a5);
    pcie_slv0_mem_wr_rd(0x01000020, 0xa6a6a6a6);
    pcie_slv0_mem_wr_rd(0x01004000, 0xa7a7a7a7);
#endif
#ifdef DM1_RC
    pcie_slv1_mem_wr_rd(0x01040000, 0xb5b5b5b5);
    pcie_slv1_mem_wr_rd(0x01000020, 0xb5b5b6b6);
    pcie_slv1_mem_wr_rd(0x01004000, 0xb7b7b5b5);
#endif
#ifdef DM0_EP
    pcie_slv0_mem_wr_rd(0x10100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x20100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    pcie_slv0_mem_wr_rd(0x30100, 0x5a5a5a5a);
#endif
#ifdef DM1_EP
    pcie_slv1_mem_wr_rd(0x10100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x20100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x1B100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x2B100, 0x5a5a5a5a);
    pcie_slv1_mem_wr_rd(0x30100, 0x5a5a5a5a);
#endif
    wait_on(10);
    timeout = PCIE_POLL_TIMEOUT;
    data_rd = read_reg(PCIE_SYNC_REG);
    while ((data_rd != PCIE_SYNC_EXPECTED) && (timeout > 0U)) {
        wait_on(5);
        data_rd = read_reg(PCIE_SYNC_REG);
        timeout--;
    }
    if (timeout == 0U) {
        g_ctx.errors++;
        out->status = -1;
    }
    if (g_ctx.errors == 0U) { out->status = 0; }
    return out->status;
}

int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    if (g_ctx.errors != 0U) {
        LOGE("pcie_mem_wr_rd_test FAILED with %u errors", g_ctx.errors);
    } else {
        LOGT("pcie_mem_wr_rd_test PASSED");
    }
    return g_ctx.errors == 0U ? 0 : -1;
}
