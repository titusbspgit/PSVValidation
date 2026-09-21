// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_subsys_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_dsi_subsys_reg_wr_rd_test
 * Description: This testcase performs register write-read verification on MIPI DSI
 * subsystem registers. It uses an array-driven approach where register addresses
 * are stored in addr_array[] and iterated over. For each register, the test first
 * reads the register to check the reset default value (chk_rst_val), then performs
 * a write followed by a read-back to verify the written value (chk_rd_wr). The
 * registers under test are: MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
 * MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE,
 * MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW.
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
 * Description: Reads a register and compares the masked value against the expected default.
 * Parameters:
 *   reg_addr - Register address to read.
 *   expected_default - Expected reset default value.
 *   rd_mask - Read mask to apply before comparison.
 *   reg_index - Index of the register in the array for logging.
 * Returns:
 *   0 on match, -1 on mismatch.
 */
static int chk_rst_val(uintptr_t reg_addr,
                       uint32_t expected_default,
                       uint32_t rd_mask,
                       unsigned int reg_index)
{
    uint32_t read_val;

    read_val = readl_reg(reg_addr);
    g_ctx.checks_total++;

    if ((read_val & rd_mask) != (expected_default & rd_mask)) {
        LOGE("chk_rst_val failed: reg[%u] addr=0x%lx read=0x%lx expected=0x%lx rd_mask=0x%lx",
             reg_index,
             (unsigned long)reg_addr,
             (unsigned long)read_val,
             (unsigned long)expected_default,
             (unsigned long)rd_mask);
        g_ctx.errors++;
        return -1;
    }

    LOGT("chk_rst_val passed: reg[%u] addr=0x%lx read=0x%lx rd_mask=0x%lx",
         reg_index,
         (unsigned long)reg_addr,
         (unsigned long)read_val,
         (unsigned long)rd_mask);
    g_ctx.checks_passed++;
    return 0;
}

/*
 * Function: chk_rd_wr
 * Description: Writes a test value to a register and reads it back to verify write-read integrity.
 * Parameters:
 *   reg_addr - Register address to write and read.
 *   data_wr - Test data value to write.
 *   wr_mask - Write mask to apply to the data before writing.
 *   rd_mask - Read mask to apply before comparison.
 *   reg_index - Index of the register in the array for logging.
 * Returns:
 *   0 on match, -1 on mismatch.
 */
static int chk_rd_wr(uintptr_t reg_addr,
                     uint32_t data_wr,
                     uint32_t wr_mask,
                     uint32_t rd_mask,
                     unsigned int reg_index)
{
    uint32_t read_val;
    uint32_t write_val;

    write_val = data_wr & wr_mask;
    writel_reg(reg_addr, write_val);

    read_val = readl_reg(reg_addr);
    g_ctx.checks_total++;

    if ((read_val & rd_mask) != (write_val & rd_mask)) {
        LOGE("chk_rd_wr failed: reg[%u] addr=0x%lx written=0x%lx read=0x%lx rd_mask=0x%lx wr_mask=0x%lx",
             reg_index,
             (unsigned long)reg_addr,
             (unsigned long)write_val,
             (unsigned long)read_val,
             (unsigned long)rd_mask,
             (unsigned long)wr_mask);
        g_ctx.errors++;
        return -1;
    }

    LOGT("chk_rd_wr passed: reg[%u] addr=0x%lx written=0x%lx read=0x%lx",
         reg_index,
         (unsigned long)reg_addr,
         (unsigned long)write_val,
         (unsigned long)read_val);
    g_ctx.checks_passed++;
    return 0;
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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test init: starting testcase initialization");
    LOGT("Number of registers under test: %u", MIPI_DSI_SUBSYS_REG_COUNT);

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
    unsigned int i;
    unsigned int err1;
    unsigned int err2;
    unsigned int errors_before;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test run: starting register write-read verification");

    /* Step 1: addr_array is initialized in test_define.inc */

    /* Step 2: Phase 1 - Reset value verification (chk_rst_val) */
    LOGT("Phase 1: Reset default value verification for %u registers", MIPI_DSI_SUBSYS_REG_COUNT);
    err1 = 0U;
    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_COUNT; i++) {
        errors_before = g_ctx.errors;
        (void)chk_rst_val(addr_array[i],
                          default_val_array[i],
                          rd_mask_array[i],
                          i);
        if (g_ctx.errors > errors_before) {
            err1++;
        }
    }

    LOGT("Phase 1 complete: err1=%u", err1);

    /* Step 3: Phase 2 - Write-read verification (chk_rd_wr) */
    LOGT("Phase 2: Write-read verification for %u registers", MIPI_DSI_SUBSYS_REG_COUNT);
    err2 = 0U;
    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_COUNT; i++) {
        errors_before = g_ctx.errors;
        (void)chk_rd_wr(addr_array[i],
                        MIPI_DSI_SUBSYS_TEST_DATA_WR,
                        wr_mask_array[i],
                        rd_mask_array[i],
                        i);
        if (g_ctx.errors > errors_before) {
            err2++;
        }
    }

    LOGT("Phase 2 complete: err2=%u", err2);

    /* Step 4-5: Final status based on accumulated error counts */
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u err1=%u err2=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         err1,
         err2,
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
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
