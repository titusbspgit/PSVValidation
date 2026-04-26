// Author - AI Force 1.3.2. Date 26-04-2026
// (EMBENGG-SYSAPPS)

/* Include only test_define.c as mandated */
#include "test_define.c"

/*
 * Test: test_gpio_negedge_intr_en
 * Description (from metadata):
 * program.c: test_case() sets test_err=0; ifdef GPIO0 GIC_EnableIRQ(87); ifdef GPIO1 GIC_EnableIRQ(88);
 * write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR/…GPIO1_INTR). write_reg(0xA0243ffc, 0xffffffff).
 * For i=0..31: addr1=MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(addr1, (1<<20)|(1<<18)|(1<<16)); wait_on(10).
 * For i=0..31: wr_val=1<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
 * wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val);
 * timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0){printf("ERROR: Timeout ..."); test_err++;}. finish(test_err).
 * ISR Default_IRQHandler(): local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8 + i*4; rdata=read_reg(raddr);
 * if((rdata & 0x1) != 0) test_err++; if((rdata & 0x2) != 0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr) == 0) test_err++;
 * raddr2=MIZAR_GPIO_GP0_GPIO_8 + i*4; write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
 * rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp != 0x0) test_err++;
 * #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif
 * #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif  else { test_err++; }
 */

/* Globals for ISR coordination */
static volatile unsigned int int_pend = 0;
static volatile unsigned int test_err = 0;
static volatile unsigned int g_cur_i = 0; /* tracks current GPIO index for ISR */

/*
 * Function: Default_IRQHandler
 * Purpose : Handle GPIO group interrupt; validate status and clear, per acceptance criteria.
 */
void Default_IRQHandler(void)
{
    unsigned int i = g_cur_i; /* use current index */
    unsigned int local_wr = (1u << (i & 31u));
    int_pend = 0;

    /* Read back pad control and status only if impacted */
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
    if (is_addr_impacted(raddr)) {
        unsigned int rdata = read_reg(raddr);
        if ((rdata & 0x1u) != 0u) {
            ++test_err; /* DATA_IN should be 0 for negedge at this check */
#ifdef DEBUG_DISPLAY
            printf("[ERR] ISR: DATA_IN bit unexpected high at i=%u\n", i);
#endif
        }
    }

    /* Group status check and clear sequence when registers are in impacted set */
    if (is_addr_impacted(MIZAR_GPIO_GP0_INTR1_INTR_STS1)) {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            ++test_err; /* Expected bit not set */
#ifdef DEBUG_DISPLAY
            printf("[ERR] ISR: INTR1_STS1 bit not set for i=%u (sts=0x%08x)\n", i, rdata_grp);
#endif
        }
        /* Re-configure: IO_CTRL and INTR_CLR bits */
        if (is_addr_impacted(raddr)) {
            write_reg(raddr, (1u<<20) | (1u<<16)); /* IO_CTRL(20)=1, INTR_CLR(16)=1 */
        }
        if (is_addr_impacted(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1)) {
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
        }
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
            ++test_err; /* Expected cleared */
#ifdef DEBUG_DISPLAY
            printf("[ERR] ISR: INTR1_STS1 not cleared (0x%08x)\n", rdata_grp);
#endif
        }
    } else {
        ++test_err; /* Required status register not available in impacted set */
#ifdef DEBUG_DISPLAY
        printf("[ERR] ISR: MIZAR_GPIO_GP0_INTR1_INTR_STS1 not in impacted set\n");
#endif
    }

#ifdef GPIO0
    if (is_addr_impacted(MIZAR_LSS_SYSREG_RAW_STCR1)) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    }
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    if (is_addr_impacted(MIZAR_LSS_SYSREG_RAW_STCR1)) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    }
    GIC_ClearIRQ(88);
#endif
}

/*
 * Function: test_case
 * Purpose : Configure GPIO pads for negedge interrupt, enable system interrupt, and verify interrupt handling with timeout.
 */
void test_case(void)
{
    test_err = 0;
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    /* Enable GPIO group interrupt at system level (only if impacted) */
    if (is_addr_impacted(MIZAR_LSS_SYSREG_INTR_EN1)) {
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif
    }

    /* Configure per-GPIO: IO_CTRL(20)=1, NEDGE_INTR_EN(18)=1, INTR_CLR(16)=1 */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        if (is_addr_impacted(addr1)) {
            write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        }
        wait_on(10);
    }

    /* For each pad, enable group interrupt and wait for ISR (no external toggle as raw address is not in impacted set) */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);
        g_cur_i = i;
        if (is_addr_impacted(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1)) {
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        }
        if (is_addr_impacted(MIZAR_GPIO_GP0_INTR1_INTR_EN1)) {
            write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        }
        wait_on(10);

        int_pend = 1;
        /* External pad toggle via 0xA0243ffc is omitted to respect impacted register rule */

        int timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for GPIO%u negedge interrupt\n", (i + 8u));
#endif
            ++test_err;
        }
    }

    if (test_err != 0u) finish(1); else finish(0);
}
