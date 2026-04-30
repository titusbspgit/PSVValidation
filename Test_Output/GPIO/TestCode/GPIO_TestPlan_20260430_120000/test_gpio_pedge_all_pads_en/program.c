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

// Purpose: Entry point implementing pedge all pads test and validation
int test_case(void) {
    unsigned int errors = 0U;
#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] Start\n");
#endif

    // Map indices for group registers
    int idx_raw = idx_of(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1);
    int idx_en  = idx_of(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
    int idx_sts = idx_of(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    int idx_g1  = idx_of(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1);
    int idx_g2  = idx_of(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2);
    int idx_g3  = idx_of(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3);
    int idx_g4  = idx_of(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4);

    if (idx_raw < 0 || idx_en < 0 || idx_sts < 0 || idx_g1 < 0 || idx_g2 < 0 || idx_g3 < 0 || idx_g4 < 0) {
#ifdef DEBUG_DISPLAY
        printf("[ERR] Required group registers missing (raw=%d en=%d sts=%d g1=%d g2=%d g3=%d g4=%d)\n", idx_raw, idx_en, idx_sts, idx_g1, idx_g2, idx_g3, idx_g4);
#endif
        errors++;
    }

    // 2) Enable positive-edge detection per pin: write all writable bits in per-pin controls
    for (unsigned int pin = 0; pin < 32U; ++pin) {
        unsigned int i = pin; // per-pin indices are 0..31
        if (i >= CNT) break;
        unsigned long raddr = addr_array[i];
        unsigned int wmask = (unsigned int)write_mask_array[i];
        if (wmask != 0U) {
            mmio_write_u32(raddr, wmask);
        }
    }
    wait_on(WAIT_UNITS);

    // 4) Configure group input mode via GPIO_IO_CTRL_GROUP1..4
    if (idx_g1 >= 0) mmio_write_u32(addr_array[idx_g1], (unsigned int)write_mask_array[idx_g1]);
    if (idx_g2 >= 0) mmio_write_u32(addr_array[idx_g2], (unsigned int)write_mask_array[idx_g2]);
    if (idx_g3 >= 0) mmio_write_u32(addr_array[idx_g3], (unsigned int)write_mask_array[idx_g3]);
    if (idx_g4 >= 0) mmio_write_u32(addr_array[idx_g4], (unsigned int)write_mask_array[idx_g4]);
    wait_on(WAIT_UNITS);

    // 5) Enable all bits in group enable register
    if (idx_en >= 0) {
        unsigned int en_all = (unsigned int)write_mask_array[idx_en];
        mmio_write_u32(addr_array[idx_en], en_all);
    }

    // Iterate each bit 0..31 corresponding to pins 8..39
    for (unsigned int b = 0; b < 32U; ++b) {
        // a) Drive the pad driver low and wait
        mmio_write_u32(PAD_DRIVER_ADDR, 0x00000000U);
        wait_on(WAIT_UNITS);

        // b) Arm waiter and drive high to create a single rising edge for bit b
        unsigned int high_val = (1U << b);
        mmio_write_u32(PAD_DRIVER_ADDR, high_val);

        // c) Poll for interrupt with bounded timeout
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
            printf("[TIMEOUT] PinIdx=%u did not signal pedge within timeout\n", b);
#endif
            break; // as per plan, break on timeout
        }

        // ISR-like: mask group, clear per-pin and group raw, verify cleared, re-enable group
        if (idx_en >= 0) {
            mmio_write_u32(addr_array[idx_en], 0U); // mask group output
        }
        // Clear per-pin raw by writing all writable bits
        unsigned int pin_idx = b;
        if (pin_idx < CNT) {
            unsigned long paddr = addr_array[pin_idx];
            unsigned int pwmask = (unsigned int)write_mask_array[pin_idx];
            if (pwmask != 0U) mmio_write_u32(paddr, pwmask);
        }
        // Clear group raw
        if (idx_raw >= 0) {
            unsigned int clr_val = (1U << b) & (unsigned int)write_mask_array[idx_raw];
            mmio_write_u32(addr_array[idx_raw], clr_val);
        }
        // Verify group status cleared
        if (idx_sts >= 0) {
            unsigned int sts = mmio_read_u32(addr_array[idx_sts]);
            if ((sts & (1U << b)) != 0U) {
                errors++;
#ifdef DEBUG_DISPLAY
                printf("[CLRCHK-FAIL] Group STS bit still set for pinIdx=%u (STS=0x%08X)\n", b, sts);
#endif
            }
        }
        // Re-enable group output for next iteration
        if (idx_en >= 0) {
            unsigned int en_all = (unsigned int)write_mask_array[idx_en];
            mmio_write_u32(addr_array[idx_en], en_all);
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] %s\n", (errors==0U)?"PASS":"FAIL");
#endif

    if (errors == 0U) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
