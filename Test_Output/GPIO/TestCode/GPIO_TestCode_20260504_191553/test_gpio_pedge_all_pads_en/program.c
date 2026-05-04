// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

// High-level description (from META):
// Positive-edge interrupt enable across GPIO[8..39], input-mode configuration via group IO control registers,
// rising-edge generation via external pad driver, verification of group masked status, clearing per-pin raw
// status, and system raw clear checks.

#include "test_define.c"

// Globals for ISR synchronization
static volatile unsigned int int_pend = 0;
static volatile unsigned int current_index = 0;
static volatile unsigned int isr_err_accum = 0;

// Function: Default_IRQHandler
// Purpose: ISR that validates non-zero group status, clears per-pin raw across all, clears system RAW and acks GIC.
void Default_IRQHandler(void)
{
    int_pend = 0; // signal

    // Snapshot group status
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    // Mask group
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        ++isr_err_accum; // Expect at least one bit set
#ifdef DEBUG_DISPLAY
        printf("[ISR-POS] Group status zero on entry\n");
#endif
    }

    // Clear per-pin RAW via per-pin ICLR
    for (unsigned int j = 0; j < 32u; ++j) {
        unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(raddr, 0x00010000u); // iclr=1
    }
    wait_on(2);

    // Verify group status cleared
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        ++isr_err_accum;
#ifdef DEBUG_DISPLAY
        printf("[ISR-POS] Group status not cleared sts=0x%08x\n", rdata_grp);
#endif
    }

    // Clear sysreg raw and re-enable group, ack GIC
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    unsigned int sys_rb = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_rb & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
        ++isr_err_accum;
    }
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    unsigned int sys_rb = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((sys_rb & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
        ++isr_err_accum;
    }
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    GIC_ClearIRQ(88);
#endif
}

// Function: test_case
// Purpose: Enable PEIE per pin, configure input mode via group IO CTRL, and validate rising-edge IRQ behavior.
int test_case(void)
{
    unsigned int test_err = 0u;

    // Enable system interrupt output and GIC line per selected GPIO instance
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    // Enable PEIE per pin
    for (unsigned int i = 0; i < 32u; ++i) {
        if (skip_array[i]) continue;
        unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u); // peie=1
    }
    wait_on(10);

    // Configure input mode via group IO control: set low 8 bits for each group
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    // Enable all group pin interrupts
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Generate rising edges per pin and wait for ISR
    for (unsigned int i = 0; i < 32u; ++i) {
        if (skip_array[i]) continue;
        current_index = i;

        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(10);

        int_pend = 1u;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu); // rising edge on all; group status should reflect

        int timeout = 2000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[POS] Timeout waiting for IRQ idx=%u\n", i);
#endif
            break; // As per META, break on timeout
        }

        // Drive low again before next iteration
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(10);

        // Accumulate ISR errors
        if (isr_err_accum) {
            test_err += isr_err_accum;
            isr_err_accum = 0;
        }
    }

    if (test_err == 0u) {
        finish(0); // PASS
    } else {
        finish(1); // FAIL
    }
    return 0;
}
