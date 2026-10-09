// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_2bit_data_modify_test.h"
#include "test_define.inc"

/* Global variables as per Meta Test Description /
unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int int_expected;
unsigned int eccstat;
unsigned int count;
unsigned int region;
unsigned int ecc_region_map;
unsigned int sbr_init;
unsigned int err0;
unsigned int slverr_taken;
unsigned int num_slverr_taken;
unsigned int int_pend;
unsigned int int_pend1;

/ Controller and PHY base addresses /
unsigned long int ctl_base;
unsigned long int phy_base;

/ Training configuration globals /
unsigned int bus_width;
unsigned int SG;
unsigned int DBI_EN;
unsigned int DM_EN;
unsigned int ECC_EN;

/
 * Function: memory_init_scrb
 * Description: Initialize memory scrubbing for a given address region.
 * Partially visible from source; remaining steps marked MANUAL_REVIEW.
 * Parameters:
 * address - base address for the region to scrub
 * Returns:
 * void
 /
static void memory_init_scrb(unsigned long int address)
{
 unsigned long int i;
 unsigned int rd_data;

 (void)address;
 (void)i;
 (void)rd_data;

 / Step 1: sbr_init = 1 /
 sbr_init = 1;

 / Step 1 comment: ECCCFG1.ecc_region_parity_lock to 1 /
 / Programmed before task call /

 / Step 2: Disable ports /
 write_reg(ctl_base + 0x00000490, 0x00000000); / UMCTL2_MP.PCTRL_0 /
 write_reg(ctl_base + 0x00000540, 0x00000000); / UMCTL2_MP.PCTRL_1 /

 / MANUAL_REVIEW: Remaining memory_init_scrb steps are not fully visible /
 / in retrieved source context. Complete the scrubber register programming /
 / sequence (SBR configuration, scrub start, polling for SBR done interrupt, /
 / re-enabling ports, etc.) based on the full DV source. /
}

/
 * Function: Default_IRQHandler
 * Description: IRQ handler for ECC uncorrectable error interrupts.
 * Validates ECCSTAT bit 16, clears error via ECCCTL bit 1,
 * clears SII interrupt, and clears GIC interrupt.
 * Parameters:
 * none
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 / Step 1: Check if interrupt is expected /
 if (!int_expected) {
 LOGE("ERROR0: Unexpected interrupt");
 err0++;
 int_pend = 1;
 return;
 }

 / Step 2: Read ECCSTAT /
 eccstat = read_reg(ctl_base + 0x00000078); / UMCTL2_REGS.ECCSTAT /

 / Step 3: Check bit 16 for 2-bit uncorrectable error /
 if ((eccstat & 0x10000) == 0x10000) {
 / Clear uncorrectable ECC error via ECCCTL bit 1 /
 write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000002); / UMCTL2_REGS.ECCCTL /
 / Clear SII interrupt /
 write_reg(ctl_base + 0x00008194, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK);
 LOGT("Interrupt cleared");
 int_pend1 = 0;
 } else {
 LOGE("ERROR: Unexpected interrupt");
 err0++;
 }

 / Step 4: Write to SYSREG interrupt reset control /
 write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);

 / Step 5: Clear GIC IRQ /
 GIC_ClearIRQ(CTL_INT_NO);
}

/
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_init
 * Description: Initializes the LPDDR4 inline ECC 2-bit uncorrectable error
 * data modify test. Configures port addresses, controller/PHY bases,
 * training parameters, performs training, configures ECC registers,
 * and scrubs ECC-enabled memory regions.
 * Parameters:
 * cfg - pointer to test configuration structure
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_init(const TestsItem cfg)
{
 (void)cfg;

 LOGT("LPDDR4 inline ECC 2-bit data modify test init start");

 / Step 1: gpv_programming() - DV infrastructure, prohibited in PSV /
 / MANUAL_REVIEW: gpv_programming() is a prohibited DV construct. /
 / If PSV equivalent GPV programming is required, add it here. /
 LOGD("DV gpv_programming() skipped in PSV");

 / Step 2: err0 = 0 /
 err0 = 0;

 / Step 3: Conditional port/base address configuration /
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

 LOGT("port0_addr=0x%lx port1_addr=0x%lx ctl_base=0x%lx phy_base=0x%lx
" (unsigned long)port0_addr,
 (unsigned long)port1_addr,
 (unsigned long)ctl_base,
 (unsigned long)phy_base);

 / Step 4: bus_width = 0 (Full Bus) /
 bus_width = 0; / 0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus /

 / Step 5: Speed grade selection /
#if defined(SG2667)
 SG = 2667;
#elif defined(SG2133)
 SG = 2133;
#else
 SG = 3200;
#endif

 / Step 6: DBI_EN = 0 /
 DBI_EN = 0;

 / Step 7: DM_EN = 1 /
 DM_EN = 1;

 / Step 8: ECC_EN = 1 /
 ECC_EN = 1;

 LOGT("bus_width=%u SG=%u DBI_EN=%u DM_EN=%u ECC_EN=%u
" bus_width, SG, DBI_EN, DM_EN, ECC_EN);

 / Step 9: Call lpddr4_training() /
 lpddr4_training();

 / Step 10: wait_on(1000) - converted to deterministic busy-wait /
 for (volatile int d = 0; d < 10000; d++);

 / Step 11: Call training_done() /
 training_done();

 / Step 12: wait_on(1000) - converted to deterministic busy-wait /
 for (volatile int d = 0; d < 10000; d++);

 / Step 13: Clear ECCCTL register /
 write_reg(ctl_base + 0x0000007c, 0x00000000); / UMCTL2_REGS.ECCCTL /

 / Step 14: Enable SBR done interrupt /
 write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK); / enabling intr /

 / Step 15: ecc_region_map = 0x55 /
 ecc_region_map = 0x55;

 / Step 16: Unlock SWCTL /
 write_reg(ctl_base + 0x00000320, 0x00000000); / UMCTL2_REGS.SWCTL /

 / Step 17: Configure ECCCFG0 /
 write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4); / UMCTL2_REGS.ECCCFG0 /

 / Step 18: Configure ECCCFG1 /
 write_reg(ctl_base + 0x00000074, 0x00000330); / UMCTL2_REGS.ECCCFG1 /

 / Step 19: Lock SWCTL /
 write_reg(ctl_base + 0x00000320, 0x00000001); / UMCTL2_REGS.SWCTL /

 / Step 20: int_expected = 0 /
 int_expected = 0;

 / Step 21: offset = 0x80000000 /
 / offset is set in _run, but init sets int_expected /

 / Step 22: Scrub ECC-enabled memory regions /
 {
 unsigned long int offset = 0x80000000UL;
 for (region = 0; region < 7; region++) {
 if ((ecc_region_map >> region) & 0x1) {
 memory_init_scrb(offset * region);
 }
 }
 }

 LOGT("LPDDR4 inline ECC 2-bit data modify test init complete");

 return 0;
}

/
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_run
 * Description: Main test execution. Writes deterministic 64-bit data patterns
 * to 7 memory regions via port1, enables ECC uncorrected error
 * interrupts, reads back data to trigger ECC checking, and polls
 * ECCSTAT for 2-bit uncorrectable error detection.
 * Parameters:
 * cfg - pointer to test configuration structure
 * out - pointer to test output structure
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int offset;
 unsigned long int read_data;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("LPDDR4 inline ECC 2-bit data modify test run start");

 / Step 21 (continued): offset = 0x80000000 /
 offset = 0x80000000UL;

 / Step 23: Write 64-bit data patterns to all 7 regions via port1 /
 / MANUAL_REVIEW: Original DV uses rand() which is prohibited in PSV. /
 / Deterministic replacement data values are not supplied by Meta. /
 / Using deterministic pattern based on region index as placeholder. /
 for (region = 0; region < 7; region++) {
 / MANUAL_REVIEW: exp_data_array[region] = rand() is prohibited. /
 / MANUAL_REVIEW: exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32) is prohibited. /
 / Using deterministic pattern instead of rand(). Supply actual test data if available. /
 exp_data_array[region] = (unsigned long int)(0xDEAD0000UL | (unsigned long int)region);
 exp_data_array[region] = exp_data_array[region] | ((unsigned long int)(0xBEEF0000UL | (unsigned long int)region) << 32);
 write_reg64(port1_addr + ((unsigned long int)region * offset), exp_data_array[region]);
 }

 / Step 24: Enable ECC uncorrected error interrupt and SBR done interrupt /
 write_reg(ctl_base + 0x0000819C, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK); / enabling intr /

 / Step 25: Enable uncorrected ECC error interrupt in ECCCTL (bit 9) /
 write_reg(ctl_base + 0x0000007c, 1 << 9); / UMCTL2_REGS.ECCCTL Interrupt enable /

 / Step 26: For each of 7 regions: read back data to trigger ECC check /
 for (region = 0; region < 7; region++) {
 slverr_taken = 0;
 LOGT("region = %u", region);
 int_expected = (ecc_region_map >> region) & 0x1;
 int_pend1 = 1;
 read_reg64(port1_addr + ((unsigned long int)region * offset), &read_data);
 }

 / Step 27: ECCSTAT polling /
 read_reg64(port1_addr, &read_data);
 eccstat = read_reg(ctl_base + 0x00000078); / UMCTL2_REGS.ECCSTAT /
 while (!(eccstat & (1 << 16))) {
 eccstat = read_reg(ctl_base + 0x00000078); / UMCTL2_REGS.ECCSTAT /
 }

 LOGT("ECCSTAT polling complete, bit 16 set: eccstat=0x%x", eccstat);

 / Step 28: finish(err0) - converted to PSV status /
 out->status = (err0 == 0) ? 0 : -1;

 LOGT("Run complete: %s err0=%u num_slverr_taken=%u
" (out->status == 0) ? "PASS" : "FAIL",
 err0,
 num_slverr_taken);

 return out->status;
}

/
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_teardown
 * Description: Teardown for the LPDDR4 inline ECC 2-bit data modify test.
 * Returns final test status based on error counter.
 * Parameters:
 * cfg - pointer to test configuration structure
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 inline ECC 2-bit data modify test teardown: err0=%u", err0);
 return (err0 == 0) ? 0 : -1;
}
