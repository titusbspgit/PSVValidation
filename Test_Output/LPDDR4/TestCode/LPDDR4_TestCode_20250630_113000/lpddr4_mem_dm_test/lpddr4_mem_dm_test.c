// Author - AI Force 2.3. 2025-06-30 11:30:00 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

/* --------------------------------------------------------------------------
 * Phase 1 (conditional) parameters from Meta TestPlan K4 steps 5-7
 * -------------------------------------------------------------------------- */
#define LPDDR4_DM_PHASE1_LOCATIONS  10U
#define LPDDR4_DM_PHASE1_STRIDE     0x1000UL

/* --------------------------------------------------------------------------
 * Phase 2 (unconditional) parameters from Meta TestPlan K4 steps 8-9
 * -------------------------------------------------------------------------- */
#define LPDDR4_DM_PHASE2_LOCATIONS  32U
#define LPDDR4_DM_PHASE2_STRIDE     0x20000000UL

/* --------------------------------------------------------------------------
 * Total expected checks for sanity verification
 * Phase 1: 10 (port0 read-back) + 10 (port1 cross-port) = 20
 * Phase 2: 32 (port1 read-back)
 * Total: 52 (when Phase 1 is enabled)
 * -------------------------------------------------------------------------- */
#define LPDDR4_DM_TOTAL_CHECKS_PHASE1  (LPDDR4_DM_PHASE1_LOCATIONS * 2U)
#define LPDDR4_DM_TOTAL_CHECKS_PHASE2  LPDDR4_DM_PHASE2_LOCATIONS

/* --------------------------------------------------------------------------
 * Port address configuration from Meta TestPlan H4
 * port0_addr: 0 (both APS_DRAM and else)
 * port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)
 * -------------------------------------------------------------------------- */
#ifndef LPDDR4_DM_PORT0_ADDR
#ifdef APS_DRAM
#define LPDDR4_DM_PORT0_ADDR  0UL
#else
#define LPDDR4_DM_PORT0_ADDR  0UL
#endif
#endif

#ifndef LPDDR4_DM_PORT1_ADDR
#ifdef APS_DRAM
#define LPDDR4_DM_PORT1_ADDR  0x15A0000000UL
#else
#define LPDDR4_DM_PORT1_ADDR  0x11A0000000UL
#endif
#endif

/* --------------------------------------------------------------------------
 * Deterministic pattern generation macro.
 * Meta source uses rand() which is prohibited. This deterministic pattern
 * replaces rand() to produce unique 64-bit values per index and phase.
 * MANUAL_REVIEW: The DV source populated exp_data_array via rand().
 *               Verify that this deterministic pattern is acceptable for
 *               FV/PSV validation, or supply authoritative test vectors.
 * -------------------------------------------------------------------------- */
#define LPDDR4_DM_PATTERN(index, phase_seed) \
    ((uint64_t)0xA5A5A5A5A5A5A5A5ULL ^ \
     ((uint64_t)(index) << 32) ^ \
     (uint64_t)(phase_seed))

/* --------------------------------------------------------------------------
 * Testcase context structure (FV Template structural pattern)
 * -------------------------------------------------------------------------- */
typedef struct {
    uintptr_t      port0_addr;
    uintptr_t      port1_addr;
    unsigned int   errors;
    unsigned int   checks_total;
    unsigned int   checks_passed;
    unsigned int   checks_failed;
} lpddr4_dm_test_ctx_t;

static lpddr4_dm_test_ctx_t g_ctx;

/* --------------------------------------------------------------------------
 * Meta Arrays (I4): unsigned long int exp_data_array[50]
 * Declared with size 50. No static initializer. Populated at runtime.
 * -------------------------------------------------------------------------- */
static unsigned long int exp_data_array[50];

/* --------------------------------------------------------------------------
 * Helper: check a single memory location against expected data
 * (FV Template structural pattern adapted for this testcase)
 * -------------------------------------------------------------------------- */

/*
 * Function: lpddr4_dm_check_location
 * Description: Read a 64-bit value from (base + offset) and compare against
 *              the expected value from exp_data_array.
 * Parameters:
 *   base     - port base address
 *   offset   - byte offset from base
 *   index    - array index for logging
 *   expected - expected 64-bit value
 * Returns:
 *   0 on match, -1 on mismatch.
 */
