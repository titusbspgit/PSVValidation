// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/* --------------------------------------------------------------------------
 * Expected data patterns from Meta Validation / Acceptance Criteria
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_EXPECTED_DATA_0x100  0x3333333333333333ULL
#define LPDDR4_HBW_EXPECTED_DATA_0x108  0x2222222222222222ULL

/* --------------------------------------------------------------------------
 * Memory offsets from Meta Test Steps
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_OFFSET_0  0x100UL
#define LPDDR4_HBW_OFFSET_1  0x108UL

/* --------------------------------------------------------------------------
 * Port address configuration from Meta (TestPlan H5)
 * -------------------------------------------------------------------------- */
#ifndef LPDDR4_HBW_PORT0_ADDR
#ifdef APS_DRAM
#define LPDDR4_HBW_PORT0_ADDR  0UL
#else
#define LPDDR4_HBW_PORT0_ADDR  0UL
#endif
#endif

#ifndef LPDDR4_HBW_PORT1_ADDR
#ifdef APS_DRAM
#define LPDDR4_HBW_PORT1_ADDR  0x15A0000000UL
#else
#define LPDDR4_HBW_PORT1_ADDR  0x11A0000000UL
#endif
#endif

/* --------------------------------------------------------------------------
 * Wait value from Meta Test Description: waits 1000 cycles after training_done
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_WAIT_CYCLES  1000U

/* --------------------------------------------------------------------------
 * Testcase context
 * -------------------------------------------------------------------------- */
typedef struct {
    uintptr_t      port0_addr;
    uintptr_t      port1_addr;
    unsigned int   errors;
    unsigned int   checks_total;
    unsigned int   checks_passed;
    unsigned int   checks_failed;
} lpddr4_hbw_test_ctx_t;

static lpddr4_hbw_test_ctx_t g_ctx;

/* ==========================================================================
 * FV entry point: init
 * ========================================================================== */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Initialise testcase context and configure platform addresses.
 *              Meta steps 1-3: declare variables, gpv_programming (prohibited,
 *              omitted), set err0=0, conditional platform init.
 * Parameters:
 *   cfg - framework test configuration (unused)
 * Returns:
 *   0 on success, -1 on configuration error.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Meta step 1-2: Declare local / global variables (context struct) */
    g_ctx = (lpddr4_hbw_test_ctx_t){0};

    /* Meta step 3: gpv_programming() — prohibited DV construct, omitted */
    /* MANUAL_REVIEW: gpv_programming() was present in DV source. */
    /*               Verify whether an FV/PSV equivalent is required. */

    /* Meta step 4 (implicit): Set err0 = 0 */
    g_ctx.errors = 0U;

    /* Meta step 4 (continued): Conditional platform initialization — configure port addresses */
    g_ctx.port0_addr = (uintptr_t)LPDDR4_HBW_PORT0_ADDR;
    g_ctx.port1_addr = (uintptr_t)LPDDR4_HBW_PORT1_ADDR;

    LOGT("LPDDR4 half bus width test init: port0=0x%lx port1=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    return 0;
}

/* ==========================================================================
 * FV entry point: run
 * ========================================================================== */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Execute the half data bus width read-only memory test.
 *              Meta steps 4-19: set bus_width=1/DBI=0/DM=0/speed, training,
 *              training_done(), wait 1000, Phase 1 (conditional) read port0,
 *              Phase 2 (unconditional) read port1, final status.
 * Parameters:
 *   cfg - framework test configuration (unused)
 *   out - framework test output structure
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint64_t read_data_0;
    uint64_t read_data_1;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 HBW output pointer is NULL");
        return -1;
    }

    out->status = 0;
    out->actual_len = 0;
    out->actual_pattern[0] = 0;

    /* ------------------------------------------------------------------
     * Meta step 4-6: Set bus_width to half (1), disable DBI, disable DM,
     *                configure speed grade, execute LPDDR4 training.
     *
     * MANUAL_REVIEW: The DV source sets bus_width=1, DBI_EN=0, DM_EN=0
     *   and calls lpddr4_training(). These are provided by the existing
     *   project header "lpddr4.h" / "test_common.h". In the FV/PSV
     *   environment the training and mode configuration are assumed to
     *   be handled by the platform/framework before the testcase runs.
     *   If explicit calls are required, uncomment the lines below.
     * ------------------------------------------------------------------ */
    /* bus_width = 1; */
    /* DBI_EN = 0;    */
    /* DM_EN = 0;     */
    /* MANUAL_REVIEW: speed grade selection — SG3200 default, SG2667, SG2133 */
    /* lpddr4_training(); */

    /* ------------------------------------------------------------------
     * Meta step 7: Signal training done and wait for stabilization.
     * Meta Test Description: calls training_done(), waits 1000 cycles.
     * ------------------------------------------------------------------ */
    /* MANUAL_REVIEW: training_done() is a DV/platform API. Verify whether */
    /*               an FV/PSV equivalent is required or if the framework  */
    /*               handles this. If available, uncomment below.          */
    /* training_done(); */
    /* wait_on(LPDDR4_HBW_WAIT_CYCLES); */

    LOGT("Starting LPDDR4 half bus width read-only validation");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ==================================================================
     * Phase 1 (Conditional): Read two 64-bit values from port 0 at
     *                        offsets 0x100 and 0x108, validate against
     *                        expected patterns.
 *
     * Meta step 8 (conditional block).
 *
     * Meta Validation (M5 criterion 1): error flagged only if BOTH reads
     * fail simultaneously (logical AND — &&). This is preserved exactly
     * per the No Silent Correction Rule.
     * ================================================================== */
