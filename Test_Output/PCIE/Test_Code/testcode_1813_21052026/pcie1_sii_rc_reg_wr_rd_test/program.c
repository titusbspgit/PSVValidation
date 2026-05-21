// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Phase 1 - Check default/reset values for readable registers
// Notes   : Skips registers with read_mask_array[i] == 0 and
//           skips the SII PHY reset control register during default checks.
// -----------------------------------------------------------------------------
static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        // Skip if read mask is zero (no readable bits)
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%d addr=0x%08lx skipped (read_mask==0)\n", i, addr);
#endif
            continue;
        }
        // Skip default verification for the PHY reset control register
        if (addr == mizar_PCIE1_SII PHY RST CONTROL) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%d addr=0x%08lx skipped (PHY RST CONTROL)\n", i, addr);
#endif
            continue;
        }
        // Read the register and compare against the expected default value
        unsigned int data_rd = read_reg(addr);
#ifdef DEBUG_DISPLAY
        printf("[DBG] chk_rst_val: i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, default_value_array[i]);
#endif
        if (data_rd != (unsigned int)default_value_array[i]) {
            extern int def_fail_cnt; // declared in test_case
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] Default mismatch at i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, default_value_array[i]);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Phase 2 - Masked write and read-back verification for impacted regs
// Notes   : Respects skip_array and read/write masks as provided by Meta Arrays.
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    static const unsigned int chk_val[6] = {
        0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000
    };

    for (int pat = 0; pat < 6; pat++) {
        unsigned int data_wr = chk_val[pat];
#ifdef DEBUG_DISPLAY
        printf("[DBG] chk_rd_wr: pattern[%d]=0x%08x\n", pat, data_wr);
#endif
        // Write pass
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write-skip: i=%d (skip_array==1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write-skip: i=%d (write_mask==0)\n", i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
#ifdef DEBUG_DISPLAY
            printf("[DBG] write: i=%d addr=0x%08lx wr=0x%08x\n", i, addr, data_wr);
#endif
            write_reg(addr, data_wr);
        }

        // Read/verify pass
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (skip_array==1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (write_mask==0)\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (read_mask==0)\n", i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (unsigned int)(write_mask_array[i] ^ 0xffffffff);
            unsigned int exp_val = (unsigned int)((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                 (wr_n & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[DBG] verify: i=%d addr=0x%08lx rd=0x%08x exp=0x%08x rm=0x%08x wm=0x%08x def=0x%08x\n",
                   i, addr, data_rd, exp_val, read_mask_array[i], write_mask_array[i], default_value_array[i]);
#endif
            if (data_rd != exp_val) {
                extern int wr_fail_cnt; // declared in test_case
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR] R/W mismatch at i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, exp_val);
#endif
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Function: test_case (ENTRY POINT)
// Purpose : Execute test steps in order and finalize with finish(0/1)
// -----------------------------------------------------------------------------
int def_fail_cnt = 0;
int wr_fail_cnt = 0;
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] Enter test_case: pcie1_sii_rc_reg_wr_rd_test\n");
#endif
    // Initialize failure counters
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    // Phase 1: Default value checks
    chk_rst_val();

    // Phase 2: Masked write/read verification
    chk_rd_wr();

    // Final verdict per Acceptance Criteria
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
#ifdef DEBUG_DISPLAY
        printf("[FAIL] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1); // FAIL
        return 1;
    }
#ifdef DEBUG_DISPLAY
    printf("[PASS] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
    finish(0); // PASS
    return 0;
}
