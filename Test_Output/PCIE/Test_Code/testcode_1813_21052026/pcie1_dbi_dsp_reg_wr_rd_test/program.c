// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

/*
 * Auto-generated program for test: pcie1_dbi_dsp_reg_wr_rd_test
 * This file converts Meta Test Steps into executable C logic without reordering
 * or optimization. Uses only impacted registers and arrays declared in test_define.c.
 */

#include "test_define.c"  /* Only include as per rules */

/* Forward declarations */
static void chk_rst_val(void);
static void chk_rd_wr(void);
static void soft_reset_chk(void); /* helper, not invoked */

/* Local state */
static int def_fail_cnt = 0;  /* default-value check failures */
static int wr_fail_cnt  = 0;  /* write/readback check failures */

/*
 * Function: chk_rst_val
 * Purpose: Phase 1 - Check reset/default values of readable registers.
 */
static void chk_rst_val(void)
{
    int i;
    unsigned long addr;
    unsigned int data_rd;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rst_val()\n");
#endif

    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        /* Skip if no readable bits */
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] idx=%d addr[%d]=0x%lx -> skip (read_mask==0)\n", i, i, addr);
#endif
            continue;
        }

        /* Skip default check for specific addresses per Meta Steps */
        if (addr == mizar_PCIE1_DBI_DSP CAP ID NXT PTR REG ||
            addr == mizar_PCIE1_DBI_DSP DEVICE CONTROL DEVICE STATUS ||
            addr == mizar_PCIE1_DBI_DSP PL DEBUG1 OFF) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] idx=%d addr=0x%lx -> skip default check (special)\n", i, addr);
#endif
            continue;
        }

        /* Read and compare against default value */
        data_rd = read_reg(addr);
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] R i=%d addr=0x%08lx data_rd=0x%08x exp_def=0x%08x\n", i, addr, data_rd, (unsigned int)default_value_array[i]);
#endif
        if (data_rd != (unsigned int)default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEBUG][DEF_MISMATCH] i=%d addr=0x%08lx rd=0x%08x exp=0x%08x def_fail_cnt=%d\n",
                   i, addr, data_rd, (unsigned int)default_value_array[i], def_fail_cnt);
#endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose: Phase 2 - Masked write followed by masked read/verify.
 */
static void chk_rd_wr(void)
{
    int i, p;
    unsigned long addr;
    unsigned int data_rd;
    unsigned int data_wr;
    unsigned int wr_n;
    unsigned int exp_val;

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rd_wr()\n");
#endif

    /* Iterate over all write patterns */
    for (p = 0; p < 6; p++) {
        data_wr = (unsigned int)chk_val[p];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern[%d]=0x%08x -> write pass\n", p, data_wr);
#endif
        /* Write pass */
        for (i = 0; i < CNT; i++) {
            addr = addr_array[i];

            if (skip_array[i] == 1)
                continue; /* skip explicitly flagged registers */
            if (write_mask_array[i] == 0)
                continue; /* skip non-writable registers */

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] W i=%d addr=0x%08lx wr=0x%08x\n", i, addr, data_wr);
#endif
        }

#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern[%d]=0x%08x -> read/verify pass\n", p, data_wr);
#endif
        /* Read/verify pass */
        for (i = 0; i < CNT; i++) {
            addr = addr_array[i];

            if (skip_array[i] == 1)
                continue; /* skip explicitly flagged registers */
            if (write_mask_array[i] == 0)
                continue; /* no write -> nothing to verify */
            if (read_mask_array[i] == 0)
                continue; /* not readable -> skip */

            data_rd = read_reg(addr);
            wr_n = (unsigned int)(write_mask_array[i] ^ 0xffffffffu);
            exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                       (wr_n    & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] V i=%d addr=0x%08lx rd=0x%08x exp=0x%08x rm=0x%08x wm=0x%08x def=0x%08x\n",
                   i, addr, data_rd, exp_val,
                   (unsigned int)read_mask_array[i], (unsigned int)write_mask_array[i], (unsigned int)default_value_array[i]);
#endif
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][WR_MISMATCH] i=%d addr=0x%08lx rd=0x%08x exp=0x%08x wr_fail_cnt=%d\n",
                       i, addr, data_rd, exp_val, wr_fail_cnt);
#endif
            }
        }
    }
}

/*
 * Function: soft_reset_chk
 * Purpose: Helper to exercise soft reset write/restore. Not invoked in test flow.
 */
static void soft_reset_chk(void)
{
    unsigned int rst_default;
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter soft_reset_chk()\n");
#endif
    rst_default = read_reg(SOFT_RST REG ADDRESS);
    write_reg(SOFT_RST REG ADDRESS, SOFT_RST REG DATA);
    wait_on(1000);
    write_reg(SOFT_RST REG ADDRESS, rst_default);
    wait_on(1000);
}

/*
 * Entry point: test_case
 */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] >>> Enter test_case: pcie1_dbi_dsp_reg_wr_rd_test <<<\n");
#endif

    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST FAIL def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1); /* FAIL */
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] TEST PASS\n");
#endif
        finish(0); /* PASS */
    }

    return 0; /* Unreached if finish() terminates, retained for formality */
}
