// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Test: test_gpio_pedge_all_pads_en
 * Description: Convert Meta Test Steps / Procedure to executable C.
 * Notes:
 * - Uses ONLY impacted register macros provided via headers.
 * - Execution sequence preserved strictly.
 */

/* Global state for ISR synchronization */
static volatile int test_err = 0;               /* Accumulated error counter */
static volatile int int_pend = 0;               /* Pending interrupt flag controlled by ISR */
static volatile unsigned int g_isr_index = 0;   /* Current pad index for ISR context */

/* Forward declaration of ISR */
void Default_IRQHandler(void);

/* Helper: compute GPIO pad register address for index i (0..31) */
static inline unsigned int gpio_pad_addr(unsigned int idx)
{
    return (MIZAR_GPIO_GP0_GPIO_8 + (idx * 4U));
}

/* Entry point */
int main(void)
{
    unsigned int i;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_gpio_pedge_all_pads_en: START\n");
#endif

    /* Initialization: test_err=0 (already initialized), optional initial waits */

    /* Conditionally enable GIC and platform interrupt source */
#ifdef GPIO0
    GIC_EnableIRQ(87);
    /* Enable platform interrupt for GPIO0 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enabled GIC IRQ 87 and platform intr for GPIO0\n");
#endif
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    /* Enable platform interrupt for GPIO1 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enabled GIC IRQ 88 and platform intr for GPIO1\n");
#endif
#endif

    /* Configure positive-edge on pads: for (i=0..31) write_reg(GPIO_8 + i*4, 0x00020000). */
    for (i = 0; i < 32U; ++i)
    {
        write_reg(gpio_pad_addr(i), 0x00020000U);
    }
    wait_on(10);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Configured positive-edge detection on GPIO_8..GPIO_39\n");
#endif

    /* Configure input mode groups */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);
    wait_on(10);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Configured IO CTRL GROUP1..4 to input mode\n");
#endif

    /* Enable group interrupt output */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enabled group interrupt output (INTR1_INTR_EN1=0xFFFFFFFF)\n");
#endif

    /* For each pad index i=0..31, generate rising edge via stimulus register and wait for ISR */
    for (i = 0; i < 32U; ++i)
    {
        unsigned int timeout = 2000U;

        /* Drive stimulus low, arm wait flag, then drive high to create rising edge */
        write_reg(0xA0243ffcu, 0x00000000U);
        wait_on(10);
        int_pend = 1;
        g_isr_index = i; /* Provide context for ISR */
        write_reg(0xA0243ffcu, 0xFFFFFFFFU);

        /* Timeout loop waiting for ISR to clear int_pend */
        while ((int_pend == 1) && (timeout-- > 0U))
        {
            wait_on(10);
        }

        if (timeout == 0U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][ERR] Timeout waiting for ISR on GPIO index %u (GPIO_%u)\n", i, (i + 8U));
#endif
            test_err++;
            break; /* break on timeout per procedure */
        }

        /* Optionally drive low again to prepare next iteration */
        write_reg(0xA0243ffcu, 0x00000000U);
        wait_on(10);
    }

    /* Termination based on accumulated errors */
    if (test_err == 0)
    {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] test_gpio_pedge_all_pads_en: PASS\n");
#endif
        finish(0);
    }
    else
    {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] test_gpio_pedge_all_pads_en: FAIL (errors=%d)\n", (int)test_err);
#endif
        finish(1);
    }

    return 0; /* Unreachable due to finish() */
}

/*
 * ISR: Default_IRQHandler
 * - Clears int_pend
 * - Reads group status, masks group during service
 * - Clears per-pin raw statuses by writing 0x00010000 to GPIO_8 + j*4
 * - Verifies group status becomes 0
 * - Clears system RAW_STCR1 bit corresponding to GPIO instance and verifies
 * - Re-enables group and clears GIC IRQ
 */
void Default_IRQHandler(void)
{
    unsigned int rdata_grp;
    unsigned int j;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG][ISR] Enter Default_IRQHandler (idx=%u)\n", g_isr_index);
#endif

    /* Clear pending flag to signal main loop */
    int_pend = 0;

    /* Read group interrupt status and mask group during service */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    if ((rdata_grp & 0xFFFFFFFFU) == 0U)
    {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ISR][ERR] Group status is 0 on ISR entry\n");
#endif
        test_err++;
    }

    /* Clear per-pin raw interrupt status for all pads GPIO_8..GPIO_39 */
    for (j = 0; j < 32U; ++j)
    {
        write_reg(gpio_pad_addr(j), 0x00010000U);
    }
    wait_on(2);

    /* Verify group status becomes 0 */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0U)
    {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][ISR][ERR] Group status not cleared: 0x%08X\n", rdata_grp);
#endif
        test_err++;
    }

    /* Clear system RAW_STCR1 bit for the GPIO interrupt source and verify */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    {
        unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR) != 0U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][ISR][ERR] SYSREG RAW_STCR1 GPIO0 bit not cleared: 0x%08X\n", rdata);
#endif
            test_err++;
        }
    }
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    {
        unsigned int rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
        if ((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR) != 0U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][ISR][ERR] SYSREG RAW_STCR1 GPIO1 bit not cleared: 0x%08X\n", rdata);
#endif
            test_err++;
        }
    }
    GIC_ClearIRQ(88);
#endif

    /* Re-enable group interrupt output */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

#ifdef DEBUG_DISPLAY
    printf("[DEBUG][ISR] Exit Default_IRQHandler\n");
#endif
}
