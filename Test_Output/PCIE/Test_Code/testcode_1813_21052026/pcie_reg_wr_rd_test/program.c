// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* Banner: Test entry point and helpers for pcie_reg_wr_rd_test */
static void chk_rst_val(void);
static void chk_rd_wr(void);

/* Error counters */
static int err1 = 0; /* DBI control errors */
static int err2 = 0; /* SII/PHY errors */

/* Inline helpers for readability */
static inline unsigned int select_halfword(unsigned int addr, unsigned int data)
{
    /* Select upper or lower 16 bits depending on (addr % 4) */
    if ((addr & 0x2U) != 0U) {
        return (data >> 16) & 0xFFFFU; /* upper 16-bits */
    } else {
        return data & 0xFFFFU; /* lower 16-bits */
    }
}

/* Test entry point */
// ------------------------------------------------------------
// Function: test_case
// Purpose : Execute reset/default checks and masked R/W checks
// ------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[pcie_reg_wr_rd_test] Enter test_case()\n");
#endif

    chk_rst_val();      /* Phase 1: Reset/default validation */
    chk_rd_wr();        /* Phase 2: Masked write/readback */

#ifdef DEBUG_DISPLAY
    printf("[pcie_reg_wr_rd_test] Exit test_case(): err1=%d err2=%d -> %s\n", err1, err2, (err1||err2)?"FAIL":"PASS");
#endif

    if (err2 || err1) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }

    return 0; /* Unreachable due to finish(); keeps compiler happy if needed */
}

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Validate default/reset values per Meta Steps
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    int i;
    unsigned int data_rd;

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for DBI RC0\n");
#endif
    /* RC0 DBI control defaults */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            err1++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][RC0][%d] exp=0x%08X act=0x%08X\n", i, ctl_default[i], data_rd);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for DBI RC1\n");
#endif
    /* RC1 DBI control defaults */
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            err1++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][RC1][%d] exp=0x%08X act=0x%08X\n", i, ctl_default[i], data_rd);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for SII RC0\n");
#endif
    /* SII0 defaults */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][SII0][%d] exp=0x%08X act=0x%08X\n", i, sii_default[i], data_rd);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for SII RC1\n");
#endif
    /* SII1 defaults */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][SII1][%d] exp=0x%08X act=0x%08X\n", i, sii_default[i], data_rd);
#endif
        }
    }

    /* Apply PHY reset controls for both controllers as per Meta Steps */
    write_reg(mizar_PCIE0_SII_PHY RST CONTROL, 0x01203000);
    write_reg(mizar_PCIE1_SII_PHY RST CONTROL, 0x01203000);

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for PHY0\n");
#endif
    /* PHY0 defaults (compare selected 16-bit value) */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy0_addr[i]);
        unsigned int sel = select_halfword(phy0_addr[i], data_rd);
        if (sel != (phy0_default[i] & 0xFFFFU)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][PHY0][%d] exp=0x%04X act=0x%04X (raw=0x%08X)\n", i, (phy0_default[i] & 0xFFFFU), sel, data_rd);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[chk_rst_val] Start default checks for PHY1\n");
