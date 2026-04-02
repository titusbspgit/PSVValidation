#include "test_define.c"

// Test: test_gpio_pedge_all_pads_en
// Description: Positive-edge interrupt enable across all pads (GPIO8..GPIO39),
// group enable, ISR service with per-pin raw clear, group status verification,
// sysreg clear verification, and bounded wait.

static volatile unsigned int test_err = 0;
static volatile unsigned int int_pend = 0;
static volatile unsigned int current_pin = 0;

static inline void drive_all_high(void) {
    write_reg(GPIO_DRIVE_REG, 0xFFFFFFFFu);
}

static inline void drive_all_low(void) {
    write_reg(GPIO_DRIVE_REG, 0x00000000u);
}

static inline void drive_falling_edge(unsigned int pin)
{
    unsigned int mask = (1u << pin);
    write_reg(GPIO_DRIVE_REG, 0xFFFFFFFFu);
    wait_on(30);
    write_reg(GPIO_DRIVE_REG, ~mask);
}

static inline void drive_rising_edge(unsigned int pin)
{
    unsigned int mask = (1u << pin);
    write_reg(GPIO_DRIVE_REG, 0x00000000u);
    wait_on(10);
    write_reg(GPIO_DRIVE_REG, 0xFFFFFFFFu);
    (void)mask; // mask used only for documentation; full high drives rising edge on selected pad
}

void Default_IRQHandler(void)
{
    // Observe group status
    unsigned int rdata_grp = read_reg(REG_GPIO_INTR1_STS1);

    // Mask group interrupts during service
    write_reg(REG_GPIO_INTR1_EN1, 0x00000000u);

#ifdef DEBUG_DISPLAY
    if ((rdata_grp & 0xFFFFFFFFu) != 0u) {
        printf("[ISR] Group status non-zero: 0x%08X\n", rdata_grp);
    } else {
        printf("[ISR][ERR] Group status is zero, expected set.\n");
    }
#endif
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        test_err++;
    }

    // Per-pin raw clear: write iclr=1 (bit16) for all pins
    for (unsigned int j = 0; j < GPIO_PINS; ++j) {
        unsigned long addr_pin = REG_GPIO_PIN_BASE + (j * 4u);
        write_reg(addr_pin, GPIO_RAW_CLEAR_VAL);
    }
    wait_on(2);

    // Verify group clear
    rdata_grp = read_reg(REG_GPIO_INTR1_STS1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] Post-clear group status: 0x%08X\n", rdata_grp);
#endif
    if (rdata_grp != 0x0u) {
        test_err++;
    }

    // Clear sysreg raw status for GPIO0/GPIO1 interrupt line
#if defined(LSS_SYSREG_RAW_STCR1_GPIO0_INTR)
    write_reg(REG_LSS_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int sreg = read_reg(REG_LSS_RAW_STCR1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] SYSREG RAW_STCR1 after clear (GPIO0): 0x%08X\n", sreg);
#endif
    if ((sreg & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        test_err++;
    }
#elif defined(LSS_SYSREG_RAW_STCR1_GPIO1_INTR)
    write_reg(REG_LSS_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int sreg = read_reg(REG_LSS_RAW_STCR1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] SYSREG RAW_STCR1 after clear (GPIO1): 0x%08X\n", sreg);
#endif
    if ((sreg & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        test_err++;
    }
#else
#ifdef DEBUG_DISPLAY
    printf("[ISR][WARN] No SYSREG RAW_STCR1 GPIOx bit macro defined.\n");
#endif
#endif

    // Re-enable group interrupts and clear GIC
    write_reg(REG_GPIO_INTR1_EN1, 0xFFFFFFFFu);
#if defined(LSS_SYSREG_INTR_EN1_GPIO0_INTR)
    GIC_ClearIRQ(87);
#elif defined(LSS_SYSREG_INTR_EN1_GPIO1_INTR)
    GIC_ClearIRQ(88);
#else
    // Unknown line; best-effort clear both lines
    GIC_ClearIRQ(87);
    GIC_ClearIRQ(88);
#endif

    int_pend = 0;
}

void test_case(void)
{
    test_err = 0;

    // Enable GIC IRQ based on defined line
#if defined(LSS_SYSREG_INTR_EN1_GPIO0_INTR)
    GIC_EnableIRQ(87);
#elif defined(LSS_SYSREG_INTR_EN1_GPIO1_INTR)
    GIC_EnableIRQ(88);
#else
#ifdef DEBUG_DISPLAY
    printf("[MAIN][WARN] No LSS_SYSREG_INTR_EN1 GPIO line macro defined; enabling both 87 and 88.\n");
#endif
    GIC_EnableIRQ(87);
    GIC_EnableIRQ(88);
#endif

    // Enable SYSREG interrupt for the chosen GPIO instance
#if defined(LSS_SYSREG_INTR_EN1_GPIO0_INTR)
    write_reg(REG_LSS_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#elif defined(LSS_SYSREG_INTR_EN1_GPIO1_INTR)
    write_reg(REG_LSS_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#else
#ifdef DEBUG_DISPLAY
    printf("[MAIN][WARN] No LSS_SYSREG_INTR_EN1 GPIOx macro; writing 0 to avoid unintended enable.\n");
#endif
    write_reg(REG_LSS_INTR_EN1, 0x00000000u);
#endif

    // Known state: drive all pads low then high during per-pin operations
    drive_all_high();

    // Configure per-pin for positive-edge detection
    for (unsigned int i = 0; i < GPIO_PINS; ++i) {
        unsigned long addr_pin = REG_GPIO_PIN_BASE + (i * 4u);
        write_reg(addr_pin, GPIO_POSEDGE_EN_VAL);
    }
    wait_on(10);

    // Configure input mode for all pads via IO control groups
    for (unsigned int g = 0; g < 4u; ++g) {
        write_reg(REG_GPIO_IO_CTRL_GRP[g], 0x000000FFu);
    }
    wait_on(10);

    // Enable all group interrupt bits
    write_reg(REG_GPIO_INTR1_EN1, 0xFFFFFFFFu);

    // Per-pin stimulus and bounded wait
    for (unsigned int i = 0; i < GPIO_PINS; ++i) {
        if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[MAIN] Skipping pin %u as per skip_array.\n", i);
#endif
            continue;
        }

        // Prepare low level, then generate rising edge
        drive_all_low();
        wait_on(10);

        int_pend = 1;
        current_pin = i;

        drive_rising_edge(i);

        int timeout = 2000;
        while ((int_pend == 1u) && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout == 0) {
#ifdef DEBUG_DISPLAY
            printf("[MAIN][ERR] Timeout waiting for interrupt on pin %u.\n", i);
#endif
            test_err++;
            break; // optional: break on first timeout
        }

        // Return to low before next iteration
        drive_all_low();
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    if (test_err == 0) {
        printf("[RESULT] test_gpio_pedge_all_pads_en: PASS\n");
    } else {
        printf("[RESULT] test_gpio_pedge_all_pads_en: FAIL, errors=%u\n", test_err);
    }
#endif

    finish(test_err == 0 ? 0 : 1);
}
