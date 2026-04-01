#include "test_define.c"

/*
 * gpio_reg_wr_rd_test
 *
 * High-level intent (from Test Description):
 * - Verify GPIO register default reset values and masked write/read behavior
 *   across the defined register set using per-register read/write masks and
 *   skip lists. Writes six data patterns, reads back with masks, and compares
 *   against expected values computed from masks and defaults.
 *
 * Implementation notes:
 * - Uses only external arrays (addr_array, default_value_array, read_mask_array,
 *   write_mask_array, skip_array, skip_rst_array) that must be defined by the
 *   integration.
 * - Default-value check applies an extra mask (0xFFFFFFFE) to the read value
 *   before comparison as per plan.
 * - Any mismatch increments failure counters and is logged with DEBUG_DISPLAY.
 * - Overall pass/fail decided per acceptance criteria and reported via finish().
 */

static void chk_rst_val(int *p_def_fail_cnt)
{
    int i;
    unsigned int addr, data_rd, data_cmp;

    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Starting reset/default-value checks\n");

    for (i = 0; i < (int)CNT; ++i) {
        /* Skip registers per reset-skip list */
        if (skip_rst_array[i] == 1u)
            continue;

        /* If register is not readable (read mask 0), skip */
        if (read_mask_array[i] == 0x00000000u)
            continue;

        addr = addr_array[i];
        data_rd = read_reg(addr);

        /* Apply default read mask behavior: clear bit0 in reset-value check */
        data_cmp = (data_rd & 0xFFFFFFFEu);

        if (data_cmp != default_value_array[i]) {
            (*p_def_fail_cnt)++;
            DEBUG_DISPLAY("[DEF-MISMATCH] idx=%d addr=0x%08X exp=0x%08X rd=0x%08X (masked=0x%08X)\n",
                          i, addr, default_value_array[i], data_rd, data_cmp);
        }
#ifdef DEBUG_RW_MSG
        else {
            DEBUG_DISPLAY("[DEF-OK] idx=%d addr=0x%08X exp=0x%08X rd(masked)=0x%08X\n",
                          i, addr, default_value_array[i], data_cmp);
        }
#endif
    }
}

static void chk_rd_wr(int *p_wr_fail_cnt)
{
    int i, j;
    unsigned int addr;
    const unsigned int patterns[6] = {
        0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Starting masked write/read verification\n");

    for (j = 0; j < 6; ++j) {
        unsigned int data_wr = patterns[j];

        DEBUG_DISPLAY("  Pattern %d: 0x%08X (write phase)\n", j, data_wr);

        /* Write phase */
        for (i = 0; i < (int)CNT; ++i) {
            unsigned int wmask;

            if (skip_array[i] == 1u)
                continue;

            wmask = write_mask_array[i];
            if (wmask == 0x00000000u)
                continue; /* Not writable */

            addr = addr_array[i];
            write_reg(addr, (data_wr & wmask));
#ifdef DEBUG_RW_MSG
            DEBUG_DISPLAY("    [WR] idx=%d addr=0x%08X data=0x%08X (wmask=0x%08X)\n", i, addr, (data_wr & wmask), wmask);
#endif
        }

        wait_on(10); /* small wait between write and read-back phases */

        DEBUG_DISPLAY("  Pattern %d: 0x%08X (read/verify phase)\n", j, data_wr);

        /* Read/verify phase */
        for (i = 0; i < (int)CNT; ++i) {
            unsigned int rmask, wmask, rd, wr_n, exp_val;

            if (skip_array[i] == 1u)
                continue;

            wmask = write_mask_array[i];
            if (wmask == 0x00000000u)
                continue; /* Not writable */

            rmask = read_mask_array[i];
            if (rmask == 0x00000000u)
                continue; /* Not readable */

            addr = addr_array[i];
            rd = (read_reg(addr) & rmask);

            /* Expected = (data_wr & rmask & wmask) | (~wmask & rmask & default) */
            wr_n = (~wmask);
            exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));

            if (rd != exp_val) {
                (*p_wr_fail_cnt)++;
                DEBUG_DISPLAY("    [RD-MISMATCH] idx=%d addr=0x%08X exp=0x%08X rd=0x%08X rmask=0x%08X wmask=0x%08X\n",
                              i, addr, exp_val, rd, rmask, wmask);
            }
#ifdef DEBUG_RW_MSG
            else {
                DEBUG_DISPLAY("    [RD-OK] idx=%d addr=0x%08X rd=0x%08X exp=0x%08X\n",
                              i, addr, rd, exp_val);
            }
#endif
        }

        wait_on(10);
    }
}

void test_case(void)
{
    int def_fail_cnt = 0;
    int wr_fail_cnt  = 0;

    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Enter test_case()\n");

#if 0
    /* Optional soft reset check (disabled as per plan) */
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(10);
#endif

    chk_rst_val(&def_fail_cnt);
    chk_rd_wr(&wr_fail_cnt);

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        DEBUG_DISPLAY("[gpio_reg_wr_rd_test][RESULT] FAIL: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
        return;
    }

    DEBUG_DISPLAY("[gpio_reg_wr_rd_test][RESULT] PASS\n");
    finish(0);
}
