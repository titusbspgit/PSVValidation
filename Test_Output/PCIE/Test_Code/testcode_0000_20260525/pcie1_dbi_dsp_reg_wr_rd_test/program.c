// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: test_case
 * ---------------------------------------
 * Entry point for the test. Implements the exact Meta Test Steps:
 * 1) Default value verification over readable, non-excluded registers.
 * 2) Write/read-back verification for 6 patterns with mask-based expectation.
 * Terminates only via finish(0)/finish(1) based on acceptance criteria.
 */
int test_case(void)
{
    unsigned int i = 0U, j = 0U;
    unsigned long addr = 0UL;
    unsigned int data_rd = 0U;
    unsigned int data_wr = 0U;
    unsigned int wr_n = 0U;
    unsigned int exp_val = 0U;
    unsigned int def_fail_cnt = 0U;
    unsigned int wr_fail_cnt = 0U;

    /* Use only available entries; respect CNT while preventing out-of-bounds */
    const unsigned int arr_count = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));
    const unsigned int limit = (CNT < arr_count) ? CNT : arr_count;

    /* Phase 1: Default value check */
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Starting default value verification on %u entries (limit by CNT=%u)\n", arr_count, CNT);
#endif
    for (i = 0U; i < limit; i++) {
        addr = addr_array[i];

        /* Skip unreadable entries */
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Skip default check (read_mask=0) idx=%u addr=0x%08lX\n", i, addr);
#endif
            continue;
        }

        /* Skip specific addresses per Meta */
        if ((addr == mizar_PCIE1_DBI_DSP_CAP_ID_NXT_PTR_REG) ||
            (addr == mizar_PCIE1_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS) ||
            (addr == mizar_PCIE1_DBI_DSP_PL_DEBUG1_OFF)) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Skip default check (excluded) idx=%u addr=0x%08lX\n", i, addr);
#endif
            continue;
        }

        data_rd = read_reg(addr);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Default chk idx=%u addr=0x%08lX exp=0x%08X rd=0x%08X\n", i, addr, default_value_array[i], data_rd);
#endif
        if (data_rd != default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][FAIL] Default mismatch idx=%u addr=0x%08lX exp=0x%08X rd=0x%08X\n", i, addr, default_value_array[i], data_rd);
#endif
        }
    }

    /* Phase 2: Write/Read-back check across 6 patterns */
    {
        const unsigned int chk_val[6] = {
            0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
        };
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Starting write/read-back verification\n");
#endif
        for (j = 0U; j < 6U; j++) {
            data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Pattern %u: data_wr=0x%08X\n", j, data_wr);
#endif
            /* Write loop */
            for (i = 0U; i < limit; i++) {
                addr = addr_array[i];
                if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG] Skip write (skip_array=1) idx=%u addr=0x%08lX\n", i, addr);
#endif
                    continue;
                }
                if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG] Skip write (write_mask=0) idx=%u addr=0x%08lX\n", i, addr);
#endif
                    continue;
                }
                write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write idx=%u addr=0x%08lX data=0x%08X\n", i, addr, data_wr);
#endif
            }

            /* Read/verify loop */
            for (i = 0U; i < limit; i++) {
                addr = addr_array[i];
                if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG] Skip verify (skip_array=1) idx=%u addr=0x%08lX\n", i, addr);
#endif
                    continue;
                }
                if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG] Skip verify (write_mask=0) idx=%u addr=0x%08lX\n", i, addr);
#endif
                    continue;
                }
                if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG] Skip verify (read_mask=0) idx=%u addr=0x%08lX\n", i, addr);
#endif
                    continue;
                }

                data_rd = read_reg(addr);
                wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
                exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                           (wr_n & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Verify idx=%u addr=0x%08lX exp=0x%08X rd=0x%08X wr_n=0x%08X rm=0x%08X wm=0x%08X\n",
                       i, addr, exp_val, data_rd, wr_n, read_mask_array[i], write_mask_array[i]);
#endif
                if (data_rd != exp_val) {
                    wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG][FAIL] WR mismatch idx=%u addr=0x%08lX exp=0x%08X rd=0x%08X\n", i, addr, exp_val, data_rd);
#endif
                }
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Summary: def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1);
    } else {
        finish(0);
    }

    return 0; /* finish() is terminal; return to satisfy compiler */
}
