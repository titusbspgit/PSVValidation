#include "test_define.c"

/*
Hidden_Test_Description (verbatim):
program.c enables GIC (87/88) and routes the interrupt via MIZAR_LSS_SYSREG_INTR_EN1. It writes MIZAR_GPIO_GP0_GPIO_8+(i*4) = 0x00020000 (peie) for i=0..31, sets MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4 = 0x000000FF (input), enables MIZAR_GPIO_GP0_INTR1_INTR_EN1 = 0xFFFFFFFF, then loops i=0..31: write 0xA0243ffc=0x0, wait, int_pend=1, write 0xA0243ffc=0xFFFFFFFF; poll int_pend with timeout; drive low again. Default_IRQHandler(): wr_val=(1<<i); int_pend=0; rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) success else error; clear per-pin raw by writing 0x00010000 to each MIZAR_GPIO_GP0_GPIO_8+(j*4); verify group status cleared; clear MIZAR_LSS_SYSREG_RAW_STCR1 (GPIO0/1) and confirm readback bit clears; re-enable MIZAR_GPIO_GP0_INTR1_INTR_EN1; clear GIC.
*/

static volatile int int_pend = 0;
static volatile int test_err = 0;
static volatile unsigned int i;

void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    for (i = 0; i < 32; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000); // PEIE per pin
    }
    wait_on(10);

    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); // input mode group1
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); // input mode group2
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); // input mode group3
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); // input mode group4
    wait_on(10);

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); // enable group interrupts

    for (i = 0; i < 32; i++) {
        write_reg(PAD_STIM_TOGGLE_ADDR, 0x00000000);
        wait_on(10);
        int_pend = 1;
        write_reg(PAD_STIM_TOGGLE_ADDR, 0xFFFFFFFF);
        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout == 0) {
            printf("Timeout waiting for ISR on pin %u\n", i);
            test_err++;
            break;
        }
        write_reg(PAD_STIM_TOGGLE_ADDR, 0x00000000);
        wait_on(10);
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    unsigned int rdata_grp;
    unsigned int j;
    unsigned int wr_val = (1u << i);

    (void)wr_val; // value noted per steps; not used further in validation

    int_pend = 0;

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);
    if ((rdata_grp & (0xffffffffU)) != 0) {
        // success
    } else {
        printf("ISR: Group status zero\n");
        test_err++;
    }

    for (j = 0; j < 32; j++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000); // clear per-pin raw
        wait_on(2);
    }

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp == 0x0) {
        // success
    } else {
        printf("ISR: Group status not cleared (0x%08x)\n", rdata_grp);
        test_err++;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    {
        unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) {
            printf("RAW_STCR1 GPIO0 bit not cleared\n");
            test_err++;
        }
    }
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    {
        unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) {
            printf("RAW_STCR1 GPIO1 bit not cleared\n");
            test_err++;
        }
    }
#endif

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);
#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}
