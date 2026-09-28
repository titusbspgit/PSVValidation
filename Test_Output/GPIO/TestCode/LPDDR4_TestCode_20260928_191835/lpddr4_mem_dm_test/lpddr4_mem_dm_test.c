// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

/*
 * Test Case: lpddr4_mem_dm_test
 * Description: Validates LPDDR4 memory data mask (DM) functionality by
 *              performing write and read-back verification across two memory
 *              access patterns. Phase 1 (conditional): 10 locations via port0
 *              at 0x1000 stride, verified from port0 and port1. Phase 2
 *              (unconditional): 32 locations via port1 at 0x20000000 stride.
 */

/* --------------------------------------------------------------------------
 * Test context
 * -------------------------------------------------------------------------- */
typedef struct {
    uintptr_t port0_addr;
    uintptr_t port1_addr;
    uintptr_t ctl_base;
    uintptr_t phy_base;
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} lpddr4_dm_test_ctx_t;

static lpddr4_dm_test_ctx_t g_ctx;

/* --------------------------------------------------------------------------
 * lpddr4_check_location - Read and compare a single 64-bit memory location
 * -------------------------------------------------------------------------- */
static int lpddr4_check_location(uintptr_t base,
                                 unsigned long int offset,
                                 unsigned long int index,
                                 uint64_t expected)
{
    uint64_t actual;

    actual = read_reg64(base + (uintptr_t)offset);
    g_ctx.checks_total++;

    if (actual != expected) {
        LOGE("LPDDR4 read mismatch addr=0x%lx index=%lu exp=0x%llx actual=0x%llx",
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

/* --------------------------------------------------------------------------
 * FV Functions
 * -------------------------------------------------------------------------- */

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              lpddr4_mem_dm_test. Calls gpv_programming, configures port
 *              addresses, bus width, speed grade, DM/DBI settings, and
 *              performs LPDDR4 training.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_dm_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (lpddr4_dm_test_ctx_t){0};

    LOGT("lpddr4_mem_dm_test_init: start");

    /* Step 1: Call gpv_programming() for bus configuration */
    gpv_programming();

    /* Step 2: Set err0=0 (using g_ctx.errors) */
    g_ctx.errors = 0U;

    /* Step 3: Configure port addresses based on APS_DRAM or MPS_DRAM */
#if defined(APS_DRAM)
    g_ctx.port0_addr = 0UL;
    g_ctx.port1_addr = 0x15A0000000UL;
    g_ctx.ctl_base   = 0x9EE03000UL;
    g_ctx.phy_base   = 0x9F000000UL;
#elif defined(MPS_DRAM)
    g_ctx.port0_addr = 0UL;
    g_ctx.port1_addr = 0x11A0000000UL;
    g_ctx.ctl_base   = 0x11D003000UL;
    g_ctx.phy_base   = 0x11D500000UL;
#else
    LOGE("Neither APS_DRAM nor MPS_DRAM defined");
    return -1;
#endif

    LOGT("port0=0x%lx port1=0x%lx ctl_base=0x%lx phy_base=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr,
         (unsigned long)g_ctx.ctl_base,
         (unsigned long)g_ctx.phy_base);

    /* Step 4: Set bus_width=0 (Full Bus) */
    bus_width = 0U;

    /* Step 5: Set SG speed grade */
#if defined(SG2667)
    SG = 2667U;
#elif defined(SG2133)
    SG = 2133U;
#else
    SG = 3200U;
#endif

    /* Step 6: Set DBI_EN=0, DM_EN=1 */
    DBI_EN = 0U;
    DM_EN  = 1U;

    /* Step 7: Call lpddr4_training() */
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    LOGT("lpddr4_mem_dm_test_init: complete");
    return 0;
}

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_dm_test.
 *              Phase 1 (conditional): writes 10 random 64-bit values to port0
 *              at 0x1000 stride, reads back from port0 and port1.
 *              Phase 2 (unconditional): writes 32 random 64-bit values to port1
 *              at 0x20000000 stride, reads back from port1.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_dm_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int index;
    uint64_t expected;
    uintptr_t addr;
    uint64_t read_data;
    unsigned long offset;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("lpddr4_mem_dm_test_run: start");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ------------------------------------------------------------------ */
    /* Step 8: Phase 1 (conditional on initiator/subsystem preprocessor   */
    /*         guard)                                                     */
    /* ------------------------------------------------------------------ */
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

    LOGT("Phase 1: port0 write, port0 read, port1 read, %u locations, stride=0x%lx",
         LPDDR4_PHASE1_LOCATIONS,
         (unsigned long)LPDDR4_PHASE1_STRIDE);

    /* Step 8a: Set offset=0x1000 */
    offset = LPDDR4_PHASE1_STRIDE;

    /* Step 8b: Write 10 random 64-bit values to port0 */
    for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
        exp_data_array[index] = (uint64_t)rand() | ((uint64_t)((unsigned long)rand()) << 32);
        addr = g_ctx.port0_addr + ((uintptr_t)index * (uintptr_t)offset);
        write_reg64(addr, exp_data_array[index]);
        LOGT("Phase1 write index=%lu data=0x%llx addr=0x%lx",
             index,
             (unsigned long long)exp_data_array[index],
             (unsigned long)addr);
    }

    /* Step 8c: Read back from port0 and verify */
    LOGT("Phase 1: port0 read-back verification");
    for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
        expected = exp_data_array[index];
        (void)lpddr4_check_location(g_ctx.port0_addr,
                                    (unsigned long int)(index * LPDDR4_PHASE1_STRIDE),
                                    index,
                                    expected);
    }

    /* Step 8d: Read back from port1 and verify */
    LOGT("Phase 1: port1 read-back verification");
    for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
        expected = exp_data_array[index];
        (void)lpddr4_check_location(g_ctx.port1_addr,
                                    (unsigned long int)(index * LPDDR4_PHASE1_STRIDE),
                                    index,
                                    expected);
    }

