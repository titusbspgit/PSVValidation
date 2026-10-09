// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_1bit_data_modify_test.h"
#include "test_define.inc"

// Global variables as per Meta
unsigned long int port0_addr;
unsigned long int port1_addr;
unsigned int int_expected;
unsigned int eccstat;
unsigned int count;
unsigned int region;
unsigned int ecc_region_map;
unsigned int sbr_init;
unsigned int err0;

// Additional globals from Meta
static unsigned long int ctl_base;
static unsigned long int phy_base;
static unsigned int bus_width;
static unsigned int SG;
static unsigned int DBI_EN;
static unsigned int DM_EN;
static unsigned int ECC_EN;
static unsigned int int_pend;
static unsigned int int_pend1;
static unsigned int num_slverr_taken;

// Deterministic 64-bit data replacing rand() for 7 regions
// MANUAL_REVIEW: Original DV used rand() to generate exp_data_array values.
// Replace with application-appropriate deterministic 64-bit patterns if needed.
static const unsigned long int deterministic_data_lo[7] = {
    0xA1B2C3D4UL,
    0xE5F60718UL,
    0x293A4B5CUL,
    0x6D7E8F90UL,
    0x01122334UL,
    0x55667788UL,
    0x99AABBCCUL
};

static const unsigned long int deterministic_data_hi[7] = {
    0xDDEEFF00UL,
    0x11223344UL,
    0x55667788UL,
    0x99AABBCCUL,
    0xDDEEFF01UL,
    0x12345678UL,
    0x9ABCDEF0UL
};

/*
 * Function: memory_init_scrb
 * Description: Initialize memory scrubbing for a given address region.
 *              Disables ports and initiates scrubber operation.
 * Parameters:
 *   address - base address of the memory region to scrub
 * Returns:
 *   void
 */
void memory_init_scrb(unsigned long int address)
{
    unsigned long int i;
    unsigned int rd_data;

    (void)i;
    (void)rd_data;
    (void)address;

    // Meta memory_init_scrb Step 1: sbr_init = 1
    sbr_init = 1;

    // Meta memory_init_scrb Step 2: Disable port 0
    write_reg(ctl_base + 0x00000490, 0x00000000);
    LOGT("memory_init_scrb: PCTRL_0 disabled (ctl_base+0x490)");

    // Meta memory_init_scrb Step 3: Disable port 1
    write_reg(ctl_base + 0x00000540, 0x00000000);
    LOGT("memory_init_scrb: PCTRL_1 disabled (ctl_base+0x540)");

    // MANUAL_REVIEW: Remaining memory_init_scrb steps are not fully visible
    // in retrieved source context. The scrubber register programming sequence
    // (SBRCTL, SBRWDATA0, SBRSTART, SBRSTAT, SBR done wait, port re-enable)
    // must be completed based on the full source implementation.
}

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for ECC correctable error interrupts.
 *              Validates ECCSTAT bit 8, clears error via ECCCTL bit 0,
 *              clears SII interrupt, and clears GIC interrupt.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    // Meta Default_IRQHandler Step 1: Check if interrupt is expected
    if (!int_expected) {
        LOGE("ERROR0: Unexpected interrupt");
        err0++;
        int_pend = 1;
        return;
    }

    // Meta Default_IRQHandler Step 2: Read ECCSTAT
    eccstat = read_reg(ctl_base + 0x00000078);
    LOGT("Default_IRQHandler: ECCSTAT=0x%x", eccstat);

    // Meta Default_IRQHandler Step 3: Check bit 8 and clear
    if ((eccstat & 0x100) == 0x100) {
        write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000001);
        write_reg(ctl_base + 0x00008194, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK);
        LOGT("Interrupt cleared");
        int_pend1 = 0;
    }

    // Meta Default_IRQHandler Step 4: Write to SYSREG_INTR_RSTCR_ADDR
    write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);

    // Meta Default_IRQHandler Step 5: Clear GIC IRQ
    GIC_ClearIRQ(CTL_INT_NO);
}

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_init
 * Description: Initialize LPDDR4 inline ECC 1-bit correctable error test.
 *              Configures port addresses, performs training, and waits
 *              for stabilization.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 inline ECC 1-bit data modify test init start");

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

    // Meta Step 4: bus_width = 0 (Full Bus)
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

    // Meta Step 10: Call wait_on(1000) - adapted to deterministic busy-wait
    LOGT("Wait for stabilization (wait_on 1000)");
    for (volatile int d = 0; d < 1000; d++);

    // Meta Step 11: Call training_done()
    LOGT("Calling training_done()");
    training_done();

    // Meta Step 12: Call wait_on(1000) - adapted to deterministic busy-wait
    LOGT("Wait for stabilization (wait_on 1000)");
    for (volatile int d = 0; d < 1000; d++);

    LOGT("LPDDR4 inline ECC 1-bit data modify test init complete");
    return 0;
}

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_run
 * Description: Execute LPDDR4 inline ECC 1-bit correctable error test.
 *              Configures ECC registers, scrubs regions, writes data patterns,
 *              enables ECC interrupts, reads back data to trigger ECC checks,
 *              and validates data integrity and interrupt behavior.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   0 on success (PASS), -1 on failure (FAIL).
 */