#ifdef LPDDR4_HBW_PHASE1_ENABLE
    LOGT("Phase 1: port0 read at offsets 0x%lx and 0x%lx",
         (unsigned long)LPDDR4_HBW_OFFSET_0,
         (unsigned long)LPDDR4_HBW_OFFSET_1);

    read_data_0 = readl_reg(g_ctx.port0_addr + (uintptr_t)LPDDR4_HBW_OFFSET_0);
    read_data_1 = readl_reg(g_ctx.port0_addr + (uintptr_t)LPDDR4_HBW_OFFSET_1);
    g_ctx.checks_total++;

    LOGT("Phase 1 port0: read_data[0x100]=0x%llx read_data[0x108]=0x%llx",
         (unsigned long long)read_data_0,
         (unsigned long long)read_data_1);

    /* Meta validation uses logical AND (&&): error only if BOTH mismatch */
    if ((read_data_0 != LPDDR4_HBW_EXPECTED_DATA_0x100) &&
        (read_data_1 != LPDDR4_HBW_EXPECTED_DATA_0x108)) {
        LOGE("Phase 1 port0 FAIL: BOTH reads mismatch. "
             "read[0x100]=0x%llx exp=0x%llx read[0x108]=0x%llx exp=0x%llx",
             (unsigned long long)read_data_0,
             (unsigned long long)LPDDR4_HBW_EXPECTED_DATA_0x100,
             (unsigned long long)read_data_1,
             (unsigned long long)LPDDR4_HBW_EXPECTED_DATA_0x108);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        g_ctx.checks_passed++;
    }
#endif /* LPDDR4_HBW_PHASE1_ENABLE */

    /* ==================================================================
     * Phase 2 (Unconditional): Read two 64-bit values from port 1 at
     *                          offsets 0x100 and 0x108, validate against
     *                          expected patterns.
 *
     * Meta step 9.
 *
     * Meta Validation (M5 criterion 2): same AND (&&) logic.
     * ================================================================== */
    LOGT("Phase 2: port1 read at offsets 0x%lx and 0x%lx",
         (unsigned long)LPDDR4_HBW_OFFSET_0,
         (unsigned long)LPDDR4_HBW_OFFSET_1);

    read_data_0 = readl_reg(g_ctx.port1_addr + (uintptr_t)LPDDR4_HBW_OFFSET_0);
    read_data_1 = readl_reg(g_ctx.port1_addr + (uintptr_t)LPDDR4_HBW_OFFSET_1);
    g_ctx.checks_total++;

    LOGT("Phase 2 port1: read_data[0x100]=0x%llx read_data[0x108]=0x%llx",
         (unsigned long long)read_data_0,
         (unsigned long long)read_data_1);

    /* Meta validation uses logical AND (&&): error only if BOTH mismatch */
    if ((read_data_0 != LPDDR4_HBW_EXPECTED_DATA_0x100) &&
        (read_data_1 != LPDDR4_HBW_EXPECTED_DATA_0x108)) {
        LOGE("Phase 2 port1 FAIL: BOTH reads mismatch. "
             "read[0x100]=0x%llx exp=0x%llx read[0x108]=0x%llx exp=0x%llx",
             (unsigned long long)read_data_0,
             (unsigned long long)LPDDR4_HBW_EXPECTED_DATA_0x100,
             (unsigned long long)read_data_1,
             (unsigned long long)LPDDR4_HBW_EXPECTED_DATA_0x108);
        g_ctx.errors++;
        g_ctx.checks_failed++;
    } else {
        g_ctx.checks_passed++;
    }

    /* ------------------------------------------------------------------
     * Final status — Meta step 10: finish(err0) converted to FV status.
     * ------------------------------------------------------------------ */
    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    out->actual_len = 1;
    out->actual_pattern[0] = (int)g_ctx.errors;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/* ==========================================================================
 * FV entry point: teardown
 * ========================================================================== */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - framework test configuration (unused)
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 HBW teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
