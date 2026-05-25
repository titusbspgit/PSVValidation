// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 * Testcase: pcie_reg_wr_rd_test
 * Description: Executes default checks, masked write/read-back for DBI-DSP and SII registers,
 *              as well as PHY halfword access checks as per Meta Test Steps.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Entry point for the testcase */
int test_case(void)
{
    int err1 = 0; // Error counter group 1
    int err2 = 0; // Error counter group 2
    unsigned int data_rd = 0;
    unsigned int data_wr = 0;
    int i = 0;
    int j = 0;

#ifdef DEBUG_DISPLAY
    printf("[DBG] Starting pcie_reg_wr_rd_test\n");
#endif

    // Defaults phase: RC0 control space
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] RC0 default idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err1++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] RC0 default mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        }
    }

    // Defaults phase: RC1 control space
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] RC1 default idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        if (data_rd != ctl_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] RC1 default mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
#endif
        }
    }

    // Defaults phase: SII0 space
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII0 default idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii0_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] SII0 default mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii0_addr[i], data_rd, sii_default[i]);
#endif
        }
    }

    // Defaults phase: SII1 space
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
#ifdef DEBUG_DISPLAY
        printf("[DBG] SII1 default idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii1_addr[i], data_rd, sii_default[i]);
#endif
        if (data_rd != sii_default[i]) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] SII1 default mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii1_addr[i], data_rd, sii_default[i]);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    // SII PHY reset assertion step mentioned in Meta; value not specified, so only logging here per non-inference rule
    printf("[DBG] SII PHY reset step acknowledged (no write due to unspecified reset value)\n");
#endif

    // Defaults phase: PHY0 halfword checks
    for (i = 0; i < 3; i++) {
        unsigned int val = read_reg(phy0_addr[i]);
        unsigned int half = ((phy0_addr[i] % 4) != 0) ? ((val >> 16) & 0x0000FFFF) : (val & 0x0000FFFF);
#ifdef DEBUG_DISPLAY
        printf("[DBG] PHY0 default idx=%d addr=0x%08X val=0x%08X half=0x%04X exp=0x%04X\n", i, phy0_addr[i], val, half, (phy0_default[i] & 0xFFFF));
#endif
        if (half != (phy0_default[i] & 0xFFFF)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] PHY0 default mismatch idx=%d addr=0x%08X half=0x%04X exp=0x%04X\n", i, phy0_addr[i], half, (phy0_default[i] & 0xFFFF));
#endif
        }
    }

    // Defaults phase: PHY1 halfword checks
    for (i = 0; i < 3; i++) {
        unsigned int val = read_reg(phy1_addr[i]);
        unsigned int half = ((phy1_addr[i] % 4) != 0) ? ((val >> 16) & 0x0000FFFF) : (val & 0x0000FFFF);
#ifdef DEBUG_DISPLAY
        printf("[DBG] PHY1 default idx=%d addr=0x%08X val=0x%08X half=0x%04X exp=0x%04X\n", i, phy1_addr[i], val, half, (phy1_default[i] & 0xFFFF));
#endif
        if (half != (phy1_default[i] & 0xFFFF)) {
            err2++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] PHY1 default mismatch idx=%d addr=0x%08X half=0x%04X exp=0x%04X\n", i, phy1_addr[i], half, (phy1_default[i] & 0xFFFF));
#endif
        }
    }

    // Write/Read phase (DBI-DSP and SII)
    {
        const unsigned int chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xFFFF0000};
        for (j = 0; j < 6; j++) {
            data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
            printf("[DBG] Pattern %d = 0x%08X\n", j, data_wr);
#endif
            // Writes to RC0/RC1 control registers
            for (i = 0; i < 5; i++) {
                write_reg(rc0_ctl_addr[i], data_wr);
                write_reg(rc1_ctl_addr[i], data_wr);
            }
            // Writes to SII with masks applied
            for (i = 0; i < 3; i++) {
                write_reg(sii0_addr[i], (data_wr & sii0_write_mask[i]));
                write_reg(sii1_addr[i], (data_wr & sii1_write_mask[i]));
            }
            // Read-back verify RC0/RC1
            for (i = 0; i < 5; i++) {
                data_rd = read_reg(rc0_ctl_addr[i]);
                if (data_rd != data_wr) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] RC0 rdwr mismatch idx=%d addr=0x%08X rd=0x%08X wr=0x%08X\n", i, rc0_ctl_addr[i], data_rd, data_wr);
#endif
                }
                data_rd = read_reg(rc1_ctl_addr[i]);
                if (data_rd != data_wr) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] RC1 rdwr mismatch idx=%d addr=0x%08X rd=0x%08X wr=0x%08X\n", i, rc1_ctl_addr[i], data_rd, data_wr);
