// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Hidden_Test_Description:
// Enables positive-edge interrupts on all GPIO pads and verifies interrupt assertion, group status, and clear behavior across all pins.

static volatile unsigned int int_pend = 0;
static volatile unsigned int g_idx = 0; // current GPIO index causing interrupt
static volatile unsigned int test_err = 0;

// Function: Default_IRQHandler
// Purpose: Service GPIO interrupt, verify group status, clear raw via per-pin iclr, clear system route, and re-enable.
void Default_IRQHandler(void)
{
    unsigned int wr_val = (1u << g_idx);
    int_pend = 0; // release waiter

    // Read group status
    unsigned int rdata_grp = read_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    // Mask during service
    write_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[IRQ] Group STS zero on entry, idx=%u\n", g_idx);
#endif
    }

    // Clear per-pin raw across all pins (iclr)
    for (unsigned int j = 0; j < 32u; ++j) {
        unsigned int raddr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(raddr, 0x00010000u);
    }
    wait_on(2);

    // Group status must clear to 0
    rdata_grp = read_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[IRQ] Group STS not cleared after iclr, grp=0x%08x\n", rdata_grp);
#endif
    }

    // System route clear for GPIO0 and GPIO1, ensure not set
    write_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1, (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int rv0 = read_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rv0 & (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[IRQ] SYS RAW_STCR1 GPIO0 bit set after clear\n");
#endif
    }
    write_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1, (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int rv1 = read_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rv1 & (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[IRQ] SYS RAW_STCR1 GPIO1 bit set after clear\n");
#endif
    }

    // Re-enable interrupts
    write_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Clear GIC pending
    GIC_ClearIRQ(87);
    GIC_ClearIRQ(88);
}

int main(void)
{
    // Enable IRQs
    GIC_EnableIRQ(87);
    GIC_EnableIRQ(88);

    // Route interrupts via system register
    write_reg((unsigned int)MIZAR_LSS_SYSREG_INTR_EN1, (unsigned int)LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    write_reg((unsigned int)MIZAR_LSS_SYSREG_INTR_EN1, (unsigned int)LSS_SYSREG_INTR_EN1_GPIO1_INTR);

    // Enable positive-edge per pin
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u);
    }
    wait_on(10);

    // Set IO control groups to input mode (0xFF per group)
    write_reg((unsigned int)MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg((unsigned int)MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg((unsigned int)MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg((unsigned int)MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    // Enable all interrupts in group
    write_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (unsigned int i = 0; i < 32u; ++i) {
        // Drive low then rising edge using external pad control
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
        int_pend = 1u; g_idx = i;
        write_reg(0xA0243ffcu, 0xFFFFFFFFu);

        int timeout = 2000;
        while (int_pend && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout == 0) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[MAIN] Timeout on posedge for idx=%u\n", i);
#endif
            break;
        }
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
    }

    if (test_err == 0u) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }

    return 0;
}
