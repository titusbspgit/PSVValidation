// Author - AI Force 2.3. 03-Jul-2025 21:00 IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd;
unsigned int err1;
unsigned int err2;

void chk_rst_val(unsigned int addr, unsigned int exp_val, unsigned int *err)
{
    unsigned int rd_val;
    rd_val = read_reg(addr);
    if (rd_val != exp_val)
    {
        LOGI("ERROR: chk_rst_val addr=0x%x exp=0x%x got=0x%x\n", addr, exp_val, rd_val);
        (*err)++;
    }
    else
    {
        LOGI("PASS: chk_rst_val addr=0x%x val=0x%x\n", addr, rd_val);
    }
}

void chk_rst_val_phy(unsigned int addr, unsigned int exp_val, unsigned int *err)
{
    unsigned int rd_val;
    rd_val = read_reg(addr);
    if ((addr & 0x2) == 0x2) { rd_val = (rd_val >> 16) & 0xFFFF; }
    else { rd_val = rd_val & 0xFFFF; }
    if (rd_val != exp_val)
    {
        LOGI("ERROR: chk_rst_val_phy addr=0x%x exp=0x%x got=0x%x\n", addr, exp_val, rd_val);
        (*err)++;
    }
    else
    {
        LOGI("PASS: chk_rst_val_phy addr=0x%x val=0x%x\n", addr, rd_val);
    }
}

void chk_rd_wr(unsigned int addr, unsigned int wr_val, unsigned int write_mask, unsigned int *err)
{
    unsigned int rd_val;
    unsigned int exp_val;
    write_reg(addr, wr_val);
    rd_val = read_reg(addr);
    exp_val = wr_val & write_mask;
    if (rd_val != exp_val)
    {
        LOGI("ERROR: chk_rd_wr addr=0x%x wrote=0x%x exp=0x%x got=0x%x\n", addr, wr_val, exp_val, rd_val);
        (*err)++;
    }
    else
    {
        LOGI("PASS: chk_rd_wr addr=0x%x val=0x%x\n", addr, rd_val);
    }
}

void chk_rd_wr_phy(unsigned int addr, unsigned int wr_val, unsigned int write_mask, unsigned int *err)
{
    unsigned int rd_val;
    unsigned int exp_val;
    write_reg(addr, wr_val);
    rd_val = read_reg(addr);
    if ((addr & 0x2) == 0x2) { rd_val = (rd_val >> 16) & 0xFFFF; }
    else { rd_val = rd_val & 0xFFFF; }
    exp_val = wr_val & write_mask;
    if (rd_val != exp_val)
    {
        LOGI("ERROR: chk_rd_wr_phy addr=0x%x wrote=0x%x exp=0x%x got=0x%x\n", addr, wr_val, exp_val, rd_val);
        (*err)++;
    }
    else
    {
        LOGI("PASS: chk_rd_wr_phy addr=0x%x val=0x%x\n", addr, rd_val);
    }
}

int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe reg wr rd test: %s\n", cfg->test_name);
    err1 = 0;
    err2 = 0;
    return 0;
}

int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    int i;
    int j;
    (void)cfg;
    LOGI("[Test Run] PCIe reg wr rd test: %s\n", cfg->test_name);

    /* Phase 1: Reset value check - DBI controller 0 */
    for (i = 0; i < 5; i++) { chk_rst_val(rc0_ctl_addr[i], ctl_default[i], &err1); }
    /* DBI controller 1 */
    for (i = 0; i < 5; i++) { chk_rst_val(rc1_ctl_addr[i], ctl_default[i], &err1); }
    /* SII controller 0 */
    for (i = 0; i < 3; i++) { chk_rst_val(sii0_addr[i], sii_default[i], &err1); }
    /* SII controller 1 */
    for (i = 0; i < 3; i++) { chk_rst_val(sii1_addr[i], sii_default[i], &err1); }

    /* PHY reset release */
    data_rd = read_reg(mizar_PCIE0_SII_PHY_RST_CONTROL);
    data_rd = data_rd | 0x1;
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, data_rd);
    data_rd = read_reg(mizar_PCIE1_SII_PHY_RST_CONTROL);
    data_rd = data_rd | 0x1;
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, data_rd);
    wait_on(10);

    /* PHY controller 0 */
    for (i = 0; i < 3; i++) { chk_rst_val_phy(phy0_addr[i], phy0_default[i], &err1); }
    /* PHY controller 1 */
    for (i = 0; i < 3; i++) { chk_rst_val_phy(phy1_addr[i], phy1_default[i], &err1); }

    /* Phase 2: Read-write with three test patterns */
    for (j = 0; j < 3; j++)
    {
        for (i = 0; i < 5; i++) { chk_rd_wr(rc0_ctl_addr[i], chk_val[j], 0xFFFFFFFF, &err2); }
        for (i = 0; i < 5; i++) { chk_rd_wr(rc1_ctl_addr[i], chk_val[j], 0xFFFFFFFF, &err2); }
        for (i = 0; i < 3; i++) { chk_rd_wr(sii0_addr[i], chk_val[j], sii0_write_mask[i], &err2); }
        for (i = 0; i < 3; i++) { chk_rd_wr(sii1_addr[i], chk_val[j], sii1_write_mask[i], &err2); }
        for (i = 0; i < 3; i++) { chk_rd_wr_phy(phy0_addr[i], chk_val_phy[j], phy0_write_mask[i], &err2); }
        for (i = 0; i < 3; i++) { chk_rd_wr_phy(phy1_addr[i], chk_val_phy[j], phy1_write_mask[i], &err2); }
    }

    LOGI("[Run] err1=%d err2=%d\n", err1, err2);
    finish(err2 || err1);
    return out->status = (err2 || err1);
}

int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe reg wr rd test: err1=%d err2=%d\n", err1, err2);
    return 0;
}
