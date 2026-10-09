// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_2bit_data_modify_test.h"
#include "test_define.inc"

// Global variables as per Meta Test Description
unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int int_expected;
unsigned int eccstat;
unsigned int count;
unsigned int region;
unsigned int ecc_region_map;
unsigned int sbr_init;
unsigned int err0;
unsigned int slverr_taken;
unsigned int num_slverr_taken;

// Global variables used across functions
static unsigned long int ctl_base;
static unsigned long int phy_base;
static unsigned int bus_width;
static unsigned int SG;
static unsigned int DBI_EN;
static unsigned int DM_EN;
static unsigned int ECC_EN;
static unsigned int int_pend;
static unsigned int int_pend1;

// Meta Arrays: exp_data_array declared as local in test_case,
// placed at file scope for access by run function
static unsigned long int exp_data_array[50];

/*
 * Function: Default_IRQHandler
 * Description: IRQ handler for ECC uncorrectable error interrupts.
 *              Validates ECCSTAT bit 16, clears error via ECCCTL bit 1,
 *              clears SII interrupt, and clears GIC interrupt.
 * Parameters:
 *   None
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    // Step 1: Check if interrupt is expected
    if (!int_expected) {
        LOGE("ERROR0: Unexpected interrupt");
        err0++;
        int_pend = 1;
        return;
    }

    // Step 2: Read ECCSTAT
    eccstat = read_reg(ctl_base + 0x00000078); // UMCTL2_REGS.ECCSTAT

    // Step 3: Check bit 16 for 2-bit uncorrectable error
    if ((eccstat & 0x10000) == 0x10000) {
        // Clear uncorrectable ECC error via ECCCTL bit 1
        write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000002); // UMCTL2_REGS.ECCCTL
        // Clear SII interrupt
        write_reg(ctl_base + 0x00008194, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK);
        LOGT("Interrupt cleared");
        int_pend1 = 0;
    } else {
        LOGE("ERROR: Unexpected interrupt");
        err0++;
    }

    // Step 4: Write to SYSREG_INTR_RSTCR_ADDR
    write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);

    // Step 5: Clear GIC IRQ
    GIC_ClearIRQ(CTL_INT_NO);
}

/*
 * Function: memory_init_scrb
 * Description: Memory initialization scrub for ECC-enabled regions.
 *              Disables ports and performs scrubber initialization.
 * Parameters:
 *   address - base address of the memory region to scrub
 * Returns:
 *   void
 */
