// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_reg_wr_rd_test.h"
#include "test_define.inc"

/* Global variables */
unsigned int data_rd;
unsigned int test_err;
int err1;
int err2;
unsigned int i;
unsigned int j;

/*
 * Function: read_phy_reg_16bit
 * Description: Reads a PHY register with 16-bit extraction based on address alignment.
 * Parameters:
 *   addr - PHY register address.
 * Returns:
 *   16-bit extracted register value.
 */
static unsigned int read_phy_reg_16bit(unsigned int addr)
{
    unsigned int rd_data;
    unsigned int aligned_addr = addr & ~0x3;

    rd_data = read_reg(aligned_addr);

    if (addr & 0x2) {
        /* Upper 16 bits */
        rd_data = (rd_data >> 16) & 0xFFFF;
    } else {
        /* Lower 16 bits */
        rd_data = rd_data & 0xFFFF;
    }
    return rd_data;
}

/*
 * Function: write_phy_reg_16bit
 * Description: Writes a PHY register with 16-bit insertion based on address alignment.
 * Parameters:
 *   addr - PHY register address.
 *   val  - 16-bit value to write.
 * Returns:
 *   None.
 */
static void write_phy_reg_16bit(unsigned int addr, unsigned int val)
{
    unsigned int rd_data;
    unsigned int aligned_addr = addr & ~0x3;

    rd_data = read_reg(aligned_addr);

    if (addr & 0x2) {
        /* Upper 16 bits */
        rd_data = (rd_data & 0x0000FFFF) | ((val & 0xFFFF) << 16);
    } else {
        /* Lower 16 bits */
        rd_data = (rd_data & 0xFFFF0000) | (val & 0xFFFF);
    }
    write_reg(aligned_addr, rd_data);
}

/*
 * Function: chk_rst_val
 * Description: Checks reset default values for DBI DSP, SII, and PHY register domains
 *              on both PCIE0 and PCIE1 ports.
 * Parameters:
 *   None.
 * Returns:
 *   None. Increments err1/err2 on mismatch.
 */
static void chk_rst_val(void)
{
    LOGI("[chk_rst_val] Begin reset value check phase\n");

    /* Step 2: Check DBI DSP registers on PCIE0 (rc0_ctl_addr) */
    LOGI("Step 2: Check reset values - PCIE0 DBI DSP registers\n");
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc0_ctl_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("rc0_ctl_addr[%d] addr=0x%08x read=0x%08x expected=0x%08x\n", i, rc0_ctl_addr[i], data_rd, ctl_default[i]);
        #endif
        if (data_rd != ctl_default[i]) {
            LOGI("ERROR: rc0_ctl_addr[%d] mismatch read=0x%08x expected=0x%08x\n", i, data_rd, ctl_default[i]);
            err1++;
        }
    }

    /* Step 3: Check DBI DSP registers on PCIE1 (rc1_ctl_addr) */
    LOGI("Step 3: Check reset values - PCIE1 DBI DSP registers\n");
    for (i = 0; i < 5; i++) {
        data_rd = read_reg(rc1_ctl_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("rc1_ctl_addr[%d] addr=0x%08x read=0x%08x expected=0x%08x\n", i, rc1_ctl_addr[i], data_rd, ctl_default[i]);
        #endif
        if (data_rd != ctl_default[i]) {
            LOGI("ERROR: rc1_ctl_addr[%d] mismatch read=0x%08x expected=0x%08x\n", i, data_rd, ctl_default[i]);
            err2++;
        }
    }

    /* Step 4: Check SII registers on PCIE0 (sii0_addr) */
    LOGI("Step 4: Check reset values - PCIE0 SII registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii0_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("sii0_addr[%d] addr=0x%08x read=0x%08x expected=0x%08x\n", i, sii0_addr[i], data_rd, sii_default[i]);
        #endif
        if (data_rd != sii_default[i]) {
            LOGI("ERROR: sii0_addr[%d] mismatch read=0x%08x expected=0x%08x\n", i, data_rd, sii_default[i]);
            err2++;
        }
    }

    /* Step 5: Check SII registers on PCIE1 (sii1_addr) */
    LOGI("Step 5: Check reset values - PCIE1 SII registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_reg(sii1_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("sii1_addr[%d] addr=0x%08x read=0x%08x expected=0x%08x\n", i, sii1_addr[i], data_rd, sii_default[i]);
        #endif
        if (data_rd != sii_default[i]) {
            LOGI("ERROR: sii1_addr[%d] mismatch read=0x%08x expected=0x%08x\n", i, data_rd, sii_default[i]);
            err2++;
        }
    }

    /* Step 6: Bring PHY out of reset on PCIE0 */
    LOGI("Step 6: Write PHY reset control on PCIE0\n");
    write_reg(mizar_PCIE0_SII_PHY_RST_CONTROL, 0x01203000);

    /* Step 7: Bring PHY out of reset on PCIE1 */
    LOGI("Step 7: Write PHY reset control on PCIE1\n");
    write_reg(mizar_PCIE1_SII_PHY_RST_CONTROL, 0x01203000);

    /* Step 8: Check PHY registers on PCIE0 (phy0_addr, 16-bit extraction) */
    LOGI("Step 8: Check reset values - PCIE0 PHY registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_phy_reg_16bit(phy0_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("phy0_addr[%d] addr=0x%08x read=0x%04x expected=0x%04x\n", i, phy0_addr[i], data_rd, phy0_default[i]);
        #endif
        if (data_rd != phy0_default[i]) {
            LOGI("ERROR: phy0_addr[%d] mismatch read=0x%04x expected=0x%04x\n", i, data_rd, phy0_default[i]);
            err2++;
        }
    }

    /* Step 9: Check PHY registers on PCIE1 (phy1_addr, 16-bit extraction) */
    LOGI("Step 9: Check reset values - PCIE1 PHY registers\n");
    for (i = 0; i < 3; i++) {
        data_rd = read_phy_reg_16bit(phy1_addr[i]);
        #ifdef DEBUG_DISPLAY
            LOGI("phy1_addr[%d] addr=0x%08x read=0x%04x expected=0x%04x\n", i, phy1_addr[i], data_rd, phy1_default[i]);
        #endif
        if (data_rd != phy1_default[i]) {
            LOGI("ERROR: phy1_addr[%d] mismatch read=0x%04x expected=0x%04x\n", i, data_rd, phy1_default[i]);
            err2++;
        }
    }

    LOGI("[chk_rst_val] Reset value check phase complete: err1=%d err2=%d\n", err1, err2);
}

