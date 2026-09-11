// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: pcie_reg_wr_rd_test
 * Description: Validates register reset default values and write-read integrity
 *   for PCIe DBI DSP controller registers, SII registers, and PHY registers
 *   across both PCIE0 and PCIE1 controllers.
 */

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_reg_wr_rd_test_ctx_t;

static pcie_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: extract_phy_16bit
 * Description: Extracts 16-bit value from PHY register read based on address alignment.
 *   If addr % 4 is non-zero, shift right 16; else mask with 0x0000FFFF.
 * Parameters:
 *   addr - PHY register address.
 *   data_rd - Raw 32-bit value read from the register.
 * Returns:
 *   Extracted 16-bit value.
 */
static unsigned int extract_phy_16bit(unsigned long addr, unsigned int data_rd)
{
    if ((addr % 4U) != 0U) {
        return (data_rd >> 16);
    } else {
        return (data_rd & 0x0000FFFFU);
    }
}

/*
 * Function: chk_rst_val
 * Description: Checks reset default values for DBI DSP, SII, and PHY registers
 *   across PCIE0 and PCIE1 controllers.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int data_rd;
    unsigned int phy_val;

    /* Step 2: Read rc0_ctl_addr[] and compare against ctl_default[] */
    LOGT("chk_rst_val: Checking RC0 DBI DSP register reset defaults");
    for (i = 0U; i < RC0_CTL_COUNT; i++) {
        data_rd = readl_reg(rc0_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) {
            LOGE("RC0 CTL reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)rc0_ctl_addr[i], ctl_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 3: Read rc1_ctl_addr[] and compare against ctl_default[] */
    LOGT("chk_rst_val: Checking RC1 DBI DSP register reset defaults");
    for (i = 0U; i < RC1_CTL_COUNT; i++) {
        data_rd = readl_reg(rc1_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) {
            LOGE("RC1 CTL reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)rc1_ctl_addr[i], ctl_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 4: Read sii0_addr[] and compare against sii_default[] */
    LOGT("chk_rst_val: Checking SII0 register reset defaults");
    for (i = 0U; i < SII_COUNT; i++) {
        data_rd = readl_reg(sii0_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) {
            LOGE("SII0 reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)sii0_addr[i], sii_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 5: Read sii1_addr[] and compare against sii_default[] */
    LOGT("chk_rst_val: Checking SII1 register reset defaults");
    for (i = 0U; i < SII_COUNT; i++) {
        data_rd = readl_reg(sii1_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) {
            LOGE("SII1 reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)sii1_addr[i], sii_default[i], data_rd);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 6: Release PCIE0 PHY reset */
    writel_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VAL);
    LOGT("Wrote 0x%x to mizar_PCIE0_SII_PHY_RST_CONTROL", PHY_RST_CONTROL_VAL);

    /* Step 7: Release PCIE1 PHY reset */
    writel_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VAL);
    LOGT("Wrote 0x%x to mizar_PCIE1_SII_PHY_RST_CONTROL", PHY_RST_CONTROL_VAL);

    /* Step 8: Read phy0_addr[] with 16-bit extraction, compare against phy0_default[] */
    LOGT("chk_rst_val: Checking PHY0 register reset defaults");
    for (i = 0U; i < PHY_COUNT; i++) {
        data_rd = readl_reg(phy0_addr[i]);
        phy_val = extract_phy_16bit(phy0_addr[i], data_rd);
        g_ctx.checks_total++;
        if (phy_val != phy0_default[i]) {
            LOGE("PHY0 reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)phy0_addr[i], phy0_default[i], phy_val);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    /* Step 9: Read phy1_addr[] with 16-bit extraction, compare against phy1_default[] */
    LOGT("chk_rst_val: Checking PHY1 register reset defaults");
    for (i = 0U; i < PHY_COUNT; i++) {
        data_rd = readl_reg(phy1_addr[i]);
        phy_val = extract_phy_16bit(phy1_addr[i], data_rd);
        g_ctx.checks_total++;
        if (phy_val != phy1_default[i]) {
            LOGE("PHY1 reset mismatch: addr=0x%lx exp=0x%x act=0x%x",
                 (unsigned long)phy1_addr[i], phy1_default[i], phy_val);
            g_ctx.errors++;
        } else {
            g_ctx.checks_passed++;
        }
    }

    LOGT("chk_rst_val complete: errors=%u", g_ctx.errors);
}

/*
 * Function: chk_rd_wr
 * Description: Performs write-read integrity check for DBI DSP, SII, and PHY
 *   registers using multiple test patterns.
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
    unsigned int phy_val;
    unsigned int expected;

    LOGT("chk_rd_wr: Starting write-read integrity checks");

    /* Step 11: Loop over 3 test patterns */
    for (j = 0U; j < CHK_VAL_WR_RD_COUNT; j++) {
        LOGT("chk_rd_wr: pattern j=%u chk_val=0x%x chk_val_phy=0x%x",
             j, chk_val[j], chk_val_phy[j]);

        /* Step 12: Write chk_val[j] to all rc0_ctl_addr[] */
        for (i = 0U; i < RC0_CTL_COUNT; i++) {
            writel_reg(rc0_ctl_addr[i], chk_val[j]);
        }

        /* Step 13: Write chk_val[j] to all rc1_ctl_addr[] */
        for (i = 0U; i < RC1_CTL_COUNT; i++) {
            writel_reg(rc1_ctl_addr[i], chk_val[j]);
        }

        /* Step 14: Write (chk_val[j] & sii0_write_mask[i]) to sii0_addr[] */
        for (i = 0U; i < SII_COUNT; i++) {
            writel_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]);
        }

        /* Step 15: Write (chk_val[j] & sii1_write_mask[i]) to sii1_addr[] */
        for (i = 0U; i < SII_COUNT; i++) {
            writel_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]);
        }

        /* Step 16: Re-apply PCIE0 PHY reset */
        writel_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VAL);

        /* Step 17: Re-apply PCIE1 PHY reset */
        writel_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_CONTROL_VAL);

        /* Step 18: Write (chk_val_phy[j] & phy0_write_mask[i]) to phy0_addr[] */
        for (i = 0U; i < PHY_COUNT; i++) {
            writel_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }

        /* Step 19: Write (chk_val_phy[j] & phy1_write_mask[i]) to phy1_addr[] */
        for (i = 0U; i < PHY_COUNT; i++) {
            writel_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }

        /* Step 20: Read back rc0_ctl_addr[] and compare against chk_val[j] */
        for (i = 0U; i < RC0_CTL_COUNT; i++) {
            data_rd = readl_reg(rc0_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) {
                LOGE("RC0 CTL wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)rc0_ctl_addr[i], chk_val[j], chk_val[j], data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 21: Read back rc1_ctl_addr[] and compare against chk_val[j] */
        for (i = 0U; i < RC1_CTL_COUNT; i++) {
            data_rd = readl_reg(rc1_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) {
                LOGE("RC1 CTL wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)rc1_ctl_addr[i], chk_val[j], chk_val[j], data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 22: Read back sii0_addr[] and compare against (chk_val[j] & sii0_write_mask[i]) */
        for (i = 0U; i < SII_COUNT; i++) {
            data_rd = readl_reg(sii0_addr[i]);
            expected = chk_val[j] & sii0_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) {
                LOGE("SII0 wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)sii0_addr[i], chk_val[j], expected, data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 23: Read back sii1_addr[] and compare against (chk_val[j] & sii1_write_mask[i]) */
        for (i = 0U; i < SII_COUNT; i++) {
            data_rd = readl_reg(sii1_addr[i]);
            expected = chk_val[j] & sii1_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) {
                LOGE("SII1 wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)sii1_addr[i], chk_val[j], expected, data_rd);
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 24: Read back phy0_addr[] with 16-bit extraction and mask comparison */
        for (i = 0U; i < PHY_COUNT; i++) {
            data_rd = readl_reg(phy0_addr[i]);
            phy_val = extract_phy_16bit(phy0_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((phy_val & phy0_write_mask[i]) != expected) {
                LOGE("PHY0 wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)phy0_addr[i], chk_val_phy[j], expected,
                     (phy_val & phy0_write_mask[i]));
                g_ctx.errors++;
            } else {
                g_ctx.checks_passed++;
            }
        }

        /* Step 25: Read back phy1_addr[] with 16-bit extraction and mask comparison */
        for (i = 0U; i < PHY_COUNT; i++) {
            data_rd = readl_reg(phy1_addr[i]);
            phy_val = extract_phy_16bit(phy1_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((phy_val & phy1_write_mask[i]) != expected) {
                LOGE("PHY1 wr/rd mismatch: addr=0x%lx pat=0x%x exp=0x%x act=0x%x",
                     (unsigned long)phy1_addr[i], chk_val_phy[j], expected,
                     (phy_val & phy1_write_mask[i]));
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

    /* Step 26: DV finish(err2 || err1) converted to PSV/FV status */
    // MANUAL_REVIEW: DV finish(err2 || err1) was present in the source flow. Converted to PSV/FV-native out->status completion.
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
