#include <lss_sysreg.h>
#include <stdio.h>
#include <test_common.h>

static int test_err = 0;
static volatile int int_pend = 0;
static volatile unsigned int current_i = 0;

static inline unsigned int rd(unsigned long a){ return read_reg(a); }
static inline void wr(unsigned long a, unsigned int v){ write_reg(a,v); }

void test_case(void)
{
    test_err = 0;

    #ifdef GPIO0
      GIC_EnableIRQ(87);
      wr(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    #endif
    #ifdef GPIO1
      GIC_EnableIRQ(88);
      wr(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
    #endif

    // Per-pin positive-edge interrupt enable
    for (unsigned int i = 0; i < 32; i++) {
        wr(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u), 0x00020000u); // PEIE=1
    }
    wait_on(10);

    // Input mode via group IO control
    wr(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    wr(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    wr(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x00FF00FFu);
    wr(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x00FF00FFu);
    wait_on(10);

    // Enable all pin interrupts in group
    wr(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (unsigned int i = 0; i < 32; i++) {
        // Start low, then rise to generate PE
        wr(0xA0243ffcu, 0x00000000u);
        wait_on(10);
        int_pend = 1;
        current_i = i;
        wr(0xA0243ffcu, 0xFFFFFFFFu);

        int timeout = 2000;
        while (int_pend && --timeout > 0) {
            wait_on(10);
        }
        if (timeout == 0) {
            printf("Timeout waiting for posedge interrupt on bit %u\n", i);
            test_err++;
            break;
        }

        // Return low before next iteration
        wr(0xA0243ffcu, 0x00000000u);
        wait_on(10);
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    unsigned int local_i = current_i;

    // Latch and acknowledge entry
    int_pend = 0;

    // Read masked group status
    unsigned int rdata_grp = rd(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    // Mask group while servicing
    wr(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        printf("Group masked status is zero on ISR entry (unexpected)\n");
        test_err++;
    }

    // Clear per-pin raw across all pins
    for (unsigned int j = 0; j < 32; j++) {
        wr(MIZAR_GPIO_GP0_GPIO_8 + (j * 4u), 0x00010000u); // ICLR=1
    }
    wait_on(2);

    rdata_grp = rd(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        printf("Group masked status not cleared after per-pin raw clear (grp=%#x)\n", rdata_grp);
        test_err++;
    }

    // Clear system RAW and re-enable
    #ifdef GPIO0
      wr(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
      GIC_ClearIRQ(87);
    #endif
    #ifdef GPIO1
      wr(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
      GIC_ClearIRQ(88);
    #endif

    wr(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}
