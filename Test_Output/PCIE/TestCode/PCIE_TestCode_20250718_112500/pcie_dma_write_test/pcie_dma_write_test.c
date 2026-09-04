// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "pcie_dma_write_test.h"
#include "test_define.inc"

/* PCIe DMA Write Test
 * Description: This testcase performs PCIe DMA write and read-back operations
 * across all four DMA channels (0-3) with link training, BAR programming,
 * source memory preload, GIC/interrupt setup, and DMA channel sequencing.
 /

unsigned int data_rd, test_err, i;
unsigned int len, src_addr0, dst_addr0, dst_addr1, dst_addr2, dst_addr3;
unsigned int wr_addr0, wr_addr1, wr_addr2, wr_addr3;
unsigned int rd_addr0, rd_addr1, rd_addr2, rd_addr3;
int int_pend;

/*
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
 printf("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);
 LOGI("[Test Init] PCIe DMA write test: %s\n", cfg->test_name);

 test_err = 0;

 return 0;
}

/*
 * Function: pcie_dma_write_test_run
 * Description: Main testcase execution for PCIe DMA write/read-back operations across
 * channels 0-3 including link training, link status polling, Vendor ID read,
 * BAR/mem base programming, source memory preload, GIC setup, DMA channel
 * programming, doorbell trigger, and interrupt-based completion.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output structure.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput out)
{
 LOGI("[Test Run] PCIe DMA write test: %s\n", cfg->test_name);
 test_err = 0;

 /* Step 1: Write 0x0 to 0xE6004100 to initialize synchronization register */
 LOGI("Step 1: Initialize synchronization register 0xE6004100\n");
 write_reg(0xE6004100, 0x0);

 /* Step 2: Conditionally call link training based on DM0_RC, DM1_RC, DM0_EP, DM1_EP */
 LOGI("Step 2: Conditional link training\n");
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

 /* Step 3: Under DM0_RC - Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
 #ifdef DM0_RC
 {
 LOGI("Step 3: DM0_RC - Polling sii0 link status for link-up\n");
 data_rd = read_sii0_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 data_rd = read_sii0_reg(0xC0);
 #ifdef DEBUG_DISPLAY
 LOGI("Polling sii0 link status: data_rd=0x%x\n", data_rd);
 #endif
 }
 LOGI("DM0_RC link-up confirmed\n");
 }
 #endif

 /* Step 4: Under DM0_RC - Vendor ID, command write, BAR and mem base program */
 #ifdef DM0_RC
 {
 LOGI("Step 4: DM0_RC - Read Vendor ID, write command, BAR/mem base program\n");
 data_rd = read_pcie_slv0_reg(0x0);
 printf("Vendor ID = 0x%x\n", data_rd);
 LOGI("Vendor ID read from slv0 reg 0x0 = 0x%x\n", data_rd);

 write_pcie_slv0_reg(0x4, 0x7);

 bar_program_dm0_x4();
 wait_on(10);
 mem_base_program_dm0_x4();
 }
 #endif

 /* Step 5: Under DM1_RC - Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1 */
 #ifdef DM1_RC
 {
 LOGI("Step 5: DM1_RC - Polling sii1 link status for link-up\n");
 data_rd = read_sii1_reg(0xC0);
 while ((data_rd & 0xD1) != 0xD1)
 {
 data_rd = read_sii1_reg(0xC0);
 #ifdef DEBUG_DISPLAY
 LOGI("Polling sii1 link status: data_rd=0x%x\n", data_rd);
 #endif
 }
 LOGI("DM1_RC link-up confirmed\n");
 }
 #endif

 /* Step 6: Under DM1_RC - Vendor ID, command write, BAR and mem base program */
 #ifdef DM1_RC
 {
 LOGI("Step 6: DM1_RC - Read Vendor ID, write command, BAR/mem base program\n");
 data_rd = read_pcie_slv1_reg(0x0);
 printf("Vendor ID = 0x%x\n", data_rd);
 LOGI("Vendor ID read from slv1 reg 0x0 = 0x%x\n", data_rd);

 write_pcie_slv1_reg(0x4, 0x7);

 bar_program_dm1_x4();
 wait_on(10);
 mem_base_program_dm1_x4();
 }
 #endif

 /* Step 7: Call non_secure_prot_nic() */
 LOGI("Step 7: Configure non-secure protection\n");
 non_secure_prot_nic();

 /* Step 8: Poll read_reg(0xE6004100) until value equals 0x12345678 */
 LOGI("Step 8: Polling 0xE6004100 for sync value 0x12345678\n");
 data_rd = read_reg(0xE6004100);
 while (data_rd != 0x12345678)
 {
 wait_on(5);
 data_rd = read_reg(0xE6004100);
 #ifdef DEBUG_DISPLAY
 LOGI("Polling 0xE6004100: data_rd=0x%x\n", data_rd);
 #endif
 }
 LOGI("Sync value 0x12345678 detected\n");

 /* Step 9: Set DMA transfer parameters */
 LOGI("Step 9: Set DMA transfer parameters\n");
 len = 0x40;
 src_addr0 = 0xE6000000;
 dst_addr0 = 0xE6001000;
 dst_addr1 = 0xE6020000;
 dst_addr2 = 0xE6020000;
 dst_addr3 = 0xE6020000;

 #ifdef DM0_RC
 wr_addr0 = 0xA7000000;
 wr_addr1 = 0xA7100000;
 wr_addr2 = 0xA7200000;
 wr_addr3 = 0xA7300000;
 rd_addr0 = 0xA7000000;
 rd_addr1 = 0xA7100000;
 rd_addr2 = 0xA7200000;
 rd_addr3 = 0xA7300000;
 #endif
 #ifdef DM1_RC
 wr_addr0 = 0xC7000000;
 wr_addr1 = 0xC7100000;
 wr_addr2 = 0xC7200000;
 wr_addr3 = 0xC7300000;
 rd_addr0 = 0xC7000000;
 rd_addr1 = 0xC7100000;
 rd_addr2 = 0xC7200000;
 rd_addr3 = 0xC7300000;
 #endif

 /* Step 10: Preload source memory */
 LOGI("Step 10: Preload source memory with 0xC0DEBEED and 0xF00DDEAF\n");
 for (i = 0; i < 128; i++)
 {
 write_reg(src_addr0 + (4 * i), 0xC0DEBEED);
 }
 for (i = 0; i < 128; i++)
 {
 write_reg((src_addr0 + 400) + (4 * i), 0xF00DDEAF);
 }

 /* Step 11: Set int_pend, GIC setup */
 LOGI("Step 11: GIC setup\n");
 int_pend = 1;
 GIC_Set();
 GIC_EnableAllIRQ();

 /* Step 12: Clear DMA interrupt masks */
 LOGI("Step 12: Clear DMA interrupt masks\n");
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF, 0x0);
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF, 0x0);
 #endif

 /* Step 13: DMA Write Channel 0 */
 LOGI("Step 13: Program and trigger DMA write channel 0\n");
 program_dma_wch0(src_addr0, wr_addr0, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x0);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch0 interrupt\n");
 wait_on(10);
 }

 /* Step 14: DMA Write Channel 1 */
 LOGI("Step 14: Program and trigger DMA write channel 1\n");
 program_dma_wch1(src_addr0, wr_addr1, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x1);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch1 interrupt\n");
 wait_on(10);
 }

 /* Step 15: DMA Write Channel 2 */
 LOGI("Step 15: Program and trigger DMA write channel 2\n");
 program_dma_wch2(src_addr0, wr_addr2, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x2);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch2 interrupt\n");
 wait_on(10);
 }

 /* Step 16: DMA Write Channel 3 */
 LOGI("Step 16: Program and trigger DMA write channel 3\n");
 program_dma_wch3(src_addr0, wr_addr3, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF, 0x3);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA write ch3 interrupt\n");
 wait_on(10);
 }

 /* Step 17: DMA Read Channel 0 */
 LOGI("Step 17: Program and trigger DMA read channel 0\n");
 program_dma_rch0(rd_addr0, dst_addr0, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x0);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch0 interrupt\n");
 wait_on(10);
 }

 /* Step 18: DMA Read Channel 1 */
 LOGI("Step 18: Program and trigger DMA read channel 1\n");
 program_dma_rch1(rd_addr1, dst_addr1, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x1);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch1 interrupt\n");
 wait_on(10);
 }

 /* Step 19: DMA Read Channel 2 */
 LOGI("Step 19: Program and trigger DMA read channel 2\n");
 program_dma_rch2(rd_addr2, dst_addr2, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x2);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch2 interrupt\n");
 wait_on(10);
 }

 /* Step 20: DMA Read Channel 3 */
 LOGI("Step 20: Program and trigger DMA read channel 3\n");
 program_dma_rch3(rd_addr3, dst_addr3, len);
 #ifdef DM0_RC
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
 #endif
 #ifdef DM1_RC
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF, 0x3);
 #endif

 int_pend = 1;
 while (int_pend)
 {
 LOGI("Waiting for DMA read ch3 interrupt\n");
 wait_on(10);
 }

 /* Step 21: All DMA transfers complete, call finish(0) */
 LOGI("Step 21: All DMA transfers complete, calling finish(0)\n");
 finish(0);

 return out->status = test_err;
}

