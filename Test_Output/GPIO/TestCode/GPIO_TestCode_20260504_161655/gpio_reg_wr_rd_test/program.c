// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Test: gpio_reg_wr_rd_test
 * Description (verbatim from metadata):
 * Performs two phases: (1) Default value check for each address in addr_array using read_mask_array and skip_rst_array; read data is masked with 0xFFFFFFFE then compared to default_value_array. (2) Read/write verification using six patterns {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each address not in skip_array and with nonzero write mask, writes (data_wr & write_mask_array[i]); then reads back data_rd=(read_reg(addr) & read_mask_array[i]) and computes expected value exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Compares data_rd to exp_val. Tracks def_fail_cnt and wr_fail_cnt. Finishes with finish(0) if both counts are zero, else finish(1).
 */

/* Helper API prototypes expected from platform/harness */
extern unsigned int read_reg(unsigned long addr);
extern void write_reg(unsigned long addr, unsigned int val);
extern void wait_on(unsigned int cycles);
extern void finish(int status);
extern int printf(const char *fmt, ...);

/* Bannered helper functions implementing the steps */

/*
 * Purpose: Check default/reset values across impacted registers defined in addr_array[]
 */
static unsigned int def_fail_cnt = 0;
static void chk_rst_val(void)
{
    for (unsigned int idx = 0; idx < CNT; ++idx) {
        unsigned long addr = addr_array[idx];
        if (skip_rst_array[idx] == 1u) continue;               /* skip reset check if requested */
        if (read_mask_array[idx] == 0u) continue;              /* nothing readable */

        unsigned int rd = read_reg(addr);
        unsigned int data = (rd & 0xFFFFFFFEu);                 /* mask bit0 per spec */
        if (data != default_value_array[idx]) {
            ++def_fail_cnt;
#ifdef DEBUG_DISPLAY
            printf("[gpio_reg_wr_rd_test][RST_CHK] idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x\n",
                   idx, addr, rd, default_value_array[idx]);
#endif
        }
    }
}

/*
 * Purpose: Perform masked write then verify masked read against expected composition
 */
static unsigned int wr_fail_cnt = 0;
static void chk_rd_wr(void)
{
    for (unsigned int p = 0; p < 6u; ++p) {
        unsigned int data_wr = chk_val[p];

        /* Write phase over all entries */
        for (unsigned int idx = 0; idx < CNT; ++idx) {
            if (skip_array[idx] == 1u) continue;               /* skip if requested */
            if (write_mask_array[idx] == 0u) continue;         /* not writable */
            unsigned long addr = addr_array[idx];
            write_reg(addr, (data_wr & write_mask_array[idx]));
        }

        /* Read and compare phase */
        for (unsigned int idx = 0; idx < CNT; ++idx) {
            if (skip_array[idx] == 1u) continue;
            if (write_mask_array[idx] == 0u) continue;
            if (read_mask_array[idx] == 0u) continue;
            unsigned long addr = addr_array[idx];

            unsigned int rd = (read_reg(addr) & read_mask_array[idx]);
            unsigned int wr_n = (write_mask_array[idx] ^ 0xFFFFFFFFu);
            unsigned int exp = ((data_wr & read_mask_array[idx] & write_mask_array[idx]) |
                                (wr_n & read_mask_array[idx] & default_value_array[idx]));
            if (rd != exp) {
                ++wr_fail_cnt;
#ifdef DEBUG_DISPLAY
                printf("[gpio_reg_wr_rd_test][WR_RD] idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x pat=0x%08x\n",
                       idx, addr, rd, exp, data_wr);
#endif
            }
        }
    }
}

/*
 * Purpose: Entry point to execute reset-value and read/write verification, then conclude per acceptance criteria.
 */
void gpio_reg_wr_rd_test(void)
{
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt == 0u && wr_fail_cnt == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[gpio_reg_wr_rd_test] PASS\n");
#endif
        finish(0); /* PASS */
    } else {
#ifdef DEBUG_DISPLAY
        printf("[gpio_reg_wr_rd_test] FAIL def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1); /* FAIL */
    }
}
