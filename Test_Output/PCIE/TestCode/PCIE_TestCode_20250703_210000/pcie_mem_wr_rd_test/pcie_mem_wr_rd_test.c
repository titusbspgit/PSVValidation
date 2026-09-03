// Author - AI Force 2.3. 03-Jul-2025 21:00 IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int data_wr;
unsigned int test_err;

int pcie_mem_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe mem wr rd test: %s\n", cfg->test_name);
    write_reg(0xE6004100, 0x0);
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
    return 0;
}

int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    test_err = 0;
    LOGI("[Test Run] PCIe mem wr rd test\n");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1) { wait_on(10); data_rd = read_sii0_reg(0xC0); }
    #ifdef DM0
        data_wr = 0xDEADBEEF;
        write_pcie_slv0_reg(0x0, data_wr);
        data_rd = read_pcie_slv0_reg(0x0);
        if (data_rd != data_wr) { test_err++; }
    #endif
    finish(0);
    return out->status = test_err;
}

int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    return 0;
}
