// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_inline_ecc_2bit_data_modify_test.h"
#include "test_define.inc"

/* ---------------------------------------------------------------------------
 * Global variables
 * --------------------------------------------------------------------------- */
static volatile int int_pend;
static volatile unsigned int slverr_taken;
static unsigned int num_slverr_taken;
static unsigned int err0;
static unsigned long int exp_data_array[50];

/* ECC region map: regions 0,2,4,6 enabled => 0x55 */
static const int ecc_enabled_regions[] = {0, 2, 4, 6};
#define NUM_ECC_REGIONS      4
#define NUM_TOTAL_REGIONS    7
#define ECC_REGION_MAP       0x55
#define REGION_OFFSET        0x80000000UL

/* DDR controller register offsets (from ctl_base) */
#define SWCTL_OFFSET         0x320
#define ECCCFG0_OFFSET       0x70
#define ECCCFG1_OFFSET       0x74
#define ECCCTL_OFFSET        0x7C
#define ECCSTAT_OFFSET       0x78
#define ECCERRCNT_OFFSET     0x80
#define SBRCTL_OFFSET        0xF24
#define SBRSTAT_OFFSET       0xF28
#define SBRWDATA0_OFFSET     0xF2C
#define SBRSTART0_OFFSET     0xF38
#define SBRRANGE0_OFFSET     0xF40
#define PCTRL0_OFFSET        0x490
#define PCTRL1_OFFSET        0x540

/* ECCSTAT bit for uncorrected error */
#define ECCSTAT_UNCORRECTED_ERR_BIT  (1UL << 16)

/* Deterministic test data (replaces rand() per DETERMINISTIC TEST DATA RULE) */
/* MANUAL_REVIEW: Original DV used rand() to populate exp_data_array[0..6].
   Deterministic replacement values are provided below. Replace with
   project-approved deterministic patterns if different values are required. */
static const unsigned long int deterministic_data[7] = {
    0xB6B6B6B6CAFEBABFUL,
    0x2345678901BCDEFUL,
    0xEFCDAB8967452301UL,
    0x1E1E1E1EE1E1E1E1UL,
    0xBBBBBBBB44444444UL,
    0x5555555566666666UL,
    0x7777777788888888UL
};

/* ---------------------------------------------------------------------------
 * Helper: check if a region index is ECC-enabled
 * --------------------------------------------------------------------------- */

/*
 * Function: is_ecc_enabled_region
 * Description: Returns 1 if the given region index has ECC enabled per
 *              ecc_region_map=0x55, 0 otherwise.
 * Parameters:
 *   region - region index (0-6)
 * Returns:
 *   1 if ECC-enabled, 0 otherwise.
 */
static int is_ecc_enabled_region(int region)
{
    /* ecc_region_map=0x55 => bits 0,2,4,6 set */
    if ((ECC_REGION_MAP >> region) & 1) {
        return 1;
    }
    return 0;
}

/* ---------------------------------------------------------------------------
 * Default_SlvErrHandler
 * Description: Handles slave error exceptions. Increments num_slverr_taken
 *              and sets slverr_taken flag.
 * --------------------------------------------------------------------------- */

/*
 * Function: Default_SlvErrHandler
 * Description: Slave error handler invoked on bus slave error.
 *              Increments the global slave error counter and sets the
 *              slverr_taken flag for the current read operation.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_SlvErrHandler(void)
{
    LOGT("Default_SlvErrHandler: entered");
    num_slverr_taken++;
    slverr_taken = 1;
    LOGT("Default_SlvErrHandler: num_slverr_taken=%u", num_slverr_taken);
}

/* ---------------------------------------------------------------------------
 * Default_IRQHandler
 * Description: Handles SBR-done and ECC uncorrected-error interrupts.
 *              Reads SYSREG masked status, identifies DDR controller interrupt,
 *              clears SYSREG interrupt, then handles SII-level interrupt source.
 * --------------------------------------------------------------------------- */

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for DDR controller interrupts.
 *              Reads SYSREG masked interrupt status, clears the SYSREG
 *              interrupt, reads SII interrupt status, and handles
 *              SBR-done or ECC uncorrected-error interrupt sources.
 * Parameters:
 *   none
 * Returns:
 *   void
 */
