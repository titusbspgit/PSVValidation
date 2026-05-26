// Author - AI Force 1.3.2. Date 26-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// Banner: Function to check reset/default values of GPIO registers
static void chk_rst_val(void);

// Banner: Function to perform write/readback checks on GPIO registers with multiple patterns
static void chk_rd_wr(void);

// Internal counters for failures
static unsigned int def_fail_cnt = 0; // default value check failures
static unsigned int wr_fail_cnt  = 0; // write/readback check failures

// Compute number of registers from arrays defined in test_define.c
static const unsigned int REG_COUNT = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));

// -----------------------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Validate default reset values for all non-skipped, readable registers
// Notes   : Bit0 is ignored during default check as per meta (mask 0xFFFFFFFE)
// -----------------------------------------------------------------------------
static void chk_rst_val(void)
{
    unsigned int i;
    for (i = 0U; i < CNT; i++) {
        if (i >= REG_COUNT) {
            break; // Guard against CNT > actual array size
        }

        unsigned long int addr = addr_array[i];

        // Skip if register is marked to skip in reset check
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skipping index %u (addr 0x%08lX) due to skip_rst_array\n", i, addr);
#endif
            continue;
        }

        // Skip if register is not readable
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DBG][RST] Skipping index %u (addr 0x%08lX) due to read_mask=0\n", i, addr);
#endif
            continue;
        }

        // Read and mask out bit0 as per requirement
        unsigned int data_rd = read_reg(addr);
        unsigned int data_m  = (data_rd & 0xFFFFFFFEU);
        unsigned int exp_val = default_value_array[i];

        if (data_m != exp_val) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR][RST] Mismatch @0x%08lX idx=%u exp=0x%08X rd=0x%08X\n", addr, i, exp_val, data_m);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[OK ][RST] Match   @0x%08lX idx=%u val=0x%08X\n", addr, i, data_m);
#endif
        }
    }
#ifdef DEBUG_DISPLAY
    printf("[DBG][RST] Completed reset/default checks. def_fail_cnt=%u\n", def_fail_cnt);
#endif
}

// -----------------------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Perform write/readback validation using specified patterns
// Notes   : Only non-skipped, writable (write_mask!=0) registers are written.
//           For read/verify, register must also be readable (read_mask!=0).
// -----------------------------------------------------------------------------
static void chk_rd_wr(void)
{
    static const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAA AU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int j = 0U; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG][WR ] Pattern %u: 0x%08X\n", j, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0U; i < CNT; i++) {
            if (i >= REG_COUNT) {
                break; // Guard against CNT > actual array size
            }

            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR ] Skip write idx=%u\n", i);
#endif
                continue;
            }

            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DBG][WR ] Not writable idx=%u (write_mask=0)\n", i);
#endif
                continue;
            }

            unsigned long int addr = addr_array[i];
            unsigned int wr_data = (data_wr & write_mask_array[i]);
            write_reg(addr, wr_data);
#ifdef DEBUG_DISPLAY
            printf("[DBG][WR ] Wrote 0x%08X to 0x%08lX (idx=%u)\n", wr_data, addr, i);
#endif
        }

        // Read/verify phase
        for (unsigned int i = 0U; i < CNT; i++) {
            if (i >= REG_COUNT) {
                break; // Guard against CNT > actual array size
            }

            if (skip_array[i] == 1U) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue; // Not writable -> no change expected/verified per steps
            }
            if (read_mask_array[i] == 0x00000000U) {
                continue; // Not readable -> skip verification
            }

            unsigned long int addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n    & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR][VR ] Mismatch @0x%08lX idx=%u exp=0x%08X rd=0x%08X\n", addr, i, exp_val, data_rd);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[OK ][VR ] Match   @0x%08lX idx=%u val=0x%08X\n", addr, i, data_rd);
#endif
            }
        }
    }
#ifdef DEBUG_DISPLAY
    printf("[DBG][WR ] Completed write/readback checks. wr_fail_cnt=%u\n", wr_fail_cnt);
#endif
}

// -----------------------------------------------------------------------------
// Entry Point: test_case
// Purpose    : Execute the test flow and report PASS/FAIL
// Terminate  : finish(0) on PASS; finish(1) on FAIL (no alternate termination)
// -----------------------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DBG] Starting test_case: gpio_reg_wr_rd_test\n");
#endif

    chk_rst_val();
#ifdef DEBUG_DISPLAY
    printf("[DBG] Completed chk_rst_val()\n");
#endif

    chk_rd_wr();
#ifdef DEBUG_DISPLAY
    printf("[DBG] Completed chk_rd_wr()\n");
    printf("[DBG] def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("[RES] TEST FAIL\n");
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RES] TEST PASS\n");
#endif
        finish(0);
    }
}
