#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>

static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;

static void chk_rst_val(void)
{
    for (int i = 0; i < CNT; i++) {
        if (skip_array[i]) continue;
        unsigned int addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];
        unsigned int expected = ((unsigned int)default_value_array[i]) & rmask;
        unsigned int rdata = read_reg(addr) & rmask;
#ifdef DEBUG_RW_MSG
        printf("[RST] idx=%d addr=0x%08X rmask=0x%08X exp=0x%08X got=0x%08X\n", i, addr, rmask, expected, rdata);
#endif
        if (rdata != expected) {
            printf("ERROR: Default mismatch @idx=%d addr=0x%08X exp=0x%08X got=0x%08X\n", i, addr, expected, rdata);
            def_fail_cnt++;
        }
    }
}

static void wr_rd_masked(void)
{
    for (int i = 0; i < CNT; i++) {
        if (skip_array[i]) continue;
        unsigned int addr = addr_array[i];
        unsigned int wmask = write_mask_array[i];
        unsigned int rmask = read_mask_array[i];
        unsigned int base = (unsigned int)default_value_array[i];

        /* Derive a deterministic pattern under write mask */
        unsigned int patt1 = (0xAAAAAAAAu & wmask);
        unsigned int patt2 = (0x55555555u & wmask);

        /* Write-1 pattern */
        unsigned int wval = (base & ~wmask) | patt1;
        write_reg(addr, wval);
        wait_on(1);
        unsigned int r1 = read_reg(addr) & rmask;
        unsigned int exp1 = ((wval) & rmask);
#ifdef DEBUG_RW_MSG
        printf("[WR1] idx=%d addr=0x%08X w=0x%08X exp=0x%08X got=0x%08X\n", i, addr, wval, exp1, r1);
#endif
        if (r1 != exp1) {
            printf("ERROR: WR1 mismatch @idx=%d addr=0x%08X exp=0x%08X got=0x%08X\n", i, addr, exp1, r1);
            wr_fail_cnt++;
        }

        /* Write-0 pattern */
        wval = (base & ~wmask) | patt2;
        write_reg(addr, wval);
        wait_on(1);
        unsigned int r2 = read_reg(addr) & rmask;
        unsigned int exp2 = ((wval) & rmask);
#ifdef DEBUG_RW_MSG
        printf("[WR0] idx=%d addr=0x%08X w=0x%08X exp=0x%08X got=0x%08X\n", i, addr, wval, exp2, r2);
#endif
        if (r2 != exp2) {
            printf("ERROR: WR0 mismatch @idx=%d addr=0x%08X exp=0x%08X got=0x%08X\n", i, addr, exp2, r2);
            wr_fail_cnt++;
        }
    }
}

void test_case(void)
{
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    chk_rst_val();
    wr_rd_masked();

    int test_err = (def_fail_cnt + wr_fail_cnt);
    printf("Summary: def_fail=%d wr_fail=%d => %s\n", def_fail_cnt, wr_fail_cnt, test_err ? "FAIL" : "PASS");
    finish(test_err);
}

void Default_IRQHandler(void)
{
    /* No interrupts are expected in this test. Any IRQ is a failure. */
    printf("ERROR: Unexpected IRQ in gpio_reg_wr_rd_test\n");
}
