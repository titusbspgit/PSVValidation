// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_1bit_data_modify_test.h"
#include "test_define.inc"

// Global variables as per Meta
unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int int_expected;
unsigned int eccstat;
unsigned int count;
unsigned int region;
unsigned int ecc_region_map;
unsigned int sbr_init;
unsigned int err0;
unsigned int int_pend1;
unsigned int int_pend;
unsigned int num_slverr_taken;

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for ECC corrected error interrupts.
 * Checks if interrupt is expected, reads ECCSTAT,
 * clears correctable ECC error, clears interrupt.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 // Step 1: Check if interrupt is expected
 if (!int_expected) {
 LOGE("ERROR0: Unexpected interrupt");
 err0++;
 int_pend = 1;
 return;
 }

 // Step 2: Read ECCSTAT
 eccstat = read_reg(ctl_base + 0x00000078);
 LOGT("Default_IRQHandler: ECCSTAT = 0x%x", eccstat);

 // Step 3: Check bit 8 for 1-bit correctable ECC error
 if ((eccstat & 0x100) == 0x100) {
 // Clear correctable ECC error via ECCCTL
 write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000001);
 // Write to SII interrupt status register
 write_reg(ctl_base + 0x00008194, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);
 LOGT("Interrupt cleared");
 int_pend1 = 0;
 }

 // Step 4: Write to SYSREG_INTR_RSTCR_ADDR
 write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);

 // Step 5: Clear GIC IRQ
 GIC_ClearIRQ(CTL_INT_NO);
}

/
 * Function: memory_init_scrb
 * Description: Initializes memory scrubbing for a given address region.
 * Disables port controllers before scrubbing.
 * Parameters:
 * address - base address of the memory region to scrub
 * Returns:
 * void
 /
