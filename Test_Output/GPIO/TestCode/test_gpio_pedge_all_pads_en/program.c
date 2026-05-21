// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: get_pad_addr
 * Purpose : Compute the register address for GPIO pad index (0..31) relative to GPIO_8 base.
 */
static inline unsigned long int get_pad_addr(unsigned int idx)
{
    /* Each GPIO pad register is 4 bytes apart; index 0 corresponds to GPIO_8 */
    return (GPIO_BASE_PAD_REG + ((unsigned long int)idx * 4ul));
}

/*
 * Function: poll_for_intr_set
 * Purpose : Poll the group interrupt status until the specified bitmask is set or timeout expires.
 * Return  : 0 on success (bit observed set), -1 on timeout.
 */
static int poll_for_intr_set(unsigned int bitmask, unsigned int max_loops)
{
    while (max_loops-- > 0u) {
        unsigned int sts = read_reg(REG_GPIO_INTR_STS1);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] poll: INTR_STS1=0x%08x, mask=0x%08x, loops=%u\n", sts, bitmask, max_loops);
#endif
        if ((sts & bitmask) != 0u) {
            return 0; /* observed */
        }
        wait_on(10);
    }
    return -1; /* timeout */
}

/*
 * Function: test_case
 * Entry   : Framework entry point
 * Flow    :
 *  - Enable system interrupt source (no GIC programming here, only system register per Meta).
 *  - For i in 0..31:
 *      * Configure pad for posedge detection: bits 20,17,16 set.
 *      * Clear any raw status, enable per-pad mask.
 *      * Wait briefly then poll for interrupt status bit to be set.
 *      * Validate pad register bits [0]==1 (level) and [1]==1 (edge/status), and group status reflects the pad.
 *      * Re-configure (bits 20,16), clear raw, verify group status cleared, clear system raw status bit.
 *  - Terminate with finish(0) on PASS, finish(1) on any failure.
 */
int test_case(void)
{
    int test_err = 0;                 /* cumulative error counter */
    const unsigned int PAD_COUNT = 32;/* pads GPIO_8..GPIO_39 */
    const unsigned int TIMEOUT_LOOPS = 5000u;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_gpio_pedge_all_pads_en: START\n");
#endif

    /* Enable system-level interrupt source. Exact bit mapping is platform-defined; write a generic enable. */
    {
        unsigned int sys_en = read_reg(REG_SYSREG_INTR_EN1);
        sys_en |= 0x00000001u; /* enable bit0 generically */
        write_reg(REG_SYSREG_INTR_EN1, sys_en);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] SYSREG_INTR_EN1 set to 0x%08x\n", sys_en);
#endif
    }

    /* Iterate through all 32 pads starting from GPIO_8 */
    for (unsigned int i = 0u; i < PAD_COUNT; ++i) {
        const unsigned long int addr = get_pad_addr(i);
        const unsigned int bitmask = (1u << i);

#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Configure pad index=%u, addr=0x%08lx\n", i, addr);
#endif

        /* Configure pad: enable posedge detect and necessary enables (bits 20,17,16) */
        write_reg(addr, ((1u << 20) | (1u << 17) | (1u << 16)));

        /* Clear any pending raw status for this pad */
        write_reg(REG_GPIO_INTR_RAW_STCLR1, bitmask);

        /* Enable per-pad interrupt mask */
        {
            unsigned int en = read_reg(REG_GPIO_INTR_EN1);
            en |= bitmask;
            write_reg(REG_GPIO_INTR_EN1, en);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] INTR_EN1 updated: 0x%08x (mask 0x%08x)\n", en, bitmask);
#endif
        }

        /* Brief delay before status polling */
        wait_on(10);

        /* Poll for interrupt status bit to be observed set */
        if (poll_for_intr_set(bitmask, TIMEOUT_LOOPS) != 0) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][FAIL] Timeout waiting for INTR_STS1 bit 0x%08x\n", bitmask);
#endif
            test_err++;
            /* Continue sequence to attempt cleanup before next pad */
        }

        /* Read pad register for validation */
        {
            unsigned int rdata = read_reg(addr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] PAD[addr=0x%08lx] rdata=0x%08x\n", addr, rdata);
#endif
            if ((rdata & 0x1u) != 0x1u) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] Pad level bit[0] not set for index %u\n", i);
#endif
                test_err++;
            }
            if ((rdata & 0x2u) != 0x2u) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] Pad edge/status bit[1] not set for index %u\n", i);
#endif
                test_err++;
            }
        }

        /* Group status must indicate this pad */
        {
            unsigned int grp = read_reg(REG_GPIO_INTR_STS1);
            if ((grp & bitmask) == 0u) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] Group INTR_STS1 did not reflect pad index %u (grp=0x%08x)\n", i, grp);
#endif
                test_err++;
            }
        }

        /* Re-program pad (bits 20,16) and clear raw status for this pad */
        write_reg(addr, ((1u << 20) | (1u << 16)));
        write_reg(REG_GPIO_INTR_RAW_STCLR1, bitmask);

        /* Verify group status cleared for this pad */
        {
            unsigned int grp = read_reg(REG_GPIO_INTR_STS1);
            if ((grp & bitmask) != 0u) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][FAIL] Group status not cleared for index %u (grp=0x%08x)\n", i, grp);
#endif
                test_err++;
            }
        }

        /* Clear system raw status (generic clear) */
        write_reg(REG_SYSREG_RAW_STCR1, 0x00000001u);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Cleared system RAW_STCR1 for pad index %u\n", i);
#endif
    }

#ifdef DEBUG_DISPLAY
    if (test_err == 0) {
        printf("[DEBUG] test_gpio_pedge_all_pads_en: PASS\n");
    } else {
        printf("[DEBUG] test_gpio_pedge_all_pads_en: FAIL (errors=%d)\n", test_err);
    }
#endif

    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }

    return 0; /* Unreachable, finish() terminates */
}
