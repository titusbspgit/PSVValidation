// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Hidden_Test_Description:
// Enables negative-edge interrupts per GPIO pin and verifies interrupt assertion, raw/group status, and clear behavior.

// Globals used by ISR and main control flow
static volatile unsigned int int_pend = 0;
static volatile unsigned int g_idx = 0; // current GPIO index under test
static volatile unsigned int test_err = 0;

// Function: Default_IRQHandler
// Purpose: Handle GPIO interrupt, validate raw and group status, perform clear sequences, and release wait.
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << g_idx);
    int_pend = 0; // release waiter

    // Return pad high as per steps
    write_reg(0xA0243ffcU, 0xFFFFFFFFu);

    // Read per-pin register
    unsigned int raddr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (g_idx * 4u));
    unsigned int rdata = read_reg(raddr);

    // DIN must be 0 after negedge (bit[0] check as per steps)
    if ((rdata & 0x1u) != 0u) {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[IRQ] DIN not low after negedge, idx=%u r=0x%08x\n", g_idx, rdata);
#endif
    }

    // Raw bit check at per-pin (bit[1] as per steps)
    if ((rdata & 0x2u) != 0x0u) {
        unsigned int rdata_grp = read_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[IRQ] Group STS missing bit for idx=%u grp=0x%08x\n", g_idx, rdata_grp);
#endif
        }
        // Set doe=1 and iclr=1 (per steps: (1<<20)|(1<<16))
        unsigned int raddr2 = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (g_idx * 4u));
        write_reg(raddr2, ((1u<<20) | (1u<<16)));
        // Group raw clear for this bit
        write_reg((unsigned int)MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        // Re-read group status should be 0
        rdata_grp = read_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[IRQ] Group STS not cleared, grp=0x%08x\n", rdata_grp);
#endif
        }
        // System route raw clear and GIC clear (GPIO0/1)
        write_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1, (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
        write_reg((unsigned int)MIZAR_LSS_SYSREG_RAW_STCR1, (unsigned int)LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
    } else {
        ++test_err; // raw bit not set unexpectedly
#ifdef DEBUG_DISPLAY
        printf("[IRQ] Raw bit not set for idx=%u r=0x%08x\n", g_idx, rdata);
#endif
    }
}

// Entry point
int main(void)
{
    // test_err = 0 (implicit via static init)

    // Enable IRQs for GPIO instances (both as per steps)
    GIC_EnableIRQ(87);
    GIC_EnableIRQ(88);

    // Route interrupts via system register for GPIO0 and GPIO1
    write_reg((unsigned int)MIZAR_LSS_SYSREG_INTR_EN1, (unsigned int)LSS_SYSREG_INTR_EN1_GPIO0_INTR);
    write_reg((unsigned int)MIZAR_LSS_SYSREG_INTR_EN1, (unsigned int)LSS_SYSREG_INTR_EN1_GPIO1_INTR);

    // Drive all high initially
    write_reg(0xA0243ffcu, 0xFFFFFFFFu);

    // Phase 1: Configuration - doe=1, neie=1, iclr=1 for each pin
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int addr1 = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, ((1u<<20) | (1u<<18) | (1u<<16)));
        wait_on(10);
    }

    // Phase 2: Per-pin negative edge generation and wait for ISR
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);
        // Pre-clear group raw for this bit
        write_reg((unsigned int)MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        // Enable interrupt bit i
        write_reg((unsigned int)MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Arm and record index
        int_pend = 1u;
        g_idx = i;

        // Create falling edge: ensure high then drop targeted bit
        write_reg(0xA0243ffcu, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(0xA0243ffcu, (~wr_val));

        // Wait with timeout
        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[MAIN] Timeout waiting IRQ for idx=%u\n", i);
#endif
        }
    }

    if (test_err == 0u) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }

    return 0;
}
