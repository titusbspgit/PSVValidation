// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: test_case
 * Description:
 *   Executes default value verification and write/read-back checks
 *   for the impacted PCIe0 DBI USP registers as specified in Meta Steps.
 */
int test_case(void)
{
    unsigned int def_fail_cnt = 0U;   /* Default value check failures */
    unsigned int wr_fail_cnt  = 0U;   /* Write/read-back check failures */

    unsigned int i, j;                /* Loop indices */
    unsigned long addr;               /* Current register address */
    unsigned int data_rd;             /* Read data */
    unsigned int data_wr;             /* Write data (pattern) */
    unsigned int exp_val;             /* Expected value after masking */
    unsigned int wr_n;                /* Inverted write mask */

    /* Test patterns per Meta */
    const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAaU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
    };

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Starting default value verification...\n");
#endif

    /* Phase 1: Default value check */
    for (i = 0U; i < (unsigned int)CNT; i++)
    {
        addr = addr_array[i];

        /* Skip when read mask is zero */
        if (read_mask_array[i] == 0x00000000U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Skipping addr_array[%u] due to read_mask==0\n", i);
#endif
            continue;
        }

        /* Exclude specific addresses from default check per Meta */
        if (addr == mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG ||
            addr == mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS ||
            addr == mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Excluding addr_array[%u] from default check\n", i);
#endif
            continue;
        }

        /* Read and compare with default value */
        data_rd = read_reg(addr);
        if (data_rd != default_value_array[i])
        {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF-FAIL] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X\n", (unsigned)i, addr, data_rd, default_value_array[i]);
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Starting write/read-back verification...\n");
#endif

    /* Phase 2: Write and read-back checks */
    for (j = 0U; j < 6U; j++)
    {
        data_wr = chk_val[j];

#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern[%u]=0x%08X - Write phase\n", (unsigned)j, data_wr);
#endif
        /* Write phase */
        for (i = 0U; i < (unsigned int)CNT; i++)
        {
            addr = addr_array[i];
            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skipping write at i=%u (skip_array==1)\n", (unsigned)i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skipping write at i=%u (write_mask==0)\n", (unsigned)i);
#endif
                continue;
            }
            write_reg(addr, data_wr);
        }

#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern[%u]=0x%08X - Read/verify phase\n", (unsigned)j, data_wr);
#endif
        /* Read/verify phase */
        for (i = 0U; i < (unsigned int)CNT; i++)
        {
            addr = addr_array[i];
            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skipping verify at i=%u (skip_array==1)\n", (unsigned)i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skipping verify at i=%u (write_mask==0)\n", (unsigned)i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skipping verify at i=%u (read_mask==0)\n", (unsigned)i);
#endif
                continue;
            }

            data_rd = read_reg(addr);
            wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n    & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val)
            {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WR-FAIL] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X pattern=0x%08X\n",
                       (unsigned)i, addr, data_rd, exp_val, data_wr);
#endif
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Summary: def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    /* Final decision per Acceptance Criteria */
    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U))
    {
        finish(1);
    }
    else
    {
        finish(0);
    }

    return 0;
}
