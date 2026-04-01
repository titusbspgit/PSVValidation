#include "test_define.c"

/* Local testcase state */
static volatile int test_err = 0;
static volatile int int_pend = 0;

static inline unsigned get_pin_reg_addr(unsigned idx)
{
    /* idx in [0..31] maps to MIZAR_GPIO_GP0_GPIO_8 + idx*4 */
    return (unsigned)(MIZAR_GPIO_GP0_GPIO_8 + (idx * 4u));
}

/* ISR as per CSV */
void Default_IRQHandler(void)
{
    unsigned rdata_grp;

    /* Signal completion to the poller */
    int_pend = 0;

    /* Read group status and immediately mask group */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        test_err++;
        DEBUG_DISPLAY("[ISR] Group status was zero on entry.\n");
    } else {
        DEBUG_DISPLAY("[ISR] Group status on entry: 0x%08x\n", rdata_grp);
    }

    /* Clear per-pin raw for all pins (GPIO[8..39]) */
    for (unsigned j = 0; j < 32u; ++j) {
        unsigned raddr = get_pin_reg_addr(j);
        write_reg(raddr, 0x00010000u); /* per CSV: clear raw at pin-level */
        wait_on(2);
    }

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000u) {
        test_err++;
        DEBUG_DISPLAY("[ISR] Group status not cleared: 0x%08x\n", rdata_grp);
    }

    /* Clear system raw and GIC */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    /* Best-effort readback: ensure bit cleared if readable */
    {
        unsigned sys_raw = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((sys_raw & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
            test_err++;
            DEBUG_DISPLAY("[ISR] SYS RAW_STCR1 GPIO0 bit not cleared: 0x%08x\n", sys_raw);
        }
    }
    GIC_ClearIRQ(87);
#elif defined(GPIO1)
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    {
        unsigned sys_raw = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((sys_raw & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
            test_err++;
            DEBUG_DISPLAY("[ISR] SYS RAW_STCR1 GPIO1 bit not cleared: 0x%08x\n", sys_raw);
        }
    }
    GIC_ClearIRQ(88);
#else
    /* Domain not specified; suppress undefined behavior. */
#endif

    /* Re-enable group */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

/* Entry function as mandated */
void test_case(void)
{
    DEBUG_DISPLAY("[START] test_gpio_pedge_all_pads_en\n");

    /* GIC and SYSREG enable for the appropriate GPIO domain */
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

    /* Configure per-pin posedge for GPIO[8..39] */
    for (unsigned i = 0; i < 32u; ++i) {
        unsigned addr = get_pin_reg_addr(i);
        write_reg(addr, 0x00020000u);
        wait_on(10);
    }

    /* Put GPIOs 8-39 in input mode via IO CTRL groups 1..4 */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable group interrupt for all pins */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* Drive sequences: for each i, create a rising edge and wait for ISR */
    for (unsigned i = 0; i < 32u; ++i) {
        unsigned timeout = 2000u;

        /* Ensure low, short wait */
        write_reg(0xA0243ffc, 0x00000000u);
        wait_on(10);

        /* Arm and generate rising edge */
        int_pend = 1;
        write_reg(0xA0243ffc, 0xFFFFFFFFu);

        /* Poll with timeout */
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0u) {
            test_err++;
            DEBUG_DISPLAY("[TIMEOUT] Iteration %u: ISR did not clear int_pend in time.\n", i);
        }

        /* Drive low again before next iteration */
        write_reg(0xA0243ffc, 0x00000000u);
        wait_on(10);
    }

    /* Final result */
    if (test_err == 0) {
        DEBUG_DISPLAY("[PASS] test_gpio_pedge_all_pads_en\n");
        finish(0);
    } else {
        DEBUG_DISPLAY("[FAIL] test_gpio_pedge_all_pads_en, errors=%d\n", test_err);
        finish(1);
    }
}
