// Author - AI Force 2.3. 2025-06-30 11:30:00 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/* --------------------------------------------------------------------------
 * Expected data patterns from Meta Validation / Acceptance Criteria (M5)
 * offset 0x100 → 0x3333333333333333
 * offset 0x108 → 0x2222222222222222
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_EXPECTED_DATA_0x100  0x3333333333333333ULL
#define LPDDR4_HBW_EXPECTED_DATA_0x108  0x2222222222222222ULL

/* --------------------------------------------------------------------------
 * Memory offsets from Meta TestPlan K5 steps 6-7
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_OFFSET_0  0x100UL
#define LPDDR4_HBW_OFFSET_1  0x108UL

/* --------------------------------------------------------------------------
 * Port address configuration from Meta TestPlan H5
 * port0_addr: 0 (both APS_DRAM and else)
 * port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)
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
 * Wait value from Meta Test Description (C5):
 * "calls training_done(), waits 1000 cycles"
 * -------------------------------------------------------------------------- */
#define LPDDR4_HBW_WAIT_CYCLES  1000U

/* --------------------------------------------------------------------------
 * Testcase context (FV Template structural pattern)
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
 *              Meta steps: declare variables (D5 steps 1-2),
 *              gpv_programming (D5 step 3 — prohibited, omitted),
 *              set err0=0, conditional platform init (K5 step 2).
 * Parameters:
 *   cfg - framework test configuration
 * Returns:
 *   0 on success.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    /* Meta D5 step 1-2: Declare local / global variables */
    g_ctx = (lpddr4_hbw_test_ctx_t){0};

    /* Meta D5 step 3: gpv_programming() — prohibited DV construct, omitted */
    /* MANUAL_REVIEW: gpv_programming() was present in DV source (K5 step 1). */
    /*               Verify whether an FV/PSV equivalent is required.        */

    /* Meta implicit: Set err0 = 0 */
    g_ctx.errors = 0U;

    /* Meta K5 step 2: Configure platform-specific base addresses */
    g_ctx.port0_addr = (uintptr_t)LPDDR4_HBW_PORT0_ADDR;
    g_ctx.port1_addr = (uintptr_t)LPDDR4_HBW_PORT1_ADDR;

    LOGT("LPDDR4 half bus width test init: port0=0x%lx port1=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ------------------------------------------------------------------
     * Meta K5 steps 3-4: Set bus_width=1 (Half Bus), DBI_EN=0, DM_EN=0,
     *   configure speed grade, execute LPDDR4 training.
 *
     * MANUAL_REVIEW: The DV source sets bus_width=1, DBI_EN=0, DM_EN=0
     *   and calls lpddr4_training(). These are provided by the existing
     *   project headers "lpddr4.h" / "test_common.h". In the FV/PSV
     *   environment the training and mode configuration may be handled
     *   by the platform/framework before the testcase runs.
     *   If explicit calls are required, uncomment the lines below.
     * ------------------------------------------------------------------ */
    /* bus_width = 1; */
    /* DBI_EN = 0;    */
    /* DM_EN = 0;     */
    /* MANUAL_REVIEW: speed grade selection — SG3200 default, SG2667, SG2133 */
    /* lpddr4_training(); */

    /* ------------------------------------------------------------------
     * Meta K5 step 5 / Meta C5: Signal training done and wait for
     *   stabilization. "calls training_done(), waits 1000 cycles"
     *
     * MANUAL_REVIEW: training_done() and wait_on() are DV/platform APIs.
     *   Verify whether FV/PSV equivalents are required or if the
     *   framework handles this. If available, uncomment below.
     * ------------------------------------------------------------------ */
    /* training_done(); */
    /* wait_on(LPDDR4_HBW_WAIT_CYCLES); */

    return 0;
}

/* ==========================================================================
 * FV entry point: run
 * ========================================================================== */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Execute the half data bus width read-only memory test.
 *              Meta K5 steps 6-8: Phase 1 (conditional) read port0 at
 *              offsets 0x100 and 0x108, Phase 2 (unconditional) read port1
 *              at same offsets, validate using logical AND (&&), final status.
 * Parameters:
 *   cfg - framework test configuration
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

    LOGT("Starting LPDDR4 half bus width read-only validation");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ==================================================================
     * Phase 1 (Conditional): Meta K5 step 6
 *
     * "(Conditional) Read two 64-bit values from port 0 at offsets 0x100
 *  and 0x108, and validate against expected patterns."
 *
     * Meta Validation M5 criterion 1:
 *   "Data read from port 0 at offset 0x100 must equal
 *    0x3333333333333333 or data at offset 0x108 must equal
 *    0x2222222222222222"
 *
     * Meta Remarks J5:
 *   "The validation uses logical AND (&&), meaning an error is only
 *    flagged if BOTH read values fail to match their expected patterns
 *    simultaneously."
 *
     * This AND (&&) logic is preserved exactly per the No Silent
 *   Correction Rule.
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
        LOGT("Phase 1 port0 PASS");
        g_ctx.checks_passed++;
    }
#endif /* LPDDR4_HBW_PHASE1_ENABLE */

    /* ==================================================================
     * Phase 2 (Unconditional): Meta K5 step 7
 *
     * "Read two 64-bit values from port 1 at offsets 0x100 and 0x108,
 *  and validate against expected patterns."
 *
     * Meta Validation M5 criterion 2:
 *   "Data read from port 1 at offset 0x100 must equal
 *    0x3333333333333333 or data at offset 0x108 must equal
 *    0x2222222222222222"
 *
     * Same AND (&&) logic as Phase 1.
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
        LOGT("Phase 2 port1 PASS");
        g_ctx.checks_passed++;
    }

    /* ------------------------------------------------------------------
     * Meta K5 step 8 / Meta F5 criterion 3:
 *   "Call finish with the error counter to determine pass or fail."
 *   finish() is a prohibited DV construct — converted to FV status
 *   via out->status. Test passes if err0 == 0.
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
 *   cfg - framework test configuration
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 HBW teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
