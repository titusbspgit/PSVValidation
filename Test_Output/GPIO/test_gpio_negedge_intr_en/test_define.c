// Test_Stage3 Agent generated test_define.c for test_gpio_negedge_intr_en
// High-level: Negative-edge interrupt enable and service validation for GPIO pads 8..39.
// All headers and defines are placed here. program.c must include only this file.

#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Macros extracted from testcase context (kept unchanged if present there)
#define CNT 49

// TODO: The following bit position macros must be provided by platform headers.
// If they are not available, pin configuration writes will set no bits (skipped gracefully).
// #define GPIO_DOE_BIT 20   // TODO: confirm data-out enable bit index
// #define GPIO_NEIE_BIT 18  // TODO: confirm negative-edge interrupt enable bit index
// #define GPIO_ICLR_BIT 16  // TODO: confirm per-pin raw clear bit index (W1C)

// TODO: Provide a platform macro for pad drive register address used to force pad levels.
// Example (do NOT hardcode here): #define GPIO_PAD_DRIVE_ADDR 0xA0243FFC

// Impacted registers (symbolic macros only; values come from included headers)
static const unsigned long impacted_registers[] = {
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_LSS_SYSREG_RAW_STCR1
};

// Skip array aligned with impacted_registers[]; set entries to 1 if a register must be skipped.
static unsigned char skip_array[sizeof(impacted_registers)/sizeof(impacted_registers[0])] = {0};

// Pin count used by this testcase (GPIO8..GPIO39 => 32 pins)
#define PIN_CNT 32u

// IRQ IDs (symbolic usage in case platform provides alternates)
#ifndef GPIO0_GIC_IRQ
#define GPIO0_GIC_IRQ 87u  // TODO: confirm or override via build flags
#endif

// Helper: compute per-pin configuration value using optional bit macros if available
static inline unsigned int gpio_pin_cfg_bits(void) {
    unsigned int v = 0u;
#ifdef GPIO_DOE_BIT
    v |= (1u << GPIO_DOE_BIT);
#endif
#ifdef GPIO_NEIE_BIT
    v |= (1u << GPIO_NEIE_BIT);
#endif
#ifdef GPIO_ICLR_BIT
    v |= (1u << GPIO_ICLR_BIT);
#endif
    return v;
}

// Helpers to drive pads high/low using an external pad-drive register if provided by platform.
static inline void drive_all_pads_high(void) {
#ifdef GPIO_PAD_DRIVE_ADDR
    write_reg(GPIO_PAD_DRIVE_ADDR, 0xFFFFFFFFu);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] GPIO_PAD_DRIVE_ADDR not defined. Skipping drive_all_pads_high()\n");
#endif
#endif
}

static inline void drive_single_pad_low(unsigned int bit_index) {
#ifdef GPIO_PAD_DRIVE_ADDR
    unsigned int mask = (1u << bit_index);
    // Clear only the selected bit; keep others high
    write_reg(GPIO_PAD_DRIVE_ADDR, ~mask);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] GPIO_PAD_DRIVE_ADDR not defined. Skipping drive_single_pad_low(bit=%u)\n", bit_index);
#endif
#endif
}

static inline void prepare_pad_high_then_fall(unsigned int bit_index) {
#ifdef GPIO_PAD_DRIVE_ADDR
    // Ensure all high, then create a single falling edge on the required bit
    write_reg(GPIO_PAD_DRIVE_ADDR, 0xFFFFFFFFu);
    wait_on(30);
    drive_single_pad_low(bit_index);
#else
#ifdef DEBUG_DISPLAY
    printf("[TODO] GPIO_PAD_DRIVE_ADDR not defined. Skipping prepare_pad_high_then_fall(bit=%u)\n", bit_index);
#endif
#endif
}

// Extern declarations for IRQ handler symbol resolution in some platforms
void Default_IRQHandler(void);
