// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/* Global variables */
unsigned int data_rd;
unsigned int test_err;
int err1;
int err2;

unsigned int rc0_ctl_addr[5] = {
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

unsigned int rc1_ctl_addr[5] = {
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

unsigned int ctl_default[5] = {0x0, 0x0, 0x0, 0x0, 0x0};

unsigned int sii0_addr[3] = {
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    mizar_PCIE0_SII_PHY_CONTROL_23
};

unsigned int sii1_addr[3] = {
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    mizar_PCIE1_SII_PHY_CONTROL_23
};

unsigned int sii_default[3] = {0x0, 0x0, 0x0};

unsigned int sii0_write_mask[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xF000F};
unsigned int sii1_write_mask[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xF000F};

unsigned int phy0_addr[3] = {0xE68860B8, 0xE68862B8, 0xE68864B8};
unsigned int phy1_addr[3] = {0xE68A60B8, 0xE68A62B8, 0xE68A64B8};

unsigned int phy0_default[3] = {0x0, 0x0, 0x0};
unsigned int phy1_default[3] = {0x0, 0x0, 0x0};

unsigned int phy0_write_mask[3] = {0x1FFF, 0x1FFF, 0x1FFF};
unsigned int phy1_write_mask[3] = {0x1FFF, 0x1FFF, 0x1FFF};

int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};
int chk_val_phy[3] = {0x7baf, 0x1, 0x003b};

/*
 * Function: read_phy_reg_16bit
 * Description: Read PHY register with 16-bit extraction based on address alignment.
 * Parameters:
 *   addr - PHY register address.
 * Returns:
 *   16-bit extracted value.
 */
unsigned int read_phy_reg_16bit(unsigned int addr)
{
    unsigned int data_rd;
    unsigned int aligned_addr = addr & ~0x3;

    data_rd = read_reg(aligned_addr);

    if (addr & 0x2) {
        /* Upper 16 bits */
        data_rd = (data_rd >> 16) & 0xFFFF;
    } else {
        /* Lower 16 bits */
        data_rd = data_rd & 0xFFFF;
    }
    return data_rd;
}

/*
 * Function: write_phy_reg_16bit
 * Description: Write PHY register with 16-bit insertion based on address alignment.
 * Parameters:
 *   addr - PHY register address.
 *   val  - 16-bit value to write.
 */
void write_phy_reg_16bit(unsigned int addr, unsigned int val)
{
    unsigned int data_rd;
    unsigned int aligned_addr = addr & ~0x3;

    data_rd = read_reg(aligned_addr);

    if (addr & 0x2) {
        /* Upper 16 bits */
        data_rd = (data_rd & 0x0000FFFF) | ((val & 0xFFFF) << 16);
    } else {
        /* Lower 16 bits */
        data_rd = (data_rd & 0xFFFF0000) | (val & 0xFFFF);
    }
    write_reg(aligned_addr, data_rd);
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
    LOGI("[Test Init] PCIE reg wr rd test: %s\n", cfg->test_name);

    return 0;
}

/*
 * Function: chk_rst_val
 * Description: Check reset default values for all register domains.
 */
void chk_rst_val(void)
{
    unsigned int data_rd;
    int i;

    /* Step 2: Check DBI DSP registers on PCIE0 (rc0_ctl_addr) */
    LOGI("Step 2: Check reset values - PCIE0 DBI DSP registers\n");
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        LOGI("rc0_ctl_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, ctl_default[i]);
        if (data_rd != ctl_default[i]) {
            err1++;
            LOGI("FAIL: PCIE0 DBI DSP register reset value mismatch\n");
        }
    }

    /* Step 3: Check DBI DSP registers on PCIE1 (rc1_ctl_addr) */
    LOGI("Step 3: Check reset values - PCIE1 DBI DSP registers\n");
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        LOGI("rc1_ctl_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, ctl_default[i]);
        if (data_rd != ctl_default[i]) {
            err2++;
            LOGI("FAIL: PCIE1 DBI DSP register reset value mismatch\n");
        }
    }

    /* Step 4: Check SII registers on port 0 */
    LOGI("Step 4: Check reset values - PCIE0 SII registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
        LOGI("sii0_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, sii_default[i]);
        if (data_rd != sii_default[i]) {
            err2++;
            LOGI("FAIL: PCIE0 SII register reset value mismatch\n");
        }
    }

    /* Step 5: Check SII registers on port 1 */
    LOGI("Step 5: Check reset values - PCIE1 SII registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
        LOGI("sii1_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, sii_default[i]);
        if (data_rd != sii_default[i]) {
            err2++;
            LOGI("FAIL: PCIE1 SII register reset value mismatch\n");
        }
    }

    /* Step 6: Bring PHY out of reset on port 0 */
    LOGI("Step 6: Write PHY reset control on PCIE0\n");
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);

    /* Step 7: Bring PHY out of reset on port 1 */
    LOGI("Step 7: Write PHY reset control on PCIE1\n");
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);

    /* Step 8: Check PHY registers on port 0 */
    LOGI("Step 8: Check reset values - PCIE0 PHY registers (16-bit)\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_phy_reg_16bit(phy0_addr[i]);
        LOGI("phy0_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, phy0_default[i]);
        if (data_rd != phy0_default[i]) {
            err2++;
            LOGI("FAIL: PCIE0 PHY register reset value mismatch\n");
        }
    }

    /* Step 9: Check PHY registers on port 1 */
    LOGI("Step 9: Check reset values - PCIE1 PHY registers (16-bit)\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_phy_reg_16bit(phy1_addr[i]);
        LOGI("phy1_addr[%d] = 0x%08x (expected 0x%08x)\n", i, data_rd, phy1_default[i]);
        if (data_rd != phy1_default[i]) {
            err2++;
            LOGI("FAIL: PCIE1 PHY register reset value mismatch\n");
        }
    }
}

/*
 * Function: chk_rd_wr
 * Description: Write-read verification for all register domains.
 */
void chk_rd_wr(void)
{
    unsigned int data_rd;
    unsigned int data_wr;
    unsigned int expected;
    int i, j;

    /* Iterate through check value patterns */
    for (j = 0; j < 3; j++) {
        /* Step 10: Write-read DBI DSP registers on PCIE0 */
        LOGI("Step 10: Write-read check - PCIE0 DBI DSP registers with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 5; i++) {
            write_reg(rc0_ctl_addr[i], (unsigned int)chk_val[j]);
            data_rd = read_reg(rc0_ctl_addr[i]);
            LOGI("rc0_ctl_addr[%d] wrote=0x%08x read=0x%08x\n", i, (unsigned int)chk_val[j], data_rd);
            if (data_rd != (unsigned int)chk_val[j]) {
                err1++;
                LOGI("FAIL: PCIE0 DBI DSP write-read mismatch\n");
            }
        }

        /* Step 11: Write-read DBI DSP registers on PCIE1 */
        LOGI("Step 11: Write-read check - PCIE1 DBI DSP registers with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 5; i++) {
            write_reg(rc1_ctl_addr[i], (unsigned int)chk_val[j]);
            data_rd = read_reg(rc1_ctl_addr[i]);
            LOGI("rc1_ctl_addr[%d] wrote=0x%08x read=0x%08x\n", i, (unsigned int)chk_val[j], data_rd);
            if (data_rd != (unsigned int)chk_val[j]) {
                err2++;
                LOGI("FAIL: PCIE1 DBI DSP write-read mismatch\n");
            }
        }

        /* Step 12: Write-read SII registers on port 0 (with write mask) */
        LOGI("Step 12: Write-read check - PCIE0 SII registers with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val[j];
            write_reg(sii0_addr[i], data_wr);
            data_rd = read_reg(sii0_addr[i]);
            expected = data_wr & sii0_write_mask[i];
            LOGI("sii0_addr[%d] wrote=0x%08x read=0x%08x expected=0x%08x\n", i, data_wr, data_rd, expected);
            if (data_rd != expected) {
                err2++;
                LOGI("FAIL: PCIE0 SII write-read mismatch\n");
            }
        }

        /* Step 13: Write-read SII registers on port 1 (with write mask) */
        LOGI("Step 13: Write-read check - PCIE1 SII registers with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val[j];
            write_reg(sii1_addr[i], data_wr);
            data_rd = read_reg(sii1_addr[i]);
            expected = data_wr & sii1_write_mask[i];
            LOGI("sii1_addr[%d] wrote=0x%08x read=0x%08x expected=0x%08x\n", i, data_wr, data_rd, expected);
            if (data_rd != expected) {
                err2++;
                LOGI("FAIL: PCIE1 SII write-read mismatch\n");
            }
        }

        /* Step 14: Write-read PHY registers on port 0 (16-bit, with write mask) */
        LOGI("Step 14: Write-read check - PCIE0 PHY registers with pattern 0x%04x\n", (unsigned int)chk_val_phy[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val_phy[j];
            write_phy_reg_16bit(phy0_addr[i], data_wr);
            data_rd = read_phy_reg_16bit(phy0_addr[i]);
            expected = data_wr & phy0_write_mask[i];
            LOGI("phy0_addr[%d] wrote=0x%04x read=0x%04x expected=0x%04x\n", i, data_wr, data_rd, expected);
            if (data_rd != expected) {
                err2++;
                LOGI("FAIL: PCIE0 PHY write-read mismatch\n");
            }
        }

        /* Step 15: Write-read PHY registers on port 1 (16-bit, with write mask) */
        LOGI("Step 15: Write-read check - PCIE1 PHY registers with pattern 0x%04x\n", (unsigned int)chk_val_phy[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val_phy[j];
            write_phy_reg_16bit(phy1_addr[i], data_wr);
            data_rd = read_phy_reg_16bit(phy1_addr[i]);
            expected = data_wr & phy1_write_mask[i];
            LOGI("phy1_addr[%d] wrote=0x%04x read=0x%04x expected=0x%04x\n", i, data_wr, data_rd, expected);
            if (data_rd != expected) {
                err2++;
                LOGI("FAIL: PCIE1 PHY write-read mismatch\n");
            }
        }
    }
}

/*
 * Function: pcie_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_reg_wr_rd_test including
 *              reset value verification and write-read verification for DBI DSP,
 *              SII, and PHY registers across both PCIE0 and PCIE1 ports.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;
    LOGI("[Test Run] PCIE reg wr rd test: %s\n", cfg->test_name);
    test_err = 0;
    err1 = 0;
    err2 = 0;

    /* Step 1: Reset value check phase */
    LOGI("Step 1: Begin reset value check phase\n");
    chk_rst_val();

    if (err1 == 0 && err2 == 0) {
        LOGI("PASS: All reset value checks passed\n");
    } else {
        LOGI("WARN: Reset value errors: err1=%d err2=%d\n", err1, err2);
    }

    /* Write-read verification phase */
    LOGI("Step 10: Begin write-read verification phase\n");
    chk_rd_wr();

    if (err1 == 0 && err2 == 0) {
        LOGI("PASS: All write-read checks passed\n");
    } else {
        LOGI("WARN: Write-read errors: err1=%d err2=%d\n", err1, err2);
    }

    /* Final result */
    LOGI("Final error count: err1=%d err2=%d\n", err1, err2);
    if (err1 == 0 && err2 == 0) {
        LOGI("PASS: pcie_reg_wr_rd_test PASSED\n");
    } else {
        LOGI("FAIL: pcie_reg_wr_rd_test FAILED\n");
    }

    finish(err2 || err1);

    return out->status = test_err;
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
    LOGI("[TEARDOWN] PCIE reg wr rd test: %s\n", cfg->test_name);

    return 0;
}
