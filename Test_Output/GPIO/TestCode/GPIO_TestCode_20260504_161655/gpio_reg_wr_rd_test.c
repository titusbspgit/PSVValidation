/*
 * Test: gpio_reg_wr_rd_test
 * Description (verbatim from metadata):
 * Performs two phases: (1) Default value check for each address in addr_array using read_mask_array and skip_rst_array; read data is masked with 0xFFFFFFFE then compared to default_value_array. (2) Read/write verification using six patterns {0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000}. For each address not in skip_array and with nonzero write mask, writes (data_wr & write_mask_array[i]); then reads back data_rd=(read_reg(addr) & read_mask_array[i]) and computes expected value exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Compares data_rd to exp_val. Tracks def_fail_cnt and wr_fail_cnt. Finishes with finish(0) if both counts are zero, else finish(1).
 */

#include <stdint.h>

/* The following APIs/macros are expected to be provided by the repository template/harness:
 * - uint32_t read_reg(uint32_t addr);
 * - void write_reg(uint32_t addr, uint32_t val);
 * - void wait_on(uint32_t cycles);
 * - void finish(int status);
 * - printf (logging)
 *
 * The following arrays and counts are expected to be defined by the repository/platform:
 * - extern uint32_t CNT;
 * - extern uint32_t addr_array[];
 * - extern uint32_t read_mask_array[];
 * - extern uint32_t write_mask_array[];
 * - extern uint32_t default_value_array[];
 * - extern uint8_t  skip_array[];
 * - extern uint8_t  skip_rst_array[];
 */

extern uint32_t CNT;
extern uint32_t addr_array[];
extern uint32_t read_mask_array[];
extern uint32_t write_mask_array[];
extern uint32_t default_value_array[];
extern uint8_t  skip_array[];
extern uint8_t  skip_rst_array[];

static uint32_t def_fail_cnt = 0;
static uint32_t wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    for (uint32_t i = 0; i < CNT; ++i) {
        uint32_t addr = addr_array[i];
        if (skip_rst_array[i] == 1) continue;
        if (read_mask_array[i] == 0) continue;

        uint32_t data_rd = read_reg(addr);
        uint32_t data = (data_rd & 0xFFFFFFFEu);
        if (data == default_value_array[i]) {
            // pass
        } else {
            ++def_fail_cnt;
            printf("[gpio_reg_wr_rd_test][RST_CHK] Mismatch at idx=%lu addr=0x%08lx rd=0x%08lx exp=0x%08lx\n",
                   (unsigned long)i, (unsigned long)addr, (unsigned long)data, (unsigned long)default_value_array[i]);
        }
    }
}

static void chk_rd_wr(void)
{
    const uint32_t chk_val[6] = {
        0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    for (uint32_t j = 0; j < 6; ++j) {
        uint32_t data_wr = chk_val[j];

        /* Write phase */
        for (uint32_t i = 0; i < CNT; ++i) {
            uint32_t addr = addr_array[i];
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0) continue;

            write_reg(addr, (data_wr & write_mask_array[i]));
        }

        /* Read/compare phase */
        for (uint32_t i = 0; i < CNT; ++i) {
            uint32_t addr = addr_array[i];
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0) continue;
            if (read_mask_array[i] == 0) continue;

            uint32_t data_rd = (read_reg(addr) & read_mask_array[i]);
            uint32_t wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            uint32_t exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd == exp_val) {
                // pass
            } else {
                ++wr_fail_cnt;
                printf("[gpio_reg_wr_rd_test][WR_RD] Mismatch at idx=%lu addr=0x%08lx rd=0x%08lx exp=0x%08lx pat=0x%08lx\n",
                       (unsigned long)i, (unsigned long)addr, (unsigned long)data_rd, (unsigned long)exp_val, (unsigned long)data_wr);
            }
        }
    }
}

void gpio_reg_wr_rd_test(void)
{
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        printf("[gpio_reg_wr_rd_test] FAIL: def_fail_cnt=%lu wr_fail_cnt=%lu\n",
               (unsigned long)def_fail_cnt, (unsigned long)wr_fail_cnt);
        finish(1);
    } else {
        printf("[gpio_reg_wr_rd_test] PASS\n");
        finish(0);
    }
}
