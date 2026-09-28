// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_1bit_data_modify_test.h"
#include "test_define.inc"

/*
 * Test Case: lpddr4_inline_ecc_1bit_data_modify_test
 * Description: Validates LPDDR4 inline ECC 1-bit corrected error detection
 *              with data modification across multiple ECC regions.
 *              Configures ECC with ecc_region_map=0x55 (regions 0, 2, 4, 6 enabled),
 *              initializes memory via scrubber (SBR), writes random 64-bit data,
 *              reads back and checks for ECC corrected errors and interrupt behavior.
 *              Verifies ECCERRCNT equals 5 after all region reads.
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
} lpddr4_ecc_1bit_test_ctx_t;

static lpddr4_ecc_1bit_test_ctx_t g_ctx;

/* Volatile flags shared with IRQ handler */
static volatile unsigned int int_pend  = 1U;
static volatile unsigned int int_pend1 = 0U;
static volatile unsigned int sbr_init  = 0U;
static volatile unsigned int int_expected = 0U;

/* --------------------------------------------------------------------------
 * memory_init_scrb - Scrubber-based memory initialization
 * -------------------------------------------------------------------------- */
static void memory_init_scrb(unsigned long address)
{
    uint32_t set_data;

    sbr_init = 1U;

    LOGT("memory_init_scrb: start address=0x%lx", (unsigned long)address);

    /* Step 37: Disable PCTRL_0 and PCTRL_1 */
    writel_reg(g_ctx.ctl_base + 0x00000490UL, 0x00000000UL);
    writel_reg(g_ctx.ctl_base + 0x00000540UL, 0x00000000UL);

    /* Set SBRCTL scrub_mode=1 (bit[2]), scrub_interval=0 (bits[31:8]) */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data |= (1U << 2);           /* scrub_mode = 1 */
    set_data &= 0x000000FFUL;        /* scrub_interval = 0 (clear bits[31:8]) */
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Write SBRWDATA0 and SBRWDATA1 with zeros */
    writel_reg(g_ctx.ctl_base + 0x00000f2cUL, 0x00000000UL);
    writel_reg(g_ctx.ctl_base + 0x00000f30UL, 0x00000000UL);

    /* Set SBRSTART0/SBRSTART1 for target address */
    writel_reg(g_ctx.ctl_base + 0x00000f38UL, (uint32_t)(address >> 3));
    writel_reg(g_ctx.ctl_base + 0x00000f3cUL, (uint32_t)(address >> 3));

    /* Set SBRRANGE0/SBRRANGE1 for target address range */
    writel_reg(g_ctx.ctl_base + 0x00000f40UL, (uint32_t)((address + 0x1000UL) >> 3));
    writel_reg(g_ctx.ctl_base + 0x00000f44UL, (uint32_t)((address + 0x1000UL) >> 3));

    /* Enable scrub (SBRCTL bit[0]=1) */
    int_pend1 = 1U;
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data |= 0x00000001UL;
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Wait for SBR done interrupt */
    LOGT("memory_init_scrb: waiting for SBR done interrupt");
    while (int_pend1 != 0U) {
        wait_on(10);
    }

    /* Restore scrub_mode=0, scrub_interval=1, disable scrub */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data &= ~(1U << 2);          /* scrub_mode = 0 */
    set_data |= (1U << 8);           /* scrub_interval = 1 */
    set_data &= ~(0x00000001UL);     /* disable scrub */
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Re-enable PCTRL_0 and PCTRL_1 */
    writel_reg(g_ctx.ctl_base + 0x00000490UL, 0x00000001UL);
    writel_reg(g_ctx.ctl_base + 0x00000540UL, 0x00000001UL);

    sbr_init = 0U;

    LOGT("memory_init_scrb: complete");
}

/* --------------------------------------------------------------------------
 * Default_IRQHandler - IRQ handler for ECC and SBR interrupts
 * -------------------------------------------------------------------------- */
