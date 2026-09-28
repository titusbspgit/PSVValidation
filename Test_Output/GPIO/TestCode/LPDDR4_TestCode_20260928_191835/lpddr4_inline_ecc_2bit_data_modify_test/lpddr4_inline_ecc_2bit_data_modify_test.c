// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_2bit_data_modify_test.h"
#include "test_define.inc"

/*
 * Test Case: lpddr4_inline_ecc_2bit_data_modify_test
 * Description: Validates LPDDR4 inline ECC 2-bit uncorrectable error detection
 *              with data modification across multiple ECC regions.
 *              Configures ECC with ecc_region_map=0x55 (regions 0, 2, 4, 6 enabled),
 *              initializes memory via scrubber (SBR), writes random 64-bit data,
 *              reads back and checks for ECC uncorrectable errors, slave errors,
 *              and interrupt behavior. Verifies ECCERRCNT bits[31:16] equals 5
 *              and num_slverr_taken equals 5 after all region reads.
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
} lpddr4_ecc_2bit_test_ctx_t;

static lpddr4_ecc_2bit_test_ctx_t g_ctx;

/* Volatile flags shared with IRQ handler and slave error handlers */
static volatile unsigned int int_pend       = 1U;
static volatile unsigned int int_pend1      = 0U;
static volatile unsigned int sbr_init       = 0U;
static volatile unsigned int int_expected   = 0U;
static volatile unsigned int slverr_taken   = 0U;
static volatile unsigned int num_slverr_taken = 0U;

/* --------------------------------------------------------------------------
 * memory_init_scrb - Scrubber-based memory initialization
 * -------------------------------------------------------------------------- */
static void memory_init_scrb(unsigned long address)
{
    uint32_t set_data;

    sbr_init = 1U;

    LOGT("memory_init_scrb: start address=0x%lx", (unsigned long)address);

    /* Step 38: Disable PCTRL_0 and PCTRL_1 */
    writel_reg(g_ctx.ctl_base + 0x00000490UL, 0x00000000UL);
    writel_reg(g_ctx.ctl_base + 0x00000540UL, 0x00000000UL);

    /* Read-modify-write SBRCTL to set scrub_mode=1 (bit[2]) */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data |= (1U << 2);
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Read-modify-write SBRCTL to set scrub_interval=0 (bits[31:8]) */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data &= 0x000000FFUL;
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Write SBRWDATA0 and SBRWDATA1 with zeros */
    writel_reg(g_ctx.ctl_base + 0x00000f2cUL, 0x00000000UL);
    writel_reg(g_ctx.ctl_base + 0x00000f30UL, 0x00000000UL);

    /* Set int_pend1=1 before enabling scrub */
    int_pend1 = 1U;

    /* Write SBRSTART0 and SBRSTART1 with (address>>3) */
    writel_reg(g_ctx.ctl_base + 0x00000f38UL, (uint32_t)(address >> 3));
    writel_reg(g_ctx.ctl_base + 0x00000f3cUL, (uint32_t)(address >> 3));

    /* Write SBRRANGE0 and SBRRANGE1 with ((address+0x1000)>>3) */
    writel_reg(g_ctx.ctl_base + 0x00000f40UL, (uint32_t)((address + 0x1000UL) >> 3));
    writel_reg(g_ctx.ctl_base + 0x00000f44UL, (uint32_t)((address + 0x1000UL) >> 3));

    /* Enable scrub via SBRCTL bit[0]=1 */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data |= 0x00000001UL;
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Wait for SBR done interrupt (poll int_pend1) */
    LOGT("memory_init_scrb: waiting for SBR done interrupt");
    while (int_pend1 != 0U) {
        wait_on(10);
    }

    /* Read SBRCTL, set scrub_mode=0 (bit[2]), scrub_interval=1 (bits[31:8]), write back */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data &= ~(1U << 2);
    set_data &= 0x000000FFUL;
    set_data |= (1U << 8);
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Disable scrub via SBRCTL bit[0]=0 */
    set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
    set_data &= ~(0x00000001UL);
    writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

    /* Re-enable PCTRL_0 and PCTRL_1 */
    writel_reg(g_ctx.ctl_base + 0x00000490UL, 0x00000001UL);
    writel_reg(g_ctx.ctl_base + 0x00000540UL, 0x00000001UL);

    sbr_init = 0U;

    LOGT("memory_init_scrb: complete");
}

