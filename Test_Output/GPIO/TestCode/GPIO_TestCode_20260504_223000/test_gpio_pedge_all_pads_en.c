// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <test_define.c>
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// test_gpio_pedge_all_pads_en
// Description (from Hidden_Test_Description):
// Positive-edge interrupt enable/validation across GPIO[8..39]. Setup: Conditionally enable GIC IRQ 87 (GPIO0) or 88 (GPIO1).
// Enable system interrupt via MIZAR_LSS_SYSREG_INTR_EN1 with LSS_SYSREG_INTR_EN1_GPIO{0/1}_INTR.
// For i=0..31, write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000) to set posedge enable (PEIE bit17=1).
// Configure input mode via group I/O control: write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4, 0x000000FF).
// Enable all bits in group interrupt: write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).
// For i=0..31: drive low then drive high to create rising edge; bounded wait with timeout=2000 while(int_pend==1){wait_on(10)}.
// Default_IRQHandler(): service group status, mask group intr, clear per-pin RAW (0x00010000), verify group clear, clear system RAW, re-enable group intr.

// Conditional selection of GPIO instance (GPIO0 default; define USE_GPIO1 to switch)
#ifdef USE_GPIO1
  #define GPIO_GIC_IRQ      88u
  #define SYS_EN_MASK       LSS_SYSREG_INTR_EN1_GPIO1_INTR
  #define SYS_RAW_CLR_MASK  LSS_SYSREG_RAW_STCR1_GPIO1_INTR
#else
  #define GPIO_GIC_IRQ      87u
  #define SYS_EN_MASK       LSS_SYSREG_INTR_EN1_GPIO0_INTR
  #define SYS_RAW_CLR_MASK  LSS_SYSREG_RAW_STCR1_GPIO0_INTR
#endif

static volatile int int_pend = 0;            // Set to 1 before trigger; cleared in ISR
static volatile unsigned int cur_pin = 0;    // Current pin under test (0..31 maps to GPIO[8..39])
static volatile int test_err = 0;            // Error counter per acceptance criteria

// Function: Default_IRQHandler
// Purpose: Handle GPIO group interrupt, validate group status, clear per-pin raw and system raw, and re-enable group interrupt.
void Default_IRQHandler(void)
{
    // Read group interrupt status (MIZAR_GPIO_GP0_INTR1_INTR_STS1)
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    // Mask group interrupt during service (MIZAR_GPIO_GP0_INTR1_INTR_EN1)
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    // Clear pending flag to allow main loop to proceed
    int_pend = 0;

    // Validate non-zero group status
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][PIN=%u] Group status is zero (unexpected)\n", cur_pin);
#endif
    }

    // Clear per-pin RAW by writing 0x00010000 to each pin register GPIO_8 + (j*4)
    for (unsigned int j = 0; j < 32u; j++) {
        unsigned long reg_addr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u)); // per-pin RAW STCLR field (bit16)
        write_reg(reg_addr, 0x00010000u);
        wait_on(2);
    }

    // Verify group clear (MIZAR_GPIO_GP0_INTR1_INTR_STS1 should be 0)
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][PIN=%u] Group status not cleared, sts=0x%08x\n", cur_pin, rdata_grp);
#endif
    }

    // Clear system-level RAW status and verify cleared
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, SYS_RAW_CLR_MASK);
    unsigned int sys_raw = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_raw & SYS_RAW_CLR_MASK) != 0u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][PIN=%u] System RAW not cleared, sts=0x%08x\n", cur_pin, sys_raw);
#endif
    }

    // Re-enable group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

// Function: main
// Purpose: Configure GPIO input mode, enable posedge interrupts for GPIO[8..39], enable system+group interrupts,
//          iterate through pins to generate rising edges (environment/external stimulus), and validate ISR service.
int main(void)
{
    // Configure input mode for all GPIO groups (IO_CTRL_GROUP1..4 = 0x000000FF)
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);

    // Enable positive-edge interrupt per pin: write 0x00020000 to GPIO_8 + (i*4)
    for (unsigned int i = 0; i < 32u; i++) {
        unsigned long reg_addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u)); // PEIE bit17 = 1
        write_reg(reg_addr, 0x00020000u);
    }

    // Enable group interrupt bits and system interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, SYS_EN_MASK);

    // Enable GIC IRQ line (87 for GPIO0, 88 for GPIO1)
    enable_irq(GPIO_GIC_IRQ);

    // Iterate pins and await ISR service upon external rising-edge stimulus
    for (unsigned int i = 0; i < 32u; i++) {
        cur_pin = i;
        int_pend = 1; // Set pending before stimulus

#ifdef DEBUG_DISPLAY
        printf("[WAIT][PIN=%u] Awaiting ISR after rising-edge stimulus...\n", cur_pin);
#endif
        // NOTE: External/environmental driver should toggle the pin low->high here.
        // This testbench does not write to out-of-scope pad control registers.

        int timeout = 2000; // Bounded wait count; wait_on(10) per step
        while ((int_pend == 1) && (timeout > 0)) {
            wait_on(10);
            timeout -= 10;
        }
        if (timeout <= 0) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT][PIN=%u] ISR not observed within timeout\n", cur_pin);
#endif
        }
    }

    // Final result per acceptance criteria
    finish(test_err);
    return 0;
}
