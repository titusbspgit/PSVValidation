// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

// ------------------------------------------------------------
// Function: chk_rst_val
// Purpose : Verify default/reset values per Meta Test Steps
// ------------------------------------------------------------
static void chk_rst_val(void)
{
    unsigned int i;
    // Determine safe iteration count using CNT and actual array size
    const unsigned int array_cnt = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));
    const unsigned int cnt = (CNT < array_cnt) ? CNT : array_cnt;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rst_val: cnt=%u\n", cnt);
#endif

    for (i = 0; i < cnt; i++) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: Skipping index %u due to skip_rst_array\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: Skipping index %u due to read_mask==0\n", i);
#endif
            continue;
        }

        unsigned long addr = addr_array[i];
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU); // Ignore bit[0] per meta
        unsigned int exp  = default_value_array[i];

        if (data != exp) {
            extern int def_fail_cnt; // defined below
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[FAIL][RST] idx=%u addr=0x%08lx exp=0x%08x got=0x%08x raw=0x%08x\n",
                   i, addr, exp, data, data_rd);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[PASS][RST] idx=%u addr=0x%08lx val=0x%08x\n", i, addr, data);
#endif
        }
    }
}

// ------------------------------------------------------------
// Function: chk_rd_wr
// Purpose : Write/read integrity checks using fixed patterns
// ------------------------------------------------------------
static void chk_rd_wr(void)
{
    unsigned int i, j;
    const unsigned int array_cnt = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));
    const unsigned int cnt = (CNT < array_cnt) ? CNT : array_cnt;

    unsigned int chk_val[6] = { 0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U };

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] chk_rd_wr: cnt=%u patterns=%zu\n", cnt, sizeof(chk_val)/sizeof(chk_val[0]));
#endif

    for (j = 0; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %u: 0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (i = 0; i < cnt; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write: skip idx %u\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write: mask==0 at idx %u\n", i);
#endif
                continue;
            }
            unsigned long addr = addr_array[i];
            unsigned int wdata = (data_wr & write_mask_array[i]);
            write_reg(addr, wdata);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Write: idx=%u addr=0x%08lx wdata=0x%08x\n", i, addr, wdata);
#endif
        }

        // Read/verify phase
        for (i = 0; i < cnt; i++) {
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : skip idx %u\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : write_mask==0 at idx %u\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : read_mask==0 at idx %u\n", i);
#endif
                continue;
            }

            unsigned long addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
            	extern int wr_fail_cnt; // defined below
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[FAIL][WR] idx=%u addr=0x%08lx exp=0x%08x got=0x%08x wmask=0x%08x rmask=0x%08x def=0x%08x\n",
                       i, addr, exp_val, data_rd, write_mask_array[i], read_mask_array[i], default_value_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[PASS][WR] idx=%u addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}

#ifdef 0
// ------------------------------------------------------------
// Function: soft_reset_chk (Disabled)
// Purpose : Example soft reset write/read with waits
// ------------------------------------------------------------
static void soft_reset_chk(void)
{
    unsigned int save = read_reg(SOFT_RST_REG_ADDRESS);
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(1000);
    write_reg(SOFT_RST_REG_ADDRESS, save);
    wait_on(1000);
}
#endif

// Failure counters (global scope as per meta description)
int def_fail_cnt = 0;
int wr_fail_cnt = 0;

// ------------------------------------------------------------
// Function: test_case (ENTRY POINT)
// Purpose : Execute reset check and write/read check, report PASS/FAIL
// ------------------------------------------------------------
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] test_case: START\n");
#endif
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] FAIL: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[RESULT] PASS\n");
#endif
        finish(0);
    }

    return 0; // Control should not reach here due to finish()
}