#endif /* Phase 1 conditional guard */

    /* ------------------------------------------------------------------ */
    /* Step 9: Phase 2 (unconditional)                                    */
    /* ------------------------------------------------------------------ */
    LOGT("Phase 2: port1 write/read, %u locations, stride=0x%lx",
         LPDDR4_PHASE2_LOCATIONS,
         (unsigned long)LPDDR4_PHASE2_STRIDE);

    /* Step 9a: Set offset=0x20000000 */
    offset = LPDDR4_PHASE2_STRIDE;

    /* Step 9b: Write 32 random 64-bit values to port1 */
    for (index = 0UL; index < LPDDR4_PHASE2_LOCATIONS; index++) {
        exp_data_array[index] = (uint64_t)rand() | ((uint64_t)((unsigned long)rand()) << 32);
        addr = g_ctx.port1_addr + ((uintptr_t)index * (uintptr_t)offset);
        write_reg64(addr, exp_data_array[index]);
        LOGT("Phase2 write index=%lu data=0x%llx addr=0x%lx",
             index,
             (unsigned long long)exp_data_array[index],
             (unsigned long)addr);
    }

    /* Step 9c: Read back from port1 and verify */
    LOGT("Phase 2: port1 read-back verification");
    for (index = 0UL; index < LPDDR4_PHASE2_LOCATIONS; index++) {
        expected = exp_data_array[index];
        (void)lpddr4_check_location(g_ctx.port1_addr,
                                    (unsigned long int)(index * LPDDR4_PHASE2_STRIDE),
                                    index,
                                    expected);
    }

    /* Step 10: Report final status */
    // MANUAL_REVIEW: DV finish(err0) was present in the source flow. Converted to PSV/FV out->status reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              lpddr4_mem_dm_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_mem_dm_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
