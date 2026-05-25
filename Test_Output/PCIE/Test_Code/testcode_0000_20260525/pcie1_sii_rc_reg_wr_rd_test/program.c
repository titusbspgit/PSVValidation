// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Default value verification per Meta Test Steps
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    // def_fail_cnt is maintained in test_case() scope via static linkage
    extern int def_fail_cnt;

    for (int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        // Skip if not readable
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping addr index %d (read_mask=0)\n", i);
#endif
            continue;
        }
        // Skip specific register for default check
        if (addr == mizar_PCIE1_SII_PHY_RST_CONTROL) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping default check for PHY_RST_CONTROL at idx %d\n", i);
#endif
            continue;
        }

        unsigned int data_rd = read_reg(addr);
        if (data_rd != (unsigned int)default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[FAIL-DEF] idx=%d addr=0x%08lx exp=0x%08x rd=0x%08x\n", i, addr, (unsigned int)default_value_array[i], data_rd);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[PASS-DEF] idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Write/Read-back verification per Meta Test Steps
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    // wr_fail_cnt is maintained in test_case() scope via static linkage
    extern int wr_fail_cnt;

    unsigned int chk_val[6] = {0xffffffffu, 0xaaaaaaaau, 0x55555555u, 0x00000000u, 0xA5A5A5A5u, 0xffff0000u};

    for (int j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] Pattern %d: 0x%08x\n", j, data_wr);
#endif
        // Write loop
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Write-skip idx %d (skip_array==1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Write-skip idx %d (write_mask==0)\n", i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[WR ] idx=%d addr=0x%08lx data=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify loop
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Read-skip idx %d (skip_array==1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Read-skip idx %d (write_mask==0)\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Read-skip idx %d (read_mask==0)\n", i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (unsigned int)(write_mask_array[i] ^ 0xffffffffu);
            unsigned int exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                                    (wr_n & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[FAIL-WR] idx=%d addr=0x%08lx exp=0x%08x rd=0x%08x\n", i, addr, exp_val, data_rd);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[PASS-WR] idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}

// ------------------------------------------------------------
// Function: test_case (Entry Point)
// Purpose : Orchestrate default and read/write checks; finalize
// ------------------------------------------------------------
int def_fail_cnt = 0;
int wr_fail_cnt = 0;
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[TEST] pcie1_sii_rc_reg_wr_rd_test start\n");
#endif
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
#ifdef DEBUG_DISPLAY
        printf("[TEST] FAIL: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return 1;
    }
#ifdef DEBUG_DISPLAY
    printf("[TEST] PASS\n");
#endif
    finish(0);
    return 0;
}
