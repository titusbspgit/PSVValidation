// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/* ---------------------------------------------------------------------------
 * Global variables
 * --------------------------------------------------------------------------- */
static unsigned int err0;

/* Expected data patterns from Meta Validation */
#define EXPECTED_PATTERN_0x100  0x3333333333333333UL
#define EXPECTED_PATTERN_0x108  0x2222222222222222UL

/* Memory offsets */
#define OFFSET_0x100  0x100UL
#define OFFSET_0x108  0x108UL

/* ---------------------------------------------------------------------------
 * FV entry points
 * --------------------------------------------------------------------------- */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initializes the testcase: performs platform-specific base
 *              address configuration, sets bus width to half, disables DBI
 *              and DM, configures speed grade, runs LPDDR4 training,
 *              signals training done, and waits for stabilization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("=== lpddr4_mem_half_data_bus_width_test_init: START ===");

    /* Meta Step 3: gpv_programming() - prohibited in FV/PSV */
    /* MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
       If GPV programming is required in the target FV/PSV environment,
       provide an approved replacement API. */

    /* Set err0 = 0 */
    err0 = 0;

    /* K5 Step 2: Conditional platform initialization */
#ifdef APS_DRAM
    ctl_base = APS_LPDDR4_CTL_BASE;
    phy_base = APS_LPDDR4_PHY_BASE;
    port0_addr = 0;
    port1_addr = 0x15A0000000UL;
    LOGT("Platform: APS_DRAM, ctl_base=0x%lx, port0_addr=0x%lx, port1_addr=0x%lx",
         (unsigned long)ctl_base, (unsigned long)port0_addr, (unsigned long)port1_addr);
#else
    ctl_base = MPS_LPDDR4_CTL_BASE;
    phy_base = MPS_LPDDR4_PHY_BASE;
    port0_addr = 0;
    port1_addr = 0x11A0000000UL;
    LOGT("Platform: MPS, ctl_base=0x%lx, port0_addr=0x%lx, port1_addr=0x%lx",
         (unsigned long)ctl_base, (unsigned long)port0_addr, (unsigned long)port1_addr);
#endif

    /* K5 Step 3: Set bus_width=1 (Half Bus) */
    bus_width = 1;
    LOGT("bus_width = 1 (Half Bus)");

    /* K5 Step 3: Conditional speed grade */
#ifdef SG2667
    SG = 2667;
    LOGT("Speed grade: SG2667");
#elif defined(SG2133)
    SG = 2133;
    LOGT("Speed grade: SG2133");
#else
    SG = 3200;
    LOGT("Speed grade: SG3200 (default)");
#endif

    /* K5 Step 3: Set DBI_EN=0, DM_EN=0 */
    DBI_EN = 0;
    DM_EN = 0;
    LOGT("DBI_EN=0, DM_EN=0");

    /* K5 Step 4: Execute LPDDR4 training sequence */
    lpddr4_training();
    LOGT("lpddr4_training() completed");

    /* K5 Step 5: Signal training done and wait for stabilization */
    training_done();
    LOGT("training_done() called");

    wait_on(1000);
    LOGT("wait_on(1000) completed — stabilization wait");

    LOGT("=== lpddr4_mem_half_data_bus_width_test_init: DONE ===");
    return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes the half data bus width read-only test:
 *              Phase 1 (conditional/APS_DRAM): reads two 64-bit values from
 *              port0 at offsets 0x100 and 0x108, validates using AND (&&) logic.
 *              Phase 2 (unconditional): reads two 64-bit values from port1 at
 *              offsets 0x100 and 0x108, validates using AND (&&) logic.
 *              The AND logic means an error is flagged only if BOTH reads fail
 *              to match their expected patterns simultaneously.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int read_data_0x100;
    unsigned long int read_data_0x108;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("=== lpddr4_mem_half_data_bus_width_test_run: START ===");

    /* ---------------------------------------------------------------
     * Phase 1 (Conditional): Read from port 0 at offsets 0x100 and
     *                        0x108, validate using AND (&&) logic
     * --------------------------------------------------------------- */
