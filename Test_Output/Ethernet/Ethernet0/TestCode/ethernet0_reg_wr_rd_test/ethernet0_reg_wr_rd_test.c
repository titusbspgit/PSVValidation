// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_reg_wr_rd_test.h"
#include "test_define.inc"

#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000

// MANUAL_REVIEW: Runtime condition 14 requires unsigned int for all variables. Meta DV source uses
// signed int for data_rd, data_wr, def_fail_cnt, wr_fail_cnt. Preserving Meta source types as the
// DV validation logic depends on signed comparison with const int arrays (default_value_array,
// read_mask_array, write_mask_array). Changing to unsigned may alter validation behavior.
int data_rd;
int data_wr;
int def_fail_cnt = 0;
int wr_fail_cnt = 0;


/*
 * Function: chk_rst_val
 * Description: Validates default/reset values of all 434 Ethernet0 registers
 * Parameters:
 * none
 * Returns:
 * void
 */
static void chk_rst_val(void)
{
 int i;
 unsigned long int addr;
 for (i = 0; i < CNT; i++)
 {
 addr = addr_array[i];
 if (read_mask_array[i] == 0x00000000)
 {
 LOGT("RST : Address 0x%lx is not readable, skipped for reading", addr);
 continue;
 }
 if (skip_rst_array[i] == 1)
 {
 LOGT("RST : Skipping Address : 0x%lx", addr);
 continue;
 }

 data_rd = read_reg(addr);

 if (data_rd == default_value_array[i])
 {
 LOGT("RST : PASS Reading Default value from Address :0x%lx Expected : 0x%x Read_data : 0x%x", addr, default_value_array[i], data_rd);
 }
 else
 {
 def_fail_cnt++;
 LOGE("RST : Failed Default value mismatch Addr :0x%lx Expected : 0x%x Read_data : 0x%x", addr, default_value_array[i], data_rd);
 }
 }
}


/*
 * Function: chk_rd_wr
 * Description: Writes 6 test patterns to all writable registers and reads back to validate
 * Parameters:
 * none
 * Returns:
 * void
 */
static void chk_rd_wr(void)
{
 int i, j, exp_val, wr_n;
 unsigned long int addr;
 int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};
 for (j = 0; j < 6; j++)
 {
 data_wr = chk_val[j];

 // writing into Registers

 for (i = 0; i < CNT; i++)
 {
 addr = addr_array[i];
 if (skip_array[i] == 1)
 {
 LOGT("Read_write : Writing into Address : 0x%lx is skipped because address present in skip_array", addr);
 continue;
 }
 if ((write_mask_array[i] == 0x00000000))
 {
 LOGT("Read_write : Address 0x%lx is not writable, skipped for writing", addr);
 continue;
 }
 else
 {
 write_reg(addr, data_wr);
 LOGT("Read_write : Writing into register Address : 0x%lx data :0x%x", addr, data_wr);
 }
 }

 // Reading from registers

 for (i = 0; i < CNT; i++)
 {
 addr = addr_array[i];
 if (skip_array[i] == 1)
 {
 LOGT("Read_write : Reading from Address : 0x%lx is skipped because address present in skip_array", addr);
 continue;
 }
 if (write_mask_array[i] == 0x00000000)
 {
 LOGT("Read_write : Address 0x%lx is not Writable, skipped for reading", addr);
 continue;
 }
 if (read_mask_array[i] == 0x00000000)
 {
 LOGT("Read_write : Address 0x%lx is not Readable, skipped for reading", addr);
 continue;
 }
 else
 {
 data_rd = read_reg(addr);
 wr_n = (write_mask_array[i] ^ 0xffffffff);
 exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i]));
 if (data_rd == exp_val)
 {
 LOGT("Read_write : PASS : For Address 0x%lx, Expected value=0x%x Read value=0x%x", addr, exp_val, data_rd);
 }
 else
 {
 wr_fail_cnt++;
 LOGE("Read_write : Failed : Write Read mismatch For Address 0x%lx, Expected value=0x%x Read value=0x%x", addr, exp_val, data_rd);
 }
 }
 }
 }
}


/*
 * Function: soft_reset_chk
 * Description: Performs soft reset check - defined but not called in DV test_case flow (commented out)
 * Parameters:
 * none
 * Returns:
 * void
 */
static void soft_reset_chk(void)
{
 int default_value;
 default_value = read_reg(SOFT_RST_REG_ADDRESS);
 write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
 // PSV adaptation: wait_on(1000) converted to deterministic busy-wait
 for (volatile int d = 0; d < 10000; d++);
 write_reg(SOFT_RST_REG_ADDRESS, default_value);
 // PSV adaptation: wait_on(1000) converted to deterministic busy-wait
 for (volatile int d = 0; d < 10000; d++);
}


/*
 * Function: ethernet0_reg_wr_rd_test_init
 * Description: Initializes the Ethernet0 register write/read test
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_reg_wr_rd_test_init(const TestsItem *cfg)
{
 (void)cfg;

 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 data_rd = 0;
 data_wr = 0;

 LOGT("Ethernet0 register write/read test init complete");

 return 0;
}


/*
 * Function: ethernet0_reg_wr_rd_test_run
 * Description: Executes the Ethernet0 register write/read validation test
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output structure
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL)
 */
int ethernet0_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out)
{
 (void)cfg;

 if (out == 0)
 {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting Ethernet0 register default value check");

 // Phase 1: Default value check
 chk_rst_val();

 LOGT("Default value check end");

 // Phase 2: Write and read check
 chk_rd_wr();

 LOGT("Write & Read from registers end");

 // soft_reset_chk() is commented out in DV test_case flow - preserving as commented out
 // soft_reset_chk();
 // chk_rst_val();

 // Final validation: replaces finish(1)/finish(0) from DV
 if (def_fail_cnt > 0 || wr_fail_cnt > 0)
 {
 LOGE("Test FAILED: def_fail_cnt=%d wr_fail_cnt=%d", def_fail_cnt, wr_fail_cnt);
 out->status = -1;
 }
 else
 {
 LOGT("Test PASSED: all register checks passed");
 out->status = 0;
 }

 LOGT("Run complete: %s def_fail_cnt=%d wr_fail_cnt=%d",
 (out->status == 0) ? "PASS" : "FAIL",
 def_fail_cnt,
 wr_fail_cnt);

 return out->status;
}


/*
 * Function: ethernet0_reg_wr_rd_test_teardown
 * Description: Teardown for the Ethernet0 register write/read test
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_reg_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("Ethernet0 register write/read test teardown: def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);

 return (def_fail_cnt == 0 && wr_fail_cnt == 0) ? 0 : -1;
}
