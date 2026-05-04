// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

// High-level description (from META):
// Default reset value verification for GPIO per-pin and group registers, followed by masked write/read
// verification using multiple data patterns across all non-skipped registers.

#include "test_define.c"

// Forward declarations
static int chk_rst_val(void);
static int chk_rd_wr(void);

// Function: chk_rst_val
// Purpose: Verify reset values for all registers where reset check is enabled and readable per mask.
static int chk_rst_val(void)
{
    int def_fail_cnt = 0;
    for (unsigned int i = 0; i < CNT; ++i) {
        if (skip_rst_array[i] == 1u) {
            continue; // Skip reset-check for this index
        }
        if (read_mask_array[i] == 0u) {
            continue; // No readable fields
        }
        unsigned long addr = addr_array[i];
        unsigned int data_rd = read_reg(addr);
        unsigned int data_cmp = (data_rd & 0xFFFFFFFEu); // Ignore LSB per META
        unsigned int def_val = default_value_array[i];
        if (data_cmp != def_val) {
            ++def_fail_cnt;
#ifdef DEBUG_DISPLAY
            printf("[RSTCHK] Mismatch @idx=%u addr=0x%08lx rd=0x%08x cmp=0x%08x exp=0x%08x\n",
                   i, addr, data_rd, data_cmp, def_val);
#endif
        }
    }
    return def_fail_cnt;
}

// Function: chk_rd_wr
// Purpose: Perform masked write then read-back verification for multiple data patterns across all registers.
static int chk_rd_wr(void)
{
    int wr_fail_cnt = 0;
    for (unsigned int p = 0; p < (sizeof(chk_val)/sizeof(chk_val[0])); ++p) {
        unsigned int data_wr = chk_val[p];
        // Write phase
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u) {
                continue; // Skip per directive
            }
            unsigned int wmask = write_mask_array[i];
            if (wmask == 0u) {
                continue; // Nothing writable
            }
            unsigned long addr = addr_array[i];
            write_reg(addr, (data_wr & wmask));
        }
        // Read/verify phase
        for (unsigned int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u) {
                continue; // Skip
            }
            unsigned int wmask = write_mask_array[i];
            unsigned int rmask = read_mask_array[i];
            if (wmask == 0u || rmask == 0u) {
                continue; // Not verifiable
            }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & rmask);
            unsigned int wr_n = (~wmask);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));
            if (data_rd != exp_val) {
                ++wr_fail_cnt;
#ifdef DEBUG_DISPLAY
                printf("[WRCHK] Mismatch @pat=%u idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x wmask=0x%08x rmask=0x%08x\n",
                       p, i, addr, data_rd, exp_val, wmask, rmask);
#endif
            }
        }
    }
    return wr_fail_cnt;
}

// Function: test_case
// Purpose: Execute reset check and read/write masked verification; conclude with finish() based on errors.
int test_case(void)
{
    int def_fail_cnt = chk_rst_val();
    int wr_fail_cnt = chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%d, wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
    return 0;
}
