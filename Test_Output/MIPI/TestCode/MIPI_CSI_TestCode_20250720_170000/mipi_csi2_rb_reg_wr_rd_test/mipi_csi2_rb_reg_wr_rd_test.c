// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_rb_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * mipi_csi2_rb_reg_wr_rd_test
 * Validates the default reset values and write-read functionality of 10
 * MIPI CSI-2 RB REG block registers. Phase 1 checks reset defaults.
 * Phase 2 writes 6 test patterns and verifies read-back values.
 */

/* Testcase context structure */
typedef struct {
    unsigned int def_fail_cnt;
    unsigned int wr_fail_cnt;
} mipi_csi2_rb_reg_wr_rd_ctx_t;

static mipi_csi2_rb_reg_wr_rd_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Reads each register and compares against default_value_array.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    uint32_t rd_val;

    LOGT("chk_rst_val: checking default reset values");

    for (i = 0U; i < CNT; i++) {
        /* Skip non-readable registers */
        if (read_mask_array[i] == 0x00000000U) {
            LOGT("chk_rst_val: skip index %u (non-readable)", i);
            continue;
        }

        rd_val = readl_reg(addr_array[i]);
        rd_val = rd_val & read_mask_array[i];

        if (rd_val != default_value_array[i]) {
            LOGE("chk_rst_val: MISMATCH index=%u addr=0x%lx read=0x%lx expected=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)rd_val,
                 (unsigned long)default_value_array[i]);
            g_ctx.def_fail_cnt++;
        } else {
            LOGT("chk_rst_val: MATCH index=%u addr=0x%lx val=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)rd_val);
        }
    }

    LOGT("chk_rst_val: complete, def_fail_cnt=%u", g_ctx.def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Writes 6 test patterns to each writable register and verifies read-back.
 * Parameters:
 *   None.
 * Returns:
 *   void.
 */
static void chk_rd_wr(void)
{
    unsigned int i;
    unsigned int p;
    uint32_t wr_val;
    uint32_t rd_val;
    uint32_t expected_val;

    LOGT("chk_rd_wr: starting write-read verification");

    for (p = 0U; p < 6U; p++) {
        LOGT("chk_rd_wr: pattern %u = 0x%lx", p, (unsigned long)chk_val[p]);

        for (i = 0U; i < CNT; i++) {
            /* Skip registers marked in skip_array */
            if (skip_array[i] == 1U) {
                LOGT("chk_rd_wr: skip index %u (skip_array)", i);
                continue;
            }

            /* Skip non-writable registers */
            if (write_mask_array[i] == 0x00000000U) {
                LOGT("chk_rd_wr: skip index %u (non-writable)", i);
                continue;
            }

            /* Skip non-readable registers */
            if (read_mask_array[i] == 0x00000000U) {
                LOGT("chk_rd_wr: skip index %u (non-readable)", i);
                continue;
            }

            wr_val = chk_val[p];
            writel_reg(addr_array[i], wr_val);

            rd_val = readl_reg(addr_array[i]);

            /* Compute expected value */
            expected_val = (wr_val & write_mask_array[i]) |
                           (default_value_array[i] & ~write_mask_array[i]);
            expected_val = expected_val & read_mask_array[i];
            rd_val = rd_val & read_mask_array[i];

            if (rd_val != expected_val) {
                LOGE("chk_rd_wr: MISMATCH index=%u pattern=%u addr=0x%lx wrote=0x%lx read=0x%lx expected=0x%lx",
                     i, p,
                     (unsigned long)addr_array[i],
                     (unsigned long)wr_val,
                     (unsigned long)rd_val,
                     (unsigned long)expected_val);
                g_ctx.wr_fail_cnt++;
            } else {
                LOGT("chk_rd_wr: MATCH index=%u pattern=%u addr=0x%lx val=0x%lx",
                     i, p,
                     (unsigned long)addr_array[i],
                     (unsigned long)rd_val);
            }
        }
    }

    LOGT("chk_rd_wr: complete, wr_fail_cnt=%u", g_ctx.wr_fail_cnt);
}

/*
 * Function: mipi_csi2_rb_reg_wr_rd_test_init
 * Description: Performs testcase initialization for mipi_csi2_rb_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_rb_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_rb_reg_wr_rd_ctx_t){0};

    LOGT("mipi_csi2_rb_reg_wr_rd_test init: starting");
    LOGT("mipi_csi2_rb_reg_wr_rd_test init: complete");

    return 0;
}

/*
 * Function: mipi_csi2_rb_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for mipi_csi2_rb_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_rb_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("RB_REG: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_rb_reg_wr_rd_test run: starting");

    /* Phase 1: Check default reset values */
    chk_rst_val();

    /* Phase 2: Write-read verification with 6 test patterns */
    chk_rd_wr();

    /* soft_reset_chk() is commented out in original source and not executed */
    // MANUAL_REVIEW: soft_reset_chk() was present in source but commented out
    // in test_case(). Not included in FV testcase per Meta TestPlan JSON.

    /* Final status */
    if ((g_ctx.def_fail_cnt == 0U) && (g_ctx.wr_fail_cnt == 0U)) {
        out->status = 0;
        LOGT("mipi_csi2_rb_reg_wr_rd_test run: PASS def_fail=%u wr_fail=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);
    } else {
        out->status = -1;
        LOGE("mipi_csi2_rb_reg_wr_rd_test run: FAIL def_fail=%u wr_fail=%u",
             g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);
    }

    return out->status;
}

/*
 * Function: mipi_csi2_rb_reg_wr_rd_test_teardown
 * Description: Performs testcase cleanup for mipi_csi2_rb_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_rb_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    // MANUAL_REVIEW: DV finish(0) was present in the source flow, converted to
    // PSV/FV-native status reporting via out->status in the run function.

    LOGT("mipi_csi2_rb_reg_wr_rd_test teardown: def_fail=%u wr_fail=%u",
         g_ctx.def_fail_cnt, g_ctx.wr_fail_cnt);

    return ((g_ctx.def_fail_cnt == 0U) && (g_ctx.wr_fail_cnt == 0U)) ? 0 : -1;
}