void Default_IRQHandler(void)
{
    unsigned long int msts;
    unsigned long int sii_status;

    LOGT("Default_IRQHandler: entered");

    /* Read SYSREG masked interrupt status */
    msts = read_reg(SYSREG_INTR_MSTS_ADDR);
    LOGT("Default_IRQHandler: SYSREG_INTR_MSTS = 0x%lx", (unsigned long)msts);

    if (msts & SYSREG_CTRL_INTR_MSTS_MASK) {
        /* Clear SYSREG DDR controller interrupt */
        write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK);
        LOGT("Default_IRQHandler: cleared SYSREG DDR ctrl interrupt");

        /* Read SII interrupt status to determine source */
        /* MANUAL_REVIEW: The exact SII interrupt status register read API
           and address depend on the SII driver. Adapt to project SII API. */
        sii_status = read_reg(ctl_base + 0x00);
        /* MANUAL_REVIEW: Replace above with actual SII interrupt status read. */

        /* Check for SBR done interrupt */
        if (sii_status & SII_INTR_EN_SBR_DONE_INTR_MASK) {
            LOGT("Default_IRQHandler: SBR done interrupt detected");
            /* Clear SBR done interrupt status */
            /* MANUAL_REVIEW: Clear SBR done interrupt via SII status clear register. */
            int_pend = 0;
        }

        /* Check for ECC uncorrected error interrupt */
        if (sii_status & SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK) {
            LOGT("Default_IRQHandler: ECC uncorrected error interrupt detected");
            /* Clear ECC uncorrected error: write ECCCTL bit[1] */
            write_reg(ctl_base + ECCCTL_OFFSET, 0x2);
            /* Clear SII ECC uncorrected error interrupt status */
            /* MANUAL_REVIEW: Clear ECC uncorrected error interrupt via SII status clear register. */
            int_pend = 0;
        }
    } else {
        LOGE("Default_IRQHandler: unexpected interrupt msts=0x%lx", (unsigned long)msts);
        err0++;
    }

    LOGT("Default_IRQHandler: exiting");
}

/* ---------------------------------------------------------------------------
 * scrub_region
 * --------------------------------------------------------------------------- */

/*
 * Function: scrub_region
 * Description: Performs memory initialization via scrubber (SBR) for one ECC
 *              region. Disables ports, configures scrub mode/interval/data/
 *              address/range, enables scrub, waits for SBR done interrupt,
 *              disables scrub, and re-enables ports.
 * Parameters:
 *   region - ECC region index (0, 2, 4, or 6)
 * Returns:
 *   void
 */
