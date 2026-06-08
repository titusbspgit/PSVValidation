// Author - AI Force 1.3.2. Date 08-06-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* External platform services (provided by test harness). */
extern void write_reg(unsigned long addr, unsigned int val);
extern unsigned int read_reg(unsigned long addr);
extern void wait_on(unsigned int cycles);
extern void finish(int status);
#ifdef DEBUG_DISPLAY
extern int printf(const char *fmt, ...);
#endif

/* ------------------------------------------------------------
 * Function: test_case
 * Description:
 *  Implements Meta Test Steps for test_gpio_pedge_all_pads_en.
 *  Strict sequence:
 *    - Clear pending interrupts
 *    - Configure positive-edge detection and enable interrupts
 *    - Set GPIO direction to output and drive low
 *    - For each pad bit: generate rising edge, validate raw status, clear, revalidate
 *    - Optionally check group/aggregate status at the end
 *  PASS/FAIL is decided strictly per Acceptance Criteria.
 * ------------------------------------------------------------ */
int test_case(void) {
    int error_count = 0;                 /* Cumulative error counter */
    unsigned int i;                      /* Pad bit index */
    const unsigned int kPadBits = 32u;   /* Operate over 32 bits (register width) */

    /* Initialize: clear any pending interrupts */
#ifdef DEBUG_DISPLAY
    printf("[INIT] Clearing pending pad interrupts via INT_STAT_CLR = 0xFFFFFFFF\n");
#endif
    write_reg(REG_INT_STAT_CLR[0], 0xFFFFFFFFu);

    /* Optional pre-check of group status (no criteria here, just debug) */
#ifdef DEBUG_DISPLAY
    {
        unsigned int grp = read_reg(REG_GROUP_INT_STATUS[0]);
        printf("[INIT] GROUP_INT_STATUS (pre) = 0x%08X\n", grp);
    }
#endif

    /* Configure detection: enable POS edge and global interrupt enables */
#ifdef DEBUG_DISPLAY
    printf("[CFG] Enabling POS_EDGE_EN and INT_EN (all bits)\n");
#endif
    write_reg(REG_POS_EDGE_EN[0], 0xFFFFFFFFu);
    write_reg(REG_INT_EN[0],      0xFFFFFFFFu);

    /* If group/aggregate enable exists, enable all */
#ifdef DEBUG_DISPLAY
    printf("[CFG] Enabling GROUP_INT_ENABLE (all bits)\n");
#endif
    write_reg(REG_GROUP_INT_ENABLE[0], 0xFFFFFFFFu);

    /* I/O control: set direction to output and drive 0 prior to edge generation */
#ifdef DEBUG_DISPLAY
    printf("[IO] Setting DIR to outputs and driving DATA_OUT = 0x00000000\n");
#endif
    write_reg(REG_DIR[0], 0xFFFFFFFFu);
    write_reg(REG_DATA_OUT[0], 0x00000000u);

    /* NOTE: CPU/GIC interrupt enabling is platform-controlled. Not modified here
     * to respect the constraint of using only impacted registers.
     */

    /* Per-pad rising-edge generation and validation */
    for (i = 0u; i < kPadBits; ++i) {
        unsigned int mask = (1u << i);

#ifdef DEBUG_DISPLAY
        printf("[PAD %u] Generate rising edge: DATA_OUT <- 0x%08X\n", i, mask);
#endif
        /* Drive the specific pad from 0 to 1 to create a rising edge */
        write_reg(REG_DATA_OUT[0], 0x00000000u);  /* ensure low */
        wait_on(5u);
        write_reg(REG_DATA_OUT[0], mask);         /* rising edge */
        wait_on(10u);                              /* allow status to latch */

        /* Validate raw status bit sets */
        {
            unsigned int raw = read_reg(REG_INT_RAW_STAT[0]);
            if ((raw & mask) == 0u) {
#ifdef DEBUG_DISPLAY
                printf("[ERR ] PAD %u: INT_RAW_STAT bit not set (raw=0x%08X)\n", i, raw);
#endif
                error_count++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("[ OK ] PAD %u: INT_RAW_STAT bit asserted (raw=0x%08X)\n", i, raw);
#endif
            }
        }

        /* Clear the specific interrupt and re-validate deassertion */
#ifdef DEBUG_DISPLAY
        printf("[PAD %u] Clearing interrupt via INT_STAT_CLR <- 0x%08X\n", i, mask);
#endif
        write_reg(REG_INT_STAT_CLR[0], mask);
        wait_on(5u);

        {
            unsigned int raw2 = read_reg(REG_INT_RAW_STAT[0]);
            if ((raw2 & mask) != 0u) {
#ifdef DEBUG_DISPLAY
                printf("[ERR ] PAD %u: INT_RAW_STAT did not clear (raw=0x%08X)\n", i, raw2);
#endif
                error_count++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("[ OK ] PAD %u: INT_RAW_STAT cleared (raw=0x%08X)\n", i, raw2);
#endif
            }
        }

        /* Restore output low for next iteration */
        write_reg(REG_DATA_OUT[0], 0x00000000u);
        wait_on(2u);
    }

    /* Optional group status check should be de-asserted after all clears */
    {
        unsigned int grp_end = read_reg(REG_GROUP_INT_STATUS[0]);
        if (grp_end != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[WARN] GROUP_INT_STATUS not de-asserted at end: 0x%08X\n", grp_end);
#endif
            /* Treat as error per optional group criteria */
            error_count++;
        } else {
#ifdef DEBUG_DISPLAY
            printf("[ OK ] GROUP_INT_STATUS de-asserted at end.\n");
#endif
        }
    }

    /* Terminate with finish(0/1) per acceptance criteria */
    if (error_count == 0) {
#ifdef DEBUG_DISPLAY
        printf("[PASS] test_gpio_pedge_all_pads_en\n");
#endif
        finish(0);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[FAIL] test_gpio_pedge_all_pads_en: errors=%d\n", error_count);
#endif
        finish(1);
    }

    return 0; /* Unreachable in harness if finish() terminates */
}
