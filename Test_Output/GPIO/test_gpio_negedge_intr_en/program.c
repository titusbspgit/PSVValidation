#include "test_define.c"

/* Local testcase state */
static volatile int test_err = 0;
static volatile int int_pend = 0;
static volatile unsigned current_pin = 0; /* 0..31 corresponds to GPIO[8..39] */

/* Helper: compute per-pin register address for GPIO[8..39] using symbolic base */
static inline unsigned get_pin_reg_addr(unsigned idx)
{
    /* idx in [0..31] maps to MIZAR_GPIO_GP0_GPIO_8 + idx*4 */
    return (unsigned)(MIZAR_GPIO_GP0_GPIO_8 + (idx * 4u));
}

/* ISR as per CSV: Default_IRQHandler runtime flow */
void Default_IRQHandler(void)
{
    unsigned mask = (1u << current_pin);
    unsigned raddr = get_pin_reg_addr(current_pin);
    unsigned rdata;
    unsigned rdata_grp;

    /* Mark interrupt observed */
    int_pend = 0;

    /* Drive pad back high to remove active edge condition */
    write_reg(0xA0243ffc, 0xFFFFFFFFu);

    /* Read per-pin register and perform checks */
    rdata = read_reg(raddr);

    /* Check: bit0 should be 0 */
    if ((rdata & 0x1u) != 0u) {
        test_err++;
        DEBUG_DISPLAY("[ISR] GPIO pin reg bit0 not 0. addr=0x%08x val=0x%08x\n", raddr, rdata);
    }

    /* Check: bit1 should be set (indicates latched interrupt/raw) */
    if ((rdata & 0x2u) == 0u) {
        test_err++;
        DEBUG_DISPLAY("[ISR] GPIO pin reg bit1 not set. addr=0x%08x val=0x%08x\n", raddr, rdata);
    } else {
        /* Verify group status bit set for the current pin */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & mask) == 0u) {
            test_err++;
            DEBUG_DISPLAY("[ISR] Group STS bit not set. sts=0x%08x mask=0x%08x\n", rdata_grp, mask);
        }

        /* Clear per-pin raw: via pin register fields and RAW_STCLR1 */
        write_reg(raddr, (1u<<20) | (1u<<16)); /* per CSV: clear actions */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, mask);

        /* Verify group status cleared */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            test_err++;
            DEBUG_DISPLAY("[ISR] Group STS not cleared. sts=0x%08x\n", rdata_grp);
        }
    }

    /* Clear system raw status and GIC */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#elif defined(GPIO1)
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#else
    /* If GPIO domain not specified, still attempt to clear both in a safe order if headers allow. */
    /* This branch is intentionally silent to avoid undefined macros. */
#endif
}

/* Entry function as mandated */
void test_case(void)
{
    DEBUG_DISPLAY("[START] test_gpio_negedge_intr_en\n");

    /* Initialization: Enable GIC IRQ and System Interrupt for selected GPIO */
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    DEBUG_DISPLAY("[INIT] GPIO0 domain: GIC IRQ 87 enabled, SYSREG EN set.\n");
#elif defined(GPIO1)
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    DEBUG_DISPLAY("[INIT] GPIO1 domain: GIC IRQ 88 enabled, SYSREG EN set.\n");
#else
    DEBUG_DISPLAY("[WARN] Neither GPIO0 nor GPIO1 defined. Proceeding without GIC/SYSREG domain select.\n");
#endif

    /* Initialize drive: set all pads high */
    write_reg(0xA0243ffc, 0xFFFFFFFFu);

    /* Configure per-pin negative-edge: for i=0..31 */
    for (unsigned i = 0; i < 32u; ++i) {
        unsigned addr1 = get_pin_reg_addr(i);
        write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
    }

    /* For each pin enable, generate negedge, and wait for ISR */
    for (unsigned i = 0; i < 32u; ++i) {
        unsigned wr_val = (1u << i);
        unsigned timeout = 5000u;

        current_pin = i;

        /* Clear raw status for this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        /* Enable only this pin interrupt */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        /* Arm and generate falling edge */
        int_pend = 1;
        write_reg(0xA0243ffc, 0xFFFFFFFFu); /* ensure high */
        wait_on(30);
        write_reg(0xA0243ffc, ~wr_val);      /* drive selected bit low */

        /* Poll for ISR completion */
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0u) {
            test_err++;
            DEBUG_DISPLAY("[TIMEOUT] Pin %u: ISR did not clear int_pend in time.\n", i);
        }

        /* Optional small gap between iterations */
        wait_on(10);
    }

    /* Final result */
    if (test_err == 0) {
        DEBUG_DISPLAY("[PASS] test_gpio_negedge_intr_en\n");
        finish(0);
    } else {
        DEBUG_DISPLAY("[FAIL] test_gpio_negedge_intr_en, errors=%d\n", test_err);
        finish(1);
    }
}
