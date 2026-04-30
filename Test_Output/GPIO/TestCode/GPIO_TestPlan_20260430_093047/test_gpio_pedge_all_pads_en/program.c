// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase: test_gpio_pedge_all_pads_en
 * Description: Enables positive-edge interrupts on all GPIO pads and verifies interrupt assertion,
 *              group status, and clear behavior across all pins.
 */

static volatile int test_err = 0;
static volatile int int_pend = 0; /* released by ISR */

void Default_IRQHandler(void)
{
    /* Group interrupt status must be nonzero */
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u); /* mask during service */
    if ((rdata_grp & 0xffffffffu) == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[PEDGE-ISR] Group STS is zero unexpectedly\n");
#endif
        test_err++;
    }

    /* Clear per-pin raw across all via iclr */
    for (int j = 0; j < 32; j++) {
        unsigned long paddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + ((unsigned long)j * 4u));
        write_reg(paddr, 0x00010000u); /* iclr */
    }
    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
#ifdef DEBUG_DISPLAY
        printf("[PEDGE-ISR] Group STS not cleared sts=0x%08x\n", rdata_grp);
#endif
        test_err++;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    {
        unsigned int rv = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rv & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[PEDGE-ISR] SYS RAW GPIO0 not cleared rv=0x%08x\n", rv);
#endif
            test_err++;
        }
    }
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    {
        unsigned int rv = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rv & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[PEDGE-ISR] SYS RAW GPIO1 not cleared rv=0x%08x\n", rv);
#endif
            test_err++;
        }
    }
    GIC_ClearIRQ(88);
#endif

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xffffffffu); /* re-enable */
    int_pend = 0;
}

int test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    for (int i = 0; i < 32; i++) {
        unsigned long addr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + ((unsigned long)i * 4u));
        write_reg(addr, 0x00020000u); /* peie */
    }
    wait_on(10);

    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xffffffffu);

    for (int i = 0; i < 32; i++) {
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
        int_pend = 1;
        write_reg(0xA0243ffcu, 0xffffffffu); /* rising edge */
        int timeout = 2000;
        while (int_pend == 1 && --timeout > 0) { wait_on(10); }
        if (timeout == 0) {
#ifdef DEBUG_DISPLAY
            printf("[PEDGE] Timeout waiting for IRQ on index %d\n", i);
#endif
            test_err++;
            break;
        }
        write_reg(0xA0243ffcu, 0x00000000u);
        wait_on(10);
    }

    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
