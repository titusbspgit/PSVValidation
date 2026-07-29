// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: test_case
 * Purpose: Execute PCIe register write/read validation per meta_json steps
 */
int test_case(void)
{
    unsigned int i = 0U;
    unsigned int j = 0U;
    unsigned int data_rd = 0U;
    unsigned int selected = 0U;
    unsigned int err1 = 0U;
    unsigned int err2 = 0U;

    /* Step 1: Initialize error counters */
    err1 = 0U;
    err2 = 0U;
#ifdef DEBUG_DISPLAY
    printf("[pcie_reg_wr_rd_test] Start: err1=%u err2=%u\n", err1, err2);
#endif

    /* Step 2: Reset default verification for RC0 control registers */
    for (i = 0U; i < 5U; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("RC0 CTL[%u] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err1++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch RC0 CTL[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, ctl_default[i]);
#endif
        }
    }

    /* Step 3: Reset default verification for RC1 control registers */
    for (i = 0U; i < 5U; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("RC1 CTL[%u] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch RC1 CTL[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, ctl_default[i]);
#endif
        }
    }

    /* Step 4: Reset default verification for SII0 registers */
    for (i = 0U; i < 3U; i++) {
        data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("SII0[%u] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii0_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch SII0[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, sii_default[i]);
#endif
        }
    }

    /* Step 5: Reset default verification for SII1 registers */
    for (i = 0U; i < 3U; i++) {
        data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("SII1[%u] addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii1_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch SII1[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, sii_default[i]);
#endif
        }
    }

    /* Step 6: Assert SII PHY reset control on both controllers */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000U);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000U);
#ifdef DEBUG_DISPLAY
    printf("Asserted PHY_RST: PCIE0=0x%08X PCIE1=0x%08X\n", mizar_PCIE0_SII_PHY_RST_CONTROL, mizar_PCIE1_SII_PHY_RST_CONTROL);
