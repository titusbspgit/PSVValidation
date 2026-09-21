// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * File: ethernet1_reg_wr_rd_test.c
 * Description: FV testcase for Ethernet1 register default-value verification
 *              and write-read verification of MAC registers.
 *              Verifies reset defaults via chk_rst_val and performs write-read
 *              checks with six data patterns via chk_rd_wr.
 */

#include "ethernet1_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase context structure for ethernet1_reg_wr_rd_test.
 * Tracks error counts and check statistics.
 */
typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
    int def_fail_cnt;
    int wr_fail_cnt;
} ethernet1_reg_wr_rd_test_ctx_t;

static ethernet1_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Reads each register in addr_array and compares against
 *              expected default reset value in default_value_array.
 *              Skips registers with read_mask 0 or skip_rst_array set.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int data_rd;
    unsigned int addr;

    LOGT("chk_rst_val: Starting default reset value verification");

    for (i = 0U; i < CNT; i++) {
        addr = addr_array[i];

        /* Step 3: Skip if not readable */
        if (read_mask_array[i] == 0x00000000U) {
            LOGT("chk_rst_val: Skipping index %u (read_mask is 0x00000000)", i);
            continue;
        }

        /* Step 4: Skip if skip_rst_array is set */
        if (skip_rst_array[i] == 1U) {
            LOGT("chk_rst_val: Skipping index %u (skip_rst_array is 1)", i);
            continue;
        }

        /* Step 5: Read register */
        data_rd = readl_reg((uintptr_t)addr);
        g_ctx.checks_total++;

        /* Step 6: Compare against default value */
        if (data_rd != default_value_array[i]) {
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
            LOGE("chk_rst_val: Index %u, Addr 0x%08x, Expected 0x%08x, Read 0x%08x",
                 i, addr, default_value_array[i], data_rd);
        } else {
            g_ctx.checks_passed++;
            LOGT("chk_rst_val: Index %u, Addr 0x%08x, Value 0x%08x matches default",
                 i, addr, data_rd);
        }
    }

    g_ctx.checks_failed = g_ctx.errors;
    LOGT("chk_rst_val: Completed. def_fail_cnt=%d", g_ctx.def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: For each of six test patterns in chk_val[], writes the pattern
 *              to each writable register, reads back, and computes the expected
 *              value using read/write masks and default values.
 */
static void chk_rd_wr(void)
{
    unsigned int i, j;
    unsigned int data_wr, data_rd;
    unsigned int addr;
    unsigned int wr_n, exp_val;

    LOGT("chk_rd_wr: Starting write-read verification");

    /* Step 7: Iterate over 6 patterns */
    for (j = 0U; j < 6U; j++) {
        /* Step 8: Set data_wr to current pattern */
        data_wr = chk_val[j];
        LOGT("chk_rd_wr: Pattern %u = 0x%08x", j, data_wr);

        /* Step 9: Write loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            if (skip_array[i] == 1U) {
                continue;
            }

            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            writel_reg((uintptr_t)addr, (uint64_t)data_wr);
            LOGT("chk_rd_wr: Write Addr 0x%08x, Data 0x%08x", addr, data_wr);
        }

        /* Step 10: Read loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            if (skip_array[i] == 1U) {
                continue;
            }

            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }

            /* Step 11: Read register */
            data_rd = (unsigned int)readl_reg((uintptr_t)addr);
            g_ctx.checks_total++;

            /* Step 12: Compute wr_n */
            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);

            /* Step 13: Compute exp_val */
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            /* Step 14: Compare data_rd against exp_val */
            if (data_rd != exp_val) {
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
                LOGE("chk_rd_wr: Pattern %u, Index %u, Addr 0x%08x, Expected 0x%08x, Read 0x%08x",
                     j, i, addr, exp_val, data_rd);
            } else {
                g_ctx.checks_passed++;
                LOGT("chk_rd_wr: Pattern %u, Index %u, Addr 0x%08x, Value 0x%08x matches expected",
                     j, i, addr, data_rd);
            }
        }
    }

    g_ctx.checks_failed = g_ctx.errors;
    LOGT("chk_rd_wr: Completed. wr_fail_cnt=%d", g_ctx.wr_fail_cnt);
}

/*
 * Function: ethernet1_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet1_reg_wr_rd_test. Zeroes the context structure.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (ethernet1_reg_wr_rd_test_ctx_t){0};

    LOGT("ethernet1_reg_wr_rd_test init: Ethernet1 register write-read test initialization");

    return 0;
}

/*
 * Function: ethernet1_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet1_reg_wr_rd_test.
 *              Calls chk_rst_val for default value verification, then chk_rd_wr
 *              for write-read verification with six data patterns.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("ethernet1_reg_wr_rd_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet1_reg_wr_rd_test: Starting register default-value and write-read verification");

    /* Step 2: Call chk_rst_val - default reset value verification */
    LOGT("Step: Calling chk_rst_val() - Default reset value verification");
    chk_rst_val();

    /* Step 7: Call chk_rd_wr - write-read verification */
    LOGT("Step: Calling chk_rd_wr() - Write-read verification with 6 patterns");
    chk_rd_wr();

    /* Step 15: Final pass/fail determination */
    // MANUAL_REVIEW: DV finish() was present in the source flow. Converted to out->status PSV/FV-native reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u def_fail_cnt=%d wr_fail_cnt=%d checks_passed=%u checks_total=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.def_fail_cnt,
         g_ctx.wr_fail_cnt,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return out->status;
}

/*
 * Function: ethernet1_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for ethernet1_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet1_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet1_reg_wr_rd_test teardown: def_fail_cnt=%d wr_fail_cnt=%d errors=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
