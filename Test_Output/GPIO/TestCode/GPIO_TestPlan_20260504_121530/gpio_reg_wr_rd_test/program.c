// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Hidden_Test_Description:
// Verifies default reset values and masked write/read behavior across a defined set of GPIO registers.

// Banner: This file implements the exact procedure as specified in Hidden_Test_Steps_Procedure for gpio_reg_wr_rd_test.

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt = 0;

// Function: chk_rst_val
// Purpose: Read default values from impacted GPIO registers and compare against expected defaults per acceptance criteria.
static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; ++i) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i]) continue;                 // Skip registers per context
        if (read_mask_array[i] == 0x00000000u) continue; // Not readable

        unsigned int data_rd = read_reg((unsigned int)addr);
        unsigned int data_cmp = (data_rd & 0xFFFFFFFEu); // As per acceptance criteria
        unsigned int exp = default_value_array[i];
        if (data_cmp != exp) {
            ++def_fail_cnt;
#ifdef DEBUG_DISPLAY
            printf("[DEFCHK] Mismatch @[%u] addr=0x%08lx rd&~1=0x%08x exp=0x%08x\n", i, addr, data_cmp, exp);
#endif
        }
    }
}

// Function: chk_rd_wr
// Purpose: Perform masked write patterns to registers and validate readback against expected values.
static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6; ++j) {
        unsigned int data_wr = chk_val[j];
        // Phase WRITE
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i]) continue;                  // Skip writes where specified
            if (write_mask_array[i] == 0x00000000u) continue; // Not writable
            unsigned int wr = (data_wr & write_mask_array[i]);
            write_reg((unsigned int)addr, wr);
        }
        // Phase READ/COMPARE
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i]) continue;
            if (write_mask_array[i] == 0x00000000u) continue;
            if (read_mask_array[i] == 0x00000000u) continue;

            unsigned int data_rd = (read_reg((unsigned int)addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                ++wr_fail_cnt;
#ifdef DEBUG_DISPLAY
                printf("[WRCHK] Mismatch @[%u] addr=0x%08lx rd=0x%08x exp=0x%08x wr_mask=0x%08x rd_mask=0x%08x\n",
                       i, addr, data_rd, exp_val, write_mask_array[i], read_mask_array[i]);
#endif
            }
        }
    }
}

// Entry point
int main(void)
{
    // Step 1: Check reset values
    chk_rst_val();

    // Step 2: Check read/write behavior
    chk_rd_wr();

    // Finalization per acceptance criteria
    if (def_fail_cnt > 0u || wr_fail_cnt > 0u) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
    return 0;
}
