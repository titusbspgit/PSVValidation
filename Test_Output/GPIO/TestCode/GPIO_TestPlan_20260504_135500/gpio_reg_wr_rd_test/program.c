// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/*
Hidden_Test_Description (AS-IS):
Test checks GPIO register default values and masked write/read functionality. It iterates over a fixed address list, conditionally skips non-readable/non-writable entries, writes test patterns with masks, and verifies read-back values against expected results computed from write masks, read masks, and default values.
*/

#include "test_define.c"

/*
Purpose: Read current value and compare against documented default using mask and additional 0xFFFFFFFE per acceptance criteria.
*/
static int chk_rst_val(void)
{
    int def_fail_cnt = 0;
    for (unsigned int i = 0; i < CNT; i++) {
        if (skip_rst_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[RSTCHK] Skipping index %u (addr=0x%08lx) for default check\n", i, addr_array[i]);
#endif
            continue;
        }
        unsigned int rdv = read_reg(addr_array[i]);
        unsigned int exp = (default_value_array[i] & read_mask_array[i]) & 0xFFFFFFFEu; /* As per criteria */
        unsigned int got = (rdv & read_mask_array[i]) & 0xFFFFFFFEu;
        if (got != exp) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[RSTCHK][FAIL] idx=%u addr=0x%08lx exp=0x%08x got=0x%08x rm=0x%08x\n", i, addr_array[i], exp, got, read_mask_array[i]);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[RSTCHK][PASS] idx=%u addr=0x%08lx val=0x%08x\n", i, addr_array[i], got);
#endif
        }
    }
    return def_fail_cnt;
}

/*
Purpose: Perform masked write-readback validation using provided read/write masks and defaults.
*/
static int chk_wr_rd(void)
{
    int wr_fail_cnt = 0;
    const unsigned int patterns[2] = {0xAAAAAAAAu, 0x55555555u};

    for (unsigned int i = 0; i < CNT; i++) {
        if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[WRRD] Skipping index %u (addr=0x%08lx) for write/read check\n", i, addr_array[i]);
#endif
            continue;
        }
        unsigned int wm = write_mask_array[i];
        unsigned int rm = read_mask_array[i];
        unsigned int defv = default_value_array[i];

        for (unsigned int p = 0; p < 2; p++) {
            unsigned int to_write = (defv & ~wm) | (patterns[p] & wm);
            write_reg(addr_array[i], to_write);
            unsigned int got = read_reg(addr_array[i]) & rm;
            unsigned int exp = to_write & rm;
            if (got != exp) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[WRRD][FAIL] idx=%u patt=%u addr=0x%08lx exp=0x%08x got=0x%08x rm=0x%08x wm=0x%08x\n",
                       i, p, addr_array[i], exp, got, rm, wm);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[WRRD][PASS] idx=%u patt=%u addr=0x%08lx val=0x%08x\n", i, p, addr_array[i], got);
#endif
            }
        }
    }
    return wr_fail_cnt;
}

/*
Entry point mapping Hidden_Test_Steps_Procedure (AS-IS: Entry point: test_case())
Implements: default check followed by masked write-read validation.
Acceptance: PASS if both counters are zero; else FAIL.
*/
int test_case(void)
{
    int def_fail_cnt = chk_rst_val();
    int wr_fail_cnt  = chk_wr_rd();

#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt == 0 && wr_fail_cnt == 0) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
