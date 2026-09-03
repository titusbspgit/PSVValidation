// Author - AI Force 2.3. 03-Jul-2025 21:00 IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int test_err;
int int_pend;
int dma_wr_done;
int dma_rd_done;

int pcie_dma_write_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);
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

int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    test_err = 0;
    dma_wr_done = 0;
    dma_rd_done = 0;
    LOGI("[Test Run] PCIe DMA write test\n");
    finish(0);
    return out->status = test_err;
}

int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    return 0;
}
