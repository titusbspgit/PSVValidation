// Author - AI Force 1.3.2. Date 20-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase: gpio_reg_wr_rd_test
 *
 * Implements the Meta Test Steps / Procedure exactly as provided:
 * - chk_rst_val(): Validate default values for readable registers not skipped
 *   (applies mask 0xFFFFFFFE to read data before compare).
 * - chk_rd_wr(): For each of 6 patterns, write to writable registers then
 *   validate readback with expected value formula using read/write masks.
 * - test_case(): Run both checks and terminate with finish(0) on success or
 *   finish(1) on failure.
 */

static unsigned int def_fail_cnt = 0U;
static unsigned int wr_fail_cnt  = 0U;

/* -------------------------------------------------------------------------
 * Function: chk_rst_val
 * Purpose : Check reset/default values per Meta logic
 * ------------------------------------------------------------------------- */
static void chk_rst_val(void)
{
    for (unsigned int i = 0U; i < (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0])); i++) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] RST-SKIP: idx=%u\n", i);
#endif
            continue; /* skip reset check for this index */
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] RST-MASK0: idx=%u (no readable bits)\n", i);
#endif
            continue; /* nothing readable to validate */
        }

        uint32_t addr    = addr_array[i];
        uint32_t data_rd = read_reg(addr);            /* register read */
        uint32_t data    = (data_rd & 0xFFFFFFFEU);   /* mask off bit0 */
        uint32_t exp_def = default_value_array[i];

#ifdef DEBUG_DISPLAY
        printf("[DBG] RST-CHK: idx=%u addr=0x%08X rd=0x%08X data=0x%08X exp=0x%08X\n",
               i, addr, data_rd, data, exp_def);
#endif

        if (data != exp_def) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] RST-MISMATCH: idx=%u addr=0x%08X got=0x%08X exp=0x%08X\n",
                   i, addr, data, exp_def);
#endif
        }
    }
}

/* -------------------------------------------------------------------------
 * Function: chk_rd_wr
 * Purpose : Perform read/write validation with specified patterns
 * ------------------------------------------------------------------------- */
static void chk_rd_wr(void)
{
    const uint32_t chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U,
        0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int p = 0U; p < 6U; p++) {
        uint32_t data_wr = chk_val[p];

#ifdef DEBUG_DISPLAY
        printf("[DBG] PATTERN: p=%u val=0x%08X\n", p, data_wr);
#endif

        /* First: perform writes to writable registers (not skipped) */
        for (unsigned int i = 0U; i < (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0])); i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] WR-SKIP: idx=%u\n", i);
#endif
                continue; /* skip write */
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] WR-MASK0: idx=%u\n", i);
#endif
                continue; /* not writable */
            }

            uint32_t addr   = addr_array[i];
            uint32_t wr_val = (data_wr & write_mask_array[i]);
#ifdef DEBUG_DISPLAY
            printf("[DBG] WR: idx=%u addr=0x%08X data=0x%08X mask=0x%08X wr=0x%08X\n",
                   i, addr, data_wr, write_mask_array[i], wr_val);
#endif
            write_reg(addr, wr_val);
        }

        /* Second: read back and compare expected values */
        for (unsigned int i = 0U; i < (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0])); i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-SKIP: idx=%u\n", i);
#endif
                continue; /* skip read/compare */
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-WMASK0: idx=%u\n", i);
#endif
                continue; /* not writable: no expectation */
            }
            if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] RD-RMASK0: idx=%u\n", i);
#endif
                continue; /* nothing readable to compare */
            }

            uint32_t addr    = addr_array[i];
            uint32_t data_rd = (read_reg(addr) & read_mask_array[i]);
            uint32_t wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            uint32_t exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[DBG] RD: idx=%u addr=0x%08X rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X def=0x%08X\n",
                   i, addr, data_rd, exp_val, read_mask_array[i], write_mask_array[i], default_value_array[i]);
#endif
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR] RD-MISMATCH: idx=%u addr=0x%08X got=0x%08X exp=0x%08X\n",
                       i, addr, data_rd, exp_val);
#endif
            }
        }
    }
}

/* -------------------------------------------------------------------------
 * Function: test_case
 * Purpose : Execute the test sequence and terminate via finish(status)
 * ------------------------------------------------------------------------- */
void test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] TEST START: gpio_reg_wr_rd_test\n");
#endif

    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[DBG] RESULT: def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
}

/*
 * Optional soft reset check (compiled out as per Meta description)
 */
#if 0
static void soft_reset_chk(void)
{
    (void)SOFT_RST_REG_ADDRESS; /* placeholder to avoid unused warnings */
    (void)SOFT_RST_REG_DATA;
}
#endif
