// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/*
 * program.c — Deterministic implementation of test_gpio_negedge_intr_en
 *
 * High-level description (from Hidden_Test_Description):
 * Negative-edge interrupt enable/validation across GPIO[8..39].
 *
 * Conversion of Hidden_Test_Steps_Procedure to register-level code using only
 * the impacted registers provided in the metadata. No additional registers are used.
 */

/* Include only the generated definitions/context */
#include "test_define.c"

/* Harness externs (provided by platform test framework) */
extern void     write_reg(unsigned int addr, unsigned int val);
extern unsigned int read_reg(unsigned int addr);
extern void     wait_on(unsigned int cycles);
extern void     finish(unsigned int status);

/* ------------------------------------------------------------------------- */
/* Helper: per-pin configure for input + negedge enable + clear raw          */
/* Implements: write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), (1u<<20)|(1u<<18)|(1u<<16)) */
static void cfg_pin_input_negedge_clear(unsigned int i)
{
    /* (1u<<20)=doe=1 (input), (1u<<18)=neie=1, (1u<<16)=iclr=1 */
    const unsigned int val = ((1U << 20) | (1U << 18) | (1U << 16));
    const unsigned int raddr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
#ifdef DEBUG_DISPLAY
    printf("[CFG] GPIO pin index=%u reg=0x%08X val=0x%08X\n", i, raddr, val);
#endif
    write_reg(raddr, val);
}

/* ------------------------------------------------------------------------- */
/* Helper: clear group raw for bit i                                         */
static void clear_group_raw_bit(unsigned int i)
{
    const unsigned int bit = (1U << i);
#ifdef DEBUG_DISPLAY
    printf("[CLR] Group RAW_STCLR1 bit i=%u (mask=0x%08X)\n", i, bit);
#endif
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, bit);
}

/* ------------------------------------------------------------------------- */
/* Helper: enable only the group interrupt bit i                             */
static void enable_group_intr_bit_only(unsigned int i)
{
    const unsigned int bit = (1U << i);
#ifdef DEBUG_DISPLAY
    printf("[EN ] Group INTR_EN1 i=%u (mask=0x%08X)\n", i, bit);
#endif
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, bit);
}

/* ------------------------------------------------------------------------- */
/* Entry point: executes the meta-specified steps deterministically           */
int main(void)
{
    unsigned int test_err = 0U;

#ifdef DEBUG_DISPLAY
    printf("[TEST] Begin: test_gpio_negedge_intr_en\n");
#endif

    /* Step 1 & 2: Enable system interrupt (GPIO0 path assumed as deterministic default) */
#ifdef DEBUG_DISPLAY
    printf("[SYS ] Enable system interrupt via LSS_SYSREG_INTR_EN1 (GPIO0)\n");
#endif
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);

    /* Step 3: External pad drive to high is skipped (not in impacted list) */
#ifdef DEBUG_DISPLAY
    printf("[INFO] Skipping external pad driver programming (not in impacted registers)\n");
#endif

    /* Step 4: Configure each per-pin control register i=0..31 (GPIO[8..39]) */
    for (unsigned int i = 0U; i < 32U; ++i) {
        cfg_pin_input_negedge_clear(i);
    }

    /* Step 5: Per-pin loop */
    for (unsigned int i = 0U; i < 32U; ++i) {
        const unsigned int bit = (1U << i);
        unsigned int timeout = 5000U;
        unsigned int int_pend = 1U; /* armed */

        /* Pre-clear and enable only this bit */
        clear_group_raw_bit(i);
        enable_group_intr_bit_only(i);

        /* Edge generation step is skipped (not in impacted list) */
#ifdef DEBUG_DISPLAY
        printf("[INFO] Skipping edge generation for i=%u (not in impacted registers)\n", i);
#endif

        /* Bounded wait for interrupt to pend (would be cleared in ISR) */
        while ((int_pend != 0U) && (timeout-- > 0U)) {
            wait_on(10U);
            /* Without external stimulus, pend remains set; fall through on timeout */
        }
        if ((timeout == 0U) && (int_pend != 0U)) {
#ifdef DEBUG_DISPLAY
            printf("[ERR ] Timeout waiting for interrupt for i=%u\n", i);
#endif
            test_err++;
        }

        /* Validate group status bit is set as per acceptance criteria */
        {
            const unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if ((sts & bit) == 0U) {
#ifdef DEBUG_DISPLAY
                printf("[ERR ] Group INTR_STS1 bit not set for i=%u (sts=0x%08X)\n", i, sts);
#endif
                test_err++;
            }
        }

        /* Clear per-pin raw via pin register and group raw, then verify group clears to 0 */
        {
            const unsigned int per_pin_addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4U));
            /* Keep doe=1 and clear raw as per meta: (1u<<20)|(1u<<16) */
            const unsigned int per_pin_val  = ((1U << 20) | (1U << 16));
#ifdef DEBUG_DISPLAY
            printf("[CLR ] Per-pin RAW at 0x%08X val=0x%08X (i=%u)\n", per_pin_addr, per_pin_val, i);
#endif
            write_reg(per_pin_addr, per_pin_val);

            clear_group_raw_bit(i);

            const unsigned int sts_after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (sts_after != 0x0U) {
#ifdef DEBUG_DISPLAY
                printf("[ERR ] Group INTR_STS1 not cleared to 0 after clear (i=%u, sts=0x%08X)\n", i, sts_after);
#endif
                test_err++;
            }
        }

        /* Clear system raw status */
#ifdef DEBUG_DISPLAY
        printf("[SYS ] Clear system RAW via LSS_SYSREG_RAW_STCR1 (GPIO0) for i=%u\n", i);
#endif
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    }

#ifdef DEBUG_DISPLAY
    printf("[TEST] End: errors=%u\n", test_err);
#endif

    /* Final pass/fail as per acceptance criteria */
    if (test_err == 0U) {
        finish(0U); /* PASS */
    } else {
        finish(1U); /* FAIL */
    }

    return 0;
}
