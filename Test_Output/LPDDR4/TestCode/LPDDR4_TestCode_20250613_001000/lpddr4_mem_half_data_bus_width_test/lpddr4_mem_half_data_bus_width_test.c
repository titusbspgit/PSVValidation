// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

// Meta Step 1: File-scope declarations
unsigned long int port0_addr;
unsigned long int port1_addr;

// Testcase error counter (single final status path)
static unsigned int g_err0;

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initializes LPDDR4 half data bus width test.
 *              Configures port addresses, controller/PHY bases,
 *              bus width, speed grade, DBI_EN, DM_EN per Meta steps 4-10.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    // Meta Step 4: Call gpv_programming() - PROHIBITED in PSV
    // MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
    // If PSV bus interconnect setup is required, provide the PSV-approved equivalent.
    LOGT("LPDDR4 half bus width test init: gpv_programming skipped (DV-only)");

    // Meta Step 5: Initialize error counter to 0
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

    // Meta Step 7: Set bus width to Half Bus mode
    bus_width = 1; //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus

    // Meta Step 8: Speed grade selection (preprocessor conditional)
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    // Meta Step 9: Disable Data Bus Inversion
    DBI_EN = 0;

    // Meta Step 10: Disable Data Mask
    DM_EN = 0;

    LOGT("LPDDR4 half bus width test init complete: port0=0x%lx port1=0x%lx bus_width=%d",
         (unsigned long)port0_addr, (unsigned long)port1_addr, bus_width);

    return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes LPDDR4 half data bus width test.
 *              Performs training, waits for stabilization, then
 *              verifies read-back data from Port0 (conditional) and
 *              Port1 (unconditional) against expected patterns.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    // Meta Step 3: Local variable declarations
    unsigned long int index;
    unsigned long int offset;
    unsigned long int read_data0;
    unsigned long int read_data1;
    unsigned long int read_data2;
    unsigned long int read_data3;

    (void)cfg;
    (void)index;
    (void)offset;

    if (out == 0) {
        LOGE("LPDDR4 half bus width test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("LPDDR4 half bus width test run: starting");

    // Meta Step 11: Call lpddr4_training()
    lpddr4_training();
    LOGT("LPDDR4 training sequence executed");

    // Meta Step 12: Call training_done()
    training_done();
    LOGT("LPDDR4 training completion confirmed");

    // Meta Step 13: Call wait_on(1000) - PSV busy-wait adaptation
    for (volatile int d = 0; d < 10000; d++);
    LOGT("LPDDR4 stabilization delay completed (wait_on 1000 adapted)");

    // Meta Step 14: Port0 read-back verification (preprocessor conditional)
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))
    read_reg64(port0_addr + 0x100, &read_data0);
    read_reg64(port0_addr + 0x108, &read_data1);
    LOGT("Port0 read: read_data0=0x%lx read_data1=0x%lx",
         (unsigned long)read_data0, (unsigned long)read_data1);
    if ((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))
    {
        printf("ERROR: port0 read_data0 = %lx, read_data1 = %lx\n", read_data0, read_data1);
        LOGE("Port0 verification FAILED: read_data0=0x%lx read_data1=0x%lx",
             (unsigned long)read_data0, (unsigned long)read_data1);
        g_err0++;
    }
#endif

    // Meta Step 15: Port1 read-back verification (unconditional)
    read_reg64(port1_addr + 0x100, &read_data2);
    read_reg64(port1_addr + 0x108, &read_data3);
    LOGT("Port1 read: read_data2=0x%lx read_data3=0x%lx",
         (unsigned long)read_data2, (unsigned long)read_data3);
    if ((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))
    {
        printf("ERROR: port1 read_data2 = %lx, read_data3 = %lx\n", read_data2, read_data3);
        LOGE("Port1 verification FAILED: read_data2=0x%lx read_data3=0x%lx",
             (unsigned long)read_data2, (unsigned long)read_data3);
        g_err0++;
    }

    // Meta Step 16: finish(err0) - converted to PSV status reporting
    out->status = (g_err0 == 0U) ? 0 : -1;

    LOGT("LPDDR4 half bus width test run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL", g_err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Teardown for LPDDR4 half data bus width test.
 *              Reports final status based on accumulated errors.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 half bus width test teardown: errors=%u", g_err0);

    return (g_err0 == 0U) ? 0 : -1;
}
