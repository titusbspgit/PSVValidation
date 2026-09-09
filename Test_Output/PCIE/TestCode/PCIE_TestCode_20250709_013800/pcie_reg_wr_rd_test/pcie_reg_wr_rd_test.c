// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

typedef struct {
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} pcie_reg_wr_rd_test_ctx_t;

static pcie_reg_wr_rd_test_ctx_t g_ctx;

static unsigned int phy_extract_16bit(unsigned int addr, unsigned int data_rd)
{
    if ((addr % 4U) != 0U) {
        return (data_rd >> 16);
    } else {
        return (data_rd & 0x0000FFFFU);
    }
}

static void chk_rst_val(void)
{
    unsigned int i;
    unsigned int data_rd;
    unsigned int extracted;
    for (i = 0U; i < RC_CTL_COUNT; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
    for (i = 0U; i < RC_CTL_COUNT; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != ctl_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
    for (i = 0U; i < SII_COUNT; i++) {
        data_rd = read_reg(sii0_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
    for (i = 0U; i < SII_COUNT; i++) {
        data_rd = read_reg(sii1_addr[i]);
        g_ctx.checks_total++;
        if (data_rd != sii_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
    for (i = 0U; i < PHY_COUNT; i++) {
        data_rd = read_reg(phy0_addr[i]);
        extracted = phy_extract_16bit(phy0_addr[i], data_rd);
        g_ctx.checks_total++;
        if (extracted != phy0_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
    for (i = 0U; i < PHY_COUNT; i++) {
        data_rd = read_reg(phy1_addr[i]);
        extracted = phy_extract_16bit(phy1_addr[i], data_rd);
        g_ctx.checks_total++;
        if (extracted != phy1_default[i]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
    }
}

static void chk_rd_wr(void)
{
    unsigned int i, j;
    unsigned int data_rd, extracted, expected;
    for (j = 0U; j < CHK_VAL_WR_RD_COUNT; j++) {
        for (i = 0U; i < RC_CTL_COUNT; i++) { write_reg(rc0_ctl_addr[i], chk_val[j]); }
        for (i = 0U; i < RC_CTL_COUNT; i++) { write_reg(rc1_ctl_addr[i], chk_val[j]); }
        for (i = 0U; i < SII_COUNT; i++) { write_reg(sii0_addr[i], chk_val[j] & sii0_write_mask[i]); }
        for (i = 0U; i < SII_COUNT; i++) { write_reg(sii1_addr[i], chk_val[j] & sii1_write_mask[i]); }
        write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
        write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, PHY_RST_RELEASE_VAL);
        for (i = 0U; i < PHY_COUNT; i++) { write_reg(phy0_addr[i], chk_val_phy[j] & phy0_write_mask[i]); }
        for (i = 0U; i < PHY_COUNT; i++) { write_reg(phy1_addr[i], chk_val_phy[j] & phy1_write_mask[i]); }
        for (i = 0U; i < RC_CTL_COUNT; i++) {
            data_rd = read_reg(rc0_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
        for (i = 0U; i < RC_CTL_COUNT; i++) {
            data_rd = read_reg(rc1_ctl_addr[i]);
            g_ctx.checks_total++;
            if (data_rd != chk_val[j]) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
        for (i = 0U; i < SII_COUNT; i++) {
            data_rd = read_reg(sii0_addr[i]);
            expected = chk_val[j] & sii0_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
        for (i = 0U; i < SII_COUNT; i++) {
            data_rd = read_reg(sii1_addr[i]);
            expected = chk_val[j] & sii1_write_mask[i];
            g_ctx.checks_total++;
            if (data_rd != expected) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
        for (i = 0U; i < PHY_COUNT; i++) {
            data_rd = read_reg(phy0_addr[i]);
            extracted = phy_extract_16bit(phy0_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((extracted & phy0_write_mask[i]) != expected) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
        for (i = 0U; i < PHY_COUNT; i++) {
            data_rd = read_reg(phy1_addr[i]);
            extracted = phy_extract_16bit(phy1_addr[i], data_rd);
            expected = chk_val_phy[j] & PHY_CHK_MASK;
            g_ctx.checks_total++;
            if ((extracted & phy1_write_mask[i]) != expected) { g_ctx.errors++; } else { g_ctx.checks_passed++; }
        }
    }
}

int pcie_reg_wr_rd_test_init(const TestsItem *cfg)
{
    (void)cfg;
    g_ctx = (pcie_reg_wr_rd_test_ctx_t){0};
    return 0;
}

int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    if (out == 0) { return -1; }
    out->status = 0;
    chk_rst_val();
    chk_rd_wr();
    g_ctx.checks_failed = g_ctx.errors;
    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    return out->status;
}

int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;
    return g_ctx.errors == 0U ? 0 : -1;
}
