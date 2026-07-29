// Author - AI Force 1.3.2. Date 29-07-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: Default_IRQHandler
// Description:
//   Interrupt Service Routine for GPIO negative-edge interrupt enable test.
//   - Clears pending flag, restores pad drive high.
//   - Verifies per-pin status (bit0 == 0, bit1 != 0).
//   - Verifies group status register has the expected bit set.
//   - Restores per-pin configuration, clears RAW status, ensures group status clears.
//   - Clears SoC-level RAW status and GIC pending IRQ under GPIO0/GPIO1 build flags.
// -----------------------------------------------------------------------------
static volatile unsigned int int_pend = 0;   // Set by test_case(), cleared here
static volatile unsigned int cur_pin  = 0;   // Current GPIO pin index under test (0..19)
static unsigned int g_test_err = 0;          // Global error counter

void Default_IRQHandler(void)
{
    // Local snapshot of the tested bit
    unsigned int local_wr = (1u << cur_pin);

    // Mark interrupt handled
    int_pend = 0;

    // Drive all pads high to remove the edge condition
    write_reg(0xA0243ffc, 0xffffffff);

    // Read per-pin control/status register
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (cur_pin * 4u));
    unsigned int rdata = read_reg(raddr);

#ifdef DEBUG_DISPLAY
    printf("[DBG][ISR] pin=%u raddr=0x%08lx rdata=0x%08x\n", cur_pin, raddr, rdata);
#endif

    // Expect data_in (bit0) low after falling edge; if not, record error
    if ((rdata & 0x1u) != 0x0u) {
        g_test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ERR][ISR] pin%u: data_in bit0 not low. rdata=0x%08x\n", cur_pin, rdata);
#endif
    }

    // Expect interrupt indication present (bit1 != 0)
    if ((rdata & 0x2u) != 0x0u) {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
        printf("[DBG][ISR] grp_sts=0x%08x expect_bit=0x%08x\n", rdata_grp, local_wr);
#endif
        if ((rdata_grp & local_wr) == 0u) {
            g_test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] pin%u: group status missing expected bit. sts=0x%08x\n", cur_pin, rdata_grp);
#endif
        }

        // Restore per-pin configuration (clear edge latch as per sequence)
        unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (cur_pin * 4u));
        write_reg(raddr2, (1u<<20) | (1u<<16));

        // Clear RAW status for this pin and verify group status clears
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            g_test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] pin%u: group status not cleared. sts=0x%08x\n", cur_pin, rdata_grp);
#endif
        }

#ifdef GPIO0
        // Clear SoC-level RAW and GIC pending for GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        // Clear SoC-level RAW and GIC pending for GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        // Interrupt indication not present
        g_test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ERR][ISR] pin%u: per-pin interrupt indication (bit1) not set.\n", cur_pin);
#endif
    }
}

// -----------------------------------------------------------------------------
// Function: test_case
// Description:
//   Entry point. Configures SoC/platform interrupt routing, programs GPIO per-pin
//   controls, then for each pin generates a falling edge by toggling the pad
//   drive register at 0xA0243ffc. Waits for ISR to acknowledge within timeout.
//   Accumulates errors and terminates with PASS/FAIL.
// -----------------------------------------------------------------------------
int test_case(void)
{
    unsigned int i;
    g_test_err = 0u;

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_gpio_negedge_intr_en: start\n");
#endif

#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Drive all pads high initially
    write_reg(0xA0243ffc, 0xffffffff);

    // Configure per-pin control for 20 pins (GPIO_8..GPIO_27)
    for (i = 0u; i < 20u; i++) {
        unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
#ifdef DEBUG_DISPLAY
        printf("[DBG] cfg pin%u addr=0x%08lx\n", i, addr1);
#endif
    }

    // For each pin, enable interrupt, generate falling edge, and wait for ISR
    for (i = 0u; i < 20u; i++) {
        unsigned int wr_val = (1u << i);

        // Clear any pending RAW status and enable interrupt for this pin
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Record current pin and set pending flag for ISR synchronization
        cur_pin = i;
        int_pend = 1u;

        // Ensure pads are high, then drive a single pin low to create negedge
        write_reg(0xA0243ffc, 0xffffffff);
        wait_on(30);
        write_reg(0xA0243ffc, ~wr_val);

#ifdef DEBUG_DISPLAY
        printf("[DBG] trigger pin%u wr_val=0x%08x\n", i, wr_val);
#endif

        // Poll for ISR to clear int_pend with timeout
        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            g_test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR on GPIO_%u\n", (i + 8u));
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DBG] test complete. errors=%u\n", g_test_err);
#endif

    if (g_test_err > 0u) {
        finish(1);
    } else {
        finish(0);
    }

    return 0; // Unreachable, finish() terminates
}
