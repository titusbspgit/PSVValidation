// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Global state for test execution
static volatile int test_err = 0;      // Cumulative error counter
static volatile int int_pend = 0;      // Set by main loop, cleared by ISR

// -----------------------------------------------------------------------------
// Function: Default_IRQHandler
// Purpose : Interrupt Service Routine for GPIO group interrupt servicing
// Notes   : Implements the exact ISR behavior as per Meta Test Steps
// -----------------------------------------------------------------------------
void Default_IRQHandler(void)
{
#ifdef DEBUG_DISPLAY
    printf("[ISR] Enter Default_IRQHandler()\n");
#endif
    // Read group interrupt status
    unsigned int grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] Group STS before clear = 0x%08X\n", grp);
#endif

    // Mask group while servicing
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);

    // Validate interrupt presence
    if (grp == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] No interrupt indicated in group status!\n");
#endif
        test_err++;
    }

    // Clear per-pin raw status by writing clear value to each pad control reg
    for (unsigned int j = 0u; j < 32u; j++) {
        unsigned long addr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(addr, 0x00010000u);
        wait_on(1);
    }

    // Verify group status cleared
    unsigned int after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] Group STS after clear = 0x%08X\n", after);
#endif
    if (after != 0x00000000u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] Group status not cleared after per-pin raw clear!\n");
#endif
        test_err++;
    }

    // Acknowledge system RAW and clear platform IRQ
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif

    // Re-enable group output
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Clear pending flag for main loop
    int_pend = 0;
#ifdef DEBUG_DISPLAY
    printf("[ISR] Exit Default_IRQHandler()\n");
#endif
}

// -----------------------------------------------------------------------------
// Function: test_case
// Purpose : Entry point for the testcase execution
// Returns : finish(0) on PASS, finish(1) on FAIL (no other termination)
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[TEST] test_gpio_pedge_all_pads_en: START\n");
#endif

    // Platform IRQ enable and system interrupt routing
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Configure pads for positive-edge detection
    for (unsigned int i = 0u; i < 32u; i++) {
        unsigned long addr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u);     // Positive-edge enable per Meta
        wait_on(10);
    }

    // Configure IO direction groups (input mode pattern as provided)
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    wait_on(5);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    wait_on(5);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    wait_on(5);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(5);

    // Enable all per-pad interrupts in the group
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Main loop: stimulate rising edge per pad and wait for ISR to service
    for (unsigned int i = 0u; i < 32u; i++) {
#ifdef DEBUG_DISPLAY
        printf("[TEST] Iteration i=%u: Stimulate rising edge\n", i);
#endif
        // Drive stimulus low, arm, then drive high for rising edge
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(30);
        int_pend = 1;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu);

        // Poll with bounded timeout for ISR to clear int_pend
        int timeout = 5000;
        while ((int_pend != 0) && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[TEST][ERR] Timeout waiting for rising-edge interrupt on pad %u\n", i);
#endif
            test_err++;
            break; // As per procedure, break on timeout
        }

        // Optionally drive low again as per procedure and wait briefly
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    printf("[TEST] Completed with test_err=%d\n", test_err);
#endif

    if (test_err == 0) {
#ifdef DEBUG_DISPLAY
        printf("[TEST] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[TEST] FAIL\n");
#endif
        finish(1);
    }

    // Unreachable, required to satisfy function signature
    return 0;
}
