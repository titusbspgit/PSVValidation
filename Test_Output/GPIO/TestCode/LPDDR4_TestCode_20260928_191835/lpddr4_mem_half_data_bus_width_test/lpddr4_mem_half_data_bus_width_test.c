// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/*
 * Test Case: lpddr4_mem_half_data_bus_width_test
 * Description: Validates LPDDR4 memory access in half data bus width mode.
 *              Configures bus_width=1 (Half Bus), DBI_EN=0, DM_EN=0, performs
 *              lpddr4_training(), training_done(), wait_on(1000), then reads
 *              two 64-bit values from fixed addresses in two phases and
 *              validates against expected patterns 0x3333333333333333 and
 *              0x2222222222222222 using && logic for error detection.
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
} lpddr4_half_bus_test_ctx_t;

static lpddr4_half_bus_test_ctx_t g_ctx;

/* --------------------------------------------------------------------------
 * FV Functions
 * -------------------------------------------------------------------------- */

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              lpddr4_mem_half_data_bus_width_test. Calls gpv_programming,
 *              configures port addresses, bus width, speed grade, DBI/DM
 *              settings, and performs LPDDR4 training.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (lpddr4_half_bus_test_ctx_t){0};

    LOGT("lpddr4_mem_half_data_bus_width_test_init: start");

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

    /* Step 4: Set bus_width=1 (Half Bus) */
    bus_width = 1U;

    /* Step 5: Set SG speed grade */
#if defined(SG2667)
    SG = 2667U;
#elif defined(SG2133)
    SG = 2133U;
#else
    SG = 3200U;
#endif

    /* Step 6: Set DBI_EN=0, DM_EN=0 */
    DBI_EN = 0U;
    DM_EN  = 0U;

    /* Step 7: Call lpddr4_training() */
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    /* Step 8: Call training_done() */
    training_done();

    /* Step 9: Call wait_on(1000) */
    wait_on(1000);

    LOGT("lpddr4_mem_half_data_bus_width_test_init: complete");
    return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes the main testcase flow for
 *              lpddr4_mem_half_data_bus_width_test. Phase 1 (conditional):
 *              reads port0_addr+0x100 and port0_addr+0x108, validates against
 *              expected patterns. Phase 2 (unconditional): reads port1_addr+0x100
 *              and port1_addr+0x108, validates against expected patterns.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput *out)
{
    uint64_t read_data0;
    uint64_t read_data1;
    uint64_t read_data2;
    uint64_t read_data3;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("lpddr4_mem_half_data_bus_width_test_run: start");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    /* ------------------------------------------------------------------ */
    /* Step 10: Phase 1 (conditional on initiator/subsystem preprocessor  */
    /*          guard)                                                    */
    /* ------------------------------------------------------------------ */
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

    LOGT("Phase 1: port0 read verification at offsets 0x100 and 0x108");

    /* Step 10a: Read port0_addr+0x100 via read_reg64 into read_data0 */
    read_data0 = read_reg64(g_ctx.port0_addr + LPDDR4_READ_OFFSET_0);
    LOGT("Phase1 read_data0=0x%llx from addr=0x%lx",
         (unsigned long long)read_data0,
         (unsigned long)(g_ctx.port0_addr + LPDDR4_READ_OFFSET_0));

    /* Step 10b: Read port0_addr+0x108 via read_reg64 into read_data1 */
    read_data1 = read_reg64(g_ctx.port0_addr + LPDDR4_READ_OFFSET_1);
    LOGT("Phase1 read_data1=0x%llx from addr=0x%lx",
         (unsigned long long)read_data1,
         (unsigned long)(g_ctx.port0_addr + LPDDR4_READ_OFFSET_1));

    /* Step 10c: Check if (read_data0 != EXPECTED_DATA_0) && (read_data1 != EXPECTED_DATA_1) */
    if ((read_data0 != LPDDR4_EXPECTED_DATA_0) && (read_data1 != LPDDR4_EXPECTED_DATA_1)) {
        LOGE("Phase1 port0 ERROR: read_data0=0x%llx (exp=0x%llx) read_data1=0x%llx (exp=0x%llx)",
             (unsigned long long)read_data0,
             (unsigned long long)LPDDR4_EXPECTED_DATA_0,
             (unsigned long long)read_data1,
             (unsigned long long)LPDDR4_EXPECTED_DATA_1);
        g_ctx.errors++;
    } else {
        LOGT("Phase1 port0 verification passed");
    }

#endif /* Phase 1 conditional guard */

    /* ------------------------------------------------------------------ */
    /* Step 11: Phase 2 (unconditional)                                   */
    /* ------------------------------------------------------------------ */
    LOGT("Phase 2: port1 read verification at offsets 0x100 and 0x108");

    /* Step 11a: Read port1_addr+0x100 via read_reg64 into read_data2 */
    read_data2 = read_reg64(g_ctx.port1_addr + LPDDR4_READ_OFFSET_0);
    LOGT("Phase2 read_data2=0x%llx from addr=0x%lx",
         (unsigned long long)read_data2,
         (unsigned long)(g_ctx.port1_addr + LPDDR4_READ_OFFSET_0));

    /* Step 11b: Read port1_addr+0x108 via read_reg64 into read_data3 */
    read_data3 = read_reg64(g_ctx.port1_addr + LPDDR4_READ_OFFSET_1);
    LOGT("Phase2 read_data3=0x%llx from addr=0x%lx",
         (unsigned long long)read_data3,
         (unsigned long)(g_ctx.port1_addr + LPDDR4_READ_OFFSET_1));

    /* Step 11c: Check if (read_data2 != EXPECTED_DATA_0) && (read_data3 != EXPECTED_DATA_1) */
    if ((read_data2 != LPDDR4_EXPECTED_DATA_0) && (read_data3 != LPDDR4_EXPECTED_DATA_1)) {
        LOGE("Phase2 port1 ERROR: read_data2=0x%llx (exp=0x%llx) read_data3=0x%llx (exp=0x%llx)",
             (unsigned long long)read_data2,
             (unsigned long long)LPDDR4_EXPECTED_DATA_0,
             (unsigned long long)read_data3,
             (unsigned long long)LPDDR4_EXPECTED_DATA_1);
        g_ctx.errors++;
    } else {
        LOGT("Phase2 port1 verification passed");
    }

    /* Step 12: Report final status */
    // MANUAL_REVIEW: DV finish(err0) was present in the source flow. Converted to PSV/FV out->status reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              lpddr4_mem_half_data_bus_width_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_mem_half_data_bus_width_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
