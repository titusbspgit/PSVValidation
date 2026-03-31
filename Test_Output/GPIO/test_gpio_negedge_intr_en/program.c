#include "test_define.c"

extern volatile int int_pend;

void test_case(void)
{
    int test_err = 0;
    int i;
    unsigned int addr1;
    unsigned int wr_val;
    unsigned int timeout;

    DEBUG_DISPLAY("test_gpio_negedge_intr_en: init");

#if defined(GPIO0)
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#elif defined(GPIO1)
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#else
    DEBUG_DISPLAY("GPIO0/GPIO1 not defined");
#endif

    /* Initialize drive */
    write_reg(0xA0243ffc, 0xffffffff);

    /* Configure per-pin regs for GPIO[8..39] */
    for (i = 0; i < 32; i++) {
        addr1 = MIZAR_GPIO_GP0_GPIO_8 + (unsigned int)(i * 4u);
        write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16));
        wait_on(10);
    }

    /* Exercise negative-edge interrupt per pin */
    for (i = 0; i < 32; i++) {
        wr_val = (1u << i);

        /* Clear raw status for this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        /* Enable only this pin interrupt */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        /* Arm and generate negedge */
        int_pend = 1;
        write_reg(0xA0243ffc, 0xffffffff);
        wait_on(30);
        write_reg(0xA0243ffc, ~wr_val);

        /* Poll for ISR completion with timeout */
        timeout = 5000u;
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (int_pend) {
            DEBUG_DISPLAY("Timeout waiting for ISR clear");
            test_err++;
        }

        /* Mask all interrupts before next iteration */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);
    }

    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }
}
