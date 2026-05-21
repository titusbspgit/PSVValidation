// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* ============================================
 * Function: chk_rst_val
 * Description: Perform reset/default value checks for DBI, SII and PHY arrays.
 * ============================================ */
static void chk_rst_val(void)
{
    int i;
    unsigned int data_rd;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rst_val: Begin reset checks\n");
#endif

    /* RC0 DBI control default checks */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] RC0_CTL[%d] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err1++;
        }
    }

    /* RC1 DBI control default checks */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] RC1_CTL[%d] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err1++;
        }
    }

    /* SII0 default checks */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII0[%d] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii0_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
        }
    }

    /* SII1 default checks */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII1[%d] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii1_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
        }
    }

    /* Write SII PHY RST CONTROL for both controllers */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Wrote SII PHY RST CONTROL: 0x01203000 to both controllers\n");
#endif

    /* PHY0 default checks with 16-bit selection based on (addr % 4) */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy0_addr[i]);
        unsigned int half = ((phy0_addr[i] % 4) == 0) ? (data_rd & 0xFFFFu) : ((data_rd >> 16) & 0xFFFFu);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] PHY0[%d] addr=0x%08X rd=0x%08X sel16=0x%04X exp=0x%08X\n", i, phy0_addr[i], data_rd, half, phy0_default[i]);
#endif
        if (half != (phy0_default[i] & 0xFFFFu)) {
            err2++;
        }
    }

    /* PHY1 default checks with 16-bit selection based on (addr % 4) */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy1_addr[i]);
        unsigned int half = ((phy1_addr[i] % 4) == 0) ? (data_rd & 0xFFFFu) : ((data_rd >> 16) & 0xFFFFu);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] PHY1[%d] addr=0x%08X rd=0x%08X sel16=0x%04X exp=0x%08X\n", i, phy1_addr[i], data_rd, half, phy1_default[i]);
#endif
        if (half != (phy1_default[i] & 0xFFFFu)) {
            err2++;
        }
    }
}

/* ============================================
 * Function: chk_rd_wr
 * Description: Perform write-then-read validation across DBI, SII and PHY blocks.
 * ============================================ */
static void chk_rd_wr(void)
{
    int i, j;
    unsigned int data_rd;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rd_wr: Begin pattern write/read checks\n");
#endif

    for (j = 0; j < 3; j++) {
        /* Write patterns to RC0/RC1 DBI control registers */
        for (i = 0; i < 5; i++) {
            write_reg(rc0_ctl_addr[i], (unsigned int)chk_val[j]);
            write_reg(rc1_ctl_addr[i], (unsigned int)chk_val[j]);
        }
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Wrote DBI patterns chk_val[%d]=0x%08X to RC0/RC1\n", j, (unsigned int)chk_val[j]);
#endif

        /* Write masked patterns to SII registers */
        for (i = 0; i < 3; i++) {
            write_reg(sii0_addr[i], ((unsigned int)chk_val[j]) & sii0_write_mask[i]);
            write_reg(sii1_addr[i], ((unsigned int)chk_val[j]) & sii1_write_mask[i]);
        }
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Wrote SII masked patterns for j=%d\n", j);
#endif

        /* Write SII PHY RST CONTROL value for both controllers */
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Wrote SII PHY RST CONTROL during chk_rd_wr for j=%d\n", j);
#endif

        /* Write masked PHY patterns */
        for (i = 0; i < 3; i++) {
            write_reg(phy0_addr[i], ((unsigned int)chk_val_phy[j]) & phy0_write_mask[i]);
            write_reg(phy1_addr[i], ((unsigned int)chk_val_phy[j]) & phy1_write_mask[i]);
        }
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Wrote PHY masked patterns chk_val_phy[%d]=0x%08X\n", j, (unsigned int)chk_val_phy[j]);
#endif

        /* Verify DBI control writes */
        for (i = 0; i < 5; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                err1++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] RC0_CTL verify i=%d rd=0x%08X exp=0x%08X\n", i, data_rd, (unsigned int)chk_val[j]);
#endif
            }
            data_rd = read_reg(rc1_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                err1++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] RC1_CTL verify i=%d rd=0x%08X exp=0x%08X\n", i, data_rd, (unsigned int)chk_val[j]);
#endif
            }
        }

        /* Verify SII writes (masked) */
        for (i = 0; i < 3; i++) {
            unsigned int exp0 = ((unsigned int)chk_val[j]) & sii0_write_mask[i];
            unsigned int exp1 = ((unsigned int)chk_val[j]) & sii1_write_mask[i];
            data_rd = read_reg(sii0_addr[i]);
            if (data_rd != exp0) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] SII0 verify i=%d rd=0x%08X exp=0x%08X\n", i, data_rd, exp0);
#endif
            }
            data_rd = read_reg(sii1_addr[i]);
            if (data_rd != exp1) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] SII1 verify i=%d rd=0x%08X exp=0x%08X\n", i, data_rd, exp1);
#endif
            }
        }

        /* Verify PHY reads (16-bit selection + mask compare) */
        for (i = 0; i < 3; i++) {
            unsigned int exp_phy = ((unsigned int)chk_val_phy[j]) & 0x00001FFFu;

            data_rd = read_reg(phy0_addr[i]);
            unsigned int half0 = ((phy0_addr[i] % 4) == 0) ? (data_rd & 0xFFFFu) : ((data_rd >> 16) & 0xFFFFu);
            if ( (half0 & phy0_write_mask[i]) != exp_phy ) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] PHY0 verify i=%d rd=0x%08X sel16=0x%04X exp=0x%04X mask=0x%04X\n", i, data_rd, half0, exp_phy, phy0_write_mask[i]);
#endif
            }

            data_rd = read_reg(phy1_addr[i]);
            unsigned int half1 = ((phy1_addr[i] % 4) == 0) ? (data_rd & 0xFFFFu) : ((data_rd >> 16) & 0xFFFFu);
            if ( (half1 & phy1_write_mask[i]) != exp_phy ) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] PHY1 verify i=%d rd=0x%08X sel16=0x%04X exp=0x%04X mask=0x%04X\n", i, data_rd, half1, exp_phy, phy1_write_mask[i]);
#endif
            }
        }
    }
}

/* ============================================
 * Function: test_case
 * Description: Entry point executing reset checks and write/read validations.
 * ============================================ */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Entering test_case: pcie_reg_wr_rd_test\n");
#endif

    /* Perform reset validations */
    chk_rst_val();

    /* Perform write/read validations */
    chk_rd_wr();

    /* Terminate with PASS/FAIL as per accumulated errors */
    if (err1 || err2) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST RESULT: FAIL (err1=%d, err2=%d)\n", err1, err2);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST RESULT: PASS\n");
#endif
        finish(0);
    }

    return 0; /* Unreachable due to finish() */
}
