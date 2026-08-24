/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

#include <stdio.h>
#include "test_define.c"

/* File-scope variables for cross-function access */
static int err1 = 0;
static int err2 = 0;

/********************************************************************
 * Function Name  : chk_rst_val
 * Description    : Verify all registers contain expected reset defaults
 *                  for DBI DSP controller, SII interface, and PHY
 *                  register groups on both PCIE0 and PCIE1.
 * Parameters     : p_err1 - Pointer to error counter 1,
 *                  p_err2 - Pointer to error counter 2.
 * Return Value   : None.
 ********************************************************************/
void chk_rst_val(int *p_err1, int *p_err2)
{
    unsigned int data_rd = 0;
    int i = 0;

    /* ------------------------------------------------------------ */
    /* Step 9: Read PCIE0 DBI DSP controller registers and verify   */
    /*         reset default values                                  */
    /* ------------------------------------------------------------ */
    for (i = 0; i < CTL_REG_COUNT; i++)
    {
        data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: rc0_ctl_addr[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i])
        {
            (*p_err1)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: rc0_ctl_addr[%d] mismatch\n", i);
#endif
        }
    }

    /* ------------------------------------------------------------ */
    /* Step 10: Read PCIE1 DBI DSP controller registers and verify  */
    /* ------------------------------------------------------------ */
    for (i = 0; i < CTL_REG_COUNT; i++)
    {
        data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: rc1_ctl_addr[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i])
        {
            (*p_err2)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: rc1_ctl_addr[%d] mismatch\n", i);
#endif
        }
    }

    /* ------------------------------------------------------------ */
    /* Step 11: Read PCIE0 SII interface registers and verify       */
    /* ------------------------------------------------------------ */
    for (i = 0; i < SII_REG_COUNT; i++)
    {
        data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: sii0_addr[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i])
        {
            (*p_err2)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: sii0_addr[%d] mismatch\n", i);
#endif
        }
    }

    /* ------------------------------------------------------------ */
    /* Step 12: Read PCIE1 SII interface registers and verify       */
    /* ------------------------------------------------------------ */
    for (i = 0; i < SII_REG_COUNT; i++)
    {
        data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: sii1_addr[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i])
        {
            (*p_err2)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: sii1_addr[%d] mismatch\n", i);
#endif
        }
    }

    /* ------------------------------------------------------------ */
    /* Step 13: Write PHY reset control value to PCIE0              */
    /* ------------------------------------------------------------ */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VALUE);
#ifdef DEBUG_DISPLAY
    debug_print("chk_rst_val: Wrote 0x01203000 to PCIE0 PHY_RST_CONTROL\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 14: Write PHY reset control value to PCIE1              */
    /* ------------------------------------------------------------ */
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VALUE);
#ifdef DEBUG_DISPLAY
    debug_print("chk_rst_val: Wrote 0x01203000 to PCIE1 PHY_RST_CONTROL\n");
#endif

    /* ------------------------------------------------------------ */
    /* Step 15: Read PCIE0 PHY registers with 16-bit extraction     */
    /* ------------------------------------------------------------ */
    for (i = 0; i < PHY_REG_COUNT; i++)
    {
        data_rd = read_reg(phy0_addr[i]);
        if ((phy0_addr[i] % 4) != 0)
        {
            data_rd = data_rd >> 16;
        }
        else
        {
            data_rd = data_rd & PHY_16BIT_MASK;
        }
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: phy0_addr[%d] read(16-bit) = 0x%04X, expected = 0x%04X\n", i, data_rd, phy0_default[i]);
#endif
        if (data_rd != phy0_default[i])
        {
            (*p_err2)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: phy0_addr[%d] mismatch\n", i);
#endif
        }
    }

    /* ------------------------------------------------------------ */
    /* Step 16: Read PCIE1 PHY registers with 16-bit extraction     */
    /* ------------------------------------------------------------ */
    for (i = 0; i < PHY_REG_COUNT; i++)
    {
        data_rd = read_reg(phy1_addr[i]);
        if ((phy1_addr[i] % 4) != 0)
        {
            data_rd = data_rd >> 16;
        }
        else
        {
            data_rd = data_rd & PHY_16BIT_MASK;
        }
#ifdef DEBUG_DISPLAY
        debug_print("chk_rst_val: phy1_addr[%d] read(16-bit) = 0x%04X, expected = 0x%04X\n", i, data_rd, phy1_default[i]);
#endif
        if (data_rd != phy1_default[i])
        {
            (*p_err2)++;
#ifdef DEBUG_DISPLAY
            debug_print("FAIL: phy1_addr[%d] mismatch\n", i);
#endif
        }
    }
}

