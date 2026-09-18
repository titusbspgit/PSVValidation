// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_reg_wr_rd_test
 * Description: Performs register default value verification and write-read
 *              verification on Ethernet MAC registers defined in addr_array.
 *              Registers tested: mizar_ETHERNET0_MAC_CONFIGURATION,
 *              mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
 *              mizar_ETHERNET0_MAC_PACKET_FILTER,
 *              mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
 *              mizar_ETHERNET0_MAC_HASH_TABLE_REG0.
 */

typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
} ethernet0_reg_wr_rd_test_ctx_t;

static ethernet0_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Iterates over addr_array and verifies each register reads back
 *              its expected default value. Skips registers where
 *              read_mask_array[i]==0x00000000 or skip_rst_array[i]==1.
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

    LOGT("chk_rst_val: starting default value verification");

    for (i = 0U; i < CNT; i++) {
        addr = addr_array[i];

        /* Skip if register is not readable */
        if (read_mask_array[i] == 0x00000000U) {
            continue;
        }

        /* Skip if register is in the reset-skip list */
        if (skip_rst_array[i] == 1U) {
            continue;
        }

        /* Read register and compare against default value */
        data_rd = readl_reg(addr);

        if (data_rd != default_value_array[i]) {
            LOGE("chk_rst_val FAIL: addr=0x%08x exp=0x%08x act=0x%08x index=%u",
                 addr, default_value_array[i], data_rd, i);
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
        }
    }

    LOGT("chk_rst_val: complete, def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Iterates over 6 test data patterns. For each pattern, writes
 *              the pattern to each writable register, reads back, computes
 *              expected value using read/write masks and default value, and
 *              compares. Skips registers where skip_array[i]==1 or masks are 0.
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

    LOGT("chk_rd_wr: starting write-read verification with 6 patterns");

    for (j = 0U; j < 6U; j++) {
        data_wr = chk_val[j];

        LOGT("chk_rd_wr: pattern[%u]=0x%08x", j, data_wr);

        /* Write phase: write pattern to each writable register */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            /* Skip if register is in the skip list */
            if (skip_array[i] == 1U) {
                continue;
            }

            /* Skip if register is not writable */
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            writel_reg(addr, data_wr);
        }

        /* Read-back phase: read and verify each register */
        for (i = 0U; i < CNT; i++) {
            addr = addr_array[i];

            /* Skip if register is in the skip list */
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

            data_rd = readl_reg(addr);

            /* Compute expected value */
            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                LOGE("chk_rd_wr FAIL: addr=0x%08x pattern=0x%08x exp=0x%08x act=0x%08x index=%u",
                     addr, data_wr, exp_val, data_rd, i);
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
            }
        }
    }

    LOGT("chk_rd_wr: complete, wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
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

    g_ctx = (ethernet0_reg_wr_rd_test_ctx_t){0};

    LOGT("ethernet0_reg_wr_rd_test_init: initialization complete");

    return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for ethernet0_reg_wr_rd_test.
 *              Performs register default value check (chk_rst_val) followed by
 *              write-read verification (chk_rd_wr) using 6 test patterns.
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

    LOGT("ethernet0_reg_wr_rd_test_run: starting register verification");

    /* Step 2: Check reset/default values */
    chk_rst_val();

    /* Step 3-5: Write-read verification with 6 patterns */
    chk_rd_wr();

    /* Step 6: Determine final pass/fail status */
    if ((g_ctx.def_fail_cnt > 0U) || (g_ctx.wr_fail_cnt > 0U)) {
        LOGE("ethernet0_reg_wr_rd_test_run: FAIL def_fail_cnt=%u wr_fail_cnt=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("ethernet0_reg_wr_rd_test_run: PASS all checks succeeded");
        out->status = 0;
    }

    LOGT("ethernet0_reg_wr_rd_test_run: complete, errors=%u", g_ctx.errors);

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
