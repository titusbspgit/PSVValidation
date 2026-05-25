// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Default value verification over addr_array
// Notes   : Skips entries with read_mask == 0 and PHY RST control
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    unsigned long int addr = 0U;      // Register address placeholder
    unsigned int data_rd = 0U;         // Read data

    for (int i = 0; i < CNT; i++)
    {
        addr = addr_array[i];

        // Skip if read mask is 0
        if (read_mask_array[i] == 0x00000000)
        {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%d skipped (read_mask==0)\n", i);
#endif
            continue;
        }

        // Skip if this is the PHY reset control register
        if (addr_array[i] == mizar_PCIE0_SII_PHY_RST_CONTROL)
        {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: i=%d skipped (PHY_RST_CONTROL)\n", i);
#endif
            continue;
        }

        // Perform the read
        data_rd = read_reg(addr);

#ifdef DEBUG_DISPLAY
        printf("[DBG] chk_rst_val: i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, (unsigned int)default_value_array[i]);
#endif

        // Compare with expected default value
        if (data_rd != (unsigned int)default_value_array[i])
        {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Default mismatch @i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, (unsigned int)default_value_array[i]);
#endif
            extern int def_fail_cnt; // Counter defined below
            def_fail_cnt++;
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Write/read-back verification for fixed patterns
// Notes   : Applies write/read masks as specified
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    unsigned long int addr = 0U;     // Register address placeholder
    unsigned int data_rd = 0U;        // Read data
    unsigned int data_wr = 0U;        // Write data
    unsigned int wr_n = 0U;           // Inverted write mask
    unsigned int exp_val = 0U;        // Expected value after write

    const unsigned int chk_val[6] = {0xffffffffU, 0xaaaaaaaaU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xffff0000U};

    for (int j = 0; j < 6; j++)
    {
        data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] chk_rd_wr: pattern[%d]=0x%08x\n", j, data_wr);
#endif

        // Write phase
        for (int i = 0; i < CNT; i++)
        {
            addr = addr_array[i];

            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write-skip: i=%d (skip_array)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("[DBG] write-skip: i=%d (write_mask==0)\n", i);
#endif
                continue;
            }

#ifdef DEBUG_DISPLAY
            printf("[DBG] write: i=%d addr=0x%08lx data_wr=0x%08x\n", i, addr, data_wr);
#endif
            write_reg(addr, data_wr);
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++)
        {
            addr = addr_array[i];

            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (skip_array)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (write_mask==0)\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("[DBG] read-skip: i=%d (read_mask==0)\n", i);
#endif
                continue;
            }

            data_rd = read_reg(addr);
            wr_n = (write_mask_array[i] ^ 0xffffffffU);
            exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                       (wr_n & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));

#ifdef DEBUG_DISPLAY
            printf("[DBG] verify: i=%d addr=0x%08lx rd=0x%08x exp=0x%08x rm=0x%08x wm=0x%08x\n",
                   i, addr, data_rd, exp_val, (unsigned int)read_mask_array[i], (unsigned int)write_mask_array[i]);
#endif

            if (data_rd != exp_val)
            {
#ifdef DEBUG_DISPLAY
                printf("[ERR] WR/RD mismatch @i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, exp_val);
#endif
            	extern int wr_fail_cnt; // Counter defined below
                wr_fail_cnt++;
            }
        }
    }
}

// Global counters for failures (visible to helper functions)
int def_fail_cnt = 0;
int wr_fail_cnt = 0;

// ------------------------------------------------------------
// Function: test_case (Entry Point)
// Purpose : Executes default and write/read-back checks per Meta Steps
// Result  : finish(0) on PASS; finish(1) on FAIL
// ------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] test_case: start\n");
#endif

    // Initialize counters
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    // Execute phases in order
    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[DBG] test_case: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    // Final decision per acceptance criteria
    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0))
    {
        finish(1); // FAIL
    }
    else
    {
        finish(0); // PASS
    }

    return 0; // Not used by framework
}
