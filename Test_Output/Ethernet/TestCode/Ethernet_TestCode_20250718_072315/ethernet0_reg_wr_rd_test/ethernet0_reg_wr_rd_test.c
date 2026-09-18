// Author - AI Force 2.3. 18-Jul-2025 01:53 IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: ethernet0_reg_wr_rd_test
 * Description: Performs register default-value verification and write-read
 *   verification for Ethernet0 MAC registers. Iterates over addr_array
 *   containing mizar_ETHERNET0_MAC_CONFIGURATION, mizar_ETHERNET0_MAC_EXT_CONFIGURATION,
 *   mizar_ETHERNET0_MAC_PACKET_FILTER, mizar_ETHERNET0_MAC_WD_JB_TIMEOUT,
 *   mizar_ETHERNET0_MAC_HASH_TABLE_REG0. Checks default reset values, then
 *   writes 6 test patterns and verifies read-back using write/read masks.
 */

/* Test context structure */
typedef struct {
    unsigned int errors;
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
} eth0_reg_test_ctx_t;

static eth0_reg_test_ctx_t g_ctx;

/*
 * Static helper: chk_rst_val
 * Checks default reset values for all registers in addr_array.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    uint32_t addr;
    uint32_t data_rd;

    LOGT("chk_rst_val: Starting default reset value verification");

    for (i = 0U; i < CNT; i++) {
        /* Skip non-readable registers */
        if (read_mask_array[i] == 0x00000000U) {
            continue;
        }
        /* Skip registers marked in skip_rst_array */
        if (skip_rst_array[i] == 1U) {
            continue;
        }

        addr = addr_array[i];
        data_rd = read_reg(addr);

        if (data_rd != default_value_array[i]) {
            LOGE("chk_rst_val FAIL: index=%u addr=0x%08x exp=0x%08x actual=0x%08x",
                 i,
                 (unsigned int)addr,
                 (unsigned int)default_value_array[i],
                 (unsigned int)data_rd);
            g_ctx.def_fail_cnt++;
            g_ctx.errors++;
        } else {
            LOGT("chk_rst_val PASS: index=%u addr=0x%08x val=0x%08x",
                 i,
                 (unsigned int)addr,
                 (unsigned int)data_rd);
        }
    }

    LOGT("chk_rst_val: Complete, def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Static helper: chk_rd_wr
 * Writes 6 test patterns to each writable register and verifies read-back.
 */
static void chk_rd_wr(void)
{
    unsigned int i;
    unsigned int j;
    uint32_t addr;
    uint32_t data_wr;
    uint32_t data_rd;
    uint32_t wr_n;
    uint32_t exp_val;

    LOGT("chk_rd_wr: Starting write-read verification with %u patterns", CHK_VAL_CNT);

    for (j = 0U; j < CHK_VAL_CNT; j++) {
        data_wr = chk_val[j];

        LOGT("chk_rd_wr: Pattern[%u] = 0x%08x", j, (unsigned int)data_wr);

        /* Write loop */
        for (i = 0U; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }

            addr = addr_array[i];
            write_reg(addr, data_wr);
        }

        /* Read and verify loop */
        for (i = 0U; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }

            addr = addr_array[i];
            data_rd = read_reg(addr);

            wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                LOGE("chk_rd_wr FAIL: pattern[%u]=0x%08x index=%u addr=0x%08x exp=0x%08x actual=0x%08x",
                     j,
                     (unsigned int)data_wr,
                     i,
                     (unsigned int)addr,
                     (unsigned int)exp_val,
                     (unsigned int)data_rd);
                g_ctx.wr_fail_cnt++;
                g_ctx.errors++;
            } else {
                LOGT("chk_rd_wr PASS: pattern[%u]=0x%08x index=%u addr=0x%08x val=0x%08x",
                     j,
                     (unsigned int)data_wr,
                     i,
                     (unsigned int)addr,
                     (unsigned int)data_rd);
            }
        }
    }

    LOGT("chk_rd_wr: Complete, wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
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

    g_ctx = (eth0_reg_test_ctx_t){0};

    LOGT("ethernet0_reg_wr_rd_test_init: Testcase initialization complete");
    LOGT("ethernet0_reg_wr_rd_test_init: Register count CNT=%u", CNT);

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

    LOGT("ethernet0_reg_wr_rd_test_run: Starting register default value check");

    /* Step 1: Check default reset values */
    chk_rst_val();

    LOGT("ethernet0_reg_wr_rd_test_run: Starting register write-read check");

    /* Step 2: Check write-read for 6 test patterns */
    chk_rd_wr();

    /* Determine final pass/fail status */
    if ((g_ctx.def_fail_cnt > 0U) || (g_ctx.wr_fail_cnt > 0U)) {
        LOGE("ethernet0_reg_wr_rd_test_run: FAIL def_fail_cnt=%u wr_fail_cnt=%u",
             g_ctx.def_fail_cnt,
             g_ctx.wr_fail_cnt);
        out->status = -1;
    } else {
        LOGT("ethernet0_reg_wr_rd_test_run: PASS all checks succeeded");
        out->status = 0;
    }

    LOGT("ethernet0_reg_wr_rd_test_run: Run complete: %s errors=%u def_fail=%u wr_fail=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.def_fail_cnt,
         g_ctx.wr_fail_cnt);

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

    LOGT("ethernet0_reg_wr_rd_test_teardown: errors=%u def_fail_cnt=%u wr_fail_cnt=%u",
         g_ctx.errors,
         g_ctx.def_fail_cnt,
         g_ctx.wr_fail_cnt);

    return (g_ctx.errors == 0U) ? 0 : -1;
}
