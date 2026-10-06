// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_access_test.h"
#include "test_define.inc"

static unsigned int err0;
static unsigned long int port0_addr;
static unsigned long int port1_addr;
static unsigned long int ctl_base;
static unsigned long int phy_base;
static unsigned int bus_width;
static unsigned int SG;
static unsigned int DBI_EN;
static unsigned int DM_EN;

/*
 * Deterministic 64-bit data pattern generator.
 * Replaces DV rand() usage. Each call_index produces a unique pattern.
 * MANUAL_REVIEW: Original DV used rand() to generate 64-bit random data.
 * Replace these deterministic patterns with appropriate values if needed.
 /
static unsigned long int deterministic_data(unsigned int call_index)
{
 unsigned long int val;
 val = (unsigned long int)0xA5A5A5A5UL | ((unsigned long int)(0x5A5A5A5AUL ^ ((unsigned long int)call_index * 0x01010101UL)) << 32);
 val = val ^ ((unsigned long int)call_index * 0x0123456789ABCDEFULL);
 return val;
}

/*
 * Function: check_mem_access
 * Description: Verifies memory access at multiple data widths (8/16/32/64-bit)
 * for both read and write operations at the given address.
 * Parameters:
 * addr - target memory address to test
 * data_index - deterministic data pattern index
 * Returns:
 * void (errors accumulated in global err0)
 */
static void check_mem_access(unsigned long int addr, unsigned int data_index)
{
 unsigned int i;
 char *ptr8;
 char data8;
 unsigned short int *ptr16;
 unsigned short int data16;
 unsigned int *ptr32;
 unsigned int data32;
 unsigned long int ptr64;
 unsigned long int data64;
 unsigned long int data;

 /* Steps 18-19: Generate deterministic 64-bit data (replaces rand()) /
 / MANUAL_REVIEW: Original DV used rand() to produce random 64-bit data. /
 / Using deterministic pattern instead. */
 data = deterministic_data(data_index);

 /* Steps 20-23: Set up typed pointers to data */
 ptr8 = (char *)&data;
 ptr16 = (unsigned short int *)&data;
 ptr32 = (unsigned int *)&data;
 ptr64 = (unsigned long int )&data;

 /* Step 24: Print address and data */
 LOGT("addr: 0x%lx, data: 0x%016lx", (unsigned long)addr, (unsigned long)data);

 /* Step 25: Write 64-bit data to target address */
 write_reg64(addr, data);

 /* Steps 26-29: 8-bit read verification, i = 0 to 7 */
 for (i = 0; i < 8; i++) {
 data8 = read_reg8(addr + i);
 if (data8 != (ptr8 + i)) {
 LOGE("8-bit mismatch at addr=0x%lx, read=0x%02x, expected=0x%02x",
 (unsigned long)(addr + i),
 (unsigned char)data8,
 (unsigned char)((ptr8 + i)));
 err0++;
 }
 }

 /* Steps 30-33: 16-bit read verification, i = 0 to 3 */
 for (i = 0; i < 4; i++) {
 data16 = read_reg16(addr + (i * 2));
 if (data16 != (ptr16 + i)) {
 LOGE("16-bit mismatch at addr=0x%lx, read=0x%04x, expected=0x%04x",
 (unsigned long)(addr + (i * 2)),
 (unsigned int)data16,
 (unsigned int)((ptr16 + i)));
 err0++;
 }
 }

 /* Steps 34-37: 32-bit read verification, i = 0 to 1 */
 for (i = 0; i < 2; i++) {
 data32 = read_reg(addr + (i * 4));
 if (data32 != (ptr32 + i)) {
 LOGE("32-bit mismatch at addr=0x%lx, read=0x%08x, expected=0x%08x",
 (unsigned long)(addr + (i * 4)),
 (unsigned int)data32,
 (unsigned int)((ptr32 + i)));
 err0++;
 }
 }

 /* Steps 38-39: 64-bit read verification */
 read_reg64(addr, &data64);
 if (data64 != *ptr64) {
 LOGE("64-bit mismatch at addr=0x%lx, read=0x%016lx, expected=0x%016lx",
 (unsigned long)addr,
 (unsigned long)data64,
 (unsigned long)(ptr64));
 err0++;
 }

 /* Step 40: Advance address by 0x1000 */
 addr = addr + 0x1000;

 /* Steps 41-42: Generate new deterministic 64-bit data (replaces rand()) /
 / MANUAL_REVIEW: Original DV used rand() to produce random 64-bit data. */
 data = deterministic_data(data_index + 100);

 /* Step 43: Print address and data */
 LOGT("addr: 0x%lx, data: 0x%016lx", (unsigned long)addr, (unsigned long)data);

 /* Step 44: Write byte 0 */
 write_reg8(addr + 0, (ptr8 + 0));

 /* Step 45: Write byte 1 */
 write_reg8(addr + 1, (ptr8 + 1));

 /* Step 46: Write halfword at offset 2 */
 write_reg16(addr + 2, (ptr16 + 1));

 /* Step 47: Write word at offset 4 */
 write_reg(addr + 4, (ptr32 + 1));

 /* Steps 48-49: Read back full 64-bit value and validate */
 read_reg64(addr, &data64);
 if (data64 != data) {
 LOGE("Mixed-width write mismatch at addr=0x%lx, expected=0x%016lx, read=0x%016lx",
 (unsigned long)addr,
 (unsigned long)data,
 (unsigned long)data64);
 /* Step 49: error printed (err0 not explicitly incremented per Meta for this check) */
 }

 /* Step 50: Return from check_mem_access */
}

