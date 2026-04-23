// Author - AI Force 1.3.2. Date 23-04-2026
// (EMBENGG-SYSAPPS)

#include<test_define.c>
#include<test_common.h>
#include<stdio.h>

/*
Purpose: gpio_reg_wr_rd_test/
program.c performs two phases: chk_rst_val() to verify defaults and chk_rd_wr() to verify masked write/read using arrays from test_define.c (addr_array, default_value_array, read_mask_array, write_mask_array, skip_array, skip_rst_array). Fail counts (def_fail_cnt, wr_fail_cnt) accumulate and finish(0/1) indicates pass/fail.
*/

static inline unsigned int read_reg(volatile unsigned int addr){return *((volatile unsigned int*)addr);} 
static inline void write_reg(volatile unsigned int addr, unsigned int val){*((volatile unsigned int*)addr)=val;}

static int def_fail_cnt=0, wr_fail_cnt=0;

/*
Function: chk_rst_val
Verifies reset/default values for each impacted register using default_value_array and read masks.
*/
static void chk_rst_val(void){
    for(int i=0;i<CNT;i++){
        if (skip_rst_array[i]==1) { continue; }
        if (read_mask_array[i]==0) { continue; }
        unsigned int addr = addr_array[i];
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xfffffffeu);
        unsigned int exp = (unsigned int)default_value_array[i];
        if (data != exp){
#ifdef DEBUG_DISPLAY
            printf("RST Mismatch @idx %d addr 0x%08X: rd=0x%08X exp=0x%08X\n", i, addr, data, exp);
#endif
            def_fail_cnt++;
        }
    }
}

/*
Function: chk_rd_wr
Per acceptance: write masked patterns to writable fields and verify readback considering read and write masks.
*/
static void chk_rd_wr(void){
    unsigned int chk_val[6] = {0xFFFFFFFFu,0xAAAAAAAau,0x55555555u,0xF5F5F5F5u,0xA5A5A5A5u,0xFFFF0000u};
    for(int j=0;j<6;j++){
        unsigned int data_wr = chk_val[j];
        // Write phase
        for(int i=0;i<CNT;i++){
            if (skip_array[i]==1) { continue; }
            if (write_mask_array[i]==0) { continue; }
            unsigned int addr = addr_array[i];
            unsigned int wval = (data_wr & (unsigned int)write_mask_array[i]);
            write_reg(addr, wval);
        }
        // Read/compare phase
        for(int i=0;i<CNT;i++){
            if (skip_array[i]==1) { continue; }
            if (write_mask_array[i]==0) { continue; }
            if (read_mask_array[i]==0) { continue; }
            unsigned int addr = addr_array[i];
            unsigned int data_rd = (read_reg(addr) & (unsigned int)read_mask_array[i]);
            unsigned int wr_n = ((unsigned int)write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                                     (wr_n & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));
            if (data_rd != exp_val){
#ifdef DEBUG_DISPLAY
                printf("WR Mismatch @idx %d addr 0x%08X: rd=0x%08X exp=0x%08X j=%d\n", i, addr, data_rd, exp_val, j);
#endif
                wr_fail_cnt++;
            }
        }
    }
}

/*
Function: test_case
Entry for executing the testcase per metadata.
*/
void test_case(void){
#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] Start\n");
#endif
    chk_rst_val();
    chk_rd_wr();
    if (def_fail_cnt>0 || wr_fail_cnt>0){
#ifdef DEBUG_DISPLAY
        printf("[gpio_reg_wr_rd_test] FAIL def=%d wr=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return;
    }
#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] PASS\n");
#endif
    finish(0);
}
