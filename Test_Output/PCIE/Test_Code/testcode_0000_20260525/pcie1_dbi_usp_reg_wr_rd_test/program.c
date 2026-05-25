// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* Banner: Test entry point and helpers */
static int def_fail_cnt = 0;  // Default value mismatches
static int wr_fail_cnt  = 0;  // Write/read-back mismatches

// Forward declarations
static void chk_rst_val(void);
static void chk_rd_wr(void);

// -----------------------------------------------------------------------------
// Function: test_case
// Description: Entry point for the test. Executes reset/default checks followed
//              by write/read-back verification. Concludes with finish(0/1).
// -----------------------------------------------------------------------------
int test_case(void)
{
    // Phase 1: Check default/reset values
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Starting default value verification...\n");
#endif
    chk_rst_val();

    // Phase 2: Check write -> read-back with masks applied
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Starting write/read-back verification...\n");
#endif
    chk_rd_wr();

    // Final decision based on accumulated errors
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] def_fail_cnt=%d, wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    return 0; // Not reached; finish() terminates
}

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Description: Iterate all addresses and check default values where readable and
//              not excluded by address condition.
// -----------------------------------------------------------------------------
static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];

        // Skip if not readable
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] idx=%d addr=0x%08lx skip (read_mask==0)\n", i, addr);
#endif
            continue;
        }

        // Skip specific addresses from default check
        if ((addr == mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG) ||
            (addr == mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS) ||
            (addr == mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF)) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] idx=%d addr=0x%08lx default-check skipped by rule\n", i, addr);
#endif
            continue;
        }

        // Read and compare to default value
        unsigned int data_rd = read_reg(addr);
        unsigned int exp_def = (unsigned int)default_value_array[i];
        if (data_rd != exp_def) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF] Mismatch at idx=%d addr=0x%08lx exp=0x%08x rd=0x%08x\n",
                   i, addr, exp_def, data_rd);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF] Match at idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Description: For each test pattern, write to allowed registers and verify the
//              read-back value using read/write masks and default values.
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    unsigned int chk_val[6] = {
        0xffffffffu, 0xaaaaaaaau, 0x55555555u, 0x00000000u, 0xA5A5A5A5u, 0xffff0000u
    };

    for (int j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG][WR] Pattern[%d]=0x%08x\n", j, data_wr);
#endif

        // Write phase
        for (int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];

            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WR] idx=%d addr=0x%08lx skipped (skip_array)\n", i, addr);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WR] idx=%d addr=0x%08lx skipped (write_mask==0)\n", i, addr);
#endif
                continue;
            }

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][WR] idx=%d addr=0x%08lx wrote 0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];

            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RD] idx=%d addr=0x%08lx skipped (skip_array)\n", i, addr);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RD] idx=%d addr=0x%08lx skipped (write_mask==0)\n", i, addr);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RD] idx=%d addr=0x%08lx skipped (read_mask==0)\n", i, addr);
#endif
                continue;
            }

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n    = (unsigned int)(write_mask_array[i] ^ 0xffffffffu);
            unsigned int exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                                    (wr_n    & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));

            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RD] Mismatch idx=%d addr=0x%08lx exp=0x%08x rd=0x%08x\n",
                       i, addr, exp_val, data_rd);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RD] Match idx=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}