/* --------------------------------------------------------------------------
 * Default_IRQHandler - IRQ handler for ECC uncorrectable and SBR interrupts
 * -------------------------------------------------------------------------- */
void Default_IRQHandler(void)
{
    uint32_t msts;
    uint32_t sii_val;
    uint32_t eccstat;
    uint32_t eccctl;
    uint32_t set_data;

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
                set_data = readl_reg(g_ctx.ctl_base + 0x00000f24UL);
                set_data &= ~(0x00000001UL);
                writel_reg(g_ctx.ctl_base + 0x00000f24UL, set_data);

                /* Clear SBR interrupt in SII register */
                writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_INTR_EN_SBR_DONE_INTR_MASK);

                int_pend1 = 0U;
            }
        } else {
            /* ECC uncorrectable error interrupt path */
            if (int_expected == 0U) {
                LOGE("IRQ: ECC uncorrectable interrupt received but int_expected=0");
                g_ctx.errors++;
            }

            eccstat = readl_reg(g_ctx.ctl_base + 0x00000078UL);

            if ((eccstat & (1U << 16)) == 0x00010000UL) {
                /* Clear uncorrectable error via ECCCTL OR 0x00000002 */
                eccctl = readl_reg(g_ctx.ctl_base + 0x0000007cUL);
                eccctl |= 0x00000002UL;
                writel_reg(g_ctx.ctl_base + 0x0000007cUL, eccctl);

                /* Clear ECC interrupt in SII register */
                writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK);

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
 * Slave error handlers
 * Step 37: lower_el_aarch64_irq_vector and curr_el_spx_fiq_vector
 * -------------------------------------------------------------------------- */
void lower_el_aarch64_irq_vector(void)
{
    slverr_taken = 1U;
    num_slverr_taken++;
    /* MANUAL_REVIEW: ISB SY and DAIFClr #0xF are architecture-specific inline assembly instructions. */
    /* Preserve as comment: __asm volatile("ISB SY"); __asm volatile("MSR DAIFClr, #0xF"); */
}

void curr_el_spx_fiq_vector(void)
{
    slverr_taken = 1U;
    num_slverr_taken++;
    /* MANUAL_REVIEW: ISB SY and DAIFClr #0xF are architecture-specific inline assembly instructions. */
    /* Preserve as comment: __asm volatile("ISB SY"); __asm volatile("MSR DAIFClr, #0xF"); */
}

/* --------------------------------------------------------------------------
 * FV Functions
 * -------------------------------------------------------------------------- */

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_init
 * Description: Performs testcase initialization and pre-condition setup for
 *              lpddr4_inline_ecc_2bit_data_modify_test. Enables GIC IRQ,
 *              controller interrupt, performs bus programming, configures
 *              port addresses, training parameters, and ECC settings.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_init(const TestsItem *cfg)
{
    (void)cfg;

    g_ctx = (lpddr4_ecc_2bit_test_ctx_t){0};

    LOGT("lpddr4_inline_ecc_2bit_data_modify_test_init: start");

    /* Step 1: Set int_pend=1, num_slverr_taken=0, enable GIC IRQ for CTL_INT_NO */
    int_pend = 1U;
    num_slverr_taken = 0U;
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

    LOGT("lpddr4_inline_ecc_2bit_data_modify_test_init: complete");
    return 0;
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_run
 * Description: Executes the main testcase flow for
 *              lpddr4_inline_ecc_2bit_data_modify_test. Configures ECC,
 *              initializes memory via scrubber, writes random data,
 *              reads back and validates ECC uncorrectable error detection,
 *              slave error behavior, and interrupt handling across all regions.
 * Parameters:
 *   cfg - Test configuration input.
 *   out - Test output capture structure.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_run(const TestsItem *cfg, TestOutput *out)
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

    LOGT("lpddr4_inline_ecc_2bit_data_modify_test_run: start");

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

    /* Step 23: Poll ECCSTAT bit[16] until set */
    while (!(eccstat & (1U << 16))) {
        eccstat = readl_reg(g_ctx.ctl_base + 0x00000078UL);
    }
    LOGT("ECCSTAT bit[16] set: uncorrectable error detected");

    /* Step 24: Read-modify-write ECCCTL OR with 0x00000002 to clear uncorrectable error */
    eccctl = readl_reg(g_ctx.ctl_base + 0x0000007cUL);
    eccctl |= 0x00000002UL;
    writel_reg(g_ctx.ctl_base + 0x0000007cUL, eccctl);
    LOGT("ECCCTL uncorrectable error cleared");

    /* Step 25: Write SII status register with SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK */
    writel_reg(g_ctx.ctl_base + 0x00008194UL, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK);

    /* Step 26: Wait */
    wait_on(1000);

    /* Step 27: Write SII interrupt enable register */
    writel_reg(g_ctx.ctl_base + 0x0000819CUL,
               SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("SII interrupt enable updated for ECC uncorrectable + SBR done");

    /* Step 28: Write ECCCTL with 1<<9 to enable ECC uncorrectable error interrupt */
    writel_reg(g_ctx.ctl_base + 0x0000007cUL, (1U << 9));
    LOGT("ECCCTL ECC uncorrectable error interrupt enabled");

    /* Steps 29-31: Loop region 0..6 for interrupt-based and slave error validation */
    LOGT("Starting region loop for ECC uncorrectable interrupt and slave error validation");
    for (region = 0U; region < 7U; region++) {
        /* Step 29: Set slverr_taken=0, int_expected, int_pend1=1, read data */
        slverr_taken = 0U;
        int_expected = (ecc_region_map >> region) & 0x1U;
        int_pend1 = 1U;

        read_data = read_reg64(g_ctx.port1_addr + ((uintptr_t)region * (uintptr_t)offset));
        wait_on(1000);

        LOGT("Region %u: int_expected=%u read_data=0x%llx",
             region, int_expected, (unsigned long long)read_data);

        /* Step 30: If int_expected, poll int_pend1 and check slverr_taken */
        if (int_expected != 0U) {
            while (int_pend1 != 0U) {
                wait_on(10);
            }
            LOGT("Region %u: ECC uncorrectable interrupt handled", region);

            if (slverr_taken != 1U) {
                LOGE("Region %u: slverr_taken expected=1 actual=%u",
                     region, slverr_taken);
                g_ctx.errors++;
            } else {
                LOGT("Region %u: slave error confirmed", region);
            }
        }

        /* Step 31: For all regions, data must NOT match (2-bit uncorrectable) */
        if (exp_data_array[region] == read_data) {
            LOGE("Region %u: data unexpectedly matched 0x%llx (2-bit error should corrupt)",
                 region, (unsigned long long)read_data);
            g_ctx.errors++;
        } else {
            LOGT("Region %u: data mismatch confirmed (expected for 2-bit uncorrectable)", region);
        }
    }

    /* Step 32: Read ECCERRCNT */
    count = readl_reg(g_ctx.ctl_base + 0x00000080UL);
    LOGT("ECCERRCNT=0x%lx", (unsigned long)count);

    /* Step 33: Verify uncorrectable error count (bits[31:16]) equals 5 */
    if ((count >> 16) != 5U) {
        LOGE("ECCERRCNT uncorrectable mismatch: expected=5 actual=%u", (unsigned int)(count >> 16));
        g_ctx.errors++;
    } else {
        LOGT("ECCERRCNT uncorrectable verified: count=5");
    }

    /* Step 34: Verify num_slverr_taken equals 5 */
    if (num_slverr_taken != 5U) {
        LOGE("num_slverr_taken mismatch: expected=5 actual=%u", (unsigned int)num_slverr_taken);
        g_ctx.errors++;
    } else {
        LOGT("num_slverr_taken verified: count=5");
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
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_teardown
 * Description: Performs testcase validation, cleanup, and final status handling for
 *              lpddr4_inline_ecc_2bit_data_modify_test.
 * Parameters:
 *   cfg - Test configuration input.
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("lpddr4_inline_ecc_2bit_data_modify_test teardown: errors=%u", g_ctx.errors);

    return g_ctx.errors == 0U ? 0 : -1;
}
