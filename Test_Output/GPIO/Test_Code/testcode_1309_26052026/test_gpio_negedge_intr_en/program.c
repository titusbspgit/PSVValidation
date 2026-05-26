// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase: test_gpio_negedge_intr_en
 * Description: Configure interrupt routing and per-pad settings; iterate over 32 GPIOs starting from GP0_GPIO_8.
 * Convert Meta Steps to exact C logic without reordering or optimization.
 */

/* Global state for PASS/FAIL tracking and ISR coordination */
static volatile int test_err = 0;           /* Error counter */
static volatile int int_pend = 0;           /* Set to 1 before stimulus; cleared in ISR */
static volatile unsigned int g_cur_i = 0;   /* Current GPIO index (0..31) used by ISR */

/* Forward declarations (provided by platform headers) */
/* extern void wait_on(unsigned int cycles); */
/* extern void write_reg(unsigned long addr, unsigned int val); */
/* extern unsigned int read_reg(unsigned long addr); */
/* extern void GIC_EnableIRQ(unsigned int id); */
/* extern void GIC_ClearIRQ(unsigned int id); */
/* extern void finish(int status); */

/*
 * Default interrupt handler as per Meta Procedure
 */
void Default_IRQHandler(void)
{
    unsigned int local_wr = (1u << g_cur_i);                 /* Bit for current pad */
    unsigned long raddr = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (g_cur_i * 4u));
    unsigned int rdata = 0u;
    unsigned int rdata_grp = 0u;

    int_pend = 0;                                            /* ISR observed; clear pending flag */

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] ISR: Enter. g_cur_i=%u, local_wr=0x%08X\n", g_cur_i, local_wr);
#endif

    /* Drive stimulus high after capture */
    write_reg(0xA0243FFC, 0xFFFFFFFFu);

    /* Read pad register */
    rdata = read_reg(raddr);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] ISR: Pad raddr=0x%08lX rdata=0x%08X\n", raddr, rdata);
#endif

    if ((rdata & 0x1u) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] ISR: ERROR - Pad level bit[0] expected low after negedge.\n");
#endif
        test_err++;
    }

    if ((rdata & 0x2u) != 0x0u) {
        /* Group status should show the interrupt before clear */
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] ISR: Group STS=0x%08X (expect bit set)\n", rdata_grp);
#endif
        if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] ISR: ERROR - Group status bit not set for pad.\n");
#endif
            test_err++;
        }

        /* Re-arm/clear sequence on the pad and RAW STCLR */
        write_reg(raddr, (1u<<20)|(1u<<16));                 /* Re-arm: keep edge cfg + clear as per Meta */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] ISR: Group STS after clear=0x%08X (expect 0x00000000)\n", rdata_grp);
#endif
        if (rdata_grp != 0x0u) {
            test_err++;
        }

        /* System/GIC acknowledges */
#if defined(GPIO0)
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
        GIC_ClearIRQ(87u);
#endif
#if defined(GPIO1)
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
        GIC_ClearIRQ(88u);
#endif
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] ISR: ERROR - Interrupt indicator bit[1] not asserted in pad register.\n");
#endif
        test_err++;
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] ISR: Exit. test_err=%d\n", test_err);
#endif
}

/*
 * Entry point for the testcase
 */
int test_case(void)
{
    unsigned int i = 0u;
    unsigned int wr_val = 0u;
    int timeout = 0;
    unsigned long addr1 = 0u;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Begin test: test_gpio_negedge_intr_en\n");
#endif

    /* Initialization: enable GIC and system interrupt routes as per GPIO instance */
#if defined(GPIO0)
    GIC_EnableIRQ(87u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#if defined(GPIO1)
    GIC_EnableIRQ(88u);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Stimulus initializer */
    write_reg(0xA0243FFC, 0xFFFFFFFFu);

    /* Configure 32 GPIO pads starting from GP0_GPIO_8 */
    for (i = 0u; i < 32u; i++) {
        addr1 = (unsigned long)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16));       /* input/interrupt/falling-edge per Meta */
        wait_on(10u);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Config pad idx=%u addr=0x%08lX val=0x%08X\n", i, addr1, (1u<<20)|(1u<<18)|(1u<<16));
#endif
    }

    /* Main loop: per-pad interrupt enable and negedge stimulus */
    for (i = 0u; i < 32u; i++) {
        g_cur_i = i;                                        /* Share with ISR */
        wr_val = (1u << i);

        /* Clear RAW status and enable interrupt for this pad */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10u);

        /* Prepare and apply falling edge stimulus */
        int_pend = 1;                                       /* Expect ISR to clear this */
        write_reg(0xA0243FFC, 0xFFFFFFFFu);                 /* drive all high */
        wait_on(30u);
        write_reg(0xA0243FFC, ~wr_val);                     /* drive only current bit low */

        /* Wait for ISR or timeout */
        timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10u);
        }
        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (i + 8u));
#endif
            test_err++;
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Test complete. Errors=%d\n", test_err);
#endif

    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }

    /* No alternate termination path allowed; finish() ends the test. */
    return 0;
}
