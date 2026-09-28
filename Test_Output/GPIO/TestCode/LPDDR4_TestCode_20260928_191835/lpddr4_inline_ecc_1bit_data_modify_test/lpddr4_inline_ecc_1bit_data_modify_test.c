// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_1bit_data_modify_test.h"
#include "test_define.inc"

/*
 * Testcase: lpddr4_inline_ecc_1bit_data_modify_test
 * Description: Validates LPDDR4 inline ECC 1-bit corrected error detection
 * with data modification across multiple ECC regions.
 * Configures ECC with ecc_region_map=0x55, initializes memory
 * via scrubber, writes random data, performs training, reads
 * back and checks for ECC corrected errors and interrupt behavior.
 /

#define ECC_REGION_MAP 0x55U
#define ECC_REGION_COUNT 7U
#define ECC_REGION_OFFSET 0x80000000UL
#define ECCCFG0_OFFSET 0x00000070UL
#define ECCCFG1_OFFSET 0x00000074UL
#define ECCSTAT_OFFSET 0x00000078UL
#define ECCCTL_OFFSET 0x0000007CUL
#define ECCERRCNT_OFFSET 0x00000080UL
#define SWCTL_OFFSET 0x00000320UL
#define PCTRL_0_OFFSET 0x00000490UL
#define PCTRL_1_OFFSET 0x00000540UL
#define SII_INTR_OFFSET 0x0000819CUL
#define SII_FAULT_OFFSET 0x00008194UL
#define SBRCTL_OFFSET 0x00000F24UL
#define SBRRANGE1_OFFSET 0x00000F28UL
#define SBRWDATA0_OFFSET 0x00000F2CUL
#define SBRWDATA1_OFFSET 0x00000F30UL
#define SBRSTART0_OFFSET 0x00000F00UL
#define SBRSTART1_OFFSET 0x00000F04UL
#define SBRRANGE0_OFFSET 0x00000F08UL
#define ECCSTAT_CORR_BIT (1U << 8)
#define ECCCTL_CORR_CLR 0x00000001U
#define ECCCTL_CORR_INT_EN (1U << 8)
#define ECCCFG1_VALUE 0x00000330U
#define EXPECTED_ERR_COUNT 5U

typedef struct {
 uintptr_t port0_addr;
 uintptr_t port1_addr;
 uintptr_t ctl_base;
 uintptr_t phy_base;
 unsigned int errors;
 unsigned int checks_total;
 unsigned int checks_passed;
 unsigned int checks_failed;
 volatile unsigned int int_pend;
 volatile unsigned int int_pend1;
 unsigned int int_expected;
 unsigned int sbr_init;
} ecc_test_ctx_t;

static ecc_test_ctx_t g_ctx;

/
 * Function: memory_init_scrb
 * Description: Initializes memory region via scrubber (SBR) for ECC.
 * Disables ports, configures scrub, waits for SBR done,
 * then restores ports.
 * Parameters:
 * region_addr - Target address for scrub region.
 * Returns:
 * void
 /
static void memory_init_scrb(unsigned long region_addr)
{
 unsigned int sbrctl_val;
 unsigned int timeout;

 LOGT("memory_init_scrb: starting scrub for region_addr=0x%lx",
 (unsigned long)region_addr);

 / Step 37: Disable PCTRL_0 and PCTRL_1 /
 writel_reg(g_ctx.ctl_base + PCTRL_0_OFFSET, 0x00000000U);
 writel_reg(g_ctx.ctl_base + PCTRL_1_OFFSET, 0x00000000U);

 / Set SBRCTL scrub_mode=1 (bit[2]), scrub_interval=0 (bits[31:8]) /
 sbrctl_val = (1U << 2);
 writel_reg(g_ctx.ctl_base + SBRCTL_OFFSET, sbrctl_val);

 / Write SBRWDATA0 and SBRWDATA1 with zeros /
 writel_reg(g_ctx.ctl_base + SBRWDATA0_OFFSET, 0x00000000U);
 writel_reg(g_ctx.ctl_base + SBRWDATA1_OFFSET, 0x00000000U);

 / Set SBRSTART0/SBRSTART1 and SBRRANGE0/SBRRANGE1 for target address /
 // MANUAL_REVIEW: SBRSTART and SBRRANGE register values depend on SoC address mapping.
 // The exact start/range values for the target region_addr must be configured per platform.
 writel_reg(g_ctx.ctl_base + SBRSTART0_OFFSET, (unsigned int)(region_addr & 0xFFFFFFFFUL));
 writel_reg(g_ctx.ctl_base + SBRSTART1_OFFSET, (unsigned int)((region_addr >> 32) & 0xFFFFFFFFUL));
 // MANUAL_REVIEW: SBRRANGE0/SBRRANGE1 values must be set per platform memory map.

 / Enable scrubbing (SBRCTL bit[0]=1) /
 g_ctx.sbr_init = 1U;
 g_ctx.int_pend1 = 1U;
 sbrctl_val = sbrctl_val | 0x00000001U;
 writel_reg(g_ctx.ctl_base + SBRCTL_OFFSET, sbrctl_val);

 / Wait for SBR done interrupt /
 timeout = 10000U;
 while ((g_ctx.int_pend1 != 0U) && (timeout > 0U)) {
 wait_on(10);
 timeout--;
 }
 if (timeout == 0U) {
 LOGE("memory_init_scrb: timeout waiting for SBR done interrupt");
 g_ctx.errors++;
 }

 / Restore scrub_mode=0, scrub_interval=1, disable scrub /
 sbrctl_val = (1U << 8);
 writel_reg(g_ctx.ctl_base + SBRCTL_OFFSET, sbrctl_val);

 / Re-enable PCTRL_0 and PCTRL_1 /
 writel_reg(g_ctx.ctl_base + PCTRL_0_OFFSET, 0x00000001U);
 writel_reg(g_ctx.ctl_base + PCTRL_1_OFFSET, 0x00000001U);

 g_ctx.sbr_init = 0U;

 LOGT("memory_init_scrb: scrub complete for region_addr=0x%lx",
 (unsigned long)region_addr);
}

/
 * Function: Default_IRQHandler
 * Description: IRQ handler for SBR done and ECC corrected error interrupts.
 * Reads SYSREG_INTR_MSTS_ADDR, identifies interrupt source,
 * clears the interrupt, and updates pending flags.
 * Parameters:
 * None
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 unsigned int msts;
 unsigned int sii_val;
 unsigned int eccstat;
 unsigned int eccctl;
 unsigned int sbrrange1;
 unsigned int timeout;

 / Read master interrupt status /
 msts = readl_reg(SYSREG_INTR_MSTS_ADDR);

 if ((msts & SYSREG_CTRL_INTR_MSTS_MASK) != 0U) {
 if (g_ctx.sbr_init != 0U) {
 / SBR done interrupt path /
 sii_val = readl_reg(g_ctx.ctl_base + SII_FAULT_OFFSET);

 if ((sii_val & SII_INTR_EN_SBR_DONE_INTR_MASK) != 0U) {
 / Poll SBRRANGE1 bit[0] until 0 /
 timeout = 10000U;
 do {
 sbrrange1 = readl_reg(g_ctx.ctl_base + SBRRANGE1_OFFSET);
 timeout--;
 } while (((sbrrange1 & 0x00000001U) != 0U) && (timeout > 0U));

 / Disable scrub via SBRCTL /
 writel_reg(g_ctx.ctl_base + SBRCTL_OFFSET, 0x00000000U);

 / Clear SBR interrupt in SII register /
 writel_reg(g_ctx.ctl_base + SII_FAULT_OFFSET, SII_INTR_EN_SBR_DONE_INTR_MASK);

 g_ctx.int_pend1 = 0U;
 }
 } else {
 / ECC corrected error interrupt path /
 eccstat = readl_reg(g_ctx.ctl_base + ECCSTAT_OFFSET);

 if ((eccstat & ECCSTAT_CORR_BIT) != 0U) {
 / Read-modify-write ECCCTL OR 0x00000001 to clear corrected error /
 eccctl = readl_reg(g_ctx.ctl_base + ECCCTL_OFFSET);
 eccctl = eccctl | ECCCTL_CORR_CLR;
 writel_reg(g_ctx.ctl_base + ECCCTL_OFFSET, eccctl);

 / Clear ECC interrupt in SII register /
 writel_reg(g_ctx.ctl_base + SII_FAULT_OFFSET, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);

 g_ctx.int_pend1 = 0U;
 }
 }

 / Clear controller interrupt /
 writel_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);
 GIC_ClearIRQ(CTL_INT_NO);
 }
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 * lpddr4_inline_ecc_1bit_data_modify_test. Enables GIC IRQ,
 * configures controller interrupt, sets up port addresses,
 * memory parameters, and performs LPDDR4 training.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_init(const TestsItem cfg)
{
 (void)cfg;

 g_ctx = (ecc_test_ctx_t){0};

 / Step 1: Set int_pend=1, enable GIC IRQ for CTL_INT_NO /
 g_ctx.int_pend = 1U;
 GIC_EnableIRQ(CTL_INT_NO);

 / Step 2: Write SYSREG_INTR_EN_ADDR to enable controller interrupt /
 writel_reg(SYSREG_INTR_EN_ADDR, SYSREG_CTRL_INTR_EN_MASK);

 / Step 3: Call gpv_programming() for bus configuration /
 gpv_programming();

 / Step 4: Set err0=0 /
 g_ctx.errors = 0U;

 / Step 5: Configure port addresses based on APS_DRAM or MPS_DRAM /
#if defined(APS_DRAM)
 // MANUAL_REVIEW: Set port0_addr, port1_addr, ctl_base, phy_base for APS_DRAM platform.
 g_ctx.port0_addr = 0UL;
 g_ctx.port1_addr = 0UL;
 g_ctx.ctl_base = 0UL;
 g_ctx.phy_base = 0UL;
#elif defined(MPS_DRAM)
 // MANUAL_REVIEW: Set port0_addr, port1_addr, ctl_base, phy_base for MPS_DRAM platform.
 g_ctx.port0_addr = 0UL;
 g_ctx.port1_addr = 0UL;
 g_ctx.ctl_base = 0UL;
 g_ctx.phy_base = 0UL;
#else
 // MANUAL_REVIEW: Set port0_addr, port1_addr, ctl_base, phy_base for default platform.
 g_ctx.port0_addr = 0UL;
 g_ctx.port1_addr = 0UL;
 g_ctx.ctl_base = 0UL;
 g_ctx.phy_base = 0UL;
#endif

 if ((g_ctx.port0_addr == (uintptr_t)0U) ||
 (g_ctx.port1_addr == (uintptr_t)0U) ||
 (g_ctx.ctl_base == (uintptr_t)0U)) {
 LOGE("LPDDR4 port/ctl addresses are not configured");
 return -1;
 }

 / Step 6: Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=1, ECC_EN=1 /
 // MANUAL_REVIEW: bus_width, DBI_EN, DM_EN, ECC_EN are passed to lpddr4_training or set via globals.
 // These parameters are platform-specific and must be configured per SoC.

 / Step 7: Set SG speed grade based on defines /
#if defined(SG2667)
 // MANUAL_REVIEW: Set speed grade for SG2667.
#elif defined(SG2133)
 // MANUAL_REVIEW: Set speed grade for SG2133.
#else
 // MANUAL_REVIEW: Default speed grade 3200.
#endif

 / Step 8: Call lpddr4_training() /
 lpddr4_training();

 LOGT("lpddr4_inline_ecc_1bit_data_modify_test init: port0=0x%lx port1=0x%lx ctl_base=0x%lx",
 (unsigned long)g_ctx.port0_addr,
 (unsigned long)g_ctx.port1_addr,
 (unsigned long)g_ctx.ctl_base);

 return 0;
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_run
 * Description: Executes the main testcase flow for
 * lpddr4_inline_ecc_1bit_data_modify_test. Configures ECC regions,
 * initializes memory via scrubber, writes data, reads back,
 * validates ECC corrected error detection and interrupt behavior.
 * Parameters:
 * cfg - Test configuration input.
 * out - Test output capture structure.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned int region;
 unsigned int ecc_region_map;
 unsigned long offset;
 uint64_t read_data;
 unsigned int eccstat;
 unsigned int eccctl;
 unsigned int count;
 unsigned int timeout;

 (void)cfg;

 if (out == 0) {
 LOGE("LPDDR4 ECC test output pointer is NULL");
 return -1;
 }

 out->status = 0;
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("Starting LPDDR4 inline ECC 1-bit data modify test");

 / Step 9: Write SII_INTR_OFFSET to enable SBR done interrupt /
 writel_reg(g_ctx.ctl_base + SII_INTR_OFFSET, SII_INTR_EN_SBR_DONE_INTR_MASK);

 / Step 10: Set ecc_region_map=0x55 /
 ecc_region_map = ECC_REGION_MAP;

 / Step 11: Disable quasi-dynamic programming (SWCTL=0) /
 writel_reg(g_ctx.ctl_base + SWCTL_OFFSET, 0x00000000U);

 / Step 12: Write ECCCFG0 with (ecc_region_map<<8)|0xb4 /
 writel_reg(g_ctx.ctl_base + ECCCFG0_OFFSET,
 ((unsigned int)ecc_region_map << 8) | 0x000000B4U);

 / Step 13: Write ECCCFG1 with 0x00000330 /
 writel_reg(g_ctx.ctl_base + ECCCFG1_OFFSET, ECCCFG1_VALUE);

 / Step 14: Re-enable quasi-dynamic programming (SWCTL=1) /
 writel_reg(g_ctx.ctl_base + SWCTL_OFFSET, 0x00000001U);

 / Step 15: Set int_expected=0 /
 g_ctx.int_expected = 0U;

 / Step 16: Set offset=0x80000000 /
 offset = ECC_REGION_OFFSET;

 LOGT("ECC region map=0x%x, offset=0x%lx", ecc_region_map, (unsigned long)offset);

 / Step 17: Loop region 0..6: if ecc_region_map bit set, call memory_init_scrb /
 for (region = 0U; region < ECC_REGION_COUNT; region++) {
 if (((ecc_region_map >> region) & 0x1U) != 0U) {
 LOGT("Initializing scrub for region %u at offset 0x%lx",
 region, (unsigned long)(offset * region));
 memory_init_scrb(offset * region);
 }
 }

 / Step 18: Loop region 0..6: generate random data, write to port1 /
 for (region = 0U; region < ECC_REGION_COUNT; region++) {
 exp_data_array[region] = (uint64_t)rand();
 exp_data_array[region] = (exp_data_array[region] << 32) | (uint64_t)rand();
 write_reg64(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset),
 exp_data_array[region]);
 LOGT("Region %u: wrote exp_data=0x%llx to addr=0x%lx",
 region,
 (unsigned long long)exp_data_array[region],
 (unsigned long)(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset)));
 }

 / Step 19: Wait, training_done, wait /
 wait_on(1000);
 training_done();
 wait_on(1000);

 / Step 20: Write ECCCTL with 0x00000000 /
 writel_reg(g_ctx.ctl_base + ECCCTL_OFFSET, 0x00000000U);

 / Step 21: Read port1_addr via read_reg64 into read_data /
 read_data = read_reg64(g_ctx.port1_addr);

 / Step 22: Read ECCSTAT /
 eccstat = readl_reg(g_ctx.ctl_base + ECCSTAT_OFFSET);

 / Step 23: Poll ECCSTAT bit[8] until set /
 timeout = 10000U;
 while (((eccstat & ECCSTAT_CORR_BIT) == 0U) && (timeout > 0U)) {
 eccstat = readl_reg(g_ctx.ctl_base + ECCSTAT_OFFSET);
 timeout--;
 }
 if (timeout == 0U) {
 LOGE("Timeout polling ECCSTAT bit[8]");
 g_ctx.errors++;
 }

 / Step 24: Read-modify-write ECCCTL OR with 0x00000001 to clear corrected error /
 eccctl = readl_reg(g_ctx.ctl_base + ECCCTL_OFFSET);
 eccctl = eccctl | ECCCTL_CORR_CLR;
 writel_reg(g_ctx.ctl_base + ECCCTL_OFFSET, eccctl);

 / Step 25: Write SII_FAULT_OFFSET with SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK /
 writel_reg(g_ctx.ctl_base + SII_FAULT_OFFSET, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);

 / Step 26: Wait /
 wait_on(1000);

 / Step 27: Compare exp_data_array[0] with read_data /
 if (exp_data_array[0] != read_data) {
 LOGE("Region 0 data mismatch: exp=0x%llx actual=0x%llx",
 (unsigned long long)exp_data_array[0],
 (unsigned long long)read_data);
 g_ctx.errors++;
 } else {
 LOGT("Region 0 data match verified: 0x%llx",
 (unsigned long long)read_data);
 }

 / Step 28: Write SII_INTR_OFFSET with combined mask /
 writel_reg(g_ctx.ctl_base + SII_INTR_OFFSET,
 SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);

 / Step 29: Write ECCCTL with 1<<8 to enable ECC corrected error interrupt /
 writel_reg(g_ctx.ctl_base + ECCCTL_OFFSET, ECCCTL_CORR_INT_EN);

 / Step 30-32: Loop region 0..6: validate interrupt behavior and data integrity /
 LOGT("Starting per-region ECC validation loop");
 for (region = 0U; region < ECC_REGION_COUNT; region++) {
 g_ctx.int_expected = (ecc_region_map >> region) & 0x1U;
 g_ctx.int_pend1 = 1U;

 / Read data from port1 for this region /
 read_data = read_reg64(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset));
 wait_on(1000);

 if (g_ctx.int_expected != 0U) {
 / Step 31: ECC-enabled region - data must match, interrupt must fire /
 if (exp_data_array[region] != read_data) {
 LOGE("ECC region %u data mismatch: exp=0x%llx actual=0x%llx",
 region,
 (unsigned long long)exp_data_array[region],
 (unsigned long long)read_data);
 g_ctx.errors++;
 } else {
 LOGT("ECC region %u data match OK: 0x%llx",
 region, (unsigned long long)read_data);
 }

 / Poll int_pend1 until 0 (interrupt handled) /
 timeout = 10000U;
 while ((g_ctx.int_pend1 != 0U) && (timeout > 0U)) {
 wait_on(10);
 timeout--;
 }
 if (timeout == 0U) {
 LOGE("ECC region %u: timeout waiting for ECC corrected interrupt", region);
 g_ctx.errors++;
 }
 } else {
 / Step 32: Non-ECC region - data must NOT match /
 if (exp_data_array[region] == read_data) {
 LOGE("Non-ECC region %u data unexpectedly matches: 0x%llx",
 region, (unsigned long long)read_data);
 g_ctx.errors++;
 } else {
 LOGT("Non-ECC region %u data mismatch as expected", region);
 }
 }
 }

 / Step 33: Read ECCERRCNT /
 count = readl_reg(g_ctx.ctl_base + ECCERRCNT_OFFSET);

 / Step 34: Verify corrected error count equals 5 /
 if (count != EXPECTED_ERR_COUNT) {
 LOGE("ECCERRCNT mismatch: expected=%u actual=%u", EXPECTED_ERR_COUNT, count);
 g_ctx.errors++;
 } else {
 LOGT("ECCERRCNT verified: count=%u", count);
 }

 / Step 35: finish(err0) - converted to PSV/FV status reporting /
 g_ctx.checks_failed = g_ctx.errors;

 out->status = (g_ctx.errors == 0U) ? 0 : -1;
 out->actual_len = 1;
 out->actual_pattern[0] = (int)g_ctx.errors;

 LOGT("Run complete: %s errors=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_ctx.errors);

 return out->status;
}

/
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 * lpddr4_inline_ecc_1bit_data_modify_test.
 * Parameters:
 * cfg - Test configuration input.
 * Returns:
 * FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("LPDDR4 ECC 1-bit data modify test teardown: errors=%u", g_ctx.errors);
 return g_ctx.errors == 0U ? 0 : -1;
}
