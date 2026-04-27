// Author - AI Force 1.3.2. Date 27-04-2026
// (EMBENGG-SYSAPPS)

/* High-level description (verbatim from metadata):
   program.c includes test_define.c and runs: chk_rst_val(); chk_rd_wr(); then if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). chk_rst_val(): for i in 0..CNT-1, addr=addr_array[i]; if read_mask_array[i]==0 skip; data_rd=read_reg(addr); compare to default_value_array[i]; if mismatch def_fail_cnt++. chk_rd_wr(): for six patterns in chk_val[], write each to all addresses with write_mask!=0 and not skipped; then read back for those with read_mask!=0 and write_mask!=0 and not skipped; compute exp_val=((data_wr & read_mask & write_mask) | ((~write_mask) & read_mask & default_value)); compare and increment wr_fail_cnt on mismatch.
*/

/* Only include test_define.c as mandated */
#include "test_define.c"

/* External platform/testbench APIs expected from headers included via test_define.c */
extern unsigned int read_reg(unsigned int addr);
extern void write_reg(unsigned int addr, unsigned int val);
extern void finish(unsigned int status);

/* Local counters as per metadata */
static unsigned int def_fail_cnt = 0U;
static unsigned int wr_fail_cnt  = 0U;

/*
 * Function: chk_rst_val
 * Purpose: For each register with non-zero read mask, read the register and
 *          compare to the expected default value. Increment def_fail_cnt on
 *          mismatch. Skips entries with read_mask == 0 as specified.
 */
static void chk_rst_val(void)
{
    for (unsigned int i = 0U; i < (unsigned int)CNT; i++)
    {
        unsigned int addr = (unsigned int)addr_array[i];
        if (read_mask_array[i] == 0U)
        {
            continue;
        }
        unsigned int data_rd = read_reg(addr);
        if (data_rd == default_value_array[i])
        {
#ifdef DEBUG_DISPLAY
            printf("[spi_reg_wr_rd_test][RST] PASS @%u: addr=0x%08X val=0x%08X\n", i, addr, data_rd);
#endif
        }
        else
        {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[spi_reg_wr_rd_test][RST] FAIL @%u: addr=0x%08X rd=0x%08X exp=0x%08X\n", i, addr, data_rd, default_value_array[i]);
#endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose: For each of six data patterns, write to all registers that are
 *          writable and not skipped, then read back from registers that are
 *          both readable and writable and not skipped. The expected value is
 *          computed from writable/readable masks and defaults, and mismatches
 *          increment wr_fail_cnt.
 */
static void chk_rd_wr(void)
{
    for (unsigned int j = 0U; j < 6U; j++)
    {
        unsigned int data_wr = chk_val[j];

        /* Write phase */
        for (unsigned int i = 0U; i < (unsigned int)CNT; i++)
        {
            if (skip_array[i] == 1U)
            {
                continue;
            }
            if (write_mask_array[i] == 0U)
            {
                continue;
            }
            unsigned int addr = (unsigned int)addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[spi_reg_wr_rd_test][WR ] pat=%u @%u: addr=0x%08X data=0x%08X\n", j, i, addr, data_wr);
#endif
        }

        /* Read/verify phase */
        for (unsigned int i = 0U; i < (unsigned int)CNT; i++)
        {
            if (skip_array[i] == 1U)
            {
                continue;
            }
            if (write_mask_array[i] == 0U)
            {
                continue;
            }
            if (read_mask_array[i] == 0U)
            {
                continue;
            }

            unsigned int addr   = (unsigned int)addr_array[i];
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n    & read_mask_array[i] & default_value_array[i]));

            if (data_rd == exp_val)
            {
#ifdef DEBUG_DISPLAY
                printf("[spi_reg_wr_rd_test][VER] pat=%u @%u: addr=0x%08X PASS rd=0x%08X exp=0x%08X\n", j, i, addr, data_rd, exp_val);
#endif
            }
            else
            {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[spi_reg_wr_rd_test][VER] pat=%u @%u: addr=0x%08X FAIL rd=0x%08X exp=0x%08X\n", j, i, addr, data_rd, exp_val);
#endif
            }
        }
    }
}

/*
 * Function: test_case
 * Purpose: Execute reset-value and read/write checks and conclude per the
 *          acceptance criteria: PASS only if both counters remain zero.
 */
void test_case(void)
{
    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt == 0U) && (wr_fail_cnt == 0U))
    {
#ifdef DEBUG_DISPLAY
        printf("[spi_reg_wr_rd_test] PASS def_fail_cnt=0 wr_fail_cnt=0\n");
#endif
        finish(0U); /* PASS */
    }
    else
    {
#ifdef DEBUG_DISPLAY
        printf("[spi_reg_wr_rd_test] FAIL def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1U); /* FAIL */
    }
}
