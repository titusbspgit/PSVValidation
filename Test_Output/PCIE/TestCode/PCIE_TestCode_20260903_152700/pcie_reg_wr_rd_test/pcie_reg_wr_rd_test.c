// Author - AI Force 2.3. 03-Sep-2026 15:27 IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.cin"

unsigned int data_rd, test_err, err1, err2, i;

/*
 * Function: chk_rst_val
 * Description: Checks register reset default values against expected defaults.
 *              Reads each register address and compares with expected default.
 * Parameters:
 *   addr_arr   - Array of register addresses.
 *   def_arr    - Array of expected default values.
 *   count      - Number of registers to check.
 *   p_err      - Pointer to error counter.
 */
static void chk_rst_val(const unsigned int addr_arr[],
                        const unsigned int def_arr[],
                        unsigned int count,
                        unsigned int *p_err)
{
    unsigned int idx;
    unsigned int rdata;

    for (idx = 0; idx < count; idx++)
    {
        rdata = read_reg(addr_arr[idx]);
        if (rdata != def_arr[idx])
        {
            LOGI("ERROR: Reset default mismatch addr=0x%x exp=0x%x act=0x%x\n",
                 addr_arr[idx], def_arr[idx], rdata);
            (*p_err)++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: Reset default match addr=0x%x val=0x%x\n",
                     addr_arr[idx], rdata);
            #endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Description: Performs read-write check on registers using a test pattern
 *              and write mask. Writes pattern, reads back, and compares.
 * Parameters:
 *   addr_arr   - Array of register addresses.
 *   mask_arr   - Array of write masks.
 *   count      - Number of registers to check.
 *   pattern    - Test pattern to write.
 *   p_err      - Pointer to error counter.
 */
static void chk_rd_wr(const unsigned int addr_arr[],
                      const unsigned int mask_arr[],
                      unsigned int count,
                      unsigned int pattern,
                      unsigned int *p_err)
{
    unsigned int idx;
    unsigned int rdata;
    unsigned int expected;

    for (idx = 0; idx < count; idx++)
    {
        expected = pattern & mask_arr[idx];
        write_reg(addr_arr[idx], pattern);
        rdata = read_reg(addr_arr[idx]);
        if ((rdata & mask_arr[idx]) != expected)
        {
            LOGI("ERROR: RD/WR mismatch addr=0x%x pattern=0x%x mask=0x%x exp=0x%x act=0x%x\n",
                 addr_arr[idx], pattern, mask_arr[idx], expected, rdata);
            (*p_err)++;
        }
        else
        {
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: RD/WR match addr=0x%x val=0x%x\n",
                     addr_arr[idx], rdata);
            #endif
        }
    }
}

/*
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              PCIe register reset default and read-write verification test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[Test Init] PCIe reg wr rd test: %s\n", cfg->test_name);

    err1 = 0;
    err2 = 0;
    test_err = 0;

    return 0;
}

/*
 * Function: pcie_reg_wr_rd_test_run
 * Description: Main testcase execution for PCIe register reset default value
 *              verification and read-write functionality across DBI, SII, and
 *              PHY register domains. Checks reset defaults, performs PHY reset
 *              and default check, then executes read-write with three test
 *              patterns.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    LOGI("[Test Run] PCIe reg wr rd test: %s\n", cfg->test_name);
    test_err = 0;
    err1 = 0;
    err2 = 0;

    /* Step 1: test_case() entry */
    LOGI("[Run] test_case() entry\n");

    /* Steps 2-3: chk_rst_val for DBI registers - RC0 and RC1 */
    LOGI("[Run] Checking reset defaults for RC0 DBI registers\n");
    chk_rst_val(rc0_ctl_addr, ctl_default, RC_CTL_COUNT, &err1);

    LOGI("[Run] Checking reset defaults for RC1 DBI registers\n");
    chk_rst_val(rc1_ctl_addr, ctl_default, RC_CTL_COUNT, &err1);

    /* Steps 4-5: chk_rst_val for SII registers - SII0 and SII1 */
    LOGI("[Run] Checking reset defaults for SII0 registers\n");
    chk_rst_val(sii0_addr, sii_default, SII_COUNT, &err1);

    LOGI("[Run] Checking reset defaults for SII1 registers\n");
    chk_rst_val(sii1_addr, sii_default, SII_COUNT, &err1);

    /* Steps 6-7: PHY reset */
    LOGI("[Run] Asserting PHY0 reset via mizar_PCIE0_SII_PHY_RST_CONTROL\n");
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x1);
    wait_on(10);
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x0);
    wait_on(10);

    LOGI("[Run] Asserting PHY1 reset via mizar_PCIE1_SII_PHY_RST_CONTROL\n");
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x1);
    wait_on(10);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x0);
    wait_on(10);

    /* Steps 8-9: PHY default check */
    LOGI("[Run] Checking reset defaults for PHY0 registers\n");
    chk_rst_val(phy0_addr, phy0_default, PHY_COUNT, &err1);

    LOGI("[Run] Checking reset defaults for PHY1 registers\n");
    chk_rst_val(phy1_addr, phy1_default, PHY_COUNT, &err1);

    /* Steps 10-12: chk_rd_wr with test pattern 0 for DBI registers */
    LOGI("[Run] RD/WR check RC0 DBI with pattern chk_val[0]=0x%x\n", chk_val[0]);
    chk_rd_wr(rc0_ctl_addr, sii0_write_mask, RC_CTL_COUNT, chk_val[0], &err2);

    LOGI("[Run] RD/WR check RC1 DBI with pattern chk_val[0]=0x%x\n", chk_val[0]);
    chk_rd_wr(rc1_ctl_addr, sii1_write_mask, RC_CTL_COUNT, chk_val[0], &err2);

    /* Steps 13-14: chk_rd_wr with test pattern 0 for SII registers */
    LOGI("[Run] RD/WR check SII0 with pattern chk_val[1]=0x%x\n", chk_val[1]);
    chk_rd_wr(sii0_addr, sii0_write_mask, SII_COUNT, chk_val[1], &err2);

    LOGI("[Run] RD/WR check SII1 with pattern chk_val[1]=0x%x\n", chk_val[1]);
    chk_rd_wr(sii1_addr, sii1_write_mask, SII_COUNT, chk_val[1], &err2);

    /* Steps 15-16: chk_rd_wr with test pattern 1 for DBI registers */
    LOGI("[Run] RD/WR check RC0 DBI with pattern chk_val[2]=0x%x\n", chk_val[2]);
    chk_rd_wr(rc0_ctl_addr, sii0_write_mask, RC_CTL_COUNT, chk_val[2], &err2);

    LOGI("[Run] RD/WR check RC1 DBI with pattern chk_val[2]=0x%x\n", chk_val[2]);
    chk_rd_wr(rc1_ctl_addr, sii1_write_mask, RC_CTL_COUNT, chk_val[2], &err2);

    /* Steps 17-18: chk_rd_wr with test pattern 1 for SII registers */
    LOGI("[Run] RD/WR check SII0 with pattern chk_val[3]=0x%x\n", chk_val[3]);
    chk_rd_wr(sii0_addr, sii0_write_mask, SII_COUNT, chk_val[3], &err2);

    LOGI("[Run] RD/WR check SII1 with pattern chk_val[3]=0x%x\n", chk_val[3]);
    chk_rd_wr(sii1_addr, sii1_write_mask, SII_COUNT, chk_val[3], &err2);

    /* Steps 19-20: chk_rd_wr with test pattern 2 for DBI registers */
    LOGI("[Run] RD/WR check RC0 DBI with pattern chk_val[4]=0x%x\n", chk_val[4]);
    chk_rd_wr(rc0_ctl_addr, sii0_write_mask, RC_CTL_COUNT, chk_val[4], &err2);

    LOGI("[Run] RD/WR check RC1 DBI with pattern chk_val[4]=0x%x\n", chk_val[4]);
    chk_rd_wr(rc1_ctl_addr, sii1_write_mask, RC_CTL_COUNT, chk_val[4], &err2);

    /* Steps 21-22: chk_rd_wr with test pattern 2 for SII registers */
    LOGI("[Run] RD/WR check SII0 with pattern chk_val[5]=0x%x\n", chk_val[5]);
    chk_rd_wr(sii0_addr, sii0_write_mask, SII_COUNT, chk_val[5], &err2);

    LOGI("[Run] RD/WR check SII1 with pattern chk_val[5]=0x%x\n", chk_val[5]);
    chk_rd_wr(sii1_addr, sii1_write_mask, SII_COUNT, chk_val[5], &err2);

    /* Steps 23-24: chk_rd_wr for PHY registers with PHY patterns */
    LOGI("[Run] RD/WR check PHY0 with pattern chk_val_phy[0]=0x%x\n", chk_val_phy[0]);
    chk_rd_wr(phy0_addr, phy0_write_mask, PHY_COUNT, chk_val_phy[0], &err2);

    LOGI("[Run] RD/WR check PHY1 with pattern chk_val_phy[0]=0x%x\n", chk_val_phy[0]);
    chk_rd_wr(phy1_addr, phy1_write_mask, PHY_COUNT, chk_val_phy[0], &err2);

    LOGI("[Run] RD/WR check PHY0 with pattern chk_val_phy[1]=0x%x\n", chk_val_phy[1]);
    chk_rd_wr(phy0_addr, phy0_write_mask, PHY_COUNT, chk_val_phy[1], &err2);

    LOGI("[Run] RD/WR check PHY1 with pattern chk_val_phy[1]=0x%x\n", chk_val_phy[1]);
    chk_rd_wr(phy1_addr, phy1_write_mask, PHY_COUNT, chk_val_phy[1], &err2);

    LOGI("[Run] RD/WR check PHY0 with pattern chk_val_phy[2]=0x%x\n", chk_val_phy[2]);
    chk_rd_wr(phy0_addr, phy0_write_mask, PHY_COUNT, chk_val_phy[2], &err2);

    LOGI("[Run] RD/WR check PHY1 with pattern chk_val_phy[2]=0x%x\n", chk_val_phy[2]);
    chk_rd_wr(phy1_addr, phy1_write_mask, PHY_COUNT, chk_val_phy[2], &err2);

    /* Step 25: finish(err2 || err1) */
    LOGI("[Run] err1=%u err2=%u\n", err1, err2);
    test_err = err1 + err2;
    finish(err2 || err1);

    return out->status = test_err;
}

/*
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs validation observations, cleanup, and testcase
 *              completion for PCIe register reset default and read-write test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIe reg wr rd test teardown: %s\n", cfg->test_name);

    /* Validation observations:
     * 1. Reset value check for all DBI registers - verified in run.
     * 2. Reset value check for all SII registers - verified in run.
     * 3. PHY reset value check - verified in run.
     * 4. Read-write check with three patterns - verified in run.
     * 5. finish(err2 || err1) - called in run.
     */

    LOGI("[TEARDOWN] Final error counts: err1=%u err2=%u total=%u\n", err1, err2, test_err);

    return 0;
}
