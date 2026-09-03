// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_mem_wr_rd_test.h"
#include "test_define.cin"

/* PCIe Memory Write Read Test
 * Description: This testcase performs PCIe memory write and read verification
 * through the PCIe slave interfaces.
 /

unsigned int data_rd, test_err, rdata;
int int_pend;

/
 * Function: pcie_mem_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_mem_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[Test Init] PCIe mem wr rd test: %s\n", cfg->test_name);

 return 0;
}

/
 * Function: pcie_mem_wr_rd_test_run
 * Description: Main testcase execution for PCIe memory write and read verification
 * including link training, cache programming, link-up polling, BAR programming,
 * cache disable, memory write-read tests, and completion polling.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 LOGI("[Test Run] PCIe mem wr rd test: %s\n", cfg->test_name);
 test_err = 0;

 / Step 1: Initialize control register /
 write_reg(0xE6004100, 0x0);
 LOGI("Step 1: write_reg(0xE6004100, 0x0) done\n");

 / Step 2: Link training /
 #ifdef DM0_RC
 link_training_dm0_x4(4);
 LOGI("Step 2: link_training_dm0_x4(4) called (DM0_RC)\n");
 #endif
 #ifdef DM1_RC
 link_training_dm1_x4(4);
 LOGI("Step 2: link_training_dm1_x4(4) called (DM1_RC)\n");
 #endif
 #ifdef DM0_EP
 link_training_dm0_x4(4);
 LOGI("Step 2: link_training_dm0_x4(4) called (DM0_EP)\n");
 #endif
 #ifdef DM1_EP
 link_training_dm1_x4(4);
 LOGI("Step 2: link_training_dm1_x4(4) called (DM1_EP)\n");
 #endif

 / Steps 3-4: CACHE PROGRAMMING - PCIE0 phase 1 /
 LOGI("Steps 3-4: PCIE0 cache programming\n");
 rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0xf);
 rdata = set_data(rdata, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);

 / CACHE PROGRAMMING - PCIE0 phase 2 /
 rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 27, 30, 0xf);
 rdata = set_data(rdata, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Steps 3-4: PCIE0 cache programming done\n");

 / Steps 5-6: CACHE PROGRAMMING - PCIE1 phase 1 /
 LOGI("Steps 5-6: PCIE1 cache programming\n");
 rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0xf);
 rdata = set_data(rdata, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);

 / CACHE PROGRAMMING - PCIE1 phase 2 /
 rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 27, 30, 0xf);
 rdata = set_data(rdata, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Steps 5-6: PCIE1 cache programming done\n");

 / Step 7: wait_on(20) /
 wait_on(20);

 / Step 8: CACHE PROGRAMMING - PCIE0 all fields /
 rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0xf);
 rdata = set_data(rdata, 3, 6, 0xf);
 rdata = set_data(rdata, 27, 30, 0xf);
 rdata = set_data(rdata, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);

 / CACHE PROGRAMMING - PCIE1 all fields /
 rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0xf);
 rdata = set_data(rdata, 3, 6, 0xf);
 rdata = set_data(rdata, 27, 30, 0xf);
 rdata = set_data(rdata, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Step 8: Cache programming all fields done\n");

 / Step 9: SII0 link-up polling /
 LOGI("Step 9: Polling SII0 link status\n");
 data_rd = read_sii0_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SII0 polling: data_rd=0x%x\n", data_rd);
 #endif
 wait_on(10);
 data_rd = read_sii0_reg(0xC0);
 }
 LOGI("Step 9: SII0 link-up status achieved\n");

 / Step 10: SII1 link-up polling /
 LOGI("Step 10: Polling SII1 link status\n");
 data_rd = read_sii1_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SII1 polling: data_rd=0x%x\n", data_rd);
 #endif
 wait_on(10);
 data_rd = read_sii1_reg(0xC0);
 }
 LOGI("Step 10: SII1 link-up status achieved\n");

 / Steps 11-12: non_secure_prot_nic /
 non_secure_prot_nic();
 LOGI("Steps 11-12: non_secure_prot_nic() done\n");

 / Steps 13-14: BAR programming under DM0_RC /
 #ifdef DM0_RC
 {
 unsigned int vendor_id;
 vendor_id = read_pcie_slv0_reg(0x0);
 LOGI("Step 13: Vendor ID = 0x%x\n", vendor_id);

 write_pcie_slv0_reg(0x4, 0x7);
 LOGI("Step 13: write_pcie_slv0_reg(0x4, 0x7) done\n");

 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 LOGI("Step 14: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() done\n");

 wait_on(10);
 }
 #endif

 / Steps 15-16: BAR programming under DM1_RC /
 #ifdef DM1_RC
 {
 unsigned int vendor_id;
 vendor_id = read_pcie_slv0_reg(0x0);
 LOGI("Step 15: Vendor ID = 0x%x\n", vendor_id);

 write_pcie_slv0_reg(0x4, 0x7);
 LOGI("Step 15: write_pcie_slv0_reg(0x4, 0x7) done\n");

 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 LOGI("Step 16: mem_base_program done\n");

 wait_on(10);
 }
 #endif

 / Steps 17-18: DISABLE_CACHE PROGRAMMING - PCIE0 /
 LOGI("Steps 17-18: PCIE0 cache disable programming\n");
 rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 19, 22, 0x0);
 rdata = set_data(rdata, 27, 30, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Steps 17-18: PCIE0 cache disable done\n");

 / Steps 19-20: DISABLE_CACHE PROGRAMMING - PCIE1 /
 LOGI("Steps 19-20: PCIE1 cache disable programming\n");
 rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 19, 22, 0x0);
 rdata = set_data(rdata, 27, 30, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Steps 19-20: PCIE1 cache disable done\n");

 / Step 21: wait_on(10) /
 wait_on(10);

 / Steps 22-23: Clear all cache fields /
 rdata = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0x0);
 rdata = set_data(rdata, 3, 6, 0x0);
 rdata = set_data(rdata, 27, 30, 0x0);
 rdata = set_data(rdata, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);

 rdata = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 rdata = set_data(rdata, 11, 14, 0x0);
 rdata = set_data(rdata, 3, 6, 0x0);
 rdata = set_data(rdata, 27, 30, 0x0);
 rdata = set_data(rdata, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, rdata);
 LOGI("Steps 22-23: All cache fields cleared\n");

 / wait_on(30) /
 wait_on(30);

 / Steps 24-25: Memory write-read test on pcie_slv0 /
 LOGI("Steps 24-25: Memory write-read test on pcie_slv0\n");
 #ifdef DM0
 {
 unsigned int wr_data, rd_data;

 / Write test pattern to pcie_slv0 /
 wr_data = 0xA5A5A5A5;
 write_pcie_slv0_reg(0x0, wr_data);
 rd_data = read_pcie_slv0_reg(0x0);
 #ifdef DEBUG_DISPLAY
 LOGI("pcie_slv0 wr=0x%x rd=0x%x\n", wr_data, rd_data);
 #endif
 if (rd_data != wr_data)
 {
 LOGI("ERROR: pcie_slv0 mem wr/rd mismatch wr=0x%x rd=0x%x\n", wr_data, rd_data);
 test_err++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: pcie_slv0 mem wr/rd match\n");
 #endif
 }
 }
 #endif

 / Steps 26-27: Memory write-read test on pcie_slv1 /
 LOGI("Steps 26-27: Memory write-read test on pcie_slv1\n");
 #ifdef DM1
 {
 unsigned int wr_data, rd_data;

 / Write test pattern to pcie_slv1 /
 wr_data = 0x5A5A5A5A;
 write_pcie_slv1_reg(0x0, wr_data);
 rd_data = read_pcie_slv1_reg(0x0);
 #ifdef DEBUG_DISPLAY
 LOGI("pcie_slv1 wr=0x%x rd=0x%x\n", wr_data, rd_data);
 #endif
 if (rd_data != wr_data)
 {
 LOGI("ERROR: pcie_slv1 mem wr/rd mismatch wr=0x%x rd=0x%x\n", wr_data, rd_data);
 test_err++;
 }
 else
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: pcie_slv1 mem wr/rd match\n");
 #endif
 }
 }
 #endif

 / Step 28: wait_on(10) /
 wait_on(10);

 / Steps 29-30: Completion polling - Poll read_reg(0xE6004100) until 0x12345678 /
 LOGI("Steps 29-30: Polling 0xE6004100 for completion synchronization\n");
 rdata = read_reg(0xE6004100);
 while (rdata != 0x12345678)
 {
 wait_on(5);
 rdata = read_reg(0xE6004100);
 }
 LOGI("Steps 29-30: Completion synchronization achieved (0x12345678)\n");

 / finish(0) /
 finish(0);

 return out->status = test_err;
}

/
 * Function: pcie_mem_wr_rd_test_teardown
 * Description: Performs teardown and final observation for pcie_mem_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe mem wr rd test teardown: %s\n", cfg->test_name);

 return 0;
}
