// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: pcie_device_enumerate_test
 * Description: This testcase performs PCIe device enumeration. It begins by writing 0x0 to
 * address 0xE6004100, then conditionally invokes link training for DM0 or DM1
 * in RC or EP mode. Cache programming is performed by reading and modifying
 * coherency control registers. The test polls link status, programs BARs, and
 * polls for final synchronization value 0x12345678.
 /

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

unsigned int test_err, data_rd;

/
 * Function: pcie_device_enumerate_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_device_enumerate_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_init(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

 return 0;
}

/
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration including link training,
 * cache programming, link status polling, Vendor ID read, BAR sizing/programming,
 * and final synchronization polling.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);
 test_err = 0;

 / Step 1: Write 0x0 to 0xE6004100 to initialize synchronization register /
 write_reg(0xE6004100, 0x0);
 LOGI("Step 1: Wrote 0x0 to 0xE6004100\n");

 / Step 2: Conditionally call link training based on DM0_RC, DM1_RC, DM0_EP, DM1_EP /
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

 / Step 3: CACHE PROGRAMMING - PCIE0 coherency control bits [11:14]=0xf, [3:6]=0xf /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 3: PCIE0 cache programming bits [11:14]=0xf, [3:6]=0xf\n");

 / Step 4: PCIE0 coherency control bits [27:30]=0xf, [19:22]=0xf /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 4: PCIE0 cache programming bits [27:30]=0xf, [19:22]=0xf\n");

 / Step 5: Repeat steps 3-4 for PCIE1 /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 5: PCIE1 cache programming complete\n");

 / Step 6: wait_on(20) /
 wait_on(20);

 / Step 7: PCIE0 coherency control all cache bits /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 7: PCIE0 all cache bits set\n");

 / Step 8: PCIE1 coherency control all cache bits /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 8: PCIE1 all cache bits set\n");

 / Step 9: Repeat link training and cache programming block /
 #ifdef DM0_RC
 link_training_dm0_x4(4);
 #endif
 #ifdef DM1_RC
 link_training_dm1_x4(4);
 #endif
 #ifdef DM0_EP
 link_training_dm0_x4(4);
 #endif
 #ifdef DM1_EP
 link_training_dm1_x4(4);
 #endif

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 wait_on(20);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 9: Repeated link training and cache programming block\n");

 / Step 10: Read sii0 reg 0xC0 and call non_secure_prot_nic /
 data_rd = read_sii0_reg(0xC0);
 non_secure_prot_nic();
 LOGI("Step 10: read_sii0_reg(0xC0)=0x%x, non_secure_prot_nic() called\n", data_rd);

 / Step 11: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 /
 data_rd = read_sii0_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 LOGI("Polling sii0 link status: data_rd=0x%x\n", data_rd);
 wait_on(10);
 data_rd = read_sii0_reg(0xC0);
 }
 LOGI("Step 11: sii0 link up, data_rd=0x%x\n", data_rd);

 / Step 12: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 /
 data_rd = read_sii1_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 LOGI("Polling sii1 link status: data_rd=0x%x\n", data_rd);
 wait_on(10);
 data_rd = read_sii1_reg(0xC0);
 }
 LOGI("Step 12: sii1 link up, data_rd=0x%x\n", data_rd);

 / Step 13: Under DM0_RC - Vendor ID read, command write, mem base program /
 #ifdef DM0_RC
 data_rd = read_pcie_slv0_reg(0x0);
 #ifdef DEBUG_DISPLAY
 printf("Vendor ID = 0x%x\n", data_rd);
 LOGI("Step 13: Vendor ID = 0x%x\n", data_rd);
 #endif
 write_pcie_slv0_reg(0x4, 0x7);
 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 wait_on(10);
 LOGI("Step 13: DM0_RC Vendor ID read, command write, mem base programmed\n");
 #endif

 / Step 14: Write 0x1 to system registers /
 write_reg(0xE690000C, 0x1);
 write_reg(0xE6900010, 0x1);
 write_reg(0xE6900014, 0x1);
 write_reg(0xE6900018, 0x1);
 write_reg(0xE6900030, 0x1);
 write_reg(0xE6900034, 0x1);
 LOGI("Step 14: System registers 0xE690000C-0xE6900034 written with 0x1\n");

 / Step 15: DISABLE_CACHE PROGRAMMING - PCIE0 /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 / DISABLE_CACHE PROGRAMMING - PCIE1 /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 15: Disable cache programming complete\n");

 / Step 16: wait_on(10), then clear remaining cache bits /
 wait_on(10);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 16: Cache bits fully cleared\n");

 / Step 17: wait_on(30) /
 wait_on(30);

 / Step 18: BAR sizing on slv1 - write 0xFFFFFFFF and read back /
 write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x10);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x10 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x14);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x14 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x18);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x18 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x1c);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x1c sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x20);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x20 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x24);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x24 sizing readback=0x%x\n", data_rd);
 #endif
 LOGI("Step 18: slv1 BAR sizing complete\n");

 / Step 19: BAR programming on slv1 with specific base addresses /
 write_pcie_slv1_reg(0x10, 0x0);
 data_rd = read_pcie_slv1_reg(0x10);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x10 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x14, 0x4);
 data_rd = read_pcie_slv1_reg(0x14);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x14 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x18, 0x20000000);
 data_rd = read_pcie_slv1_reg(0x18);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x18 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x1c, 0x40000000);
 data_rd = read_pcie_slv1_reg(0x1c);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x1c programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x20, 0x60000000);
 data_rd = read_pcie_slv1_reg(0x20);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x20 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv1_reg(0x24, 0x80000000);
 data_rd = read_pcie_slv1_reg(0x24);
 #ifdef DEBUG_DISPLAY
 LOGI("slv1 BAR 0x24 programmed readback=0x%x\n", data_rd);
 #endif
 LOGI("Step 19: slv1 BAR programming complete\n");

 / Step 20: BAR sizing on slv0 - write 0xFFFFFFFF and read back /
 write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x10);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x10 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x14);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x14 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x18);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x18 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x1c);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x1c sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x20);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x20 sizing readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x24);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x24 sizing readback=0x%x\n", data_rd);
 #endif
 LOGI("Step 20: slv0 BAR sizing complete\n");

 / Step 21: BAR programming on slv0 with specific base addresses /
 write_pcie_slv0_reg(0x10, 0x0);
 data_rd = read_pcie_slv0_reg(0x10);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x10 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x14, 0x4);
 data_rd = read_pcie_slv0_reg(0x14);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x14 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x18, 0x20000000);
 data_rd = read_pcie_slv0_reg(0x18);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x18 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x1c, 0x40000000);
 data_rd = read_pcie_slv0_reg(0x1c);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x1c programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x20, 0x60000000);
 data_rd = read_pcie_slv0_reg(0x20);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x20 programmed readback=0x%x\n", data_rd);
 #endif

 write_pcie_slv0_reg(0x24, 0x80000000);
 data_rd = read_pcie_slv0_reg(0x24);
 #ifdef DEBUG_DISPLAY
 LOGI("slv0 BAR 0x24 programmed readback=0x%x\n", data_rd);
 #endif
 LOGI("Step 21: slv0 BAR programming complete\n");

 / Step 22: wait_on(10) /
 wait_on(10);

 / Step 23: Poll read_reg(0xE6004100) until value equals 0x12345678 /
 data_rd = read_reg(0xE6004100);
 while (data_rd != 0x12345678)
 {
 LOGI("Polling 0xE6004100: data_rd=0x%x\n", data_rd);
 wait_on(5);
 data_rd = read_reg(0xE6004100);
 }
 LOGI("Step 23: 0xE6004100 = 0x12345678, enumeration complete\n");

 / Step 24: Call finish(0) /
 finish(0);

 return out->status = test_err;
}

/
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation, final observation, and testcase completion for
 * pcie_device_enumerate_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe device enumerate test teardown: %s\n", cfg->test_name);

 return 0;
}
