// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Purpose: Busy-wait helper per unit granularity.
static inline void wait_on(unsigned int units) {
    volatile unsigned int i, j;
    for (i = 0; i < units; ++i) {
        for (j = 0; j < 1000U; ++j) {
            __asm__ __volatile__("nop");
        }
    }
}

// Purpose: MMIO read (32-bit)
static inline unsigned int mmio_read_u32(unsigned long addr) {
    return *((volatile unsigned int*)(addr));
}
// Purpose: MMIO write (32-bit)
static inline void mmio_write_u32(unsigned long addr, unsigned int val) {
    *((volatile unsigned int*)(addr)) = val;
}

// Purpose: Find index of an address within addr_array; returns -1 if not present
static int idx_of(unsigned long addr) {
    for (int i = 0; i < (int)CNT; ++i) {
        if (addr_array[i] == addr) return i;
    }
    return -1;
}

// Purpose: Entry point implementing negedge interrupt enable and validation
int test_case(void) {
    unsigned int errors = 0U;
#ifdef DEBUG_DISPLAY
    printf("[test_gpio_negedge_intr_en] Start\n");
#endif

    // Map indices for group registers (if present in impacted set)
    int idx_raw = idx_of(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
    int idx_en  = idx_of(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
    int idx_sts = idx_of(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    if (idx_raw < 0 || idx_en < 0 || idx_sts < 0) {
#ifdef DEBUG_DISPLAY
        printf("[ERR] Required group registers not in impacted set (raw=%d en=%d sts=%d)\n", idx_raw, idx_en, idx_sts);
#endif
        errors++;
        // Continue to honor deterministic flow, but behavior will be limited.
    }

    // 1) Enable platform interrupt source and CPU IRQ (implementation dependent)
    // NOTE: Using framework defaults; exact platform IRQ routing not altered here.

    // 2) Drive pad driver all-high to known state
    mmio_write_u32(PAD_DRIVER_ADDR, 0xFFFFFFFFU);
    wait_on(WAIT_UNITS);

    // 3) Configure per-pin control for input + negedge enable by writing writable bits
    for (unsigned int pin = 0; pin < 32U; ++pin) { // pins 8..39 map to indices 0..31
        unsigned int i = pin; // per-pin register index within arrays
        if (i >= CNT) break;
        unsigned long raddr = addr_array[i];
        unsigned int wmask = (unsigned int)write_mask_array[i];
        if (wmask != 0U) {
            mmio_write_u32(raddr, wmask); // assert all writable bits (enables as per HW def)
        }
    }
    wait_on(WAIT_UNITS);

    // Iterate each bit 0..31 corresponding to pins 8..39
    for (unsigned int b = 0; b < 32U; ++b) {
        // a) Pre-clear group raw for bit b
        if (idx_raw >= 0) {
            unsigned int clr_val = (1U << b) & (unsigned int)write_mask_array[idx_raw];
            mmio_write_u32(addr_array[idx_raw], clr_val);
        }

        // b) Enable only current bit in group enable
        if (idx_en >= 0) {
            unsigned int en_val = (1U << b) & (unsigned int)write_mask_array[idx_en];
            mmio_write_u32(addr_array[idx_en], en_val);
        }
        wait_on(WAIT_UNITS);

        // c) Arm waiter and generate a falling edge on current bit
        mmio_write_u32(PAD_DRIVER_ADDR, 0xFFFFFFFFU);
        wait_on(3U * WAIT_UNITS);
        unsigned int low_mask = ~(1U << b);
        mmio_write_u32(PAD_DRIVER_ADDR, 0xFFFFFFFFU & low_mask);

        // d) Wait bounded for group status bit to set
        int got = 0;
        for (unsigned int t = 0; t < TIMEOUT_ITERS; ++t) {
            if (idx_sts >= 0) {
                unsigned int sts = mmio_read_u32(addr_array[idx_sts]);
                if ((sts & (1U << b)) != 0U) { got = 1; break; }
            }
            wait_on(WAIT_UNITS);
        }
        if (!got) {
            errors++;
#ifdef DEBUG_DISPLAY
            printf("[TIMEOUT] PinIdx=%u did not signal negedge within timeout\n", b);
#endif
            continue;
        }

        // ISR-like service: clear per-pin raw (write all writable bits), and clear group raw
        unsigned int pin_idx = b; // per-pin register index (0..31)
        if (pin_idx < CNT) {
            unsigned long paddr = addr_array[pin_idx];
            unsigned int pwmask = (unsigned int)write_mask_array[pin_idx];
            if (pwmask != 0U) {
                mmio_write_u32(paddr, pwmask);
            }
        }
        if (idx_raw >= 0) {
            unsigned int clr_val = (1U << b) & (unsigned int)write_mask_array[idx_raw];
            mmio_write_u32(addr_array[idx_raw], clr_val);
        }

        // e) Verify group status reads back zero after clears
        if (idx_sts >= 0) {
            unsigned int sts = mmio_read_u32(addr_array[idx_sts]);
            if ((sts & (1U << b)) != 0U) {
                errors++;
#ifdef DEBUG_DISPLAY
                printf("[CLRCHK-FAIL] Group STS bit still set for pinIdx=%u (STS=0x%08X)\n", b, sts);
#endif
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_negedge_intr_en] %s\n", (errors==0U)?"PASS":"FAIL");
#endif

    if (errors == 0U) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
