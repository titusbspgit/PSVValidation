// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
Hidden_Test_Description:
Performs two phases: (1) Default value check for each address in addr_array using read_mask_array and skip_rst_array; read data is masked with 0xFFFFFFFE then compared to default_value_array. (2) Read/write verification using six patterns {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each address not in skip_array and with nonzero write mask, writes (data_wr & write_mask_array[i]); then reads back data_rd=(read_reg(addr) & read_mask_array[i]) and computes expected value exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Compares data_rd to exp_val. Tracks def_fail_cnt and wr_fail_cnt. Finishes with finish(0) if both counts are zero, else finish(1).
*/

static uint32_t def_fail_cnt = 0;
static uint32_t wr_fail_cnt = 0;

/*
Function: chk_rst_val
Purpose: Perform default value check across all addresses, honoring read masks and reset-skip flags.
*/
static void chk_rst_val(void)
{
    for (uint32_t i = 0; i < CNT; i++) {
        uint32_t addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
            continue; /* Skip reset validation for this address */
        }
        if (read_mask_array[i] == 0U) {
            continue; /* Not readable */
        }
        uint32_t data_rd = read_reg(addr);
        uint32_t data = (data_rd & MASK_RESET_CLR);
        uint32_t exp = default_value_array[i];
        if (data != exp) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEFCHK][%lu] addr=0x%08lx rd=0x%08lx masked=0x%08lx exp=0x%08lx\n",
                   (unsigned long)i, (unsigned long)addr, (unsigned long)data_rd,
                   (unsigned long)data, (unsigned long)exp);
#endif
        }
    }
}

/*
Function: chk_rd_wr
Purpose: Verify read/write behavior using provided patterns and masks.
*/
static void chk_rd_wr(void)
{
    for (uint32_t j = 0; j < 6U; j++) {
        uint32_t data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[WRCHK] pattern[%lu]=0x%08lx\n", (unsigned long)j, (unsigned long)data_wr);
#endif
        /* Write phase */
        for (uint32_t i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* Skip write on this address */
            }
            if (write_mask_array[i] == 0U) {
                continue; /* Not writable */
            }
            uint32_t addr = addr_array[i];
            uint32_t wdata = (data_wr & write_mask_array[i]);
            write_reg(addr, wdata);
        }
        /* Read/compare phase */
        for (uint32_t i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* Skip read/compare on this address */
            }
            if ((write_mask_array[i] == 0U) || (read_mask_array[i] == 0U)) {
                continue; /* Requires both readable and writable */
            }
            uint32_t addr = addr_array[i];
            uint32_t data_rd = (read_reg(addr) & read_mask_array[i]);
            uint32_t wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            uint32_t exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                 (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[MISMATCH][%lu] addr=0x%08lx rd=0x%08lx exp=0x%08lx rmask=0x%08lx wmask=0x%08lx def=0x%08lx\n",
                       (unsigned long)i, (unsigned long)addr, (unsigned long)data_rd,
                       (unsigned long)exp_val, (unsigned long)read_mask_array[i],
                       (unsigned long)write_mask_array[i], (unsigned long)default_value_array[i]);
#endif
            }
        }
    }
}

int main(void)
{
#ifdef DEBUG_DISPLAY
    printf("[START] gpio_reg_wr_rd_test\n");
#endif
    def_fail_cnt = 0U;
    wr_fail_cnt = 0U;

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] FAIL def_fail_cnt=%lu wr_fail_cnt=%lu\n", (unsigned long)def_fail_cnt, (unsigned long)wr_fail_cnt);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] PASS\n");
#endif
        finish(0);
    }
    return 0; /* Unreached if finish() terminates */
}
