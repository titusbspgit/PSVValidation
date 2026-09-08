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
    unsigned int err1;
    unsigned int err2;
} pcie_reg_wr_rd_test_ctx_t;

static pcie_reg_wr_rd_test_ctx_t g_ctx;

/* PCIE0 DBI DSP register addresses */
static const unsigned long rc0_ctl_addr[PCIE_DBI_DSP_REG_COUNT] = {
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

/* PCIE1 DBI DSP register addresses */
static const unsigned long rc1_ctl_addr[PCIE_DBI_DSP_REG_COUNT] = {
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

/* DBI DSP register reset default values */
static const unsigned int ctl_default[PCIE_DBI_DSP_REG_COUNT] = {
    0x0, 0x0, 0x0, 0x0, 0x0
};

/* PCIE0 SII register addresses */
static const unsigned long sii0_addr[PCIE_SII_REG_COUNT] = {
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    mizar_PCIE0_SII_PHY_CONTROL_23
};

/* PCIE1 SII register addresses */
static const unsigned long sii1_addr[PCIE_SII_REG_COUNT] = {
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    mizar_PCIE1_SII_PHY_CONTROL_23
};

/* SII register reset default values */
static const unsigned int sii_default[PCIE_SII_REG_COUNT] = {
    0x0, 0x0, 0x0
};

/* SII write masks */
static const unsigned int sii0_write_mask[PCIE_SII_REG_COUNT] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xF000F
};

static const unsigned int sii1_write_mask[PCIE_SII_REG_COUNT] = {
    0xFFFFFFFF, 0xFFFFFFFF, 0xF000F
};

/* PCIE0 PHY register addresses */
static const unsigned long phy0_addr[PCIE_PHY_REG_COUNT] = {
    0xE68860B8UL, 0xE68862B8UL, 0xE68864B8UL
};

/* PCIE1 PHY register addresses */
static const unsigned long phy1_addr[PCIE_PHY_REG_COUNT] = {
    0xE68A60B8UL, 0xE68A62B8UL, 0xE68A64B8UL
};

/* PHY register reset default values */
static const unsigned int phy0_default[PCIE_PHY_REG_COUNT] = {
    0x0, 0x0, 0x0
};

static const unsigned int phy1_default[PCIE_PHY_REG_COUNT] = {
    0x0, 0x0, 0x0
};

/* PHY write masks */
static const unsigned int phy0_write_mask[PCIE_PHY_REG_COUNT] = {
    0x1FFF, 0x1FFF, 0x1FFF
};

static const unsigned int phy1_write_mask[PCIE_PHY_REG_COUNT] = {
    0x1FFF, 0x1FFF, 0x1FFF
};

/* Write-read test patterns for DBI DSP and SII registers */
static const unsigned int chk_val[PCIE_CHK_VAL_COUNT] = {
    0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000
};

/* Write-read test patterns for PHY registers */
static const unsigned int chk_val_phy[PCIE_CHK_VAL_PHY_COUNT] = {
    0x7baf, 0x1, 0x003b
};

/*
 * Function: pcie_read_phy_reg
 * Description: Reads a PHY register with 16-bit extraction based on address alignment.
 * Parameters:
 *   addr - PHY register address.
 * Returns:
 *   16-bit extracted value.
 */
static unsigned int pcie_read_phy_reg(unsigned long addr)
{
    unsigned int data_rd;

    data_rd = read_reg(addr);
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

    LOGT("chk_rst_val: checking reset default values");

    /* Step 2: Check PCIE0 DBI DSP register reset defaults */
    for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            LOGE("PCIE0 DBI DSP reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.err1++;
        } else {
            LOGT("PCIE0 DBI DSP reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Step 3: Check PCIE1 DBI DSP register reset defaults */
    for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        if (data_rd != ctl_default[i]) {
            LOGE("PCIE1 DBI DSP reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, ctl_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 DBI DSP reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Step 4: Check PCIE0 SII register reset defaults */
    for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
        data_rd = read_reg(sii0_addr[i]);
        if (data_rd != sii_default[i]) {
            LOGE("PCIE0 SII reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE0 SII reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Step 5: Check PCIE1 SII register reset defaults */
    for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
        data_rd = read_reg(sii1_addr[i]);
        if (data_rd != sii_default[i]) {
            LOGE("PCIE1 SII reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, sii_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 SII reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Step 6-7: Release PHY from reset */
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PCIE_PHY_RST_RELEASE_VAL);
    LOGT("PCIE0 PHY reset released");
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PCIE_PHY_RST_RELEASE_VAL);
    LOGT("PCIE1 PHY reset released");

    /* Step 8: Check PCIE0 PHY register reset defaults */
    for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
        data_rd = pcie_read_phy_reg(phy0_addr[i]);
        if (data_rd != phy0_default[i]) {
            LOGE("PCIE0 PHY reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, phy0_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE0 PHY reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    /* Step 9: Check PCIE1 PHY register reset defaults */
    for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
        data_rd = pcie_read_phy_reg(phy1_addr[i]);
        if (data_rd != phy1_default[i]) {
            LOGE("PCIE1 PHY reg[%u] rst default mismatch: exp=0x%x got=0x%x",
                 i, phy1_default[i], data_rd);
            g_ctx.err2++;
        } else {
            LOGT("PCIE1 PHY reg[%u] rst default OK: 0x%x", i, data_rd);
        }
    }

    LOGT("chk_rst_val complete: err1=%u err2=%u", g_ctx.err1, g_ctx.err2);
}

/*
 * Function: chk_rd_wr
 * Description: Performs write-read integrity checks on all DBI DSP, SII, and PHY registers
 *              using multiple test patterns.
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
    unsigned int expected;

    LOGT("chk_rd_wr: checking write-read integrity");

    /* Step 11: Loop over three test patterns */
    for (j = 0U; j < PCIE_CHK_VAL_PHY_COUNT; j++) {

        LOGT("Pattern j=%u: chk_val=0x%x chk_val_phy=0x%x", j, chk_val[j], chk_val_phy[j]);

        /* Step 12: Write pattern to PCIE0 DBI DSP registers */
        for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
            write_reg(rc0_ctl_addr[i], chk_val[j]);
        }
        LOGT("PCIE0 DBI DSP registers written with 0x%x", chk_val[j]);

        /* Step 13: Write pattern to PCIE1 DBI DSP registers */
        for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
            write_reg(rc1_ctl_addr[i], chk_val[j]);
        }
        LOGT("PCIE1 DBI DSP registers written with 0x%x", chk_val[j]);

        /* Step 14: Write masked pattern to PCIE0 SII registers */
        for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
            write_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]);
        }
        LOGT("PCIE0 SII registers written with masked pattern");

        /* Step 15: Write masked pattern to PCIE1 SII registers */
        for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
            write_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]);
        }
        LOGT("PCIE1 SII registers written with masked pattern");

        /* Step 16-17: Re-apply PHY reset release */
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PCIE_PHY_RST_RELEASE_VAL);
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PCIE_PHY_RST_RELEASE_VAL);
        LOGT("PHY reset re-released for pattern j=%u", j);

        /* Step 18: Write masked PHY pattern to PCIE0 PHY registers */
        for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
            write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]);
        }
        LOGT("PCIE0 PHY registers written with masked PHY pattern");

        /* Step 19: Write masked PHY pattern to PCIE1 PHY registers */
        for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
            write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]);
        }
        LOGT("PCIE1 PHY registers written with masked PHY pattern");

        /* Step 20: Read back and verify PCIE0 DBI DSP registers */
        for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                LOGE("PCIE0 DBI DSP reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, chk_val[j], data_rd);
                g_ctx.err1++;
            } else {
                LOGT("PCIE0 DBI DSP reg[%u] wr/rd OK: 0x%x", i, data_rd);
            }
        }

        /* Step 21: Read back and verify PCIE1 DBI DSP registers */
        for (i = 0U; i < PCIE_DBI_DSP_REG_COUNT; i++) {
            data_rd = read_reg(rc1_ctl_addr[i]);
            if (data_rd != chk_val[j]) {
                LOGE("PCIE1 DBI DSP reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, chk_val[j], data_rd);
                g_ctx.err1++;
            } else {
                LOGT("PCIE1 DBI DSP reg[%u] wr/rd OK: 0x%x", i, data_rd);
            }
        }

        /* Step 22: Read back and verify PCIE0 SII registers */
        for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
            data_rd = read_reg(sii0_addr[i]);
            expected = chk_val[j] & sii0_write_mask[i];
            if (data_rd != expected) {
                LOGE("PCIE0 SII reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, expected, data_rd);
                g_ctx.err1++;
            } else {
                LOGT("PCIE0 SII reg[%u] wr/rd OK: 0x%x", i, data_rd);
            }
        }

        /* Step 23: Read back and verify PCIE1 SII registers */
        for (i = 0U; i < PCIE_SII_REG_COUNT; i++) {
            data_rd = read_reg(sii1_addr[i]);
            expected = chk_val[j] & sii1_write_mask[i];
            if (data_rd != expected) {
                LOGE("PCIE1 SII reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, expected, data_rd);
                g_ctx.err1++;
            } else {
                LOGT("PCIE1 SII reg[%u] wr/rd OK: 0x%x", i, data_rd);
            }
        }

        /* Step 24: Read back and verify PCIE0 PHY registers */
        for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
            data_rd = pcie_read_phy_reg(phy0_addr[i]);
            expected = chk_val_phy[j] & PCIE_PHY_13BIT_MASK;
            if ((data_rd & phy0_write_mask[i]) != expected) {
                LOGE("PCIE0 PHY reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, expected, data_rd & phy0_write_mask[i]);
                g_ctx.err1++;
            } else {
                LOGT("PCIE0 PHY reg[%u] wr/rd OK: 0x%x", i, data_rd & phy0_write_mask[i]);
            }
        }

        /* Step 25: Read back and verify PCIE1 PHY registers */
        for (i = 0U; i < PCIE_PHY_REG_COUNT; i++) {
            data_rd = pcie_read_phy_reg(phy1_addr[i]);
            expected = chk_val_phy[j] & PCIE_PHY_13BIT_MASK;
            if ((data_rd & phy1_write_mask[i]) != expected) {
                LOGE("PCIE1 PHY reg[%u] wr/rd mismatch: exp=0x%x got=0x%x",
                     i, expected, data_rd & phy1_write_mask[i]);
                g_ctx.err1++;
            } else {
                LOGT("PCIE1 PHY reg[%u] wr/rd OK: 0x%x", i, data_rd & phy1_write_mask[i]);
            }
        }

        LOGT("Pattern j=%u complete: err1=%u err2=%u", j, g_ctx.err1, g_ctx.err2);
    }

    LOGT("chk_rd_wr complete: err1=%u err2=%u", g_ctx.err1, g_ctx.err2);
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

    /* Step 1: Call chk_rst_val to verify reset defaults */
    chk_rst_val();

    /* Step 10: Call chk_rd_wr to verify write-read integrity */
    chk_rd_wr();

    // MANUAL_REVIEW: DV finish(err2 || err1) was present in the source flow (step 26).
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.

    /* Final status */
    if ((g_ctx.err1 > 0U) || (g_ctx.err2 > 0U)) {
        out->status = -1;
    }

    LOGT("Run complete: %s err1=%u err2=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.err1, g_ctx.err2);

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

    LOGT("PCIe register write/read test teardown: err1=%u err2=%u",
         g_ctx.err1, g_ctx.err2);

    return ((g_ctx.err1 == 0U) && (g_ctx.err2 == 0U)) ? 0 : -1;
}
