// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_subsys_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case Name: mipi_dsi_subsys_reg_wr_rd_test
 * Description: This testcase performs register write-read verification on MIPI DSI
 *              subsystem registers. It uses an array-driven approach where register
 *              addresses are iterated over. For each register, the test first reads
 *              the register to check the reset default value, then performs a write
 *              followed by a read-back to verify the written value.
 */

#define MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS 5U

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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test_init: initialization complete");

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
    uint32_t read_data = 0U;
    uint32_t data_wr = 0xA5A5A5A5UL;
    unsigned int i = 0U;
    unsigned int err1 = 0U;
    unsigned int err2 = 0U;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test_run: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test_run: starting test execution");

    /* Step 1: Initialize register address array */
    uintptr_t addr_array[MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS] = {
        MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
        MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
        MIZAR_MIPI_DSI_SUBSYS_DBITE,
        MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
        MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
    };

    uint32_t default_val[MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS] = {
        MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL,
        MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL,
        MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL,
        MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL,
        MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL
    };

    uint32_t read_mask[MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS] = {
        MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_READ_MASK,
        MIPI_DSI_SUBSYS_LOW_PWR_READ_MASK,
        MIPI_DSI_SUBSYS_DBITE_READ_MASK,
        MIPI_DSI_SUBSYS_DBI_FDIV_READ_MASK,
        MIPI_DSI_SUBSYS_INTERRUPT_RAW_READ_MASK
    };

    uint32_t write_mask[MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS] = {
        MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WRITE_MASK,
        MIPI_DSI_SUBSYS_LOW_PWR_WRITE_MASK,
        MIPI_DSI_SUBSYS_DBITE_WRITE_MASK,
        MIPI_DSI_SUBSYS_DBI_FDIV_WRITE_MASK,
        MIPI_DSI_SUBSYS_INTERRUPT_RAW_WRITE_MASK
    };

    LOGT("Step 1: Initialized addr_array with %u register addresses", MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS);

    /* Step 2: chk_rst_val - Reset default value verification */
    LOGT("Step 2: Phase 1 - Reset default value verification");
    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS; i++) {
        read_data = readl_reg(addr_array[i]);
        g_ctx.checks_total++;
        if ((read_data & read_mask[i]) != (default_val[i] & read_mask[i])) {
            LOGE("chk_rst_val FAIL: reg[%u] addr=0x%lx read=0x%lx expected=0x%lx mask=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)read_data,
                 (unsigned long)default_val[i],
                 (unsigned long)read_mask[i]);
            err1++;
            g_ctx.errors++;
            g_ctx.checks_failed++;
        } else {
            LOGT("chk_rst_val PASS: reg[%u] addr=0x%lx read=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)read_data);
            g_ctx.checks_passed++;
        }
    }

    LOGT("Phase 1 complete: err1=%u", err1);

    /* Step 3-4: chk_rd_wr - Write-Read verification */
    LOGT("Step 3-4: Phase 2 - Write-Read verification");
    for (i = 0U; i < MIPI_DSI_SUBSYS_REG_WR_RD_NUM_REGS; i++) {
        uint32_t wr_val = data_wr & write_mask[i];
        writel_reg(addr_array[i], wr_val);
        read_data = readl_reg(addr_array[i]);
        g_ctx.checks_total++;
        if ((read_data & read_mask[i]) != (wr_val & read_mask[i])) {
            LOGE("chk_rd_wr FAIL: reg[%u] addr=0x%lx wrote=0x%lx readback=0x%lx mask=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)wr_val,
                 (unsigned long)read_data,
                 (unsigned long)read_mask[i]);
            err2++;
            g_ctx.errors++;
            g_ctx.checks_failed++;
        } else {
            LOGT("chk_rd_wr PASS: reg[%u] addr=0x%lx wrote=0x%lx readback=0x%lx",
                 i,
                 (unsigned long)addr_array[i],
                 (unsigned long)wr_val,
                 (unsigned long)read_data);
            g_ctx.checks_passed++;
        }
    }

    LOGT("Phase 2 complete: err2=%u", err2);

    /* Step 5: Final status */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test_run complete: %s err1=%u err2=%u errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         err1,
         err2,
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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test_teardown: errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return g_ctx.errors == 0U ? 0 : -1;
}
