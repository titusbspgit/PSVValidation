// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_boundary_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_boundary_test
 * Description: Verifies LPDDR4 memory data integrity at boundary address
 *              ranges up to 2GB and 16GB limits. Configures base addresses
 *              based on APS_DRAM or MPS_DRAM, performs LPDDR4 training, then
 *              validates write/read-back across Phase 1 (port0 and port1,
 *              boundary val_2g) and Phase 2 (port1, boundary val_16g).
 */

typedef struct {
    uintptr_t port0_addr;
    uintptr_t port1_addr;
    uintptr_t ctl_base;
    uintptr_t phy_base;
    unsigned int bus_width;
    unsigned int sg;
    unsigned int errors;
    unsigned int checks_total;
    unsigned int checks_passed;
    unsigned int checks_failed;
} lpddr4_mem_boundary_test_ctx_t;

static lpddr4_mem_boundary_test_ctx_t g_ctx;

static int lpddr4_check_location(uintptr_t base,
                                  unsigned long int offset,
                                  unsigned long int index,
                                  uint64_t expected)
{
    uint64_t actual;

    actual = readl_reg(base + (uintptr_t)offset);
    g_ctx.checks_total++;

    if (actual != expected) {
        LOGE("LPDDR4 read mismatch addr=0x%lx index=%lu exp=0x%llx actual=0x%llx",
             (unsigned long)(base + (uintptr_t)offset),
             index,
             (unsigned long long)expected,
             (unsigned long long)actual);
        g_ctx.errors++;
        return -1;
    }

    g_ctx.checks_passed++;
    return 0;
}

/*
 * Function: lpddr4_mem_boundary_test_init
 * Description: Performs testcase initialization and pre-condition setup for lpddr4_mem_boundary_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_boundary_test_init(const TestsItem cfg)
{
    (void)cfg;

    g_ctx = (lpddr4_mem_boundary_test_ctx_t){0};

    /* Step 1: Call gpv_programming() for GPV configuration */
    gpv_programming();
    LOGT("gpv_programming() called");

    /* Step 2: Configure base addresses based on APS_DRAM or MPS_DRAM */
    g_ctx.port0_addr = 0UL;

#if defined(APS_DRAM)
    g_ctx.port1_addr = 0x15A0000000UL;
    g_ctx.ctl_base = 0x9EE03000UL;
    g_ctx.phy_base = 0x9F000000UL;
    LOGT("APS_DRAM config: port1=0x%lx ctl=0x%lx phy=0x%lx",
         (unsigned long)g_ctx.port1_addr,
         (unsigned long)g_ctx.ctl_base,
         (unsigned long)g_ctx.phy_base);
#elif defined(MPS_DRAM)
    g_ctx.port1_addr = 0x11A0000000UL;
    g_ctx.ctl_base = 0x11D003000UL;
    g_ctx.phy_base = 0x11D500000UL;
    LOGT("MPS_DRAM config: port1=0x%lx ctl=0x%lx phy=0x%lx",
         (unsigned long)g_ctx.port1_addr,
         (unsigned long)g_ctx.ctl_base,
         (unsigned long)g_ctx.phy_base);
#else
    LOGE("Neither APS_DRAM nor MPS_DRAM is defined");
    return -1;
#endif

    /* Step 3: Set bus_width, DBI_EN, DM_EN */
    g_ctx.bus_width = 0U;
    DBI_EN = 0U;
    DM_EN = 0U;
    LOGT("bus_width=0 (Full Bus), DBI_EN=0, DM_EN=0");

    /* Step 4: Set speed grade conditionally */
#if defined(SG2667)
    g_ctx.sg = 2667U;
#elif defined(SG2133)
    g_ctx.sg = 2133U;
#else
    g_ctx.sg = 3200U;
#endif
    LOGT("Speed grade SG=%u", g_ctx.sg);

    /* Step 6: Call lpddr4_training() */
    lpddr4_training();
    LOGT("lpddr4_training() called");

    LOGT("LPDDR4 mem boundary test init: port0=0x%lx port1=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);

    return 0;
}

