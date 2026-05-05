// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/*
  Test: gpio_reg_wr_rd_test
  High-level description (from Hidden_Test_Description):
    Directed test that validates GPIO register default values and read/write behavior using arrays:
    addr_array, default_value_array, read_mask_array, write_mask_array, skip_array, skip_rst_array.
    It masks off LSB (0xFFFFFFFE) during default comparisons and computes expected values for write/read
    based on masks and defaults across multiple patterns.
*/

#include "test_define.c"

/* Local globals for counters and scratch registers */
static volatile unsigned int def_fail_cnt = 0;
static volatile unsigned int wr_fail_cnt  = 0;

/* Framework functions expected via test_common.h */
extern unsigned int read_reg(unsigned int addr);
extern void         write_reg(unsigned int addr, unsigned int val);
extern void         wait_on(unsigned int cycles);
extern void         finish(int status);

/* Bit mask used to clear LSB during default comparisons */
#define DEFAULT_MASK_LSB_CLEAR 0xFFFFFFFEu

/*
  Function: chk_rst_val
  Purpose: Implements Hidden_Test_Steps_Procedure step 2 (reset/default validation):
    - Iterate across all entries, applying read mask non-zero and reset-skip conditions.
    - Read register, clear LSB, and compare with default_value_array; on mismatch, increment def_fail_cnt.
*/
static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; ++i) {
        unsigned int addr = addr_array[i];

        /* Skip default check if requested or if read mask is zero */
        if (skip_rst_array[i] == 1u) {
            continue;
        }
        if (read_mask_array[i] == 0u) {
            continue;
        }

        /* Read and mask per requirement */
        unsigned int rd = read_reg(addr);
        unsigned int masked = (rd & DEFAULT_MASK_LSB_CLEAR);

        if (masked != default_value_array[i]) {
            ++def_fail_cnt;
            #ifdef DEBUG_DISPLAY
            printf("DEF_MISMATCH: addr=0x%08X exp=0x%08X got_masked=0x%08X full=0x%08X\n",
                   addr, default_value_array[i], masked, rd);
            #endif
        }
    }
}

/*
  Function: chk_rd_wr
  Purpose: Implements Hidden_Test_Steps_Procedure step 3 (write/readback validation):
    - For each of six patterns, write (pattern & write_mask) to writable registers not skipped.
    - Read back with read_mask and compute expected = (wr & rd_mask & wr_mask) | (~wr_mask & rd_mask & default).
    - On any mismatch, increment wr_fail_cnt.
*/
static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6u; ++j) {
        unsigned int data_wr = chk_val[j];

        /* Write phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u) {
                continue;
            }
            if (write_mask_array[i] == 0u) {
                continue;
            }
            unsigned int addr = addr_array[i];
            write_reg(addr, (data_wr & write_mask_array[i]));
        }

        /* Read/verify phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u) {
                continue;
            }
            if (write_mask_array[i] == 0u) {
                continue;
            }
            if (read_mask_array[i] == 0u) {
                continue;
            }

            unsigned int addr = addr_array[i];
            unsigned int rd_full = read_reg(addr);
            unsigned int rd_masked = (rd_full & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (rd_masked != exp) {
                ++wr_fail_cnt;
                #ifdef DEBUG_DISPLAY
                printf("WR_MISMATCH: pat=%u addr=0x%08X exp=0x%08X got=0x%08X rmask=0x%08X wmask=0x%08X def=0x%08X wr=0x%08X\n",
                       j, addr, exp, rd_masked, read_mask_array[i], write_mask_array[i],
                       default_value_array[i], data_wr);
                #endif
            }
        }
    }
}

/*
  Function: test_case
  Purpose: Entry point that executes the reset and write/readback checks, then evaluates pass/fail
           strictly per Hidden_Validation_Acceptance_Criteria.
*/
void test_case(void)
{
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt == 0u && wr_fail_cnt == 0u) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}