static int lpddr4_dm_check_location(uintptr_t base,
                                    unsigned long int offset,
                                    unsigned long int index,
                                    uint64_t expected)
{
    uint64_t actual;

    actual = readl_reg(base + (uintptr_t)offset);
    g_ctx.checks_total++;

    if (actual != expected) {
        LOGE("LPDDR4 DM read mismatch addr=0x%lx index=%lu exp=0x%llx actual=0x%llx",
             (unsigned long)(base + (uintptr_t)offset),
             index,
             (unsigned long long)expected,
             (unsigned long long)actual);
        g_ctx.errors++;
        g_ctx.checks_failed++;
        return -1;
    }

    g_ctx.checks_passed++;
    return 0;
}

/* ==========================================================================
 * FV entry point: init
 * ========================================================================== */

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Initialise testcase context, configure platform addresses,
 *              and perform pre-test setup.
 *              Meta steps 1-5: declare variables, gpv_programming (prohibited),
 *              set err0=0, conditional platform initialization.
 * Parameters:
 *   cfg - framework test configuration
 * Returns:
 *   0 on success, -1 on configuration error.
 */
int lpddr4_mem_dm_test_init(const TestsItem *cfg)
{
    unsigned long int i;

    (void)cfg;

    /* Meta step 1-2: Declare local / global variables */
    g_ctx = (lpddr4_dm_test_ctx_t){0};

    /* Meta step 3: gpv_programming() — prohibited DV construct, omitted */
    /* MANUAL_REVIEW: gpv_programming() was present in DV source. */
    /*               Verify whether an FV/PSV equivalent is required. */

    /* Meta step 4: Set err0 = 0 */
    g_ctx.errors = 0U;

    /* Meta step 5: Conditional platform initialization — configure port addresses */
    /* Meta TestPlan H4: port0_addr=0, port1_addr=0x15A0000000 (APS) / 0x11A0000000 */
    g_ctx.port0_addr = (uintptr_t)LPDDR4_DM_PORT0_ADDR;
    g_ctx.port1_addr = (uintptr_t)LPDDR4_DM_PORT1_ADDR;

    /* Initialize exp_data_array to zero */
    for (i = 0UL; i < 50UL; i++) {
        exp_data_array[i] = 0UL;
    }

    LOGT("LPDDR4 DM test init: port0=0x%lx port1=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ------------------------------------------------------------------
     * Meta TestPlan K4 steps 2-4: Configure platform-specific base addresses,
 *   set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=1, configure speed grade,
 *   execute LPDDR4 training.
 *
     * MANUAL_REVIEW: The DV source sets bus_width=0, DBI_EN=0, DM_EN=1
     *   and calls lpddr4_training(). These are provided by the existing
     *   project headers "lpddr4.h" / "test_common.h". In the FV/PSV
     *   environment the training and mode configuration may be handled
     *   by the platform/framework before the testcase runs.
     *   If explicit calls are required, uncomment the lines below.
     * ------------------------------------------------------------------ */
    /* bus_width = 0; */
    /* DBI_EN = 0;    */
    /* DM_EN = 1;     */
    /* MANUAL_REVIEW: speed grade selection — SG3200 default, SG2667, SG2133 */
    /* lpddr4_training(); */

    return 0;
}

/* ==========================================================================
 * FV entry point: run
 * ========================================================================== */

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Execute the LPDDR4 DM memory write/read test.
 *              Meta TestPlan K4 steps 5-9: Phase 1 (conditional) write 10
 *              values to port0, read-back port0, cross-port read-back port1;
 *              Phase 2 (unconditional) write 32 values to port1, read-back
 *              port1. Meta step 10: finish(err0) converted to FV status.
 * Parameters:
 *   cfg - framework test configuration
 *   out - framework test output structure
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_dm_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int index;
    uintptr_t addr;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 DM output pointer is NULL");
        return -1;
    }

    out->status = 0;
    out->actual_len = 0;
    out->actual_pattern[0] = 0;

    LOGT("Starting LPDDR4 DM write/read validation");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ==================================================================
     * Phase 1 (Conditional): Meta TestPlan K4 steps 5-7
 *
     * Step 5: Write 10 random 64-bit data values to port 0 at 0x1000 spacing.
 * Step 6: Read back all 10 values from port 0 and validate.
 * Step 7: Read back all 10 values from port 1 (cross-port) and validate.
 *
     * Meta Validation F4 criteria 1-2:
 *   1. Phase 1 port0 read-back: validate read_data == exp_data_array[index]
 *      for each index 0-9.
 *   2. Phase 1 port1 cross-port read-back: validate read_data ==
 *      exp_data_array[index] for each index 0-9.
     * ================================================================== */
