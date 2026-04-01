#include "test_define.c"

/*
 * Test: GPIO negedge interrupt enable/validation for pads 8..39 (GPIO0)
 * High-level procedure (from CSV):
 *  - Enable GIC IRQ and system-level interrupt output for GPIO0.
 *  - Initialize pad driver to known state high (DRIVE_PORT_ADDR).
 *  - Configure each pad (8..39) for input + negedge detect and clear raw.
 *  - For each pad, clear group raw, enable only that bit, arm, generate falling edge,
 *    wait for ISR with bounded timeout, and validate in ISR:
 *      DIN (LSB) == 0, group status shows the bit, per-pin and group raw clear to 0,
 *      and system RAW_STCR1 is cleared. On any failure increment test_err.
 */

/* Local ISR */
void Default_IRQHandler(void)
{
    unsigned int mask;
    unsigned int raddr;
    unsigned int rdata;
    unsigned int rdata_grp;

    if (cur_idx < 0 || cur_idx >= GPIO_PAD_COUNT) {
        /* Spurious or out-of-window interrupt */
        test_err++;
        return;
    }

    mask = (1u << (unsigned)cur_idx);

    /* Mark interrupt observed to unblock poll loop */
    int_pend = 0;

    /* Drive all pads high to complete negedge pulse and avoid re-entries */
    write_reg(DRIVE_PORT_ADDR, 0xFFFFFFFFu);

    /* Per-pin register address for pad (8 + cur_idx) */
    raddr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (cur_idx * 4));

    /* Validate DIN LSB observed low (negedge) */
    rdata = read_reg(raddr);
    if ((rdata & 0x1u) != 0x0u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR] DIN check failed: pad=%u addr=0x%08X val=0x%08X\n", (unsigned)(cur_idx + 8), raddr, rdata);
#endif
    }

    /* Group status must indicate the tested bit */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & mask) == 0u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR] Group status bit not set: sts=0x%08X mask=0x%08X\n", rdata_grp, mask);
#endif
    }

    /* Clear per-pin raw (write iclr + edge raw clear bits) */
    write_reg(raddr, (1u << 20) | (1u << 16));

    /* Clear group raw for this bit */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, mask);

    /* Verify group clear to zero */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
        test_err++;
#ifdef DEBUG_DISPLAY
        printf("[ISR] Group status not cleared: sts=0x%08X expected=0x00000000\n", rdata_grp);
#endif
    }

    /* Clear system-level RAW_STCR1 for GPIO0 and clear GIC IRQ */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
}

void test_case(void)
{
    int i;

#ifdef DEBUG_DISPLAY
    printf("[TEST] Start: test_gpio_negedge_intr_en (pads 8..39)\n");
#endif

    /* Enable GIC and system-level interrupt for GPIO0 */
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);

    /* Initialize external drive port high */
    write_reg(DRIVE_PORT_ADDR, 0xFFFFFFFFu);

    /* Configure pads 8..39: input enable + negedge detect + per-pin raw clear */
    for (i = 0; i < GPIO_PAD_COUNT; ++i) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4));
        if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[CFG] Skipping pad %u (addr=0x%08X)\n", (unsigned)(i + 8), addr);
#endif
            continue;
        }
        write_reg(addr, (1u << 20) | (1u << 18) | (1u << 16));
        wait_on(10);
#ifdef DEBUG_DISPLAY
        printf("[CFG] Pad %u configured (addr=0x%08X)\n", (unsigned)(i + 8), addr);
#endif
    }

    /* Test each pad individually */
    for (i = 0; i < GPIO_PAD_COUNT; ++i) {
        unsigned int mask = (1u << (unsigned)i);
        int timeout;

        if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[RUN] Skipping test for pad %u\n", (unsigned)(i + 8));
#endif
            continue;
        }

        /* Pre-clear group raw bit and enable only this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, mask);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, mask);
        wait_on(10);

        /* Arm and record current index for ISR */
        int_pend = 1;
        cur_idx = i;

        /* Generate a falling edge on this bit */
        write_reg(DRIVE_PORT_ADDR, 0xFFFFFFFFu);
        wait_on(30);
        write_reg(DRIVE_PORT_ADDR, ~mask);

        /* Bounded wait for ISR completion */
        timeout = 5000;
        while (int_pend && (timeout-- > 0)) {
            wait_on(10);
        }
        if (int_pend) {
            test_err++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR on pad %u\n", (unsigned)(i + 8));
#endif
            /* Attempt to restore state for subsequent iterations */
            int_pend = 0;
            write_reg(DRIVE_PORT_ADDR, 0xFFFFFFFFu);
        }

        /* Mask group enable before next bit to avoid re-entries */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    if (test_err == 0) {
        printf("[TEST] PASS: test_gpio_negedge_intr_en\n");
    } else {
        printf("[TEST] FAIL: errors=%d\n", test_err);
    }
#endif

    finish(test_err ? 1 : 0);
}
