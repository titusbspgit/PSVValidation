// Author - AI Force 1.3.2. Date 30-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* ------------------------------------------------------------
 * Function: chk_rst_val
 * Purpose : Validate default/reset values for DBI, SII and PHY regs
 * ------------------------------------------------------------ */
static void chk_rst_val(void)
{
    /* Check RC0 DBI control address defaults */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] RC0 DBI idx=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err1++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n", ctl_default[i], data_rd);
        }
    }

    /* Check RC1 DBI control address defaults */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] RC1 DBI idx=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n", ctl_default[i], data_rd);
        }
    }

    /* Check SII0 defaults */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII0 idx=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, sii0_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n", sii_default[i], data_rd);
        }
    }

    /* Check SII1 defaults */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII1 idx=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, sii1_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
            printf("Data mismatch => Default value : 0x%x, Read data : 0x%x : FAILED\n", sii_default[i], data_rd);
        }
    }

    /* Write PHY reset control */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);

    /* PHY0 16-bit field checks */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy0_addr[i]);
        data_rd = (phy0_addr[i] % 4) ? (data_rd >> 16) : (data_rd & 0x0000FFFF);
#ifdef DEBUG_DISPLAY
        printf("[DBG] PHY0 idx=%d addr=0x%08x field=0x%04x exp=0x%04x\n", i, phy0_addr[i], (unsigned)(data_rd & 0xFFFF), (unsigned)(phy0_default[i] & 0xFFFF));
#endif
        if (data_rd != phy0_default[i]) {
            err2++;
            printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n", phy0_addr[i], phy0_default[i], data_rd);
        }
    }

    /* PHY1 16-bit field checks */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy1_addr[i]);
        data_rd = (phy1_addr[i] % 4) ? (data_rd >> 16) : (data_rd & 0x0000FFFF);
#ifdef DEBUG_DISPLAY
        printf("[DBG] PHY1 idx=%d addr=0x%08x field=0x%04x exp=0x%04x\n", i, phy1_addr[i], (unsigned)(data_rd & 0xFFFF), (unsigned)(phy1_default[i] & 0xFFFF));
#endif
        if (data_rd != phy1_default[i]) {
            err2++;
            printf("Reset value mismatch => Reg address : 0x%x, Default value : 0x%x, Read data : 0x%x : FAILED\n", phy1_addr[i], phy1_default[i], data_rd);
        }
    }
}

/* ------------------------------------------------------------
 * Function: chk_rd_wr
 * Purpose : Perform write/readback tests for DBI, SII and PHY regs
 * ------------------------------------------------------------ */
static void chk_rd_wr(void)
{
    int chk_val[6]     = {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000};
    int chk_val_phy[3] = {0x7baf,0x1,0x003b};

    for (j = 0; j < 3; j++) {
#ifdef DEBUG_DISPLAY
        printf("[DBG] Pattern iteration j=%d pat=0x%08x phy=0x%04x\n", j, chk_val[j], chk_val_phy[j]);
#endif
        /* DBI writes */
        for (i = 0; i < 5; i++) { write_reg(rc0_ctl_addr[i], chk_val[j]); }
        for (i = 0; i < 5; i++) { write_reg(rc1_ctl_addr[i], chk_val[j]); }

        /* SII writes with masks */
        for (i = 0; i < 3; i++) { write_reg(sii0_addr[i], (chk_val[j] & sii0_write_mask[i])); }
        for (i = 0; i < 3; i++) { write_reg(sii1_addr[i], (chk_val[j] & sii1_write_mask[i])); }

        /* Re-assert PHY reset control as per steps */
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);

        /* PHY masked writes (lower 13 bits significant) */
        for (i = 0; i < 3; i++) { write_reg(phy0_addr[i], (chk_val_phy[j] & phy0_write_mask[i])); }
        for (i = 0; i < 3; i++) { write_reg(phy1_addr[i], (chk_val_phy[j] & phy1_write_mask[i])); }

        /* Readbacks: DBI must equal written pattern */
        for (i = 0; i < 5; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n", chk_val[j], data_rd);
            }
        }
        for (i = 0; i < 5; i++) {
            data_rd = read_reg(rc1_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n", chk_val[j], data_rd);
            }
        }

        /* Readbacks: SII must equal masked pattern */
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(sii0_addr[i]);
            if (data_rd != (chk_val[j] & sii0_write_mask[i])) {
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n", chk_val[j], data_rd);
            }
        }
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(sii1_addr[i]);
            if (data_rd != (chk_val[j] & sii1_write_mask[i])) {
                err1++;
                printf("Data mismatch => Write data : 0x%x, Read data : 0x%x : FAILED\n", chk_val[j], data_rd);
            }
        }

        /* Readbacks: PHY field extraction and 13-bit compare */
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(phy0_addr[i]);
            data_rd = (phy0_addr[i] % 4) ? (data_rd >> 16) : (data_rd & 0x0000FFFF);
            if ((data_rd & phy0_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF)) {
                err1++;
                printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n", phy0_addr[i], chk_val_phy[j], data_rd);
            }
        }
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(phy1_addr[i]);
            data_rd = (phy1_addr[i] % 4) ? (data_rd >> 16) : (data_rd & 0x0000FFFF);
            if ((data_rd & phy1_write_mask[i]) != (chk_val_phy[j] & 0x00001FFF)) {
                err1++;
                printf("Data mismatch => Reg address : 0x%x, Write data : 0x%x, Read data : 0x%x : FAILED\n", phy1_addr[i], chk_val[j], data_rd);
            }
        }
    }
}

/* ------------------------------------------------------------
 * Function: test_case
 * Purpose : Entry point for testcase execution
 * ------------------------------------------------------------ */
int test_case(void)
{
    printf("Entered test case\n");
#ifdef DEBUG_DISPLAY
    printf("[DBG] Starting chk_rst_val()\n");
#endif
    chk_rst_val();

    printf("READ_WRITE test_case called\n");
#ifdef DEBUG_DISPLAY
    printf("[DBG] Starting chk_rd_wr()\n");
#endif
    chk_rd_wr();

    /* Terminate with PASS/FAIL based on accumulated errors */
    {
        int status = (err2 || err1) ? 1 : 0;
#ifdef DEBUG_DISPLAY
        printf("[DBG] Test completed. err1=%d err2=%d status=%d\n", err1, err2, status);
#endif
        finish(status);
        return status;
    }
}
