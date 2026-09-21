// Author - AI Force 2.3. 21-Sep-2025 20:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name : ethernet2_reg_wr_rd_test
 * Test Description: Register default-value verification and write-read-back
 *                   verification for Ethernet2 MAC registers. Iterates over
 *                   addr_array containing mizar_ETHERNET2_MAC_CONFIGURATION,
 *                   mizar_ETHERNET2_MAC_EXT_CONFIGURATION,
 *                   mizar_ETHERNET2_MAC_PACKET_FILTER,
 *                   mizar_ETHERNET2_MAC_WD_JB_TIMEOUT.
 *                   Phase 1 (chk_rst_val): reads each register and compares
 *                   against default_value_array.
 *                   Phase 2 (chk_rd_wr): writes six data patterns and
 *                   validates read-back against computed expected values.
 */

#include "ethernet2_reg_wr_rd_test.h"
#include "test_define.inc"

/* Testcase context structure */
typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
    unsigned int checks_total;
    unsigned int checks_passed;
} ethernet2_reg_wr_rd_test_ctx_t;

static ethernet2_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Reads each register in addr_array and compares against
 *              default_value_array. Skips registers with read_mask==0
 *              or skip_rst_array==1. Mismatches increment def_fail_cnt.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int addr;
    unsigned int data_rd;

    LOGT("chk_rst_val: Starting default reset value verification");

    for (i = 0U; i < CNT; i++) {
        addr = addr_array[i];

        /* Step 4: Skip if register is not readable */
        if (read_mask_array[i] == 0x00000000U) {
            LOGT("chk_rst_val: Skipping index %u (addr=0x%08x): read_mask is 0x00000000", i, addr);
            continue;
        }

        /* Step 5: Skip if marked in skip_rst_array */
        if (skip_rst_array[i] == 1U) {
            LOGT("chk_rst_val: Skipping index %u (addr=0x%08x): skip_rst_array is 1", i, addr);
            continue;
        }

        /* Step 6: Read the register */
        data_rd = readl_reg(addr);
        g_ctx.checks_total++;

        /* Step 7: Compare against default value */
        if (data_rd != default_value_array[i]) {
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
            LOGE("chk_rst_val: FAIL index %u addr=0x%08x expected=0x%08x actual=0x%08x",
                 i, addr, default_value_array[i], data_rd);
        } else {
            g_ctx.checks_passed++;
            LOGT("chk_rst_val: PASS index %u addr=0x%08x value=0x%08x",
                 i, addr, data_rd);
        }
    }

    LOGT("chk_rst_val: Complete, def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Writes six data patterns to each register and reads back.
 *              Expected value is computed using read_mask, write_mask, and
 *              default_value. Mismatches increment wr_fail_cnt.
 * Parameters:
 *   None.
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
    unsigned int exp_val;
    unsigned int wr_n;

    LOGT("chk_rd_wr: Starting write-read verification");

    /* Step 10: Outer loop over 6 data patterns */
    for (j = 0U; j < 6U; j++) {
        data_wr = chk_val[j];
        LOGT("chk_rd_wr: Testing pattern[%u]=0x%08x", j, data_wr);

        /* Step 11: Inner write loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            /* Skip if marked in skip_array */
            if (skip_array[i] == 1U) {
                continue;
            }

            /* Skip if register is not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            /* Write the pattern */
            writel_reg(addr, data_wr);
        }

        /* Step 12: Inner read loop */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

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

            /* Step 13: Read back the register */
            data_rd = readl_reg(addr);
            g_ctx.checks_total++;

            /* Step 14: Compute wr_n */
            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);

            /* Step 15: Compute expected value */
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            /* Step 16: Compare */
            if (data_rd != exp_val) {
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
                LOGE("chk_rd_wr: FAIL index %u addr=0x%08x pattern=0x%08x expected=0x%08x actual=0x%08x",
                     i, addr, data_wr, exp_val, data_rd);
            } else {
                g_ctx.checks_passed++;
                LOGT("chk_rd_wr: PASS index %u addr=0x%08x pattern=0x%08x value=0x%08x",
                     i, addr, data_wr, data_rd);
            }
        }
    }

    LOGT("chk_rd_wr: Complete, wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
}

/*
 * Function: ethernet2_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for ethernet2_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (ethernet2_reg_wr_rd_test_ctx_t){0};

    LOGT("ethernet2_reg_wr_rd_test_init: Initialization complete");

    return 0;
}

/*
 * Function: ethernet2_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet2_reg_wr_rd_test.
 *              Phase 1: default reset value verification via chk_rst_val.
 *              Phase 2: write-read verification via chk_rd_wr.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("ethernet2_reg_wr_rd_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("ethernet2_reg_wr_rd_test_run: Starting testcase execution");

    /* Step 1: Phase 1 - Default reset value verification */
    LOGT("ethernet2_reg_wr_rd_test_run: Phase 1 - Default reset value verification");
    chk_rst_val();

    /* Step 8: Phase 2 - Write-read verification */
    LOGT("ethernet2_reg_wr_rd_test_run: Phase 2 - Write-read verification");
    chk_rd_wr();

    /* Step 17: Final pass/fail determination */
    // MANUAL_REVIEW: DV finish(0)/finish(1) was present in the source flow, but PSV/FV-native
    // equivalent uses out->status based PASS/FAIL reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("ethernet2_reg_wr_rd_test_run: Run complete: %s def_fail_cnt=%u wr_fail_cnt=%u errors=%u checks_total=%u checks_passed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.def_fail_cnt,
         g_ctx.wr_fail_cnt,
         g_ctx.errors,
         g_ctx.checks_total,
         g_ctx.checks_passed);

    return out->status;
}

/*
 * Function: ethernet2_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for ethernet2_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int ethernet2_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet2_reg_wr_rd_test_teardown: def_fail_cnt=%u wr_fail_cnt=%u errors=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt, g_ctx.errors);

    LOGT("ethernet2_reg_wr_rd_test_teardown: Teardown complete");

    return g_ctx.errors == 0U ? 0 : -1;
}
