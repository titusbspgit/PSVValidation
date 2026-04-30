// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase: test_gpio_negedge_intr_en
 * Description: Enables negative-edge interrupts per GPIO pin and verifies interrupt assertion, raw/group status, and clear behavior.
 */

/* Globals used by ISR and main test flow */
static volatile int test_err = 0;
static volatile int int_pend = 0;
static volatile int current_i = -1;

/* Forward declaration of ISR symbol expected by platform */
void Default_IRQHandler(void);

/*
 * Function: Default_IRQHandler
 * Purpose: Interrupt service routine as per Hidden_Test_Steps_Procedure.
 */
void Default_IRQHandler(void)
{
    unsigned int local_wr = 0u;
    if (current_i >= 0 && current_i < 32) {
        local_wr = (1u << current_i);
    }
    int_pend = 0; /* clear pending flag to release wait loop */

    /* Return pad high */
    write_reg(0xA0243ffcu, 0xffffffffu); /* external drive back high */

    /* Read back per-pin register for DIN/raw checks */
    unsigned long raddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + ((unsigned long)current_i * 4u));
    unsigned int rdata = read_reg(raddr);

    /* DIN must be 0 after negedge */
    if ((rdata & 0x1u) != 0x0u) {
#ifdef DEBUG_DISPLAY
        printf("[NEG-ISR] DIN not low after negedge, i=%d rdata=0x%08x\n", current_i, rdata);
#endif
        test_err++;
    }

    /* RAW bit (assumed bit1) must be set */
    if ((rdata & 0x2u) != 0x2u) {
#ifdef DEBUG_DISPLAY
        printf("[NEG-ISR] RAW bit not set on pin i=%d rdata=0x%08x\n", current_i, rdata);
#endif
        test_err++;
    } else {
        /* Group status must reflect the bit */
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[NEG-ISR] Group STS missing bit i=%d sts=0x%08x expected_mask=0x%08x\n", current_i, rdata_grp, local_wr);
#endif
            test_err++;
        }

        /* Clear per-pin raw via iclr and group raw via RAW_STCLR1 */
        unsigned long raddr2 = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + ((unsigned long)current_i * 4u));
        write_reg(raddr2, ((1u<<20) | (1u<<16))); /* doe=1, iclr=1 */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        /* Group status must be cleared */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0x0u) {
#ifdef DEBUG_DISPLAY
            printf("[NEG-ISR] Group STS not cleared sts=0x%08x\n", rdata_grp);
#endif
            test_err++;
        }

        /* Clear system route and ensure bit doesn't read back set */
#ifdef GPIO0
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        {
            unsigned int rv = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rv & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
                printf("[NEG-ISR] SYS RAW GPIO0 not cleared rv=0x%08x\n", rv);
#endif
                test_err++;
            }
        }
        GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        {
            unsigned int rv = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
            if ((rv & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0u) {
#ifdef DEBUG_DISPLAY
                printf("[NEG-ISR] SYS RAW GPIO1 not cleared rv=0x%08x\n", rv);
#endif
                test_err++;
            }
        }
        GIC_ClearIRQ(88);
#endif
    }
}

/*
 * Function: test_case
 * Purpose: Implements the end-to-end negative-edge interrupt enable/verify/clear procedure.
 */
int test_case(void)
{
    test_err = 0;

#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Drive all high initially */
    write_reg(0xA0243ffcu, 0xffffffffu);

    /* Phase 1: per-pin configuration doe=1, neie=1, iclr=1 across 32 pins */
    for (int i = 0; i < 32; i++) {
        unsigned long addr1 = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + ((unsigned long)i * 4u));
        write_reg(addr1, ((1u<<20) | (1u<<18) | (1u<<16)));
        wait_on(10);
    }

    /* Phase 2: per-pin interrupt test */
    for (int i = 0; i < 32; i++) {
        unsigned int wr_val = (1u << i);
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); /* pre-clear */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);   /* enable bit i */
        wait_on(10);

        int_pend = 1; current_i = i;

        /* Create falling edge on bit i */
        write_reg(0xA0243ffcu, 0xffffffffu);
        wait_on(30);
        write_reg(0xA0243ffcu, ~wr_val);

        int timeout = 5000;
        while (int_pend && timeout--) { wait_on(10); }
        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[NEG] Timeout waiting for IRQ on bit %d\n", i);
#endif
            test_err++;
        }
    }

    if (test_err == 0) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
    return 0;
}
