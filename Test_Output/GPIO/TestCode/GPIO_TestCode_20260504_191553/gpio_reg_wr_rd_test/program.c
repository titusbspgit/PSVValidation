#include <lss_sysreg.h>
#include <stdio.h>
#include <test_common.h>
#include <test_define.c>

static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;

static inline unsigned int rd(unsigned long addr) { return read_reg(addr); }
static inline void wr(unsigned long addr, unsigned int v) { write_reg(addr, v); }

static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i]) continue;
        if ((unsigned int)read_mask_array[i] == 0u) continue;
        unsigned int data_rd = rd(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); // ignore bit0 as per spec
        unsigned int exp  = ((unsigned int)default_value_array[i]) & (unsigned int)read_mask_array[i];
        unsigned int got  = data & (unsigned int)read_mask_array[i];
        if (got != exp) {
            #ifdef DEBUG_DISPLAY
            printf("RSTVAL MISMATCH @%#lx: got=%#010x exp=%#010x (idx=%d)\n", addr, got, exp, i);
            #endif
            def_fail_cnt++;
        }
    }
}

static void chk_rd_wr(void)
{
    const unsigned int chk_val[] = {
        0xFFFFFFFFu, 0xAAAAAAA Au, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };
    const int npat = (int)(sizeof(chk_val)/sizeof(chk_val[0]));

    for (int p = 0; p < npat; p++) {
        unsigned int data_wr = chk_val[p];
        // Write phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) continue;
            unsigned int wmask = (unsigned int)write_mask_array[i];
            if (wmask == 0u) continue;
            wr(addr_array[i], (data_wr & wmask));
        }
        wait_on(2);
        // Read/verify phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) continue;
            unsigned int wmask = (unsigned int)write_mask_array[i];
            unsigned int rmask = (unsigned int)read_mask_array[i];
            if (wmask == 0u || rmask == 0u) continue;
            unsigned int data_rd = rd(addr_array[i]) & rmask;
            unsigned int wr_n   = ~wmask;
            unsigned int exp    = ((data_wr & rmask & wmask) | (wr_n & rmask & (unsigned int)default_value_array[i]));
            if (data_rd != exp) {
                #ifdef DEBUG_DISPLAY
                printf("WRRD MISMATCH @%#lx: got=%#010x exp=%#010x pat=%#010x idx=%d\n", addr_array[i], data_rd, exp, data_wr, i);
                #endif
                wr_fail_cnt++;
            }
        }
    }
}

void test_case(void)
{
    def_fail_cnt = 0;
    wr_fail_cnt = 0;
    chk_rst_val();
    chk_rd_wr();
    int fail = (def_fail_cnt > 0) || (wr_fail_cnt > 0);
    #ifdef DEBUG_DISPLAY
    printf("gpio_reg_wr_rd_test: def_fail_cnt=%d wr_fail_cnt=%d => %s\n", def_fail_cnt, wr_fail_cnt, fail?"FAIL":"PASS");
    #endif
    finish(fail ? 1 : 0);
}
