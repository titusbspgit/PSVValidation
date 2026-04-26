// Author - AI Force 1.3.2. Date 26-04-2026
// (EMBENGG-SYSAPPS)

/* Include only test_define.c as mandated */
#include "test_define.c"

/*
 * Test: test_gpio_pedge_all_pads_en
 * Description (from metadata):
 * test_case(): ifdef GPIO0 GIC_EnableIRQ(87); ifdef GPIO1 GIC_EnableIRQ(88);
 * write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR/…GPIO1_INTR).
 * for(i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000) // peie=1. wait_on(10).
 * write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4,0xFF) // doe=1 input. wait_on(10).
 * write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF).
 * For i=0..31: write_reg(0xA0243ffc,0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc,0xFFFFFFFF);
 * timeout loop; on ISR: check STS, clear, verify RAW_STCR1, re-enable EN1, clear GIC.
 */

/* Globals for ISR coordination */
static volatile unsigned int int_pend = 0;
static volatile unsigned int test_err = 0;

/*
 * Function: Default_IRQHandler
 * Purpose : Validate group interrupt status and clear sequence for posedge scenario.
 */
void Default_IRQHandler(void)
{
    int_pend = 0;

    if (is_addr_impacted(MIZAR_GPIO_GP0_INTR1_INTR_STS1)) {
        unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((sts & 0xFFFFFFFFu) == 0u) {
            ++test_err; /* Expected some bit set */
#ifdef DEBUG_DISPLAY
            printf("[ERR] ISR: Group Interrupt not occurred (STS1=0x%08x)\n", sts);
#endif
        }
        /* Clear all per-pin latch by writing INTR_CLR via per-GPIO and clear RAW via sysreg path */
        for (unsigned int j = 0; j < 32u; ++j) {
            unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
            if (is_addr_impacted(raddr)) {
                write_reg(raddr, 0x00010000u); /* INTR_CLR(16)=1 */
            }
        }
        /* Verify group status cleared */
        unsigned int sts2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (sts2 != 0x0u) {
            ++test_err;
#ifdef DEBUG_DISPLAY
            printf("[ERR] ISR: Group Interrupt clear failed (STS1=0x%08x)\n", sts2);
#endif
        }
#ifdef GPIO0
        if (is_addr_impacted(MIZAR_LSS_SYSREG_RAW_STCR1)) {
            write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
            unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
                ++test_err; /* sysreg status not cleared */
#ifdef DEBUG_DISPLAY
                printf("[ERR] ISR: SYSREG RAW_STCR1 GPIO0 bit not cleared (0x%08x)\n", rdata);
#endif
            }
        }
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        if (is_addr_impacted(MIZAR_LSS_SYSREG_RAW_STCR1)) {
            write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
            unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
                ++test_err; /* sysreg status not cleared */
#ifdef DEBUG_DISPLAY
                printf("[ERR] ISR: SYSREG RAW_STCR1 GPIO1 bit not cleared (0x%08x)\n", rdata);
#endif
            }
        }
        GIC_ClearIRQ(88);
#endif
        if (is_addr_impacted(MIZAR_GPIO_GP0_INTR1_INTR_EN1)) {
            write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
        }
    } else {
        ++test_err; /* Required status register not available in impacted set */
#ifdef DEBUG_DISPLAY
        printf("[ERR] ISR: MIZAR_GPIO_GP0_INTR1_INTR_STS1 not in impacted set\n");
#endif
    }
}

/*
 * Function: test_case
 * Purpose : Enable posedge interrupt on all pads and verify IRQ occurrence and clearance with timeouts.
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

    if (is_addr_impacted(MIZAR_LSS_SYSREG_INTR_EN1)) {
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif
    }

    /* Enable posedge interrupt (PEDGE_INTR_EN bit17) for all pads */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        if (is_addr_impacted(addr)) {
            write_reg(addr, 0x00020000u); /* PEDGE_INTR_EN(17)=1 */
        }
    }
    wait_on(10);

    /* Configure IO CTRL group registers to 0xFF if impacted */
    if (is_addr_impacted(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1)) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    if (is_addr_impacted(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2)) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    if (is_addr_impacted(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3)) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    if (is_addr_impacted(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4)) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    if (is_addr_impacted(MIZAR_GPIO_GP0_INTR1_INTR_EN1)) {
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
    }

    /* For each pad, expect interrupt after external toggle (omitted as raw address not in impacted set) */
    for (unsigned int i = 0; i < 32u; ++i) {
        int_pend = 1;
        int timeout = 2000;
        while (int_pend && (--timeout > 0)) {
            wait_on(10);
        }
        if (timeout == 0) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for GPIO IRQ at i=%u\n", i);
#endif
            ++test_err;
            break;
        }
    }

    if (test_err != 0u) finish(1); else finish(0);
}
