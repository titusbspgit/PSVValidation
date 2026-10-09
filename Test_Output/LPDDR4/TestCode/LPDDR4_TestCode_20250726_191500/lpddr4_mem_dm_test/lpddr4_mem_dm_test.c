// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

// Global variables as per Meta Test Description
unsigned long int port0_addr;
unsigned long int port1_addr;

// Deterministic pattern generator to replace prohibited rand()
// MANUAL_REVIEW: Original DV code used rand() to generate 64-bit random data.
// rand() is prohibited in PSV. Deterministic pattern provided below.
// Adjust pattern if specific random seed behavior is required.
static unsigned long int dm_test_deterministic_pattern(unsigned long int index)
{
 // Deterministic 64-bit pattern based on index
 unsigned long int lo = (0xA5A50000UL | (index * 0x1111UL));
 unsigned long int hi = (0x5A5A0000UL | ((index + 1UL) * 0x2222UL));
 return (hi << 32) | (lo & 0xFFFFFFFFUL);
}

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Initialize LPDDR4 DM test - configure port addresses,
 * controller/PHY base, bus width, speed grade, DBI/DM settings,
 * and perform LPDDR4 training.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_init(const TestsItem cfg)
{
 (void)cfg;

 LOGT("LPDDR4 DM test init: starting initialization");

 // Step 1: Call gpv_programming()
 // MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
 // If GPV programming is required in PSV, provide an approved replacement.

 // Step 2: err0 = 0 (handled in _run)

 // Step 3: Conditional on APS_DRAM - configure port addresses, ctl_base, phy_base
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

 LOGT("LPDDR4 DM test: port0_addr=0x%lx port1_addr=0x%lx",
 port0_addr, port1_addr);
 LOGT("LPDDR4 DM test: ctl_base=0x%lx phy_base=0x%lx",
 (unsigned long)ctl_base, (unsigned long)phy_base);

 // Step 4: bus_width = 0 (Full Bus)
 bus_width = 0;
 LOGT("LPDDR4 DM test: bus_width=%d (Full Bus)", bus_width);

 // Step 5: Conditional on SG2667/SG2133 - select speed grade
#if defined(SG2667)
 SG = 2667;
#elif defined(SG2133)
 SG = 2133;
#else
 SG = 3200;
#endif
 LOGT("LPDDR4 DM test: SG=%d", SG);

 // Step 6: Disable Data Bus Inversion
 DBI_EN = 0;
 LOGT("LPDDR4 DM test: DBI_EN=%d (disabled)", DBI_EN);

 // Step 7: Enable Data Mask
 DM_EN = 1;
 LOGT("LPDDR4 DM test: DM_EN=%d (enabled)", DM_EN);

 // Step 8: Call lpddr4_training()
 LOGT("LPDDR4 DM test: calling lpddr4_training()");
 lpddr4_training();

 LOGT("LPDDR4 DM test init: initialization complete");

 return 0;
}

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Execute LPDDR4 DM test - write deterministic 64-bit data
 * patterns to port0, read back from port0 and port1, verify
 * data integrity with Data Mask enabled.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int index;
 unsigned long int offset;
 unsigned long int read_data;
 unsigned int err0;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 DM test: output pointer is NULL");
 return -1;
 }

 out->status = 0;

 // Step 2: Clear error counter
 err0 = 0;

 // Step 9: Set memory access offset
 offset = 0x1000;
 LOGT("LPDDR4 DM test run: offset=0x%lx", offset);

 // Step 10: Conditionally write 10 data values to port0
 // Guarded by XOR-based preprocessor condition
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

 // Port0 Write Loop
 LOGT("LPDDR4 DM test: Port0 write loop - 10 iterations");
 for (index = 0; index < 10; index++)
 {
 // MANUAL_REVIEW: Original DV code used rand() to generate random data.
 // rand() is prohibited in PSV. Using deterministic pattern instead.
 // Original: exp_data_array[index] = rand();
 // Original: exp_data_array[index] = exp_data_array[index] | ((unsigned long)rand() << 32);
 exp_data_array[index] = dm_test_deterministic_pattern(index);
 write_reg64(port0_addr + ((unsigned long)index * offset), exp_data_array[index]);
 }

 // Step 11: Port0 Read-back Verification Loop
 LOGT("LPDDR4 DM test: Port0 read-back verification - 10 iterations");
 for (index = 0; index < 10; index++)
 {
 read_reg64(port0_addr + ((unsigned long)index * offset), &read_data);
 if (read_data != exp_data_array[index])
 {
 LOGE("ERROR_0: port0 index = %lu, exp_data = %lx, actual_data = %lx",
 index, exp_data_array[index], read_data);
 err0++;
 }
 }

#endif

 // Step 12: Port1 Read-back Verification Loop
 LOGT("LPDDR4 DM test: Port1 read-back verification - 10 iterations");
 for (index = 0; index < 10; index++)
 {
 read_reg64(port1_addr + ((unsigned long)index * offset), &read_data);
 if (read_data != exp_data_array[index])
 {
 LOGE("ERROR_0: port1 index = %lu, exp_data = %lx, actual_data = %lx",
 index, exp_data_array[index], read_data);
 err0++;
 }
 }

 // Final status
 out->status = (err0 == 0U) ? 0 : -1;

 LOGT("LPDDR4 DM test run complete: %s err0=%u",
 (out->status == 0) ? "PASS" : "FAIL", err0);

 return out->status;
}

/*
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Teardown for LPDDR4 DM test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 DM test teardown: no additional cleanup required");
 return 0;
}
