// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

// Global variables as per Meta Test Description
unsigned long int port0_addr;
unsigned long int port1_addr;

// Additional global/file-scope variables used across init/run
static unsigned long int ctl_base;
static unsigned long int phy_base;
static unsigned int bus_width;
static unsigned int SG;
static unsigned int DBI_EN;
static unsigned int DM_EN;
static unsigned int err0;

// Meta Arrays: exp_data_array - single authoritative declaration
static unsigned long int exp_data_array[50];

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Initialize LPDDR4 Data Mask test. Configures port addresses,
 *              controller/PHY bases, bus width, speed grade, DBI/DM settings,
 *              and performs LPDDR4 memory training.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_mem_dm_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 Data Mask (DM) test init started");

    // Meta Step 1: Call gpv_programming()
    // MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
    // If a PSV equivalent is required, provide the replacement API.

    // Meta Step 2: err0 = 0
    err0 = 0;

    // Meta Step 3: Conditional on APS_DRAM
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
         port0_addr, port1_addr, ctl_base, phy_base);

    // Meta Step 4: bus_width = 0 (0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus)
    bus_width = 0;

    // Meta Step 5: Conditional on SG2667/SG2133
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    LOGT("bus_width=%u SG=%u", bus_width, SG);

    // Meta Step 6: DBI_EN = 0
    DBI_EN = 0;

    // Meta Step 7: DM_EN = 1
    DM_EN = 1;

    LOGT("DBI_EN=%u DM_EN=%u", DBI_EN, DM_EN);

    // Meta Step 8: Call lpddr4_training()
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    LOGT("LPDDR4 Data Mask (DM) test init completed");

    return 0;
}

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Execute LPDDR4 Data Mask test. Writes deterministic 64-bit
 *              data patterns to port0 (conditional), reads back from port0
 *              and port1, and validates data integrity.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_mem_dm_test_run(
    const TestsItem *cfg,
    TestOutput *out
)
{
    unsigned long int index;
    unsigned long int offset;
    unsigned long int read_data;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("Starting LPDDR4 Data Mask (DM) test run");

    // Meta Step 9: offset = 0x1000
    offset = 0x1000;

    // Meta Step 10: Conditional port0 write loop
    // Meta Step 11: Conditional port0 read-back verification loop
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

    // Meta Step 10: Port0 Write Loop
    LOGT("Port0 write loop: 10 iterations, offset=0x%lx", offset);
    for (index = 0; index < 10; index++)
    {
        // MANUAL_REVIEW: Meta uses rand() to generate exp_data_array values.
        // rand() is prohibited in PSV. Deterministic replacement values are
        // required but not supplied by Meta. Using placeholder deterministic
        // pattern. Replace with authoritative test data.
        // Original code: exp_data_array[index] = rand();
        // Original code: exp_data_array[index] = exp_data_array[index] | ((unsigned long)rand() << 32);
        exp_data_array[index] = (0xA5A5A5A5UL) | ((unsigned long)(0x5A5A0000UL + index) << 32);
        write_reg64(port0_addr + ((unsigned long)index * offset), exp_data_array[index]);
        LOGT("Port0 write index=%lu data=0x%lx addr=0x%lx",
             index, exp_data_array[index], port0_addr + ((unsigned long)index * offset));
    }

    // Meta Step 11: Port0 Read-back Verification Loop
    LOGT("Port0 read-back verification: 10 iterations");
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

    // Meta Step 12: Port1 Read-back Verification Loop
    LOGT("Port1 read-back verification: 10 iterations");
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

    // Final status: err0 == 0 is PASS, err0 != 0 is FAIL
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("Run complete: %s err0=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Teardown and cleanup for LPDDR4 Data Mask test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_mem_dm_test_teardown(
    const TestsItem *cfg
)
{
    (void)cfg;

    LOGT("LPDDR4 Data Mask (DM) test teardown: err0=%u", err0);
    return (err0 == 0) ? 0 : -1;
}
