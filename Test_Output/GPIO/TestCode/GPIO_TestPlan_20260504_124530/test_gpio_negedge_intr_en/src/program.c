#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>

#define TIMEOUT_ITER  100000

static volatile int isr_seen = 0;

static inline void gen_rise(unsigned gpio_rel) { write_reg(EXT_PAD_CTRL, (1u << gpio_rel)); }
static inline void gen_fall(unsigned gpio_rel) { write_reg(EXT_PAD_CTRL, 0x00000000u); }

static int wait_for_isr(void)
{
    for (volatile int t = 0; t < TIMEOUT_ITER; ++t) {
        if (isr_seen) return 0; /* success */
    }
    return -1; /* timeout */
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

    /* Configure pads 8-39 as inputs and enable group interrupt path */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x00000000u);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x00000000u);

    /* Unmask platform/system interrupt line if required */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0xFFFFFFFFu);

    /* Enable negative-edge detection: assume INTR1 corresponds to negedge */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (unsigned i = 0; i < GPIO_CNT; ++i) {
        unsigned rel = i;            /* 0..31 relative to pad8 */
        unsigned pad = 8u + i;       /* absolute pad id */
        isr_seen = 0;

        /* Arm by driving pad high then generating a falling edge */
        gen_rise(rel);
        wait_on(5);
        gen_fall(rel);

        if (wait_for_isr() != 0) {
            printf("ERROR: Timeout waiting for ISR on pad %u (neg edge)\n", pad);
            test_err++;
            continue;
        }

        /* Validate DIN low on relevant group */
        unsigned din;
        if (pad < 16) {
            din = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP1);
        } else if (pad < 24) {
            din = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP2);
        } else if (pad < 32) {
            din = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP3);
        } else {
            din = read_reg(MIZAR_GPIO_GPIO_DIN_GROUP4);
        }
        unsigned bit = (pad & 0x7);
        if (din & (1u << bit)) {
            printf("ERROR: DIN not low after negedge on pad %u (din=0x%08X)\n", pad, din);
            test_err++;
        }

        /* Clear raw status if still set (safety) */
        unsigned raw = read_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
        if (raw & (1u << rel)) {
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << rel));
        }

        /* Clear system raw status */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFFu);

        /* small gap before next pad */
        wait_on(10);
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    /* Latch ISR and service GPIO raw/group status */
    unsigned raw = read_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
    if (raw) {
        /* mask group during service */
        unsigned en = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

        /* clear per-pin raw (all that are set) */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, raw);

        /* re-enable group after service */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, en);

        /* clear platform IRQ */
#ifdef GPIO0
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        GIC_ClearIRQ(88);
#endif
    }
    isr_seen = 1;
}
