// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/* PCIe Device Enumerate Test
 * Description: This testcase performs PCIe device enumeration including link training,
 * cache programming, link status polling, BAR sizing/programming, and
 * final synchronization polling.
 */

unsigned int data_rd, test_err;

/*
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
 LOGT("[Test Init] PCIe device enumerate test: %s", cfg->test_name);

 test_err = 0;

 return 0;
}

/*
 * Function: pcie_device_enumerate_test_run
 * Description: Main testcase execution for PCIe device enumeration including link training,
 * cache programming, link status polling, Vendor ID read, BAR sizing/programming,
 * system register writes, cache disable, and final synchronization polling.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;

 LOGT("[Test Run] PCIe device enumerate test: %s", cfg->test_name);
 test_err = 0;

 /* Step 1: Write 0x0 to 0xE6004100 to initialize synchronization register */
 LOGT("Step 1: Initialize synchronization register 0xE6004100");
 write_reg(0xE6004100, 0x0);

 /* Step 2: Conditionally call link training based on DM0_RC, DM1_RC, DM0_EP, DM1_EP */
 LOGT("Step 2: Conditional link training");
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

 /* Step 3: CACHE PROGRAMMING - PCIE0 coherency control bits [11:14]=0xf, [3:6]=0xf */
 LOGT("Step 3: Cache programming PCIE0 - set bits [11:14]=0xf, [3:6]=0xf");
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 4: PCIE0 coherency control bits [27:30]=0xf, [19:22]=0xf */
 LOGT("Step 4: Cache programming PCIE0 - set bits [27:30]=0xf, [19:22]=0xf");
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 5: Repeat steps 3-4 for PCIE1 */
 LOGT("Step 5: Cache programming PCIE1 - set bits [11:14]=0xf, [3:6]=0xf");
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 LOGT("Step 5: Cache programming PCIE1 - set bits [27:30]=0xf, [19:22]=0xf");
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 6: wait_on(20) */
 LOGT("Step 6: Wait 20");
 wait_on(20);

 /* Step 7: PCIE0 coherency control all bits set */
 LOGT("Step 7: Cache programming PCIE0 - set all cache bits");
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 8: PCIE1 coherency control all bits set */
 LOGT("Step 8: Cache programming PCIE1 - set all cache bits");
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 9: Repeat link training and cache programming block (duplicate) */
 LOGT("Step 9: Repeat link training and cache programming");
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

 /* Repeat cache programming for PCIE0 */
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Repeat cache programming for PCIE1 */
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

 /* Step 10: Read sii0 reg 0xC0 and call non_secure_prot_nic() */
 LOGT("Step 10: Read sii0 link status and call non_secure_prot_nic");
 data_rd = read_sii0_reg(0xC0);
 non_secure_prot_nic();

 /* Step 11: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
 LOGT("Step 11: Polling sii0 link status for link-up");
 while ((data_rd & 0xD1) != 0xD1)
 {
 data_rd = read_sii0_reg(0xC0);
 #ifdef DEBUG_DISPLAY
 LOGT("Polling sii0 link status: data_rd=0x%x", data_rd);
 #endif
 }

 /* Step 12: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
 LOGT("Step 12: Polling sii1 link status for link-up");
 data_rd = read_sii1_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 data_rd = read_sii1_reg(0xC0);
 #ifdef DEBUG_DISPLAY
 LOGT("Polling sii1 link status: data_rd=0x%x", data_rd);
 #endif
 }

 /* Step 13: Under DM0_RC - Vendor ID read, command write, mem base program */
 #ifdef DM0_RC
 {
 LOGT("Step 13: DM0_RC - Read Vendor ID, write command, mem base program");
 data_rd = read_pcie_slv0_reg(0x0);
 printf("Vendor ID = 0x%x\n", data_rd);
 LOGT("Vendor ID read from slv0 reg 0x0 = 0x%x", data_rd);

 write_pcie_slv0_reg(0x4, 0x7);

 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();

 wait_on(10);
 }
 #endif

 /* Step 14: Write 0x1 to system registers */
 LOGT("Step 14: Write 0x1 to system registers 0xE690000C - 0xE6900034");
 write_reg(0xE690000C, 0x1);
 write_reg(0xE6900010, 0x1);
 write_reg(0xE6900014, 0x1);
 write_reg(0xE6900018, 0x1);
 write_reg(0xE6900030, 0x1);
 write_reg(0xE6900034, 0x1);

 /* Step 15: DISABLE_CACHE PROGRAMMING for PCIE0 */
 LOGT("Step 15: Disable cache programming PCIE0");
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* DISABLE_CACHE PROGRAMMING for PCIE1 */
 LOGT("Step 15: Disable cache programming PCIE1");
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 /* Step 16: wait_on(10), then clear cache bits [27:30] and [19:22] for PCIE0 and PCIE1 */
 LOGT("Step 16: Final cache disable for PCIE0 and PCIE1");
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

 /* Step 17: wait_on(30) */
 LOGT("Step 17: Wait 30");
 wait_on(30);

 /* Step 18: BAR sizing - write 0xFFFFFFFF to slv1 offsets 0x10-0x24, read back */
 LOGT("Step 18: BAR sizing on slv1 - write 0xFFFFFFFF and read back");
 write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x10);
 LOGT("slv1 BAR offset 0x10 readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x14);
 LOGT("slv1 BAR offset 0x14 readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x18);
 LOGT("slv1 BAR offset 0x18 readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x1c);
 LOGT("slv1 BAR offset 0x1c readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x20);
 LOGT("slv1 BAR offset 0x20 readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
 data_rd = read_pcie_slv1_reg(0x24);
 LOGT("slv1 BAR offset 0x24 readback = 0x%x", data_rd);

 /* Step 19: BAR programming - write specific base addresses to slv1, read back */
 LOGT("Step 19: BAR programming on slv1 - write base addresses and read back");
 write_pcie_slv1_reg(0x10, 0x0);
 data_rd = read_pcie_slv1_reg(0x10);
 LOGT("slv1 BAR offset 0x10 programmed readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x14, 0x4);
 data_rd = read_pcie_slv1_reg(0x14);
 LOGT("slv1 BAR offset 0x14 programmed readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x18, 0x20000000);
 data_rd = read_pcie_slv1_reg(0x18);
 LOGT("slv1 BAR offset 0x18 programmed readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x1c, 0x40000000);
 data_rd = read_pcie_slv1_reg(0x1c);
 LOGT("slv1 BAR offset 0x1c programmed readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x20, 0x60000000);
 data_rd = read_pcie_slv1_reg(0x20);
 LOGT("slv1 BAR offset 0x20 programmed readback = 0x%x", data_rd);

 write_pcie_slv1_reg(0x24, 0x80000000);
 data_rd = read_pcie_slv1_reg(0x24);
 LOGT("slv1 BAR offset 0x24 programmed readback = 0x%x", data_rd);

 /* Step 20: BAR sizing - write 0xFFFFFFFF to slv0 offsets 0x10-0x24, read back */
 LOGT("Step 20: BAR sizing on slv0 - write 0xFFFFFFFF and read back");
 write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x10);
 LOGT("slv0 BAR offset 0x10 readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x14);
 LOGT("slv0 BAR offset 0x14 readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x18);
 LOGT("slv0 BAR offset 0x18 readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x1c);
 LOGT("slv0 BAR offset 0x1c readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x20);
 LOGT("slv0 BAR offset 0x20 readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
 data_rd = read_pcie_slv0_reg(0x24);
 LOGT("slv0 BAR offset 0x24 readback = 0x%x", data_rd);

 /* Step 21: BAR programming - write specific base addresses to slv0, read back */
 LOGT("Step 21: BAR programming on slv0 - write base addresses and read back");
 write_pcie_slv0_reg(0x10, 0x0);
 data_rd = read_pcie_slv0_reg(0x10);
 LOGT("slv0 BAR offset 0x10 programmed readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x14, 0x4);
 data_rd = read_pcie_slv0_reg(0x14);
 LOGT("slv0 BAR offset 0x14 programmed readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x18, 0x20000000);
 data_rd = read_pcie_slv0_reg(0x18);
 LOGT("slv0 BAR offset 0x18 programmed readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x1c, 0x40000000);
 data_rd = read_pcie_slv0_reg(0x1c);
 LOGT("slv0 BAR offset 0x1c programmed readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x20, 0x60000000);
 data_rd = read_pcie_slv0_reg(0x20);
 LOGT("slv0 BAR offset 0x20 programmed readback = 0x%x", data_rd);

 write_pcie_slv0_reg(0x24, 0x80000000);
 data_rd = read_pcie_slv0_reg(0x24);
 LOGT("slv0 BAR offset 0x24 programmed readback = 0x%x", data_rd);

 /* Step 22: wait_on(10) */
 LOGT("Step 22: Wait 10");
 wait_on(10);

 /* Step 23: Poll read_reg(0xE6004100) until value equals 0x12345678 */
 LOGT("Step 23: Polling 0xE6004100 for enumeration completion (0x12345678)");
 data_rd = read_reg(0xE6004100);
 while (data_rd != 0x12345678)
 {
 wait_on(5);
 data_rd = read_reg(0xE6004100);
 #ifdef DEBUG_DISPLAY
 LOGT("Polling 0xE6004100: data_rd=0x%x", data_rd);
 #endif
 }
 LOGT("Enumeration completion detected: 0xE6004100 = 0x12345678");

 /* Step 24: Call finish(0) */
 LOGT("Step 24: Calling finish(0)");
 finish(0);

 return out->status = test_err;
}

/*
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs validation, final observation, and testcase completion for pcie_device_enumerate_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem cfg)
{
 (void)cfg;
 LOGT("[TEARDOWN] PCIe device enumerate test teardown: %s", cfg->test_name);

 /* Validation: Final polling confirmed 0xE6004100 == 0x12345678 indicating successful enumeration */
 /* BAR sizing and programming readbacks were logged during run phase */
 /* Link status polling for sii0 and sii1 confirmed (data_rd & 0xD1) == 0xD1 */

 return test_err == 0 ? 0 : -1;
}
