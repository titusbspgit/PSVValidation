// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_device_enumerate_test.h"
#include "test_define.inc"

/* PCIe Device Enumerate Test /
/ This testcase performs PCIe device enumeration including link training, /
/ cache programming, link status polling, BAR sizing/assignment, and /
/ final synchronization polling. /

static pcie_enum_test_ctx_t g_ctx;

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
 unsigned int data_rd;

 (void)cfg;

 g_ctx = (pcie_enum_test_ctx_t){0};

 LOGT("PCIe device enumerate test init started");

 / Step 1: Write 0x0 to synchronization register 0xE6004100 /
 write_reg(PCIE_SYNC_REG, 0x0);
 LOGT("Wrote 0x0 to sync register 0x%lx", (unsigned long)PCIE_SYNC_REG);

 / Step 2: Conditionally call link training based on compile-time defines /
#if defined(DM0_RC) || defined(DM0_EP)
 link_training_dm0_x4(4);
 LOGT("link_training_dm0_x4(4) called");
#endif
#if defined(DM1_RC) || defined(DM1_EP)
 link_training_dm1_x4(4);
 LOGT("link_training_dm1_x4(4) called");
#endif

 / Step 3: Cache programming PCIE0 - set bits [11:14]=0xf, [3:6]=0xf /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE0 cache programming phase1a done");

 / Step 4: Cache programming PCIE0 - set bits [27:30]=0xf, [19:22]=0xf /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE0 cache programming phase1b done");

 / Step 5: Cache programming PCIE1 - set bits [11:14]=0xf, [3:6]=0xf /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE1 cache programming phase1a done");

 / Cache programming PCIE1 - set bits [27:30]=0xf, [19:22]=0xf /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE1 cache programming phase1b done");

 / Step 6: wait_on(20) /
 wait_on(20);
 LOGT("wait_on(20) completed");

 / Step 7: Repeat cache programming PCIE0 - all bits /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE0 cache programming phase2 done");

 / Step 8: Repeat cache programming PCIE1 - all bits /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE1 cache programming phase2 done");

 / Step 9: Repeat link training and cache programming (duplicate block) /
#if defined(DM0_RC) || defined(DM0_EP)
 link_training_dm0_x4(4);
#endif
#if defined(DM1_RC) || defined(DM1_EP)
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
 LOGT("Duplicate link training and cache programming block done");

 LOGT("PCIe device enumerate test init completed");
 return 0;
}

/
 * Function: pcie_device_enumerate_test_run
 * Description: Executes the main testcase flow for pcie_device_enumerate_test.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned int data_rd;

 (void)cfg;

 if (out == 0) {
 LOGE("PCIe enumerate test output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting PCIe device enumeration run");

 / Step 10: Read SII0 link status register /
 data_rd = read_sii0_reg(PCIE_SII_LINK_STATUS_OFF);
 LOGT("Initial read_sii0_reg(0xC0) = 0x%x", data_rd);

 / Step 11: Call non_secure_prot_nic /
 non_secure_prot_nic();
 LOGT("non_secure_prot_nic() called");

 / Step 12: Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 /
 data_rd = read_sii0_reg(PCIE_SII_LINK_STATUS_OFF);
 while ((data_rd & PCIE_LINK_STATUS_MASK) != PCIE_LINK_STATUS_EXPECTED) {
 data_rd = read_sii0_reg(PCIE_SII_LINK_STATUS_OFF);
 }
 LOGT("SII0 link status poll passed: data_rd=0x%x", data_rd);

 / Step 13: Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 /
 data_rd = read_sii1_reg(PCIE_SII_LINK_STATUS_OFF);
 while ((data_rd & PCIE_LINK_STATUS_MASK) != PCIE_LINK_STATUS_EXPECTED) {
 data_rd = read_sii1_reg(PCIE_SII_LINK_STATUS_OFF);
 }
 LOGT("SII1 link status poll passed: data_rd=0x%x", data_rd);

#ifdef DM0_RC
 / Step 14: Read Vendor ID from pcie_slv0 offset 0x0 /
 data_rd = read_pcie_slv0_reg(0x0);
 LOGT("Vendor ID from pcie_slv0: 0x%x", data_rd);

 / Step 15: Write command register pcie_slv0 offset 0x4 = 0x7 /
 write_pcie_slv0_reg(0x4, 0x7);
 LOGT("Command register written: write_pcie_slv0_reg(0x4, 0x7)");

 / Step 16: Memory base programming for dm0 and dm1 /
 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 LOGT("mem_base_program_dm0_x4() and mem_base_program_dm1_x4() called");
#endif

 / Step 17: wait_on(10) /
 wait_on(10);

 / Step 18: Write 0x1 to system-level registers /
 write_reg(PCIE_SYS_REG_0C, 0x1);
 write_reg(PCIE_SYS_REG_10, 0x1);
 write_reg(PCIE_SYS_REG_14, 0x1);
 write_reg(PCIE_SYS_REG_18, 0x1);
 write_reg(PCIE_SYS_REG_30, 0x1);
 write_reg(PCIE_SYS_REG_34, 0x1);
 LOGT("System registers 0xE690000C-0xE6900034 written with 0x1");

 / Step 19: Cache disable PCIE0 /
 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE0 cache disable done");

 / Step 20: Cache disable PCIE1 /
 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 11, 14, 0xf);
 data_rd = set_data(data_rd, 3, 6, 0xf);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0xf);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("PCIE1 cache disable done");

 / Step 21: wait_on(10), then combined cache disable bits [27:30]=0x0, [19:22]=0x0 /
 wait_on(10);

 data_rd = read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);

 data_rd = read_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF);
 data_rd = set_data(data_rd, 27, 30, 0x0);
 data_rd = set_data(data_rd, 19, 22, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, data_rd);
 LOGT("Combined cache disable for PCIE0 and PCIE1 done");

 / Step 22: wait_on(30) /
 wait_on(30);
 LOGT("wait_on(30) completed");

 / Step 23: BAR sizing - write 0xFFFFFFFF to pcie_slv1 BAR registers /
 write_pcie_slv1_reg(PCIE_BAR0_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv1_reg(PCIE_BAR1_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv1_reg(PCIE_BAR2_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv1_reg(PCIE_BAR3_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv1_reg(PCIE_BAR4_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv1_reg(PCIE_BAR5_OFF, PCIE_BAR_SIZE_PROBE);
 LOGT("pcie_slv1 BAR sizing writes done");

 / Step 24: Read back pcie_slv1 BAR registers /
 data_rd = read_pcie_slv1_reg(PCIE_BAR0_OFF);
 LOGT("pcie_slv1 BAR0 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR1_OFF);
 LOGT("pcie_slv1 BAR1 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR2_OFF);
 LOGT("pcie_slv1 BAR2 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR3_OFF);
 LOGT("pcie_slv1 BAR3 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR4_OFF);
 LOGT("pcie_slv1 BAR4 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR5_OFF);
 LOGT("pcie_slv1 BAR5 size readback: 0x%x", data_rd);

 / Step 25: Write specific address values to pcie_slv1 BAR registers /
 write_pcie_slv1_reg(PCIE_BAR0_OFF, 0x0);
 write_pcie_slv1_reg(PCIE_BAR1_OFF, 0x4);
 write_pcie_slv1_reg(PCIE_BAR2_OFF, 0x20000000);
 write_pcie_slv1_reg(PCIE_BAR3_OFF, 0x40000000);
 write_pcie_slv1_reg(PCIE_BAR4_OFF, 0x60000000);
 write_pcie_slv1_reg(PCIE_BAR5_OFF, 0x80000000);
 LOGT("pcie_slv1 BAR address assignment done");

 / Step 26: Read back pcie_slv1 BAR registers after assignment /
 data_rd = read_pcie_slv1_reg(PCIE_BAR0_OFF);
 LOGT("pcie_slv1 BAR0 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR1_OFF);
 LOGT("pcie_slv1 BAR1 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR2_OFF);
 LOGT("pcie_slv1 BAR2 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR3_OFF);
 LOGT("pcie_slv1 BAR3 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR4_OFF);
 LOGT("pcie_slv1 BAR4 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv1_reg(PCIE_BAR5_OFF);
 LOGT("pcie_slv1 BAR5 assigned readback: 0x%x", data_rd);

 / Step 27: BAR sizing - write 0xFFFFFFFF to pcie_slv0 BAR registers /
 write_pcie_slv0_reg(PCIE_BAR0_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv0_reg(PCIE_BAR1_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv0_reg(PCIE_BAR2_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv0_reg(PCIE_BAR3_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv0_reg(PCIE_BAR4_OFF, PCIE_BAR_SIZE_PROBE);
 write_pcie_slv0_reg(PCIE_BAR5_OFF, PCIE_BAR_SIZE_PROBE);
 LOGT("pcie_slv0 BAR sizing writes done");

 / Read back pcie_slv0 BAR registers /
 data_rd = read_pcie_slv0_reg(PCIE_BAR0_OFF);
 LOGT("pcie_slv0 BAR0 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR1_OFF);
 LOGT("pcie_slv0 BAR1 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR2_OFF);
 LOGT("pcie_slv0 BAR2 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR3_OFF);
 LOGT("pcie_slv0 BAR3 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR4_OFF);
 LOGT("pcie_slv0 BAR4 size readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR5_OFF);
 LOGT("pcie_slv0 BAR5 size readback: 0x%x", data_rd);

 / Write specific address values to pcie_slv0 BAR registers /
 write_pcie_slv0_reg(PCIE_BAR0_OFF, 0x0);
 write_pcie_slv0_reg(PCIE_BAR1_OFF, 0x4);
 write_pcie_slv0_reg(PCIE_BAR2_OFF, 0x20000000);
 write_pcie_slv0_reg(PCIE_BAR3_OFF, 0x40000000);
 write_pcie_slv0_reg(PCIE_BAR4_OFF, 0x60000000);
 write_pcie_slv0_reg(PCIE_BAR5_OFF, 0x80000000);
 LOGT("pcie_slv0 BAR address assignment done");

 / Read back pcie_slv0 BAR registers after assignment /
 data_rd = read_pcie_slv0_reg(PCIE_BAR0_OFF);
 LOGT("pcie_slv0 BAR0 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR1_OFF);
 LOGT("pcie_slv0 BAR1 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR2_OFF);
 LOGT("pcie_slv0 BAR2 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR3_OFF);
 LOGT("pcie_slv0 BAR3 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR4_OFF);
 LOGT("pcie_slv0 BAR4 assigned readback: 0x%x", data_rd);
 data_rd = read_pcie_slv0_reg(PCIE_BAR5_OFF);
 LOGT("pcie_slv0 BAR5 assigned readback: 0x%x", data_rd);

 / Step 28: wait_on(10) /
 wait_on(10);

 / Step 29: Poll read_reg(0xE6004100) until data_rd == 0x12345678 /
 data_rd = read_reg(PCIE_SYNC_REG);
 while (data_rd != PCIE_SYNC_EXPECTED) {
 wait_on(5);
 data_rd = read_reg(PCIE_SYNC_REG);
 }
 LOGT("Final sync poll passed: read_reg(0xE6004100) == 0x12345678");

 / Step 30: Call finish(0) /
 finish(0);

 LOGT("PCIe device enumerate test run completed: PASS");

 out->status = 0;
 return 0;
}

/
 * Function: pcie_device_enumerate_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for pcie_device_enumerate_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_device_enumerate_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("PCIe device enumerate test teardown: no additional cleanup required");
 return g_ctx.errors == 0U ? 0 : -1;
}
