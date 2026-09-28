// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_dm_test
 * Description: Validates LPDDR4 memory data mask (DM) functionality by
 * performing write and read-back verification across two memory
 * access patterns. Configures DM_EN=1, DBI_EN=0, bus_width=0
 * (Full Bus), performs lpddr4_training(), then executes Phase 1
 * (conditional) and Phase 2 (unconditional) write/read checks.
 */

#define LPDDR4_DM_PHASE1_STRIDE 0x1000UL
#define LPDDR4_DM_PHASE1_LOCATIONS 10U
#define LPDDR4_DM_PHASE2_STRIDE 0x20000000UL
#define LPDDR4_DM_PHASE2_LOCATIONS 32U
#define LPDDR4_DM_TOTAL_CHECKS (LPDDR4_DM_PHASE1_LOCATIONS * 3U + LPDDR4_DM_PHASE2_LOCATIONS)

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

/*
 * Function: lpddr4_dm_check_location
 * Description: Reads a 64-bit value from the given base+offset and compares
 * against expected. Increments error/check counters.
 * Parameters:
 * base - Base address of the port.
 * offset - Byte offset from base.
 * index - Loop index for logging.
 * expected - Expected 64-bit data value.
 * Returns:
 * 0 on match, -1 on mismatch.
 */
static int lpddr4_dm_check_location(uintptr_t base,
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
 return -1;
 }

 g_ctx.checks_passed++;
 return 0;
}

/*
 * Function: lpddr4_mem_dm_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 * lpddr4_mem_dm_test. Calls gpv_programming, configures port
 * addresses via APS_DRAM/MPS_DRAM, sets bus_width, speed grade,
 * DBI_EN=0, DM_EN=1, and calls lpddr4_training().
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_ctx = (lpddr4_dm_test_ctx_t){0};

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

 /* Step 4: Set bus_width=0 (Full Bus) */
 // MANUAL_REVIEW: bus_width=0 is passed to lpddr4_training or set via globals per platform.

 /* Step 5: Set SG speed grade based on defines */
#if defined(SG2667)
 // MANUAL_REVIEW: Set speed grade for SG2667.
#elif defined(SG2133)
 // MANUAL_REVIEW: Set speed grade for SG2133.
#else
 // MANUAL_REVIEW: Default speed grade 3200.
#endif

 /* Step 6: Set DBI_EN=0, DM_EN=1 */
 // MANUAL_REVIEW: DBI_EN=0, DM_EN=1 are passed to lpddr4_training or set via globals per platform.

 /* Step 7: Call lpddr4_training() */
 lpddr4_training();

 LOGT("LPDDR4 DM test init: port0=0x%lx port1=0x%lx ctl_base=0x%lx phy_base=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr,
 (unsigned long)g_ctx.ctl_base,
 (unsigned long)g_ctx.phy_base);

 return 0;
}

/*
 * Function: lpddr4_mem_dm_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_dm_test.
 * Phase 1 (conditional): writes 10 random 64-bit values to port0
 * at 0x1000 stride, reads back from port0 and port1.
 * Phase 2 (unconditional): writes 32 random 64-bit values to port1
 * at 0x20000000 stride, reads back from port1.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int index;
 uint64_t expected;
 uint64_t read_data;
 uintptr_t addr;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 output pointer is NULL");
 return -1;
 }

 out->status = 0;
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("Starting LPDDR4 DM write/read validation");
 LOGT("port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 /* Step 8: Phase 1 (conditional on initiator/subsystem preprocessor guard) */
#if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^ (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ (defined(MPS_DRAM) && defined(DSP_INITIATOR)))

 LOGT("Phase 1: port0 write/read, %u locations, stride=0x%lx",
 LPDDR4_DM_PHASE1_LOCATIONS,
 (unsigned long)LPDDR4_DM_PHASE1_STRIDE);

 /* Step 8a: Set offset=0x1000 */
 /* Step 8b: Loop index 0..9: generate random data, write to port0 */
 for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
 expected = (uint64_t)rand() | ((uint64_t)((unsigned long)rand()) << 32);
 exp_data_array[index] = expected;
 addr = g_ctx.port0_addr +
 ((uintptr_t)index * (uintptr_t)LPDDR4_DM_PHASE1_STRIDE);
 write_reg64(addr, expected);
 }

 /* Step 8c: Loop index 0..9: read from port0, compare */
 for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
 expected = exp_data_array[index];
 (void)lpddr4_dm_check_location(g_ctx.port0_addr,
 (unsigned long int)(index * LPDDR4_DM_PHASE1_STRIDE),
 index,
 expected);
 }

 /* Step 8d: Loop index 0..9: read from port1, compare */
 for (index = 0UL; index < LPDDR4_DM_PHASE1_LOCATIONS; index++) {
 expected = exp_data_array[index];
 (void)lpddr4_dm_check_location(g_ctx.port1_addr,
 (unsigned long int)(index * LPDDR4_DM_PHASE1_STRIDE),
 index,
 expected);
 }

#endif /* Phase 1 conditional guard */

 /* Step 9: Phase 2 (unconditional) */
 LOGT("Phase 2: port1 write/read, %u locations, stride=0x%lx",
 LPDDR4_DM_PHASE2_LOCATIONS,
 (unsigned long)LPDDR4_DM_PHASE2_STRIDE);

 /* Step 9a: Set offset=0x20000000 */
 /* Step 9b: Loop index 0..31: generate random data, write to port1 */
 for (index = 0UL; index < LPDDR4_DM_PHASE2_LOCATIONS; index++) {
 expected = (uint64_t)rand() | ((uint64_t)((unsigned long)rand()) << 32);
 exp_data_array[index] = expected;
 addr = g_ctx.port1_addr +
 ((uintptr_t)index * (uintptr_t)LPDDR4_DM_PHASE2_STRIDE);
 write_reg64(addr, expected);
 }

 /* Step 9c: Loop index 0..31: read from port1, compare */
 for (index = 0UL; index < LPDDR4_DM_PHASE2_LOCATIONS; index++) {
 expected = exp_data_array[index];
 (void)lpddr4_dm_check_location(g_ctx.port1_addr,
 (unsigned long int)(index * LPDDR4_DM_PHASE2_STRIDE),
 index,
 expected);
 }

 /* Step 10: finish(err0) - converted to PSV/FV status reporting */
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
 * Function: lpddr4_mem_dm_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 * lpddr4_mem_dm_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_dm_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 DM test teardown: no additional cleanup required");
 return g_ctx.errors == 0U ? 0 : -1;
}
