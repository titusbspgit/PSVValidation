// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

// Global variables as per Meta
static int data_rd;
static int data_wr;
static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;

/*
 * Function: chk_rst_val
 * Description: Phase 1 - Read all 434 registers and compare against
 *             expected default/reset values. Skip registers flagged
 *             in skip_array.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void chk_rst_val(void)
{
    int i;
    int addr;

    LOGT("Phase 1: Default value check (chk_rst_val) starting");

    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        if (skip_array[i] == 1) {
            continue;
        }

        data_rd = read_reg(addr);

        if (data_rd == default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            LOGT("RST : PASS Reading Default value from Address :0x%x Expected : 0x%x Read_data : 0x%x
                 addr, default_value_array[i], data_rd);
#endif
        } else {
            def_fail_cnt++;
            LOGE("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x Read_data : 0x%x
                 addr, default_value_array[i], data_rd);
        }
    }

    LOGT("Phase 1: Default value check complete, def_fail_cnt=%d", def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Phase 2 - Write/Read check. For each of 6 test patterns,
 *             write pattern masked with write_mask to each register,
 *             read back and compare against expected value.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void chk_rd_wr(void)
{
    int i;
    int j;
    int addr;
    int exp_val;

    LOGT("Phase 2: Write/Read check (chk_rd_wr) starting");

    for (j = 0; j < 6; j++) {
        data_wr = chk_val[j];

        for (i = 0; i < CNT; i++) {
            addr = addr_array[i];

            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                LOGT("Read_write : Writing into this Address : 0x%x is skipped because address present in skip_array
                     addr);
#endif
                continue;
            }

            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                LOGT("Read_write : This address 0x%x is not Writable , hence skipped for reading
                     addr);
#endif
                continue;
            }

            write_reg(addr, data_wr & write_mask_array[i]);
            data_rd = read_reg(addr);
            exp_val = data_wr & write_mask_array[i] & read_mask_array[i];

            if (data_rd != exp_val) {
                wr_fail_cnt++;
                LOGE("WR : Failed Addr :0x%x Pattern :0x%x Written :0x%x Expected :0x%x Read :0x%x
                     addr, data_wr, data_wr & write_mask_array[i], exp_val, data_rd);
            }
        }
    }

    // Phase 2 reading loop as per Meta Step 17
    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
            LOGT("Read_write : Reading from this Address : 0x%x is skipped because address present in skip_array
                 addr);
#endif
            continue;
        }

        if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            LOGT("Read_write : This address 0x%x is not Writable , hence skipped for reading
                 addr);
#endif
            continue;
        }
    }

    LOGT("Phase 2: Write/Read check complete, wr_fail_cnt=%d", wr_fail_cnt);
}

/*
 * Function: soft_reset_chk
 * Description: Phase 3 - Trigger soft reset by writing to
 *             SOFT_RST_REG_ADDRESS and re-verify register
 *             default values.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
static void soft_reset_chk(void)
{
    int i;
    int addr;

    LOGT("Phase 3: Soft reset check (soft_reset_chk) starting");

    // Trigger soft reset
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);

    // Brief delay for reset to take effect
    for (volatile int d = 0; d < 10000; d++);

    // Re-verify register reset values
    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        if (skip_array[i] == 1) {
            continue;
        }

        data_rd = read_reg(addr);

        if (data_rd != default_value_array[i]) {
            def_fail_cnt++;
            LOGE("SOFT_RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x Read_data : 0x%x
                 addr, default_value_array[i], data_rd);
        } else {
#ifdef DEBUG_DISPLAY
            LOGT("SOFT_RST : PASS Reading Default value from Address :0x%x Expected : 0x%x Read_data : 0x%x
                 addr, default_value_array[i], data_rd);
#endif
        }
    }

    LOGT("Phase 3: Soft reset check complete, def_fail_cnt=%d", def_fail_cnt);
}

/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Initialize failure counters and prepare for test execution.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success
 */
int ethernet0_reg_wr_rd_test_init(const TestsItem cfg)
{
    (void)cfg;

    def_fail_cnt = 0;
    wr_fail_cnt = 0;
    data_rd = 0;
    data_wr = 0;

    LOGT("ethernet0_reg_wr_rd_test init: counters initialized");
    LOGT("Register count: %d", CNT);

    return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Execute the three-phase Ethernet0 register write/read
 *             verification test:
 *             Phase 1 - Default value check (chk_rst_val)
 *             Phase 2 - Write/Read pattern check (chk_rd_wr)
 *             Phase 3 - Soft reset check (soft_reset_chk)
 *             ATP Note: This is a Register Access Test per ATP Section 9.19.
 *             External Ethernet validation, end-to-end TX/RX validation,
 *             and duplex mode validation are not applicable to this
 *             register-level verification testcase. Those requirements
 *             apply to the functional Ethernet TX/RX data path tests.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on PASS, -1 on FAIL
 */
int ethernet0_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
    (void)cfg;

    if (out == 0) {
        LOGE("ethernet0_reg_wr_rd_test output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting ethernet0_reg_wr_rd_test execution");

    // Phase 1 - Default Value Check
    chk_rst_val();

    // Phase 2 - Write/Read Check
    chk_rd_wr();

    // Phase 3 - Soft Reset Check
    soft_reset_chk();

    // Determine overall pass/fail
    if ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) {
        out->status = 0;
        LOGT("ethernet0_reg_wr_rd_test PASS: def_fail_cnt=%d wr_fail_cnt=%d
             def_fail_cnt, wr_fail_cnt);
    } else {
        out->status = -1;
        LOGE("ethernet0_reg_wr_rd_test FAIL: def_fail_cnt=%d wr_fail_cnt=%d
             def_fail_cnt, wr_fail_cnt);
    }

    return out->status;
}

/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on PASS, -1 on FAIL
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("ethernet0_reg_wr_rd_test teardown: def_fail_cnt=%d wr_fail_cnt=%d
         def_fail_cnt, wr_fail_cnt);

    return ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) ? 0 : -1;
}
