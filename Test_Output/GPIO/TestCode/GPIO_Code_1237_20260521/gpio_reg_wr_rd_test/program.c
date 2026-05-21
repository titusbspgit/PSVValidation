// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

/*
 * Auto-generated test program for: gpio_reg_wr_rd_test
 * This file includes only test_define.c as mandated.
 */

#include "test_define.c"

/* -------------------------------------------------------------------------
 * Function: main
 * Purpose : Execute the Meta Test Steps / Procedure exactly in sequence.
 * Notes   :
 *  - Uses arrays and masks defined in test_define.c.
 *  - Iterates strictly over available register entries to avoid OOB.
 *  - Validation per Meta Acceptance Criteria.
 * ------------------------------------------------------------------------- */
int main(void)
{
    unsigned int def_fail_cnt = 0U;
    unsigned int wr_fail_cnt  = 0U;

    /* Determine number of impacted registers from arrays */
    const unsigned int REG_COUNT = (unsigned int)(sizeof(addr_array) / sizeof(addr_array[0]));

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] gpio_reg_wr_rd_test: REG_COUNT=%u\n", REG_COUNT);
#endif

    /* A) Default check loop */
    for (unsigned int i = 0U; i < REG_COUNT; ++i)
    {
        /* if (skip_rst_array[i]==1) continue; */
        if (skip_rst_array[i] == 1U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] DefaultCheck: Skipping index %u due to skip_rst_array\n", i);
#endif
            continue;
        }

        /* if (read_mask_array[i]==0x00000000) continue; */
        if (read_mask_array[i] == 0x00000000U)
        {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] DefaultCheck: Skipping index %u due to read_mask=0\n", i);
#endif
            continue;
        }

        /* data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); */
        unsigned int data_rd = read_reg(addr_array[i]);
        unsigned int data    = (data_rd & 0xFFFFFFFEU);

        /* if (data==default_value_array[i]) pass; else {def_fail_cnt++; log mismatch} */
        if (data != default_value_array[i])
        {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF-MISMATCH] i=%u addr=0x%08X rd=0x%08X cmp=0x%08X exp=0x%08X\n",
                   i, addr_array[i], data_rd, data, default_value_array[i]);
#endif
        }
#ifdef DEBUG_DISPLAY
        else
        {
            printf("[DEBUG][DEF-OK] i=%u addr=0x%08X val=0x%08X\n", i, addr_array[i], data);
        }
#endif
    }

    /* B) Read/Write check for each pattern in chk_val[] */
    const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int p = 0U; p < 6U; ++p)
    {
        const unsigned int pattern = chk_val[p];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %u: 0x%08X\n", p, pattern);
#endif

        /* 1) Write phase */
        for (unsigned int i = 0U; i < REG_COUNT; ++i)
        {
            /* if (skip_array[i]==1) continue; */
            if (skip_array[i] == 1U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] WritePhase: Skipping index %u due to skip_array\n", i);
#endif
                continue;
            }

            /* if (write_mask_array[i]==0x00000000) continue; */
            if (write_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] WritePhase: Skipping index %u due to write_mask=0\n", i);
#endif
                continue;
            }

            /* write_reg(addr, (pattern & write_mask_array[i])) */
            unsigned int wr_val = (pattern & write_mask_array[i]);
            write_reg(addr_array[i], wr_val);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] WritePhase: i=%u addr=0x%08X wr=0x%08X\n", i, addr_array[i], wr_val);
#endif
        }

        /* 2) Read/verify phase */
        for (unsigned int i = 0U; i < REG_COUNT; ++i)
        {
            /* if (skip_array[i]==1) continue; */
            if (skip_array[i] == 1U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] VerifyPhase: Skipping index %u due to skip_array\n", i);
#endif
                continue;
            }

            /* if (write_mask_array[i]==0x00000000) continue; */
            if (write_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] VerifyPhase: Skipping index %u due to write_mask=0\n", i);
#endif
                continue;
            }

            /* if (read_mask_array[i]==0x00000000) continue; */
            if (read_mask_array[i] == 0x00000000U)
            {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] VerifyPhase: Skipping index %u due to read_mask=0\n", i);
#endif
                continue;
            }

            /* data_rd = (read_reg(addr) & read_mask_array[i]); */
            unsigned int data_rd = (read_reg(addr_array[i]) & read_mask_array[i]);

            /* wr_n = (write_mask_array[i] ^ 0xFFFFFFFF); */
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);

            /* exp_val = ((pattern & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); */
            unsigned int exp_val = ((pattern & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val)
            {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][RW-MISMATCH] i=%u addr=0x%08X rd=0x%08X exp=0x%08X pat=0x%08X rm=0x%08X wm=0x%08X def=0x%08X\n",
                       i, addr_array[i], data_rd, exp_val, pattern,
                       read_mask_array[i], write_mask_array[i], default_value_array[i]);
#endif
            }
#ifdef DEBUG_DISPLAY
            else
            {
                printf("[DEBUG][RW-OK] i=%u addr=0x%08X val=0x%08X\n", i, addr_array[i], data_rd);
            }
#endif
        }
    }

    /* C) Completion: finish(1) on any failure, else finish(0) */
    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U))
    {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Test Result: FAIL (def_fail_cnt=%u, wr_fail_cnt=%u)\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return 0;
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Test Result: PASS (def_fail_cnt=%u, wr_fail_cnt=%u)\n", def_fail_cnt, wr_fail_cnt);
#endif
    finish(0);
    return 0;
}