void Default_IRQHandler(void)
{
    uint32_t msts;
    uint32_t sii_val;
    uint32_t eccstat;
    uint32_t eccctl;

    /* Read system register masked interrupt status */
    msts = readl_reg(SYSREG_INTR_MSTS_ADDR);

    if (msts & SYSREG_CTRL_INTR_MSTS_MASK) {
        if (sbr_init != 0U) {
            /* SBR done interrupt path */
            sii_val = readl_reg(g_ctx.ctl_base + 0x00008194UL);

            if (sii_val & SII_INTR_EN_SBR_DONE_INTR_MASK) {
                /* Poll SBRRANGE1 bit[0] until 0 */
                uint32_t sbrrange1;
                sbrrange1 = readl_reg(g_ctx.ctl_base + 0x00000f28UL);
                while (sbrrange1 & 0x00000001UL) {
                    sbrrange1 = readl_reg(g_ctx.ctl_base + 0x00000f28UL);
                }

                /* Disable scrub via SBRCTL bit[0]=0 */
                uint32_t sbrctl;
                sbrctl = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
                sbrctl &= ~(0x00000001UL);
                writel_reg(g_ctx.ctl_base + 0x00000f24UL, sbrctl);

                /* Clear SBR interrupt in SII register */
                writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_INTR_EN_SBR_DONE_INTR_MASK);

                int_pend1 = 0U;
            }
        } else {
            /* ECC corrected error interrupt path */
            eccstat = readl_reg(g_ctx.ctl_base + 0x00000078UL);

            if (eccstat & (1U << 8)) {
                /* Clear corrected error via ECCCTL OR 0x00000001 */
                eccctl = readl_reg(g_ctx.ctl_base + 0x0000007cUL);
                eccctl |= 0x00000001UL;
                writel_reg(g_ctx.ctl_base + 0x0000007cUL, eccctl);

                /* Clear ECC interrupt in SII register */
                writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);

                int_pend1 = 0U;
            }
        }
    }

    /* Clear system register interrupt */
    writel_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);

    /* Clear GIC IRQ */
    GIC_ClearIRQ(CTL_INT_NO);
}

/* --------------------------------------------------------------------------
 * FV Functions
 * -------------------------------------------------------------------------- */

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              lpddr4_inline_ecc_1bit_data_modify_test. Enables GIC IRQ,
 *              controller interrupt, performs bus programming, configures
 *              port addresses, training parameters, and ECC settings.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (lpddr4_ecc_1bit_test_ctx_t){0};

    LOGT("lpddr4_inline_ecc_1bit_data_modify_test_init: start");

    /* Step 1: Set int_pend=1, enable GIC IRQ for CTL_INT_NO */
    int_pend = 1U;
    GIC_EnableIRQ(CTL_INT_NO);

    /* Step 2: Write SYSREG_INTR_EN_ADDR to enable controller interrupt */
    writel_reg(SYSREG_INTR_EN_ADDR, SYSREG_CTRL_INTR_EN_MASK);

    /* Step 3: Call gpv_programming() for bus configuration */
    gpv_programming();

    /* Step 4: Set err0=0 (using g_ctx.errors) */
    g_ctx.errors = 0U;

    /* Step 5: Configure port addresses based on APS_DRAM or MPS_DRAM */
#if defined(APS_DRAM)
    g_ctx.port0_addr = APS_DRAM_PORT0_ADDR;
    g_ctx.port1_addr = APS_DRAM_PORT1_ADDR;
    g_ctx.ctl_base   = APS_DRAM_CTL_BASE;
    g_ctx.phy_base   = APS_DRAM_PHY_BASE;
#elif defined(MPS_DRAM)
    g_ctx.port0_addr = MPS_DRAM_PORT0_ADDR;
    g_ctx.port1_addr = MPS_DRAM_PORT1_ADDR;
    g_ctx.ctl_base   = MPS_DRAM_CTL_BASE;
    g_ctx.phy_base   = MPS_DRAM_PHY_BASE;
#else
    LOGE("Neither APS_DRAM nor MPS_DRAM defined");
    return -1;
#endif

    if ((g_ctx.port0_addr == (uintptr_t)0U) ||
        (g_ctx.port1_addr == (uintptr_t)0U)) {
        LOGE("LPDDR4 port addresses are not configured");
        return -1;
    }

    LOGT("port0=0x%lx port1=0x%lx ctl_base=0x%lx phy_base=0x%lx",
         (unsigned long)g_ctx.port0_addr,
         (unsigned long)g_ctx.port1_addr,
         (unsigned long)g_ctx.ctl_base,
         (unsigned long)g_ctx.phy_base);

    /* Step 6: Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=1, ECC_EN=1 */
    bus_width = 0U;
    DBI_EN    = 0U;
    DM_EN     = 1U;
    ECC_EN    = 1U;

    /* Step 7: Set SG speed grade */