#endif
    /* PHY1 defaults (compare selected 16-bit value) */
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(phy1_addr[i]);
        unsigned int sel = select_halfword(phy1_addr[i], data_rd);
        if (sel != (phy1_default[i] & 0xFFFFU)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[chk_rst_val][PHY1][%d] exp=0x%04X act=0x%04X (raw=0x%08X)\n", i, (phy1_default[i] & 0xFFFFU), sel, data_rd);
#endif
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Execute masked write/readback per Meta Steps
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    int i, j;
    unsigned int data_rd;

    /* Iterate j=0..2 as specified (paired DBI/SII pattern and PHY pattern) */
    for (j = 0; j < 3; j++) {
#ifdef DEBUG_DISPLAY
        printf("[chk_rd_wr] Iteration j=%d: DBI/SII pattern=0x%08X, PHY pattern=0x%04X\n", j, (unsigned int)chk_val[j], (unsigned int)(chk_val_phy[j] & 0xFFFF));
#endif
        /* Write DBI control arrays (RC0, RC1) */
        for (i = 0; i < 5; i++) {
            write_reg(rc0_ctl_addr[i], (unsigned int)chk_val[j]);
        }
        for (i = 0; i < 5; i++) {
            write_reg(rc1_ctl_addr[i], (unsigned int)chk_val[j]);
        }

        /* Write SII arrays with masks */
        for (i = 0; i < 3; i++) {
            write_reg(sii0_addr[i], ((unsigned int)chk_val[j]) & sii0_write_mask[i]);
        }
        for (i = 0; i < 3; i++) {
            write_reg(sii1_addr[i], ((unsigned int)chk_val[j]) & sii1_write_mask[i]);
        }

        /* Re-assert PHY reset controls as specified */
        write_reg(mizar_PCIE0_SII_PHY RST CONTROL, 0x01203000);
        write_reg(mizar_PCIE1_SII_PHY RST CONTROL, 0x01203000);

        /* Write PHY arrays with masks */
        for (i = 0; i < 3; i++) {
            write_reg(phy0_addr[i], ((unsigned int)chk_val_phy[j]) & phy0_write_mask[i]);
        }
        for (i = 0; i < 3; i++) {
            write_reg(phy1_addr[i], ((unsigned int)chk_val_phy[j]) & phy1_write_mask[i]);
        }

        /* Verify DBI control arrays (RC0, RC1) */
        for (i = 0; i < 5; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                err1++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][RC0][%d] exp=0x%08X act=0x%08X\n", i, (unsigned int)chk_val[j], data_rd);
#endif
            }
        }
        for (i = 0; i < 5; i++) {
            data_rd = read_reg(rc1_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                err1++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][RC1][%d] exp=0x%08X act=0x%08X\n", i, (unsigned int)chk_val[j], data_rd);
#endif
            }
        }

        /* Verify SII arrays with masks */
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(sii0_addr[i]);
            if (data_rd != ((((unsigned int)chk_val[j]) & sii0_write_mask[i]))) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][SII0][%d] exp=0x%08X act=0x%08X\n", i, (((unsigned int)chk_val[j]) & sii0_write_mask[i]), data_rd);
#endif
            }
        }
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(sii1_addr[i]);
            if (data_rd != ((((unsigned int)chk_val[j]) & sii1_write_mask[i]))) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][SII1][%d] exp=0x%08X act=0x%08X\n", i, (((unsigned int)chk_val[j]) & sii1_write_mask[i]), data_rd);
#endif
            }
        }

        /* Verify PHY arrays: select 16-bit and compare masked */
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(phy0_addr[i]);
            unsigned int sel = select_halfword(phy0_addr[i], data_rd) & 0xFFFFU;
            if ( (sel & phy0_write_mask[i]) != (((unsigned int)chk_val_phy[j]) & 0x00001FFFU) ) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][PHY0][%d] exp=0x%04X act=0x%04X mask=0x%04X raw=0x%08X\n", i, ((unsigned int)chk_val_phy[j]) & 0x1FFFU, sel & 0xFFFFU, phy0_write_mask[i] & 0xFFFFU, data_rd);
#endif
            }
        }
        for (i = 0; i < 3; i++) {
            data_rd = read_reg(phy1_addr[i]);
            unsigned int sel = select_halfword(phy1_addr[i], data_rd) & 0xFFFFU;
            if ( (sel & phy1_write_mask[i]) != (((unsigned int)chk_val_phy[j]) & 0x00001FFFU) ) {
                err2++;
#ifdef DEBUG_DISPLAY
                printf("[chk_rd_wr][PHY1][%d] exp=0x%04X act=0x%04X mask=0x%04X raw=0x%08X\n", i, ((unsigned int)chk_val_phy[j]) & 0x1FFFU, sel & 0xFFFFU, phy1_write_mask[i] & 0xFFFFU, data_rd);
#endif
            }
        }
    }
}
