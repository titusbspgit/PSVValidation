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

// Purpose: MMIO read (32-bit) from a given address
static inline unsigned int mmio_read_u32(unsigned long addr) {
    return *((volatile unsigned int*)(addr));
}

// Purpose: MMIO write (32-bit) to a given address
static inline void mmio_write_u32(unsigned long addr, unsigned int val) {
    *((volatile unsigned int*)(addr)) = val;
}

// Purpose: Main test entry as per plan - default checks and masked write/read
int test_case(void) {
    unsigned int errors = 0U;

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] Start\n");
#endif

    // 1) Default value checks
    for (unsigned int i = 0; i < CNT; ++i) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DefaultChk] Skipping index %u due to skip_rst_array\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0U) {
#ifdef DEBUG_DISPLAY
            printf("[DefaultChk] Skipping index %u due to read_mask=0\n", i);
#endif
            continue;
        }
        unsigned long addr = addr_array[i];
        unsigned int rd = mmio_read_u32(addr);
        // Apply read mask and clear LSB per test plan before compare
        rd &= read_mask_array[i];
        rd &= ~1U;
        unsigned int exp = (default_value_array[i] & read_mask_array[i]);
        if (rd != exp) {
            errors++;
#ifdef DEBUG_DISPLAY
            printf("[DefaultChk][IDX=%u][0x%08lX] RD=0x%08X EXP=0x%08X\n", i, addr, rd, exp);
#endif
        }
    }

    // 2) Masked write-read verification across six patterns
    const unsigned int patterns[6] = {
        0x00000000U, 0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x12345678U, 0x87654321U
    };

    for (unsigned int p = 0; p < 6U; ++p) {
        unsigned int pat = patterns[p];
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[WrRd][PAT=%u] Skipping index %u due to skip_array\n", p, i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0U || read_mask_array[i] == 0U) {
#ifdef DEBUG_DISPLAY
                printf("[WrRd][PAT=%u] Skipping index %u due to mask=0 (WR or RD)\n", p, i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
            unsigned int wr = (pat & write_mask_array[i]);
            mmio_write_u32(addr, wr);
            // Read back and validate expected composition
            unsigned int rd = mmio_read_u32(addr) & read_mask_array[i];
            unsigned int def = default_value_array[i];
            unsigned int exp = ((def & (~write_mask_array[i])) | wr) & read_mask_array[i];
            if (rd != exp) {
                errors++;
#ifdef DEBUG_DISPLAY
                printf("[WrRd][PAT=%u][IDX=%u][0x%08lX] RD=0x%08X EXP=0x%08X WR=0x%08X DEF=0x%08X WMSK=0x%08X RMSK=0x%08X\n",
                       p, i, addr, rd, exp, wr, def, write_mask_array[i], read_mask_array[i]);
#endif
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] %s\n", (errors==0U)?"PASS":"FAIL");
#endif

    if (errors == 0U) {
        finish(0);
    } else {
        finish(1);
    }
    return 0;
}
