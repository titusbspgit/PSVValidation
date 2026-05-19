// Author - AI Force 1.3.2. Date 19-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Perform default value checks on readable registers
// Notes   : Skips addresses per Meta Steps and entries with read_mask == 0
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    unsigned int i;
    for (i = 0; i < CNT; i++) {
        unsigned int addr = addr_array[i];
        if (read_mask_array[i] == 0U) {
            // Skip entries with no readable bits
            continue;
        }
        if (addr == mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG ||
            addr == mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS ||
            addr == mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF) {
            // Explicitly skipped for default-value check
            continue;
        }

        unsigned int data_rd = read_reg(addr);
        if (data_rd != default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("DEFCHK FAIL: idx=%u addr=0x%08X exp=0x%08X rd=0x%08X\n", i, addr, default_value_array[i], data_rd);
#endif
            extern volatile unsigned int def_fail_cnt; // defined below
            def_fail_cnt++;
        } else {
#ifdef DEBUG_DISPLAY
            printf("DEFCHK PASS: idx=%u addr=0x%08X val=0x%08X\n", i, addr, data_rd);
#endif
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Write patterns to writable registers and verify readback
// Notes   : Uses masks as specified in Meta Steps
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    unsigned int i, j;
    unsigned int chk_val[6] = { 0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U };

    for (j = 0U; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("WRITE PHASE: pattern[%u]=0x%08X\n", j, data_wr);
#endif
        // Write phase
        for (i = 0U; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1U) {
                continue; // skip per skip_array
            }
            if (write_mask_array[i] == 0U) {
                continue; // not writable
            }
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("WR: idx=%u addr=0x%08X data=0x%08X\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
#ifdef DEBUG_DISPLAY
        printf("READ PHASE: pattern[%u]=0x%08X\n", j, data_wr);
#endif
        for (i = 0U; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1U) {
                continue; // skip per skip_array
            }
            if (write_mask_array[i] == 0U) {
                continue; // not writable -> no expectation
            }
            if (read_mask_array[i] == 0U) {
                continue; // not readable -> cannot verify
            }

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("WRCHK FAIL: idx=%u addr=0x%08X exp=0x%08X rd=0x%08X wrmask=0x%08X rmask=0x%08X\n",
                       i, addr, exp_val, data_rd, write_mask_array[i], read_mask_array[i]);
#endif
            
                extern volatile unsigned int wr_fail_cnt; // defined below
                wr_fail_cnt++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("WRCHK PASS: idx=%u addr=0x%08X val=0x%08X\n", i, addr, data_rd);
#endif
            }
        }
    }
}

// ------------------------------------------------------------
// Function: soft_reset_chk
// Purpose : Perform a soft reset write/restore sequence (not invoked)
// ------------------------------------------------------------
static void soft_reset_chk(void)
{
    unsigned int default_value = read_reg(SOFT_RST_REG_ADDRESS);
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(1000);
    write_reg(SOFT_RST_REG_ADDRESS, default_value);
    wait_on(1000);
}

// Error counters
volatile unsigned int def_fail_cnt = 0U;
volatile unsigned int wr_fail_cnt  = 0U;

// ------------------------------------------------------------
// Function: test_case
// Purpose : Main test entry - executes reset check and read/write check
// ------------------------------------------------------------
void test_case(void)
{
    def_fail_cnt = 0U;
    wr_fail_cnt  = 0U;

#ifdef DEBUG_DISPLAY
    printf("Starting chk_rst_val()\n");
#endif
    chk_rst_val();

#ifdef DEBUG_DISPLAY
    printf("Starting chk_rd_wr()\n");
#endif
    chk_rd_wr();

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
#ifdef DEBUG_DISPLAY
        printf("TEST FAIL: def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("TEST PASS\n");
#endif
        finish(0);
    }
}
