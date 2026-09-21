// Author - AI Force 2.3. 18-Jul-2025 17:45 IST
// (EMBENGG-SYSAPPS)

#include "ethernet3_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet3_reg_wr_rd_test
 * Description: Performs register default value verification and write-read
 *              verification for Ethernet3 MAC registers defined in addr_array.
 *              Phase 1 (chk_rst_val): Reads each register and compares against
 *              default_value_array. Phase 2 (chk_rd_wr): Writes 6 test patterns
 *              and verifies read-back using masks and default values.
 *              soft_reset_chk() is commented out and not executed.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
} ethernet3_test_ctx_t;

static ethernet3_test_ctx_t g_ctx;

/*
 * Static helper: chk_rst_val
 * Description: Reads each register in addr_array and compares against
 *              default_value_array, skipping non-readable or skip-marked registers.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int addr;
    unsigned int data_rd;

    LOGT("ethernet3_reg_wr_rd_test: chk_rst_val() - Start default value check");

    for (i = 0U; i < CNT; i++) {
        /* Skip if register is not readable */
        if (read_mask_array[i] == 0x00000000U) {
            continue;
        }
        /* Skip if marked in skip_rst_array */
        if (skip_rst_array[i] == 1U) {
            continue;
        }

        addr = addr_array[i];
        data_rd = read_reg(addr);

        if (data_rd != default_value_array[i]) {
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
            LOGE("chk_rst_val FAIL: reg[%u] addr=0x%08x exp=0x%08x act=0x%08x",
                 i, addr, default_value_array[i], data_rd);
        } else {
            LOGT("chk_rst_val PASS: reg[%u] addr=0x%08x val=0x%08x",
                 i, addr, data_rd);
        }
    }

    LOGT("ethernet3_reg_wr_rd_test: chk_rst_val() - Complete, def_fail_cnt=%u",
         g_ctx.def_fail_cnt);
}

/*
 * Static helper: chk_rd_wr
 * Description: Writes 6 test patterns to each writable register and reads back,
 *              comparing against expected value computed from masks and defaults.
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

    LOGT("ethernet3_reg_wr_rd_test: chk_rd_wr() - Start write-read check");

    for (j = 0U; j < 6U; j++) {
        data_wr = chk_val[j];

        LOGT("chk_rd_wr: pattern[%u] = 0x%08x", j, data_wr);

        /* Write phase */
        for (i = 0U; i < CNT; i++) {
            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }
            /* Skip if register is not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            addr = addr_array[i];
            write_reg(addr, data_wr);
        }

        /* Read-back phase */
        for (i = 0U; i < CNT; i++) {
            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }
            /* Skip if register is not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }
            /* Skip if register is not readable */
            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }

            addr = addr_array[i];
            data_rd = read_reg(addr);

            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
                LOGE("chk_rd_wr FAIL: pat=0x%08x reg[%u] addr=0x%08x exp=0x%08x act=0x%08x",
                     data_wr, i, addr, exp_val, data_rd);
            } else {
                LOGT("chk_rd_wr PASS: pat=0x%08x reg[%u] addr=0x%08x val=0x%08x",
                     data_wr, i, addr, data_rd);
            }
        }
    }

    LOGT("ethernet3_reg_wr_rd_test: chk_rd_wr() - Complete, wr_fail_cnt=%u",
         g_ctx.wr_fail_cnt);
}

/*
 * Function: ethernet3_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet3_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet3_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Initialize test context */
    g_ctx.errors = 0U;
    g_ctx.def_fail_cnt = 0U;
    g_ctx.wr_fail_cnt = 0U;

    LOGT("ethernet3_reg_wr_rd_test_init: Initialization complete, CNT=%u", CNT);

    return 0;
}

/*
 * Function: ethernet3_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet3_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet3_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("ethernet3_reg_wr_rd_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet3_reg_wr_rd_test_run: Starting testcase execution");

    /* Step 1: Check reset/default values */
    LOGT("ethernet3_reg_wr_rd_test_run: Phase 1 - Default value verification");
    chk_rst_val();

    /* Step 2: Check write-read with 6 patterns */
    LOGT("ethernet3_reg_wr_rd_test_run: Phase 2 - Write-read verification");
    chk_rd_wr();

    /* soft_reset_chk() is commented out and not executed */
    // MANUAL_REVIEW: soft_reset_chk() was present in the source flow but is commented out per the test procedure.

    /* Determine final status */
    if ((g_ctx.def_fail_cnt > 0U) || (g_ctx.wr_fail_cnt > 0U)) {
        out->status = -1;
        LOGE("ethernet3_reg_wr_rd_test_run: FAIL def_fail_cnt=%u wr_fail_cnt=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);
    } else {
        out->status = 0;
        LOGT("ethernet3_reg_wr_rd_test_run: PASS all checks succeeded");
    }

    LOGT("ethernet3_reg_wr_rd_test_run: Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: ethernet3_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet3_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet3_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet3_reg_wr_rd_test_teardown: def_fail_cnt=%u wr_fail_cnt=%u errors=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);

    LOGT("ethernet3_reg_wr_rd_test_teardown: Teardown complete");

    return (g_ctx.errors == 0U) ? 0 : -1;
}
