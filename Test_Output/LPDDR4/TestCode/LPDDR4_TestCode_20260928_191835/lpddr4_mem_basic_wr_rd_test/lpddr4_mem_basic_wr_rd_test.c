// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_basic_wr_rd_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_basic_wr_rd_test
 * Description: Verifies basic LPDDR4 memory write and read operations using
 * 64-bit data patterns across two memory ports. Configures base
 * addresses based on APS_DRAM or MPS_DRAM, performs LPDDR4
 * training, then validates write/read-back across Phase 1
 * (port0 and port1, stride 0x1000, 10 locations) and Phase 2
 * (port1, stride 0x20000000, 32 locations).
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
} lpddr4_mem_basic_wr_rd_test_ctx_t;

static lpddr4_mem_basic_wr_rd_test_ctx_t g_ctx;

static int lpddr4_check_location(uintptr_t base,
 unsigned long int offset,
 unsigned long int index,
 uint64_t expected)
{
 uint64_t actual;
 uint64_t read_data;

 read_reg64(base + (uintptr_t)offset, &read_data);
 actual = read_data;
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
 * Function: lpddr4_mem_basic_wr_rd_test_init
 * Description: Performs testcase initialization and pre-condition setup for lpddr4_mem_basic_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_basic_wr_rd_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_ctx = (lpddr4_mem_basic_wr_rd_test_ctx_t){0};

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

 /* Step 5: Call lpddr4_training() */
 lpddr4_training();
 LOGT("lpddr4_training() called");

 LOGT("LPDDR4 mem basic wr/rd test init: port0=0x%lx port1=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 return 0;
}

/*
 * Function: lpddr4_mem_basic_wr_rd_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_basic_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_basic_wr_rd_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned long int index;
 uintptr_t addr;
 uint64_t read_data;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 output pointer is NULL");
 return -1;
 }

 out->status = 0;
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("Starting LPDDR4 basic write/read validation");
 LOGT("port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 /* Step 6: Phase 1 (conditional on initiator/DRAM macro combination) */
 // MANUAL_REVIEW: The exact initiator/DRAM configuration macro combination
 // for conditionally compiling Phase 1 was not fully specified. Add the
 // appropriate #if condition as needed.
#if defined(APS_DRAM) || defined(MPS_DRAM)
 {
 LOGT("Phase 1: port0 write/read, 10 locations, stride=0x%lx",
 (unsigned long)LPDDR4_PHASE1_STRIDE);

 /* Step 6b: Write 10 random 64-bit values to port0 */
 for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
 exp_data_array[index] = (unsigned long)rand() | ((unsigned long)rand() << 32);
 addr = g_ctx.port0_addr +
 ((uintptr_t)index * (uintptr_t)LPDDR4_PHASE1_STRIDE);
 write_reg64(addr, exp_data_array[index]);
 }

 /* Step 6c: Read back from port0 and compare */
 for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
 (void)lpddr4_check_location(g_ctx.port0_addr,
 (unsigned long int)(index * LPDDR4_PHASE1_STRIDE),
 index,
 (uint64_t)exp_data_array[index]);
 }

 /* Step 6d: Read back from port1 and compare */
 for (index = 0UL; index < LPDDR4_PHASE1_LOCATIONS; index++) {
 (void)lpddr4_check_location(g_ctx.port1_addr,
 (unsigned long int)(index * LPDDR4_PHASE1_STRIDE),
 index,
 (uint64_t)exp_data_array[index]);
 }
 }
#endif

 /* Step 7: Phase 2 (unconditional) */
 LOGT("Phase 2: port1 write/read, 32 locations, stride=0x%lx",
 (unsigned long)LPDDR4_PHASE2_STRIDE);

 /* Step 7b: Write 32 random 64-bit values to port1 */
 for (index = 0UL; index < LPDDR4_PHASE2_LOCATIONS; index++) {
 exp_data_array[index] = (unsigned long)rand() | ((unsigned long)rand() << 32);
 addr = g_ctx.port1_addr +
 ((uintptr_t)index * (uintptr_t)LPDDR4_PHASE2_STRIDE);
 write_reg64(addr, exp_data_array[index]);
 }

 /* Step 7c: Read back from port1 and compare */
 for (index = 0UL; index < LPDDR4_PHASE2_LOCATIONS; index++) {
 (void)lpddr4_check_location(g_ctx.port1_addr,
 (unsigned long int)(index * LPDDR4_PHASE2_STRIDE),
 index,
 (uint64_t)exp_data_array[index]);
 }

 g_ctx.checks_failed = g_ctx.errors;

 out->status = (g_ctx.errors == 0U) ? 0 : -1;
 out->actual_len = 1;
 out->actual_pattern[0] = (int)g_ctx.errors;

 if (g_ctx.checks_total != LPDDR4_TOTAL_CHECKS) {
 LOGE("Unexpected LPDDR4 check count: expected=%u actual=%u",
 LPDDR4_TOTAL_CHECKS,
 g_ctx.checks_total);
 out->status = -1;
 }

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
 * Function: lpddr4_mem_basic_wr_rd_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for lpddr4_mem_basic_wr_rd_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_basic_wr_rd_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 teardown: no additional cleanup required");
 return g_ctx.errors == 0U ? 0 : -1;
}
