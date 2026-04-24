// Author - AI Force 1.3.2. Date 24-04-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
High-level description (AS-IS from Hidden_Test_Description):
program.c runs chk_rst_val() then chk_rd_wr(). chk_rst_val(): loops i=0..CNT-1; addr=addr_array[i]; if skip_rst_array[i]==1 continue; if read_mask_array[i]==0 continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); if(data==default_value_array[i]) pass else def_fail_cnt++. chk_rd_wr(): six patterns in chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; For each pattern: write phase over i=0..CNT-1: addr=addr_array[i]; if skip_array[i]==1 continue; if write_mask_array[i]==0 continue; write_reg(addr,(data_wr & write_mask_array[i])); read phase: skip if skip_array[i]==1 or write_mask_array[i]==0 or read_mask_array[i]==0; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd==exp_val) pass else wr_fail_cnt++. test_case() finishes with finish(1) if (def_fail_cnt>0 || wr_fail_cnt>0) else finish(0).
*/

/*
Function: chk_rst_val
Purpose : Verify reset/default values of impacted GPIO registers honoring read and reset-skip masks.
*/
static int chk_rst_val(void)
{
    int def_fail_cnt = 0;
    for (unsigned int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1) {
            continue; // skip reset check for this register
        }
        if (read_mask_array[i] == 0x00000000u) {
            continue; // unreadable entries ignored
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); // mask LSB as per procedure
#ifdef DEBUG_DISPLAY
        printf("[RST] idx=%u addr=0x%08lx rd=0x%08x masked=0x%08x exp=0x%08x\n", i, addr, data_rd, data, default_value_array[i]);
#endif
        if (data != default_value_array[i]) {
            def_fail_cnt++;
        }
    }
    return def_fail_cnt;
}

/*
Function: chk_rd_wr
Purpose : Perform write/readback patterns across impacted GPIO registers honoring skip and R/W masks.
*/
static int chk_rd_wr(void)
{
    int wr_fail_cnt = 0;
    for (unsigned int pat = 0; pat < 6; pat++) {
        unsigned int data_wr = chk_val[pat];
        // Write phase
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000u) continue;
            unsigned long addr = addr_array[i];
            unsigned int wr = (data_wr & write_mask_array[i]);
#ifdef DEBUG_DISPLAY
            printf("[WR ] pat=%u idx=%u addr=0x%08lx wr=0x%08x\n", pat, i, addr, wr);
#endif
            write_reg(addr, wr); // write masked pattern
        }
        // Read/compare phase
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000u) continue;
            if (read_mask_array[i] == 0x00000000u) continue;
            unsigned long addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[RD ] pat=%u idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x rm=0x%08x wm=0x%08x def=0x%08x\n",
                   pat, i, addr, data_rd, exp_val, read_mask_array[i], write_mask_array[i], default_value_array[i]);
#endif
            if (data_rd != exp_val) {
                wr_fail_cnt++;
            }
        }
    }
    return wr_fail_cnt;
}

/*
Function: test_case
Purpose : Orchestrate default check followed by write/readback check and report final result.
*/
int test_case(void)
{
    int def_fail_cnt = chk_rst_val();
    int wr_fail_cnt  = chk_rd_wr();
#ifdef DEBUG_DISPLAY
    printf("[SUM] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
    return 0;
}
