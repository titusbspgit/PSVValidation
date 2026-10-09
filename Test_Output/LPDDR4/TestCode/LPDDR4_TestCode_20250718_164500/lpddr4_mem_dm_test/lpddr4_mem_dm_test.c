// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

/* Global variables as per Meta Test Description */
unsigned long int port0_addr;
unsigned long int port1_addr;

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Initializes the LPDDR4 DM test by configuring port addresses,
 *              controller base, PHY base, bus width, speed grade, DBI, DM,
 *              and performing LPDDR4 memory training.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_dm_test_init(const TestsItem cfg)
{
    (void)cfg;

    LOGT("LPDDR4 DM test init: starting initialization");

    /* Meta Step 1: Call gpv_programming() */
    /* gpv_programming() is a prohibited DV construct - not generated */
    /* MANUAL_REVIEW: gpv_programming() was present in DV source. */
    /* If PSV equivalent is needed, provide approved replacement. */
    LOGD("DV gpv_programming() call skipped in PSV");

    /* Meta Step 2: err0 = 0 (handled in _run) */

    /* Meta Step 3: Conditional port address and base address configuration */
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
         (unsigned long)port0_addr, (unsigned long)port1_addr);
    LOGT("LPDDR4 DM test: ctl_base=0x%lx phy_base=0x%lx",
         (unsigned long)ctl_base, (unsigned long)phy_base);

    /* Meta Step 4: bus_width = 0 (Full Bus) */
    bus_width = 0; /* 0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus */

    /* Meta Step 5: Conditional speed grade selection */
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    LOGT("LPDDR4 DM test: SG=%lu bus_width=%lu",
         (unsigned long)SG, (unsigned long)bus_width);

    /* Meta Step 6: Disable Data Bus Inversion */
    DBI_EN = 0;

    /* Meta Step 7: Enable Data Mask */
    DM_EN = 1;

    LOGT("LPDDR4 DM test: DBI_EN=%lu DM_EN=%lu",
         (unsigned long)DBI_EN, (unsigned long)DM_EN);

    /* Meta Step 8: Call lpddr4_training() */
    LOGT("LPDDR4 DM test: calling lpddr4_training()");
    lpddr4_training();

    LOGT("LPDDR4 DM test init: complete");

    return 0;
}

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Executes the LPDDR4 DM test by writing deterministic 64-bit
 *              data patterns to port0, then reading back from port0 and port1
 *              to verify data integrity with Data Mask enabled.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   FV/template-compatible status.
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

    /* Meta Step 2: Clear error counter */
    err0 = 0;

    /* Meta Step 9: Set memory access offset */
    offset = 0x1000;

    LOGT("LPDDR4 DM test run: starting with offset=0x%lx",
         (unsigned long)offset);

    /* Meta Step 10: Conditionally write 10 data values to port0 */
    /* Meta Step 11: Conditionally read back from port0 and verify */
    /* Guarded by XOR-based preprocessor condition */
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

    LOGT("LPDDR4 DM test: Port0 write loop - 10 iterations");

    /* Port0 Write Loop */
    for (index = 0; index < 10; index++)
    {
        /* MANUAL_REVIEW: Original DV code used rand() to generate random 64-bit data. */
        /* rand() is prohibited in PSV. Replace exp_data_array[] values with */
        /* deterministic test data appropriate for this testcase. */
        /* exp_data_array[index] = rand(); */
        /* exp_data_array[index] = exp_data_array[index] | ((unsigned long)rand() << 32); */
        exp_data_array[index] = (unsigned long int)(0xA5A5A5A5UL | ((unsigned long int)(0x5A5A0000UL + index) << 32));
        write_reg64(port0_addr + ((unsigned long)index * offset), exp_data_array[index]);
    }

    LOGT("LPDDR4 DM test: Port0 read-back verification loop - 10 iterations");

    /* Port0 Read-back Verification Loop */
    for (index = 0; index < 10; index++)
    {
        read_reg64(port0_addr + ((unsigned long)index * offset), &read_data);
        if (read_data != exp_data_array[index])
        {
            LOGE("ERROR_0: port0 index = %lu, exp_data = %lx, actual_data = %lx",
                 (unsigned long)index,
                 (unsigned long)exp_data_array[index],
                 (unsigned long)read_data);
            err0++;
        }
    }

#endif

    /* Meta Step 12: Port1 Read-back Verification Loop */
    LOGT("LPDDR4 DM test: Port1 read-back verification loop - 10 iterations");

    for (index = 0; index < 10; index++)
    {
        read_reg64(port1_addr + ((unsigned long)index * offset), &read_data);
        if (read_data != exp_data_array[index])
        {
            LOGE("ERROR_0: port1 index = %lu, exp_data = %lx, actual_data = %lx",
                 (unsigned long)index,
                 (unsigned long)exp_data_array[index],
                 (unsigned long)read_data);
            err0++;
        }
    }

    /* Validation: Pass if err0 == 0, Fail if err0 != 0 */
    out->status = (err0 == 0U) ? 0 : -1;

    LOGT("LPDDR4 DM test run complete: %s err0=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Performs teardown for the LPDDR4 DM test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 DM test teardown: no additional cleanup required");
    return 0;
}
