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
 * expected default/reset values. Skip registers flagged
 * in skip_array.
 * Parameters:
 * None (uses global arrays and counters)
 * Returns:
 * void
 */
static void chk_rst_val(void)
{
 int i;
 int addr;

 LOGT("Phase 1: chk_rst_val - Default value check for %d registers", CNT);

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
}

/*
 * Function: chk_rd_wr
 * Description: Phase 2 - Write/Read check. For each of 6 test patterns,
 * write pattern masked with write_mask to each register,
 * read back and compare against expected value.
 * Parameters:
 * None (uses global arrays and counters)
 * Returns:
 * void
 */
static void chk_rd_wr(void)
{
 int i;
 int j;
 int addr;
 int exp_val;

 LOGT("Phase 2: chk_rd_wr - Write/Read check with 6 test patterns");

 for (j = 0; j < 6; j++) {
 data_wr = chk_val[j];

 LOGT("Phase 2: Testing pattern 0x%x (pattern %d of 6)", data_wr, j + 1);

 for (i = 0; i < CNT; i++) {
 addr = addr_array[i];

 if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
 LOGT("Read_write : Writing into this Address : 0x%x is skipped because address present in skip_array", addr);
#endif
 continue;
 }

 if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
 LOGT("Read_write : This address 0x%x is not Writable , hence skipped for reading", addr);
#endif
 continue;
 }

 write_reg(addr, data_wr & write_mask_array[i]);
 data_rd = read_reg(addr);
 exp_val = data_wr & write_mask_array[i] & read_mask_array[i];

 if (data_rd != exp_val) {
 wr_fail_cnt++;
 LOGE("WR : FAIL Addr :0x%x Pattern :0x%x Written :0x%x Read :0x%x Expected :0x%x
 addr, data_wr, data_wr & write_mask_array[i], data_rd, exp_val);
 }
 }
 }

 // Meta Step 17: Phase 2 reading loop
 for (i = 0; i < CNT; i++) {
 addr = addr_array[i];

 if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
 LOGT("Read_write : Reading from this Address : 0x%x is skipped because address present in skip_array", addr);
#endif
 continue;
 }

 if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
 LOGT("Read_write : This address 0x%x is not Writable , hence skipped for reading", addr);
#endif
 continue;
 }
 }
}

/*
 * Function: soft_reset_chk
 * Description: Phase 3 - Trigger soft reset by writing to
 * SOFT_RST_REG_ADDRESS and re-verify register
 * default values.
 * Parameters:
 * None (uses global arrays and counters)
 * Returns:
 * void
 */
static void soft_reset_chk(void)
{
 int i;
 int addr;

 LOGT("Phase 3: soft_reset_chk - Soft reset and re-verify default values");

 // Trigger soft reset
 write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);

 // Delay after soft reset to allow registers to settle
 for (volatile int d = 0; d < 10000; d++);

 // Re-verify register reset values
 for (i = 0; i < CNT; i++) {
 addr = addr_array[i];

 if (skip_array[i] == 1) {
 continue;
 }

 data_rd = read_reg(addr);

 if (data_rd == default_value_array[i]) {
#ifdef DEBUG_DISPLAY
 LOGT("SOFT_RST : PASS Default value from Address :0x%x Expected : 0x%x Read_data : 0x%x
 addr, default_value_array[i], data_rd);
#endif
 } else {
 def_fail_cnt++;
 LOGE("SOFT_RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x Read_data : 0x%x
 addr, default_value_array[i], data_rd);
 }
 }
}

/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Initialize failure counters and prepare for test execution.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;

 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 data_rd = 0;
 data_wr = 0;

 LOGT("ethernet0_reg_wr_rd_test_init: Failure counters initialized");
 LOGT("ethernet0_reg_wr_rd_test_init: CNT=%d registers to verify", CNT);

 return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Execute the three-phase register write/read verification:
 * Phase 1 - Default value check (chk_rst_val)
 * Phase 2 - Write/Read pattern check (chk_rd_wr)
 * Phase 3 - Soft reset check (soft_reset_chk)
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;

 if (out == 0) {
 LOGE("ethernet0_reg_wr_rd_test_run: output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("ethernet0_reg_wr_rd_test_run: Starting register write/read verification");

 // Phase 1 - chk_rst_val
 chk_rst_val();

 LOGT("Phase 1 complete: def_fail_cnt=%d", def_fail_cnt);

 // Phase 2 - chk_rd_wr
 chk_rd_wr();

 LOGT("Phase 2 complete: wr_fail_cnt=%d", wr_fail_cnt);

 // Phase 3 - soft_reset_chk
 soft_reset_chk();

 LOGT("Phase 3 complete: def_fail_cnt=%d (cumulative after soft reset)", def_fail_cnt);

 // Determine pass/fail based on accumulated failure counts
 if ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) {
 out->status = 0;
 LOGT("ethernet0_reg_wr_rd_test_run: PASS - def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);
 } else {
 out->status = -1;
 LOGE("ethernet0_reg_wr_rd_test_run: FAIL - def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);
 }

 return out->status;
}

/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_reg_wr_rd_test_teardown: def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);

 return ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) ? 0 : -1;
}
