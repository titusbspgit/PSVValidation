// Author - AI Force 2.3. 22-Jul-2025 17:50 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_subsys_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_subsys_reg_wr_rd_test
 * Description: Validates write-read integrity of MIPI DSI subsystem registers.
 *   Reads each register to verify default reset value (chk_rst_val phase), then
 *   writes six data patterns to each writable register masked with the write mask,
 *   reads back masked with the read mask, and compares (chk_rd_wr phase).
 *   Registers: DATA_FIFO_THRESHOLD_VAL, LOW_PWR, DBITE, DBI_FDIV, INTERRUPT_RAW.
 *   INTERRUPT_RAW is skipped for writes (skip_array[4]=1).
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} mipi_dsi_subsys_reg_wr_rd_test_ctx_t;

static mipi_dsi_subsys_reg_wr_rd_test_ctx_t g_ctx;

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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test init: starting subsystem register write-read validation");

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
    uint32_t rd_data = 0U;
    uint32_t wr_data = 0U;
    uint32_t exp_data = 0U;
    unsigned int i;
    unsigned int j;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting mipi_dsi_subsys_reg_wr_rd_test register validation");

    /*
     * Phase 1: chk_rst_val - Verify default reset values
     */
    LOGT("Phase 1: Checking reset values for %u registers", MIPI_DSI_SUBSYS_REG_CNT);
    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_CNT; i++) {
        rd_data = readl_reg(addr_array[i]);
        exp_data = rst_val_array[i] & rd_mask_array[i];
        g_ctx.checks_total++;

        if ((rd_data & rd_mask_array[i]) != exp_data) {
            LOGE("Phase 1 REG[%u] FAIL: addr=0x%lx read=0x%lx expected=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(rd_data & rd_mask_array[i]),
                 (unsigned long)exp_data);
            g_ctx.errors++;
            g_ctx.checks_failed++;
        } else {
            LOGT("Phase 1 REG[%u] PASS: addr=0x%lx reset_val=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(rd_data & rd_mask_array[i]));
            g_ctx.checks_passed++;
        }
    }

    /*
     * Phase 2: chk_rd_wr - Write-Read pattern validation
     */
    LOGT("Phase 2: Checking write-read patterns, %u patterns", MIPI_DSI_SUBSYS_NUM_PATTERNS);
    for (j = 0U; j < MIPI_DSI_SUBSYS_NUM_PATTERNS; j++) {
        LOGT("Phase 2: Testing pattern 0x%lx", (unsigned long)data_patterns[j]);
        for (i = 0U; i < MIPI_DSI_SUBSYS_REG_CNT; i++) {
            if (skip_array[i] == 1U) {
                LOGT("Phase 2 REG[%u] SKIP (read-only in this test)", i);
                continue;
            }

            wr_data = data_patterns[j] & wr_mask_array[i];
            writel_reg(addr_array[i], wr_data);

            rd_data = readl_reg(addr_array[i]);
            exp_data = wr_data & rd_mask_array[i];
            g_ctx.checks_total++;

            if ((rd_data & rd_mask_array[i]) != exp_data) {
                LOGE("Phase 2 REG[%u] FAIL: addr=0x%lx wrote=0x%lx read=0x%lx expected=0x%lx",
                     i,
                     (unsigned long)addr_array[i],
                     (unsigned long)wr_data,
                     (unsigned long)(rd_data & rd_mask_array[i]),
                     (unsigned long)exp_data);
                g_ctx.errors++;
                g_ctx.checks_failed++;
            } else {
                LOGT("Phase 2 REG[%u] PASS: addr=0x%lx pattern=0x%lx readback=0x%lx",
                     i,
                     (unsigned long)addr_array[i],
                     (unsigned long)wr_data,
                     (unsigned long)(rd_data & rd_mask_array[i]));
                g_ctx.checks_passed++;
            }
        }
    }

    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    out->actual_len = 1;
    out->actual_pattern[0] = (int)g_ctx.errors;

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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test teardown: errors=%u checks_passed=%u checks_total=%u",
         g_ctx.errors, g_ctx.checks_passed, g_ctx.checks_total);
    return g_ctx.errors == 0U ? 0 : -1;
}
