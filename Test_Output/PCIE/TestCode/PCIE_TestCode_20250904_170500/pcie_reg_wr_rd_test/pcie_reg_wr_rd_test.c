// Author - AI Force 2.3. 04-Sep-2025 17:05 IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/* Global variables for testcase */
unsigned int data_rd, err1, err2;

/*
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization for pcie_reg_wr_rd_test.
 *              Checks reset values of PCIE0 and PCIE1 DBI DSP registers,
 *              SII registers, releases PHY reset, and checks PHY register
 *              reset values with 16-bit alignment extraction.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    unsigned int i, exp_val, act_val, addr;
    (void)cfg;
    LOGI("[Test Init] PCIE register write-read test: %s\n", cfg->test_name);
    err1 = 0;
    err2 = 0;

    /* Step 1: PCIE0 DBI DSP reset value check */
    for (i = 0; i < rc0_ctl_addr_size; i++)
    {
        act_val = read_reg(rc0_ctl_addr[i]);
        exp_val = rc0_ctl_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PCIE0 DBI DSP reset FAIL: addr=0x%08X exp=0x%08X act=0x%08X\n",
                     rc0_ctl_addr[i], exp_val, act_val);
            #endif
        }
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PCIE0 DBI DSP reset check complete, errors=%d\n", err1);
    #endif

    /* Step 2: PCIE1 DBI DSP reset value check */
    for (i = 0; i < rc1_ctl_addr_size; i++)
    {
        act_val = read_reg(rc1_ctl_addr[i]);
        exp_val = rc1_ctl_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PCIE1 DBI DSP reset FAIL: addr=0x%08X exp=0x%08X act=0x%08X\n",
                     rc1_ctl_addr[i], exp_val, act_val);
            #endif
        }
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PCIE1 DBI DSP reset check complete, errors=%d\n", err1);
    #endif

    /* Step 3: PCIE0 SII reset value check */
    for (i = 0; i < sii0_addr_size; i++)
    {
        act_val = read_sii0_reg(sii0_addr[i]);
        exp_val = sii0_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PCIE0 SII reset FAIL: offset=0x%08X exp=0x%08X act=0x%08X\n",
                     sii0_addr[i], exp_val, act_val);
            #endif
        }
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PCIE0 SII reset check complete\n");
    #endif

    /* Step 4: PCIE1 SII reset value check */
    for (i = 0; i < sii1_addr_size; i++)
    {
        act_val = read_sii1_reg(sii1_addr[i]);
        exp_val = sii1_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PCIE1 SII reset FAIL: offset=0x%08X exp=0x%08X act=0x%08X\n",
                     sii1_addr[i], exp_val, act_val);
            #endif
        }
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PCIE1 SII reset check complete\n");
    #endif

    /* Step 5: PHY reset release for both PCIE0 and PCIE1 */
    pcie0_phy_reset_release();
    pcie1_phy_reset_release();
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PHY reset released for PCIE0 and PCIE1\n");
    #endif

    /* Step 6: PHY register reset check with 16-bit alignment extraction */
    for (i = 0; i < phy0_addr_size; i++)
    {
        addr = phy0_addr[i];
        act_val = read_phy0_reg(addr);
        if ((addr & 0x3) == 0)
        {
            act_val = act_val & 0xFFFF;
        }
        else
        {
            act_val = (act_val >> 16) & 0xFFFF;
        }
        exp_val = phy0_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PHY0 reset FAIL: addr=0x%08X exp=0x%04X act=0x%04X\n",
                     addr, exp_val, act_val);
            #endif
        }
    }

    for (i = 0; i < phy1_addr_size; i++)
    {
        addr = phy1_addr[i];
        act_val = read_phy1_reg(addr);
        if ((addr & 0x3) == 0)
        {
            act_val = act_val & 0xFFFF;
        }
        else
        {
            act_val = (act_val >> 16) & 0xFFFF;
        }
        exp_val = phy1_rst[i];
        if (act_val != exp_val)
        {
            err1++;
            #ifdef DEBUG_DISPLAY
                LOGI("[Init] PHY1 reset FAIL: addr=0x%08X exp=0x%04X act=0x%04X\n",
                     addr, exp_val, act_val);
            #endif
        }
    }
    #ifdef DEBUG_DISPLAY
        LOGI("[Init] PHY register reset check complete, total errors=%d\n", err1);
    #endif

    return 0;
}

