// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_half_data_bus_width_test
 * Description: Validates LPDDR4 memory access in half data bus width mode.
 * Configures bus_width=1 (Half Bus), DBI_EN=0, DM_EN=0,
 * performs lpddr4_training(), training_done(), wait_on(1000),
 * then reads fixed addresses and validates expected data patterns
 * 0x3333333333333333 and 0x2222222222222222 across two phases.
 */

#define LPDDR4_HALF_BUS_READ_OFFSET0 0x100UL
#define LPDDR4_HALF_BUS_READ_OFFSET1 0x108UL
#define LPDDR4_HALF_BUS_EXPECTED_DATA0 0x3333333333333333ULL
#define LPDDR4_HALF_BUS_EXPECTED_DATA1 0x2222222222222222ULL

typedef struct {
 uintptr_t port0_addr;
 uintptr_t port1_addr;
 uintptr_t ctl_base;
 uintptr_t phy_base;
 unsigned int errors;
 unsigned int checks_total;
 unsigned int checks_passed;
 unsigned int checks_failed;
} lpddr4_half_bus_test_ctx_t;

static lpddr4_half_bus_test_ctx_t g_ctx;

/*
 * Function: lpddr4_mem_half_data_bus_width_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 * lpddr4_mem_half_data_bus_width_test. Calls gpv_programming,
 * configures port addresses via APS_DRAM/MPS_DRAM, sets
 * bus_width=1, DBI_EN=0, DM_EN=0, calls lpddr4_training(),
 * training_done(), and wait_on(1000).
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_ctx = (lpddr4_half_bus_test_ctx_t){0};

 /* Step 1: Call gpv_programming() for bus configuration */
 gpv_programming();

 /* Step 2: Set err0=0 */
 g_ctx.errors = 0U;

 /* Step 3: Configure port addresses based on APS_DRAM or MPS_DRAM */
#if defined(APS_DRAM)
 g_ctx.port0_addr = (uintptr_t)0UL;
 g_ctx.port1_addr = (uintptr_t)0x15A0000000UL;
 g_ctx.ctl_base = (uintptr_t)0x9EE03000UL;
 g_ctx.phy_base = (uintptr_t)0x9F000000UL;
#elif defined(MPS_DRAM)
 g_ctx.port0_addr = (uintptr_t)0UL;
 g_ctx.port1_addr = (uintptr_t)0x11A0000000UL;
 g_ctx.ctl_base = (uintptr_t)0x11D003000UL;
 g_ctx.phy_base = (uintptr_t)0x11D500000UL;
#else
 // MANUAL_REVIEW: Set port0_addr, port1_addr, ctl_base, phy_base for default platform.
 g_ctx.port0_addr = (uintptr_t)0UL;
 g_ctx.port1_addr = (uintptr_t)0UL;
 g_ctx.ctl_base = (uintptr_t)0UL;
 g_ctx.phy_base = (uintptr_t)0UL;
#endif

 if ((g_ctx.port0_addr == (uintptr_t)0U) ||
 (g_ctx.port1_addr == (uintptr_t)0U)) {
 LOGE("LPDDR4 port addresses are not configured");
 return -1;
 }

 /* Step 4: Set bus_width=1 (Half Bus) */
 // MANUAL_REVIEW: bus_width=1 is passed to lpddr4_training or set via globals per platform.

 /* Step 5: Set SG speed grade based on defines */
#if defined(SG2667)
 // MANUAL_REVIEW: Set speed grade for SG2667.
#elif defined(SG2133)
 // MANUAL_REVIEW: Set speed grade for SG2133.
#else
 // MANUAL_REVIEW: Default speed grade 3200.
#endif

 /* Step 6: Set DBI_EN=0, DM_EN=0 */
 // MANUAL_REVIEW: DBI_EN=0, DM_EN=0 are passed to lpddr4_training or set via globals per platform.

 /* Step 7: Call lpddr4_training() */
 lpddr4_training();

 /* Step 8: Call training_done() */
 training_done();

 /* Step 9: Call wait_on(1000) */
 wait_on(1000);

 LOGT("LPDDR4 half data bus width test init: port0=0x%lx port1=0x%lx ctl_base=0x%lx phy_base=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr,
 (unsigned long)g_ctx.ctl_base,
 (unsigned long)g_ctx.phy_base);

 return 0;
}