#endif
                }
            }
            // Read-back verify SII masked writes
            for (i = 0; i < 3; i++) {
                unsigned int exp0 = (data_wr & sii0_write_mask[i]);
                unsigned int exp1 = (data_wr & sii1_write_mask[i]);
                data_rd = read_reg(sii0_addr[i]);
                if (data_rd != exp0) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] SII0 rdwr mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii0_addr[i], data_rd, exp0);
#endif
                }
                data_rd = read_reg(sii1_addr[i]);
                if (data_rd != exp1) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] SII1 rdwr mismatch idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n", i, sii1_addr[i], data_rd, exp1);
#endif
                }
            }
        }
    }

#ifdef DEBUG_DISPLAY
    // SII PHY reset step prior to PHY programming (value unspecified)
    printf("[DBG] SII PHY reset (pre-PHY programming) acknowledged (no write due to unspecified reset value)\n");
#endif

    // Write/Read phase (PHY)
    {
        const unsigned int chk_val_phy[3] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555};
        for (j = 0; j < 3; j++) {
            data_wr = chk_val_phy[j];
#ifdef DEBUG_DISPLAY
            printf("[DBG] PHY Pattern %d = 0x%08X\n", j, data_wr);
#endif
            // Program PHY0/PHY1 with masked values
            for (i = 0; i < 3; i++) {
                unsigned int wr0 = (data_wr & phy0_write_mask[i]);
                unsigned int wr1 = (data_wr & phy1_write_mask[i]);
                write_reg(phy0_addr[i], wr0);
                write_reg(phy1_addr[i], wr1);
            }
            // Verify PHY0/PHY1 halfword outcomes
            for (i = 0; i < 3; i++) {
                unsigned int rd0 = read_reg(phy0_addr[i]);
                unsigned int rd1 = read_reg(phy1_addr[i]);
                unsigned int hw0 = ((phy0_addr[i] % 4) != 0) ? ((rd0 >> 16) & 0x0000FFFF) : (rd0 & 0x0000FFFF);
                unsigned int hw1 = ((phy1_addr[i] % 4) != 0) ? ((rd1 >> 16) & 0x0000FFFF) : (rd1 & 0x0000FFFF);
                unsigned int exp_hw_masked0 = (data_wr & 0x00001FFF);
                unsigned int exp_hw_masked1 = (data_wr & 0x00001FFF);
                if ((hw0 & phy0_write_mask[i]) != exp_hw_masked0) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] PHY0 rdwr mismatch idx=%d addr=0x%08X hw=0x%04X exp=0x%04X mask=0x%04X\n", i, phy0_addr[i], hw0, exp_hw_masked0, phy0_write_mask[i]);
#endif
                }
                if ((hw1 & phy1_write_mask[i]) != exp_hw_masked1) {
                    err1++;
#ifdef DEBUG_DISPLAY
                    printf("[ERR] PHY1 rdwr mismatch idx=%d addr=0x%08X hw=0x%04X exp=0x%04X mask=0x%04X\n", i, phy1_addr[i], hw1, exp_hw_masked1, phy1_write_mask[i]);
#endif
                }
            }
        }
    }

    // Final result
    if ((err1 == 0) && (err2 == 0)) {
#ifdef DEBUG_DISPLAY
        printf("[PASS] pcie_reg_wr_rd_test completed successfully\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[FAIL] pcie_reg_wr_rd_test err1=%d err2=%d\n", err1, err2);
#endif
        finish(1);
    }

    // No alternative termination allowed; finish() is the terminal call.
    return 0;
}

#ifdef __cplusplus
}
#endif
