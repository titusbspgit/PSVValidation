#include "test_define.c"

/*
Hidden_Test_Description (verbatim):
program.c performs two phases: chk_rst_val() to verify defaults and chk_rd_wr() to verify masked write/read using arrays from test_define.c (addr_array, default_value_array, read_mask_array, write_mask_array, skip_array, skip_rst_array). Fail counts (def_fail_cnt, wr_fail_cnt) accumulate and finish(0/1) indicates pass/fail.
*/

static int def_fail_cnt = 0;
static int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    unsigned int i;
    for (i = 0; i < CNT; i++) {
        const unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1) {
            continue; // skipped by reset-skip mask
        }
        if (read_mask_array[i] == 0) {
            continue; // nothing to read/compare
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data    = (data_rd & 0xfffffffeU);
        if (data == default_value_array[i]) {
            // PASS for this index
        } else {
            def_fail_cnt++;
            printf("RST Mismatch @[%u] addr=0x%08lx rd=0x%08x exp=0x%08x\n",
                   i, addr, data, default_value_array[i]);
        }
    }
}

static void chk_rd_wr(void)
{
    unsigned int i, j;
    for (j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
        // Write phase
        for (i = 0; i < CNT; i++) {
            const unsigned long addr = addr_array[i];
            if (skip_array[i] == 1) {
                continue; // skip by write-skip mask
            }
            if (write_mask_array[i] == 0) {
                continue; // nothing to write
            }
            write_reg(addr, (data_wr & write_mask_array[i]));
        }
        // Read/compare phase
        for (i = 0; i < CNT; i++) {
            const unsigned long addr = addr_array[i];
            if (skip_array[i] == 1) {
                continue; // skip by write-skip mask
            }
            if (write_mask_array[i] == 0) {
                continue; // nothing to write
            }
            if (read_mask_array[i] == 0) {
                continue; // nothing to read/compare
            }
            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xffffffffU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd == exp_val) {
                // PASS for this index/pattern
            } else {
                wr_fail_cnt++;
                printf("WR/RD Mismatch @[%u] addr=0x%08lx pat=0x%08x rd=0x%08x exp=0x%08x rm=0x%08x wm=0x%08x\n",
                       i, addr, data_wr, data_rd, exp_val, read_mask_array[i], write_mask_array[i]);
            }
        }
    }
}

void test_case(void)
{
    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
}
