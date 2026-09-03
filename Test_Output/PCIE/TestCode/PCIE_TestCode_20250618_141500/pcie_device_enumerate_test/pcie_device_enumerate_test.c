/*
 * pcie_device_enumerate_test.c
 *
 * Test Case: pcie_device_enumerate_test
 * Description: PCIe device enumeration test. Performs link training,
 * cache programming, link status polling, vendor ID read,
 * BAR register programming, and synchronization polling.
 */

#include "pcie_device_enumerate_test.h"
#include "test_define.cin"

unsigned int test_err, data_rd, rd_wr_data1;

int pcie_device_enumerate_test_init(const TestsItem *cfg)
{
 (void)cfg;
 LOGI("[Test Init] PCIe device enumerate test: %s\n", cfg->test_name);

 return 0;
}

int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 LOGI("[Test Run] PCIe device enumerate test: %s\n", cfg->test_name);
 test_err = 0;

 /* Step 1: Clear synchronization register */
 write_reg(0xE6004100, 0x0);
 LOGI("Step 1: Cleared sync register 0xE6004100\n");

 /* Step 2: Conditional link training based on compile-time defines */
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

 /* Step 3: Cache programming - PCIE0 coherency control bits [11:14]=0xf, [3:6]=0xf */
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 3: PCIE0 coherency control bits [11:14],[3:6] set to 0xf\n");

 /* Step 4: PCIE0 coherency control bits [27:30]=0xf, [19:22]=0xf */
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 4: PCIE0 coherency control bits [27:30],[19:22] set to 0xf\n");

 /* Step 5: Repeat steps 3-4 for PCIE1 */
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 5: PCIE1 coherency control bits [11:14],[3:6] set to 0xf\n");

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 5: PCIE1 coherency control bits [27:30],[19:22] set to 0xf\n");

 /* Step 6: Wait */
 wait_on(20);
 LOGI("Step 6: wait_on(20) complete\n");

 /* Step 7: Read-modify-write PCIE0 coherency control with all four bit fields set to 0xf */
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 7: PCIE0 coherency control all four bit fields set to 0xf\n");

 /* Step 8: Read-modify-write PCIE1 coherency control similarly */
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 8: PCIE1 coherency control all four bit fields set to 0xf\n");

 /* Step 9: Duplicate link training and cache programming block (repeated in source) */
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

 /* Duplicate cache programming */
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
 LOGI("Step 9: Duplicate link training and cache programming complete\n");

 /* Step 10: Poll SII0 link status at offset 0xC0 until (data_rd & 0xD1) == 0xD1 */
 do {
 data_rd = read_sii0_reg(0xC0);
 LOGI("Step 10: Polling SII0 link status, data_rd=0x%x\n", data_rd);
 } while ((data_rd & 0xD1) != 0xD1);
 LOGI("Step 10: SII0 link status confirmed (data_rd & 0xD1) == 0xD1\n");

 /* Step 11: Poll SII1 link status at offset 0xC0 until (data_rd & 0xD1) == 0xD1 */
 do {
 data_rd = read_sii1_reg(0xC0);
 LOGI("Step 11: Polling SII1 link status, data_rd=0x%x\n", data_rd);
 } while ((data_rd & 0xD1) != 0xD1);
 LOGI("Step 11: SII1 link status confirmed (data_rd & 0xD1) == 0xD1\n");

 /* Step 12: non_secure_prot_nic() */
 non_secure_prot_nic();
 LOGI("Step 12: non_secure_prot_nic() called\n");

 #ifdef DM0_RC
 /* Step 13: Read Vendor ID from PCIe slave 0 at offset 0x0 */
 rd_wr_data1 = read_pcie_slv0_reg(0x0);
 LOGI("Step 13: Vendor ID read from pcie_slv0 offset 0x0 = 0x%x\n", rd_wr_data1);

 /* Step 14: Write command register at offset 0x4 with 0x7 */
 write_pcie_slv0_reg(0x4, 0x7);
 LOGI("Step 14: Command register at pcie_slv0 offset 0x4 written with 0x7\n");

 /* Step 15: Memory base programming */
 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 LOGI("Step 15: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() called\n");

 /* Step 16: Wait */
 wait_on(10);
 LOGI("Step 16: wait_on(10) complete\n");

 /* Step 17: Write registers with 0x1 */
 write_reg(0xE690000C, 0x1);
 write_reg(0xE6900010, 0x1);
 write_reg(0xE6900014, 0x1);
 write_reg(0xE6900018, 0x1);
 write_reg(0xE6900030, 0x1);
 write_reg(0xE6900034, 0x1);
 LOGI("Step 17: Registers 0xE690000C-0xE6900034 written with 0x1\n");

 /* Step 18: Cache disable - PCIE0 coherency control bits [19:22]=0x0, [27:30]=0x0 */
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 18: PCIE0 cache disable bits [19:22],[27:30] set to 0x0\n");

 /* Step 19: Cache disable - PCIE1 coherency control bits [19:22]=0x0, [27:30]=0x0 */
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 19: PCIE1 cache disable bits [19:22],[27:30] set to 0x0\n");

 /* Step 20: Consolidated cache disable write for both PCIE0 and PCIE1 */
 wait_on(10);
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGI("Step 20: Consolidated cache disable complete\n");

 /* Step 21: Wait */
 wait_on(30);
 LOGI("Step 21: wait_on(30) complete\n");

 /* Step 22: Write PCIe slave 1 BAR registers offsets 0x10-0x24 with 0xFFFFFFFF */
 write_pcie_slv1_reg(0x10, 0xFFFFFFFF);
 write_pcie_slv1_reg(0x14, 0xFFFFFFFF);
 write_pcie_slv1_reg(0x18, 0xFFFFFFFF);
 write_pcie_slv1_reg(0x1c, 0xFFFFFFFF);
 write_pcie_slv1_reg(0x20, 0xFFFFFFFF);
 write_pcie_slv1_reg(0x24, 0xFFFFFFFF);
 LOGI("Step 22: pcie_slv1 BAR offsets 0x10-0x24 written with 0xFFFFFFFF\n");

 /* Step 23: Read back PCIe slave 1 BAR registers */
 rd_wr_data1 = read_pcie_slv1_reg(0x10);
 LOGI("Step 23: pcie_slv1 BAR 0x10 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x14);
 LOGI("Step 23: pcie_slv1 BAR 0x14 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x18);
 LOGI("Step 23: pcie_slv1 BAR 0x18 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x1c);
 LOGI("Step 23: pcie_slv1 BAR 0x1c read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x20);
 LOGI("Step 23: pcie_slv1 BAR 0x20 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x24);
 LOGI("Step 23: pcie_slv1 BAR 0x24 read = 0x%x\n", rd_wr_data1);

 /* Step 24: Write PCIe slave 1 BAR registers with specific base addresses */
 write_pcie_slv1_reg(0x10, 0x0);
 write_pcie_slv1_reg(0x14, 0x4);
 write_pcie_slv1_reg(0x18, 0x20000000);
 write_pcie_slv1_reg(0x1c, 0x40000000);
 write_pcie_slv1_reg(0x20, 0x60000000);
 write_pcie_slv1_reg(0x24, 0x80000000);
 LOGI("Step 24: pcie_slv1 BAR offsets 0x10-0x24 written with specific base addresses\n");

 /* Step 25: Read back PCIe slave 1 BAR registers after programming */
 rd_wr_data1 = read_pcie_slv1_reg(0x10);
 LOGI("Step 25: pcie_slv1 BAR 0x10 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x14);
 LOGI("Step 25: pcie_slv1 BAR 0x14 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x18);
 LOGI("Step 25: pcie_slv1 BAR 0x18 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x1c);
 LOGI("Step 25: pcie_slv1 BAR 0x1c read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x20);
 LOGI("Step 25: pcie_slv1 BAR 0x20 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv1_reg(0x24);
 LOGI("Step 25: pcie_slv1 BAR 0x24 read = 0x%x\n", rd_wr_data1);

 /* Step 26: Repeat steps 22-25 for PCIe slave 0 */
 /* Write PCIe slave 0 BAR registers offsets 0x10-0x24 with 0xFFFFFFFF */
 write_pcie_slv0_reg(0x10, 0xFFFFFFFF);
 write_pcie_slv0_reg(0x14, 0xFFFFFFFF);
 write_pcie_slv0_reg(0x18, 0xFFFFFFFF);
 write_pcie_slv0_reg(0x1c, 0xFFFFFFFF);
 write_pcie_slv0_reg(0x20, 0xFFFFFFFF);
 write_pcie_slv0_reg(0x24, 0xFFFFFFFF);
 LOGI("Step 26: pcie_slv0 BAR offsets 0x10-0x24 written with 0xFFFFFFFF\n");

 /* Read back PCIe slave 0 BAR registers */
 rd_wr_data1 = read_pcie_slv0_reg(0x10);
 LOGI("Step 26: pcie_slv0 BAR 0x10 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x14);
 LOGI("Step 26: pcie_slv0 BAR 0x14 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x18);
 LOGI("Step 26: pcie_slv0 BAR 0x18 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x1c);
 LOGI("Step 26: pcie_slv0 BAR 0x1c read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x20);
 LOGI("Step 26: pcie_slv0 BAR 0x20 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x24);
 LOGI("Step 26: pcie_slv0 BAR 0x24 read = 0x%x\n", rd_wr_data1);

 /* Write PCIe slave 0 BAR registers with specific base addresses */
 write_pcie_slv0_reg(0x10, 0x0);
 write_pcie_slv0_reg(0x14, 0x4);
 write_pcie_slv0_reg(0x18, 0x20000000);
 write_pcie_slv0_reg(0x1c, 0x40000000);
 write_pcie_slv0_reg(0x20, 0x60000000);
 write_pcie_slv0_reg(0x24, 0x80000000);
 LOGI("Step 26: pcie_slv0 BAR offsets 0x10-0x24 written with specific base addresses\n");

 /* Read back PCIe slave 0 BAR registers after programming */
 rd_wr_data1 = read_pcie_slv0_reg(0x10);
 LOGI("Step 26: pcie_slv0 BAR 0x10 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x14);
 LOGI("Step 26: pcie_slv0 BAR 0x14 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x18);
 LOGI("Step 26: pcie_slv0 BAR 0x18 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x1c);
 LOGI("Step 26: pcie_slv0 BAR 0x1c read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x20);
 LOGI("Step 26: pcie_slv0 BAR 0x20 read = 0x%x\n", rd_wr_data1);
 rd_wr_data1 = read_pcie_slv0_reg(0x24);
 LOGI("Step 26: pcie_slv0 BAR 0x24 read = 0x%x\n", rd_wr_data1);

 #endif /* DM0_RC */

 /* Step 27: Wait */
 wait_on(10);
 LOGI("Step 27: wait_on(10) complete\n");

 /* Step 28: Poll read_reg(0xE6004100) until value equals 0x12345678 */
 do {
 data_rd = read_reg(0xE6004100);
 LOGI("Step 28: Polling 0xE6004100, data_rd=0x%x\n", data_rd);
 if (data_rd != 0x12345678) {
 wait_on(5);
 }
 } while (data_rd != 0x12345678);
 LOGI("Step 28: Synchronization confirmed, 0xE6004100 == 0x12345678\n");

 /* Step 29: finish(0) */
 finish(0);

 return out->status = test_err;
}

int pcie_device_enumerate_test_teardown(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe device enumerate test teardown: %s\n", cfg->test_name);

 return 0;
}
