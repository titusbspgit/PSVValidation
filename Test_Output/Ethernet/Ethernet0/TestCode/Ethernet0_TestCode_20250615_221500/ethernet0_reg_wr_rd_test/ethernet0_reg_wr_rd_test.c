// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

// Meta Step 6: Global declarations
static int data_rd;
static int data_wr;
static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;

/*
 * Function: chk_rst_val
 * Description: Phase 1 - Reads all 434 registers and compares each
 * against expected default/reset values stored in
 * default_value_array[]. Registers flagged in skip_array
 * (value 1) are skipped. Increments def_fail_cnt on mismatch.
 * Parameters:
 * None
 * Returns:
 * void
 */
static void chk_rst_val(void)
{
 int i;
 int addr;

 LOGT("Phase 1: chk_rst_val - Default value check starting");

 for (i = 0; i < CNT; i++) {
 addr = addr_array[i];

 if (skip_array[i] == 1) {
 continue;
 }

 data_rd = read_reg(addr);

 if (data_rd == default_value_array[i]) {
#ifdef DEBUG_DISPLAY
 LOGT("RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\tRead_data : 0x%x
 addr, default_value_array[i], data_rd);
#endif
 } else {
 def_fail_cnt++;
 LOGE("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x
 addr, default_value_array[i], data_rd);
 }
 }

 LOGT("Phase 1: chk_rst_val complete, def_fail_cnt=%d", def_fail_cnt);
}

/*
 * Function: chk_rd_wr
 * Description: Phase 2 - Iterates over 6 test patterns
 * (0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000,
 * 0xA5A5A5A5, 0xffff0000), writing each pattern masked
 * with write_mask_array[i] to each register, reading back
 * and comparing against expected value
 * (data_wr & write_mask_array[i] & read_mask_array[i]).
 * Registers flagged in skip_array are skipped.
 * Non-writable registers (write_mask = 0x00000000) are skipped.
 * Increments wr_fail_cnt on mismatch.
 * Parameters:
 * None
 * Returns:
 * void
 */
static void chk_rd_wr(void)
{
 int i;
 int j;
 int addr;
 int exp_val;

 LOGT("Phase 2: chk_rd_wr - Write/Read check starting");

 // Meta Step 16: Write and immediate read-back verification loop
 for (j = 0; j < 6; j++) {
 data_wr = chk_val[j];

 LOGT("Phase 2: Testing pattern 0x%x (pattern index %d)", data_wr, j);

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
 LOGE("WR : FAIL Addr:0x%x Pattern:0x%x Written:0x%x Expected:0x%x Read:0x%x
 addr, data_wr, (data_wr & write_mask_array[i]), exp_val, data_rd);
 }
 }
 }

 // Meta Step 17: Phase 2 reading loop
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

 LOGT("Phase 2: chk_rd_wr complete, wr_fail_cnt=%d", wr_fail_cnt);
}

/*
 * Function: soft_reset_chk
 * Description: Phase 3 - Triggers a soft reset by writing
 * SOFT_RST_REG_DATA to SOFT_RST_REG_ADDRESS and
 * re-verifies that all registers return to their
 * expected default values. Increments def_fail_cnt
 * on mismatch.
 * Parameters:
 * None
 * Returns:
 * void
 */
static void soft_reset_chk(void)
{
 int i;
 int addr;

 LOGT("Phase 3: soft_reset_chk - Soft reset check starting");

 // Meta Step 18: Trigger soft reset
 write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);

 // Deterministic busy-wait for reset to take effect
 for (volatile int d = 0; d < 10000; d++);

 LOGT("Phase 3: Soft reset triggered, re-verifying default values");

 // Re-verify register reset values after soft reset
 for (i = 0; i < CNT; i++) {
 addr = addr_array[i];

 if (skip_array[i] == 1) {
 continue;
 }

 data_rd = read_reg(addr);

 if (data_rd != default_value_array[i]) {
 def_fail_cnt++;
 LOGE("SOFT_RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x
 addr, default_value_array[i], data_rd);
 } else {
#ifdef DEBUG_DISPLAY
 LOGT("SOFT_RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\tRead_data : 0x%x
 addr, default_value_array[i], data_rd);
#endif
 }
 }

 LOGT("Phase 3: soft_reset_chk complete, def_fail_cnt=%d", def_fail_cnt);
}

/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Initializes all failure counters and global variables
 * to zero. Corresponds to Meta Step 1 and Step 6.
 * Parameters:
 * cfg - pointer to const TestsItem configuration structure
 * Returns:
 * 0 on success
 */
int ethernet0_reg_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;

 // Meta Step 1: Initialize failure counters to zero
 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 data_rd = 0;
 data_wr = 0;

 LOGT("ethernet0_reg_wr_rd_test_init: counters initialized");
 LOGT("Register count (CNT): %d", CNT);

 return 0;
}

/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Executes the three-phase Ethernet0 register write/read
 * verification test:
 * Phase 1 - chk_rst_val: Default value check
 * Phase 2 - chk_rd_wr: Write/Read pattern check (6 patterns)
 * Phase 3 - soft_reset_chk: Soft reset verification
 *
 * ATP Compliance Note (Section 9.19 Ethernet):
 * This testcase is a Register Read Write Access Test per ATP.
 * ATP pass/fail criteria for register access tests:
 * "The test is considered PASS if Ethernet 0/1/2/3 register
 * accesses are successful, reset/default values match the
 * specification."
 *
 * The following ATP requirements are NOT applicable to this
 * register-level verification testcase and apply only to
 * functional Ethernet TX/RX data path tests:
 * - External Ethernet Validation (external endpoint, topology)
 * - End-to-End Ethernet Validation (TX/RX frame, payload)
 * - Duplex Validation (Half-Duplex / Full-Duplex mode)
 *
 * Parameters:
 * cfg - pointer to const TestsItem configuration structure
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int ethernet0_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
 (void)cfg;

 if (out == (TestOutput )0) {
 LOGE("ethernet0_reg_wr_rd_test_run: output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("ethernet0_reg_wr_rd_test_run: Starting execution");

 // Meta Step 14-15: Phase 1 - Default Value Check
 chk_rst_val();

 // Meta Step 16-17: Phase 2 - Write/Read Check
 chk_rd_wr();

 // Meta Step 18: Phase 3 - Soft Reset Check
 soft_reset_chk();

 // Meta Step 19: Determine overall pass/fail based on accumulated failure counts
 if ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) {
 out->status = 0;
 LOGT("ethernet0_reg_wr_rd_test_run PASS: def_fail_cnt=%d wr_fail_cnt=%d
 def_fail_cnt, wr_fail_cnt);
 } else {
 out->status = -1;
 LOGE("ethernet0_reg_wr_rd_test_run FAIL: def_fail_cnt=%d wr_fail_cnt=%d
 def_fail_cnt, wr_fail_cnt);
 }

 return out->status;
}

/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Final cleanup and status reporting after test execution.
 * Reports accumulated failure counts.
 * Parameters:
 * cfg - pointer to const TestsItem configuration structure
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_reg_wr_rd_test_teardown: def_fail_cnt=%d wr_fail_cnt=%d
 def_fail_cnt, wr_fail_cnt);

 if ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) {
 LOGT("ethernet0_reg_wr_rd_test_teardown: PASS");
 return 0;
 }

 LOGE("ethernet0_reg_wr_rd_test_teardown: FAIL");
 return -1;
}
