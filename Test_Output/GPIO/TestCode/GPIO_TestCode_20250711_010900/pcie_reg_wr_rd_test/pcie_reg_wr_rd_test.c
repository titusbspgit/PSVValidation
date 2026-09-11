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
    unsigned int err1;
    unsigned int err2;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_reg_wr_rd_test_ctx_t;

static pcie_reg_wr_rd_test_ctx_t g_ctx;

/*
 * Function: phy_read_16bit
 * Description: Reads a PHY register and extracts 16-bit value based on address alignment.
 *   If addr%4 is non-zero, shifts right by 16. Otherwise masks with 0x0000FFFF.
 * Parameters:
 *   addr - PHY register address.
 * Returns:
 *   16-bit extracted value.
 */
static unsigned int phy_read_16bit(unsigned long addr)
{
    unsigned int data_rd;

    data_rd = readl_reg(addr);
    if ((addr % 4U) != 0U) {
        data_rd = data_rd >> 16;
    } else {
        data_rd = data_rd & 0x0000FFFFU;
    }
    return data_rd;
}

/*
 * Function: chk_rst_val
 * Description: Checks reset default values for all DBI DSP, SII, and PHY registers.
 * Parameters:
 *   None.
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int data_rd;

    /* Steps 2-3: Read PCIE0 and PCIE1 DBI DSP registers, verify reset defaults */
    LOGT("Checking PCIE0 DBI DSP register reset defaults");
    for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
        data_rd = readl_reg(rc0_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            LOGE("PCIE0 DBI DSP reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.err1++;
        } else {
            LOGT("PCIE0 DBI DSP reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    LOGT("Checking PCIE1 DBI DSP register reset defaults");
    for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
        data_rd = readl_reg(rc1_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            LOGE("PCIE1 DBI DSP reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 DBI DSP reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Steps 4-5: Read PCIE0 and PCIE1 SII registers, verify reset defaults */
    LOGT("Checking PCIE0 SII register reset defaults");
    for (i = 0U; i < SII_ADDR_COUNT; i++) {
        data_rd = readl_reg(sii0_addr[i]);
        if (data_rd != sii_default[i]) {
            LOGE("PCIE0 SII reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE0 SII reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    LOGT("Checking PCIE1 SII register reset defaults");
    for (i = 0U; i < SII_ADDR_COUNT; i++) {
        data_rd = readl_reg(sii1_addr[i]);
        if (data_rd != sii_default[i]) {
            LOGE("PCIE1 SII reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 SII reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Steps 6-7: Release PHY from reset */
    LOGT("Releasing PCIE0 PHY from reset");
    writel_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
    LOGT("Releasing PCIE1 PHY from reset");
    writel_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);

    /* Steps 8-9: Read PCIE0 and PCIE1 PHY registers with 16-bit extraction, verify reset defaults */
    LOGT("Checking PCIE0 PHY register reset defaults");
    for (i = 0U; i < PHY_ADDR_COUNT; i++) {
        data_rd = phy_read_16bit(phy0_addr[i]);
        if (data_rd != phy0_default[i]) {
            LOGE("PCIE0 PHY reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, phy0_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE0 PHY reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    LOGT("Checking PCIE1 PHY register reset defaults");
    for (i = 0U; i < PHY_ADDR_COUNT; i++) {
        data_rd = phy_read_16bit(phy1_addr[i]);
        if (data_rd != phy1_default[i]) {
            LOGE("PCIE1 PHY reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, phy1_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 PHY reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }
}

/*
 * Function: chk_rd_wr
 * Description: Performs write-read integrity checks on all DBI DSP, SII, and PHY registers
 *   using multiple test patterns.
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

    /* Steps 11-25: Loop over three test patterns */
    for (j = 0U; j < CHK_VAL_PATTERN_COUNT; j++) {
        LOGT("Write-read pattern %u: DBI=0x%x PHY=0x%x", j, chk_val[j], chk_val_phy[j]);

        /* Steps 12-13: Write pattern to all DBI DSP registers */
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            writel_reg(rc0_ctl_addr[i], chk_val[j]);
        }
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            writel_reg(rc1_ctl_addr[i], chk_val[j]);
        }

        /* Steps 14-15: Write pattern with write-mask to SII registers */
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            writel_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]);
        }
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            writel_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]);
        }

        /* Steps 16-17: Re-apply PHY reset release */
        writel_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
        writel_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);

        /* Steps 18-19: Write PHY-specific pattern with 13-bit mask to PHY registers */
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            writel_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            writel_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }

        /* Steps 20-21: Read back and verify DBI DSP registers */
        LOGT("Verifying DBI DSP registers for pattern 0x%x", chk_val[j]);
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            data_rd = readl_reg(rc0_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                LOGE("PCIE0 DBI DSP reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, chk_val[j], data_rd);
                g_ctx.err1++;
            }
        }
        for (i = 0U; i < RC_CTL_ADDR_COUNT; i++) {
            data_rd = readl_reg(rc1_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                LOGE("PCIE1 DBI DSP reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, chk_val[j], data_rd);
                g_ctx.err1++;
            }
        }

        /* Steps 22-23: Read back and verify SII registers */
        LOGT("Verifying SII registers for pattern 0x%x", chk_val[j]);
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            data_rd = readl_reg(sii0_addr[i]);
            if (data_rd != (chk_val[j] & sii0_write_mask[i])) {
                LOGE("PCIE0 SII reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, (chk_val[j] & sii0_write_mask[i]), data_rd);
                g_ctx.err1++;
            }
        }
        for (i = 0U; i < SII_ADDR_COUNT; i++) {
            data_rd = readl_reg(sii1_addr[i]);
            if (data_rd != (chk_val[j] & sii1_write_mask[i])) {
                LOGE("PCIE1 SII reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, (chk_val[j] & sii1_write_mask[i]), data_rd);
                g_ctx.err1++;
            }
        }

        /* Steps 24-25: Read back and verify PHY registers with 16-bit extraction */
        LOGT("Verifying PHY registers for pattern 0x%x", chk_val_phy[j]);
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            data_rd = phy_read_16bit(phy0_addr[i]);
            if ((data_rd & phy0_write_mask[i]) != (chk_val_phy[j] & PHY_13BIT_MASK)) {
                LOGE("PCIE0 PHY reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, (chk_val_phy[j] & PHY_13BIT_MASK), (data_rd & phy0_write_mask[i]));
                g_ctx.err1++;
            }
        }
        for (i = 0U; i < PHY_ADDR_COUNT; i++) {
            data_rd = phy_read_16bit(phy1_addr[i]);
            if ((data_rd & phy1_write_mask[i]) != (chk_val_phy[j] & PHY_13BIT_MASK)) {
                LOGE("PCIE1 PHY reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, (chk_val_phy[j] & PHY_13BIT_MASK), (data_rd & phy1_write_mask[i]));
                g_ctx.err1++;
            }
        }
    }
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

    /* Step 1: test_case() calls chk_rst_val() */
    LOGT("Phase 1: Checking reset default values");
    chk_rst_val();

    /* Step 10: test_case() calls chk_rd_wr() */
    LOGT("Phase 2: Checking write-read integrity");
    chk_rd_wr();

    /* Step 26: DV finish(err2 || err1) converted to PSV/FV status reporting */
    // MANUAL_REVIEW: DV finish(err2 || err1) was present in the source flow. Converted to PSV/FV-native out->status completion.
    g_ctx.errors = g_ctx.err1 + g_ctx.err2;
    if ((g_ctx.err2 != 0U) || (g_ctx.err1 != 0U)) {
        out->status = -1;
    } else {
        out->status = 0;
    }

    g_ctx.checks_failed = g_ctx.errors;

    LOGT("Run complete: %s err1=%u err2=%u total_errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.err1, g_ctx.err2, g_ctx.errors);

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

    LOGT("PCIe register write/read test teardown: err1=%u err2=%u", g_ctx.err1, g_ctx.err2);
    return ((g_ctx.err2 != 0U) || (g_ctx.err1 != 0U)) ? -1 : 0;
}
