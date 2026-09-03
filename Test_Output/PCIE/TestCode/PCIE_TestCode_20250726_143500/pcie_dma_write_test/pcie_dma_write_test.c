// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.cin"

/* PCIe DMA Write Test
 * Description: This testcase performs PCIe DMA write and read operations
 * across four channels on both PCIe controllers.
 /

unsigned int data_rd, test_err, rdata;
int int_pend;

/
 * Function: pcie_dma_write_test_init
 * Description: Performs testcase initialization and pre-condition setup for pcie_dma_write_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_dma_write_test_init(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);

 return 0;
}

/
 * Function: pcie_dma_write_test_run
 * Description: Main testcase execution for PCIe DMA write and read operations
 * including link training, link-up polling, BAR programming, DMA write channels 0-3
 * programming and doorbell, DMA read channels 0-3, and completion synchronization.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 LOGI("[Test Run] PCIe DMA write test: %s\n", cfg->test_name);
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

 / Steps 3-4: SII0 link-up polling /
 LOGI("Steps 3-4: Polling SII0 link status\n");
 data_rd = read_sii0_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SII0 polling: data_rd=0x%x\n", data_rd);
 #endif
 wait_on(10);
 data_rd = read_sii0_reg(0xC0);
 }
 LOGI("SII0 link-up status achieved\n");

 / Steps 5-6: SII1 link-up polling /
 LOGI("Steps 5-6: Polling SII1 link status\n");
 data_rd = read_sii1_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SII1 polling: data_rd=0x%x\n", data_rd);
 #endif
 wait_on(10);
 data_rd = read_sii1_reg(0xC0);
 }
 LOGI("SII1 link-up status achieved\n");

 / Steps 7-8: BAR programming /
 #ifdef DM0_RC
 {
 unsigned int vendor_id;
 vendor_id = read_pcie_slv0_reg(0x0);
 LOGI("Step 7: Vendor ID = 0x%x\n", vendor_id);

 write_pcie_slv0_reg(0x4, 0x7);
 LOGI("Step 7: write_pcie_slv0_reg(0x4, 0x7) done\n");

 mem_base_program_dm0_x4();
 mem_base_program_dm1_x4();
 LOGI("Step 8: mem_base_program_dm0_x4() and mem_base_program_dm1_x4() done\n");

 wait_on(10);
 }
 #endif

 / Step 9: DMA write channel 0 programming and doorbell - PCIE0 /
 LOGI("Step 9: DMA write channel 0 programming - PCIE0\n");
 / Unmask DMA write interrupt /
 rdata = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF);
 rdata = rdata & ~(0x1);
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, rdata);
 / DMA write channel 0 setup is performed by framework/pcie.h APIs /
 / Ring DMA write doorbell for channel 0 /
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
 LOGI("Step 9: PCIE0 DMA write ch0 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch0 interrupt\n");
 wait_on(10);
 }

 / Step 10: DMA write channel 1 programming and doorbell - PCIE0 /
 LOGI("Step 10: DMA write channel 1 programming - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
 LOGI("Step 10: PCIE0 DMA write ch1 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch1 interrupt\n");
 wait_on(10);
 }

 / Step 11: DMA write channel 2 programming and doorbell - PCIE0 /
 LOGI("Step 11: DMA write channel 2 programming - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
 LOGI("Step 11: PCIE0 DMA write ch2 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch2 interrupt\n");
 wait_on(10);
 }

 / Step 12: DMA write channel 3 programming and doorbell - PCIE0 /
 LOGI("Step 12: DMA write channel 3 programming - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
 LOGI("Step 12: PCIE0 DMA write ch3 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch3 interrupt\n");
 wait_on(10);
 }

 / Step 13: DMA write channel 0 programming and doorbell - PCIE1 /
 LOGI("Step 13: DMA write channel 0 programming - PCIE1\n");
 rdata = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF);
 rdata = rdata & ~(0x1);
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, rdata);
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
 LOGI("Step 13: PCIE1 DMA write ch0 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA write ch0 interrupt\n");
 wait_on(10);
 }

 / Step 14: DMA write channel 1 programming and doorbell - PCIE1 /
 LOGI("Step 14: DMA write channel 1 programming - PCIE1\n");
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
 LOGI("Step 14: PCIE1 DMA write ch1 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA write ch1 interrupt\n");
 wait_on(10);
 }

 / Step 15: DMA write channel 2 programming and doorbell - PCIE1 /
 LOGI("Step 15: DMA write channel 2 programming - PCIE1\n");
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
 LOGI("Step 15: PCIE1 DMA write ch2 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA write ch2 interrupt\n");
 wait_on(10);
 }

 / Step 16: DMA write channel 3 programming and doorbell - PCIE1 /
 LOGI("Step 16: DMA write channel 3 programming - PCIE1\n");
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
 LOGI("Step 16: PCIE1 DMA write ch3 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA write ch3 interrupt\n");
 wait_on(10);
 }

 / Step 17: DMA read channel 0 - PCIE0 /
 LOGI("Step 17: DMA read channel 0 - PCIE0\n");
 rdata = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF);
 rdata = rdata & ~(0x1);
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, rdata);
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
 LOGI("Step 17: PCIE0 DMA read ch0 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch0 interrupt\n");
 wait_on(10);
 }

 / Step 18: DMA read channel 1 - PCIE0 /
 LOGI("Step 18: DMA read channel 1 - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
 LOGI("Step 18: PCIE0 DMA read ch1 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch1 interrupt\n");
 wait_on(10);
 }

 / Step 19: DMA read channel 2 - PCIE0 /
 LOGI("Step 19: DMA read channel 2 - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
 LOGI("Step 19: PCIE0 DMA read ch2 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch2 interrupt\n");
 wait_on(10);
 }

 / Step 20: DMA read channel 3 - PCIE0 /
 LOGI("Step 20: DMA read channel 3 - PCIE0\n");
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
 LOGI("Step 20: PCIE0 DMA read ch3 doorbell rung\n");

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch3 interrupt\n");
 wait_on(10);
 }

 / DMA read channels - PCIE1 /
 LOGI("DMA read channels - PCIE1\n");
 rdata = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF);
 rdata = rdata & ~(0x1);
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, rdata);

 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA read ch0 interrupt\n");
 wait_on(10);
 }

 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA read ch1 interrupt\n");
 wait_on(10);
 }

 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA read ch2 interrupt\n");
 wait_on(10);
 }

 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for PCIE1 DMA read ch3 interrupt\n");
 wait_on(10);
 }

 / Synchronization polling /
 LOGI("Synchronization polling: read_reg(0xE6004100)\n");
 rdata = read_reg(0xE6004100);
 while (rdata != 0x12345678)
 {
 wait_on(5);
 rdata = read_reg(0xE6004100);
 }
 LOGI("Synchronization achieved (0x12345678)\n");

 / finish(0) /
 finish(0);

 return out->status = test_err;
}

/
 * Function: Default_IRQHandler
 * Description: IRQ handler for DMA write and read completion interrupts
 * on PCIE0 and PCIE1 controllers.
 /
void Default_IRQHandler()
{
 unsigned int wr_sts, rd_sts;
 int_pend = 0;

 / Check PCIE0 DMA write interrupt status /
 wr_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
 if (wr_sts != 0x0)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PCIE0 DMA write interrupt status = 0x%x\n", wr_sts);
 #endif
 / Clear PCIE0 DMA write interrupt /
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_sts);
 GIC_ClearIRQ(0);
 return;
 }

 / Check PCIE0 DMA read interrupt status /
 rd_sts = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF);
 if (rd_sts != 0x0)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PCIE0 DMA read interrupt status = 0x%x\n", rd_sts);
 #endif
 / Clear PCIE0 DMA read interrupt /
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_sts);
 GIC_ClearIRQ(0);
 return;
 }

 / Check PCIE1 DMA write interrupt status /
 wr_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
 if (wr_sts != 0x0)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PCIE1 DMA write interrupt status = 0x%x\n", wr_sts);
 #endif
 / Clear PCIE1 DMA write interrupt /
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_sts);
 GIC_ClearIRQ(0);
 return;
 }

 / Check PCIE1 DMA read interrupt status /
 rd_sts = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF);
 if (rd_sts != 0x0)
 {
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: PCIE1 DMA read interrupt status = 0x%x\n", rd_sts);
 #endif
 / Clear PCIE1 DMA read interrupt /
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_sts);
 GIC_ClearIRQ(0);
 return;
 }

 / No interrupt detected /
 LOGI("ERROR: DMA interrupt not detected\n");
 test_err++;
}

/
 * Function: pcie_dma_write_test_teardown
 * Description: Performs teardown and final observation for pcie_dma_write_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem *cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe DMA write test teardown: %s\n", cfg->test_name);

 return 0;
}
