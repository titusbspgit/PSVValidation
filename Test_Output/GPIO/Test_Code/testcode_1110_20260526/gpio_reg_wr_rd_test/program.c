// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Globals for error tracking
// -----------------------------------------------------------------------------
static unsigned int def_fail_cnt = 0U;   // default value mismatches
static unsigned int wr_fail_cnt  = 0U;   // write/readback mismatches

// -----------------------------------------------------------------------------
// Function: static void chk_rst_val(void)
// Purpose : Validate default values for readable registers
// -----------------------------------------------------------------------------
static void chk_rst_val(void)
{
    // Array length determined from provided meta arrays (20 entries)
    const unsigned int N = 20U;
    for (unsigned int i = 0U; i < N; ++i) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] skip_rst idx=%u addr=0x%08lX\n", i, addr);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] unreadable idx=%u addr=0x%08lX\n", i, addr);
#endif
            continue;
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU); // ignore bit0 per Meta
        unsigned int exp  = default_value_array[i];
        if (data != exp) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][RST] idx=%u addr=0x%08lX exp=0x%08X got=0x%08X\n", i, addr, exp, data);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[OK ][RST] idx=%u addr=0x%08lX val=0x%08X\n", i, addr, data);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Function: static void chk_rd_wr(void)
// Purpose : Perform write/readback checks using specified masks and patterns
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    const unsigned int N = 20U;
    const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAA AU, 0x55555555U,
        0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int j = 0U; j < 6U; ++j) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG][WR ] pattern[%u]=0x%08X\n", j, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0U; i < N; ++i) {
            if (skip_array[i] == 1U) { continue; }
            if (write_mask_array[i] == 0x00000000U) { continue; }
            unsigned long addr = addr_array[i];
            unsigned int wval = (data_wr & write_mask_array[i]);
            write_reg(addr, wval);
#ifdef DEBUG_DISPLAY
            printf("[DBG][WR ] idx=%u addr=0x%08lX wval=0x%08X\n", i, addr, wval);
#endif
        }
        // Read/verify phase
        for (unsigned int i = 0U; i < N; ++i) {
            if (skip_array[i] == 1U) { continue; }
            if (write_mask_array[i] == 0x00000000U) { continue; }
            if (read_mask_array[i]  == 0x00000000U) { continue; }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR][RD ] idx=%u addr=0x%08lX exp=0x%08X got=0x%08X\n", i, addr, exp_val, data_rd);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[OK ][RD ] idx=%u addr=0x%08lX val=0x%08X\n", i, addr, data_rd);
#endif
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: int test_case(void)
// Purpose : Entry point that executes the Meta-defined sequence and terminates
//           only via finish(0) on PASS or finish(1) on FAIL.
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] gpio_reg_wr_rd_test: START\n");
#endif

    // 1) Check reset/default values
    chk_rst_val();
#ifdef DEBUG_DISPLAY
    printf("[INFO] gpio_reg_wr_rd_test: chk_rst_val() done\n");
#endif

    // 2) Check read/write functionality with masks and patterns
    chk_rd_wr();
#ifdef DEBUG_DISPLAY
    printf("[INFO] gpio_reg_wr_rd_test: chk_rd_wr() done\n");
#endif

    // Final verdict
    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[INFO] END: FAIL (def=%u wr=%u)\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return 0; // not reached
    }
#ifdef DEBUG_DISPLAY
    printf("[INFO] END: PASS\n");
#endif
    finish(0);
    return 0; // not reached
}