#if defined(SG2667)
    SG = 2667U;
#elif defined(SG2133)
    SG = 2133U;
#else
    SG = 3200U;
#endif

    /* Step 8: Call lpddr4_training() */
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    LOGT("lpddr4_inline_ecc_1bit_data_modify_test_init: complete");
    return 0;
}

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_run
 * Description: Executes the main testcase flow for
 *              lpddr4_inline_ecc_1bit_data_modify_test. Configures ECC,
 *              initializes memory via scrubber, writes random data,
 *              reads back and validates ECC corrected error detection
 *              and interrupt behavior across all regions.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned int region;
    uint32_t ecc_region_map;
    unsigned long offset;
    uint64_t read_data;
    uint32_t eccstat;
    uint32_t eccctl;
    uint32_t count;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("lpddr4_inline_ecc_1bit_data_modify_test_run: start");

    /* Step 9: Enable SBR done interrupt in SII interrupt enable register */
    writel_reg(g_ctx.ctl_base + 0x0000819CUL, SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("SBR done interrupt enabled");

    /* Step 10: Set ecc_region_map=0x55 */
    ecc_region_map = 0x55U;

    /* Step 11: Disable quasi-dynamic programming via SWCTL */
    writel_reg(g_ctx.ctl_base + 0x00000320UL, 0x00000000UL);
    LOGT("SWCTL disabled for quasi-dynamic programming");

    /* Step 12: Write ECCCFG0 with (ecc_region_map<<8)|0xb4 */
    writel_reg(g_ctx.ctl_base + 0x00000070UL, ((uint32_t)ecc_region_map << 8) | 0xb4UL);
    LOGT("ECCCFG0 configured: 0x%lx", (unsigned long)(((uint32_t)ecc_region_map << 8) | 0xb4UL));

    /* Step 13: Write ECCCFG1 with 0x00000330 */
    writel_reg(g_ctx.ctl_base + 0x00000074UL, 0x00000330UL);
    LOGT("ECCCFG1 configured: 0x00000330");

    /* Step 14: Re-enable SWCTL */
    writel_reg(g_ctx.ctl_base + 0x00000320UL, 0x00000001UL);
    LOGT("SWCTL re-enabled");

    /* Step 15: Set int_expected=0 */
    int_expected = 0U;

    /* Step 16: Set offset=0x80000000 */
    offset = 0x80000000UL;

    /* Step 17: Loop region 0..6: if ecc_region_map bit set, call memory_init_scrb */
    LOGT("Initializing memory via scrubber for ECC-enabled regions");
    for (region = 0U; region < 7U; region++) {
        if ((ecc_region_map >> region) & 0x1U) {
            memory_init_scrb((unsigned long)region * offset);
        }
    }

    /* Step 18: Loop region 0..6: generate random data, write to port1 */
    LOGT("Writing random 64-bit data to 7 regions via port1");
    for (region = 0U; region < 7U; region++) {
        exp_data_array[region] = (uint64_t)rand() | ((uint64_t)rand() << 32);
        write_reg64(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset),
                    exp_data_array[region]);
        LOGT("Region %u: wrote 0x%llx to addr 0x%lx",
             region,
             (unsigned long long)exp_data_array[region],
             (unsigned long)(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset)));
    }

    /* Step 19: Wait, training_done, wait */
    wait_on(1000);
    training_done();
    wait_on(1000);

    /* Step 20: Write ECCCTL with 0x00000000 */
    writel_reg(g_ctx.ctl_base + 0x0000007cUL, 0x00000000UL);
    LOGT("ECCCTL cleared");

    /* Step 21: Read port1_addr via read_reg64 into read_data */
    read_data = read_reg64(g_ctx.port1_addr);
    LOGT("Region 0 read_data=0x%llx", (unsigned long long)read_data);

    /* Step 22: Read ECCSTAT */
    eccstat = readl_reg(g_ctx.ctl_base + 0x00000078UL);
    LOGT("ECCSTAT=0x%lx", (unsigned long)eccstat);

    /* Step 23: Poll ECCSTAT bit[8] until set */
    while (!(eccstat & (1U << 8))) {
        eccstat = readl_reg(g_ctx.ctl_base + 0x00000078UL);
    }
    LOGT("ECCSTAT bit[8] set: corrected error detected");

    /* Step 24: Read-modify-write ECCCTL OR with 0x00000001 to clear corrected error */
    eccctl = readl_reg(g_ctx.ctl_base + 0x0000007cUL);
    eccctl |= 0x00000001UL;
    writel_reg(g_ctx.ctl_base + 0x0000007cUL, eccctl);
    LOGT("ECCCTL corrected error cleared");

    /* Step 25: Write SII status register with SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK */
    writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);

    /* Step 26: Wait */
    wait_on(1000);

    /* Step 27: Compare exp_data_array[0] with read_data */
    if (exp_data_array[0] != read_data) {
        LOGE("Region 0 data mismatch: exp=0x%llx actual=0x%llx",
             (unsigned long long)exp_data_array[0],
             (unsigned long long)read_data);
        g_ctx.errors++;
    } else {
        LOGT("Region 0 data match verified");
    }

    /* Step 28: Write SII interrupt enable register */
    writel_reg(g_ctx.ctl_base + 0x0000819CUL,
               SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("SII interrupt enable updated for ECC corrected + SBR done");

    /* Step 29: Write ECCCTL with 1<<8 to enable ECC corrected error interrupt */
    writel_reg(g_ctx.ctl_base + 0x0000007cUL, (1U << 8));
    LOGT("ECCCTL ECC corrected error interrupt enabled");

    /* Steps 30-32: Loop region 0..6 for interrupt-based validation */
    LOGT("Starting region loop for ECC interrupt validation");
    for (region = 0U; region < 7U; region++) {
        int_expected = (ecc_region_map >> region) & 0x1U;
        int_pend1 = 1U;

        /* Read data from port1 for this region */
        read_data = read_reg64(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset));
        wait_on(1000);

        LOGT("Region %u: int_expected=%u read_data=0x%llx",
             region, int_expected, (unsigned long long)read_data);

        if (int_expected != 0U) {
            /* Step 31: ECC-enabled region - data must match, interrupt must fire */
            if (exp_data_array[region] != read_data) {
                LOGE("Region %u ECC data mismatch: exp=0x%llx actual=0x%llx",
                     region,
                     (unsigned long long)exp_data_array[region],
                     (unsigned long long)read_data);
                g_ctx.errors++;
            }

            /* Poll int_pend1 until 0 (interrupt handled) */
            while (int_pend1 != 0U) {
                wait_on(10);
            }
            LOGT("Region %u: ECC corrected interrupt handled", region);
        } else {
            /* Step 32: Non-ECC region - data must NOT match */
            if (exp_data_array[region] == read_data) {
                LOGE("Region %u non-ECC data unexpectedly matched: 0x%llx",
                     region,
                     (unsigned long long)read_data);
                g_ctx.errors++;
            } else {
                LOGT("Region %u: non-ECC data mismatch confirmed (expected)", region);
            }
        }
    }

    /* Step 33: Read ECCERRCNT */
    count = readl_reg(g_ctx.ctl_base + 0x00000080UL);
    LOGT("ECCERRCNT=0x%lx", (unsigned long)count);

    /* Step 34: Verify corrected error count equals 5 */
    if (count != 5U) {
        LOGE("ECCERRCNT mismatch: expected=5 actual=%u", (unsigned int)count);
        g_ctx.errors++;
    } else {
        LOGT("ECCERRCNT verified: count=5");
    }

    /* Step 35: Report final status */
    // MANUAL_REVIEW: DV finish(err0) was present in the source flow. Converted to PSV/FV out->status reporting.
    out->status = (g_ctx.errors == 0U) ? 0 : -1;

    LOGT("Run complete: %s errors=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         g_ctx.errors);

    return out->status;
}

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              lpddr4_inline_ecc_1bit_data_modify_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_inline_ecc_1bit_data_modify_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
