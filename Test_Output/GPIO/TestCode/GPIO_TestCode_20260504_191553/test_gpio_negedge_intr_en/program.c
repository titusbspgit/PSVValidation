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

    // Drive pad bus high initially
    wr(0xA0243ffcu, 0xFFFFFFFFu);

    // Configure pins: input mode (doe=1), negedge intr enable (neie=1), per-pin raw clear set (iclr=1 initially)
    for (unsigned int i = 0; i < 32; i++) {
        unsigned long addr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4u);
        wr(addr, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
    }

    for (unsigned int i = 0; i < 32; i++) {
        unsigned int wr_val = (1u << i);
        // pre-clear group raw and enable only this pin
        wr(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        wr(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        int_pend = 1;
        current_i = i;

        // Generate falling edge on this bit
        wr(0xA0243ffcu, 0xFFFFFFFFu);
        wait_on(30);
        wr(0xA0243ffcu, ~wr_val);

        int timeout = 5000;
        while (int_pend && timeout-- > 0) {
            wait_on(10);
        }
        if (timeout <= 0) {
            printf("Timeout waiting for negedge interrupt on bit %u\n", i);
            test_err++;
        }
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    unsigned int local_i = current_i;
    unsigned int local_wr = (1u << local_i);

    // Latch and acknowledge entry
    int_pend = 0;

    // Return pad to high so DIN observed low was due to falling edge
    wr(0xA0243ffcu, 0xFFFFFFFFu);

    // Per-pin readback
    unsigned long raddr = MIZAR_GPIO_GP0_GPIO_8 + (local_i * 4u);
    unsigned int rdata = rd(raddr);

    // DIN low after negedge
    if ((rdata & 0x1u) != 0u) {
        printf("DIN not low after negedge on bit %u (r=%#x)\n", local_i, rdata);
        test_err++;
    }

    // Check masked status via group register
    if ((rdata & 0x2u) != 0x0u) {
        unsigned int rdata_grp = rd(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            printf("Group masked status not set for bit %u (grp=%#x)\n", local_i, rdata_grp);
            test_err++;
        }
        // Per-pin and group raw clears
        wr(raddr, (1u<<20) | (1u<<16)); // doe=1, iclr=1
        wr(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        rdata_grp = rd(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            printf("Group status not cleared (grp=%#x) after raw clear on bit %u\n", rdata_grp, local_i);
            test_err++;
        }
        // Clear system raw and GIC
        #ifdef GPIO0
          wr(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
          GIC_ClearIRQ(87);
        #endif
        #ifdef GPIO1
          wr(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
          GIC_ClearIRQ(88);
        #endif
    } else {
        printf("Per-pin masked status not set for bit %u (r=%#x)\n", local_i, rdata);
        test_err++;
    }
}
