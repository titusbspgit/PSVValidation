#include "test_define.c"

/*
  Test: GPIO posedge interrupt on all pads (8..39)
  Summary:
    - Configure per-pin posedge detection for pads 8..39.
    - Configure GPIOs 8..39 as inputs via group IO control.
    - Enable group interrupt for all bits.
    - For each pad, generate a single rising edge using external drive register (0xA0243ffc).
    - Wait for ISR with bounded timeout; in ISR, verify group status non-zero, clear per-pin raw across pads,
      verify group clear, clear system RAW_STCR1 for instance, re-enable group interrupt, and clear GIC IRQ.
    - Test passes if no timeouts/errors occur (finish(0)); otherwise finish(1).

  Notes:
    - Uses only symbolic register names/macros assumed to be provided by included project headers.
    - External drive at 0xA0243ffc is used to generate edges as per test plan.
    - Optional logs under DEBUG_DISPLAY.
*/

static volatile int test_err = 0;
static volatile int int_pend = 0;
static volatile unsigned int g_idx = 0; /* Current pad index [0..31] corresponding to pads 8..39 */

/* Configure posedge detect on pads 8..39 (PEIE=1) */
static void cfg_pedge_all_pads(void)
{
    unsigned int i;
    for (i = 0; i < 32; i++) {
        /* Per-pin control register: MIZAR_GPIO_GP0_GPIO_8 + i*4
           Field: PEIE (posedge interrupt enable) assumed at bit[17] per platform headers.
           Value 0x00020000 used per test description to enable posedge detection.
           TODO: Field positioning is provided by platform headers. */
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U), 0x00020000U);
    }
}

/* Configure group IO control for inputs on pads 8..39 via GROUP1..4 */
static void cfg_group_io_as_inputs(void)
{
    /* Each GROUPx controls a set of 8 pads; 0x000000FF selects input mode per platform definition. */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);
}

void Default_IRQHandler(void)
{
    /* Service group interrupt: validate status, clear per-pin raw, verify clear, clear sysreg, and unmask */
    unsigned int rdata_grp;
    unsigned int j;

    int_pend = 0;

#ifdef DEBUG_DISPLAY
    printf("[ISR] Enter (g_idx=%u)\n", g_idx);
#endif

    /* Read group status */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    /* Mask group interrupt during service to avoid re-entry */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    if (rdata_grp == 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] Group status is zero; expected non-zero on posedge.\n");
#endif
        test_err++;
        /* Attempt to recover: fall-through to final steps */
    } else {
#ifdef DEBUG_DISPLAY
        printf("[ISR] Group status: 0x%08X\n", rdata_grp);
#endif
    }

    /* Clear per-pin raw across pads 8..39 (ICLR=1) */
    for (j = 0; j < 32; j++) {
        /* Field ICLR (interrupt clear) assumed at bit[16] per platform headers.
           Value 0x00010000 used per test description to write-one-to-clear. */
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4U), 0x00010000U);
        wait_on(2);
    }

    /* Verify group clear */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR][ERR] Group status not cleared: 0x%08X\n", rdata_grp);
#endif
        test_err++;
    }

    /* Clear system RAW_STCR1 for the selected GPIO instance(s), if defined */
#ifdef LSS_SYSREG_RAW_STCR1_GPIO0_INTR
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef LSS_SYSREG_RAW_STCR1_GPIO1_INTR
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif

    /* Re-enable group interrupt */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

#ifdef DEBUG_DISPLAY
    printf("[ISR] Exit (errors=%d)\n", test_err);
#endif
}

void test_case(void)
{
    unsigned int i;

    test_err = 0;
    int_pend = 0;
    g_idx = 0;

#ifdef DEBUG_DISPLAY
    printf("[TEST] Start: test_gpio_pedge_all_pads_en\n");
#endif

    /* Enable platform IRQ line(s) for GPIO instance(s), if defined */
#ifdef LSS_SYSREG_INTR_EN1_GPIO0_INTR
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef LSS_SYSREG_INTR_EN1_GPIO1_INTR
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Configure per-pin posedge detection */
    cfg_pedge_all_pads();
    wait_on(10);

    /* Configure GPIOs 8..39 as inputs via group IO control */
    cfg_group_io_as_inputs();
    wait_on(10);

    /* Enable group interrupt for all bits */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* Test each pad: generate a single rising edge and wait for ISR */
    for (i = 0; i < 32; i++) {
        int timeout = 2000;

        /* Drive all low (prepare for rising edge) */
        write_reg(0xA0243ffcU, 0x00000000U);
        wait_on(10);

        /* Arm ISR wait and record index */
        g_idx = i;
        int_pend = 1;

#ifdef DEBUG_DISPLAY
        printf("[TEST] Pad %u: generate rising edge\n", (unsigned)(8U + i));
#endif

        /* Generate rising edge on all pads (environment will assert matching group status) */
        write_reg(0xA0243ffcU, 0xFFFFFFFFU);

        /* Bounded wait for ISR to clear int_pend */
        while (int_pend && --timeout > 0) {
            wait_on(10);
        }

        if (timeout <= 0) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR on pad %u\n", (unsigned)(8U + i));
#endif
            test_err++;
            break; /* Stop on first timeout to avoid long loops */
        }

        /* Return to low for next iteration */
        write_reg(0xA0243ffcU, 0x00000000U);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    if (test_err == 0) {
        printf("[TEST] PASS: test_gpio_pedge_all_pads_en\n");
    } else {
        printf("[TEST] FAIL: errors=%d\n", test_err);
    }
#endif

    finish(test_err ? 1 : 0);
}
