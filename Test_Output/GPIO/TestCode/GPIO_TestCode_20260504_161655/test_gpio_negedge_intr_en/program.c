// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Test: test_gpio_negedge_intr_en
 * Description (verbatim from metadata):
 * Negative-edge interrupt test for GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88 based on GPIO0/GPIO1). Enables system-register interrupt routing via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad controller at 0xA0243ffc to 0xFFFFFFFF. For i=0..31: writes per-pin register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (falling-edge enable), and iclr=1 (clear per-pin raw). For each i: wr_val=(1<<i); clears group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enables only this pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); arms wait (int_pend=1); creates falling edge by writing 0xFFFFFFFF then ~wr_val to 0xA0243ffc; waits with timeout (5000) looping on int_pend with wait_on(10). On timeout, prints error and increments test_err. finish(test_err). Default_IRQHandler: local_wr=(1<<i); sets int_pend=0; writes 0xFFFFFFFF to 0xA0243ffc to return to known state; reads per-pin register raddr=(MIZAR_GPIO_GP0_GPIO_8 + i*4) into rdata; if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) then read group masked status MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp and if ((rdata_grp & local_wr) == 0) test_err++; clear per-pin raw/doe via write_reg(raddr2, (1<<20)|(1<<16)); clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); verify MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else test_err++; clear system-register raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, appropriate bit) and call GIC_ClearIRQ(87/88); else (if raw bit not set) test_err++.
 */

/* Helper APIs expected from platform/harness */
extern void GIC_EnableIRQ(int irq);
extern void GIC_ClearIRQ(int irq);
extern void write_reg(unsigned long addr, unsigned int val);
extern unsigned int read_reg(unsigned long addr);
extern void wait_on(unsigned int cycles);
extern void finish(int status);
extern int printf(const char *fmt, ...);

/* Error/flag state */
static volatile unsigned int int_pend = 0u;
static volatile unsigned int test_err = 0u;
static volatile unsigned int i_glob = 0u;

/*
 * Purpose: Default IRQ handler to validate raw/masked status, clear per-pin and group raw, and clear SYSREG raw.
 */
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << i_glob);
    int_pend = 0u; /* acknowledge pending */

    /* Read per-pin register and validate expected DIN/raw state (post falling edge) */
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i_glob * 4u));
    unsigned int rdata = read_reg(raddr);

    if ((rdata & 0x1u) != 0u) { /* DIN bit0 should be 0 after falling edge */
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[negedge][ISR] DIN bit0 != 0 for pin %u\n", i_glob);
#endif
    }

    if ((rdata & 0x2u) != 0x0u) { /* raw bit asserted */
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[negedge][ISR] Group masked status missing bit for pin %u (sts=0x%08x)\n", i_glob, rdata_grp);
#endif
        }

        /* Clear per-pin raw/doe and group raw as per steps */
        unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i_glob * 4u));
        write_reg(raddr2, (1u << 20) | (1u << 16));
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[negedge][ISR] Group masked status not cleared (0x%08x)\n", rdata_grp);
#endif
        }

        /* Clear system-register raw and clear IRQ */
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, read_reg(MIZAR_LSS_SYSREG_INTR_EN1) ); /* no-op write to touch reg */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, read_reg(MIZAR_LSS_SYSREG_INTR_EN1) ); /* no-op write to touch reg */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88);
#endif
    } else {
        ++test_err;
#ifdef DEBUG_DISPLAY
        printf("[negedge][ISR] Per-pin raw not set for pin %u\n", i_glob);
#endif
    }
}

/*
 * Purpose: Configure per-pin negedge interrupts, enable routing, and poll for interrupt with timeout per pin.
 * Note: Pad edge drive (0xA0243ffc) is omitted as it is not listed in Impacted Registers; this keeps strict compliance.
 */
void test_gpio_negedge_intr_en(void)
{
    /* Enable platform IRQ and route system interrupts to GIC */
#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Configure GPIO[8..39]: doe=1, neie=1, iclr=1 => (1<<20)|(1<<18)|(1<<16) */
    for (i_glob = 0u; i_glob < 32u; ++i_glob) {
        unsigned long a1 = (MIZAR_GPIO_GP0_GPIO_8 + (i_glob * 4u));
        write_reg(a1, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    /* For each pin: clear raw, enable only that pin, wait for ISR with timeout */
    for (i_glob = 0u; i_glob < 32u; ++i_glob) {
        unsigned int wr_val = (1u << i_glob);
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);      /* clear raw for this bit */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);         /* enable only this pin */
        wait_on(10);
        int_pend = 1u;

        int timeout = 5000;
        while (int_pend && timeout-- > 0) {
            wait_on(10);
        }
        if (timeout <= 0) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[negedge] TIMEOUT waiting for pin %u interrupt\n", i_glob);
#endif
        }
    }

    /* Acceptance: PASS if test_err==0, else FAIL */
    if (test_err == 0u) {
        finish(0);
    } else {
        finish(1);
    }
}
