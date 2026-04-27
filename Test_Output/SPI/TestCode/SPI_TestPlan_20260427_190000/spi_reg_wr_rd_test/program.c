// Author - AI Force 1.3.2. Date 27-04-2026
// (EMBENGG-SYSAPPS)

/*
 High-level description (verbatim derived):
 program.c includes test_define.c and runs: chk_rst_val(); chk_rd_wr(); then if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). chk_rst_val(): for i in 0..CNT-1, addr=addr_array[i]; if read_mask_array[i]==0 skip; data_rd=read_reg(addr); compare to default_value_array[i]; if mismatch def_fail_cnt++. chk_rd_wr(): for six patterns in chk_val[], write each to all addresses with write_mask!=0 and not skipped; then read back for those with read_mask!=0 and write_mask!=0 and not skipped; compute exp_val=((data_wr & read_mask & write_mask) | ((~write_mask) & read_mask & default_value)); compare and increment wr_fail_cnt on mismatch.
*/

/* Only include the generated definitions as mandated */
#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose : Check reset values for readable registers per read_mask_array.
 */
static unsigned int chk_rst_val(void)
{
    unsigned int def_fail_cnt = 0u;
    for (unsigned int i = 0u; i < CNT; ++i)
    {
        unsigned long addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];
        if (rmask == 0u)
        {
            continue; /* not readable */
        }
        if (skip_array[i] == 1u)
        {
            continue; /* explicitly skipped */
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int def_val = default_value_array[i];
        /* mask to compare only readable bits */
        if ((data_rd & rmask) != (def_val & rmask))
        {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[spi_reg_wr_rd_test][RST] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X mask=0x%08X\n",
                   i, addr, data_rd, def_val, rmask);
#endif
        }
    }
    return def_fail_cnt;
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform write/readback tests using chk_val patterns subject to masks and skips.
 */
static unsigned int chk_rd_wr(void)
{
    unsigned int wr_fail_cnt = 0u;

    for (unsigned int j = 0u; j < 6u; ++j)
    {
        unsigned int data_wr = chk_val[j];
        /* Write phase */
        for (unsigned int i = 0u; i < CNT; ++i)
        {
            if (skip_array[i] == 1u) { continue; }
            unsigned int wmask = write_mask_array[i];
            if (wmask == 0u) { continue; }
            unsigned long addr = addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[spi_reg_wr_rd_test][WR ] idx=%u addr=0x%08lX wr=0x%08X\n", i, addr, data_wr);
#endif
        }
        /* Read/verify phase */
        for (unsigned int i = 0u; i < CNT; ++i)
        {
            if (skip_array[i] == 1u) { continue; }
            unsigned int wmask = write_mask_array[i];
            unsigned int rmask = read_mask_array[i];
            if (wmask == 0u || rmask == 0u) { continue; }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (wmask ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));
            if (data_rd != exp_val)
            {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[spi_reg_wr_rd_test][CHK] idx=%u addr=0x%08lX rd=0x%08X exp=0x%08X rmask=0x%08X wmask=0x%08X\n",
                       i, addr, data_rd, exp_val, rmask, wmask);
#endif
            }
        }
    }

    return wr_fail_cnt;
}

/*
 * Function: test_case
 * Purpose : Execute reset and RW checks; finalize per acceptance criteria.
 */
void test_case(void)
{
    unsigned int def_fail_cnt = chk_rst_val();
    unsigned int wr_fail_cnt  = chk_rd_wr();

    /* Acceptance criteria: Pass if def_fail_cnt == 0 and wr_fail_cnt == 0; otherwise fail. */
    if ((def_fail_cnt == 0u) && (wr_fail_cnt == 0u))
    {
        finish(0); /* PASS */
    }
    else
    {
        finish(1); /* FAIL */
    }
}
