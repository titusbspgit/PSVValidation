// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.cin"

/* PCIe Register Write Read Test
 * Description: This testcase verifies PCIe register reset default values
 * and read-write functionality across three register domains.
 /

unsigned int test_err, rdata, i;
unsigned int err1, err2;

/
 * Helper: chk_rst_val
 * Reads each register address and compares against expected default value.
 */
static void chk_rst_val(const unsigned int *addr_arr, const unsigned int *def_arr, unsigned int count, unsigned int *err)
{
 unsigned int idx;
 for (idx = 0; idx < count; idx++)
 {
 rdata = read_reg(addr_arr[idx]);
 if (rdata != def_arr[idx])
 {
 LOGI("ERROR: chk_rst_val addr=0x%x exp=0x%x act=0x%x\n", addr_arr[idx], def_arr[idx], rdata);
 (*err)++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: chk_rst_val addr=0x%x val=0x%x\n", addr_arr[idx], rdata);
 #endif
 }
 }
}

/
 * Helper: chk_rd_wr
 * Writes a test pattern (masked), reads back, and compares.
 */
static void chk_rd_wr(const unsigned int *addr_arr, const unsigned int *mask_arr, unsigned int count, unsigned int pattern, unsigned int *err)
{
 unsigned int idx, wr_val, exp_val;
 for (idx = 0; idx < count; idx++)
 {
 wr_val = pattern & mask_arr[idx];
 write_reg(addr_arr[idx], wr_val);
 rdata = read_reg(addr_arr[idx]);
 exp_val = wr_val;
 if (rdata != exp_val)
 {
 LOGI("ERROR: chk_rd_wr addr=0x%x pattern=0x%x exp=0x%x act=0x%x\n", addr_arr[idx], pattern, exp_val, rdata);
 (*err)++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: chk_rd_wr addr=0x%x pattern=0x%x val=0x%x\n", addr_arr[idx], pattern, rdata);
 #endif
 }
 }
}

/
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_reg_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[Test Init] PCIe reg wr rd test: %s\n", cfg->test_name);

 err1 = 0;
 err2 = 0;
 test_err = 0;

 return 0;
}

/
 * Function: pcie_reg_wr_rd_test_run
 * Description: Main testcase execution for PCIe register reset default value verification
 * and read-write functionality check across DBI, SII, and PHY register domains
 * using three test patterns.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 LOGI("[Test Run] PCIe reg wr rd test: %s\n", cfg->test_name);
 err1 = 0;
 err2 = 0;

 / Step 1: test_case() entry /
 LOGI("Step 1: test_case() entry\n");

 / Steps 2-3: chk_rst_val for DBI registers - PCIE0 and PCIE1 /
 LOGI("Steps 2-3: chk_rst_val for DBI registers\n");
 chk_rst_val(rc0_ctl_addr, ctl_default, 5, &err1);
 chk_rst_val(rc1_ctl_addr, ctl_default, 5, &err1);

 / Steps 4-5: chk_rst_val for SII registers - PCIE0 and PCIE1 /
 LOGI("Steps 4-5: chk_rst_val for SII registers\n");
 chk_rst_val(sii0_addr, sii_default, 3, &err1);
 chk_rst_val(sii1_addr, sii_default, 3, &err1);

 / Steps 6-7: PHY reset /
 LOGI("Steps 6-7: PHY reset and default check\n");
 / Assert PHY reset /
 write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x1);
 write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x1);
 wait_on(10);
 / Deassert PHY reset /
 write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x0);
 write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x0);
 wait_on(10);

 / Steps 8-9: PHY default value check /
 LOGI("Steps 8-9: PHY default value check\n");
 chk_rst_val(phy0_addr, phy0_default, 3, &err1);
 chk_rst_val(phy1_addr, phy1_default, 3, &err1);

 / Steps 10-12: chk_rd_wr with test pattern chk_val[0] for DBI /
 LOGI("Steps 10-12: chk_rd_wr DBI with pattern chk_val[0]\n");
 chk_rd_wr(rc0_ctl_addr, sii0_write_mask, 5, chk_val[0], &err2);
 chk_rd_wr(rc1_ctl_addr, sii1_write_mask, 5, chk_val[0], &err2);

 / Steps 13-14: chk_rd_wr with test pattern chk_val[1] for DBI /
 LOGI("Steps 13-14: chk_rd_wr DBI with pattern chk_val[1]\n");
 chk_rd_wr(rc0_ctl_addr, sii0_write_mask, 5, chk_val[1], &err2);
 chk_rd_wr(rc1_ctl_addr, sii1_write_mask, 5, chk_val[1], &err2);

 / Steps 15-16: chk_rd_wr with test pattern chk_val[2] for DBI /
 LOGI("Steps 15-16: chk_rd_wr DBI with pattern chk_val[2]\n");
 chk_rd_wr(rc0_ctl_addr, sii0_write_mask, 5, chk_val[2], &err2);
 chk_rd_wr(rc1_ctl_addr, sii1_write_mask, 5, chk_val[2], &err2);

 / Steps 17-18: chk_rd_wr with test pattern chk_val[3] for SII /
 LOGI("Steps 17-18: chk_rd_wr SII with pattern chk_val[3]\n");
 chk_rd_wr(sii0_addr, sii0_write_mask, 3, chk_val[3], &err2);
 chk_rd_wr(sii1_addr, sii1_write_mask, 3, chk_val[3], &err2);

 / Steps 19-20: chk_rd_wr with test pattern chk_val[4] for SII /
 LOGI("Steps 19-20: chk_rd_wr SII with pattern chk_val[4]\n");
 chk_rd_wr(sii0_addr, sii0_write_mask, 3, chk_val[4], &err2);
 chk_rd_wr(sii1_addr, sii1_write_mask, 3, chk_val[4], &err2);

 / Steps 21-22: chk_rd_wr with test pattern chk_val[5] for SII /
 LOGI("Steps 21-22: chk_rd_wr SII with pattern chk_val[5]\n");
 chk_rd_wr(sii0_addr, sii0_write_mask, 3, chk_val[5], &err2);
 chk_rd_wr(sii1_addr, sii1_write_mask, 3, chk_val[5], &err2);

 / Steps 23-24: chk_rd_wr for PHY registers with chk_val_phy patterns /
 LOGI("Steps 23-24: chk_rd_wr PHY registers\n");
 chk_rd_wr(phy0_addr, phy0_write_mask, 3, chk_val_phy[0], &err2);
 chk_rd_wr(phy1_addr, phy1_write_mask, 3, chk_val_phy[0], &err2);
 chk_rd_wr(phy0_addr, phy0_write_mask, 3, chk_val_phy[1], &err2);
 chk_rd_wr(phy1_addr, phy1_write_mask, 3, chk_val_phy[1], &err2);
 chk_rd_wr(phy0_addr, phy0_write_mask, 3, chk_val_phy[2], &err2);
 chk_rd_wr(phy1_addr, phy1_write_mask, 3, chk_val_phy[2], &err2);

 / Step 25: finish(err2 || err1) /
 LOGI("Step 25: finish(err2 || err1) err1=%u err2=%u\n", err1, err2);
 test_err = err1 + err2;
 finish(err2 || err1);

 return out->status = test_err;
}

/
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs teardown and final observation for pcie_reg_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe reg wr rd test teardown: %s\n", cfg->test_name);

 return 0;
}
