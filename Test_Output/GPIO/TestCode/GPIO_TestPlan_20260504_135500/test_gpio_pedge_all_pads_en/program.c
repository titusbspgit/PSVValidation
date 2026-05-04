// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/*
Hidden_Test_Description (AS-IS):
Positive-edge interrupts on pins 8–39 with group status handling; ISR masks group, clears per-pin raw for all pins, verifies group clear, clears system raw status, re-enables group, and clears platform IRQ.
*/

#include "test_define.c"

static volatile unsigned int g_isr_count = 0;

/*
Purpose: ISR to service GPIO group interrupt with masking/clearing as described by the acceptance criteria.
*/
void Default_IRQHandler(void)
{
    /* Mask group interrupts while servicing */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    unsigned int sts1 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts1) {
        /* Clear per-pin raw for all pending pins in the group */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, sts1);

        /* Verify group clear */
        unsigned int after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        (void)after; /* Checked in main flow as needed */

        /* Clear system raw status */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0x1u);

        g_isr_count++;
    }

    /* Re-enable group interrupts after service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

/*
Purpose: Configure positive-edge interrupt scenario and ensure ISR triggers and clears as required.
*/
int test_case(void)
{
    int error_cnt = 0;

    /* Initialize: clear raw status and enable */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, 0xFFFFFFFFu);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x1u);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* Wait for at least one ISR before timeout */
    unsigned int timeout = 2000000u;
    while (timeout-- && g_isr_count == 0u) {
        unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (sts) {
            /* ISR expected to clear this via Default_IRQHandler */
        }
    }

    if (g_isr_count == 0u) {
        error_cnt++;
#ifdef DEBUG_DISPLAY
        printf("[POS][TIMEOUT] No ISR observed before timeout\n");
#endif
    } else {
        /* Verify group status cleared */
        unsigned int after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (after != 0u) {
            error_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[POS][CLEAR_FAIL] STS1 not cleared (after=0x%08x)\n", after);
#endif
        }
    }

    /* Disable enables */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x00000000u);

#ifdef DEBUG_DISPLAY
    printf("[POS][SUMMARY] error_cnt=%d isr_count=%u\n", error_cnt, g_isr_count);
#endif

    if (error_cnt == 0) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
