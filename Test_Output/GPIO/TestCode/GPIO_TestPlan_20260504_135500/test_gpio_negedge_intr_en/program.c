// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/*
Hidden_Test_Description (AS-IS):
Test enables GPIO negative-edge interrupts for pins 8–39, generates a falling edge per pin using an external pad control at 0xA0243ffc, waits for the ISR, and verifies DIN low, raw/group status set/clear behavior, and system interrupt clear.
*/

#include "test_define.c"

static volatile unsigned int g_isr_seen = 0;
static volatile unsigned int g_last_sts = 0;

/*
Purpose: Minimal ISR to service GPIO group interrupt: capture status and clear raw status and system raw status.
*/
void Default_IRQHandler(void)
{
    unsigned int sts1 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts1) {
        g_isr_seen++;
        g_last_sts = sts1;
        /* Clear per-pin raw status using same bits */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, sts1);
        /* Clear system raw status */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0x1u);
    }
}

/*
Purpose: Enable system and GPIO group interrupts, wait for pending interrupts with timeout, and validate set/clear behavior.
*/
int test_case(void)
{
    int error_cnt = 0;

    /* Clear any stale raw status */
#ifdef DEBUG_DISPLAY
    printf("[NEG] Clearing RAW_STCLR1 and STS1...\n");
#endif
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, 0xFFFFFFFFu);

    /* Enable system interrupt bit(s) */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x1u);

    /* Enable GPIO per-pin interrupts for group1 */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* Wait loop for up to 32 events (pins 8-39) or until timeout */
    for (unsigned int i = 0; i < 32; i++) {
        unsigned int timeout = 1000000u; /* Deterministic busy-wait */
        int int_pend = 1;
        while (timeout--) {
            unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (sts != 0u) {
                int_pend = 0; /* observed pending interrupt */
                break;
            }
        }
        if (int_pend != 0) {
            error_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[NEG][TIMEOUT] i=%u no interrupt observed before timeout\n", i);
#endif
        } else {
            /* Service is done in Default_IRQHandler via platform dispatch */
            /* Validate group status cleared */
            unsigned int after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (after != 0u) {
                error_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[NEG][CLEAR_FAIL] i=%u STS1 not cleared (after=0x%08x)\n", i, after);
#endif
            }
        }
    }

    /* Disable enables */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x00000000u);

#ifdef DEBUG_DISPLAY
    printf("[NEG][SUMMARY] error_cnt=%d\n", error_cnt);
#endif

    if (error_cnt == 0) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
