// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_reg_wr_rd_test
 * Description: Validates register reset default values and write-read integrity
 *              for PCIe DBI DSP controller registers, SII registers, and PHY
 *              registers across both PCIE0 and PCIE1 controllers.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_reg_wr_rd_test_ctx_t;

static pcie_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: pcie_phy_extract_16bit
 * Description: Extracts 16-bit value from a 32-bit register read based on
 *              address alignment. If addr % 4 is non-zero, shift right 16;
 *              otherwise mask with 0x0000FFFF.
 * Parameters:
 *   addr    - Register address used for alignment check.
 *   data_rd - Raw 32-bit value read from the register.
 * Returns:
 *   Extracted 16-bit value.
 */
static unsigned int pcie_phy_extract_16bit(unsigned long addr, unsigned int data_rd)
{
    if ((addr % 4U) != 0U) {
        return (data_rd >> 16);
    } else {
        return (data_rd & 0x0000FFFFU);
    }
}

/*
 * Function: chk_rst_val
 * Description: Checks reset default values for DBI DSP controller registers,
 *              SII registers, and PHY registers on both PCIE0 and PCIE1.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int data_rd;
    unsigned int extracted;

    LOGT("chk_rst_val: checking DBI DSP controller register defaults");

    /* Step 2: Check rc0_ctl_addr[] reset defaults */
    for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) {
            LOGE("rc0_ctl_addr[%u] mismatch: exp=0x%x actual=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 3: Check rc1_ctl_addr[] reset defaults */
    for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) {
            LOGE("rc1_ctl_addr[%u] mismatch: exp=0x%x actual=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    LOGT("chk_rst_val: checking SII register defaults");

    /* Step 4: Check sii0_addr[] reset defaults */
    for (i = 0U; i < SII_ADDR_COUNT; i++) {
        data_rd = read_reg(sii0_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) {
            LOGE("sii0_addr[%u] mismatch: exp=0x%x actual=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 5: Check sii1_addr[] reset defaults */
    for (i = 0U; i < SII_ADDR_COUNT; i++) {
        data_rd = read_reg(sii1_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) {
            LOGE("sii1_addr[%u] mismatch: exp=0x%x actual=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    LOGT("chk_rst_val: releasing PHY reset");

    /* Step 6: Release PCIE0 PHY reset */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PCIE_PHY_RST_CONTROL_VAL);
    LOGT("write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x%x)", PCIE_PHY_RST_CONTROL_VAL);

    /* Step 7: Release PCIE1 PHY reset */
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PCIE_PHY_RST_CONTROL_VAL);
    LOGT("write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x%x)", PCIE_PHY_RST_CONTROL_VAL);

    LOGT("chk_rst_val: checking PHY register defaults");

    /* Step 8: Check phy0_addr[] reset defaults with 16-bit extraction */
    for (i = 0U; i < PHY_ADDR_COUNT; i++) {
        data_rd = read_reg(phy0_addr[i]);
        extracted = pcie_phy_extract_16bit(phy0_addr[i], data_rd);
        g_ctx.checks_total++;
        if (extracted != phy0_default[i]) {
            LOGE("phy0_addr[%u] (0x%lx) mismatch: exp=0x%x actual=0x%x",
                 i, (unsigned long)phy0_addr[i], phy0_default[i], extracted);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 9: Check phy1_addr[] reset defaults with 16-bit extraction */
    for (i = 0U; i < PHY_ADDR_COUNT; i++) {
        data_rd = read_reg(phy1_addr[i]);
        extracted = pcie_phy_extract_16bit(phy1_addr[i], data_rd);
        g_ctx.checks_total++;
        if (extracted != phy1_default[i]) {
            LOGE("phy1_addr[%u] (0x%lx) mismatch: exp=0x%x actual=0x%x",
                 i, (unsigned long)phy1_addr[i], phy1_default[i], extracted);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    LOGT("chk_rst_val complete: errors=%u", g_ctx.errors);
}

/*
 * Function: chk_rd_wr
 * Description: Performs write-read integrity checks for DBI DSP controller
 *              registers, SII registers, and PHY registers using multiple
 *              test patterns across both PCIE0 and PCIE1.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void chk_rd_wr(void)
{
    unsigned int i;
    unsigned int j;
    unsigned int data_rd;
    unsigned int extracted;
    unsigned int expected;

    LOGT("chk_rd_wr: starting write-read integrity checks");

    /* Step 11: Loop over 3 test patterns */
    for (j = 0U; j < CHK_VAL_WR_RD_COUNT; j++) {
        LOGT("chk_rd_wr: pattern[%u] = 0x%x", j, chk_val[j]);

        /* Step 12: Write chk_val[j] to rc0_ctl_addr[] */
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            write_reg(rc0_ctl_addr[i], chk_val[j]);
        }

        /* Step 13: Write chk_val[j] to rc1_ctl_addr[] */
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            write_reg(rc1_ctl_addr[i], chk_val[j]);
        }

        /* Step 14: Write masked value to sii0_addr[] */
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            write_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]);
        }

        /* Step 15: Write masked value to sii1_addr[] */
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            write_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]);
        }

        /* Step 16: Re-apply PCIE0 PHY reset */
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PCIE_PHY_RST_CONTROL_VAL);

        /* Step 17: Re-apply PCIE1 PHY reset */
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PCIE_PHY_RST_CONTROL_VAL);

        /* Step 18: Write masked PHY value to phy0_addr[] */
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }

        /* Step 19: Write masked PHY value to phy1_addr[] */
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }

        /* Step 20: Read back and verify rc0_ctl_addr[] */
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) {
                LOGE("rc0_ctl_addr[%u] wr/rd mismatch: pattern=0x%x exp=0x%x actual=0x%x",
                     i, chk_val[j], chk_val[j], data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 21: Read back and verify rc1_ctl_addr[] */
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            data_rd = read_reg(rc1_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) {
                LOGE("rc1_ctl_addr[%u] wr/rd mismatch: pattern=0x%x exp=0x%x actual=0x%x",
                     i, chk_val[j], chk_val[j], data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 22: Read back and verify sii0_addr[] */
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            data_rd = read_reg(sii0_addr[i]);
            expected = chk_val[j] & sii0_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) {
                LOGE("sii0_addr[%u] wr/rd mismatch: exp=0x%x actual=0x%x",
                     i, expected, data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 23: Read back and verify sii1_addr[] */
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            data_rd = read_reg(sii1_addr[i]);
            expected = chk_val[j] & sii1_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) {
                LOGE("sii1_addr[%u] wr/rd mismatch: exp=0x%x actual=0x%x",
                     i, expected, data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 24: Read back and verify phy0_addr[] with 16-bit extraction */
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            data_rd = read_reg(phy0_addr[i]);
            extracted = pcie_phy_extract_16bit(phy0_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((extracted & phy0_write_mask[i]) != expected) {
                LOGE("phy0_addr[%u] wr/rd mismatch: exp=0x%x actual_masked=0x%x",
                     i, expected, (extracted & phy0_write_mask[i]));
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 25: Read back and verify phy1_addr[] with 16-bit extraction */
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            data_rd = read_reg(phy1_addr[i]);
            extracted = pcie_phy_extract_16bit(phy1_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((extracted & phy1_write_mask[i]) != expected) {
                LOGE("phy1_addr[%u] wr/rd mismatch: exp=0x%x actual_masked=0x%x",
                     i, expected, (extracted & phy1_write_mask[i]));
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }
    }

    LOGT("chk_rd_wr complete: errors=%u", g_ctx.errors);
}

/*
 * Function: pcie_reg_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (pcie_reg_wr_rd_test_ctx_t){0};

    LOGT("PCIe register write/read test init");

    return 0;
}

/*
 * Function: pcie_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("PCIe reg wr/rd test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting PCIe register write/read test run");

    /* Step 1-9: Check reset default values */
    chk_rst_val();

    /* Step 10-25: Check write-read integrity */
    chk_rd_wr();

    // MANUAL_REVIEW: DV finish(err2 || err1) was present in the source flow (step 26).
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.

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
 * Function: pcie_reg_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_reg_wr_rd_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("PCIe register write/read test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
