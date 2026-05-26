// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Verify reset default values for each register in addr_array using
//           read_mask_array and default_value_array. Skip entries per skip_rst_array.
// -----------------------------------------------------------------------------
static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    // Use the array length to ensure bounds-safe iteration
    unsigned int n = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));
    for (unsigned int i = 0; i < n; i++) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skipping index %u due to skip_rst_array\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skipping index %u due to read_mask=0\n", i);
#endif
            continue;
        }
        unsigned long int addr = addr_array[i];
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU); // Mask off bit[0]
        unsigned int exp  = default_value_array[i];
#ifdef DEBUG_DISPLAY
        printf("[DBG][RST] i=%u addr=0x%08lX rd=0x%08X masked=0x%08X exp=0x%08X\n",
               i, addr, data_rd, data, exp);
#endif
        if (data != exp) {
            def_fail_cnt++;
            printf("[ERR][RST] Mismatch at addr=0x%08lX exp=0x%08X got=0x%08X\n",
                   addr, exp, data);
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Perform masked write/read integrity checks across patterns for each
//           register in addr_array, skipping per skip_array and masks.
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    static const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    unsigned int n = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));

    for (unsigned int p = 0; p < 6U; p++) {
        unsigned int data_wr = chk_val[p];
#ifdef DEBUG_DISPLAY
        printf("[DBG][WR] Pattern[%u]=0x%08X\n", p, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0; i < n; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR] Skip write i=%u\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR] i=%u write_mask=0 -> skip write\n", i);
#endif
                continue;
            }
            unsigned long int addr = addr_array[i];
            unsigned int wdata = (data_wr & write_mask_array[i]);
#ifdef DEBUG_DISPLAY
            printf("[DBG][WR] i=%u addr=0x%08lX wmask=0x%08X wdata=0x%08X\n",
                   i, addr, write_mask_array[i], wdata);
#endif
            write_reg(addr, wdata);
        }

        // Read/verify phase
        for (unsigned int i = 0; i < n; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][RD] Skip read i=%u\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][RD] i=%u write_mask=0 -> skip read/compare\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][RD] i=%u read_mask=0 -> skip read/compare\n", i);
#endif
                continue;
            }
            unsigned long int addr = addr_array[i];
            unsigned int rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[DBG][RD] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X rmask=0x%08X wmask=0x%08X def=0x%08X\n",
                   i, addr, rd, exp, read_mask_array[i], write_mask_array[i], default_value_array[i]);
#endif
            if (rd != exp) {
                wr_fail_cnt++;
                printf("[ERR][RD] Mismatch at addr=0x%08lX exp=0x%08X got=0x%08X (pat=0x%08X)\n",
                       addr, exp, rd, data_wr);
            }
        }
    }
}

#if 0
// -----------------------------------------------------------------------------
// Function: soft_reset_chk (DISABLED)
// Purpose : Would perform a soft reset sequence using SOFT_RST_REG_ADDRESS.
// -----------------------------------------------------------------------------
static void soft_reset_chk(void)
{
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    // wait_on or delay as per platform (intentionally omitted)
    (void)read_reg(SOFT_RST_REG_ADDRESS);
}
#endif

// -----------------------------------------------------------------------------
// Function: test_case (Entry Point)
// Purpose : Orchestrates reset verification and masked write/read checks.
// Returns : finish(0) on PASS, finish(1) on FAIL.
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] Starting test_case: gpio_reg_wr_rd_test\n");
#endif
    chk_rst_val();
    chk_rd_wr();

    unsigned int err = def_fail_cnt + wr_fail_cnt;
#ifdef DEBUG_DISPLAY
    printf("[DBG] Summary: def_fail_cnt=%u, wr_fail_cnt=%u, total_err=%u\n", def_fail_cnt, wr_fail_cnt, err);
#endif

    if (err > 0U) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
    return 0; // Unreachable when finish() terminates, but keeps prototype consistent
}
