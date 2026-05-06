#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>

unsigned int gpio_number, test_err, i;
extern int int_pend;

// Optional: convenience timeout
#define WAIT_INT_TIMEOUT_LOOPS 2000  // adjust as needed

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

    // enabling sysreg interrupt
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif

#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Clean slate: clear any pending raw interrupt latches on GPIO[8..39]
    for (i = 0; i < 32; i++) {
        // Bit16 iclr=1 clears raw status (GPIOarchitecture.txt, Independent GPIO control Register)
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00010000);
    }
    // Also keep pins in input mode; group write does this below.

    // Program posedge enable per-pin (bit17) across GPIOs 8..39
    for (i = 0; i < 32; i++) { 
        // enabling posedge interrupt (17th bit as '1')
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000);
    }

    wait_on(10);

    // For enabling input mode and posedge interrupt for GPIOs 8-39 using group control
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);

    wait_on(10);

    // Enable group interrupts for all 32 lines
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);

    for (i = 0; i < 32; i++) { 
        // 1) Arm software flag BEFORE generating the edge to avoid race
        int_pend = 1;

        // 2) Ensure a known baseline low, then generate exactly one rising edge
        write_reg(0xA0243ffc, 0x00000000);
        wait_on(30);
        write_reg(0xA0243ffc, 0xFFFFFFFF); // rising edge for PE

        // 3) Wait for ISR, but with a timeout to avoid infinite loops
        int to = WAIT_INT_TIMEOUT_LOOPS;
        while (int_pend && to--) {
            // Optional: throttle prints to avoid log flood
            printf("Waiting for interrupt\n");
            wait_on(10);
        }
        if (int_pend) {
            printf("ERROR: Timeout waiting for GPIO interrupt at i=%u\n", i);
            test_err++;
            break; // or continue, depending on test policy
        }
    }

    finish(test_err);
}

void Default_IRQHandler() 
{
    unsigned int rdata, rdata_grp, wr_val;
    unsigned int k;

    wr_val = 1U << i; // informational only
    int_pend = 0;

#ifdef DEBUG_DISPLAY
    printf("\nEntered into default IRQ Handler!! with pad value = %d\n", i);
#endif

    // Latch and (optionally) mask group interrupt during service
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);

    if ((rdata_grp & (0xFFFFFFFF)) != 0) {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS: GPIO_NUM = %0d  status = %0x Default_IRQHandler:: group Interrupt raised\n", i, rdata_grp);
#endif
    } else {
        printf("ERROR: Group Interrupt not occured\n");
        test_err = test_err + 1;
    }

    // Clear raw status for all 32 lines using a local counter (do NOT touch global i)
    for (k = 0; k < 32; k++) {  
        // Keep doe=1 (input), clear iclr=1; 0x00110001 sets bit20 (doe=1) and bit16 (iclr)
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (k * 4), 0x00110001);
    }
    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); // 88
    if (rdata_grp == 0x0) {
#ifdef DEBUG_DISPLAY
        printf("SUCCESS : Group Interrupt cleared successfully\n");
#endif
    } else {
        printf("ERROR : Group Interrupt clear failed: Interrupt value:%x\n", rdata_grp);
        test_err = test_err + 1;
    }

    // Clear sysreg source
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

    // Re-enable GPIO group interrupt for subsequent iterations
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif
}