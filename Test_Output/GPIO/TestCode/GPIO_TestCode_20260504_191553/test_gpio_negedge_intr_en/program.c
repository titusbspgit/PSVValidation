// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

// High-level description (from META):
// Negative-edge interrupt enable and validation across GPIO[8..39]. Sets input mode and negedge enable per pin,
// triggers a falling edge via external pad data register, checks pin DIN, masked status, and clearing at both
// GPIO and system-register levels.

#include "test_define.c"

// Globals for simple ISR synchronization
static volatile unsigned int int_pend = 0;
static volatile unsigned int current_index = 0;
static volatile unsigned int isr_err_accum = 0;

// Function: configure_pin_negedge
// Purpose: Configure a GPIO pin for input mode with negedge interrupt enable and clear pending status.
static inline void configure_pin_negedge(unsigned int idx)
{
    unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (idx * 4u)); // Per-pin config register
    // Bits per META: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16)
    unsigned int cfg = (1u << 20) | (1u << 18) | (1u << 16);
    write_reg(addr1, cfg);
    wait_on(10);
}

// Function: Default_IRQHandler
// Purpose: ISR that validates DIN, masked group status, clears per-pin and group/system RAW, and acks GIC.
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << current_index);
    int_pend = 0; // Signal main thread

    // Return pad to high
    write_reg(0xA0243FFCu, 0xFFFFFFFFu);

    // Read back per-pin data to ensure DIN low was captured on negedge
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (current_index * 4u));
    unsigned int rdata = read_reg(raddr);
    if ((rdata & 0x1u) != 0x0u) {
        ++isr_err_accum; // DIN should be low after negedge
#ifdef DEBUG_DISPLAY
        printf("[ISR-NEG] DIN not low idx=%u rdata=0x%08x\n", current_index, rdata);
#endif
    }

    // Expect masked status bit set in group status
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & local_wr) == 0u) {
        ++isr_err_accum;
#ifdef DEBUG_DISPLAY
        printf("[ISR-NEG] Group status bit not set idx=%u sts=0x%08x\n", current_index, rdata_grp);
#endif
    }

    // Clear per-pin: doe=1, iclr=1
    unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (current_index * 4u));
    write_reg(raddr2, (1u << 20) | (1u << 16));

    // Clear group RAW
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

    // Verify group status cleared
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        ++isr_err_accum;
#ifdef DEBUG_DISPLAY
        printf("[ISR-NEG] Group status not cleared sts=0x%08x\n", rdata_grp);
#endif
    }

    // Clear SYSREG RAW and ack GIC
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int sys_rb = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_rb & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        ++isr_err_accum;
    }
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int sys_rb = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_rb & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        ++isr_err_accum;
    }
    GIC_ClearIRQ(88);
#endif
}

// Function: test_case
// Purpose: Execute negedge interrupt enable test across all pins with bounded waits and validation.
int test_case(void)
{
    unsigned int test_err = 0u;

    // Enable system interrupt output and GIC line per selected GPIO instance
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Drive external pad bus to all ones initially
    write_reg(0xA0243FFCu, 0xFFFFFFFFu);

    // Configure all pins for input with negedge enable and clear
    for (unsigned int i = 0; i < 32u; ++i) {
        if (skip_array[i]) continue;
        configure_pin_negedge(i);
    }

    // Iterate each pin
    for (unsigned int i = 0; i < 32u; ++i) {
        if (skip_array[i]) continue;
        unsigned int wr_val = (1u << i);

        // Pre-clear and enable only this bit in group EN1
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Generate falling edge for selected pin
        int_pend = 1u;
        current_index = i;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(0xA0243FFCu, ~wr_val);

        // Bounded wait for interrupt
        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[NEG] Timeout waiting for IRQ idx=%u\n", i);
#endif
        }

        // Accumulate any ISR errors observed
        if (isr_err_accum) {
            test_err += isr_err_accum;
            isr_err_accum = 0;
        }
    }

    if (test_err == 0u) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }
    return 0;
}