/********************************************************************
 * Function Name  : chk_rd_wr
 * Description    : Write-read-back verification with multiple patterns
 *                  across DBI DSP controller, SII interface, and PHY
 *                  register groups on both PCIE0 and PCIE1.
 * Parameters     : p_err1 - Pointer to error counter 1.
 * Return Value   : None.
 ********************************************************************/
void chk_rd_wr(int *p_err1)
{
    unsigned int data_rd = 0;
    int i = 0;
    int j = 0;

    /* ------------------------------------------------------------ */
    /* Step 17-33: Iterate over 3 test patterns                     */
    /* ------------------------------------------------------------ */
    for (j = 0; j < CHK_PATTERN_COUNT; j++)
    {
#ifdef DEBUG_DISPLAY
        debug_print("chk_rd_wr: Pattern iteration %d, chk_val = 0x%08X\n", j, chk_val[j]);
#endif

        /* Step 19: Write pattern to PCIE0 DBI DSP controller registers */
        for (i = 0; i < CTL_REG_COUNT; i++)
        {
            write_reg(rc0_ctl_addr[i], chk_val[j]);
        }

        /* Step 20: Write pattern to PCIE1 DBI DSP controller registers */
        for (i = 0; i < CTL_REG_COUNT; i++)
        {
            write_reg(rc1_ctl_addr[i], chk_val[j]);
        }

        /* Step 21: Write masked pattern to PCIE0 SII registers */
        for (i = 0; i < SII_REG_COUNT; i++)
        {
            write_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]);
        }

        /* Step 22: Write masked pattern to PCIE1 SII registers */
        for (i = 0; i < SII_REG_COUNT; i++)
        {
            write_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]);
        }

        /* Step 23: Write PHY reset control to PCIE0 */
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VALUE);

        /* Step 24: Write PHY reset control to PCIE1 */
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VALUE);

        /* Step 25: Write PHY-specific pattern to PCIE0 PHY registers */
        for (i = 0; i < PHY_REG_COUNT; i++)
        {
            write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }

        /* Step 26: Write PHY-specific pattern to PCIE1 PHY registers */
        for (i = 0; i < PHY_REG_COUNT; i++)
        {
            write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }

        /* Step 27: Read back PCIE0 DBI DSP controller registers */
        for (i = 0; i < CTL_REG_COUNT; i++)
        {
            data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: rc0_ctl[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, chk_val[j]);
#endif
            if (data_rd != chk_val[j])
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: rc0_ctl[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }

        /* Step 28: Read back PCIE1 DBI DSP controller registers */
        for (i = 0; i < CTL_REG_COUNT; i++)
        {
            data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: rc1_ctl[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, chk_val[j]);
#endif
            if (data_rd != chk_val[j])
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: rc1_ctl[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }

        /* Step 29: Read back PCIE0 SII registers with mask comparison */
        for (i = 0; i < SII_REG_COUNT; i++)
        {
            data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: sii0[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, (chk_val[j] & sii0_write_mask[i]));
#endif
            if (data_rd != (chk_val[j] & sii0_write_mask[i]))
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: sii0[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }

        /* Step 30: Read back PCIE1 SII registers with mask comparison */
        for (i = 0; i < SII_REG_COUNT; i++)
        {
            data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: sii1[%d] read = 0x%08X, expected = 0x%08X\n", i, data_rd, (chk_val[j] & sii1_write_mask[i]));
#endif
            if (data_rd != (chk_val[j] & sii1_write_mask[i]))
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: sii1[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }

        /* Step 31: Read back PCIE0 PHY registers with 16-bit extraction */
        for (i = 0; i < PHY_REG_COUNT; i++)
        {
            data_rd = read_reg(phy0_addr[i]);
            if ((phy0_addr[i] % 4) != 0)
            {
                data_rd = data_rd >> 16;
            }
            else
            {
                data_rd = data_rd & PHY_16BIT_MASK;
            }
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: phy0[%d] read(16-bit) = 0x%04X, expected = 0x%04X\n", i, (data_rd & phy0_write_mask[i]), (chk_val_phy[j] & PHY_WRITE_MASK_COMMON));
#endif
            if ((data_rd & phy0_write_mask[i]) != (chk_val_phy[j] & PHY_WRITE_MASK_COMMON))
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: phy0[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }

        /* Step 32: Read back PCIE1 PHY registers with 16-bit extraction */
        for (i = 0; i < PHY_REG_COUNT; i++)
        {
            data_rd = read_reg(phy1_addr[i]);
            if ((phy1_addr[i] % 4) != 0)
            {
                data_rd = data_rd >> 16;
            }
            else
            {
                data_rd = data_rd & PHY_16BIT_MASK;
            }
#ifdef DEBUG_DISPLAY
            debug_print("chk_rd_wr: phy1[%d] read(16-bit) = 0x%04X, expected = 0x%04X\n", i, (data_rd & phy1_write_mask[i]), (chk_val_phy[j] & PHY_WRITE_MASK_COMMON));
#endif
            if ((data_rd & phy1_write_mask[i]) != (chk_val_phy[j] & PHY_WRITE_MASK_COMMON))
            {
                (*p_err1)++;
#ifdef DEBUG_DISPLAY
                debug_print("FAIL: phy1[%d] write-readback mismatch, pattern %d\n", i, j);
#endif
            }
        }
    } /* end for j */
}

/********************************************************************
 * Function Name  : pcie_reg_wr_rd_test_init
 * Description    : Initialize testcase configuration and error counters.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful initialization.
 ********************************************************************/
int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Init] Testcase: %s\n", cfg->test_name);
#endif

    err1 = 0;
    err2 = 0;

    return 0;
}

/********************************************************************
 * Function Name  : pcie_reg_wr_rd_test_run
 * Description    : Execute testcase register reset-value verification
 *                  and write-read-back validation across DBI DSP
 *                  controller, SII interface, and PHY register groups.
 * Parameters     : cfg - Test configuration pointer,
 *                  out - Test output pointer.
 * Return Value   : Test execution status.
 ********************************************************************/
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[Test Run] Testcase: %s\n", cfg->test_name);
    debug_print("pcie_reg_wr_rd_test: Starting reset value check\n");
#endif

    /* Step 9-16: Reset value check */
    chk_rst_val(&err1, &err2);

#ifdef DEBUG_DISPLAY
    debug_print("pcie_reg_wr_rd_test: Reset check done. err1=%d, err2=%d\n", err1, err2);
    debug_print("pcie_reg_wr_rd_test: Starting write-read-back check\n");
#endif

    /* Step 17-33: Write-read-back check */
    chk_rd_wr(&err1);

#ifdef DEBUG_DISPLAY
    debug_print("pcie_reg_wr_rd_test: Write-read check done. err1=%d, err2=%d\n", err1, err2);
#endif

    return out->status;
}

/********************************************************************
 * Function Name  : pcie_reg_wr_rd_test_teardown
 * Description    : Perform output validation, error handling, and cleanup.
 *                  Evaluates combined error status and reports pass/fail.
 * Parameters     : cfg - Test configuration pointer.
 * Return Value   : 0 on successful teardown.
 ********************************************************************/
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

#ifdef DEBUG_DISPLAY
    LOGI("[TEARDOWN] Testcase: %s\n", cfg->test_name);
#endif

    /* ------------------------------------------------------------ */
    /* Step 34: Report test completion with combined error status    */
    /* ------------------------------------------------------------ */
#ifdef DEBUG_DISPLAY
    debug_print("Test complete. err1 = %d, err2 = %d\n", err1, err2);
#endif

    if ((err2 || err1) != 0)
    {
#ifdef DEBUG_DISPLAY
        LOGI("ERROR: pcie_reg_wr_rd_test FAILED with err1=%d, err2=%d\n", err1, err2);
#endif
    }
    else
    {
#ifdef DEBUG_DISPLAY
        LOGI("pcie_reg_wr_rd_test PASSED\n");
#endif
    }

    return 0;
}