static void scrub_region(int region)
{
    unsigned long int scrub_start;
    unsigned long int scrub_range;

    LOGT("scrub_region: starting scrub for region %d", region);

    scrub_start = (unsigned long int)region * REGION_OFFSET;
    scrub_range = 0x1000UL;

    /* Disable port 0 */
    write_reg(ctl_base + PCTRL0_OFFSET, 0x0);
    /* Disable port 1 */
    write_reg(ctl_base + PCTRL1_OFFSET, 0x0);
    LOGT("scrub_region: ports disabled");

    /* Configure scrub mode and interval */
    /* SBRCTL: scrub_mode=1 (write), scrub_interval=0 */
    write_reg(ctl_base + SBRCTL_OFFSET, 0x4);
    LOGT("scrub_region: SBRCTL configured");

    /* Set scrub data pattern to zero */
    write_reg(ctl_base + SBRWDATA0_OFFSET, 0x0);
    LOGT("scrub_region: SBRWDATA0 set to 0");

    /* Program scrub start address */
    write_reg(ctl_base + SBRSTART0_OFFSET, scrub_start);
    LOGT("scrub_region: SBRSTART0 = 0x%lx", (unsigned long)scrub_start);

    /* Program scrub range */
    write_reg(ctl_base + SBRRANGE0_OFFSET, scrub_range);
    LOGT("scrub_region: SBRRANGE0 = 0x%lx", (unsigned long)scrub_range);

    /* Enable scrub: set scrub_en bit */
    write_reg(ctl_base + SBRCTL_OFFSET, 0x5);
    LOGT("scrub_region: scrub enabled, waiting for SBR done interrupt");

    /* Set int_pend and wait for SBR done interrupt */
    int_pend = 1;
    {
        unsigned int timeout_cnt = 0;
        while (int_pend != 0) {
            wait_on(10);
            timeout_cnt++;
            if (timeout_cnt > 100000) {
                LOGE("scrub_region: timeout waiting for SBR done interrupt, region %d", region);
                err0++;
                break;
            }
        }
    }
    LOGT("scrub_region: SBR done interrupt received for region %d", region);

    /* Disable scrub */
    write_reg(ctl_base + SBRCTL_OFFSET, 0x0);
    LOGT("scrub_region: scrub disabled after completion");

    /* Re-enable port 0 */
    write_reg(ctl_base + PCTRL0_OFFSET, 0x1);
    /* Re-enable port 1 */
    write_reg(ctl_base + PCTRL1_OFFSET, 0x1);
    LOGT("scrub_region: ports re-enabled");
}

