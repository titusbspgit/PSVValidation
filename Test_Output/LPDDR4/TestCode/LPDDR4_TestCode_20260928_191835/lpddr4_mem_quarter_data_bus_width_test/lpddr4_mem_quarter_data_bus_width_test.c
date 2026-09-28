// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_quarter_data_bus_width_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_quarter_data_bus_width_test
 * Description: Verifies LPDDR4 memory read-back correctness when operating in
 * quarter data bus width mode. Configures base addresses based on
 * APS_DRAM or MPS_DRAM, sets bus_width=2 (Quarter bus), performs
 * LPDDR4 training, then validates read-back at offsets 0x100 and
 * 0x108 against expected patterns 0x3333333333333333 and
 * 0x2222222222222222 for both port0 (conditional) and port1
 * (unconditional).
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
} lpddr4_mem_quarter_data_bus_width_test_ctx_t;

static lpddr4_mem_quarter_data_bus_width_test_ctx_t g_ctx;

/*
 * Function: lpddr4_mem_quarter_data_bus_width_test_init
 * Description: Performs testcase initialization and pre-condition setup for lpddr4_mem_quarter_data_bus_width_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_quarter_data_bus_width_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_ctx = (lpddr4_mem_quarter_data_bus_width_test_ctx_t){0};

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

 /* Step 3: Set bus_width=2 (Quarter bus), DBI_EN=0, DM_EN=0 */
 g_ctx.bus_width = 2U;
 DBI_EN = 0U;
 DM_EN = 0U;
 LOGT("bus_width=2 (Quarter bus), DBI_EN=0, DM_EN=0");

 /* Step 4: Set speed grade conditionally */
#if defined(SG2667)
 g_ctx.sg = 2667U;
#elif defined(SG2133)
 g_ctx.sg = 2133U;
#else
 g_ctx.sg = 3200U;
#endif
 LOGT("Speed grade SG=%u", g_ctx.sg);

 /* Step 5: Call lpddr4_training() to perform LPDDR4 training */
 lpddr4_training();
 LOGT("lpddr4_training() called");

 /* Step 6: Call training_done() to confirm training completion */
 training_done();
 LOGT("training_done() called");

 /* Step 7: Call wait_on(1000) to wait 1000 time units */
 wait_on(1000);
 LOGT("wait_on(1000) called");

 LOGT("LPDDR4 quarter data bus width test init: port0=0x%lx port1=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 return 0;
}

/*
 * Function: lpddr4_mem_quarter_data_bus_width_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_quarter_data_bus_width_test.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_quarter_data_bus_width_test_run(const TestsItem *cfg, TestOutput out)
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

 LOGT("Starting LPDDR4 quarter data bus width read-back validation");
 LOGT("port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 /* Step 8: Port0 verification (conditional on initiator/DRAM macro combination) */
 // MANUAL_REVIEW: The exact initiator/DRAM configuration macro combination
 // (described as XOR expression) for conditionally compiling Port0 verification
 // was not fully specified. Add the appropriate #if condition as needed.
#if defined(APS_DRAM) || defined(MPS_DRAM)
 {
 LOGT("Port0 verification: reading at offsets 0x100 and 0x108");

 /* Step 8a: read_reg64(port0_addr + 0x100, &read_data0) */
 read_reg64(g_ctx.port0_addr + LPDDR4_OFFSET_0, &read_data0);
 LOGT("read_reg64(port0_addr+0x100) = 0x%llx",
 (unsigned long long)read_data0);

 /* Step 8b: read_reg64(port0_addr + 0x108, &read_data1) */
 read_reg64(g_ctx.port0_addr + LPDDR4_OFFSET_1, &read_data1);
 LOGT("read_reg64(port0_addr+0x108) = 0x%llx",
 (unsigned long long)read_data1);

 /* Step 8c: Check if both values mismatch expected patterns */
 g_ctx.checks_total++;
 if ((read_data0 != LPDDR4_EXPECTED_DATA0) &&
 (read_data1 != LPDDR4_EXPECTED_DATA1)) {
 LOGE("Port0 ERROR: read_data0=0x%llx (exp=0x%llx) read_data1=0x%llx (exp=0x%llx)",
 (unsigned long long)read_data0,
 (unsigned long long)LPDDR4_EXPECTED_DATA0,
 (unsigned long long)read_data1,
 (unsigned long long)LPDDR4_EXPECTED_DATA1);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 LOGT("Port0 verification PASSED");
 }
 }
#endif

 /* Step 9: Port1 verification (unconditional) */
 LOGT("Port1 verification: reading at offsets 0x100 and 0x108");

 /* Step 9a: read_reg64(port1_addr + 0x100, &read_data2) */
 read_reg64(g_ctx.port1_addr + LPDDR4_OFFSET_0, &read_data2);
 LOGT("read_reg64(port1_addr+0x100) = 0x%llx",
 (unsigned long long)read_data2);

 /* Step 9b: read_reg64(port1_addr + 0x108, &read_data3) */
 read_reg64(g_ctx.port1_addr + LPDDR4_OFFSET_1, &read_data3);
 LOGT("read_reg64(port1_addr+0x108) = 0x%llx",
 (unsigned long long)read_data3);

 /* Step 9c: Check if both values mismatch expected patterns */
 g_ctx.checks_total++;
 if ((read_data2 != LPDDR4_EXPECTED_DATA0) &&
 (read_data3 != LPDDR4_EXPECTED_DATA1)) {
 LOGE("Port1 ERROR: read_data2=0x%llx (exp=0x%llx) read_data3=0x%llx (exp=0x%llx)",
 (unsigned long long)read_data2,
 (unsigned long long)LPDDR4_EXPECTED_DATA0,
 (unsigned long long)read_data3,
 (unsigned long long)LPDDR4_EXPECTED_DATA1);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 LOGT("Port1 verification PASSED");
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
 * Function: lpddr4_mem_quarter_data_bus_width_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for lpddr4_mem_quarter_data_bus_width_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_quarter_data_bus_width_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 teardown: no additional cleanup required");
 return g_ctx.errors == 0U ? 0 : -1;
}
