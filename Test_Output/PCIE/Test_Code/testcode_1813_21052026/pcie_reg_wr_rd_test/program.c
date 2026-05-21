// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

/*
 * Auto-generated program for test: pcie_reg_wr_rd_test
 * Converts Meta Test Steps into executable C logic without reordering
 * or optimization. Uses only impacted registers and arrays declared in test_define.c.
 */

#include "test_define.c"  /* Only include as per rules */

/* Forward declarations */
static void chk_rst_val(void);
static void chk_rd_wr(void);

/* Local state */
static int err1 = 0;  /* DBI control errors */
static int err2 = 0;  /* SII/PHY errors */

/*
 * Function: chk_rst_val
 * Purpose: Check defaults for RC0/RC1 DBI control, SII blocks, and PHY arrays.
 */
static void chk_rst_val(void)
{
    int i;
    unsigned int rd;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rst_val()\n");
#endif

    /* RC0 defaults */
    for (i = 0; i < 5; i++) {
        rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] RC0 i=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, rc0_ctl_addr[i], rd, ctl_default[i]);
#endif
        if (rd != ctl_default[i]) err1++;
    }

    /* RC1 defaults */
    for (i = 0; i < 5; i++) {
        rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] RC1 i=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, rc1_ctl_addr[i], rd, ctl_default[i]);
#endif
        if (rd != ctl_default[i]) err1++;
    }

    /* SII defaults */
    for (i = 0; i < 3; i++) {
        rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII0 i=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, sii0_addr[i], rd, sii_default[i]);
#endif
        if (rd != sii_default[i]) err2++;
    }
    for (i = 0; i < 3; i++) {
        rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SII1 i=%d addr=0x%08x rd=0x%08x exp=0x%08x\n", i, sii1_addr[i], rd, sii_default[i]);
#endif
        if (rd != sii_default[i]) err2++;
    }

    /* Write SII PHY RST CONTROL for both RCs */
    write_reg(mizar_PCIE0_SII_PHY RST CONTROL, 0x01203000);
    write_reg(mizar_PCIE1_SII_PHY RST CONTROL, 0x01203000);

    /* PHY defaults with halfword selection based on addr%4 */
    for (i = 0; i < 3; i++) {
        rd = read_reg(phy0_addr[i]);
        if ((phy0_addr[i] % 4) == 0)
            rd = (rd >> 16) & 0xFFFF;
        else
            rd = rd & 0xFFFF;
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] PHY0 i=%d addr=0x%08x half_rd=0x%04x exp=0x%04x\n", i, phy0_addr[i], rd, phy0_default[i]);
#endif
        if ((rd & phy0_write_mask[i]) != (phy0_default[i] & 0x1FFF)) err2++;
    }
    for (i = 0; i < 3; i++) {
        rd = read_reg(phy1_addr[i]);
        if ((phy1_addr[i] % 4) == 0)
            rd = (rd >> 16) & 0xFFFF;
        else
            rd = rd & 0xFFFF;
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] PHY1 i=%d addr=0x%08x half_rd=0x%04x exp=0x%04x\n", i, phy1_addr[i], rd, phy1_default[i]);
#endif
        if ((rd & phy1_write_mask[i]) != (phy1_default[i] & 0x1FFF)) err2++;
    }
}

/*
 * Function: chk_rd_wr
 * Purpose: Write/verify sequences for DBI, SII, and PHY arrays.
 */
static void chk_rd_wr(void)
{
    int i, j;
    unsigned int rd;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rd_wr()\n");
#endif

    for (j = 0; j < 6; j++) {
        /* DBI writes */
        for (i = 0; i < 5; i++) write_reg(rc0_ctl_addr[i], chk_val[j]);
        for (i = 0; i < 5; i++) write_reg(rc1_ctl_addr[i], chk_val[j]);

        /* SII writes (masked) */
        for (i = 0; i < 3; i++) write_reg(sii0_addr[i], (chk_val[j] & sii0_write_mask[i]));
        for (i = 0; i < 3; i++) write_reg(sii1_addr[i], (chk_val[j] & sii1_write_mask[i]));

        /* Pulse SII PHY RST CONTROL */
        write_reg(mizar_PCIE0_SII_PHY RST CONTROL, 0x01203000);
        write_reg(mizar_PCIE1_SII_PHY RST CONTROL, 0x01203000);

        /* Verify DBI */
        for (i = 0; i < 5; i++) {
            rd = read_reg(rc0_ctl_addr[i]);
            if (rd != (unsigned int)chk_val[j]) err1++;
        }
        for (i = 0; i < 5; i++) {
            rd = read_reg(rc1_ctl_addr[i]);
            if (rd != (unsigned int)chk_val[j]) err1++;
        }

        /* Verify SII (masked) */
        for (i = 0; i < 3; i++) {
            rd = read_reg(sii0_addr[i]);
            if (rd != ((unsigned int)chk_val[j] & (unsigned int)sii0_write_mask[i])) err2++;
        }
        for (i = 0; i < 3; i++) {
            rd = read_reg(sii1_addr[i]);
            if (rd != ((unsigned int)chk_val[j] & (unsigned int)sii1_write_mask[i])) err2++;
        }

        /* PHY masked writes and verifies using 16-bit halves */
        for (i = 0; i < 3; i++) write_reg(phy0_addr[i], (chk_val_phy[j % 3] & phy0_write_mask[i]));
        for (i = 0; i < 3; i++) write_reg(phy1_addr[i], (chk_val_phy[j % 3] & phy1_write_mask[i]));

        for (i = 0; i < 3; i++) {
            rd = read_reg(phy0_addr[i]);
            if ((phy0_addr[i] % 4) == 0)
                rd = (rd >> 16) & 0xFFFF;
            else
                rd = rd & 0xFFFF;
            if ((rd & phy0_write_mask[i]) != (chk_val_phy[j % 3] & 0x1FFF)) err2++;
        }
        for (i = 0; i < 3; i++) {
            rd = read_reg(phy1_addr[i]);
            if ((phy1_addr[i] % 4) == 0)
                rd = (rd >> 16) & 0xFFFF;
            else
                rd = rd & 0xFFFF;
            if ((rd & phy1_write_mask[i]) != (chk_val_phy[j % 3] & 0x1FFF)) err2++;
        }
    }
}

/* Entry point */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] >>> Enter test_case: pcie_reg_wr_rd_test <<<\n");
#endif

    chk_rst_val();
    chk_rd_wr();

    if (err2 || err1) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST FAIL err1=%d err2=%d\n", err1, err2);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST PASS\n");
#endif
        finish(0);
    }

    return 0; /* Unreached if finish() terminates */
}
