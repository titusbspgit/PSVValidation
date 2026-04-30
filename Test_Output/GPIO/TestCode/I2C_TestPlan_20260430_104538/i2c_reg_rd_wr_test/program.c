// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

// Purpose: Validate I2C register default/reset values and masked read/write behavior using fixed data patterns.
// Derived from Hidden_Test_Description.

// Function: chk_rst_val
// Verifies reset/default values of all readable registers in impacted list using default_value_array and read_mask_array.
static void chk_rst_val(unsigned int *def_fail_cnt)
{
    unsigned int i;
    for (i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        unsigned long rmask = read_mask_array[i];
        if (rmask == 0x00000000UL) {
#ifdef DEBUG_DISPLAY
            printf("[I2C][RST] Skipping read (rmask=0) idx=%u addr=0x%08lx\n", i, addr);
#endif
            continue;
        }
        unsigned long data_rd = read_reg(addr);
        unsigned long exp = default_value_array[i];
        if (data_rd != exp) {
            (*def_fail_cnt)++;
#ifdef DEBUG_DISPLAY
            printf("[I2C][RST][FAIL] idx=%u addr=0x%08lx rd=0x%08lx exp=0x%08lx\n", i, addr, data_rd, exp);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[I2C][RST][PASS] idx=%u addr=0x%08lx val=0x%08lx\n", i, addr, data_rd);
#endif
        }
    }
}

// Function: chk_rd_wr
// Performs masked write/readback verification across all non-skipped, writable and readable registers for fixed patterns in chk_val[]
static void chk_rd_wr(unsigned int *wr_fail_cnt)
{
    unsigned int j;
    for (j = 0; j < 6; j++) {
        unsigned long data_wr = chk_val[j];
        // Write phase
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[I2C][WR] Skip idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            unsigned long wmask = write_mask_array[i];
            if (wmask == 0x00000000UL) {
#ifdef DEBUG_DISPLAY
                printf("[I2C][WR] No-write (wmask=0) idx=%u addr=0x%08lx\n", i, addr);
#endif
                continue;
            }
            write_reg(addr, data_wr);
        }
        // Readback and verify phase
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) continue;
            unsigned long wmask = write_mask_array[i];
            if (wmask == 0x00000000UL) continue;
            unsigned long rmask = read_mask_array[i];
            if (rmask == 0x00000000UL) continue;

            unsigned long data_rd = read_reg(addr);
            unsigned long wr_n = (wmask ^ 0xFFFFFFFFUL);
            unsigned long exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));
            if (data_rd != exp_val) {
                (*wr_fail_cnt)++;
#ifdef DEBUG_DISPLAY
                printf("[I2C][RD][FAIL] idx=%u addr=0x%08lx rd=0x%08lx exp=0x%08lx data_wr=0x%08lx wmask=0x%08lx rmask=0x%08lx def=0x%08lx\n",
                       i, addr, data_rd, exp_val, data_wr, wmask, rmask, default_value_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[I2C][RD][PASS] idx=%u addr=0x%08lx val=0x%08lx\n", i, addr, data_rd);
#endif
            }
        }
    }
}

// Entry point: test_case
// Implements the exact termination criteria from Hidden_Validation_Acceptance_Criteria.
void test_case(void)
{
    unsigned int def_fail_cnt = 0, wr_fail_cnt = 0;

    chk_rst_val(&def_fail_cnt);
    chk_rd_wr(&wr_fail_cnt);

#ifdef DEBUG_DISPLAY
    printf("[I2C][SUMMARY] def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
}