/*
 * Function: pcie_reg_wr_rd_test_run
 * Description: Main testcase execution for pcie_reg_wr_rd_test.
 *              Performs write-read verification with 3 check patterns on
 *              DBI DSP registers, SII registers (with write mask), and
 *              PHY registers (with alignment and mask).
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int i, j, addr, wr_val, rd_val, exp_val;
    (void)cfg;
    LOGI("[Test Run] PCIE register write-read test: %s\n", cfg->test_name);

    /* Step 7: Write-read with 3 check patterns */
    for (j = 0; j < NUM_CHK_PATTERNS; j++)
    {
        wr_val = chk_val[j];
        #ifdef DEBUG_DISPLAY
            LOGI("[Run] Pattern %d: wr_val=0x%08X\n", j, wr_val);
        #endif

        /* Step 8: DBI DSP readback verification - PCIE0 */
        for (i = 0; i < rc0_ctl_addr_size; i++)
        {
            write_reg(rc0_ctl_addr[i], wr_val);
            rd_val = read_reg(rc0_ctl_addr[i]);
            exp_val = wr_val & rc0_ctl_mask[i];
            if ((rd_val & rc0_ctl_mask[i]) != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PCIE0 DBI WR-RD FAIL: addr=0x%08X exp=0x%08X act=0x%08X\n",
                         rc0_ctl_addr[i], exp_val, rd_val);
                #endif
            }
        }

        /* DBI DSP readback verification - PCIE1 */
        for (i = 0; i < rc1_ctl_addr_size; i++)
        {
            write_reg(rc1_ctl_addr[i], wr_val);
            rd_val = read_reg(rc1_ctl_addr[i]);
            exp_val = wr_val & rc1_ctl_mask[i];
            if ((rd_val & rc1_ctl_mask[i]) != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PCIE1 DBI WR-RD FAIL: addr=0x%08X exp=0x%08X act=0x%08X\n",
                         rc1_ctl_addr[i], exp_val, rd_val);
                #endif
            }
        }

        /* Step 9: SII readback with write masks - PCIE0 */
        for (i = 0; i < sii0_addr_size; i++)
        {
            write_sii0_reg(sii0_addr[i], wr_val);
            rd_val = read_sii0_reg(sii0_addr[i]);
            exp_val = wr_val & SII_PARTIAL_WRITE_MASK;
            if ((rd_val & SII_PARTIAL_WRITE_MASK) != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PCIE0 SII WR-RD FAIL: offset=0x%08X exp=0x%08X act=0x%08X\n",
                         sii0_addr[i], exp_val, rd_val);
                #endif
            }
        }

        /* SII readback with write masks - PCIE1 */
        for (i = 0; i < sii1_addr_size; i++)
        {
            write_sii1_reg(sii1_addr[i], wr_val);
            rd_val = read_sii1_reg(sii1_addr[i]);
            exp_val = wr_val & SII_PARTIAL_WRITE_MASK;
            if ((rd_val & SII_PARTIAL_WRITE_MASK) != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PCIE1 SII WR-RD FAIL: offset=0x%08X exp=0x%08X act=0x%08X\n",
                         sii1_addr[i], exp_val, rd_val);
                #endif
            }
        }

        /* Step 10: PHY readback with alignment + mask */
        /* Re-apply PHY reset before each pattern cycle */
        pcie0_phy_reset_release();
        pcie1_phy_reset_release();

        for (i = 0; i < phy0_addr_size; i++)
        {
            addr = phy0_addr[i];
            write_phy0_reg(addr, wr_val);
            rd_val = read_phy0_reg(addr);
            if ((addr & 0x3) == 0)
            {
                rd_val = rd_val & 0xFFFF;
            }
            else
            {
                rd_val = (rd_val >> 16) & 0xFFFF;
            }
            exp_val = wr_val & PHY_WRITE_MASK;
            if (rd_val != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PHY0 WR-RD FAIL: addr=0x%08X exp=0x%04X act=0x%04X\n",
                         addr, exp_val, rd_val);
                #endif
            }
        }

        for (i = 0; i < phy1_addr_size; i++)
        {
            addr = phy1_addr[i];
            write_phy1_reg(addr, wr_val);
            rd_val = read_phy1_reg(addr);
            if ((addr & 0x3) == 0)
            {
                rd_val = rd_val & 0xFFFF;
            }
            else
            {
                rd_val = (rd_val >> 16) & 0xFFFF;
            }
            exp_val = wr_val & PHY_WRITE_MASK;
            if (rd_val != exp_val)
            {
                err2++;
                #ifdef DEBUG_DISPLAY
                    LOGI("[Run] PHY1 WR-RD FAIL: addr=0x%08X exp=0x%04X act=0x%04X\n",
                         addr, exp_val, rd_val);
                #endif
            }
        }
    }

    #ifdef DEBUG_DISPLAY
        LOGI("[Run] Write-read verification complete, err2=%d\n", err2);
    #endif

    return out->status = (err2 || err1);
}

/*
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs final validation and cleanup for pcie_reg_wr_rd_test.
 *              Calls finish with combined error status.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    LOGI("[TEARDOWN] PCIE register write-read test: %s\n", cfg->test_name);

    /*
     * Validation / Acceptance Criteria:
     * 1. All PCIE0 and PCIE1 DBI DSP registers match expected reset values.
     * 2. All PCIE0 and PCIE1 SII registers match expected reset values.
     * 3. All PHY registers match expected reset values after 16-bit extraction.
     * 4. Write-read verification passes for all 3 check patterns on DBI DSP,
     *    SII (with mask 0xF000F), and PHY (with mask 0x1FFF) registers.
     * 5. Test passes by calling finish(err2 || err1).
     */

    /* Step 11: Call finish with combined error status */
    finish(err2 || err1);

    return 0;
}