void memory_init_scrb(unsigned long int address)
{
    unsigned long int i;
    unsigned int rd_data;

    (void)address;
    (void)i;
    (void)rd_data;

    // Step 1: sbr_init = 1
    sbr_init = 1;

    // Step 2: ECCCFG1.ecc_region_parity_lock to 1
    // Programmed before task call (as per Meta comment)

    // Step 3: Disable ports
    write_reg(ctl_base + 0x00000490, 0x00000000); // UMCTL2_MP.PCTRL_0
    write_reg(ctl_base + 0x00000540, 0x00000000); // UMCTL2_MP.PCTRL_1

    // MANUAL_REVIEW: Remaining memory_init_scrb steps are not fully visible
    // in retrieved source context. The complete scrubber register programming
    // sequence (SBR configuration, scrub start, polling for SBR done interrupt,
    // re-enabling ports) must be added here based on the full source.
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_init
 * Description: Initialize LPDDR4 inline ECC 2-bit uncorrectable error
 *              data modify test. Configures ports, performs training,
 *              sets up ECC configuration, and scrubs enabled regions.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 inline ECC 2-bit data modify test init started");

    // Meta Step 1: Call gpv_programming()
    // MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
    // If a PSV equivalent is required, provide the replacement API.

    // Meta Step 2: err0 = 0
    err0 = 0;

    // Meta Step 3: Conditional on APS_DRAM
#if defined(APS_DRAM)
    port0_addr = 0;
    port1_addr = 0x15A0000000UL;
    ctl_base = 0x9EE03000UL;
    phy_base = 0x9F000000UL;
#else
    port0_addr = 0;
    port1_addr = 0x11A0000000UL;
    ctl_base = 0x11D003000UL;
    phy_base = 0x11D500000UL;
#endif

    LOGT("port0_addr=0x%lx port1_addr=0x%lx ctl_base=0x%lx phy_base=0x%lx",
         port0_addr, port1_addr, ctl_base, phy_base);

    // Meta Step 4: bus_width = 0 (0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus)
    bus_width = 0;

    // Meta Step 5: Conditional on SG2667/SG2133
#if defined(SG2667)
    SG = 2667;
#elif defined(SG2133)
    SG = 2133;
#else
    SG = 3200;
#endif

    LOGT("bus_width=%u SG=%u", bus_width, SG);

    // Meta Step 6: DBI_EN = 0
    DBI_EN = 0;

    // Meta Step 7: DM_EN = 1
    DM_EN = 1;

    // Meta Step 8: ECC_EN = 1
    ECC_EN = 1;

    LOGT("DBI_EN=%u DM_EN=%u ECC_EN=%u", DBI_EN, DM_EN, ECC_EN);

    // Meta Step 9: Call lpddr4_training()
    LOGT("Calling lpddr4_training()");
    lpddr4_training();

    // Meta Step 10: Call wait_on(1000) - converted to PSV busy-wait
    LOGT("Waiting for stabilization (wait_on 1000)");
    for (volatile int d = 0; d < 10000; d++);

    // Meta Step 11: Call training_done()
    LOGT("Calling training_done()");
    training_done();

    // Meta Step 12: Call wait_on(1000) - converted to PSV busy-wait
    LOGT("Waiting for stabilization (wait_on 1000)");
    for (volatile int d = 0; d < 10000; d++);

    // Meta Step 13: write_reg(ctl_base + 0x0000007c, 0x00000000) UMCTL2_REGS.ECCCTL
    write_reg(ctl_base + 0x0000007c, 0x00000000);
    LOGT("ECCCTL cleared");

    // Meta Step 14: write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK)
    write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("SBR done interrupt enabled");

    // Meta Step 15: ecc_region_map = 0x55
    ecc_region_map = 0x55;

    // Meta Step 16: write_reg(ctl_base + 0x00000320, 0x00000000) UMCTL2_REGS.SWCTL unlock
    write_reg(ctl_base + 0x00000320, 0x00000000);
    LOGT("SWCTL unlocked");

    // Meta Step 17: write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4) UMCTL2_REGS.ECCCFG0
    write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4);
    LOGT("ECCCFG0 configured: 0x%x", (ecc_region_map << 8) | 0xb4);

    // Meta Step 18: write_reg(ctl_base + 0x00000074, 0x00000330) UMCTL2_REGS.ECCCFG1
    write_reg(ctl_base + 0x00000074, 0x00000330);
    LOGT("ECCCFG1 configured: 0x00000330");

    // Meta Step 19: write_reg(ctl_base + 0x00000320, 0x00000001) UMCTL2_REGS.SWCTL lock
    write_reg(ctl_base + 0x00000320, 0x00000001);
    LOGT("SWCTL locked");

    // Meta Step 20: int_expected = 0
    int_expected = 0;

    // Meta Step 21: offset = 0x80000000
    // offset is used in run, store as global for access
    // (offset declared locally in original test_case, used across steps)

    // Meta Step 22: Scrub ECC-enabled memory regions
    LOGT("Scrubbing ECC-enabled memory regions");
    {
        unsigned long int offset = 0x80000000UL;
        for (region = 0; region < 7; region++) {
            if ((ecc_region_map >> region) & 0x1) {
                LOGT("Scrubbing region %u at offset 0x%lx", region, offset * region);
                memory_init_scrb(offset * region);
            }
        }
    }

    LOGT("LPDDR4 inline ECC 2-bit data modify test init completed");

    return 0;
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_run
 * Description: Execute LPDDR4 inline ECC 2-bit uncorrectable error
 *              data modify test. Writes data patterns to 7 regions,
 *              enables ECC interrupts, reads back to trigger ECC check,
 *              and polls ECCSTAT for uncorrectable error detection.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for result reporting
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_inline_ecc_2bit_data_modify_test_run(
    const TestsItem *cfg,
    TestOutput *out
)
{
    unsigned long int offset;
    unsigned long int read_data;

    (void)cfg;

    if (out == 0) {
        LOGE("LPDDR4 output pointer is NULL");
        return -1;
    }

    out->status = 0;

    // Initialize local variables as per Meta
    num_slverr_taken = 0;
    offset = 0x80000000UL;

    LOGT("Starting LPDDR4 inline ECC 2-bit data modify test run");

    // Meta Step 23: Write 64-bit data patterns to all 7 regions via port1
    // MANUAL_REVIEW: Meta uses rand() to generate exp_data_array values.
    // rand() is prohibited in PSV. Deterministic replacement values are
    // required but not supplied by Meta. Using placeholder deterministic
    // pattern. Replace with authoritative test data.
    for (region = 0; region < 7; region++) {
        // MANUAL_REVIEW: Original code: exp_data_array[region] = rand();
        // MANUAL_REVIEW: Original code: exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32);
        // Deterministic placeholder - must be replaced with authoritative values
        exp_data_array[region] = 0xA5A5A5A5UL | ((unsigned long)(0x5A5A5A5AUL + region) << 32);
        write_reg64(port1_addr + (region * offset), exp_data_array[region]);
        LOGT("Region %u: wrote 0x%lx to addr 0x%lx",
             region, exp_data_array[region], port1_addr + (region * offset));
    }

    // Meta Step 24: Enable ECC uncorrected error and SBR done interrupts
    write_reg(ctl_base + 0x0000819C, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("ECC uncorrected error and SBR done interrupts enabled");

    // Meta Step 25: Enable uncorrected ECC error interrupt in ECCCTL (bit 9)
    write_reg(ctl_base + 0x0000007c, 1 << 9);
    LOGT("ECCCTL interrupt enable (bit 9) set");

    // Meta Step 26: Read back data from each region to trigger ECC check
    for (region = 0; region < 7; region++) {
        slverr_taken = 0;
        LOGT("region = %d", region);
        int_expected = (ecc_region_map >> region) & 0x1;
        int_pend1 = 1;
        read_reg64(port1_addr + (region * offset), &read_data);
        LOGT("Region %u: read_data=0x%lx int_expected=%u",
             region, read_data, int_expected);
    }

    // Meta Step 27: ECCSTAT polling
    read_reg64(port1_addr, &read_data);
    eccstat = read_reg(ctl_base + 0x00000078); // UMCTL2_REGS.ECCSTAT
    LOGT("ECCSTAT polling: initial eccstat=0x%x", eccstat);
    while (!(eccstat & (1 << 16))) {
        eccstat = read_reg(ctl_base + 0x00000078); // UMCTL2_REGS.ECCSTAT
    }
    LOGT("ECCSTAT polling complete: eccstat=0x%x (bit 16 set)", eccstat);

    // Meta Step 28: finish(err0) - converted to FV/PSV status
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("Run complete: %s err0=%u",
         (out->status == 0) ? "PASS" : "FAIL",
         err0);

    return out->status;
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_teardown
 * Description: Teardown and cleanup for LPDDR4 inline ECC 2-bit
 *              data modify test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_teardown(
    const TestsItem *cfg
)
{
    (void)cfg;

    LOGT("LPDDR4 inline ECC 2-bit data modify test teardown: err0=%u num_slverr_taken=%u",
         err0, num_slverr_taken);
    return (err0 == 0) ? 0 : -1;
}
