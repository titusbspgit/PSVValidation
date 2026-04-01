#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"
#include <test_common.h>

unsigned int gpio_number, test_err, rdata, wr_val, i, addr1;
extern int int_pend;

int test_case()
{
    test_err = 0;

#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    // enabling sysreg interrupt
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif

#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Drive all high initially (known state)
    write_reg(0xA0243ffc, 0xffffffff);

    // Phase 1: Configure GPIOs 8..39: input + negedge, and clear any pending raw
    for (i = 0; i < 32; i++) {
        // doe=1 (input), neie=1, iclr=1
        addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
        write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    // Phase 2: For each bit, enable, generate falling edge, and wait with timeout
    for (i = 0; i < 32; i++) {
        wr_val = 1u << i;

        // Pre-clear any latched raw for this bit at group level (belt-and-suspenders)
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        // Enable only this bit
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Arm the wait BEFORE generating the edge to avoid race
        int_pend = 1;

        // Create falling edge on bit i (1 -> 0)
        write_reg(0xA0243ffc, 0xffffffff);
        wait_on(30);
        write_reg(0xA0243ffc, ~wr_val);

        // Bounded wait instead of infinite loop
        unsigned int timeout = 5000; // adjust to your sim time base if needed
        while (int_pend && timeout--) {
            wait_on(10);
        }
        if (timeout == 0) {
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (unsigned)(i + 8));
            test_err++;
            // Optionally continue to next pin
        }
    }

    finish(test_err);
}

void Default_IRQHandler()
{
    unsigned int rdata_grp, raddr, raddr2;
    // Recompute current bit mask safely based on global i
    unsigned int local_wr = 1u << i;

    int_pend = 0;

    // Return pad driver to known state (all high)
    write_reg(0xA0243ffc, 0xffffffff);

    raddr = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
    rdata = read_reg(raddr);

#ifdef DEBUG_DISPLAY
    // printf("Entered into default IRQ Handler!! with pad value = %d", i);
#endif

    // For falling edge, DIN should be 0 after the edge
    if ((rdata & 0x1) != 0) {
        // Wrong DIN value for negedge
        test_err++;
    }

    // Check raw interrupt (irs bit1 should be set)
    if ((rdata & 0x2) != 0x0) {
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); // 88

        // Ensure group bit is set for this pin
        if ((rdata_grp & local_wr) == 0) {
            test_err++;
        }

        // Clear per-pin raw status (iclr=1) while keeping doe=1
        raddr2 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4);
        write_reg(raddr2, (1u << 20) | (1u << 16)); // doe=1, iclr=1

        // Also clear group raw bit (if RAW_STCLR1 is W1C)
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        // Verify group clear
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0) {
            test_err++;
        }

#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif

#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif

    } else {
        // Raw bit not set -> unexpected for negedge
        test_err++;
    }
}
