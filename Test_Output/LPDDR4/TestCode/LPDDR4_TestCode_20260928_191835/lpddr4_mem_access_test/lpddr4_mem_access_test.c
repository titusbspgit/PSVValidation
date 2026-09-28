// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_access_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_mem_access_test
 * Description: Verifies LPDDR4 memory access at multiple data widths
 * (8-bit, 16-bit, 32-bit, 64-bit) for both read and write
 * operations. Configures base addresses based on APS_DRAM or
 * MPS_DRAM, performs LPDDR4 training, then validates multi-width
 * read-back and mixed-width write verification.
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
} lpddr4_mem_access_ctx_t;

static lpddr4_mem_access_ctx_t g_ctx;

static void check_mem_access(uintptr_t addr)
{
 unsigned long data;
 uint64_t data64;
 unsigned char *ptr8;
 unsigned short *ptr16;
 unsigned int *ptr32;
 uint64_t ptr64;
 int i;

 /* Phase 1: Full-width write then multi-width read-back */
 LOGT("check_mem_access: Phase 1 at addr=0x%lx", (unsigned long)addr);

 /* Step 8a: Generate random 64-bit data */
 data = (unsigned long)rand() | ((unsigned long)rand() << 32);

 /* Step 8b: Set typed pointers to data */
 ptr8 = (unsigned char *)&data;
 ptr16 = (unsigned short *)&data;
 ptr32 = (unsigned int *)&data;
 ptr64 = (uint64_t *)&data;

 /* Step 8c: Write 64-bit data */
 write_reg64(addr, data);
 LOGT("write_reg64 addr=0x%lx data=0x%lx", (unsigned long)addr, (unsigned long)data);

 /* Step 8d: Read back 8 bytes using read_reg8 */
 for (i = 0; i < 8; i++) {
 unsigned char data8 = read_reg8(addr + (uintptr_t)i);
 g_ctx.checks_total++;
 if (data8 != *(ptr8 + i)) {
 LOGE("8-bit mismatch addr=0x%lx i=%d exp=0x%x actual=0x%x",
 (unsigned long)(addr + (uintptr_t)i), i,
 (unsigned int)*(ptr8 + i), (unsigned int)data8);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 }
 }

 /* Step 8e: Read back 4 half-words using read_reg16 */
 for (i = 0; i < 4; i++) {
 unsigned short data16 = read_reg16(addr + (uintptr_t)(i * 2));
 g_ctx.checks_total++;
 if (data16 != *(ptr16 + i)) {
 LOGE("16-bit mismatch addr=0x%lx i=%d exp=0x%x actual=0x%x",
 (unsigned long)(addr + (uintptr_t)(i * 2)), i,
 (unsigned int)*(ptr16 + i), (unsigned int)data16);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 }
 }

 /* Step 8f: Read back 2 words using read_reg */
 for (i = 0; i < 2; i++) {
 unsigned int data32 = read_reg(addr + (uintptr_t)(i * 4));
 g_ctx.checks_total++;
 if (data32 != *(ptr32 + i)) {
 LOGE("32-bit mismatch addr=0x%lx i=%d exp=0x%x actual=0x%x",
 (unsigned long)(addr + (uintptr_t)(i * 4)), i,
 *(ptr32 + i), data32);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 }
 }

 /* Step 8g: Read back 64-bit value using read_reg64 */
 read_reg64(addr, &data64);
 g_ctx.checks_total++;
 if (data64 != *ptr64) {
 LOGE("64-bit mismatch addr=0x%lx exp=0x%llx actual=0x%llx",
 (unsigned long)addr,
 (unsigned long long)*ptr64,
 (unsigned long long)data64);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 }

 /* Phase 2: Mixed-width writes at addr + 0x1000 */
 addr = addr + 0x1000UL;
 LOGT("check_mem_access: Phase 2 (mixed-width) at addr=0x%lx", (unsigned long)addr);

 /* Step 8i: Generate new random 64-bit data */
 data = (unsigned long)rand() | ((unsigned long)rand() << 32);
 ptr8 = (unsigned char *)&data;
 ptr16 = (unsigned short *)&data;
 ptr32 = (unsigned int *)&data;

 /* Step 8j: Perform mixed-width writes */
 write_reg8(addr + 0U, *(ptr8 + 0));
 write_reg8(addr + 1U, *(ptr8 + 1));
 write_reg16(addr + 2U, *(ptr16 + 1));
 write_reg(addr + 4U, *(ptr32 + 1));

 /* Step 8k: Read back 64-bit value and compare */
 read_reg64(addr, &data64);
 g_ctx.checks_total++;
 if (data64 != (uint64_t)data) {
 LOGE("Mixed-width mismatch addr=0x%lx exp=0x%lx actual=0x%llx",
 (unsigned long)addr,
 (unsigned long)data,
 (unsigned long long)data64);
 g_ctx.errors++;
 } else {
 g_ctx.checks_passed++;
 }
}

/*
 * Function: lpddr4_mem_access_test_init
 * Description: Performs testcase initialization and pre-condition setup for lpddr4_mem_access_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_init(const TestsItem *cfg)
{
 (void)cfg;

 g_ctx = (lpddr4_mem_access_ctx_t){0};

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

 LOGT("LPDDR4 mem access test init: port0=0x%lx port1=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 return 0;
}

/*
 * Function: lpddr4_mem_access_test_run
 * Description: Executes the main testcase flow for lpddr4_mem_access_test.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_run(const TestsItem *cfg, TestOutput *out)
{
 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 output pointer is NULL");
 return -1;
 }

 out->status = 0;
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("Starting LPDDR4 multi-width memory access validation");
 LOGT("port0_addr=0x%lx port1_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr);

 /* Step 6: Conditionally call check_mem_access(port0_addr) */
 // MANUAL_REVIEW: The exact initiator/DRAM configuration macro combination
 // for conditionally calling check_mem_access(port0_addr) was not fully
 // specified. Add the appropriate #if condition as needed.
#if defined(APS_DRAM) || defined(MPS_DRAM)
 LOGT("Calling check_mem_access for port0_addr=0x%lx",
 (unsigned long)g_ctx.port0_addr);
 check_mem_access(g_ctx.port0_addr);
#endif

 /* Step 7: Call check_mem_access(port1_addr + 0x100000) */
 LOGT("Calling check_mem_access for port1_addr+0x100000=0x%lx",
 (unsigned long)(g_ctx.port1_addr + 0x100000UL));
 check_mem_access(g_ctx.port1_addr + 0x100000UL);

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
 * Function: lpddr4_mem_access_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for lpddr4_mem_access_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 mem access test teardown: errors=%u checks_total=%u",
 g_ctx.errors, g_ctx.checks_total);
 return g_ctx.errors == 0U ? 0 : -1;
}
