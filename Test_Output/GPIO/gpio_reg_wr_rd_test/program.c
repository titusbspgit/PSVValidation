// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
/*
 * Program for gpio_reg_wr_rd_test
 * Description (verbatim):
 * Performs two phases: (1) Default value check for each address in addr_array using read_mask_array and skip_rst_array; read data is masked with 0xFFFFFFFE then compared to default_value_array. (2) Read/write verification using six patterns {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each address not in skip_array and with nonzero write mask, writes (data_wr & write_mask_array[i]); then reads back data_rd=(read_reg(addr) & read_mask_array[i]) and computes expected value exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Compares data_rd to exp_val. Tracks def_fail_cnt and wr_fail_cnt. Finishes with finish(0) if both counts are zero, else finish(1).
 */

#include "test_define.c"

/* External harness APIs are expected from included headers */

/*
 * chk_rst_val - Verify default values for readable and not-skip-reset registers
 */
static unsigned int def_fail_cnt = 0;
static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; ++i) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1u) continue;                 /* per metadata */
        if (read_mask_array[i] == 0u) continue;                /* unreadable */

        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu);           /* apply mask 0xFFFFFFFE */
        if (data != default_value_array[i]) {
            ++def_fail_cnt;
#ifdef DEBUG_DISPLAY
            printf("[gpio_reg_wr_rd_test][RST_CHK] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr, data, default_value_array[i]);
#endif
        }
    }
}

/*
 * chk_rd_wr - Pattern-based write/read verification across registers
 */
static unsigned int wr_fail_cnt = 0;
static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6u; ++j) {
        unsigned int data_wr = chk_val[j];

        /* Write phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u) continue;                 /* skip per context */
            if (write_mask_array[i] == 0u) continue;           /* not writable */
            write_reg(addr, (data_wr & write_mask_array[i]));  /* masked write */
        }

        /* Read/compare phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u) continue;
            if (write_mask_array[i] == 0u) continue;
            if (read_mask_array[i] == 0u) continue;

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                ++wr_fail_cnt;
#ifdef DEBUG_DISPLAY
                printf("[gpio_reg_wr_rd_test][WR_RD] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X pat=0x%08X\n", i, addr, data_rd, exp_val, data_wr);
#endif
            }
        }
    }
}

/*
 * main test entry - executes both phases and reports per acceptance criteria
 */
void program_main(void)
{
    chk_rst_val();
    chk_rd_wr();

    /* Acceptance Criteria: pass only if both counts are zero */
    if ((def_fail_cnt == 0u) && (wr_fail_cnt == 0u)) {
#ifdef DEBUG_DISPLAY
        printf("[gpio_reg_wr_rd_test] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[gpio_reg_wr_rd_test] FAIL def=%u wr=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
    }
}
