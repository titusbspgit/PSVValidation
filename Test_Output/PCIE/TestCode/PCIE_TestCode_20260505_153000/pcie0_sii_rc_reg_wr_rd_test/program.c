// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

// Include the complete testcase context and arrays
#include "test_define.c"

// Forward declarations
static void chk_rst_val(void);
static void chk_rd_wr(void);

// Banner: Test intent and high-level description (from Hidden_Test_Description)
// Test validates reset default values and masked write-read behavior for PCIe0 SII RC registers using table-driven arrays.
// It iterates over 153 addresses from addr_array[153], with corresponding default_value_array, read_mask_array,
// write_mask_array, and skip_array defined in test_define.c.
// Execution: test_case() -> chk_rst_val() -> chk_rd_wr() -> finish().

// Function: test_case
// Purpose: Entry point for the testcase. Executes default reset value checks and masked write-read checks.
//          Aggregates failure counters and reports final PASS/FAIL using finish() strictly per acceptance criteria.
void test_case(void)
{
    // Default value verification across readable registers (with specified skip rules for defaults)
    chk_rst_val();
#ifdef DEBUG_DISPLAY
    printf("********* Default value check end ***\n");
#endif

    // Masked write-read verification over all applicable registers and patterns
    chk_rd_wr();
#ifdef DEBUG_DISPLAY
    printf(" Write & Read from registers end ***\n");
#endif

    // Final result strictly as per acceptance criteria
    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0))
    {
        finish(1); // FAIL
    }
    else
    {
        finish(0); // PASS
    }
}

// Function: chk_rst_val
// Purpose: Iterate all addresses and validate that readable registers return their defined default values.
//          Skips registers with read_mask == 0, and also skips mizar_PCIE0_SII_PHY_RST_CONTROL in default check only.
static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++)
    {
        unsigned long int addr = addr_array[i];
        int rd_mask = read_mask_array[i];

        // Skip if not readable
        if (rd_mask == 0x00000000)
        {
#ifdef DEBUG_DISPLAY
            printf("RST : This address 0x%x is not readable, hence skipped for reading\n", (unsigned int)addr);
#endif
            continue;
        }

        // Skip default check for the specified register only
        if (addr == mizar_PCIE0_SII_PHY_RST_CONTROL)
        {
            continue;
        }

        // Perform read and compare with default value
        data_rd = read_reg(addr);
        if (data_rd == default_value_array[i])
        {
#ifdef DEBUG_DISPLAY
            printf("RST : PASS Reading Default value from Address :0x%x Expected : 0x%x\tRead_data : 0x%x\n",
                   (unsigned int)addr, (unsigned int)default_value_array[i], (unsigned int)data_rd);
#endif
        }
        else
        {
            def_fail_cnt++;
            printf("RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\tRead_data : 0x%x\n",
                   (unsigned int)addr, (unsigned int)default_value_array[i], (unsigned int)data_rd);
        }
    }
}

// Function: chk_rd_wr
// Purpose: For each write pattern, write to all applicable registers and verify readback against masked expectation.
//          Adheres strictly to skip_array, read_mask_array, and write_mask_array semantics.
static void chk_rd_wr(void)
{
    int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};

    for (int j = 0; j < 6; j++)
    {
        data_wr = chk_val[j];

        // Write phase
        for (int i = 0; i < CNT; i++)
        {
            unsigned long int addr = addr_array[i];
            int wr_mask = write_mask_array[i];

            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : Writing into this Address : 0x%x is skipped because address present in skip_array\n",
                       (unsigned int)addr);
#endif
                continue;
            }

            if (wr_mask == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : This address 0x%x is not writable, hence skipped for writing\n",
                       (unsigned int)addr);
#endif
                continue;
            }

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("Read_write : Writing into register Address : 0x%x\tdata :0x%x\n",
                   (unsigned int)addr, (unsigned int)data_wr);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++)
        {
            unsigned long int addr = addr_array[i];
            int wr_mask = write_mask_array[i];
            int rd_mask = read_mask_array[i];

            if (skip_array[i] == 1)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : Reading from this Address : 0x%x is skipped because address present in skip_array\n",
                       (unsigned int)addr);
#endif
                continue;
            }

            if (wr_mask == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : This address 0x%x is not Writable , hence skipped for reading\n",
                       (unsigned int)addr);
#endif
                continue;
            }

            if (rd_mask == 0x00000000)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : This address 0x%x is not Readable , hence skipped for reading\n",
                       (unsigned int)addr);
#endif
                continue;
            }

            data_rd = read_reg(addr);
            int wr_n   = (wr_mask ^ 0xffffffff);
            int exp_val = ((data_wr & rd_mask & wr_mask) | (wr_n & rd_mask & default_value_array[i]));

            if (data_rd == exp_val)
            {
#ifdef DEBUG_DISPLAY
                printf("Read_write : PASS : For Address %x, Expected value=0x%x\tRead value=0x%x\n",
                       (unsigned int)addr, (unsigned int)exp_val, (unsigned int)data_rd);
#endif
            }
            else
            {
                wr_fail_cnt++;
                printf("Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\tRead value=0x%x\n",
                       (unsigned int)addr, (unsigned int)exp_val, (unsigned int)data_rd);
            }
        }
    }
}
