// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
/*
 * Program for test_gpio_negedge_intr_en
 * Description (verbatim):
 * Negative-edge interrupt test for GPIO[8..39]. Enables platform IRQ (GIC_EnableIRQ 87 or 88 based on GPIO0/GPIO1). Enables system-register interrupt routing via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad controller at 0xA0243ffc to 0xFFFFFFFF. For i=0..31: writes per-pin register at (MIZAR_GPIO_GP0_GPIO_8 + i*4) with (1<<20)|(1<<18)|(1<<16) to set doe=1 (input), neie=1 (falling-edge enable), and iclr=1 (clear per-pin raw). For each i: wr_val=(1<<i); clears group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); enables only this pin via write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); arms wait (int_pend=1); creates falling edge by writing 0xFFFFFFFF then ~wr_val to 0xA0243ffc; waits with timeout (5000) looping on int_pend with wait_on(10). On timeout, prints error and increments test_err. finish(test_err). Default_IRQHandler: local_wr=(1<<i); sets int_pend=0; writes 0xFFFFFFFF to 0xA0243ffc to return to known state; reads per-pin register raddr=(MIZAR_GPIO_GP0_GPIO_8 + i*4) into rdata; if ((rdata & 0x1) != 0) test_err++; if ((rdata & 0x2) != 0x0) then read group masked status MIZAR_GPIO_GP0_INTR1_INTR_STS1 into rdata_grp and if ((rdata_grp & local_wr) == 0) test_err++; clear per-pin raw/doe via write_reg(raddr2, (1<<20)|(1<<16)); clear group raw via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); verify MIZAR_GPIO_GP0_INTR1_INTR_STS1 == 0x0 else test_err++; clear system-register raw via write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, appropriate bit) and call GIC_ClearIRQ(87/88); else (if raw bit not set) test_err++.
 */

#include "test_define.c"

/* Deterministic, register-only flow per impacted registers */

void program_main(void)
{
    unsigned int error_cnt = 0u;

    /* 1) Enable system-register interrupt routing (instance-select via compile-time ifdefs) */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* 2) Configure each per-pin register: doe=1 (bit20), neie=1 (bit18), iclr=1 (bit16) */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(raddr, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
    }

    /* 3) For each pin, clear group raw, enable only this pin, poll for masked status, then clear */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);          /* clear group raw for this pin */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);            /* enable only this pin */
        wait_on(10);

        /* Timeout-based wait for interrupt masked status */
        int timeout = 5000;
        while (timeout-- > 0) {
            unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if ((sts & wr_val) != 0u) {
                break; /* observed */
            }
            wait_on(10);
        }
        if (timeout <= 0) {
            ++error_cnt; /* timeout -> failure */
#ifdef DEBUG_DISPLAY
            printf("[test_gpio_negedge_intr_en] TIMEOUT waiting pin %u\n", i);
#endif
        }

        /* Per-pin clear: iclr=1 and maintain doe=1 as per procedure */
        {
            unsigned long raddr2 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
            write_reg(raddr2, (1u << 20) | (1u << 16));
        }
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);          /* clear group raw */

        /* Verify masked status cleared */
        {
            unsigned int sts2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (sts2 != 0u) {
                ++error_cnt;
#ifdef DEBUG_DISPLAY
                printf("[test_gpio_negedge_intr_en] STS not cleared (0x%08X) for pin %u\n", sts2, i);
#endif
            }
        }

        /* Clear system-register raw */
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
#endif
    }

    /* Acceptance: test passes only if error_cnt==0 */
    if (error_cnt == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_negedge_intr_en] PASS\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[test_gpio_negedge_intr_en] FAIL err=%u\n", error_cnt);
#endif
        finish(1);
    }
}
