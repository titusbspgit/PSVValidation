#include "test_define.c"

/* Globals for synchronization and error accounting */
static volatile int test_err = 0;
static volatile int int_pend = 0;

/* Optional: Track current pin under test (for debug messages) */
static volatile uint32_t g_current_pin = 0xFFFFFFFFu;

/*
  Default IRQ handler (CSV: ISR behavior summary)
  - Acknowledge GPIO group interrupt
  - Verify per-pin status semantics where possible
  - Clear per-pin RAW status and system RAW status
  - Clear GIC IRQ
  - Signal completion via int_pend = 0
*/
void Default_IRQHandler(void)
{
#if defined(GPIO0)
    const uint32_t gic_id = 87u;
    const uint32_t sys_raw_clr = LSS_SYSREG_RAW_STCR1_GPIO0_INTR;
#elif defined(GPIO1)
    const uint32_t gic_id = 88u;
    const uint32_t sys_raw_clr = LSS_SYSREG_RAW_STCR1_GPIO1_INTR;
#else
    /* Unknown instance selection; cannot service properly */
    DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: Neither GPIO0 nor GPIO1 macro defined.\n");
    test_err++;
    int_pend = 0;
    return;
#endif

    /* Read group status to identify triggering pin(s) */
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp == 0u) {
        DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: Group status is 0 on entry.\n");
        test_err++;
        int_pend = 0;
        GIC_ClearIRQ(gic_id);
        return;
    }

    /* Handle first asserted pin bit (0..31) */
    uint32_t pin = 32u;
    for (uint32_t j = 0; j < 32u; ++j) {
        if (rdata_grp & (1u << j)) { pin = j; break; }
    }

    if (pin < 32u) {
        uint32_t raddr = GPIO_PIN_ADDR(pin);
        uint32_t rdata = read_reg(raddr);

        /* CSV Acceptance checks (best-effort): */
        if ((rdata & 0x1u) != 0u) {
            DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: PIN%u: bit0 expected 0 (input). r=0x%08x\n", pin, rdata);
            test_err++;
        }
        if ((rdata & 0x2u) == 0u) {
            DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: PIN%u: edge status bit not set. r=0x%08x\n", pin, rdata);
            test_err++;
        }

        /* Clear per-pin raw and config-latched status as per CSV write (1<<20)|(1<<16) */
        write_reg(raddr, (1u << 20) | (1u << 16));
        /* Clear group RAW status bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << pin));

        /* Verify group status cleared */
        uint32_t rdata_grp2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp2 != 0u) {
            DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: Group status not cleared. r=0x%08x\n", rdata_grp2);
            test_err++;
        }

        /* Clear system RAW status and GIC */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, sys_raw_clr);
        GIC_ClearIRQ(gic_id);
    } else {
        DEBUG_DISPLAY("[GPIO NEGEDGE ISR] ERROR: No asserted pin found in group status 0x%08x\n", rdata_grp);
        test_err++;
    }

    /* Signal completion to polling loop */
    int_pend = 0;
}

/* Entry point as mandated */
void test_case(void)
{
    DEBUG_DISPLAY("[GPIO NEGEDGE] Test start\n");

#if defined(GPIO0)
    const uint32_t gic_id = 87u;
    const uint32_t sys_en_bit = LSS_SYSREG_INTR_EN1_GPIO0_INTR;
#elif defined(GPIO1)
    const uint32_t gic_id = 88u;
    const uint32_t sys_en_bit = LSS_SYSREG_INTR_EN1_GPIO1_INTR;
#else
    DEBUG_DISPLAY("[GPIO NEGEDGE] ERROR: Neither GPIO0 nor GPIO1 macro defined.\n");
    finish(1);
    return;
#endif

    /* Enable GIC and system-level interrupt for the selected GPIO instance */
    GIC_EnableIRQ(gic_id);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, sys_en_bit);

    /* Initialize drive: set all bits high on the pad-control register */
    write_reg(0xA0243ffcu, 0xFFFFFFFFu);

    /* Configure per-pin negative-edge: value (1<<20)|(1<<18)|(1<<16) for pins 8..39 (32 pins) */
    for (uint32_t i = 0; i < gpio_pin_count; ++i) {
        if (skip_array[i]) continue;
        uint32_t addr = GPIO_PIN_ADDR(i);
        write_reg(addr, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    /* Exercise each pin: clear RAW, enable one pin at a time, generate falling edge, wait for ISR */
    for (uint32_t i = 0; i < gpio_pin_count; ++i) {
        if (skip_array[i]) continue;

        uint32_t wr_val = (1u << i);
        g_current_pin = i;

        /* Clear raw status and enable only this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        /* Arm ISR and generate a negedge on this pin via pad-control toggling */
        int_pend = 1;
        write_reg(0xA0243ffcu, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(0xA0243ffcu, ~wr_val);

        /* Poll for ISR completion with timeout */
        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            DEBUG_DISPLAY("[GPIO NEGEDGE] ERROR: Timeout waiting for ISR on pin %u\n", i);
            test_err++;
        }
    }

    if (test_err == 0) {
        DEBUG_DISPLAY("[GPIO NEGEDGE] PASS\n");
        finish(0);
    } else {
        DEBUG_DISPLAY("[GPIO NEGEDGE] FAIL: errors=%d\n", test_err);
        finish(1);
    }
}
