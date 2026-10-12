// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

// File-scope declarations (Meta Step 1)
static unsigned long int port0_addr;
static unsigned long int port1_addr;

// Error context
static unsigned int g_err0;

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initialize LPDDR4 half data bus width test.
 *   Configures port addresses, controller/PHY bases, bus width,
 *   speed grade, DBI, and DM settings based on preprocessor defines.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_mem_half_data_bus_width_test: init start");

    // Meta Step 4: gpv_programming() is a prohibited DV construct
    // MANUAL_REVIEW: gpv_programming() was called here in DV. Verify if PSV
    // environment requires an equivalent bus interconnect setup.
    LOGT("DV gpv_programming() skipped - prohibited in PSV");

    // Meta Step 5: err0 = 0
    g_err0 = 0;

    // Meta Step 6: Address configuration (preprocessor conditional)
#if defined(APS_DRAM)
    port0_addr = 0;
    port1_addr = 0x15A0000000;
    ctl_base = 0x9EE03000;
    phy_base = 0x9F000000;
#else
    port0_addr = 0;
    port1_addr = 0x11A0000000;
    ctl_base = 0x11D003000;
    phy_base = 0x11D500000;
#endif

    LOGT("port0_addr=0x%lx port1_addr=0x%lx", port0_addr, port1_addr);

    // Meta Step 7: bus_width = 1; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus
    bus_width = 1;

    // Meta Step 8: Speed grade selection (preprocessor conditional)
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    // Meta Step 9: DBI_EN = 0
    DBI_EN = 0;

    // Meta Step 10: DM_EN = 0
    DM_EN = 0;

    LOGT("bus_width=%d SG=%d DBI_EN=%d DM_EN=%d", bus_width, SG, DBI_EN, DM_EN);

    LOGT("lpddr4_mem_half_data_bus_width_test: init complete");

    return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Execute LPDDR4 half data bus width test.
 *   Performs training, waits for stabilization, then verifies
 *   64-bit read-back data from port0 (conditional) and port1
 *   (unconditional) against expected patterns.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput result structure
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    (void)cfg;

    // Meta Step 3: Local variable declarations
    unsigned long int index;
    unsigned long int offset;
    unsigned long int read_data0;
    unsigned long int read_data1;
    unsigned long int read_data2;
    unsigned long int read_data3;

    (void)index;
    (void)offset;

    if (out == 0) {
        LOGE("lpddr4_mem_half_data_bus_width_test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("lpddr4_mem_half_data_bus_width_test: run start");

    // Meta Step 11: Call lpddr4_training()
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    // Meta Step 12: Call training_done()
    LOGT("Calling training_done()");
    training_done();

    // Meta Step 13: wait_on(1000) - converted to PSV busy-wait
    LOGT("Wait for stabilization (DV wait_on 1000)");
    for (volatile int d = 0; d < 10000; d++);

    // Meta Step 14: Port0 read-back verification (preprocessor conditional)
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    LOGT("Port0 read-back verification");
    read_reg64(port0_addr + 0x100, &read_data0);
    read_reg64(port0_addr + 0x108, &read_data1);
    LOGT("Port0: read_data0=0x%lx read_data1=0x%lx", read_data0, read_data1);

    if ((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))
    {
        LOGE("ERROR: port0 read_data0 = %lx, read_data1 = %lx", read_data0, read_data1);
        g_err0++;
    }
#endif

    // Meta Step 15: Port1 read-back verification (unconditional)
    LOGT("Port1 read-back verification");
    read_reg64(port1_addr + 0x100, &read_data2);
    read_reg64(port1_addr + 0x108, &read_data3);
    LOGT("Port1: read_data2=0x%lx read_data3=0x%lx", read_data2, read_data3);

    if ((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))
    {
        LOGE("ERROR: port1 read_data2 = %lx, read_data3 = %lx", read_data2, read_data3);
        g_err0++;
    }

    // Meta Step 16: finish(err0) - converted to PSV status
    out->status = (g_err0 == 0U) ? 0 : -1;

    LOGT("lpddr4_mem_half_data_bus_width_test: run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Teardown for LPDDR4 half data bus width test.
 *   Reports final status based on accumulated error count.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_mem_half_data_bus_width_test: teardown: errors=%u", g_err0);

    return (g_err0 == 0U) ? 0 : -1;
}
