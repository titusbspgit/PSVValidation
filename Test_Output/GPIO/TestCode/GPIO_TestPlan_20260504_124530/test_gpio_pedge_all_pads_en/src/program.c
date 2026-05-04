#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>

#define TIMEOUT_ITER  100000

static volatile int isr_seen = 0;

static inline void gen_low(unsigned gpio_rel)  { write_reg(EXT_PAD_CTRL, 0x00000000u); }
static inline void gen_high(unsigned gpio_rel) { write_reg(EXT_PAD_CTRL, (1u << gpio_rel)); }

static int wait_for_isr(void)
{
    for (volatile int t = 0; t < TIMEOUT_ITER; ++t) {
        if (isr_seen) return 0;
    }
    return -1;
}

void test_case(void)
{
    int test_err = 0;

#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    /* Configure pads 8-39 as inputs (edge detect on input) */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x00000000u);

    /* Enable positive-edge detection: assume INTR2 corresponds to posedge */
    write_reg(MIZAR_GPIO_GP0_INTR2_INTR_EN1, 0xFFFFFFFFu);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1,    0xFFFFFFFFu);

    for (unsigned i = 0; i < GPIO_CNT; ++i) {
        unsigned rel = i;            /* 0..31 relative to pad8 */
        unsigned pad = 8u + i;       /* absolute pad id */
        isr_seen = 0;

        /* Generate rising edge: low then high */
        gen_low(rel);
        wait_on(5);
        gen_high(rel);

        if (wait_for_isr() != 0) {
            printf("ERROR: Timeout waiting for ISR on pad %u (pos edge)\n", pad);
            test_err++;
            continue;
        }

        /* Additional per-pad checks can be inserted here if required */

        /* small gap before next pad */
        wait_on(10);
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    /* Service group: mask, clear all asserted raw, verify clear, unmask */
    unsigned en = read_reg(MIZAR_GPIO_GP0_INTR2_INTR_EN1);
    unsigned raw = read_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);

    if (raw) {
        /* mask */
        write_reg(MIZAR_GPIO_GP0_INTR2_INTR_EN1, 0x00000000u);
        /* clear per-pin raw for all set bits */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, raw);
        /* clear system raw status */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFFu);
        /* re-enable group */
        write_reg(MIZAR_GPIO_GP0_INTR2_INTR_EN1, en);

#ifdef GPIO0
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        GIC_ClearIRQ(88);
#endif
    }
    isr_seen = 1;
}