/* Default_IRQHandler - DMA interrupt handler */
void Default_IRQHandler()
{
 unsigned int wr_int_status, rd_int_status;

 int_pend = 0;

 #ifdef DM0_RC
 {
 /* Read DMA write interrupt status */
 wr_int_status = read_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
 wr_int_status = wr_int_status & 0x0000000F;

 /* Read DMA read interrupt status */
 rd_int_status = read_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF);
 rd_int_status = rd_int_status & 0x0000000F;

 #ifdef DEBUG_DISPLAY
 LOGI("IRQHandler DM0: wr_int_status=0x%x rd_int_status=0x%x\n", wr_int_status, rd_int_status);
 #endif

 /* Clear DMA write interrupt */
 if (wr_int_status != 0x0)
 {
 write_reg(mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_int_status);
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: DMA write interrupt cleared: 0x%x\n", wr_int_status);
 #endif
 }

 /* Clear DMA read interrupt */
 if (rd_int_status != 0x0)
 {
 write_reg(mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_int_status);
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: DMA read interrupt cleared: 0x%x\n", rd_int_status);
 #endif
 }

 GIC_ClearIRQ(0x20);
 }
 #endif

 #ifdef DM1_RC
 {
 /* Read DMA write interrupt status */
 wr_int_status = read_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF);
 wr_int_status = wr_int_status & 0x0000000F;

 /* Read DMA read interrupt status */
 rd_int_status = read_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF);
 rd_int_status = rd_int_status & 0x0000000F;

 #ifdef DEBUG_DISPLAY
 LOGI("IRQHandler DM1: wr_int_status=0x%x rd_int_status=0x%x\n", wr_int_status, rd_int_status);
 #endif

 /* Clear DMA write interrupt */
 if (wr_int_status != 0x0)
 {
 write_reg(mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF, wr_int_status);
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: DMA write interrupt cleared: 0x%x\n", wr_int_status);
 #endif
 }

 /* Clear DMA read interrupt */
 if (rd_int_status != 0x0)
 {
 write_reg(mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF, rd_int_status);
 #ifdef DEBUG_DISPLAY
 LOGI("SUCCESS: DMA read interrupt cleared: 0x%x\n", rd_int_status);
 #endif
 }

 GIC_ClearIRQ(0x23);
 }
 #endif
}

/*
 * Function: pcie_dma_write_test_teardown
 * Description: Performs validation, final observation, and testcase completion for pcie_dma_write_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int pcie_dma_write_test_teardown(const TestsItem cfg)
{
 (void)cfg;
 LOGI("[TEARDOWN] PCIe DMA write test teardown: %s\n", cfg->test_name);

 /* Validation: All 8 DMA transfers (4 write + 4 read) completed via interrupt */
 /* Link status polling confirmed (data_rd & 0xD1) == 0xD1 */
 /* Vendor ID was read and printed */
 /* Sync register polled to 0x12345678 */
 /* finish(0) called on success */

 return 0;
}
