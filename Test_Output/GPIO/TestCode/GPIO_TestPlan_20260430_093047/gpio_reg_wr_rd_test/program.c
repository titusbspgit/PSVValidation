// Author - AI Force 1.3.2. Date 30-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Testcase: gpio_reg_wr_rd_test
 * Description: Verifies default reset values and masked write/read behavior across a defined set of GPIO registers.
 */

/* Banner: Function prototypes */
static int chk_rst_val(void);
static int chk_rd_wr(void);

/*
 * Function: chk_rst_val
 * Purpose: Implements step 1 from Hidden_Test_Steps_Procedure. Reads each impacted register,
 *          applies read mask and compares (data_rd & 0xfffffffe) against default_value_array.
 */
static int chk_rst_val(void)
{
    int def_fail_cnt = 0;
    for (int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1) { continue; }
        if (read_mask_array[i] == 0x00000000) { continue; }
        unsigned int data_rd = read_reg(addr); /* read register */
        unsigned int data = (data_rd & 0xfffffffeu); /* mask off bit0 per steps */
        if (data != default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[DEF_MISMATCH] idx=%d addr=0x%08lx got=0x%08x exp=0x%08x\n", i, addr, data, default_value_array[i]);
#endif
            def_fail_cnt++;
        }
    }
    return def_fail_cnt;
}

/*
 * Function: chk_rd_wr
 * Purpose: Implements step 2 from Hidden_Test_Steps_Procedure. Writes patterns with write masks
 *          and validates readback using read masks and preserved default values for non-writable bits.
 */
static int chk_rd_wr(void)
{
    int wr_fail_cnt = 0;
    for (int j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
        /* Phase WRITE */
        for (int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1) { continue; }
            if (write_mask_array[i] == 0x00000000) { continue; }
            write_reg(addr, (data_wr & write_mask_array[i]));
        }
        /* Phase READ/COMPARE */
        for (int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1) { continue; }
            if (write_mask_array[i] == 0x00000000) { continue; }
            if (read_mask_array[i] == 0x00000000) { continue; }
            unsigned int data_rd = read_reg(addr) & read_mask_array[i];
            unsigned int wr_n = (write_mask_array[i] ^ 0xffffffffu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[WR_MISMATCH] idx=%d addr=0x%08lx got=0x%08x exp=0x%08x patt=0x%08x wmask=0x%08x rmask=0x%08x def=0x%08x\n",
                       i, addr, data_rd, exp_val, data_wr, write_mask_array[i], read_mask_array[i], default_value_array[i]);
#endif
                wr_fail_cnt++;
            }
        }
    }
    return wr_fail_cnt;
}

/*
 * Function: test_case
 * Purpose: Top-level execution that follows step 3 acceptance logic strictly.
 */
int test_case(void)
{
    int def_fail_cnt = chk_rst_val();
    int wr_fail_cnt  = chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
    return 0; /* unreachable in most harnesses after finish() */
}
