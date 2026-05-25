// Author - AI Force 1.3.2. Date 25-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: test_case
 * Description:
 *   Implements the meta-defined PCIe0 DBI/DSP default check and write/read-back verification.
 *   Execution strictly follows the provided procedure without reordering or optimization.
 */
int test_case(void)
{
    unsigned int def_fail_cnt = 0;   // default value mismatch counter
    unsigned int wr_fail_cnt  = 0;   // write/read-back mismatch counter

    // Determine the number of registers available from addr_array
    const unsigned int num_regs = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] num_regs=%u\n", num_regs);
#endif

    // Default value verification loop
    for (unsigned int i = 0; i < num_regs; ++i)
    {
        unsigned long addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];

        // Skip if read_mask == 0x00000000
        if (rmask == 0x00000000U)
            continue;

        // Skip addresses specified in meta for default check
        if (addr == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG ||
            addr == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS ||
            addr == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF)
        {
            continue;
        }

        // Perform read and compare against default value
        unsigned int data_rd = read_reg(addr);
        unsigned int defval  = default_value_array[i];
        if (data_rd != defval)
        {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEFAULT_FAIL] i=%u addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, defval);
#endif
        }
#ifdef DEBUG_DISPLAY
        else
        {
            printf("[DEBUG][DEFAULT_PASS] i=%u addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
        }
#endif
    }

    // Write/read-back patterns as specified in meta
    const unsigned int chk_val[6] = {
        0xffffffffU, 0xaaaaaaaaU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xffff0000U
    };

    for (unsigned int j = 0; j < 6U; ++j)
    {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %u: 0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0; i < num_regs; ++i)
        {
            unsigned long addr = addr_array[i];
            unsigned int wmask = write_mask_array[i];
            int skip = (i < (unsigned int)(sizeof(skip_array)/sizeof(skip_array[0]))) ? skip_array[i] : 0;

            if (skip == 1)
                continue;
            if (wmask == 0x00000000U)
                continue;

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][WRITE] i=%u addr=0x%08lx data=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (unsigned int i = 0; i < num_regs; ++i)
        {
            unsigned long addr = addr_array[i];
            unsigned int rmask = read_mask_array[i];
            unsigned int wmask = write_mask_array[i];
            int skip = (i < (unsigned int)(sizeof(skip_array)/sizeof(skip_array[0]))) ? skip_array[i] : 0;

            if (skip == 1)
                continue;
            if (wmask == 0x00000000U)
                continue;
            if (rmask == 0x00000000U)
                continue;

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n    = (wmask ^ 0xffffffffU);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));

            if (data_rd != exp_val)
            {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WRRD_FAIL] i=%u addr=0x%08lx rd=0x%08x exp=0x%08x wmask=0x%08x rmask=0x%08x def=0x%08x\n",
                       i, addr, data_rd, exp_val, wmask, rmask, default_value_array[i]);
#endif
            }
#ifdef DEBUG_DISPLAY
            else
            {
                printf("[DEBUG][WRRD_PASS] i=%u addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
            }
#endif
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U))
    {
        finish(1); // FAIL
        return 1;
    }
    else
    {
        finish(0); // PASS
        return 0;
    }
}
