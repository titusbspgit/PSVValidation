// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 High-level Description (from META):
 Enable negedge interrupts on GPIO pins 8..39, generate a single falling edge per pin, wait with timeout for an ISR handshake (int_pend), and validate pin-level DIN, group status, and raw-status clear sequences.
*/

static volatile int int_pend = 0;
static unsigned int test_err = 0;

/*
 * Default_IRQHandler
 * Purpose: Minimal ISR: validate group status, clear per-pin and group raw, assert DIN state for negedge, and clear system RAW.
 */
void Default_IRQHandler(void)
{
    unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] INTR1_STS1=0x%08X\n", sts);
#endif
        for (unsigned int i = 0; i < 32; i++) {
            if (sts & (1u << i)) {
                unsigned long pin_reg = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
                /* Clear per-pin raw (iclr bit) */
                write_reg(pin_reg, (1u << 16));
                /* Clear group raw status for this bit */
                write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));
                /* Validate DIN low for negedge (bit0) */
                unsigned int din = read_reg(pin_reg) & 0x1u;
                if (din != 0u) {
#ifdef DEBUG_DISPLAY
                    printf("[ERR][ISR] DIN not low on negedge at pin %u (reg 0x%08lX)\n", i+8, pin_reg);
#endif
                    test_err++;
                }
            }
        }
        /* Verify group status cleared */
        unsigned int sts_after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (sts_after != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] Group status not cleared: 0x%08X\n", sts_after);
#endif
            test_err++;
        }
        /* Clear system RAW and confirm */
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFFu);
        if (read_reg(MIZAR_LSS_SYSREG_RAW_STCR1) != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR][ISR] System RAW_STCR1 not cleared to 0\n");
#endif
            test_err++;
        }
        int_pend = 0; /* handshake to main loop */
    }
}

/*
 * test_case
 * Purpose: Configure negedge per-pin and group, then per-pin enable and wait for ISR with timeout; finish per META.
 */
void test_case(void)
{
    test_err = 0;

    /* Enable system-level GPIO interrupts (bit usage per platform headers) */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0xFFFFFFFFu);

    /* Configure per-pin: input + negedge enable + clear raw */
    for (unsigned int i = 0; i < 32; i++) {
        unsigned long pin_reg = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        unsigned int cfg = ((1u<<20) | (1u<<18) | (1u<<16)); /* doe=1, neie=1, iclr=1 */
        write_reg(pin_reg, cfg);
        wait_on(10);
    }

    /* Per-pin stimulus sequence (edge generation is platform-specific; here we rely on external drive) */
    for (unsigned int i = 0; i < 32; i++) {
        unsigned int bit = (1u << i);
        /* Clear any lingering raw and enable only this bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, bit);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, bit);
        wait_on(10);

        int_pend = 1;
        /* Wait with timeout for ISR to clear int_pend */
        unsigned int timeout = 5000u;
        while (int_pend && timeout > 0u) {
            timeout--;
        }
        if (timeout == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for negedge interrupt on bit %u\n", i);
#endif
            test_err++;
        }
    }

    if (test_err == 0u) {
        finish(0); /* PASS */
    } else {
        finish(1); /* FAIL */
    }
}
