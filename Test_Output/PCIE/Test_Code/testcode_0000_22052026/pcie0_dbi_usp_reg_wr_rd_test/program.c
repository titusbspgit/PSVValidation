// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Validate default/reset values for readable registers
// Notes   : Skips entries with read_mask == 0x00000000 and specific addresses per Meta
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++) {
        unsigned long int addr = addr_array[i];

        // Skip if read mask is 0x00000000 as per Meta procedure
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: Skipping index %d due to read mask 0x0\n", i);
#endif
            continue;
        }

        // Skip default comparison for specific addresses per Meta
        if (addr == mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG ||
            addr == mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS ||
            addr == mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: Skipping default compare for special addr index %d (0x%08lx)\n", i, addr);
#endif
            continue;
        }

        int data_rd = read_reg(addr);              // Read current value
        int def_val = default_value_array[i];      // Expected default

        if (data_rd != def_val) {
            // Increment default failure counter on mismatch
            extern int def_fail_cnt; // defined below
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF-MISMATCH] idx=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, (unsigned int)data_rd, (unsigned int)def_val);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Default OK: idx=%d addr=0x%08lx val=0x%08x\n", i, addr, (unsigned int)data_rd);
#endif
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Perform write/readback validation with six data patterns
// Notes   : Honors skip_array, write_mask_array, and read_mask_array gating
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    for (int j = 0; j < 6; j++) {
        int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %d: data_wr=0x%08x\n", j, (unsigned int)data_wr);
#endif
        // Write phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write skip: idx=%d reason=skip_array\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write skip: idx=%d reason=write_mask==0\n", i);
#endif
                continue;
            }
            unsigned long int addr = addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Wrote: idx=%d addr=0x%08lx data=0x%08x\n", i, addr, (unsigned int)data_wr);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read skip: idx=%d reason=skip_array\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read skip: idx=%d reason=write_mask==0\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read skip: idx=%d reason=read_mask==0\n", i);
#endif
                continue;
            }

            unsigned long int addr = addr_array[i];
            int data_rd = read_reg(addr);

            int wr_n = (write_mask_array[i] ^ 0xffffffff);
            int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                           (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                extern int wr_fail_cnt; // defined below
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WR-MISMATCH] idx=%d addr=0x%08lx rd=0x%08x exp=0x%08x wmask=0x%08x rmask=0x%08x def=0x%08x\n",
                       i, addr, (unsigned int)data_rd, (unsigned int)exp_val,
                       (unsigned int)write_mask_array[i], (unsigned int)read_mask_array[i], (unsigned int)default_value_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read OK: idx=%d addr=0x%08lx val=0x%08x\n", i, addr, (unsigned int)data_rd);
#endif
            }
        }
    }
}

// ------------------------------------------------------------
// Function: soft_reset_chk
// Purpose : Soft reset write and restore sequence (not invoked per Meta)
// ------------------------------------------------------------
static void soft_reset_chk(void)
{
    int org_val = read_reg(SOFT_RST_REG_ADDRESS);
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(1000);
    write_reg(SOFT_RST_REG_ADDRESS, org_val);
    wait_on(1000);
}

// Global error counters as specified by Meta acceptance criteria
int def_fail_cnt = 0;
int wr_fail_cnt = 0;

// ------------------------------------------------------------
// Entry Point: test_case
// Purpose    : Execute reset-value checks followed by write/read checks
// Terminate  : finish(0) on PASS, finish(1) on FAIL
// ------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_case: START\n");
#endif

    // Execute checks in the given order without reordering
    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_case: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    return 0; // Function signature requires int; finish() performs test termination
}
