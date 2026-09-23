// Author - AI Force 2.3. 22-Jul-2025 13:25 IST
// (EMBENGG-SYSAPPS)

#include "mipi_dsi_subsys_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: mipi_dsi_subsys_reg_wr_rd_test
 * Description: Performs register write-read verification on a set of MIPI DSI
 *              subsystem registers. Reads default (reset) values and compares
 *              against expected defaults. For writable registers, writes test
 *              data patterns, reads back, and compares using write masks.
 *              MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is read-only and skipped
 *              during write operations.
 */

/* ======================================================================== */
/* Test Data Arrays                                                         */
/* ======================================================================== */

#define NUM_REGS      5U
#define NUM_PATTERNS  5U

/* Register addresses under test */
static const uint32_t addr_array[NUM_REGS] = {
    MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL,
    MIZAR_MIPI_DSI_SUBSYS_LOW_PWR,
    MIZAR_MIPI_DSI_SUBSYS_DBITE,
    MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV,
    MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW
};

/* Register names for logging */
static const char *reg_names[NUM_REGS] = {
    "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL",
    "MIZAR_MIPI_DSI_SUBSYS_LOW_PWR",
    "MIZAR_MIPI_DSI_SUBSYS_DBITE",
    "MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV",
    "MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW"
};

/* Expected default (reset) values */
// MANUAL_REVIEW: Replace 0x0UL placeholders with actual reset values from register spec.
static const uint32_t rst_val_array[NUM_REGS] = {
    0x0UL,  /* DATA_FIFO_THRESHOLD_VAL */
    0x0UL,  /* LOW_PWR */
    0x0UL,  /* DBITE */
    0x0UL,  /* DBI_FDIV */
    0x0UL   /* INTERRUPT_RAW */
};

/* Read masks */
// MANUAL_REVIEW: Replace 0xFFFFFFFFUL placeholders with actual read masks from register spec.
static const uint32_t rd_mask_array[NUM_REGS] = {
    0xFFFFFFFFUL,  /* DATA_FIFO_THRESHOLD_VAL */
    0xFFFFFFFFUL,  /* LOW_PWR */
    0xFFFFFFFFUL,  /* DBITE */
    0xFFFFFFFFUL,  /* DBI_FDIV */
    0xFFFFFFFFUL   /* INTERRUPT_RAW */
};

/* Write masks */
// MANUAL_REVIEW: Replace 0xFFFFFFFFUL placeholders with actual write masks from register spec.
static const uint32_t wr_mask_array[NUM_REGS] = {
    0xFFFFFFFFUL,  /* DATA_FIFO_THRESHOLD_VAL */
    0xFFFFFFFFUL,  /* LOW_PWR */
    0xFFFFFFFFUL,  /* DBITE */
    0xFFFFFFFFUL,  /* DBI_FDIV */
    0x00000000UL   /* INTERRUPT_RAW (read-only) */
};

/* Skip flags: 1 = skip write-read verification (read-only register) */
static const uint32_t skip_array[NUM_REGS] = {
    0U,  /* DATA_FIFO_THRESHOLD_VAL */
    0U,  /* LOW_PWR */
    0U,  /* DBITE */
    0U,  /* DBI_FDIV */
    1U   /* INTERRUPT_RAW (read-only, skip write) */
};

/* Test data patterns */
static const uint32_t data_patterns[NUM_PATTERNS] = {
    0x00000000UL,
    0xFFFFFFFFUL,
    0x55555555UL,
    0xAAAAAAAAUL,
    0xA5A5A5A5UL
};

typedef struct {
    unsigned int errors;
} mipi_dsi_subsys_reg_wr_rd_test_ctx_t;

static mipi_dsi_subsys_reg_wr_rd_test_ctx_t g_ctx;

/* ======================================================================== */
/* chk_rst_val - Verify default reset values                                */
/* ======================================================================== */
static void chk_rst_val(void)
{
    uint32_t rd_data;
    uint32_t expected;
    uint32_t i;

    LOGT("--- chk_rst_val: Verify default reset values ---");

    for (i = 0U; i < NUM_REGS; i++) {
        rd_data = readl_reg(addr_array[i]);
        expected = rst_val_array[i] & rd_mask_array[i];
        rd_data = rd_data & rd_mask_array[i];

        if (rd_data == expected) {
            LOGT("[%s] Default value check: Read=0x%lx Expected=0x%lx PASS",
                 reg_names[i], (unsigned long)rd_data, (unsigned long)expected);
        } else {
            LOGE("[%s] Default value MISMATCH: Read=0x%lx Expected=0x%lx FAIL",
                 reg_names[i], (unsigned long)rd_data, (unsigned long)expected);
            g_ctx.errors++;
        }
    }
}

/* ======================================================================== */
/* chk_rd_wr - Verify write-read for writable registers                     */
/* ======================================================================== */
static void chk_rd_wr(void)
{
    uint32_t rd_data;
    uint32_t data_wr;
    uint32_t expected;
    uint32_t i;
    uint32_t p;

    LOGT("--- chk_rd_wr: Verify write-read patterns ---");

    for (p = 0U; p < NUM_PATTERNS; p++) {
        LOGT("Test pattern [%lu]: 0x%lx",
             (unsigned long)p, (unsigned long)data_patterns[p]);

        for (i = 0U; i < NUM_REGS; i++) {
            if (skip_array[i] == 1U) {
                LOGT("  [%s] SKIPPED (read-only register)", reg_names[i]);
                continue;
            }

            data_wr = data_patterns[p];
            writel_reg(addr_array[i], data_wr);

            rd_data = readl_reg(addr_array[i]);
            expected = data_wr & wr_mask_array[i];
            rd_data = rd_data & wr_mask_array[i];

            if (rd_data == expected) {
                LOGT("  [%s] Write-Read check: Written=0x%lx Read=0x%lx Expected=0x%lx PASS",
                     reg_names[i], (unsigned long)data_wr,
                     (unsigned long)rd_data, (unsigned long)expected);
            } else {
                LOGE("  [%s] Write-Read MISMATCH: Written=0x%lx Read=0x%lx Expected=0x%lx FAIL",
                     reg_names[i], (unsigned long)data_wr,
                     (unsigned long)rd_data, (unsigned long)expected);
                g_ctx.errors++;
            }
        }
    }
}

/*
 * Function: mipi_dsi_subsys_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_subsys_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_dsi_subsys_reg_wr_rd_test_ctx_t){0};

    LOGT("mipi_dsi_subsys_reg_wr_rd_test init: subsystem register write-read verification");

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
    (void)cfg;

    if (out == 0) {
        LOGE("mipi_dsi_subsys_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test run: starting register write-read verification");

    /* Steps 1-2: Initialize arrays and verify default reset values */
    LOGT("Steps 1-2: Initialize register arrays and verify default reset values");
    chk_rst_val();

    /* Steps 3-5: Write-read verification for writable registers */
    LOGT("Steps 3-5: Write-read verification with multiple test data patterns");
    chk_rd_wr();

    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_dsi_subsys_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_dsi_subsys_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_dsi_subsys_reg_wr_rd_test teardown: errors=%u", g_ctx.errors);
    return g_ctx.errors == 0U ? 0 : -1;
}
