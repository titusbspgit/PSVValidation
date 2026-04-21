#include "test_define.c"

/*
Test Description:
program.c implements test_case(): calls chk_rst_val() then chk_rd_wr();
If (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0).
chk_rst_val(): for i in [0..CNT-1], if not skipped and readable, read and
compare (data_rd & 0xfffffffe) with default_value_array[i].
chk_rd_wr(): for j over chk_val[]{0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}:
  - Write masked data to each writable, non-skipped register
  - Read back masked value and compare against expected per mask/default
*/

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    unsigned int i;
    for (i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1)
            continue; // Skip reset check for this register
        if (read_mask_array[i] == 0x00000000)
            continue; // Not readable
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFE); // Mask bit0 as per spec
        if (data != default_value_array[i]) {
            def_fail_cnt++;
            printf("[DEFCHK] Mismatch at idx %u addr 0x%08lx: rd=0x%08x exp=0x%08x\n", i, addr, data, default_value_array[i]);
        }
    }
}

static void chk_rd_wr(void)
{
    unsigned int j, i;
    unsigned int chk_val[6] = {0xFFFFFFFF, 0xAAAAAAAA, 0x55555555, 0xF5F5F5F5, 0xA5A5A5A5, 0xFFFF0000};

    for (j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];

        // Write phase
        for (i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1)
                continue; // Skip write/read for this register
            if (write_mask_array[i] == 0x00000000)
                continue; // Not writable
            write_reg(addr, (data_wr & write_mask_array[i]));
        }

        // Read/compare phase
        for (i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1)
                continue; // Skip read/compare for this register
            if (write_mask_array[i] == 0x00000000)
                continue; // Was not written
            if (read_mask_array[i] == 0x00000000)
                continue; // Not readable

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFF);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                   (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
                printf("[WRRD] Mismatch at idx %u addr 0x%08lx: rd=0x%08x exp=0x%08x wrmsk=0x%08x rdmsk=0x%08x\n",
                       i, addr, data_rd, exp_val, write_mask_array[i], read_mask_array[i]);
            }
        }
    }
}

int test_case(void)
{
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }

    return 0;
}