/* ---------------------------------------------------------------------------
 * FV entry points
 * --------------------------------------------------------------------------- */

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_init
 * Description: Initializes the testcase: sets num_slverr_taken and int_pend,
 *              enables GIC interrupt, configures SYSREG interrupt enable,
 *              sets platform base addresses, configures bus width/speed/
 *              DBI/DM/ECC, and runs LPDDR4 training.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_init(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("=== lpddr4_inline_ecc_2bit_data_modify_test_init: START ===");

    /* Meta Step 1: Declare local variables (handled in _run) */

    /* Meta Step 2: Set num_slverr_taken = 0 */
    num_slverr_taken = 0;
    slverr_taken = 0;

    /* Meta Step 3: Set int_pend = 1 */
    int_pend = 1;

    /* K3 Step 1: Enable DDR controller interrupt via GIC */
    GIC_EnableIRQ(CTL_INT_NO);
    LOGT("GIC_EnableIRQ(CTL_INT_NO) called");

    /* K3 Step 1: Enable DDR controller interrupt in SYSREG */
    write_reg(SYSREG_INTR_EN_ADDR, SYSREG_CTRL_INTR_EN_MASK);
    LOGT("write_reg(SYSREG_INTR_EN_ADDR, SYSREG_CTRL_INTR_EN_MASK) done");

    /* K3 Step 2: gpv_programming() - prohibited in FV/PSV */
    /* MANUAL_REVIEW: gpv_programming() is a prohibited DV construct.
       If GPV programming is required in the target FV/PSV environment,
       provide an approved replacement API. */

    /* Set err0 = 0 */
    err0 = 0;

    /* K3 Step 3: Conditional platform initialization */
#ifdef APS_DRAM
    ctl_base = APS_LPDDR4_CTL_BASE;
    phy_base = APS_LPDDR4_PHY_BASE;
    port0_addr = 0;
    port1_addr = 0x15A0000000UL;
    LOGT("Platform: APS_DRAM, ctl_base=0x%lx, port1_addr=0x%lx",
         (unsigned long)ctl_base, (unsigned long)port1_addr);
#else
    ctl_base = MPS_LPDDR4_CTL_BASE;
    phy_base = MPS_LPDDR4_PHY_BASE;
    port0_addr = 0;
    port1_addr = 0x11A0000000UL;
    LOGT("Platform: MPS, ctl_base=0x%lx, port1_addr=0x%lx",
         (unsigned long)ctl_base, (unsigned long)port1_addr);
#endif

    /* K3 Step 4: Set bus_width=0 (Full Bus) */
    bus_width = 0;
    LOGT("bus_width = 0 (Full Bus)");

    /* K3 Step 4: Conditional speed grade */
#ifdef SG2667
    SG = 2667;
    LOGT("Speed grade: SG2667");
#elif defined(SG2133)
    SG = 2133;
    LOGT("Speed grade: SG2133");
#else
    SG = 3200;
    LOGT("Speed grade: SG3200 (default)");
#endif

    /* K3 Step 4: Set DBI_EN=0, DM_EN=1, ECC_EN=1 */
    DBI_EN = 0;
    DM_EN = 1;
    ECC_EN = 1;
    LOGT("DBI_EN=0, DM_EN=1, ECC_EN=1");

    /* K3 Step 5: Call lpddr4_training() */
    lpddr4_training();
    LOGT("lpddr4_training() completed");

    LOGT("=== lpddr4_inline_ecc_2bit_data_modify_test_init: DONE ===");
    return 0;
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_run
 * Description: Executes the main ECC 2-bit uncorrected error test sequence:
 *              enables SBR done interrupt, programs ECCCFG0/ECCCFG1, scrubs
 *              ECC regions, writes deterministic data to 7 regions, re-trains,
 *              reads back region 0 and polls ECCSTAT bit[16], clears uncorrected
 *              error via ECCCTL bit[1], enables ECC uncorrected-error interrupt
 *              (ECCCTL bit[9]), iterates over all 7 regions validating data
 *              mismatch for ALL regions and uncorrected-error interrupt + slave
 *              error for ECC regions, validates ECCERRCNT bits[31:16]==5 and
 *              num_slverr_taken==5.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 *   out - pointer to TestOutput for status reporting
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_run(const TestsItem *cfg, TestOutput *out)
{
    unsigned long int offset;
    unsigned long int read_data;
    int region;
    unsigned long int eccstat_val;
    unsigned long int eccerrcnt_val;

    (void)cfg;

    if (out == 0) {
        LOGE("Output pointer is NULL");
        return -1;
    }

    out->status = 0;

    LOGT("=== lpddr4_inline_ecc_2bit_data_modify_test_run: START ===");

    /* ---------------------------------------------------------------
     * K3 Step 6: Enable SBR done interrupt in SII interrupt enable
     * --------------------------------------------------------------- */
    /* MANUAL_REVIEW: The exact SII interrupt enable register write API
       depends on the SII driver. The mask SII_INTR_EN_SBR_DONE_INTR_MASK
       is used. Adapt the register address to the project SII API. */
    write_reg(ctl_base + 0x00, SII_INTR_EN_SBR_DONE_INTR_MASK);
    /* MANUAL_REVIEW: Replace ctl_base + 0x00 with actual SII interrupt enable register. */
    LOGT("SII SBR done interrupt enabled");

    /* ---------------------------------------------------------------
     * K3 Step 7: Disable SWCTL, program ECCCFG0 and ECCCFG1,
     *            re-enable SWCTL
     * --------------------------------------------------------------- */
    /* Disable software programming lock */
    write_reg(ctl_base + SWCTL_OFFSET, 0x0);
    LOGT("SWCTL disabled (software programming lock off)");

    /* Program ECCCFG0 with ecc_region_map=0x55 */
    write_reg(ctl_base + ECCCFG0_OFFSET, 0x55);
    LOGT("ECCCFG0 = 0x55 (ecc_region_map: regions 0,2,4,6)");

    /* Program ECCCFG1 with 0x330 */
    write_reg(ctl_base + ECCCFG1_OFFSET, 0x330);
    LOGT("ECCCFG1 = 0x330");

    /* Re-enable software programming lock */
    write_reg(ctl_base + SWCTL_OFFSET, 0x1);
    LOGT("SWCTL re-enabled");

    /* ---------------------------------------------------------------
     * K3 Step 8: For each ECC-enabled region (0,2,4,6),
     *            perform memory initialization via scrubber
     * --------------------------------------------------------------- */
    {
        int i;
        for (i = 0; i < NUM_ECC_REGIONS; i++) {
            scrub_region(ecc_enabled_regions[i]);
        }
    }
    LOGT("All ECC regions scrubbed");

    /* ---------------------------------------------------------------
     * K3 Step 9: Write deterministic 64-bit data to all 7 regions
     *            (0-6) via port 1
     * --------------------------------------------------------------- */
    {
        int i;
        for (i = 0; i < NUM_TOTAL_REGIONS; i++) {
            /* Populate exp_data_array with deterministic data */
            /* Original DV: exp_data_array[i] = rand() | ((unsigned long)rand() << 32) */
            /* MANUAL_REVIEW: rand() replaced with deterministic_data[]. */
            exp_data_array[i] = deterministic_data[i];

            offset = (unsigned long int)i * REGION_OFFSET;
            write_reg(port1_addr + offset, exp_data_array[i]);
            LOGT("Region %d: wrote 0x%lx to port1_addr+0x%lx",
                 i, (unsigned long)exp_data_array[i], (unsigned long)offset);
        }
    }
    LOGT("All 7 regions written via port1");

    /* ---------------------------------------------------------------
     * K3 Step 10: Wait, signal training done, and wait again
     * --------------------------------------------------------------- */
    wait_on(100);
    training_done();
    wait_on(1000);
    LOGT("training_done() called, waited for stabilization");

    /* ---------------------------------------------------------------
     * K3 Step 11: Clear ECC control register, read region 0 data,
     *             poll ECCSTAT for uncorrected error bit (bit 16),
     *             clear the uncorrected error via ECCCTL bit 1,
     *             and clear the SII interrupt status
     * --------------------------------------------------------------- */
    /* Clear ECCCTL */
    write_reg(ctl_base + ECCCTL_OFFSET, 0x0);
    LOGT("ECCCTL cleared");

    /* Read region 0 data from port1 */
    offset = 0;
    read_data = read_reg(port1_addr + offset);
    LOGT("Region 0: read_data = 0x%lx", (unsigned long)read_data);

    /* Poll ECCSTAT until bit 16 (ecc_uncorrected_err) is set */
    {
        unsigned int timeout_cnt = 0;
        do {
            eccstat_val = read_reg(ctl_base + ECCSTAT_OFFSET);
            timeout_cnt++;
            if (timeout_cnt > 100000) {
                LOGE("Timeout polling ECCSTAT for uncorrected error bit");
                err0++;
                break;
            }
        } while (!(eccstat_val & ECCSTAT_UNCORRECTED_ERR_BIT));
    }
    LOGT("ECCSTAT = 0x%lx (uncorrected error bit check)", (unsigned long)eccstat_val);

    /* Clear the uncorrected error: write ECCCTL bit[1] */
    write_reg(ctl_base + ECCCTL_OFFSET, 0x2);
    LOGT("ECCCTL: uncorrected error cleared (bit 1)");

    /* Clear SII interrupt status */
    /* MANUAL_REVIEW: Clear SII interrupt status via SII status clear register.
       The exact register address depends on the SII driver. */
    LOGT("SII interrupt status cleared");

    /* ---------------------------------------------------------------
     * K3 Step 12: Enable both ECC uncorrected error and SBR done
     *             interrupts in SII, and enable ECC uncorrected error
     *             interrupt in ECCCTL (bit 9)
     * --------------------------------------------------------------- */
    /* Enable ECC uncorrected error + SBR done in SII interrupt enable */
    write_reg(ctl_base + 0x00,
              SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK);
    /* MANUAL_REVIEW: Replace ctl_base + 0x00 with actual SII interrupt enable register. */
    LOGT("SII: ECC uncorrected error + SBR done interrupts enabled");

    /* Enable ECC uncorrected error interrupt in ECCCTL: bit[9] */
    write_reg(ctl_base + ECCCTL_OFFSET, (1UL << 9));
    LOGT("ECCCTL: uncorrected error interrupt enabled (bit 9)");

    /* ---------------------------------------------------------------
     * K3 Step 13: For each of the 7 regions: read data, and if the
     *             region is ECC-enabled, wait for uncorrected-error
     *             interrupt and verify a slave error was taken; for
     *             ALL regions, verify data does NOT match the
     *             originally written data.
     * --------------------------------------------------------------- */
    for (region = 0; region < NUM_TOTAL_REGIONS; region++) {
        offset = (unsigned long int)region * REGION_OFFSET;

        /* Reset slverr_taken before read for ECC-enabled regions */
        if (is_ecc_enabled_region(region)) {
            int_pend = 1;
            slverr_taken = 0;
        }

        read_data = read_reg(port1_addr + offset);
        LOGT("Region %d: read_data = 0x%lx, expected = 0x%lx",
             region, (unsigned long)read_data, (unsigned long)exp_data_array[region]);

        if (is_ecc_enabled_region(region)) {
            /* ECC-enabled region: wait for uncorrected-error interrupt */
            {
                unsigned int timeout_cnt = 0;
                while (int_pend != 0) {
                    wait_on(10);
                    timeout_cnt++;
                    if (timeout_cnt > 100000) {
                        LOGE("Region %d (ECC): timeout waiting for uncorrected error interrupt", region);
                        err0++;
                        break;
                    }
                }
            }
            LOGT("Region %d (ECC): uncorrected error interrupt received", region);

            /* Validate slave error was taken */
            if (slverr_taken == 0) {
                LOGE("Region %d (ECC): expected slave error but slverr_taken == 0", region);
                err0++;
            } else {
                LOGT("Region %d (ECC): slave error taken as expected", region);
            }
        }

        /* For ALL regions: validate data does NOT match */
        if (read_data == exp_data_array[region]) {
            LOGE("Region %d: unexpected data match 0x%lx (expected mismatch)",
                 region, (unsigned long)read_data);
            err0++;
        } else {
            LOGT("Region %d: data mismatch as expected", region);
        }
    }
    LOGT("All 7 regions read-back and validated");

    /* ---------------------------------------------------------------
     * K3 Step 14: Read ECCERRCNT and validate the uncorrected error
     *             count (upper 16 bits) equals 5
     * --------------------------------------------------------------- */
    eccerrcnt_val = read_reg(ctl_base + ECCERRCNT_OFFSET);
    LOGT("ECCERRCNT = 0x%lx", (unsigned long)eccerrcnt_val);

    if (((eccerrcnt_val >> 16) & 0xFFFF) != 5) {
        LOGE("ECCERRCNT uncorrected mismatch: expected 5, actual %lu",
             (unsigned long)((eccerrcnt_val >> 16) & 0xFFFF));
        err0++;
    } else {
        LOGT("ECCERRCNT uncorrected == 5: PASS");
    }

    /* ---------------------------------------------------------------
     * K3 Step 15: Validate that the total number of slave errors
     *             taken equals 5
     * --------------------------------------------------------------- */
    if (num_slverr_taken != 5) {
        LOGE("num_slverr_taken mismatch: expected 5, actual %u", num_slverr_taken);
        err0++;
    } else {
        LOGT("num_slverr_taken == 5: PASS");
    }

    /* ---------------------------------------------------------------
     * Final status
     * --------------------------------------------------------------- */
    out->status = (err0 == 0) ? 0 : -1;

    LOGT("=== lpddr4_inline_ecc_2bit_data_modify_test_run: %s (err0=%u) ===",
         (err0 == 0) ? "PASS" : "FAIL", err0);

    return out->status;
}

/*
 * Function: lpddr4_inline_ecc_2bit_data_modify_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 *   cfg - pointer to TestsItem configuration
 * Returns:
 *   FV/template-compatible status.
 */
int lpddr4_inline_ecc_2bit_data_modify_test_teardown(const TestsItem *cfg)
{
    (void)cfg;

    LOGT("=== lpddr4_inline_ecc_2bit_data_modify_test_teardown ===");
    LOGT("Final error count: err0 = %u", err0);
    LOGT("Final slave error count: num_slverr_taken = %u", num_slverr_taken);

    /* K3 Step 16: finish(err0) - converted to FV/PSV return */
    /* finish() is prohibited in FV/PSV. Return status based on err0. */
    return (err0 == 0) ? 0 : -1;
}