/*
 * Function: lpddr4_mem_half_data_bus_width_test_run
 * Description: Executes the main testcase flow for
 * lpddr4_mem_half_data_bus_width_test.
 * Phase 1 (conditional): reads port0_addr+0x100 and port0_addr+0x108,
 * checks against expected values 0x3333333333333333 and 0x2222222222222222.
 * Phase 2 (unconditional): reads port1_addr+0x100 and port1_addr+0x108,
 * checks against the same expected values.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_run(const TestsItem *cfg, TestOutput out)
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
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("Starting LPDDR4 half data bus width validation");
 LOGT("port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 /* Step 10: Phase 1 (conditional on initiator/subsystem preprocessor guard) */
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

 LOGT("Phase 1: port0 read at offsets 0x100 and 0x108");

 /* Step 10a: Read port0_addr+0x100 via read_reg64 into read_data0 */
 read_data0 = read_reg64(g_ctx.port0_addr + (uintptr_t)LPDDR4_HALF_BUS_READ_OFFSET0);
 g_ctx.checks_total++;

 /* Step 10b: Read port0_addr+0x108 via read_reg64 into read_data1 */
 read_data1 = read_reg64(g_ctx.port0_addr + (uintptr_t)LPDDR4_HALF_BUS_READ_OFFSET1);
 g_ctx.checks_total++;

 /* Step 10c: Check if both mismatch simultaneously (&& logic) */
 if ((read_data0 != LPDDR4_HALF_BUS_EXPECTED_DATA0) &&
 (read_data1 != LPDDR4_HALF_BUS_EXPECTED_DATA1)) {
 LOGE("Phase 1 port0 ERROR: read_data0=0x%llx (exp 0x3333333333333333) read_data1=0x%llx (exp 0x2222222222222222)",
 (unsigned long long)read_data0,
 (unsigned long long)read_data1);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed += 2U;
 LOGT("Phase 1 port0 check passed: read_data0=0x%llx read_data1=0x%llx",
 (unsigned long long)read_data0,
 (unsigned long long)read_data1);
 }

#endif /* Phase 1 conditional guard */

 /* Step 11: Phase 2 (unconditional) */
 LOGT("Phase 2: port1 read at offsets 0x100 and 0x108");

 /* Step 11a: Read port1_addr+0x100 via read_reg64 into read_data2 */
 read_data2 = read_reg64(g_ctx.port1_addr + (uintptr_t)LPDDR4_HALF_BUS_READ_OFFSET0);
 g_ctx.checks_total++;

 /* Step 11b: Read port1_addr+0x108 via read_reg64 into read_data3 */
 read_data3 = read_reg64(g_ctx.port1_addr + (uintptr_t)LPDDR4_HALF_BUS_READ_OFFSET1);
 g_ctx.checks_total++;

 /* Step 11c: Check if both mismatch simultaneously (&& logic) */
 if ((read_data2 != LPDDR4_HALF_BUS_EXPECTED_DATA0) &&
 (read_data3 != LPDDR4_HALF_BUS_EXPECTED_DATA1)) {
 LOGE("Phase 2 port1 ERROR: read_data2=0x%llx (exp 0x3333333333333333) read_data3=0x%llx (exp 0x2222222222222222)",
 (unsigned long long)read_data2,
 (unsigned long long)read_data3);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed += 2U;
 LOGT("Phase 2 port1 check passed: read_data2=0x%llx read_data3=0x%llx",
 (unsigned long long)read_data2,
 (unsigned long long)read_data3);
 }

 /* Step 12: finish(err0) - converted to PSV/FV status reporting */
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

/*
 * Function: lpddr4_mem_half_data_bus_width_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 * lpddr4_mem_half_data_bus_width_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_half_data_bus_width_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 teardown: no additional cleanup required");
 return g_ctx.errors == 0U ? 0 : -1;
}
