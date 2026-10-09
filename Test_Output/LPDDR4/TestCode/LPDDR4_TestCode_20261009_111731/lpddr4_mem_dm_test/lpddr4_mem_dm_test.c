// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

// Global variables as per Meta
unsigned long int port0_addr;
unsigned long int port1_addr;

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Initializes LPDDR4 DM test: configures port addresses,
 * controller/PHY base, bus width, speed grade, DBI/DM settings,
 * and performs LPDDR4 training.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure.
 */
int lpddr4_mem_dm_test_init(const TestsItem cfg)
{
 (void)cfg;

 // Step 1: gpv_programming() is a prohibited DV construct
 // MANUAL_REVIEW: gpv_programming() was called here in DV. Provide PSV equivalent if needed.
 LOGT("MANUAL_REVIEW: gpv_programming() DV call omitted - provide PSV equivalent if required");

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

 LOGT("LPDDR4 DM test init: port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)port0_addr, (unsigned long)port1_addr);
 LOGT("LPDDR4 DM test init: ctl_base=0x%lx phy_base=0x%lx",
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

 // Step 8: Execute LPDDR4 memory training
 LOGT("Calling lpddr4_training()");
 lpddr4_training();
 LOGT("lpddr4_training() completed");

 return 0;
}

/
 * Function: lpddr4_mem_dm_test_run
 * Description: Executes LPDDR4 DM test: writes deterministic 64-bit data
 * patterns to port0 (conditionally), reads back from port0 and
 * port1, and validates data integrity.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_mem_dm_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int index;
 unsigned long int offset;
 unsigned long int read_data;
 unsigned int err0;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 DM test output pointer is NULL");
 return -1;
 }

 out->status = 0;

 // Step 2: Clear error counter
 err0 = 0;

 // Step 9: Set memory access offset
 offset = 0x1000;
 LOGT("offset = 0x%lx", (unsigned long)offset);

 // Step 10: Conditionally write 10 deterministic 64-bit data values to port0
 // Step 11: Conditionally read back from port0 and verify
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

 // Port0 Write Loop
 LOGT("Port0 Write Loop: writing 10 deterministic 64-bit values");
 for (index = 0; index < 10; index++)
 {
 exp_data_array[index] = dm_test_traffic_data[index];
 write_reg64(port0_addr + ((unsigned long)index * offset), exp_data_array[index]);
 LOGT("Port0 Write: index=%lu addr=0x%lx data=0x%lx",
 (unsigned long)index,
 (unsigned long)(port0_addr + ((unsigned long)index * offset)),
 (unsigned long)exp_data_array[index]);
 }

 // Port0 Read-back Verification Loop
 LOGT("Port0 Read-back Verification Loop");
 for (index = 0; index < 10; index++)
 {
 read_reg64(port0_addr + ((unsigned long)index * offset), &read_data);
 if (read_data != exp_data_array[index])
 {
 LOGE("ERROR_0: port0 index = %lu, exp_data = 0x%lx, actual_data = 0x%lx",
 (unsigned long)index,
 (unsigned long)exp_data_array[index],
 (unsigned long)read_data);
 err0++;
 }
 }

#endif

 // Step 12: Port1 Read-back Verification Loop (unconditional)
 LOGT("Port1 Read-back Verification Loop");
 for (index = 0; index < 10; index++)
 {
 read_reg64(port1_addr + ((unsigned long)index * offset), &read_data);
 if (read_data != exp_data_array[index])
 {
 LOGE("ERROR_0: port1 index = %lu, exp_data = 0x%lx, actual_data = 0x%lx",
 (unsigned long)index,
 (unsigned long)exp_data_array[index],
 (unsigned long)read_data);
 err0++;
 }
 }

 // Final status
 out->status = (err0 == 0) ? 0 : -1;

 LOGT("Run complete: %s err0=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 err0);

 return out->status;
}

/
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Teardown for LPDDR4 DM test. No additional cleanup required.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 DM test teardown: no additional cleanup required");
 return 0;
}