#endif

    /* Step 7: PHY default checks for PCIE0 (16-bit selection by address alignment) */
    for (i = 0U; i < 3U; i++) {
        data_rd = read_reg(phy0_addr[i]);
        if ((phy0_addr[i] % 4U) != 0U) {
            selected = (data_rd >> 16) & 0x0000FFFFU; /* upper half */
        } else {
            selected = data_rd & 0x0000FFFFU; /* lower half */
        }
#ifdef DEBUG_DISPLAY
        printf("PHY0[%u] addr=0x%08X rd=0x%08X sel16=0x%04X exp=0x%04X\n", i, phy0_addr[i], data_rd, selected, phy0_default[i] & 0xFFFFU);
#endif
        if (selected != (phy0_default[i] & 0xFFFFU)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch PHY0[%u]: sel16=0x%04X exp=0x%04X\n", i, selected, phy0_default[i] & 0xFFFFU);
#endif
        }
    }

    /* Step 8: PHY default checks for PCIE1 (16-bit selection by address alignment) */
    for (i = 0U; i < 3U; i++) {
        data_rd = read_reg(phy1_addr[i]);
        if ((phy1_addr[i] % 4U) != 0U) {
            selected = (data_rd >> 16) & 0x0000FFFFU; /* upper half */
        } else {
            selected = data_rd & 0x0000FFFFU; /* lower half */
        }
#ifdef DEBUG_DISPLAY
        printf("PHY1[%u] addr=0x%08X rd=0x%08X sel16=0x%04X exp=0x%04X\n", i, phy1_addr[i], data_rd, selected, phy1_default[i] & 0xFFFFU);
#endif
        if (selected != (phy1_default[i] & 0xFFFFU)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("Mismatch PHY1[%u]: sel16=0x%04X exp=0x%04X\n", i, selected, phy1_default[i] & 0xFFFFU);
#endif
        }
    }

    /* Step 9: Patterned write/read-back verification */
    {
        unsigned int chk_val[3] = {0xFFFFFFFFU, 0xAAAAAAA AU, 0x55555555U};
        unsigned int chk_val_phy[3] = {0x7BAFU, 0x0001U, 0x003BU};
        for (j = 0U; j < 3U; j++) {
#ifdef DEBUG_DISPLAY
            printf("Iteration j=%u: chk_val=0x%08X chk_val_phy=0x%04X\n", j, chk_val[j], chk_val_phy[j]);
#endif
            /* 9.1: Write RC0 control registers */
            for (i = 0U; i < 5U; i++) {
                write_reg(rc0_ctl_addr[i], chk_val[j]);
            }
            /* 9.2: Write RC1 control registers */
            for (i = 0U; i < 5U; i++) {
                write_reg(rc1_ctl_addr[i], chk_val[j]);
            }
            /* 9.3: Write SII0 registers with masks */
            for (i = 0U; i < 3U; i++) {
                write_reg(sii0_addr[i], (chk_val[j] & sii0_write_mask[i]));
            }
            /* 9.4: Write SII1 registers with masks */
            for (i = 0U; i < 3U; i++) {
                write_reg(sii1_addr[i], (chk_val[j] & sii1_write_mask[i]));
            }
            /* 9.5: Re-assert SII PHY reset control on both */
            write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000U);
            write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000U);
            /* 9.6: Write PHY0 registers with masks */
            for (i = 0U; i < 3U; i++) {
                write_reg(phy0_addr[i], (chk_val_phy[j] & phy0_write_mask[i]));
            }
            /* 9.7: Write PHY1 registers with masks */
            for (i = 0U; i < 3U; i++) {
                write_reg(phy1_addr[i], (chk_val_phy[j] & phy1_write_mask[i]));
            }
            /* 9.8: Read-back RC0 control registers */
            for (i = 0U; i < 5U; i++) {
                data_rd = read_reg(rc0_ctl_addr[i]);
                if (data_rd != chk_val[j]) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch RC0 CTL[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, chk_val[j]);
#endif
                }
            }
            /* 9.9: Read-back RC1 control registers */
            for (i = 0U; i < 5U; i++) {
                data_rd = read_reg(rc1_ctl_addr[i]);
                if (data_rd != chk_val[j]) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch RC1 CTL[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, chk_val[j]);
#endif
                }
            }
            /* 9.10: Read-back SII0 registers */
            for (i = 0U; i < 3U; i++) {
                data_rd = read_reg(sii0_addr[i]);
                if (data_rd != (chk_val[j] & sii0_write_mask[i])) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch SII0[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, (chk_val[j] & sii0_write_mask[i]));
#endif
                }
            }
            /* 9.11: Read-back SII1 registers */
            for (i = 0U; i < 3U; i++) {
                data_rd = read_reg(sii1_addr[i]);
                if (data_rd != (chk_val[j] & sii1_write_mask[i])) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch SII1[%u]: got 0x%08X exp 0x%08X\n", i, data_rd, (chk_val[j] & sii1_write_mask[i]));
#endif
                }
            }
            /* 9.12: Read-back PHY0 registers with 16-bit selection and mask */
            for (i = 0U; i < 3U; i++) {
                data_rd = read_reg(phy0_addr[i]);
                if ((phy0_addr[i] % 4U) != 0U) {
                    selected = (data_rd >> 16) & 0x0000FFFFU; /* upper half */
                } else {
                    selected = data_rd & 0x0000FFFFU; /* lower half */
                }
                if ((selected & phy0_write_mask[i]) != (chk_val_phy[j] & 0x00001FFFU)) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch PHY0[%u]: sel16=0x%04X exp=0x%04X mask=0x%04X\n", i, selected & phy0_write_mask[i], chk_val_phy[j] & 0x1FFFU, phy0_write_mask[i] & 0xFFFFU);
#endif
                }
            }
            /* 9.13: Read-back PHY1 registers with 16-bit selection and mask */
            for (i = 0U; i < 3U; i++) {
                data_rd = read_reg(phy1_addr[i]);
                if ((phy1_addr[i] % 4U) != 0U) {
                    selected = (data_rd >> 16) & 0x0000FFFFU; /* upper half */
                } else {
                    selected = data_rd & 0x0000FFFFU; /* lower half */
                }
                if ((selected & phy1_write_mask[i]) != (chk_val_phy[j] & 0x00001FFFU)) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("RB Mismatch PHY1[%u]: sel16=0x%04X exp=0x%04X mask=0x%04X\n", i, selected & phy1_write_mask[i], chk_val_phy[j] & 0x1FFFU, phy1_write_mask[i] & 0xFFFFU);
#endif
                }
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[pcie_reg_wr_rd_test] Complete: err1=%u err2=%u => result=%s\n", err1, err2, ((err2 || err1) ? "FAIL" : "PASS"));
#endif

    /* Step 10: Terminate with PASS/FAIL */
    if ((err2 != 0U) || (err1 != 0U)) {
        finish(1);
    } else {
        finish(0);
    }

    return 0; /* Unreachable due to finish() */
}
