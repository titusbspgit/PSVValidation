#include "test_define.c"

// Test_Stage3 Agent: gpio_reg_wr_rd_test
// High-level description (from CSV):
// Verify GPIO register default reset values and masked write/read behavior across the defined
// register set using per-register read/write masks and skip lists. Writes six data patterns,
// reads back with masks, and compares against expected values computed from masks and defaults.

// External hooks expected from the environment
extern void wait_on(unsigned int cycles);
extern unsigned int read_reg(unsigned int addr);
extern void write_reg(unsigned int addr, unsigned int data);
extern void finish(int status);

// Local debug display macro (maps to printf via test_common.h)
#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY printf
#endif

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Checking reset values...\n");

    if (addr_array[0] == 0 && read_mask_array[0] == 0 && write_mask_array[0] == 0 && default_value_array[0] == 0) {
        DEBUG_DISPLAY("[ERROR] RAG arrays are uninitialized placeholders. Aborting.\n");
        def_fail_cnt++;
        return;
    }

    for (unsigned int i = 0; i < CNT; i++) {
        unsigned int addr = addr_array[i];
        if (skip_rst_array[i] == 1) {
            DEBUG_DISPLAY("[RST-SKIP] idx=%u addr=0x%08X\n", i, addr);
            continue;
        }
        if (read_mask_array[i] == 0x00000000) {
            DEBUG_DISPLAY("[RST-MASK0] idx=%u addr=0x%08X (unreadable by mask)\n", i, addr);
            continue;
        }
        unsigned int data_rd = read_reg(addr);
        // As per CSV: apply default read mask to clear bit0 during reset-value check
        unsigned int data = (data_rd & 0xFFFFFFFEu);
        unsigned int exp  = (default_value_array[i] & 0xFFFFFFFEu);
        if (data != exp) {
            DEBUG_DISPLAY("[RST-MISMATCH] idx=%u addr=0x%08X exp=0x%08X rd=0x%08X\n", i, addr, exp, data);
            def_fail_cnt++;
        }
    }
}

static void chk_rd_wr(void)
{
    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Checking masked write/read...\n");

    const unsigned int patterns[6] = {
        0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    for (unsigned int p = 0; p < 6; p++) {
        unsigned int data_wr = patterns[p];
        DEBUG_DISPLAY("[WRITE-PASS %u] data_wr=0x%08X\n", p, data_wr);

        // Write phase
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1) {
                DEBUG_DISPLAY("[WR-SKIP] idx=%u addr=0x%08X\n", i, addr);
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
                DEBUG_DISPLAY("[WR-MASK0] idx=%u addr=0x%08X\n", i, addr);
                continue;
            }
            unsigned int wr = (data_wr & write_mask_array[i]);
            write_reg(addr, wr);
        }

        // Small settle
        wait_on(10);

        // Read/verify phase
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000) continue;
            if (read_mask_array[i] == 0x00000000)  continue;

            unsigned int data_rd_m = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n     & read_mask_array[i] & default_value_array[i]));

            if (data_rd_m != exp) {
                DEBUG_DISPLAY("[WR-MISMATCH] p=%u idx=%u addr=0x%08X exp=0x%08X rd=0x%08X rmask=0x%08X wmask=0x%08X def=0x%08X\n",
                              p, i, addr, exp, data_rd_m, read_mask_array[i], write_mask_array[i], default_value_array[i]);
                wr_fail_cnt++;
            }
        }
    }
}

void test_case(void)
{
    DEBUG_DISPLAY("[gpio_reg_wr_rd_test] Test started. CNT=%u\n", CNT);

    def_fail_cnt = 0;
    wr_fail_cnt  = 0;

    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        DEBUG_DISPLAY("[gpio_reg_wr_rd_test] FAIL: def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
    } else {
        DEBUG_DISPLAY("[gpio_reg_wr_rd_test] PASS\n");
        finish(0);
    }
}
