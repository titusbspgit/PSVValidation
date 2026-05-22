// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: is_special_addr
 * Purpose : Identify addresses that must be skipped for default-value comparison
 */
static int is_special_addr(unsigned long int addr)
{
    if (addr == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG)
        return 1;
    if (addr == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS)
        return 1;
    if (addr == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF)
        return 1;
    return 0;
}

/*
 * Function: chk_rst_val
 * Purpose : Validate default reset values for readable registers
 */
static void chk_rst_val(int *def_fail_cnt)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rst_val: start\n");
#endif
    for (int i = 0; i < CNT; i++) {
        unsigned long int addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];

        if (rmask == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            if (addr != 0U) printf("[DEBUG] i=%d addr=0x%08lx: read_mask=0, skip\n", i, addr);
#endif
            continue; // Skip when read mask is zero
        }

        if (is_special_addr(addr)) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] i=%d addr=0x%08lx: special register skip for default compare\n", i, addr);
#endif
            continue; // Skip default-value comparison for special addresses
        }

        unsigned int data_rd = read_reg(addr);
        unsigned int def_val = default_value_array[i];

        if (data_rd != def_val) {
            (*def_fail_cnt)++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF-MISMATCH] i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, def_val);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF-OK] i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rst_val: end, def_fail_cnt=%d\n", *def_fail_cnt);
#endif
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform write-then-readback validation across patterns
 */
static void chk_rd_wr(int *wr_fail_cnt)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rd_wr: start\n");
#endif

    for (int j = 0; j < 6; j++) {
        unsigned int data_wr = (unsigned int)chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %d: data_wr=0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1)
                continue; // Skip per test plan
            if (write_mask_array[i] == 0x00000000U)
                continue; // Not writable
            unsigned long int addr = addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][WRITE] i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1)
                continue; // Skip per test plan
            if (write_mask_array[i] == 0x00000000U)
                continue; // Not writable, no expectation
            if (read_mask_array[i] == 0x00000000U)
                continue; // Not readable

            unsigned long int addr = addr_array[i];
            unsigned int rmask = read_mask_array[i];
            unsigned int wmask = write_mask_array[i];
            unsigned int def_val = default_value_array[i];

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (wmask ^ 0xffffffffU);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & def_val));

            if (data_rd != exp_val) {
                (*wr_fail_cnt)++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WRRD-MISMATCH] i=%d addr=0x%08lx rd=0x%08x exp=0x%08x wmask=0x%08x rmask=0x%08x def=0x%08x\n",
                       i, addr, data_rd, exp_val, wmask, rmask, def_val);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WRRD-OK] i=%d addr=0x%08lx rd=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rd_wr: end, wr_fail_cnt=%d\n", *wr_fail_cnt);
#endif
}

/*
 * Entry Point: test_case
 * Executes default check and write-read validation, then terminates with finish().
 */
int test_case(void)
{
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_case: start\n");
#endif

    // soft_reset_chk(); // Present in meta but intentionally not executed per test plan

    chk_rst_val(&def_fail_cnt);
    chk_rd_wr(&wr_fail_cnt);

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_case: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    return 0; // Unreachable in typical frameworks after finish(), but kept for completeness
}
