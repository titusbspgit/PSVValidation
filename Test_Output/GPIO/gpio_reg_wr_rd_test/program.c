// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 High-level Description (from Hidden_Test_Description):
 Program exercises two phases: (1) Default value check for all entries in addr_array[] subject to skip_rst_array[] and read_mask_array[]; (2) Write/read check using six patterns for all entries in addr_array[] subject to skip_array[], write_mask_array[], and read_mask_array[]. In chk_rst_val(): for i=0..CNT-1, if skip_rst_array[i]==1 then continue; if read_mask_array[i]==0x00000000 then continue; data_rd=read_reg(addr_array[i]); data=(data_rd & 0xfffffffe); compare data == default_value_array[i]; on mismatch, def_fail_cnt++. In chk_rd_wr(): for each data pattern in chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}, write phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; write_reg(addr_array[i], (data_wr & write_mask_array[i])). Read/verify phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; if read_mask_array[i]==0x0 continue; data_rd = (read_reg(addr_array[i]) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd != exp_val) wr_fail_cnt++. At end of test_case(): finish(1) if (def_fail_cnt>0 || wr_fail_cnt>0) else finish(0).
*/

/*
 * Function: main
 * Purpose : Execute GPIO register reset-value checks and masked write/read verification
 *           strictly per test steps. Terminates via finish(0/1) based on acceptance criteria.
 */
int main(void)
{
    unsigned int i, j;
    unsigned int def_fail_cnt = 0U;
    unsigned int wr_fail_cnt  = 0U;

    /* Phase 1: Default value check loop */
    for (i = 0U; i < CNT; i++) {
        if (skip_rst_array[i] == 1U) {
            continue; /* Skipped per reset-skip array */
        }
        if (read_mask_array[i] == 0x00000000U) {
            continue; /* No readable bits */
        }
        /* Read register and mask per requirement (mask off bit0) */
        unsigned int data_rd = read_reg(addr_array[i]); /* Register read */
        unsigned int data = (data_rd & 0xFFFFFFFEU);
        if (data != default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEFCHK][FAIL] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr_array[i], data, default_value_array[i]);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEFCHK][PASS] idx=%u addr=0x%08lX val=0x%08X\n", i, addr_array[i], data);
#endif
        }
    }

    /* Phase 2: Write/read check for six patterns */
    for (j = 0U; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
        /* Write phase */
        for (i = 0U; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* Skipped */
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue; /* Not writable */
            }
            unsigned int wr_val = (data_wr & write_mask_array[i]);
            write_reg(addr_array[i], wr_val); /* Masked write */
#ifdef DEBUG_DISPLAY
            printf("[WRITE] idx=%u addr=0x%08lX pat=0x%08X wr=0x%08X\n", i, addr_array[i], data_wr, wr_val);
#endif
        }
        /* Read/verify phase */
        for (i = 0U; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* Skipped */
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue; /* Not writable */
            }
            if (read_mask_array[i] == 0x00000000U) {
                continue; /* Not readable */
            }
            unsigned int data_rd = (read_reg(addr_array[i]) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[VERIFY][FAIL] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X maskR=0x%08X maskW=0x%08X\n",
                       i, addr_array[i], data_rd, exp_val, read_mask_array[i], write_mask_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[VERIFY][PASS] idx=%u addr=0x%08lX val=0x%08X\n", i, addr_array[i], data_rd);
#endif
            }
        }
    }

    /* Acceptance criteria: pass only if both counters are zero */
    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT][FAIL] def=%u wr=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT][PASS] def=%u wr=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(0);
    }

    return 0;
}
