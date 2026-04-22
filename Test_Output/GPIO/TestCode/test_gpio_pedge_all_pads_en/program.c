/*
Hidden_Test_Description:
program.c void test_case(): #ifdef GPIO0 GIC_EnableIRQ(87); #endif #ifdef GPIO1 GIC_EnableIRQ(88); #endif; write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR); for (i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); wait_on(10); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); ... GROUP2..GROUP4 likewise; wait_on(10); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); for (i=0;i<32;i++): write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); timeout=2000; while ((int_pend==1) && (--timeout>0)) wait_on(10); if (timeout==0) { printf timeout; test_err++; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10); finish(test_err). ISR Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if ((rdata_grp & 0xffffffff) != 0) { /*success log*/ } else { printf error; test_err++; } for (j=0;j<32;j++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j*4), 0x00010000); wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp == 0x0) { /*success*/ } else { printf error; test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) { printf not cleared; test_err++; } #endif #ifdef GPIO1 similar for GPIO1 #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); #ifdef GPIO0 GIC_ClearIRQ(87); #endif #ifdef GPIO1 GIC_ClearIRQ(88); #endif
*/

#include "test_define.c"

static volatile int int_pend = 0;
static volatile unsigned int test_err = 0;

/* ISR prototype (platform-specific binding assumed available in template) */
void Default_IRQHandler(void)
{
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);
    if ((rdata_grp & 0xFFFFFFFFU) == 0U) {
        test_err++;
    }
    for (unsigned int j = 0; j < 32U; j++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4U), 0x00010000U);
    }
    wait_on(2);
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x00000000U) {
        test_err++;
    }
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);
    int_pend = 0; /* signal main loop */
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

    for (unsigned int i = 0; i < 32U; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U), 0x00020000U);
    }
    wait_on(10);

    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);

    wait_on(10);

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    for (unsigned int i = 0; i < 32U; i++) {
        write_reg(0xA0243ffcu, 0x00000000U);
        wait_on(10);
        int_pend = 1;
        write_reg(0xA0243ffcu, 0xFFFFFFFFU);
        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout == 0) {
            test_err++;
            break;
        }
        write_reg(0xA0243ffcu, 0x00000000U);
        wait_on(10);
    }

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif

    if (test_err == 0U) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
