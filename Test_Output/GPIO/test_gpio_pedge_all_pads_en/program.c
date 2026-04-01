#include "test_define.c"

// Author: Test_Stage3 Agent
// Testcase: test_gpio_pedge_all_pads_en
// Description: Enable posedge interrupts on all GPIO[8..39], drive a rising edge per iteration
//              using PAD_DRIVE_REG, wait for ISR (int_pend cleared), verify group/system behavior,
//              then proceed to next pin. Logs via DEBUG_DISPLAY and reports pass/fail via finish().

extern volatile unsigned int int_pend; // ISR clears this to 0 on interrupt handling

static void configure_pins_posedge(void) {
    // Configure per-pin positive edge: write 0x00020000 to each GPIO[8..39] register
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned long raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4u);
        write_reg(raddr, 0x00020000u);
        wait_on(10);
    }
}

static void set_gpio_input_mode_groups(void) {
    // Put GPIOs 8-39 into input mode via IO control groups
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);
}

static int wait_for_isr_clear(unsigned int timeout_loops, unsigned int wait_ticks) {
    while ((int_pend == 1u) && (timeout_loops-- > 0u)) {
        wait_on(wait_ticks);
    }
    return (int_pend == 0u) ? 1 : 0;
}

void test_case(void) {
    int test_err = 0;

    DEBUG_DISPLAY("[GPIO] test_gpio_pedge_all_pads_en: START\n");

    // Enable IRQ and system interrupt based on build-time selection of GPIO instance
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#elif defined(GPIO1)
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#else
    DEBUG_DISPLAY("[GPIO] WARNING: Neither GPIO0 nor GPIO1 defined; ISR path may not trigger.\n");
#endif

    // Configure pins for posedge interrupt and set input mode
    configure_pins_posedge();
    set_gpio_input_mode_groups();

    // Enable group interrupt for GP0 INTR1
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Ensure pads start low
    write_reg(PAD_DRIVE_REG, 0x00000000u);
    wait_on(10);

    // For each pin, drive a low-to-high transition and wait for ISR to complete
    for (unsigned int i = 0; i < 32u; ++i) {
        (void)i; // index used by ISR in platform implementation; not directly here

        // Ensure low, then arm and drive high to generate posedge
        write_reg(PAD_DRIVE_REG, 0x00000000u);
        wait_on(10);
        int_pend = 1u;
        write_reg(PAD_DRIVE_REG, 0xFFFFFFFFu);

        // Poll for ISR to clear int_pend with timeout
        unsigned int timeout = 2000u;
        if (!wait_for_isr_clear(timeout, 10u)) {
            DEBUG_DISPLAY("[GPIO][ERR] Timeout waiting for ISR (posedge) on iteration %u\n", i);
            test_err++;
        } else {
            // Optional post-checks: group status should be cleared by ISR
            wait_on(10);
            unsigned int grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (grp != 0x0u) {
                DEBUG_DISPLAY("[GPIO][ERR] Group status not cleared after ISR; sts=0x%08X (iter %u)\n", grp, i);
                test_err++;
            }
        }

        // Drive low again before next iteration
        write_reg(PAD_DRIVE_REG, 0x00000000u);
        wait_on(10);

        // Re-enable group in case ISR masked it (per CSV ISR flow)
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    }

    if (test_err == 0) {
        DEBUG_DISPLAY("[GPIO] test_gpio_pedge_all_pads_en: PASS\n");
        finish(0);
    } else {
        DEBUG_DISPLAY("[GPIO] test_gpio_pedge_all_pads_en: FAIL (errors=%d)\n", test_err);
        finish(1);
    }
}