/*
 * Function: lpddr4_mem_access_test_init
 * Description: Initializes LPDDR4 memory access test - sets up port addresses,
 * controller/PHY bases, speed grade, and performs training.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_init(const TestsItem cfg)
{
 (void)cfg;

 LOGT("LPDDR4 memory access test init");

 /* Step 3: gpv_programming() is a prohibited DV construct /
 / MANUAL_REVIEW: gpv_programming() was called in DV. Provide FV/PSV equivalent if needed. */

 /* Step 4: Set err0 = 0 */
 err0 = 0;

 /* Steps 5-6: Set port addresses and controller/PHY bases based on DRAM config */
#if defined(APS_DRAM)
 port0_addr = 0;
 port1_addr = 0x15A0000000UL;
 ctl_base = 0x9EE03000UL;
 phy_base = 0x9F000000UL;
#else /* default / MPS_DRAM */
 port0_addr = 0;
 port1_addr = 0x11A0000000UL;
 ctl_base = 0x11D003000UL;
 phy_base = 0x11D500000UL;
#endif

 /* Step 7: Set bus_width = 0 (Full Bus) */
 bus_width = 0;

 /* Steps 8-10: Set speed grade based on defines */
#if defined(SG2667)
 SG = 2667;
#elif defined(SG2133)
 SG = 2133;
#else
 SG = 3200;
#endif

 /* Step 11: Set DBI_EN = 0 */
 DBI_EN = 0;

 /* Step 12: Set DM_EN = 0 */
 DM_EN = 0;

 LOGT("port0_addr=0x%lx port1_addr=0x%lx ctl_base=0x%lx phy_base=0x%lx",
 (unsigned long)port0_addr,
 (unsigned long)port1_addr,
 (unsigned long)ctl_base,
 (unsigned long)phy_base);
 LOGT("bus_width=%u SG=%u DBI_EN=%u DM_EN=%u",
 bus_width, SG, DBI_EN, DM_EN);

 /* Step 13: Call lpddr4_training() */
 lpddr4_training();

 LOGT("LPDDR4 training complete");

 return 0;
}

/*
 * Function: lpddr4_mem_access_test_run
 * Description: Executes LPDDR4 multi-width memory access verification on
 * port0 (conditionally) and port1 + 0x100000.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting LPDDR4 multi-width memory access test");

 /* Step 14: Conditionally call check_mem_access(port0_addr) /
 / Condition: NOT ((APS_DRAM && A53_INITIATOR) XOR (MPS_DRAM && AI_INITIATOR) XOR (MPS_DRAM && DSP_INITIATOR)) */
#if !( \
 (defined(APS_DRAM) && defined(A53_INITIATOR)) ^ \
 (defined(MPS_DRAM) && defined(AI_INITIATOR)) ^ \
 (defined(MPS_DRAM) && defined(DSP_INITIATOR)) \
 )
 LOGT("Calling check_mem_access on port0_addr=0x%lx", (unsigned long)port0_addr);
 check_mem_access(port0_addr, 0);
#endif

 /* Step 15: Unconditionally call check_mem_access(port1_addr + 0x100000) */
 LOGT("Calling check_mem_access on port1_addr + 0x100000 = 0x%lx",
 (unsigned long)(port1_addr + 0x100000UL));
 check_mem_access(port1_addr + 0x100000UL, 1);

 /* Step 16: finish(err0) replaced with FV/PSV status handling */
 out->status = (err0 == 0) ? 0 : -1;

 LOGT("Run complete: %s, err0=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 err0);

 return out->status;
}

/*
 * Function: lpddr4_mem_access_test_teardown
 * Description: Final cleanup and status reporting for LPDDR4 memory access test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_mem_access_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 memory access test teardown: err0=%u", err0);

 return (err0 == 0) ? 0 : -1;
}
