#include "test_define.c"

/*
 * Test flow derived from CSV:
 * 1) chk_rst_val: verify reset/default values for all readable, non-skipped registers.
 *    - apply 0xFFFFFFFE mask to read value before compare (clear bit0 during reset check).
 * 2) chk_rd_wr: write six patterns with per-register write masks, then read back using
 *    read masks and compute expected values using defaults and masks.
 * 3) Final result: finish(0) on success, finish(1) on any mismatch.
 */

static void chk_rst_val(unsigned int *def_fail_cnt)
{
    for (unsigned int i = 0; i < CNT; ++i) {
        if (skip_rst_array[i] == 1u)
            continue;
        unsigned int rmask = read_mask_array[i];
        if (rmask == 0u)
            continue; /* unreadable register */

        unsigned long addr = addr_array[i];
        unsigned int rd = read_reg(addr);
        unsigned int data = (rd & 0xFFFFFFFEu); /* clear bit0 per CSV note */
        unsigned int exp = default_value_array[i];
        if (data != exp) {
            (*def_fail_cnt)++;
            DEBUG_DISPLAY("[DEFCHK][idx=%u][0x%08lX] exp=0x%08X rd=0x%08X\n", i, addr, exp, data);
        }
    }
}

static void chk_rd_wr(unsigned int *wr_fail_cnt)
{
    for (unsigned int pat = 0; pat < (sizeof(chk_val)/sizeof(chk_val[0])); ++pat) {
        unsigned int wdata = chk_val[pat];
        DEBUG_DISPLAY("[WRPHASE] pattern[%u]=0x%08X\n", pat, wdata);
        /* Write phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u)
                continue;
            unsigned int wmask = write_mask_array[i];
            if (wmask == 0u)
                continue; /* write-protected or no implemented bits */
            unsigned long addr = addr_array[i];
            unsigned int wr = (wdata & wmask);
            write_reg(addr, wr);
        }
        /* Read/verify phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u)
                continue;
            unsigned int wmask = write_mask_array[i];
            if (wmask == 0u)
                continue;
            unsigned int rmask = read_mask_array[i];
            if (rmask == 0u)
                continue; /* unreadable */

            unsigned long addr = addr_array[i];
            unsigned int rd = (read_reg(addr) & rmask);
            unsigned int wr_n = (wmask ^ 0xFFFFFFFFu);
            unsigned int exp = ((wdata & rmask & wmask) | (wr_n & rmask & default_value_array[i]));
            if (rd != exp) {
                (*wr_fail_cnt)++;
                DEBUG_DISPLAY("[RDMIS][idx=%u][0x%08lX] exp=0x%08X rd=0x%08X (wmask=0x%08X rmask=0x%08X)\n",
                               i, addr, exp, rd, wmask, rmask);
            }
        }
    }
}

void test_case(void)
{
    unsigned int def_fail_cnt = 0;
    unsigned int wr_fail_cnt = 0;

    DEBUG_DISPLAY("==== gpio_reg_wr_rd_test: START ====%n");

#if 0
    /* Optional: soft reset sequence (disabled as per CSV remark) */
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(10);
#endif

    /* Step 1: reset/default value check */
    chk_rst_val(&def_fail_cnt);

    /* Step 2: masked write/read verification with patterns */
    chk_rd_wr(&wr_fail_cnt);

    if ((def_fail_cnt > 0u) || (wr_fail_cnt > 0u)) {
        DEBUG_DISPLAY("==== gpio_reg_wr_rd_test: FAIL def=%u wr=%u ====%n", def_fail_cnt, wr_fail_cnt);
        finish(1);
        return;
    }

    DEBUG_DISPLAY("==== gpio_reg_wr_rd_test: PASS ====%n");
    finish(0);
}
