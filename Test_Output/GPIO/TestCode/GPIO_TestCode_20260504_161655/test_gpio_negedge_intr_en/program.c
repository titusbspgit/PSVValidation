// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
Hidden_Test_Description:
Negative-edge interrupt test for GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88 based on GPIO0/GPIO1). Enables system-register interrupt routing via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad controller at 0xA0243ffc to 0xFFFFFFFF. For i=0..31: writes per-pin register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (falling-edge enable), and iclr=1 (clear per-pin raw). For each i: wr_val=(1<<i); clears group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enables only this pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); arms wait (int_pend=1); creates falling edge by writing 0xFFFFFFFF then ~wr_val to 0xA0243ffc; waits with timeout (5000) looping on int_pend with wait_on(10). On timeout, prints error and increments test_err. finish(test_err). Default_IRQHandler: local_wr=(1<<i); sets int_pend=0; writes 0xFFFFFFFF to 0xA0243ffc to return to known state; reads per-pin register raddr=(MIZAR_GPIO_GP0_GPIO_8 + i*4) into rdata; if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) then read group masked status MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp and if ((rdata_grp & local_wr) == 0) test_err++; clear per-pin raw/doe via write_reg(raddr2, (1<<20)|(1<<16)); clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); verify MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else test_err++; clear system-register raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, appropriate bit) and call GIC_ClearIRQ(87/88); else (if raw bit not set) test_err++.
*/

static volatile uint32_t int_pend = 0U;
static volatile uint32_t test_err = 0U;
static volatile uint32_t g_active_index = 0U; /* current pin index 0..31 */

/* ISR invoked by platform on GPIO interrupt */
void Default_IRQHandler(void)
{
    uint32_t i = g_active_index;
    uint32_t local_wr = (1U << (i & 31U));
    int_pend = 0U; /* Signal main loop */

    /* Return pads to known high state */
    write_reg(MIZAR_PAD_CTRL_GPIO, 0xFFFFFFFFU); /* pad controller */

    /* Read per-pin register to check DIN bit0 after falling edge */
    uint32_t raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4U)); /* per-pin config */
    uint32_t rdata = read_reg(raddr);
    if ((rdata & 0x1U) != 0U) { /* DIN[0] expected 0 */
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][NEG] DIN not low i=%lu rdata=0x%08lx\n", (unsigned long)i, (unsigned long)rdata);
#endif
    }

    /* If RAW bit indicates event (bit1?), validate masked group status contains this bit */
    if ((rdata & 0x2U) != 0x0U) {
        uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][NEG] Group STS missing bit i=%lu sts=0x%08lx\n", (unsigned long)i, (unsigned long)rdata_grp);
#endif
        }
        /* Clear per-pin raw (iclr) and doe */
        uint32_t raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
        write_reg(raddr2, ((1U<<20) | (1U<<16))); /* doe=1, iclr=1 */
        /* Clear group raw */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        /* Verify group masked status cleared */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][NEG] Group STS not cleared sts=0x%08lx\n", (unsigned long)rdata_grp);
#endif
        }
        /* Clear system-register raw and clear IRQ */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, (LSS_SYSREG_RAW_STCR1_GPIO0_INTR | LSS_SYSREG_RAW_STCR1_GPIO1_INTR));
        GIC_ClearIRQ(GPIO0_IRQ_ID);
        GIC_ClearIRQ(GPIO1_IRQ_ID);
    } else {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][NEG] RAW bit not set i=%lu rdata=0x%08lx\n", (unsigned long)i, (unsigned long)rdata);
#endif
    }
}

int main(void)
{
#ifdef DEBUG_DISPLAY
    printf("[START] test_gpio_negedge_intr_en\n");
#endif
    test_err = 0U;

    /* Enable platform IRQs for both GPIO instances (deterministic enabling) */
    GIC_EnableIRQ(GPIO0_IRQ_ID);
    GIC_EnableIRQ(GPIO1_IRQ_ID);

    /* Enable system-register interrupt routing for GPIO0 and GPIO1 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, (LSS_SYSREG_INTR_EN1_GPIO0_INTR | LSS_SYSREG_INTR_EN1_GPIO1_INTR));

    /* Drive all pads high */
    write_reg(MIZAR_PAD_CTRL_GPIO, 0xFFFFFFFFU);

    /* Configure all 32 pins: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16) */
    for (uint32_t i = 0; i < 32U; i++) {
        uint32_t addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
        write_reg(addr1, ((1U<<20) | (1U<<18) | (1U<<16)));
        wait_on(10);
    }

    /* Per-pin trigger and wait */
    for (uint32_t i = 0; i < 32U; i++) {
        uint32_t wr_val = (1U << i);
        g_active_index = i;
        /* Clear group raw for this pin and enable only this pin */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        /* Arm wait */
        int_pend = 1U;

        /* Create falling edge on pad i */
        write_reg(MIZAR_PAD_CTRL_GPIO, 0xFFFFFFFFU);
        wait_on(30);
        write_reg(MIZAR_PAD_CTRL_GPIO, (~wr_val));

        /* Poll for ISR to clear int_pend with timeout */
        uint32_t timeout = 5000U;
        while ((int_pend != 0U) && (timeout-- > 0U)) {
            wait_on(10);
        }
        if (timeout == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT][NEG] i=%lu did not trigger\n", (unsigned long)i);
#endif
        }
    }

    if (test_err == 0U) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] FAIL errors=%lu\n", (unsigned long)test_err);
#endif
        finish(1);
    }
    return 0;
}
