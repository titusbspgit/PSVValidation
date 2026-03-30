#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>

unsigned int gpio_number, test_err, i;
extern volatile int int_pend; // ensure ISR/store is observed in loop

void test_case() 
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    unsigned int rdata, wr_val;
    test_err = 0;

    // enable sysreg interrupt
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif

#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    for (i = 0; i < 32; i++)
    { 
        // enable posedge interrupt (bit17=1) per pin
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000);
    }

    wait_on(10);
    // Put GPIOs 8-39 in input mode (doe=1)
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);

    wait_on(10);

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); 

    for (i = 0; i < 32; i++)
    { 
        // Prepare known level low, arm wait, then produce exactly one posedge
        write_reg(0xA0243ffc, 0x00000000); // ensure low
        wait_on(10);
        int_pend = 1;                      // arm BEFORE the edge
        write_reg(0xA0243ffc, 0xFFFFFFFF); // rising edge

        // Wait with timeout to avoid infinite hangs
        int timeout = 2000;
        while ((int_pend == 1) && (--timeout > 0)) {
            // printf("Waiting for interrupt\n"); // optional to reduce log spam
            wait_on(10);
        }
        if (timeout == 0) {
            printf("ERROR: Timeout waiting for GPIO IRQ at i=%u\n", i);
            test_err++;
            break;
        }

        // Optionally drive low again to prep for next iteration
        write_reg(0xA0243ffc, 0x00000000);
        wait_on(10);
    }
    finish(test_err);
}

void Default_IRQHandler() 
{
    unsigned int j, rdata, rdata_grp, wr_val;
    wr_val = 1 << i;
    int_pend = 0;

#ifdef DEBUG_DISPLAY
    printf("\nEntered into default IRQ Handler!! with pad value = %d\n", i);
#endif

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); // 88
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); // mask group during service

    if ((rdata_grp & (0xffffffff)) != 0)      
    {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n", i, rdata_grp);
#endif
    }
    else
    {
        printf("ERROR: Group Interrupt not occured\n");
        test_err = test_err + 1;
    }

    // Clear per-pin raw status: write 1 to iclr (bit16)
    for (j = 0; j < 32; j++)
    {  
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000);
    }
    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp == 0x0)
    {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS : Group Interrupt cleared successfully\n");
#endif
    }
    else
    {
        printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n", rdata_grp);
        test_err = test_err + 1;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0) {
        printf("sysreg status not cleared : %0x\n", MIZAR_LSS_SYSREG_RAW_STCR1);
        test_err++;
    }
#endif

#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0) {
        printf("sysreg status not cleared : %0x\n", MIZAR_LSS_SYSREG_RAW_STCR1);
        test_err++;
    }
#endif

    // Re-enable group interrupt output for next iteration
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}