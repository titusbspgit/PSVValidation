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
    LOGI("[Init] Control register 0xE6004100 cleared to 0x0\n");
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
    LOGI("[Test Run] PCIe DMA write test\n");
    data_rd = read_sii0_reg(0xC0);
    while ((data_rd & 0xD1) != 0xD1) { wait_on(10); data_rd = read_sii0_reg(0xC0); }
    write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
    write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
    finish(0);
    return out->status = test_err;
}

void Default_IRQHandler(void)
{
    unsigned int wr_status;
    int_pend = 0;
    wr_status = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
    if (wr_status != 0x0) { write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_status); }
    #ifdef DM0_RC
        GIC_ClearIRQ(0);
    #endif
}

int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    return 0;
}
