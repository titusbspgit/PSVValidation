// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/* Global variables as per Meta Test Description /
unsigned long int port0_addr;
unsigned long int port1_addr;

/
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initializes the LPDDR4 half data bus width test by configuring
 *              port addresses, controller base, PHY base, bus width, speed grade,
 *              DBI, DM, and performing LPDDR4 memory training.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("LPDDR4 half data bus width test init: starting initialization");

    / Meta Step 1: Call gpv_programming() /
    / gpv_programming() is a prohibited DV construct - not generated /
    / MANUAL_REVIEW: gpv_programming() was present in DV source. /
    / If PSV equivalent is needed, provide approved replacement. /
    LOGD("DV gpv_programming() call skipped in PSV");

    / Meta Step 2: err0 = 0 (handled in _run) /

    / Meta Step 3: Conditional port address and base address configuration /
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

    LOGT("LPDDR4 half bus test: port0_addr=0x%lx port1_addr=0x%lx"
         (unsigned long)port0_addr, (unsigned long)port1_addr);
    LOGT("LPDDR4 half bus test: ctl_base=0x%lx phy_base=0x%lx"
         (unsigned long)ctl_base, (unsigned long)phy_base);

    / Meta Step 4: bus_width = 1 (Half Bus) /
    bus_width = 1; / 0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus /

    / Meta Step 5: Conditional speed grade selection /
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    LOGT("LPDDR4 half bus test: SG=%lu bus_width=%lu"
         (unsigned long)SG, (unsigned long)bus_width);

    / Meta Step 6: Disable Data Bus Inversion /
    DBI_EN = 0;

    / Meta Step 7: Disable Data Mask /
    DM_EN = 0;

    LOGT("LPDDR4 half bus test: DBI_EN=%lu DM_EN=%lu"
         (unsigned long)DBI_EN, (unsigned long)DM_EN);

    / Meta Step 8: Call lpddr4_training() /
    LOGT("LPDDR4 half bus test: calling lpddr4_training()");
    lpddr4_training();

    / Meta Step 9: Call training_done() /
    LOGT("LPDDR4 half bus test: calling training_done()");
    training_done();

    / Meta Step 10: Call wait_on(1000) /
    / wait_on() is not available in PSV - converted to deterministic busy-wait /
    LOGT("LPDDR4 half bus test: wait for stabilization (1000)");
    for (volatile int d = 0; d < 10000; d++);

    LOGT("LPDDR4 half bus test init: complete");

    return 0;
}

/
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes the LPDDR4 half data bus width test by reading back
 *              data from port0 and port1 at offsets 0x100 and 0x108 and
 *              verifying against expected patterns.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput out)
{
    unsigned long int read_data0;
    unsigned long int read_data1;
    unsigned long int read_data2;
    unsigned long int read_data3;
    unsigned int err0;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 half bus test: output pointer is NULL");
        return -1;
    }

    out->status = 0;

    / Meta Step 2: Clear error counter /
    err0 = 0;

    LOGT("LPDDR4 half bus test run: starting verification");

    / MANUAL_REVIEW: The write_reg64 calls that write the test patterns /
    / 0x3333333333333333 and 0x2222222222222222 to port0_addr and port1_addr /
    / at offsets 0x100 and 0x108 are not visible in the retrieved source /
    / context. If these writes are required before read-back, they must be /
    / added here. The training or firmware may have placed these patterns. /

    / Meta Step 11: Conditional port0 read-back verification /
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

    LOGT("LPDDR4 half bus test: Port0 read-back verification");

    read_reg64(port0_addr + 0x100, &read_data0);
    read_reg64(port0_addr + 0x108, &read_data1);

    LOGT("LPDDR4 half bus test: port0 read_data0=0x%lx read_data1=0x%lx"
         (unsigned long)read_data0, (unsigned long)read_data1);

    if ((read_data0 != 0x3333333333333333UL) && (read_data1 != 0x2222222222222222UL))
    {
        LOGE("ERROR: port0 read_data0 = %lx, read_data1 = %lx"
             (unsigned long)read_data0, (unsigned long)read_data1);
        err0++;
    }

#endif

    / Meta Step 12: Read port1_addr + 0x100 /
    LOGT("LPDDR4 half bus test: Port1 read-back verification");

    read_reg64(port1_addr + 0x100, &read_data2);

    / Meta Step 13: Read port1_addr + 0x108 /
    read_reg64(port1_addr + 0x108, &read_data3);

    LOGT("LPDDR4 half bus test: port1 read_data2=0x%lx read_data3=0x%lx"
         (unsigned long)read_data2, (unsigned long)read_data3);

    / Meta Step 14: Port1 validation /
    if ((read_data2 != 0x3333333333333333UL) && (read_data3 != 0x2222222222222222UL))
    {
        LOGE("ERROR: port1 read_data2 = %lx, read_data3 = %lx"
             (unsigned long)read_data2, (unsigned long)read_data3);
        err0++;
    }

    / Meta Step 15: finish(err0) - prohibited DV construct /
    / Converted to FV/PSV status mechanism /
    out->status = (err0 == 0U) ? 0 : -1;

    LOGT("LPDDR4 half bus test run complete: %s err0=%u"
         (out->status == 0) ? "PASS" : "FAIL"
         err0);

    return out->status;
}

/
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Performs teardown for the LPDDR4 half data bus width test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 half bus test teardown: no additional cleanup required");
    return 0;
}
