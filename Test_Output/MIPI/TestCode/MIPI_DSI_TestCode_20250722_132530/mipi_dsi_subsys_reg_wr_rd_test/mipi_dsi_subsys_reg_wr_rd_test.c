// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_subsys_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: mipi_dsi_subsys_reg_wr_rd_test
 * Description: Performs register write-read verification on a set of MIPI DSI
 *              subsystem registers. Reads default reset values and compares
 *              against expected defaults. For writable registers, writes test
 *              data patterns, reads back, and compares using write masks.
 *              MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is read-only and skipped
 *              during write operations.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_dsi_subsys_reg_wr_rd_test_ctx_t;

static mipi_dsi_subsys_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: chk_rst_val
 * Description: Verifies default reset values for all registers in addr_array.
 * Parameters:
 *   None (uses global arrays from test_define.inc).
 * Returns:
 *   Number of mismatches detected.
 */
static unsigned int chk_rst_val(void)
{
    unsigned int i;
    uint32_t rd_val;
    uint32_t expected;
    unsigned int mismatch_count = 0U;

    LOGT("chk_rst_val: verifying default reset values for %u registers",
         MIPI_DSI_SUBSYS_REG_COUNT);

    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_COUNT; i++) {
        rd_val = readl_reg(addr_array[i]);
        expected = rst_val_array[i] & rd_mask_array[i];
        rd_val = rd_val & rd_mask_array[i];

        g_ctx.checks_total++;

        if (rd_val != expected) {
            LOGE("chk_rst_val FAIL: reg[%u] addr=0x%lx expected=0x%lx actual=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)expected,
                 (unsigned long)rd_val);
            g_ctx.errors++;
            g_ctx.checks_failed++;
            mismatch_count++;
        } else {
            LOGT("chk_rst_val PASS: reg[%u] addr=0x%lx value=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)rd_val);
            g_ctx.checks_passed++;
        }
    }

    return mismatch_count;
}

/*
 * Function: chk_rd_wr
 * Description: Writes test data patterns to writable registers and verifies
 *              read-back values using write masks. Skips read-only registers.
 * Parameters:
 *   None (uses global arrays from test_define.inc).
 * Returns:
 *   Number of mismatches detected.
 */
static unsigned int chk_rd_wr(void)
{
    unsigned int i;
    unsigned int p;
    uint32_t rd_val;
    uint32_t expected;
    unsigned int mismatch_count = 0U;

    LOGT("chk_rd_wr: verifying write-read for %u registers with %u patterns",
         MIPI_DSI_SUBSYS_REG_COUNT,
         MIPI_DSI_SUBSYS_PATTERN_COUNT);

    for (p = 0U; p < MIPI_DSI_SUBSYS_PATTERN_COUNT; p++) {
        LOGT("chk_rd_wr: pattern[%u] = 0x%lx",
             p, (unsigned long)data_patterns[p]);

        for (i = 0U; i < MIPI_DSI_SUBSYS_REG_COUNT; i++) {
            if (skip_array[i] != 0U) {
                LOGT("chk_rd_wr: SKIP reg[%u] addr=0x%lx (read-only)",
                     i, (unsigned long)addr_array[i]);
                continue;
            }

            writel_reg(addr_array[i], data_patterns[p]);

            rd_val = readl_reg(addr_array[i]);
            expected = data_patterns[p] & wr_mask_array[i];
            rd_val = rd_val & wr_mask_array[i];

            g_ctx.checks_total++;

            if (rd_val != expected) {
                LOGE("chk_rd_wr FAIL: reg[%u] addr=0x%lx pattern=0x%lx expected=0x%lx actual=0x%lx",
                     i,
                     (unsigned long)addr_array[i],
                     (unsigned long)data_patterns[p],
                     (unsigned long)expected,
                     (unsigned long)rd_val);
                g_ctx.errors++;
                g_ctx.checks_failed++;
                mismatch_count++;
            } else {
                LOGT("chk_rd_wr PASS: reg[%u] addr=0x%lx pattern=0x%lx readback=0x%lx",
                     i,
                     (unsigned long)addr_array[i],
                     (unsigned long)data_patterns[p],
                     (unsigned long)rd_val);
                g_ctx.checks_passed++;
            }
        }
    }

    return mismatch_count;
}

/*
 * Function: mipi_dsi_subsys_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_dsi_subsys_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_subsys_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_subsys_reg_wr_rd_test_ctx_t){0};

    LOGT("mipi_dsi_subsys_reg_wr_rd_test init: register write-read verification");
    LOGT("Registers under test: %u, Data patterns: %u",
         MIPI_DSI_SUBSYS_REG_COUNT,
         MIPI_DSI_SUBSYS_PATTERN_COUNT);

    return 0;
}

/*
 * Function: mipi_dsi_subsys_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for mipi_dsi_subsys_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_subsys_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int rst_mismatches;
    unsigned int wr_rd_mismatches;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test run: starting register verification");

    /* Step 1-2: Arrays are initialized in test_define.inc */
    LOGT("Step 1-2: Register arrays initialized in test_define.inc");

    /* Step 3: Execute chk_rst_val to verify default reset values */
    LOGT("Step 3: Verify default reset values");
    rst_mismatches = chk_rst_val();
    LOGT("chk_rst_val complete: mismatches=%u", rst_mismatches);

    /* Step 4-5: Execute chk_rd_wr to verify write-read for writable registers */
    LOGT("Step 4-5: Verify write-read patterns (skip read-only registers)");
    wr_rd_mismatches = chk_rd_wr();
    LOGT("chk_rd_wr complete: mismatches=%u", wr_rd_mismatches);

    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: mipi_dsi_subsys_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_dsi_subsys_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_subsys_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test teardown: errors=%u checks_total=%u",
         g_ctx.errors, g_ctx.checks_total);
    return g_ctx.errors == 0U ? 0 : -1;
}
