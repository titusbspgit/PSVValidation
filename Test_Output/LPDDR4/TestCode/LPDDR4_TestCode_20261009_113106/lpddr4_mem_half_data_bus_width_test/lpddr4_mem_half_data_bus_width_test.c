// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

// Global variables as per Meta: unsigned long int port0_addr; unsigned long int port1_addr;
static unsigned long int port0_addr;
static unsigned long int port1_addr;
static unsigned long int ctl_base;
static unsigned long int phy_base;
static int err0;
static unsigned int bus_width;
static unsigned int SG;
static unsigned int DBI_EN;
static unsigned int DM_EN;

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initialize LPDDR4 half data bus width test configuration,
 *              perform training and wait for stabilization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 half data bus width test init start");

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

    // Meta Step 4: bus_width = 1 (0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus)
    bus_width = 1;

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

    // Meta Step 7: DM_EN = 0
    DM_EN = 0;

    LOGT("DBI_EN=%u DM_EN=%u", DBI_EN, DM_EN);

    // Meta Step 8: Call lpddr4_training()
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    // Meta Step 9: Call training_done()
    LOGT("Calling training_done()");
    training_done();

    // Meta Step 10: Call wait_on(1000) - adapted to deterministic busy-wait
    LOGT("Wait for stabilization (wait_on 1000)");
    for (volatile int d = 0; d < 1000; d++);

    LOGT("LPDDR4 half data bus width test init complete");
    return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Execute LPDDR4 half data bus width read-back validation
 *              on port0 (conditional) and port1 (unconditional).
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int read_data0 = 0;
    unsigned long int read_data1 = 0;
    unsigned long int read_data2 = 0;
    unsigned long int read_data3 = 0;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("LPDDR4 half data bus width test run start");

    // Meta Step 11: Conditional port0 read-back validation
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    LOGT("Port0 read-back validation");
    read_reg64(port0_addr + 0x100, &read_data0);
    read_reg64(port0_addr + 0x108, &read_data1);
    LOGT("port0 read_data0=0x%lx read_data1=0x%lx", read_data0, read_data1);
    if ((read_data0 != 0x3333333333333333UL) && (read_data1 != 0x2222222222222222UL))
    {
        LOGE("ERROR: port0 read_data0 = %lx, read_data1 = %lx", read_data0, read_data1);
        err0++;
    }
#endif

    // Meta Step 12: read_reg64(port1_addr + 0x100, &read_data2)
    LOGT("Port1 read-back validation");
    read_reg64(port1_addr + 0x100, &read_data2);

    // Meta Step 13: read_reg64(port1_addr + 0x108, &read_data3)
    read_reg64(port1_addr + 0x108, &read_data3);

    LOGT("port1 read_data2=0x%lx read_data3=0x%lx", read_data2, read_data3);

    // Meta Step 14: port1 validation
    if ((read_data2 != 0x3333333333333333UL) && (read_data3 != 0x2222222222222222UL))
    {
        LOGE("ERROR: port1 read_data2 = %lx, read_data3 = %lx", read_data2, read_data3);
        err0++;
    }

    // Meta Step 15: finish(err0) -> PSV status return
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("Run complete: %s err0=%d",
         (out->status == 0) ? "PASS" : "FAIL", err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Teardown and final status for LPDDR4 half data bus width test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 half data bus width test teardown: err0=%d", err0);
    return (err0 == 0) ? 0 : -1;
}
