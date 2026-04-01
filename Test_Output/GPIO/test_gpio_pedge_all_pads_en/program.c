#include "test_define.c"

// Test_Stage3 Agent: test_gpio_pedge_all_pads_en
// High-level description (from CSV):
// Enable posedge interrupts on all GPIO[8..39] pins, drive a rising edge per iteration,
// wait for ISR to acknowledge and clear, verify group status behavior, clear per-pin raw and
// system status, and re-enable for next iteration.

// External hooks expected from the environment
extern void wait_on(unsigned int cycles);
extern unsigned int read_reg(unsigned int addr);
extern void write_reg(unsigned int addr, unsigned int data);
extern void finish(int status);
extern void GIC_EnableIRQ(unsigned int id);
extern void GIC_ClearIRQ(unsigned int id);

// Synchronization flag set/cleared around ISR
extern volatile int int_pend; // expected to be provided by platform

#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY printf
#endif

// TODO: GPIO selection control. Define one of GPIO0 or GPIO1 via build flags if needed.
// #define GPIO0 1
// #define GPIO1 1

// TODO: These symbols are expected to be provided by included headers.
// Using them directly per instructions without redefining.
// MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR
// MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR
// MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1
// MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4

static unsigned int test_err = 0;

static void isr_common_clear(void)
{
    // Mask group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    // Clear per-pin RAW by writing 0x00010000 to each per-pin register (GPIO[8..39])
    for (unsigned int j = 0; j < 32; j++) {
        unsigned int raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(raddr, 0x00010000u);
        wait_on(2);
    }

    // Verify group status becomes 0
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        DEBUG_DISPLAY("[ISR] Group status not cleared: 0x%08X\n", rdata_grp);
        test_err++;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
#elif defined(GPIO1)
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
#else
    // Unknown GPIO selection; flag error but continue
    DEBUG_DISPLAY("[WARN] GPIO0/GPIO1 not defined. System RAW clear skipped.\n");
#endif

    // Re-enable group
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

// Default IRQ handler stub matching CSV behavior
void Default_IRQHandler(void)
{
    int_pend = 0; // acknowledge to main loop

    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & 0xFFFFFFFFu) == 0) {
        DEBUG_DISPLAY("[ISR] No group status set on entry.\n");
        test_err++;
    }

    isr_common_clear();

#ifdef GPIO0
    GIC_ClearIRQ(87u);
#elif defined(GPIO1)
    GIC_ClearIRQ(88u);
#else
    // Warn only
    DEBUG_DISPLAY("[WARN] GPIO0/GPIO1 not defined. GIC clear skipped.\n");
#endif
}

static void configure_pads_and_interrupts(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87u);
#elif defined(GPIO1)
    GIC_EnableIRQ(88u);
#else
    DEBUG_DISPLAY("[WARN] GPIO0/GPIO1 not defined. GIC enable skipped.\n");
#endif

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#elif defined(GPIO1)
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#else
    DEBUG_DISPLAY("[WARN] GPIO0/GPIO1 not defined. SYSREG enable skipped.\n");
#endif

    // Configure per-pin posedge: write 0x00020000 to GPIO[8..39]
    for (unsigned int i = 0; i < 32; i++) {
        unsigned int addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u);
        wait_on(10);
    }

    // Put GPIOs 8-39 in input mode
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    // Enable group interrupt
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

void test_case(void)
{
    DEBUG_DISPLAY("[test_gpio_pedge_all_pads_en] Test start. CNT=%u\n", (unsigned)CNT);

    test_err = 0;

    configure_pads_and_interrupts();

    // For each pin [8..39]: drive low then rising edge and wait for ISR
    for (unsigned int i = 0; i < 32; i++) {
        // Drive all low
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);

        // Arm ISR and generate rising edge for bit i
        int_pend = 1;
        unsigned int wr_val = (1u << i);
        write_reg(0xA0243ffcu, 0xFFFFFFFFu); // ensure high for clean edge
        wait_on(10);
        write_reg(0xA0243ffcu, wr_val); // rising edge on pin i

        // Poll for ISR completion with timeout
        unsigned int timeout = 2000u;
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0u) {
            DEBUG_DISPLAY("[TIMEOUT] pin=%u ISR did not complete.\n", i);
            test_err++;
        }

        // Drive low again before next iteration
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
    }

    if (test_err == 0) {
        DEBUG_DISPLAY("[test_gpio_pedge_all_pads_en] PASS\n");
        finish(0);
    } else {
        DEBUG_DISPLAY("[test_gpio_pedge_all_pads_en] FAIL errors=%u\n", test_err);
        finish(1);
    }
}
