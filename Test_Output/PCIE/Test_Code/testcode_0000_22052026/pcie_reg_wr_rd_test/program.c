// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose : Perform default/reset value verification for readable registers.
 * Input   : def_fail_cnt - pointer to failure counter to increment on mismatches.
 */
static void chk_rst_val(unsigned int *def_fail_cnt)
{
    unsigned int i;
    const unsigned int n = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));

#ifdef DEBUG_DISPLAY
    printf("[DBG] chk_rst_val: total entries = %u\n", n);
#endif

    for (i = 0; i < n; i++) {
        unsigned long int addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];

        /* Skip if no readable bits */
        if (rmask == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%u skipped (read_mask=0)\n", i);
#endif
            continue;
        }

        /* Read and compare with default value */
        unsigned int data_rd = read_reg(addr);
        unsigned int def_val = default_value_array[i];
        if (data_rd != def_val) {
            (*def_fail_cnt)++;
#ifdef DEBUG_DISPLAY
            printf("[DBG][DEF_MISMATCH] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr, data_rd, def_val);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%u addr=0x%08lX rd=0x%08X (OK)\n", i, addr, data_rd);
#endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform masked write and read-back verification for writable and readable registers.
 * Input   : wr_fail_cnt - pointer to failure counter to increment on mismatches.
 */
static void chk_rd_wr(unsigned int *wr_fail_cnt)
{
    unsigned int i, j;
    const unsigned int n = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));
    const unsigned int patterns[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
    };

#ifdef DEBUG_DISPLAY
    printf("[DBG] chk_rd_wr: total entries = %u\n", n);
#endif

    for (j = 0; j < 6U; j++) {
        unsigned int data_wr = patterns[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] Pattern %u: 0x%08X\n", j, data_wr);
#endif
        /* Write phase */
        for (i = 0; i < n; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write: i=%u skipped (skip_array=1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write: i=%u skipped (write_mask=0)\n", i);
#endif
                continue;
            }
            write_reg(addr_array[i], data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DBG] write: i=%u addr=0x%08lX wr=0x%08X\n", i, addr_array[i], data_wr);
#endif
        }

        /* Read/verify phase */
        for (i = 0; i < n; i++) {
            unsigned int wmask = write_mask_array[i];
            unsigned int rmask = read_mask_array[i];
            if (skip_array[i] == 1U || wmask == 0x00000000U || rmask == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read: i=%u skipped (skip=%u wmask=0x%08X rmask=0x%08X)\n", i, skip_array[i], wmask, rmask);
#endif
                continue;
            }

            unsigned long int addr = addr_array[i];
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (wmask ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));

            if (data_rd != exp_val) {
                (*wr_fail_cnt)++;
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR_MISMATCH] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X wmask=0x%08X rmask=0x%08X\n",
                       i, addr, data_rd, exp_val, wmask, rmask);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read: i=%u addr=0x%08lX rd=0x%08X (OK)\n", i, addr, data_rd);
#endif
            }
        }
    }
}

/*
 * Function: test_case
 * Purpose : Entry point for testcase execution. Tracks failures and reports PASS/FAIL.
 * Return  : 0 (not used); finish(0) or finish(1) will terminate.
 */
int test_case(void)
{
    unsigned int def_fail_cnt = 0U;
    unsigned int wr_fail_cnt = 0U;

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_case: START\n");
#endif

    /* Phase 1: Default/reset value check */
    chk_rst_val(&def_fail_cnt);

    /* Phase 2: Masked write/read-back verification */
    chk_rd_wr(&wr_fail_cnt);

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_case: def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[DBG] test_case: FAIL\n");
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DBG] test_case: PASS\n");
#endif
        finish(0);
    }

    return 0; /* Unreachable, finish() terminates */
}
