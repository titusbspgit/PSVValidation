/* Author - AI Force 2.3. Date in IST /
/ (EMBENGG-SYSAPPS) /

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/
 * PCIe Register Write Read Test
 * Description: This testcase performs register reset-value verification and register
 * write-read verification for PCIe controller registers across three
 * register groups (RC controller, SII, PHY).
 /

unsigned int test_err, rdata, i;
unsigned int err1, err2;

/
 * Function: chk_rst_val
 * Description: Checks reset default values for all three register groups
 * (RC controller, SII, PHY) across both PCIE0 and PCIE1.
 /
static void chk_rst_val(void)
{
 err1 = 0;

 / Step 3: Reset value check for RC0 controller registers /
 LOGI("[chk_rst_val] Checking RC0 controller register reset values\n");
 for (i = 0; i < 5; i++)
 {
 rdata = read_reg(rc0_ctl_addr[i]);
 if (rdata != ctl_default[i])
 {
 LOGI("ERROR: RC0 ctl reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, rc0_ctl_addr[i], ctl_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: RC0 ctl reg[%d] addr=0x%x reset value=0x%x\n", i, rc0_ctl_addr[i], rdata);
 #endif
 }
 }

 / Step 4: Reset value check for RC1 controller registers /
 LOGI("[chk_rst_val] Checking RC1 controller register reset values\n");
 for (i = 0; i < 5; i++)
 {
 rdata = read_reg(rc1_ctl_addr[i]);
 if (rdata != ctl_default[i])
 {
 LOGI("ERROR: RC1 ctl reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, rc1_ctl_addr[i], ctl_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: RC1 ctl reg[%d] addr=0x%x reset value=0x%x\n", i, rc1_ctl_addr[i], rdata);
 #endif
 }
 }

 / Step 5: Reset value check for SII0 registers /
 LOGI("[chk_rst_val] Checking SII0 register reset values\n");
 for (i = 0; i < 3; i++)
 {
 rdata = read_reg(sii0_addr[i]);
 if (rdata != sii_default[i])
 {
 LOGI("ERROR: SII0 reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, sii0_addr[i], sii_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: SII0 reg[%d] addr=0x%x reset value=0x%x\n", i, sii0_addr[i], rdata);
 #endif
 }
 }

 / Step 6: Reset value check for SII1 registers /
 LOGI("[chk_rst_val] Checking SII1 register reset values\n");
 for (i = 0; i < 3; i++)
 {
 rdata = read_reg(sii1_addr[i]);
 if (rdata != sii_default[i])
 {
 LOGI("ERROR: SII1 reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, sii1_addr[i], sii_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: SII1 reg[%d] addr=0x%x reset value=0x%x\n", i, sii1_addr[i], rdata);
 #endif
 }
 }

 / Step 7: Reset value check for PHY0 registers /
 LOGI("[chk_rst_val] Checking PHY0 register reset values\n");
 for (i = 0; i < 3; i++)
 {
 rdata = read_reg(phy0_addr[i]);
 if (rdata != phy0_default[i])
 {
 LOGI("ERROR: PHY0 reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, phy0_addr[i], phy0_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PHY0 reg[%d] addr=0x%x reset value=0x%x\n", i, phy0_addr[i], rdata);
 #endif
 }
 }

 / Step 8: Reset value check for PHY1 registers /
 LOGI("[chk_rst_val] Checking PHY1 register reset values\n");
 for (i = 0; i < 3; i++)
 {
 rdata = read_reg(phy1_addr[i]);
 if (rdata != phy1_default[i])
 {
 LOGI("ERROR: PHY1 reg[%d] addr=0x%x expected=0x%x actual=0x%x\n", i, phy1_addr[i], phy1_default[i], rdata);
 err1++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PHY1 reg[%d] addr=0x%x reset value=0x%x\n", i, phy1_addr[i], rdata);
 #endif
 }
 }

 / Step 9: Reset value check complete /
 LOGI("[chk_rst_val] Reset value check complete, err1=%d\n", err1);
}

/
 * Function: chk_rd_wr
 * Description: Performs write-read verification for all three register groups
 * (RC controller, SII, PHY) across both PCIE0 and PCIE1,
 * then restores default values.
 /
static void chk_rd_wr(void)
{
 err2 = 0;

 / Step 10: Write-read check for RC0 controller registers /
 LOGI("[chk_rd_wr] Write-read check RC0 controller registers\n");
 for (i = 0; i < 5; i++)
 {
 write_reg(rc0_ctl_addr[i], chk_val[i]);
 rdata = read_reg(rc0_ctl_addr[i]);
 if (rdata != chk_val[i])
 {
 LOGI("ERROR: RC0 ctl wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x\n", i, rc0_ctl_addr[i], chk_val[i], rdata);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: RC0 ctl wr-rd reg[%d] addr=0x%x value=0x%x\n", i, rc0_ctl_addr[i], rdata);
 #endif
 }
 }

 / Step 11: Write-read check for RC1 controller registers /
 LOGI("[chk_rd_wr] Write-read check RC1 controller registers\n");
 for (i = 0; i < 5; i++)
 {
 write_reg(rc1_ctl_addr[i], chk_val[i]);
 rdata = read_reg(rc1_ctl_addr[i]);
 if (rdata != chk_val[i])
 {
 LOGI("ERROR: RC1 ctl wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x\n", i, rc1_ctl_addr[i], chk_val[i], rdata);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: RC1 ctl wr-rd reg[%d] addr=0x%x value=0x%x\n", i, rc1_ctl_addr[i], rdata);
 #endif
 }
 }

 / Step 12: Write-read check for SII0 registers with write mask /
 LOGI("[chk_rd_wr] Write-read check SII0 registers\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(sii0_addr[i], chk_val[i]);
 rdata = read_reg(sii0_addr[i]);
 if ((rdata & sii0_write_mask[i]) != (chk_val[i] & sii0_write_mask[i]))
 {
 LOGI("ERROR: SII0 wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x mask=0x%x\n", i, sii0_addr[i], chk_val[i], rdata, sii0_write_mask[i]);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: SII0 wr-rd reg[%d] addr=0x%x value=0x%x\n", i, sii0_addr[i], rdata);
 #endif
 }
 }

 / Step 13: Write-read check for SII1 registers with write mask /
 LOGI("[chk_rd_wr] Write-read check SII1 registers\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(sii1_addr[i], chk_val[i]);
 rdata = read_reg(sii1_addr[i]);
 if ((rdata & sii1_write_mask[i]) != (chk_val[i] & sii1_write_mask[i]))
 {
 LOGI("ERROR: SII1 wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x mask=0x%x\n", i, sii1_addr[i], chk_val[i], rdata, sii1_write_mask[i]);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: SII1 wr-rd reg[%d] addr=0x%x value=0x%x\n", i, sii1_addr[i], rdata);
 #endif
 }
 }

 / Step 14: Write-read check for PHY0 registers with write mask /
 LOGI("[chk_rd_wr] Write-read check PHY0 registers\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(phy0_addr[i], chk_val_phy[i]);
 rdata = read_reg(phy0_addr[i]);
 if ((rdata & phy0_write_mask[i]) != (chk_val_phy[i] & phy0_write_mask[i]))
 {
 LOGI("ERROR: PHY0 wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x mask=0x%x\n", i, phy0_addr[i], chk_val_phy[i], rdata, phy0_write_mask[i]);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PHY0 wr-rd reg[%d] addr=0x%x value=0x%x\n", i, phy0_addr[i], rdata);
 #endif
 }
 }

 / Step 15: Write-read check for PHY1 registers with write mask /
 LOGI("[chk_rd_wr] Write-read check PHY1 registers\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(phy1_addr[i], chk_val_phy[i]);
 rdata = read_reg(phy1_addr[i]);
 if ((rdata & phy1_write_mask[i]) != (chk_val_phy[i] & phy1_write_mask[i]))
 {
 LOGI("ERROR: PHY1 wr-rd reg[%d] addr=0x%x written=0x%x read=0x%x mask=0x%x\n", i, phy1_addr[i], chk_val_phy[i], rdata, phy1_write_mask[i]);
 err2++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PHY1 wr-rd reg[%d] addr=0x%x value=0x%x\n", i, phy1_addr[i], rdata);
 #endif
 }
 }

 / Step 16: Restore RC0 controller register defaults /
 LOGI("[chk_rd_wr] Restoring RC0 controller register defaults\n");
 for (i = 0; i < 5; i++)
 {
 write_reg(rc0_ctl_addr[i], ctl_default[i]);
 }

 / Step 17: Restore RC1 controller register defaults /
 LOGI("[chk_rd_wr] Restoring RC1 controller register defaults\n");
 for (i = 0; i < 5; i++)
 {
 write_reg(rc1_ctl_addr[i], ctl_default[i]);
 }

 / Step 18: Restore SII0 register defaults /
 LOGI("[chk_rd_wr] Restoring SII0 register defaults\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(sii0_addr[i], sii_default[i]);
 }

 / Step 19: Restore SII1 register defaults /
 LOGI("[chk_rd_wr] Restoring SII1 register defaults\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(sii1_addr[i], sii_default[i]);
 }

 / Step 20: Restore PHY0 register defaults /
 LOGI("[chk_rd_wr] Restoring PHY0 register defaults\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(phy0_addr[i], phy0_default[i]);
 }

 / Step 21: Restore PHY1 register defaults /
 LOGI("[chk_rd_wr] Restoring PHY1 register defaults\n");
 for (i = 0; i < 3; i++)
 {
 write_reg(phy1_addr[i], phy1_default[i]);
 }

 LOGI("[chk_rd_wr] Write-read check complete, err2=%d\n", err2);
}

/
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup
 * for pcie_reg_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;
 printf("[Test Init] PCIe reg wr rd test: %s\n", cfg->test_name);
 LOGI("[Test Init] PCIe reg wr rd test: %s\n", cfg->test_name);

 test_err = 0;
 err1 = 0;
 err2 = 0;

 return 0;
}

/
 * Function: pcie_reg_wr_rd_test_run
 * Description: Main testcase execution for PCIe register reset-value
 * verification and register write-read verification across
 * three register groups (RC controller, SII, PHY).
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
 LOGI("[Test Run] PCIe reg wr rd test: %s\n", cfg->test_name);
 test_err = 0;

 / Step 2: Call chk_rst_val() then chk_rd_wr() /
 LOGI("Step 2: Calling chk_rst_val()\n");
 chk_rst_val();

 LOGI("Step 2: Calling chk_rd_wr()\n");
 chk_rd_wr();

 / Step 22: finish(err2 || err1) /
 LOGI("Step 22: err1=%d err2=%d, calling finish(%d)\n", err1, err2, (err2 || err1));
 test_err = (err2 || err1);
 finish(err2 || err1);

 return out->status = test_err;
}

/
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs validation, final observation, and testcase
 * completion for pcie_reg_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe reg wr rd test teardown: %s\n", cfg->test_name);

 /
 * Validation:
 * 1. Reset value check result: err1
 * 2. Write-read check result: err2
 * 3. Final pass/fail determined by finish(err2 || err1) in run phase
 /

 return 0;
}
