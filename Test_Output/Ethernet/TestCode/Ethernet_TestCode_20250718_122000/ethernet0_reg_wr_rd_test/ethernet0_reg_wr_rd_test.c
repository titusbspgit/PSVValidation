// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_reg_wr_rd_test
 * Description: Performs register default-value verification and write-read
 *              verification for Ethernet0 MAC registers using addr_array,
 *              default_value_array, read_mask_array, write_mask_array,
 *              skip_array, skip_rst_array, and chk_val test patterns.
 */

typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} ethernet0_reg_test_ctx_t;

static ethernet0_reg_test_ctx_t g_ctx;

/*
 * Static helper: chk_rst_val
 * Description: Iterates over addr_array and verifies each register
 *              reads back its expected default reset value.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int addr;
    unsigned int data_rd;

    LOGT("chk_rst_val: Starting default value verification for %u registers", (unsigned int)CNT);

    for (i = 0U; i < (unsigned int)CNT; i++) {
        /* Step 3: Skip non-readable registers */
        if (read_mask_array[i] == 0x00000000U) {
            continue;
        }
        /* Step 3: Skip registers marked in skip_rst_array */
        if (skip_rst_array[i] == 1U) {
            continue;
        }

        addr = addr_array[i];
        /* Step 3: Read register */
        data_rd = read_reg(addr);
        g_ctx.checks_total++;

        /* Step 4: Compare against default_value_array */
        if (data_rd != default_value_array[i]) {
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
            g_ctx.checks_failed++;
            LOGE("chk_rst_val FAIL: reg[%u] addr=0x%08x read=0x%08x expected=0x%08x",
                 i, addr, data_rd, default_value_array[i]);
        } else {
            g_ctx.checks_passed++;
            LOGT("chk_rst_val PASS: reg[%u] addr=0x%08x data=0x%08x",
                 i, addr, data_rd);
        }
    }

    LOGT("chk_rst_val: Complete. def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Static helper: chk_rd_wr
 * Description: Iterates over 6 test patterns, writes each pattern to
 *              every writable register, reads back, and verifies against
 *              the computed expected value.
 */
static void chk_rd_wr(void)
{
    unsigned int i;
    unsigned int j;
    unsigned int addr;
    unsigned int data_wr;
    unsigned int data_rd;
    unsigned int exp_val;
    unsigned int wr_n;

    LOGT("chk_rd_wr: Starting write-read verification with 6 patterns");

    /* Step 6: Outer loop over 6 test patterns */
    for (j = 0U; j < 6U; j++) {
        data_wr = chk_val[j];
        LOGT("chk_rd_wr: Pattern[%u] = 0x%08x", j, data_wr);

        /* Step 7: Inner write loop */
        for (i = 0U; i < (unsigned int)CNT; i++) {
            addr = addr_array[i];

            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }
            /* Skip non-writable registers */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            write_reg(addr, data_wr);
        }

        /* Step 8: Inner read loop */
        for (i = 0U; i < (unsigned int)CNT; i++) {
            addr = addr_array[i];

            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }
            /* Skip non-writable registers */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }
            /* Skip non-readable registers */
            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }

            data_rd = read_reg(addr);
            g_ctx.checks_total++;

            /* Step 9: Compute expected value */
            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            /* Step 10: Compare data_rd against exp_val */
            if (data_rd != exp_val) {
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
                g_ctx.checks_failed++;
                LOGE("chk_rd_wr FAIL: pattern[%u]=0x%08x reg[%u] addr=0x%08x read=0x%08x expected=0x%08x",
                     j, data_wr, i, addr, data_rd, exp_val);
            } else {
                g_ctx.checks_passed++;
                LOGT("chk_rd_wr PASS: pattern[%u]=0x%08x reg[%u] addr=0x%08x data=0x%08x",
                     j, data_wr, i, addr, data_rd);
            }
        }
    }

    LOGT("chk_rd_wr: Complete. wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
}

/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet0_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Initialize test context */
    g_ctx.errors = 0U;
    g_ctx.def_fail_cnt = 0U;
    g_ctx.wr_fail_cnt = 0U;
    g_ctx.checks_total = 0U;
    g_ctx.checks_passed = 0U;
    g_ctx.checks_failed = 0U;

    LOGT("ethernet0_reg_wr_rd_test_init: Initialization complete. CNT=%u", (unsigned int)CNT);

    return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet0_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_reg_wr_rd_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet0_reg_wr_rd_test_run: Starting register default value verification");

    /* Step 2: Invoke chk_rst_val */
    chk_rst_val();

    LOGT("ethernet0_reg_wr_rd_test_run: Starting register write-read verification");

    /* Step 5: Invoke chk_rd_wr */
    chk_rd_wr();

    /* Step 11: Determine final pass/fail status */
    // MANUAL_REVIEW: DV finish() was present in the source flow. Converted to PSV/FV out->status.
    if ((g_ctx.def_fail_cnt > 0U) || (g_ctx.wr_fail_cnt > 0U)) {
        out->status = -1;
        LOGE("ethernet0_reg_wr_rd_test_run: FAIL def_fail_cnt=%u wr_fail_cnt=%u errors=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);
    } else {
        out->status = 0;
        LOGT("ethernet0_reg_wr_rd_test_run: PASS all register checks succeeded");
    }

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet0_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_reg_wr_rd_test_teardown: def_fail_cnt=%u wr_fail_cnt=%u total_errors=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);
    LOGT("ethernet0_reg_wr_rd_test_teardown: no additional cleanup required");

    return (g_ctx.errors == 0U) ? 0 : -1;
}