/*
 * Function: chk_rd_wr
 * Description: Write-read verification for DBI DSP, SII, and PHY register domains
 *              using multiple data patterns across both PCIE0 and PCIE1 ports.
 * Parameters:
 *   None.
 * Returns:
 *   None. Increments err1/err2 on mismatch.
 */
static void chk_rd_wr(void)
{
    unsigned int data_wr;
    unsigned int expected;

    LOGI("[chk_rd_wr] Begin write-read verification phase\n");

    for (j = 0; j < 3; j++) {

        /* Step 10: Write-read DBI DSP registers on PCIE0 */
        LOGI("Step 10: Write-read PCIE0 DBI DSP regs with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 5; i++) {
            write_reg(rc0_ctl_addr[i], (unsigned int)chk_val[j]);
            data_rd = read_reg(rc0_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                LOGI("ERROR: rc0_ctl_addr[%d] wr-rd mismatch read=0x%08x expected=0x%08x\n", i, data_rd, (unsigned int)chk_val[j]);
                err1++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: rc0_ctl_addr[%d] read=0x%08x expected=0x%08x\n", i, data_rd, (unsigned int)chk_val[j]);
            #endif
        }

        /* Step 11: Write-read DBI DSP registers on PCIE1 */
        LOGI("Step 11: Write-read PCIE1 DBI DSP regs with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 5; i++) {
            write_reg(rc1_ctl_addr[i], (unsigned int)chk_val[j]);
            data_rd = read_reg(rc1_ctl_addr[i]);
            if (data_rd != (unsigned int)chk_val[j]) {
                LOGI("ERROR: rc1_ctl_addr[%d] wr-rd mismatch read=0x%08x expected=0x%08x\n", i, data_rd, (unsigned int)chk_val[j]);
                err2++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: rc1_ctl_addr[%d] read=0x%08x expected=0x%08x\n", i, data_rd, (unsigned int)chk_val[j]);
            #endif
        }

        /* Step 12: Write-read SII registers on PCIE0 (with write mask) */
        LOGI("Step 12: Write-read PCIE0 SII regs with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val[j];
            write_reg(sii0_addr[i], data_wr);
            data_rd = read_reg(sii0_addr[i]);
            expected = data_wr & sii0_write_mask[i];
            if (data_rd != expected) {
                LOGI("ERROR: sii0_addr[%d] wr-rd mismatch read=0x%08x expected=0x%08x\n", i, data_rd, expected);
                err2++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: sii0_addr[%d] read=0x%08x expected=0x%08x\n", i, data_rd, expected);
            #endif
        }

        /* Step 13: Write-read SII registers on PCIE1 (with write mask) */
        LOGI("Step 13: Write-read PCIE1 SII regs with pattern 0x%08x\n", (unsigned int)chk_val[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val[j];
            write_reg(sii1_addr[i], data_wr);
            data_rd = read_reg(sii1_addr[i]);
            expected = data_wr & sii1_write_mask[i];
            if (data_rd != expected) {
                LOGI("ERROR: sii1_addr[%d] wr-rd mismatch read=0x%08x expected=0x%08x\n", i, data_rd, expected);
                err2++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: sii1_addr[%d] read=0x%08x expected=0x%08x\n", i, data_rd, expected);
            #endif
        }

        /* Step 14: Write-read PHY registers on PCIE0 (16-bit, with write mask) */
        LOGI("Step 14: Write-read PCIE0 PHY regs with pattern 0x%04x\n", (unsigned int)chk_val_phy[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val_phy[j];
            write_phy_reg_16bit(phy0_addr[i], data_wr);
            data_rd = read_phy_reg_16bit(phy0_addr[i]);
            expected = data_wr & phy0_write_mask[i];
            if (data_rd != expected) {
                LOGI("ERROR: phy0_addr[%d] wr-rd mismatch read=0x%04x expected=0x%04x\n", i, data_rd, expected);
                err2++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: phy0_addr[%d] read=0x%04x expected=0x%04x\n", i, data_rd, expected);
            #endif
        }

        /* Step 15: Write-read PHY registers on PCIE1 (16-bit, with write mask) */
        LOGI("Step 15: Write-read PCIE1 PHY regs with pattern 0x%04x\n", (unsigned int)chk_val_phy[j]);
        for (i = 0; i < 3; i++) {
            data_wr = (unsigned int)chk_val_phy[j];
            write_phy_reg_16bit(phy1_addr[i], data_wr);
            data_rd = read_phy_reg_16bit(phy1_addr[i]);
            expected = data_wr & phy1_write_mask[i];
            if (data_rd != expected) {
                LOGI("ERROR: phy1_addr[%d] wr-rd mismatch read=0x%04x expected=0x%04x\n", i, data_rd, expected);
                err2++;
            }
            #ifdef DEBUG_DISPLAY
                LOGI("SUCCESS: phy1_addr[%d] read=0x%04x expected=0x%04x\n", i, data_rd, expected);
            #endif
        }
    }

    LOGI("[chk_rd_wr] Write-read verification phase complete: err1=%d err2=%d\n", err1, err2);
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
 * Function: pcie_reg_wr_rd_test_run
 * Description: Executes the main testcase flow for pcie_reg_wr_rd_test including reset value
 *              verification and write-read verification for DBI DSP, SII, and PHY registers.
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

    /* Phase 1: Reset value check */
    LOGI("Phase 1: Begin reset value check\n");
    chk_rst_val();

    if (err1 == 0 && err2 == 0) {
        LOGI("SUCCESS: All reset value checks passed\n");
    } else {
        LOGI("WARNING: Reset value errors: err1=%d err2=%d\n", err1, err2);
    }

    /* Phase 2: Write-read verification */
    LOGI("Phase 2: Begin write-read verification\n");
    chk_rd_wr();

    if (err1 == 0 && err2 == 0) {
        LOGI("SUCCESS: All write-read checks passed\n");
    } else {
        LOGI("WARNING: Write-read errors: err1=%d err2=%d\n", err1, err2);
    }

    /* Final result */
    LOGI("Final error count: err1=%d err2=%d\n", err1, err2);
    if (err1 == 0 && err2 == 0) {
        LOGI("RESULT: pcie_reg_wr_rd_test PASSED\n");
    } else {
        LOGI("RESULT: pcie_reg_wr_rd_test FAILED\n");
    }

    finish(err2 || err1);

    return out->status = (err1 || err2);
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
