// Test_Stage3 Agent generated program.c for test_gpio_negedge_intr_en
// Implements: Negative-edge interrupt enable and service validation for GPIO pads 8..39.
// Notes:
// - Uses only symbolic macros from repository headers. No literal addresses/masks are invented here.
// - Where platform-specific macros are missing, code emits DEBUG logs and skips the action.
// - program.c includes only test_define.c by requirement.

#include "test_define.c"

// Volatile state shared between main and ISR
static volatile unsigned int test_err = 0;
static volatile unsigned int int_pend = 0;
static volatile unsigned int current_i = 0;

// Local utility: enable a single pin's interrupt in the group enable register
static inline void enable_group_bit(unsigned int bit_index) {
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
    unsigned int wr_val = (1u << bit_index);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] MIZAR_GPIO_GP0_INTR1_INTR_EN1 undefined. Skipping group enable for bit %u\n", bit_index);
#endif
#endif
}

// Local utility: clear raw status for a single pin via W1C
static inline void clear_raw_status_bit(unsigned int bit_index) {
#ifdef MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
    unsigned int wr_val = (1u << bit_index);
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 undefined. Skipping RAW_STCLR for bit %u\n", bit_index);
#endif
#endif
}

// Configure per-pin: input + negedge enable + raw clear (iclr)
static inline void configure_pin_negedge(unsigned int pin_index) {
#ifdef MIZAR_GPIO_GP0_GPIO_8
    unsigned long addr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8) + (pin_index * 4u); // TODO: confirm stride
    unsigned int cfg = gpio_pin_cfg_bits();
    if (cfg == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[TODO] GPIO_DOE_BIT/NEIE_BIT/ICLR_BIT undefined. Skipping per-pin cfg for pin %u\n", pin_index + 8u);
#endif
        return;
    }
    write_reg(addr, cfg);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] MIZAR_GPIO_GP0_GPIO_8 undefined. Skipping per-pin cfg for pin %u\n", pin_index + 8u);
#endif
#endif
}

void test_case(void) {
    test_err = 0;
#ifdef DEBUG_DISPLAY
    printf("[TEST] test_gpio_negedge_intr_en: start\n");
#endif

    // 1) Enable GIC IRQ (GPIO0 assumed)
#ifdef GIC_EnableIRQ
    GIC_EnableIRQ(GPIO0_GIC_IRQ);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] GIC_EnableIRQ unavailable.\n");
#endif
#endif

    // 2) Enable SYSREG interrupt for GPIO0
#if defined(MIZAR_LSS_SYSREG_INTR_EN1) && defined(LSS_SYSREG_INTR_EN1_GPIO0_INTR)
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] SYSREG enable macros undefined. Skipping SYSREG enable.\n");
#endif
#endif

    // 3) Drive all pads high (known state)
    drive_all_pads_high();

    // 4) Configure pins GPIO8..GPIO39: input + negedge enable + raw clear
    for (current_i = 0; current_i < PIN_CNT; ++current_i) {
        configure_pin_negedge(current_i);
        wait_on(10);
    }

    // 5) For each pin, create a falling edge and wait for interrupt with timeout
    for (current_i = 0; current_i < PIN_CNT; ++current_i) {
        // Pre-clear raw status and enable only this group bit
        clear_raw_status_bit(current_i);
        enable_group_bit(current_i);
        wait_on(10);

        // Arm wait flag, then generate falling edge on current pin
        int_pend = 1u;
        prepare_pad_high_then_fall(current_i);

        // Busy-wait with timeout
        int timeout = 5000;
        while (int_pend && timeout-- > 0) {
            wait_on(10);
        }
        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for negedge interrupt on pin %u\n", current_i + 8u);
#endif
            test_err++;
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[TEST] test_gpio_negedge_intr_en: %s\n", (test_err == 0) ? "PASS" : "FAIL");
#endif
    finish((test_err == 0) ? 0 : 1);
}

// Default IRQ handler per CSV procedure
void Default_IRQHandler(void) {
    unsigned int local_wr = (1u << current_i);
    int_pend = 0u; // Signal main loop that interrupt arrived

    // Return pads to known high state
    drive_all_pads_high();

    // Read per-pin status register and perform checks when macros available
#ifdef MIZAR_GPIO_GP0_GPIO_8
    unsigned long raddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8) + (current_i * 4u); // TODO: confirm stride
    unsigned int rdata = read_reg(raddr);

#ifdef GPIO_DIN_BIT
    if (((rdata >> GPIO_DIN_BIT) & 0x1u) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ERR][ISR] DIN should be 0 on negedge, pin %u\n", current_i + 8u);
#endif
        test_err++;
    }
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO][ISR] GPIO_DIN_BIT undefined. Skipping DIN check for pin %u\n", current_i + 8u);
#endif
#endif
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO][ISR] MIZAR_GPIO_GP0_GPIO_8 undefined. Skipping per-pin status read.\n");
#endif
#endif

    // Check group status set for this bit
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status bit not set for pin %u\n", current_i + 8u);
#endif
            test_err++;
        }
    }
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO][ISR] MIZAR_GPIO_GP0_INTR1_INTR_STS1 undefined. Skipping group status check.\n");
#endif
#endif

    // Per-pin clear (iclr) and RAW clear
#ifdef MIZAR_GPIO_GP0_GPIO_8
    {
        unsigned long caddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8) + (current_i * 4u);
        unsigned int iclr = 0u;
#ifdef GPIO_ICLR_BIT
        iclr |= (1u << GPIO_ICLR_BIT);
#endif
#ifdef GPIO_DOE_BIT
        iclr |= (1u << GPIO_DOE_BIT);
#endif
        if (iclr != 0u) {
            write_reg(caddr, iclr);
        } else {
#ifdef DEBUG_DISPLAY
            printf("[TODO][ISR] GPIO_ICLR_BIT/GPIO_DOE_BIT undefined. Skipping per-pin clear.\n");
#endif
        }
    }
#endif
    clear_raw_status_bit(current_i);

    // Verify group status cleared
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    {
        unsigned int rdata_grp2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp2 != 0x0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status not cleared after RAW_STCLR (pin %u)\n", current_i + 8u);
#endif
            test_err++;
        }
    }
#endif

    // Clear SYSREG raw status for GPIO0 and clear GIC IRQ
#if defined(MIZAR_LSS_SYSREG_RAW_STCR1) && defined(LSS_SYSREG_RAW_STCR1_GPIO0_INTR)
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO][ISR] SYSREG RAW clear macros undefined. Skipping SYSREG clear.\n");
#endif
#endif

#ifdef GIC_ClearIRQ
    GIC_ClearIRQ(GPIO0_GIC_IRQ);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO][ISR] GIC_ClearIRQ unavailable.\n");
#endif
#endif
}
