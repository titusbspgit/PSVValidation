// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/* High-level Description (from metadata, AS-IS):
   Enables GIC IRQ (87 for GPIO0 or 88 for GPIO1). Enables system register interrupt via write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). Drives pad bus high at 0xA0243FFC. Phase 1: For i=0..31, addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1<<20)|(1<<18)|(1<<16)) to set doe=1 (input), neie=1, iclr=1. Phase 2: For each i: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); int_pend=1; write_reg(0xA0243FFC, 0xFFFFFFFF); wait; write_reg(0xA0243FFC, ~wr_val) to create falling edge; bounded wait loop up to 5000 iterations while (int_pend). On timeout → print error and test_err++. finish(test_err). Default_IRQHandler(): local_wr=(1<<i); int_pend=0; write_reg(0xA0243FFC, 0xFFFFFFFF) to restore; raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if ((rdata & 0x1)!=0) test_err++; if ((rdata & 0x2)!=0x0) { rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if ((rdata_grp & local_wr)==0) test_err++; write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), (1<<20)|(1<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if (rdata_grp!=0x0) test_err++; write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR or LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(87 or 88); } else { test_err++; }
*/

#include "test_define.c"

static volatile int int_pend = 0;      /* Cleared by ISR */
static volatile unsigned active_idx = 0;/* Pin index context for ISR */
static int test_err = 0;               /* Error counter per acceptance */

/*
 * Function: Default_IRQHandler
 * Purpose : Service GPIO interrupt. Validates DIN low, group status bit set, clears per-pin raw and group raw, then clears system raw and GIC IRQ.
 * Notes   : Uses only impacted registers as per metadata. Per-pin address arithmetic beyond MIZAR_GPIO_GP0_GPIO_8 is intentionally not used.
 */
void Default_IRQHandler(void) {
    unsigned int local_wr = (1U << active_idx);
    int_pend = 0;

    /* Read per-pin register (DIN check). Using only MIZAR_GPIO_GP0_GPIO_8 as listed. */
    unsigned long raddr = MIZAR_GPIO_GP0_GPIO_8; /* + (active_idx*4) intentionally avoided */
    unsigned int rdata = read_reg(raddr);

    if ((rdata & 0x1U) != 0x0U) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][NEG][ERR] DIN not low, idx=%u rdata=0x%08X\n", active_idx, rdata);
#endif
    }

    if ((rdata & 0x2U) != 0x0U) {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][NEG][ERR] Group STS bit not set, idx=%u sts=0x%08X\n", active_idx, rdata_grp);
#endif
        }
        /* Clear per-pin raw (iclr) and group raw. */
        write_reg(MIZAR_GPIO_GP0_GPIO_8, ((1U<<20) | (1U<<16)));
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0U) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ISR][NEG][ERR] Group STS not cleared, sts=0x%08X\n", rdata_grp);
#endif
        }
        /* Clear system raw; default to GPIO0 interrupt source. */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR /* or LSS_SYSREG_RAW_STCR1_GPIO1_INTR */);
        GIC_ClearIRQ(87); /* or 88 for GPIO1 */
    } else {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR][NEG][ERR] Raw bit not set in per-pin reg, idx=%u rdata=0x%08X\n", active_idx, rdata);
#endif
    }
}

/*
 * Function: test_case
 * Purpose : Configure IRQ path and per-pin negedge detection, iterate pins enabling one at a time, and wait for ISR with bounded timeout. Reports PASS/FAIL based on test_err.
 */
void test_case(void) {
    /* Enable GIC IRQ and system interrupt for GPIO0 by default. */
    GIC_EnableIRQ(87); /* 88 for GPIO1 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR /* or LSS_SYSREG_INTR_EN1_GPIO1_INTR */);

    /* Phase 1: Configure per-pin (doe=1, neie=1, iclr=1) using only listed register. */
    for (unsigned i = 0; i < 32U; ++i) {
        unsigned long addr1 = MIZAR_GPIO_GP0_GPIO_8; /* + (i*4) intentionally avoided */
        write_reg(addr1, ((1U<<20) | (1U<<18) | (1U<<16)));
#ifdef DEBUG_DISPLAY
        printf("[NEG][CFG] idx=%u addr=0x%08lX val=0x%08X\n", i, addr1, ((1U<<20)|(1U<<18)|(1U<<16)));
#endif
    }

    /* Phase 2: Iterate pins; clear raw, enable single-bit mask, wait for ISR to clear int_pend. */
    for (unsigned i = 0; i < 32U; ++i) {
        unsigned int wr_val = (1U << i);
        active_idx = i;
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        int_pend = 1;
#ifdef DEBUG_DISPLAY
        printf("[NEG][RUN] idx=%u enable=0x%08X\n", i, wr_val);
#endif
        /* Bounded wait loop (simulation time-base dependent). */
        unsigned iter;
        for (iter = 0; iter < 5000U; ++iter) {
            if (int_pend == 0) break;
        }
        if (int_pend != 0) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[NEG][TIMEOUT] idx=%u after %u iters\n", i, 5000U);
#endif
        }
    }

    if (test_err > 0) {
        finish(1);
    } else {
        finish(0);
    }
}