void memory_init_scrb(unsigned long int address)
{
 unsigned long int i;
 unsigned int rd_data;

 (void)i;
 (void)rd_data;
 (void)address;

 // Step 1: Set sbr_init flag
 sbr_init = 1;
 LOGT("memory_init_scrb: sbr_init = 1, address = 0x%lx", (unsigned long)address);

 // Step 2: Disable PCTRL_0
 write_reg(ctl_base + 0x00000490, 0x00000000);
 LOGT("memory_init_scrb: PCTRL_0 disabled");

 // Step 3: Disable PCTRL_1
 write_reg(ctl_base + 0x00000540, 0x00000000);
 LOGT("memory_init_scrb: PCTRL_1 disabled");

 // MANUAL_REVIEW: Remaining scrubber register operations (SBRCTL, SBRWDATA0,
 // SBRSTART, SBRSTAT, port re-enable, SBR done wait) are not visible in
 // retrieved source context. Implement the complete scrubber sequence here.
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_init
 * Description: Initializes LPDDR4 inline ECC 1-bit data modify test:
 * configures port addresses, controller/PHY base, bus width,
 * speed grade, DBI/DM/ECC settings, performs training,
 * configures ECC registers, and scrubs ECC-enabled regions.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_init(const TestsItem cfg)
{
 (void)cfg;

 // Step 1: gpv_programming() is a prohibited DV construct
 // MANUAL_REVIEW: gpv_programming() was called here in DV. Provide PSV equivalent if needed.
 LOGT("MANUAL_REVIEW: gpv_programming() DV call omitted - provide PSV equivalent if required");

 // Step 2: Clear error counter
 err0 = 0;
 LOGT("err0 = 0");

 // Step 3: Configure port addresses, controller base, and PHY base
#if defined(APS_DRAM)
 port0_addr = 0;
 port1_addr = 0x15A0000000UL;
 ctl_base = 0x9EE03000UL;
 phy_base = 0x9F000000UL;
#else
 port0_addr = 0;
 port1_addr = 0x11A0000000UL;
 ctl_base = 0x11D003000UL;
 phy_base = 0x11D500000UL;
#endif
 LOGT("port0_addr=0x%lx port1_addr=0x%lx ctl_base=0x%lx phy_base=0x%lx",
 (unsigned long)port0_addr, (unsigned long)port1_addr,
 (unsigned long)ctl_base, (unsigned long)phy_base);

 // Step 4: Set bus width to Full Bus mode
 bus_width = 0;
 LOGT("bus_width = %u (Full Bus)", bus_width);

 // Step 5: Select speed grade
#if defined(SG2667)
 SG = 2667;
#elif defined(SG2133)
 SG = 2133;
#else
 SG = 3200;
#endif
 LOGT("Speed Grade SG = %u", SG);

 // Step 6: Disable Data Bus Inversion
 DBI_EN = 0;
 LOGT("DBI_EN = %u (disabled)", DBI_EN);

 // Step 7: Enable Data Mask
 DM_EN = 1;
 LOGT("DM_EN = %u (enabled)", DM_EN);

 // Step 8: Enable ECC
 ECC_EN = 1;
 LOGT("ECC_EN = %u (enabled)", ECC_EN);

 // Step 9: Execute LPDDR4 memory training
 LOGT("Calling lpddr4_training()");
 lpddr4_training();
 LOGT("lpddr4_training() completed");

 // Step 10: wait_on(1000) - PSV busy-wait adaptation
 LOGT("Wait after training (wait_on 1000)");
 for (volatile int d = 0; d < 10000; d++);

 // Step 11: Call training_done()
 LOGT("Calling training_done()");
 training_done();
 LOGT("training_done() completed");

 // Step 12: wait_on(1000) - PSV busy-wait adaptation
 LOGT("Wait after training_done (wait_on 1000)");
 for (volatile int d = 0; d < 10000; d++);

 // Step 13: Clear ECCCTL register
 write_reg(ctl_base + 0x0000007c, 0x00000000);
 LOGT("ECCCTL cleared (ctl_base + 0x7c = 0x00000000)");

 // Step 14: Enable SBR done interrupt
 write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK);
 LOGT("SBR done interrupt enabled (ctl_base + 0x819C)");

 // Step 15: Set ecc_region_map
 ecc_region_map = 0x55;
 LOGT("ecc_region_map = 0x%x", ecc_region_map);

 // Step 16: Unlock SWCTL
 write_reg(ctl_base + 0x00000320, 0x00000000);
 LOGT("SWCTL unlocked (ctl_base + 0x320 = 0x00000000)");

 // Step 17: Configure ECCCFG0
 write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4);
 LOGT("ECCCFG0 configured (ctl_base + 0x70 = 0x%x)", (ecc_region_map << 8) | 0xb4);

 // Step 18: Configure ECCCFG1
 write_reg(ctl_base + 0x00000074, 0x00000330);
 LOGT("ECCCFG1 configured (ctl_base + 0x74 = 0x00000330)");

 // Step 19: Lock SWCTL
 write_reg(ctl_base + 0x00000320, 0x00000001);
 LOGT("SWCTL locked (ctl_base + 0x320 = 0x00000001)");

 // Step 20: Set int_expected = 0
 int_expected = 0;

 // Step 21: Set offset (used later in run, but set here per Meta flow)
 // offset is local to run, so int_expected is set globally here

 return 0;
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_run
 * Description: Executes LPDDR4 inline ECC 1-bit data modify test:
 * scrubs ECC-enabled regions, writes deterministic 64-bit data
 * to 7 regions via port1, enables ECC interrupts, reads back
 * data from each region to trigger ECC checking, and validates
 * data integrity.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_inline_ecc_1bit_data_modify_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int offset;
 unsigned long int read_data;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 ECC 1-bit data modify test output pointer is NULL");
 return -1;
 }

 out->status = 0;

 // Step 20 continued: int_expected already set in init
 // Step 21: Set offset
 offset = 0x80000000UL;
 LOGT("offset = 0x%lx", (unsigned long)offset);

 // Local variable init per Meta
 num_slverr_taken = 0;

 // Step 22: Scrub ECC-enabled memory regions
 LOGT("Scrubbing ECC-enabled regions (ecc_region_map=0x%x)", ecc_region_map);
 for (region = 0; region < 7; region++) {
 if ((ecc_region_map >> region) & 0x1) {
 LOGT("Scrubbing region %u at offset 0x%lx",
 region, (unsigned long)(offset * region));
 memory_init_scrb(offset * region);
 }
 }

 // Step 23: Write deterministic 64-bit data to all 7 regions via port1
 // rand() replaced with static deterministic array ecc_1bit_traffic_data[]
 LOGT("Writing deterministic 64-bit data to 7 regions via port1");
 for (region = 0; region < 7; region++) {
 exp_data_array[region] = ecc_1bit_traffic_data[region];
 write_reg64(port1_addr + ((unsigned long int)region * offset), exp_data_array[region]);
 LOGT("Port1 Write: region=%u addr=0x%lx data=0x%lx",
 region,
 (unsigned long)(port1_addr + ((unsigned long int)region * offset)),
 (unsigned long)exp_data_array[region]);
 }

 // Step 24: Enable ECC corrected error and SBR done interrupts
 write_reg(ctl_base + 0x0000819C, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
 LOGT("ECC corrected error + SBR done interrupts enabled (ctl_base + 0x819C)");

 // Step 25: Enable corrected ECC error interrupt in ECCCTL (bit 8)
 write_reg(ctl_base + 0x0000007c, 1 << 8);
 LOGT("ECCCTL interrupt enable bit 8 set (ctl_base + 0x7c = 0x%x)", 1 << 8);

 // Step 26: Read-back verification loop for 7 regions
 LOGT("Read-back verification loop for 7 regions");
 for (region = 0; region < 7; region++) {
 int_expected = (ecc_region_map >> region) & 0x1;
 int_pend1 = 1;
 LOGT("Region %u: int_expected=%u", region, int_expected);

 read_reg64(port1_addr + ((unsigned long int)region * offset), &read_data);
 LOGT("Region %u: read_data=0x%lx", region, (unsigned long)read_data);

 // wait_on(1000) - PSV busy-wait adaptation
 for (volatile int d = 0; d < 10000; d++);

 // Data comparison validation per Meta
 if (exp_data_array[0] != read_data) {
 LOGE("ERROR region 0 no int: exp_data = 0x%lx, actual_data = 0x%lx",
 (unsigned long)exp_data_array[0], (unsigned long)read_data);
 err0++;
 }
 }

 // Step 27: finish(err0) → PSV status
 // finish() is prohibited; use out->status
 out->status = (err0 == 0) ? 0 : -1;

 LOGT("Run complete: %s err0=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 err0);

 return out->status;
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_teardown
 * Description: Teardown for LPDDR4 inline ECC 1-bit data modify test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 inline ECC 1-bit data modify test teardown");
 LOGT("Final error count: err0=%u", err0);

 return (err0 == 0) ? 0 : -1;
}
