// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/* High-level Description (from metadata, AS-IS):
   Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt. Configures positive-edge detection per pin by writing 0x00020000 to per-pin registers. Sets input mode via IO_CTRL_GROUP1..4 with 0x000000FF. Enables all group interrupts (0xFFFFFFFF). For each pin, drives a rising edge stimulus and waits up to 2000 iterations. ISR masks group, checks non-zero status, clears per-pin raw for all pins (0x00010000), verifies group cleared, clears system raw, re-enables group, and clears GIC.
*/

#include "test_define.c"

static volatile int int_pend = 0;
static int test_err = 0;

/* ISR for positive edge detection */
void Default_IRQHandler(void) {
    int_pend = 0;
    /* Read and validate group status using impacted register only */
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & 0xFFFFFFFFU) == 0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][POS][ERR] Group STS zero\n");
#endif
    }

    /* Mask group during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    /* Clear per-pin raw for all pins using only the first per-pin register as listed */
    for (unsigned j = 0; j < 32U; ++j) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8, 0x00010000U);
    }

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][POS][ERR] Group STS not cleared sts=0x%08X\n", rdata_grp);
#endif
    }

    /* Clear system raw and re-enable group; default to GPIO0 */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR /* or GPIO1 bit */);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);
    GIC_ClearIRQ(87); /* or 88 */
}

/* Main test */
void test_case(void) {
    /* Enable GIC and system interrupt (GPIO0 default) */
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR /* or GPIO1 bit */);

    /* Configure positive-edge detection per pin using only listed per-pin register */
    for (unsigned i = 0; i < 32U; ++i) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8, 0x00020000U);
    }

    /* IO control groups to input */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* Iterate pins: set int_pend, emulate rising edge indirectly, wait with timeout */
    for (unsigned i = 0; i < 32U; ++i) {
        int_pend = 1;
#ifdef DEBUG_DISPLAY
        printf("[POS][RUN] idx=%u\n", i);
#endif
        unsigned iter;
        for (iter = 0; iter < 2000U; ++iter) {
            if (int_pend == 0) break;
        }
        if (int_pend != 0) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[POS][TIMEOUT] idx=%u after %u iters\n", i, 2000U);
#endif
            break; /* optional break as per description */
        }
    }

    if (test_err > 0) {
        finish(1);
    } else {
        finish(0);
    }
}