#ifdef LPDDR4_DM_PHASE1_ENABLE
    LOGT("Phase 1: port0 write/read, %u locations, stride=0x%lx",
         LPDDR4_DM_PHASE1_LOCATIONS,
         (unsigned long)LPDDR4_DM_PHASE1_STRIDE);

    /* Step 5: Write 10 values to port 0 at 0x1000 spacing */
    /* Meta: writes random 64-bit data — replaced with deterministic pattern */
    for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
        exp_data_array[index] = (unsigned long int)LPDDR4_DM_PATTERN(index, LPDDR4_DM_PHASE1_STRIDE);
        addr = g_ctx.port0_addr +
               ((uintptr_t)index * (uintptr_t)LPDDR4_DM_PHASE1_STRIDE);
        writel_reg(addr, (uint64_t)exp_data_array[index]);
    }

    /* Step 6: Read back all 10 values from port 0 and validate */
    LOGT("Phase 1: port0 read-back validation");
    for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
        (void)lpddr4_dm_check_location(g_ctx.port0_addr,
                                       (unsigned long int)(index * LPDDR4_DM_PHASE1_STRIDE),
                                       index,
                                       (uint64_t)exp_data_array[index]);
    }

    /* Step 7: Read back all 10 values from port 1 (cross-port) and validate */
    LOGT("Phase 1: port1 cross-port read-back validation");
    for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
        (void)lpddr4_dm_check_location(g_ctx.port1_addr,
                                       (unsigned long int)(index * LPDDR4_DM_PHASE1_STRIDE),
                                       index,
                                       (uint64_t)exp_data_array[index]);
    }
#endif /* LPDDR4_DM_PHASE1_ENABLE */

    /* ==================================================================
     * Phase 2 (Unconditional): Meta TestPlan K4 steps 8-9
 *
     * Step 8: Write 32 random 64-bit data values to port 1 at 0x20000000
 *          spacing.
 * Step 9: Read back all 32 values from port 1 and validate.
 *
     * Meta Validation F4 criterion 3:
 *   Phase 2 port1 read-back: validate read_data == exp_data_array[index]
 *   for each index 0-31.
     * ================================================================== */
    LOGT("Phase 2: port1 write/read, %u locations, stride=0x%lx",
         LPDDR4_DM_PHASE2_LOCATIONS,
         (unsigned long)LPDDR4_DM_PHASE2_STRIDE);

    /* Step 8: Write 32 values to port 1 at 0x20000000 spacing */
    /* Meta: writes random 64-bit data — replaced with deterministic pattern */
    for (index = 0UL; index < LPDDR4_DM_PHASE2_LOCATIONS; index++) {
        exp_data_array[index] = (unsigned long int)LPDDR4_DM_PATTERN(index, LPDDR4_DM_PHASE2_STRIDE);
        addr = g_ctx.port1_addr +
               ((uintptr_t)index * (uintptr_t)LPDDR4_DM_PHASE2_STRIDE);
        writel_reg(addr, (uint64_t)exp_data_array[index]);
    }

    /* Step 9: Read back all 32 values from port 1 and validate */
    LOGT("Phase 2: port1 read-back validation");
    for (index = 0UL; index < LPDDR4_DM_PHASE2_LOCATIONS; index++) {
        (void)lpddr4_dm_check_location(g_ctx.port1_addr,
                                       (unsigned long int)(index * LPDDR4_DM_PHASE2_STRIDE),
                                       index,
                                       (uint64_t)exp_data_array[index]);
    }

    /* ------------------------------------------------------------------
     * Meta step 10: finish(err0) — prohibited DV construct.
 * Converted to FV status via out->status.
 * Meta Validation F4 criterion 4: test passes if err0 == 0.
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
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - framework test configuration
 * Returns:
 *   0 on PASS, -1 on FAIL.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 DM teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
