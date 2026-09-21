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

/* Register address array */
static const uint32_t addr_array[] = {
    MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    MIZAR_MIPI_DSI_SUBSYS_DBITE,
    MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

/* Expected default values */
static const uint32_t default_val_array[] = {
    MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_DEFAULT_VAL,
    MIPI_DSI_SUBSYS_LOW_PWR_DEFAULT_VAL,
    MIPI_DSI_SUBSYS_DBITE_DEFAULT_VAL,
    MIPI_DSI_SUBSYS_DBI_FDIV_DEFAULT_VAL,
    MIPI_DSI_SUBSYS_INTERRUPT_RAW_DEFAULT_VAL
};

/* Read masks */
static const uint32_t rd_mask_array[] = {
    MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_RD_MASK,
    MIPI_DSI_SUBSYS_LOW_PWR_RD_MASK,
    MIPI_DSI_SUBSYS_DBITE_RD_MASK,
    MIPI_DSI_SUBSYS_DBI_FDIV_RD_MASK,
    MIPI_DSI_SUBSYS_INTERRUPT_RAW_RD_MASK
};

/* Write masks */
static const uint32_t wr_mask_array[] = {
    MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL_WR_MASK,
    MIPI_DSI_SUBSYS_LOW_PWR_WR_MASK,
    MIPI_DSI_SUBSYS_DBITE_WR_MASK,
    MIPI_DSI_SUBSYS_DBI_FDIV_WR_MASK,
    MIPI_DSI_SUBSYS_INTERRUPT_RAW_WR_MASK
};

#define NUM_REGS (sizeof(addr_array) / sizeof(addr_array[0]))

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
    uint32_t read_val;
    uint32_t data_wr = MIPI_DSI_SUBSYS_REG_TEST_WRITE_VAL;
    uint32_t err1 = 0U;  /* Reset value check errors */
    uint32_t err2 = 0U;  /* Write-read check errors */
    uint32_t i;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test run: starting register write-read verification");

    /* Phase 1: Reset value verification (chk_rst_val) */
    LOGT("Phase 1: Checking reset default values for %u registers", (unsigned int)NUM_REGS);

    for (i = 0U; i < NUM_REGS; i++) {
        read_val = readl_reg(addr_array[i]);
        g_ctx.checks_total++;
        if ((read_val & rd_mask_array[i]) != (default_val_array[i] & rd_mask_array[i])) {
            LOGE("Reset value check FAILED for register[%u] addr=0x%lx expected=0x%lx read=0x%lx mask=0x%lx",
                 (unsigned int)i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(default_val_array[i] & rd_mask_array[i]),
                 (unsigned long)(read_val & rd_mask_array[i]),
                 (unsigned long)rd_mask_array[i]);
            err1++;
            g_ctx.errors++;
        } else {
            LOGT("Reset value check PASSED for register[%u] addr=0x%lx value=0x%lx",
                 (unsigned int)i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(read_val & rd_mask_array[i]));
            g_ctx.checks_passed++;
        }
    }

    /* Phase 2: Write-read verification (chk_rd_wr) */
    LOGT("Phase 2: Performing write-read verification for %u registers", (unsigned int)NUM_REGS);

    for (i = 0U; i < NUM_REGS; i++) {
        writel_reg(addr_array[i], data_wr & wr_mask_array[i]);
        read_val = readl_reg(addr_array[i]);
        g_ctx.checks_total++;
        if ((read_val & rd_mask_array[i]) != (data_wr & wr_mask_array[i] & rd_mask_array[i])) {
            LOGE("Write-read check FAILED for register[%u] addr=0x%lx written=0x%lx read=0x%lx rd_mask=0x%lx wr_mask=0x%lx",
                 (unsigned int)i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(data_wr & wr_mask_array[i]),
                 (unsigned long)(read_val & rd_mask_array[i]),
                 (unsigned long)rd_mask_array[i],
                 (unsigned long)wr_mask_array[i]);
            err2++;
            g_ctx.errors++;
        } else {
            LOGT("Write-read check PASSED for register[%u] addr=0x%lx written=0x%lx read=0x%lx",
                 (unsigned int)i,
                 (unsigned long)addr_array[i],
                 (unsigned long)(data_wr & wr_mask_array[i]),
                 (unsigned long)(read_val & rd_mask_array[i]));
            g_ctx.checks_passed++;
        }
    }

    /* Update final status */
    g_ctx.checks_failed = g_ctx.errors;

    LOGT("Reset value errors (err1): %u", (unsigned int)err1);
    LOGT("Write-read errors (err2): %u", (unsigned int)err2);

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

    LOGT("mipi_dsi_subsys_reg_wr_rd_test teardown: errors=%u checks_passed=%u checks_total=%u",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total);

    return g_ctx.errors == 0U ? 0 : -1;
}
