/*
  Test: gpio_reg_wr_rd_test
  Meta Hidden Description:
    Directed test that validates GPIO register default values and read/write behavior using arrays:
    addr_array, default_value_array, read_mask_array, write_mask_array, skip_array, skip_rst_array.
    It masks off LSB (0xFFFFFFFE) during default comparisons and computes expected values for write/read
    based on masks and defaults across multiple patterns.

  Acceptance Criteria:
    - Default value phase: For each i, after masking LSB, data == default_value_array[i]; else def_fail_cnt++ and log.
    - Write/read phase: For each j pattern and i register, after read/mask, data_rd == exp_val computed as
      ((data_wr & read_mask & write_mask) | (~write_mask & read_mask & default)); else wr_fail_cnt++ and log.
    - Completion: finish(0) if def_fail_cnt==0 and wr_fail_cnt==0; else finish(1).

  Notes:
    - Per Meta_data_sheet: Include headers "test_common.h" and "test_define.c".
    - Arrays and constants (addr_array, default_value_array, read_mask_array, write_mask_array,
      skip_array, skip_rst_array, CNT) are expected to be provided by included sources.
*/

#include <stdint.h>
#include <stdio.h>
#include "test_common.h"
#include "test_define.c"  /* Included as specified in Meta_data_sheet */

/* Globals as specified */
volatile uint32_t data_rd = 0;
volatile uint32_t data_wr = 0;
volatile uint32_t data    = 0;
volatile uint32_t def_fail_cnt = 0;
volatile uint32_t wr_fail_cnt  = 0;

/* Externals expected from test_define.c (types chosen conservatively) */
extern const uint32_t addr_array[];
extern const uint32_t default_value_array[];
extern const uint32_t read_mask_array[];
extern const uint32_t write_mask_array[];
extern const uint8_t  skip_array[];
extern const uint8_t  skip_rst_array[];
extern const uint32_t CNT;

/* Prototypes expected to be provided by test framework */
extern uint32_t read_reg(uint32_t addr);
extern void     write_reg(uint32_t addr, uint32_t val);
extern void     finish(int status);

/* Mask used during default comparisons per metadata */
#define DEFAULT_MASK_LSB_CLEAR 0xFFFFFFFEu

static void chk_rst_val(void)
{
    /* 2. Call chk_rst_val() */
    for (uint32_t i = 0; i < CNT; ++i) {
        uint32_t addr = addr_array[i];

        /* 2.2 If skip_rst_array[i] == 1, continue */
        if (skip_rst_array[i] == 1u) {
            continue;
        }

        /* 2.3 If read_mask_array[i] == 0x00000000, continue */
        if (read_mask_array[i] == 0x00000000u) {
            continue;
        }

        /* 2.4 data_rd = read_reg(addr); data = (data_rd & 0xFFFFFFFE) */
        data_rd = read_reg(addr);
        data    = (data_rd & DEFAULT_MASK_LSB_CLEAR);

        /* 2.5 Compare against default_value_array[i] */
        if (data != default_value_array[i]) {
            ++def_fail_cnt;
            printf("DEF_MISMATCH: addr=0x%08X expected=0x%08X masked_read=0x%08X full_read=0x%08X\n",
                   addr, default_value_array[i], data, data_rd);
        }
    }
}

static void chk_rd_wr(void)
{
    /* 3.1 Define chk_val[6] */
    static const uint32_t chk_val[6] = {
        0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    for (uint32_t j = 0; j < 6u; ++j) {
        data_wr = chk_val[j];

        /* 3.3 Write phase */
        for (uint32_t i = 0; i < CNT; ++i) {
            uint32_t addr = addr_array[i];

            if (skip_array[i] == 1u) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000u) {
                continue;
            }

            /* write_reg(addr, (data_wr & write_mask_array[i])) */
            write_reg(addr, (data_wr & write_mask_array[i]));
        }

        /* 3.4 Read/verify phase */
        for (uint32_t i = 0; i < CNT; ++i) {
            uint32_t addr = addr_array[i];

            if (skip_array[i] == 1u) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000u) {
                continue;
            }
            if (read_mask_array[i] == 0x00000000u) {
                continue;
            }

            /* - data_rd = (read_reg(addr) & read_mask_array[i]) */
            uint32_t rd = read_reg(addr);
            data_rd = (rd & read_mask_array[i]);

            /* - wr_n = (write_mask_array[i] ^ 0xFFFFFFFF) */
            uint32_t wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);

            /* - exp_val = ((data_wr & read_mask & write_mask) | (wr_n & read_mask & default)) */
            uint32_t exp_val =
                ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                 (wr_n    & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                ++wr_fail_cnt;
                printf("WR_MISMATCH: j=%u addr=0x%08X expected=0x%08X read=0x%08X raw_read=0x%08X rmask=0x%08X wmask=0x%08X def=0x%08X wr=0x%08X\n",
                       j, addr, exp_val, data_rd, rd, read_mask_array[i], write_mask_array[i],
                       default_value_array[i], data_wr);
            }
        }
    }
}

void test_case(void)
{
    /* Execute phases exactly as specified */
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0u || wr_fail_cnt > 0u) {
        finish(1);
    } else {
        finish(0);
    }
}

/* Optional main for standalone builds (guarded) */
#ifdef STANDALONE_MAIN
int main(void) { test_case(); return 0; }
#endif