#ifdef APS_DRAM
    LOGT("Phase 1: port0 read at offsets 0x100 and 0x108");

    /* K5 Step 6: Read two 64-bit values from port 0 */
    read_data_0x100 = read_reg(port0_addr + OFFSET_0x100);
    LOGT("Phase1 Port0 Read [0x100]: read_data=0x%lx expected=0x%lx",
         (unsigned long)read_data_0x100, (unsigned long)EXPECTED_PATTERN_0x100);

    read_data_0x108 = read_reg(port0_addr + OFFSET_0x108);
    LOGT("Phase1 Port0 Read [0x108]: read_data=0x%lx expected=0x%lx",
         (unsigned long)read_data_0x108, (unsigned long)EXPECTED_PATTERN_0x108);

    /* Validate using AND (&&) logic: error only if BOTH fail */
    if ((read_data_0x100 != EXPECTED_PATTERN_0x100) && (read_data_0x108 != EXPECTED_PATTERN_0x108)) {
        LOGE("Phase1 Port0: BOTH reads failed — 0x100=0x%lx (exp 0x%lx), 0x108=0x%lx (exp 0x%lx)",
             (unsigned long)read_data_0x100, (unsigned long)EXPECTED_PATTERN_0x100,
             (unsigned long)read_data_0x108, (unsigned long)EXPECTED_PATTERN_0x108);
        err0++;
    } else {
        LOGT("Phase1 Port0: validation PASS (at least one pattern matched)");
    }

    LOGT("Phase 1: complete");
#endif /* APS_DRAM */

    /* ---------------------------------------------------------------
     * Phase 2 (Unconditional): Read from port 1 at offsets 0x100 and
     *                          0x108, validate using AND (&&) logic
     * --------------------------------------------------------------- */
    LOGT("Phase 2: port1 read at offsets 0x100 and 0x108");

    /* K5 Step 7: Read two 64-bit values from port 1 */
    read_data_0x100 = read_reg(port1_addr + OFFSET_0x100);
    LOGT("Phase2 Port1 Read [0x100]: read_data=0x%lx expected=0x%lx",
         (unsigned long)read_data_0x100, (unsigned long)EXPECTED_PATTERN_0x100);

    read_data_0x108 = read_reg(port1_addr + OFFSET_0x108);
    LOGT("Phase2 Port1 Read [0x108]: read_data=0x%lx expected=0x%lx",
         (unsigned long)read_data_0x108, (unsigned long)EXPECTED_PATTERN_0x108);

    /* Validate using AND (&&) logic: error only if BOTH fail */
    if ((read_data_0x100 != EXPECTED_PATTERN_0x100) && (read_data_0x108 != EXPECTED_PATTERN_0x108)) {
        LOGE("Phase2 Port1: BOTH reads failed — 0x100=0x%lx (exp 0x%lx), 0x108=0x%lx (exp 0x%lx)",
             (unsigned long)read_data_0x100, (unsigned long)EXPECTED_PATTERN_0x100,
             (unsigned long)read_data_0x108, (unsigned long)EXPECTED_PATTERN_0x108);
        err0++;
    } else {
        LOGT("Phase2 Port1: validation PASS (at least one pattern matched)");
    }

    LOGT("Phase 2: complete");

    /* ---------------------------------------------------------------
     * Final status
     * --------------------------------------------------------------- */
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("=== lpddr4_mem_half_data_bus_width_test_run: %s (err0=%u) ===",
         (err0 == 0) ? "PASS" : "FAIL", err0);

    return out->status;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("=== lpddr4_mem_half_data_bus_width_test_teardown ===");
    LOGT("Final error count: err0 = %u", err0);

    /* K5 Step 8: finish(err0) - converted to FV/PSV return */
    /* finish() is prohibited in FV/PSV. Return status based on err0. */
    return (err0 == 0) ? 0 : -1;
}
