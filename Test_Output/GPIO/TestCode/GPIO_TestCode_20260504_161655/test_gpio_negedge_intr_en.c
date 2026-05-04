/*
 * Test: test_gpio_negedge_intr_en
 * Description (verbatim from metadata):
 * Negative-edge interrupt test for GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88 based on GPIO0/GPIO1). Enables system-register interrupt routing via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad controller at 0xA0243ffc to 0xFFFFFFFF. For i=0..31: writes per-pin register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (falling-edge enable), and iclr=1 (clear per-pin raw). For each i: wr_val=(1<<i); clears group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enables only this pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); arms wait (int_pend=1); creates falling edge by writing 0xFFFFFFFF then ~wr_val to 0xA0243ffc; waits with timeout (5000) looping on int_pend with wait_on(10). On timeout, prints error and increments test_err. finish(test_err). Default_IRQHandler: local_wr=(1<<i); sets int_pend=0; writes 0xFFFFFFFF to 0xA0243ffc to return to known state; reads per-pin register raddr=(MIZAR_GPIO_GP0_GPIO_8 + i*4) into rdata; if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) then read group masked status MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp and if ((rdata_grp & local_wr) == 0) test_err++; clear per-pin raw/doe via write_reg(raddr2, (1<<20)|(1<<16)); clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); verify MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else test_err++; clear system-register raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, appropriate bit) and call GIC_ClearIRQ(87/88); else (if raw bit not set) test_err++.
 */

#include <stdint.h>
#include <stdio.h>

/* Expected platform APIs/macros:
 * - void GIC_EnableIRQ(int irq);
 * - void GIC_ClearIRQ(int irq);
 * - void write_reg(uint32_t addr, uint32_t val);
 * - uint32_t read_reg(uint32_t addr);
 * - void wait_on(uint32_t cycles);
 * - void finish(int status);
 * - Macros: MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR, LSS_SYSREG_INTR_EN1_GPIO1_INTR,
 *           MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1,
 *           MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1,
 *           LSS_SYSREG_RAW_STCR1_GPIO0_INTR, LSS_SYSREG_RAW_STCR1_GPIO1_INTR.
 */

#define PAD_CTRL_ADDR 0xA0243ffcu

static volatile uint32_t int_pend = 0;
static volatile uint32_t test_err = 0;
static volatile uint32_t i = 0;

void Default_IRQHandler(void)
{
    uint32_t local_wr = (1u << i);
    int_pend = 0;

    /* Return pads to known state (all high) */
    write_reg(PAD_CTRL_ADDR, 0xFFFFFFFFu);

    uint32_t raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
    uint32_t rdata = read_reg(raddr);

    if ((rdata & 0x1u) != 0u) {
        /* DIN bit0 should be 0 after falling edge */
        ++test_err;
        printf("[test_gpio_negedge_intr_en][ISR] DIN bit0 not 0 for pin %lu\n", (unsigned long)i);
    }

    if ((rdata & 0x2u) != 0x0u) {
        uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err;
            printf("[test_gpio_negedge_intr_en][ISR] Group masked status missing bit for pin %lu (sts=0x%08lx)\n",
                   (unsigned long)i, (unsigned long)rdata_grp);
        }

        uint32_t raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        /* Clear per-pin raw and doe as specified: (1<<20)|(1<<16) */
        write_reg(raddr2, (1u << 20) | (1u << 16));

        /* Clear group raw */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        /* Verify group masked status is 0 */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err;
            printf("[test_gpio_negedge_intr_en][ISR] Group masked status not cleared (0x%08lx)\n",
                   (unsigned long)rdata_grp);
        }

        /* Clear system-register raw and clear GIC IRQ */
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        ++test_err;
        printf("[test_gpio_negedge_intr_en][ISR] Raw bit not set for pin %lu\n", (unsigned long)i);
    }
}

void test_gpio_negedge_intr_en(void)
{
    /* Enable platform IRQ and system-register interrupt routing */
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Drive all pads high initially */
    write_reg(PAD_CTRL_ADDR, 0xFFFFFFFFu);

    /* Configure each GPIO[8..39]: doe=1, neie=1, iclr=1 => (1<<20)|(1<<18)|(1<<16) */
    for (i = 0; i < 32u; ++i) {
        uint32_t addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    /* For each pin, clear raw, enable only this pin, create falling edge, wait for ISR */
    for (i = 0; i < 32u; ++i) {
        uint32_t wr_val = (1u << i);

        /* Clear group raw for this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        /* Enable only this pin in group interrupt enable */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        int_pend = 1;

        /* Ensure high then drive falling edge for bit i */
        write_reg(PAD_CTRL_ADDR, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(PAD_CTRL_ADDR, ~wr_val);

        int timeout = 5000;
        while (int_pend && timeout-- > 0) {
            wait_on(10);
        }
        if (timeout <= 0) {
            printf("[test_gpio_negedge_intr_en] TIMEOUT waiting for pin %lu interrupt\n", (unsigned long)i);
            ++test_err;
        }
    }

    finish((int)test_err);
}
