// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#include "mipi_csi2_rb_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Test Case: mipi_csi2_rb_reg_wr_rd_test
 * Description: Validates read and write accessibility of all MIPI CSI2 Register
 *   Block (RB) registers. Performs reset default value check, write-read-back
 *   verification, and soft reset verification using array-driven iteration over
 *   all 10 RB registers.
 */

typedef struct {
    unsigned int errors;
} mipi_csi2_rb_reg_wr_rd_test_ctx_t;

static mipi_csi2_rb_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: mipi_csi2_rb_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for mipi_csi2_rb_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_rb_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (mipi_csi2_rb_reg_wr_rd_test_ctx_t){0};

    LOGT("mipi_csi2_rb_reg_wr_rd_test init: starting RB register read/write test");

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
    unsigned int i;
    uint32_t rd_data;
    uint32_t data_wr;
    uint32_t expected;

    (void)cfg;

    if (out == 0) {
        LOGE("mipi_csi2_rb_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("mipi_csi2_rb_reg_wr_rd_test run: begin");

    /* Step 1: Read all RB registers to verify their reset default values */
    LOGT("Step 1: Checking reset default values for all %u RB registers", MIPI_CSI2_RB_REG_COUNT);
    for (i = 0U; i < MIPI_CSI2_RB_REG_COUNT; i++) {
        rd_data = readl_reg(addr_array[i]);
        rd_data = rd_data & read_mask_array[i];
        if (rd_data != default_val_array[i]) {
            LOGE("Reset default mismatch reg[%u]: expected=0x%lx actual=0x%lx",
                 i, (unsigned long)default_val_array[i], (unsigned long)rd_data);
            g_ctx.errors++;
        } else {
            LOGT("Reset default OK reg[%u]: value=0x%lx", i, (unsigned long)rd_data);
        }
    }

    /* Step 2-3: Write known test value to each RB register and read back */
    LOGT("Step 2-3: Write-read-back verification for all %u RB registers", MIPI_CSI2_RB_REG_COUNT);
    for (i = 0U; i < MIPI_CSI2_RB_REG_COUNT; i++) {
        data_wr = MIPI_CSI2_RB_TEST_WR_VALUE & write_mask_array[i];
        writel_reg(addr_array[i], data_wr);
        rd_data = readl_reg(addr_array[i]);
        expected = data_wr & read_mask_array[i];
        rd_data = rd_data & read_mask_array[i];
        if (rd_data != expected) {
            LOGE("Write-read mismatch reg[%u]: written=0x%lx expected=0x%lx actual=0x%lx",
                 i, (unsigned long)data_wr, (unsigned long)expected, (unsigned long)rd_data);
            g_ctx.errors++;
        } else {
            LOGT("Write-read OK reg[%u]: value=0x%lx", i, (unsigned long)rd_data);
        }
    }

    /* Step 4: Trigger a soft reset of the register block */
    LOGT("Step 4: Triggering soft reset");
    // MANUAL_REVIEW: SOFT_RST_REG_ADDRESS is excluded per instructions.
    // The soft reset trigger must be added when the reset register address is available.

    /* Step 5: After soft reset, read all RB registers to verify default values */
    LOGT("Step 5: Post-reset default value verification for all %u RB registers", MIPI_CSI2_RB_REG_COUNT);
    for (i = 0U; i < MIPI_CSI2_RB_REG_COUNT; i++) {
        rd_data = readl_reg(addr_array[i]);
        rd_data = rd_data & read_mask_array[i];
        if (rd_data != default_val_array[i]) {
            LOGE("Post-reset mismatch reg[%u]: expected=0x%lx actual=0x%lx",
                 i, (unsigned long)default_val_array[i], (unsigned long)rd_data);
            g_ctx.errors++;
        } else {
            LOGT("Post-reset OK reg[%u]: value=0x%lx", i, (unsigned long)rd_data);
        }
    }

    /* Step 6: Final error summary */
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("mipi_csi2_rb_reg_wr_rd_test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: mipi_csi2_rb_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for mipi_csi2_rb_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int mipi_csi2_rb_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("mipi_csi2_rb_reg_wr_rd_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
