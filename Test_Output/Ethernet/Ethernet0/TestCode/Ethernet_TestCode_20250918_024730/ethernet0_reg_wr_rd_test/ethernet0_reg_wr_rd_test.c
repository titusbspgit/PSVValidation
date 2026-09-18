// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_reg_wr_rd_test
 * Description: Performs register default-value verification and write-read
 *              verification for Ethernet0 MAC registers. Iterates over
 *              addr_array, checks reset values via chk_rst_val logic, then
 *              performs write-read verification via chk_rd_wr logic using
 *              six test patterns.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
} ethernet0_reg_wr_rd_ctx_t;

static ethernet0_reg_wr_rd_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Reads each register in addr_array and compares against
 *              default_value_array. Skips non-readable and skip-flagged registers.
 * Parameters:
 *   None (uses global arrays from test_define.inc).
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int addr;
    unsigned int data_rd;

    LOGT("chk_rst_val: Starting default value verification for %u registers", (unsigned int)CNT);

    for (i = 0U; i < CNT; i++) {
        addr = addr_array[i];

        /* Step 3: Skip if not readable */
        if (read_mask_array[i] == 0x00000000U) {
            continue;
        }

        /* Step 3: Skip if marked in skip_rst_array */
        if (skip_rst_array[i] == 1U) {
            continue;
        }

        /* Step 3: Read the register */
        data_rd = read_reg(addr);

        /* Step 4: Compare against default value */
        if (data_rd != default_value_array[i]) {
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
            LOGE("chk_rst_val FAIL: addr=0x%08x rd=0x%08x exp=0x%08x index=%u",
                 addr, data_rd, default_value_array[i], i);
        }
    }

    LOGT("chk_rst_val: Complete. def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Writes six test patterns to each writable register and reads
 *              back, comparing against the computed expected value using
 *              write_mask, read_mask, and default_value arrays.
 * Parameters:
 *   None (uses global arrays from test_define.inc).
 * Returns:
 *   void
 */
static void chk_rd_wr(void)
{
    unsigned int i;
    unsigned int j;
    unsigned int addr;
    unsigned int data_wr;
    unsigned int data_rd;
    unsigned int wr_n;
    unsigned int exp_val;

    LOGT("chk_rd_wr: Starting write-read verification with %u patterns", 6U);

    /* Step 6: Outer loop over 6 test patterns */
    for (j = 0U; j < 6U; j++) {
        data_wr = chk_val[j];

        LOGT("chk_rd_wr: Pattern[%u] = 0x%08x", j, data_wr);

        /* Step 7: Inner write loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }

            /* Skip if not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            /* Write the test pattern */
            write_reg(addr, data_wr);
        }

        /* Step 8: Inner read loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }

            /* Skip if not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            /* Skip if not readable */
            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }

            /* Read back the register */
            data_rd = read_reg(addr);

            /* Step 9: Compute expected value */
            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            /* Step 10: Compare read data against expected */
            if (data_rd != exp_val) {
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
                LOGE("chk_rd_wr FAIL: addr=0x%08x pattern=0x%08x rd=0x%08x exp=0x%08x index=%u",
                     addr, data_wr, data_rd, exp_val, i);
            }
        }
    }

    LOGT("chk_rd_wr: Complete. wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
}

/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              ethernet0_reg_wr_rd_test.
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

    LOGT("ethernet0_reg_wr_rd_test_init: Testcase initialized. CNT=%u", (unsigned int)CNT);

    return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet0_reg_wr_rd_test.
 *              Performs register default-value check (chk_rst_val) followed by
 *              write-read verification (chk_rd_wr) using six test patterns.
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

    LOGT("ethernet0_reg_wr_rd_test_run: Starting register verification");

    /* Step 2: Invoke default value check */
    chk_rst_val();

    /* Step 5: Invoke write-read check */
    chk_rd_wr();

    /* Step 11: Determine final pass/fail */
    if ((g_ctx.def_fail_cnt > 0U) || (g_ctx.wr_fail_cnt > 0U)) {
        out->status = -1;
        LOGE("ethernet0_reg_wr_rd_test_run: FAIL def_fail_cnt=%u wr_fail_cnt=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);
    } else {
        out->status = 0;
        LOGT("ethernet0_reg_wr_rd_test_run: PASS all register checks succeeded");
    }

    LOGT("ethernet0_reg_wr_rd_test_run: Complete. status=%d errors=%u",
         out->status, g_ctx.errors);

    return out->status;
}

/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling
 *              for ethernet0_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_reg_wr_rd_test_teardown: def_fail_cnt=%u wr_fail_cnt=%u errors=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