/*
 * Function: lpddr4_mem_boundary_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_boundary_test.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_boundary_test_run(const TestsItem cfg, TestOutput out)
{
    unsigned long int index1;
    uintptr_t addr;
    unsigned long int offset;
    uint64_t read_data;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;
    out->actual_len = 0;
    out->actual_pattern[0] = 0;

    LOGT("Starting LPDDR4 boundary write/read validation");
    LOGT("port0_addr=0x%lx port1_addr=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr);
    LOGT("Boundary values: val_2g=0x%llx val_16g=0x%llx",
         (unsigned long long)LPDDR4_VAL_2G,
         (unsigned long long)LPDDR4_VAL_16G);

    /* Step 7: Phase 1 (conditional on initiator/DRAM macro combination) */
    // MANUAL_REVIEW: The exact initiator/DRAM configuration macro combination
    // for conditionally compiling Phase 1 was not fully specified. Add the
    // appropriate #if condition as needed.
#if defined(APS_DRAM) || defined(MPS_DRAM)
    {
        /* Step 7a: Set offset = PORT0_OFF if defined, else 0x1000000 */
#if defined(PORT0_OFF)
        offset = (unsigned long int)PORT0_OFF;
#else
        offset = 0x1000000UL;
#endif
        LOGT("Phase 1: port0 write/read, boundary=val_2g, offset=0x%lx",
             (unsigned long)offset);

        /* Step 7b: Write random 64-bit values to port0 while (index1offset) < val_2g */
        for (index1 = 0UL; ((uint64_t)index1 * (uint64_t)offset) < LPDDR4_VAL_2G; index1++) {
            exp_data_array[index1] = (unsigned long)rand() | ((unsigned long)rand() << 32);
            addr = g_ctx.port0_addr +
                   (uintptr_t)((uint64_t)index1 * (uint64_t)offset);
            write_reg64(addr, exp_data_array[index1]);
        }

        /* Step 7c: Read back from port0 and compare */
        for (index1 = 0UL; ((uint64_t)index1 * (uint64_t)offset) < LPDDR4_VAL_2G; index1++) {
            (void)lpddr4_check_location(g_ctx.port0_addr,
                                        (unsigned long int)((uint64_t)index1 * (uint64_t)offset),
                                        index1,
                                        (uint64_t)exp_data_array[index1]);
        }

        /* Step 7d: Read back from port1 and compare */
        for (index1 = 0UL; ((uint64_t)index1 * (uint64_t)offset) < LPDDR4_VAL_2G; index1++) {
            (void)lpddr4_check_location(g_ctx.port1_addr,
                                        (unsigned long int)((uint64_t)index1 * (uint64_t)offset),
                                        index1,
                                        (uint64_t)exp_data_array[index1]);
        }
    }
#endif

    /* Step 8: Phase 2 (unconditional) */
    /* Step 8a: Set offset = PORT1_OFF if defined, else 0x08000000 */
#if defined(PORT1_OFF)
    offset = (unsigned long int)PORT1_OFF;
#else
    offset = 0x08000000UL;
#endif
    LOGT("Phase 2: port1 write/read, boundary=val_16g, offset=0x%lx",
         (unsigned long)offset);

    /* Step 8b: Write random 64-bit values to port1 while (index1offset) < val_16g */
    for (index1 = 0UL; ((uint64_t)index1 * (uint64_t)offset) < LPDDR4_VAL_16G; index1++) {
        exp_data_array[index1] = (unsigned long)rand() | ((unsigned long)rand() << 32);
        addr = g_ctx.port1_addr +
               (uintptr_t)((uint64_t)index1 * (uint64_t)offset);
        write_reg64(addr, exp_data_array[index1]);
    }

    /* Step 8c: Read back from port1 and compare */
    for (index1 = 0UL; ((uint64_t)index1 * (uint64_t)offset) < LPDDR4_VAL_16G; index1++) {
        (void)lpddr4_check_location(g_ctx.port1_addr,
                                    (unsigned long int)((uint64_t)index1 * (uint64_t)offset),
                                    index1,
                                    (uint64_t)exp_data_array[index1]);
    }

    g_ctx.checks_failed = g_ctx.errors;

    out->status = (g_ctx.errors == 0U) ? 0 : -1;
    out->actual_len = 1;
    out->actual_pattern[0] = (int)g_ctx.errors;

    // MANUAL_REVIEW: DV finish(err0) was present in the source flow.
    // Converted to PSV/FV-native out->status based PASS/FAIL reporting.

    LOGT("Run complete: %s errors=%u checks_passed=%u checks_total=%u checks_failed=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors,
         g_ctx.checks_passed,
         g_ctx.checks_total,
         g_ctx.checks_failed);

    return out->status;
}

/*
 * Function: lpddr4_mem_boundary_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for lpddr4_mem_boundary_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_mem_boundary_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 teardown: no additional cleanup required");
    return g_ctx.errors == 0U ? 0 : -1;
}