int lpddr4_inline_ecc_1bit_data_modify_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int offset;
    unsigned long int exp_data_array[50];
    unsigned long int read_data;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    // Initialize local variables as per Meta
    num_slverr_taken = 0;
    read_data = 0;

    LOGT("LPDDR4 inline ECC 1-bit data modify test run start");

    // Meta Step 13: write_reg(ctl_base + 0x0000007c, 0x00000000) UMCTL2_REGS.ECCCTL
    write_reg(ctl_base + 0x0000007c, 0x00000000);
    LOGT("ECCCTL cleared (ctl_base+0x7c)");

    // Meta Step 14: write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK)
    write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("SBR done interrupt enabled (ctl_base+0x819C)");

    // Meta Step 15: ecc_region_map = 0x55
    ecc_region_map = 0x55;
    LOGT("ecc_region_map=0x%x", ecc_region_map);

    // Meta Step 16: write_reg(ctl_base + 0x00000320, 0x00000000) UMCTL2_REGS.SWCTL unlock
    write_reg(ctl_base + 0x00000320, 0x00000000);
    LOGT("SWCTL unlocked (ctl_base+0x320)");

    // Meta Step 17: write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4) UMCTL2_REGS.ECCCFG0
    write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4);
    LOGT("ECCCFG0 written: 0x%x (ctl_base+0x70)", (ecc_region_map << 8) | 0xb4);

    // Meta Step 18: write_reg(ctl_base + 0x00000074, 0x00000330) UMCTL2_REGS.ECCCFG1
    write_reg(ctl_base + 0x00000074, 0x00000330);
    LOGT("ECCCFG1 written: 0x330 (ctl_base+0x74)");

    // Meta Step 19: write_reg(ctl_base + 0x00000320, 0x00000001) UMCTL2_REGS.SWCTL lock
    write_reg(ctl_base + 0x00000320, 0x00000001);
    LOGT("SWCTL locked (ctl_base+0x320)");

    // Meta Step 20: int_expected = 0
    int_expected = 0;

    // Meta Step 21: offset = 0x80000000
    offset = 0x80000000UL;

    LOGT("int_expected=%u offset=0x%lx", int_expected, offset);

    // Meta Step 22: Scrub ECC-enabled regions
    LOGT("Scrubbing ECC-enabled regions");
    for (region = 0; region < 7; region++) {
        if ((ecc_region_map >> region) & 0x1) {
            LOGT("Scrubbing region %u at offset*region=0x%lx", region, offset * region);
            memory_init_scrb(offset * region);
        }
    }

    // Meta Step 23: Write 64-bit data patterns to all 7 regions via port1
    // MANUAL_REVIEW: Original DV used rand() to generate exp_data_array values.
    // Replaced with deterministic data. Verify patterns are appropriate for
    // triggering 1-bit ECC correctable errors in the target environment.
    LOGT("Writing data patterns to 7 regions via port1");
    for (region = 0; region < 7; region++) {
        exp_data_array[region] = deterministic_data_lo[region];
        exp_data_array[region] = exp_data_array[region] | ((unsigned long)deterministic_data_hi[region] << 32);
        write_reg64(port1_addr + (region * offset), exp_data_array[region]);
        LOGT("Region %u: wrote 0x%lx to port1_addr+0x%lx",
             region, exp_data_array[region], (unsigned long)(region * offset));
    }

    // Meta Step 24: Enable ECC corrected error and SBR done interrupts
    write_reg(ctl_base + 0x0000819C, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
    LOGT("ECC corrected error + SBR done interrupts enabled (ctl_base+0x819C)");

    // Meta Step 25: write_reg(ctl_base + 0x0000007c, 1 << 8) UMCTL2_REGS.ECCCTL interrupt enable
    write_reg(ctl_base + 0x0000007c, 1 << 8);
    LOGT("ECCCTL interrupt enable bit 8 set (ctl_base+0x7c)");

    // Meta Step 26: Read back data from each region to trigger ECC check
    LOGT("Reading back data from 7 regions to trigger ECC check");
    for (region = 0; region < 7; region++) {
        int_expected = (ecc_region_map >> region) & 0x1;
        int_pend1 = 1;
        read_reg64(port1_addr + (region * offset), &read_data);
        LOGT("Region %u: read_data=0x%lx int_expected=%u", region, read_data, int_expected);

        // Meta Step 26 wait_on(1000) - adapted to deterministic busy-wait
        for (volatile int d = 0; d < 1000; d++);

        // Meta Step 26 data comparison validation
        if (exp_data_array[0] != read_data) {
            LOGE("ERROR region 0 no int: exp_data = %lx, actual_data = %lx", exp_data_array[0], read_data);
            err0++;
        }
    }

    // Meta Step 27: finish(err0) -> PSV status return
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("Run complete: %s err0=%u",
         (out->status == 0) ? "PASS" : "FAIL", err0);

    return out->status;
}

/*
 * Function: lpddr4_inline_ecc_1bit_data_modify_test_teardown
 * Description: Teardown and final status for LPDDR4 inline ECC 1-bit test.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   0 on success, -1 on failure.
 */
int lpddr4_inline_ecc_1bit_data_modify_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("LPDDR4 inline ECC 1-bit data modify test teardown: err0=%u", err0);
    return (err0 == 0) ? 0 : -1;
}
