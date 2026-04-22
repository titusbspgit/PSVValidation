/*
Hidden_Test_Description:
program.c implements test_case(): calls chk_rst_val() then chk_rd_wr(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). chk_rst_val(): for i in [0..CNT-1], addr=addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); compare with default_value_array[i]; else def_fail_cnt++. chk_rd_wr(): for j over chk_val[]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}: data_wr=chk_val[j]; write loop: for i, if (skip_array[i]) continue; if (write_mask_array[i]==0) continue; else write_reg(addr_array[i], (data_wr & write_mask_array[i])); read/compare loop: for i, if (skip_array[i]) continue; if (write_mask_array[i]==0 || read_mask_array[i]==0) continue; data_rd=(read_reg(addr_array[i]) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare; else wr_fail_cnt++. soft_reset_chk() is #ifdef 0 and not executed.
*/

#include "test_define.c"

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt = 0;

static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
            continue; /* skipped by skip_rst_array */
        }
        if (read_mask_array[i] == 0x00000000U) {
            continue; /* no readable bits */
        }
        unsigned int data_rd = read_reg(addr); /* read register */
        unsigned int data = (data_rd & 0xFFFFFFFEU); /* mask as per test description */
        if (data != default_value_array[i]) {
            def_fail_cnt++;
        }
    }
}

static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
        /* Write phase */
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* skipped register */
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue; /* nothing to write */
            }
            unsigned long addr = addr_array[i];
            unsigned int wrval = (data_wr & write_mask_array[i]);
            write_reg(addr, wrval); /* write masked value */
        }
        /* Read and compare phase */
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) {
                continue; /* skipped register */
            }
            if ((write_mask_array[i] == 0x00000000U) || (read_mask_array[i] == 0x00000000U)) {
                continue; /* either not writable or not readable */
            }
            unsigned long addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
            }
        }
    }
}

int test_case(void)
{
    def_fail_cnt = 0U;
    wr_fail_cnt = 0U;

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
    return 0;
}
