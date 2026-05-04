// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)
#include "test_define.c"

/*
 High-level Description (from META):
 This test checks default register values and verifies masked write/read behavior across GPIO address list using six data patterns.
*/

/*
 * chk_rst_val
 * Purpose: Verify default values for readable and not-skipped registers per META.
 */
static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping default check for addr[%u]=0x%08lX (skip_rst)\n", i, addr);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping default check for addr[%u]=0x%08lX (non-readable)\n", i, addr);
#endif
            continue;
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); /* per META: mask LSB */
        if (data != default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Default mismatch at addr 0x%08lX: exp=0x%08X rd=0x%08X\n", addr, default_value_array[i], data);
#endif
            /* def_fail_cnt increments on every failure */
            extern int def_fail_cnt; /* forward ref */
            def_fail_cnt++;
        }
    }
}

/*
 * chk_rd_wr
 * Purpose: Iterate test patterns, perform masked writes and validate masked reads per META.
 */
static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] Pattern %u: 0x%08X\n", j, data_wr);
#endif
        /* WRITE phase */
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000) continue;
            unsigned long addr = addr_array[i];
            unsigned int w = (data_wr & write_mask_array[i]);
            write_reg(addr, w);
#ifdef DEBUG_DISPLAY
            printf("[DBG] WR addr 0x%08lX <= 0x%08X (mask 0x%08X)\n", addr, w, write_mask_array[i]);
#endif
        }
        /* READ/COMPARE phase */
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000) continue;
            if (read_mask_array[i] == 0x00000000) continue;
            unsigned long addr = addr_array[i];
            unsigned int rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                (wr_n & read_mask_array[i] & default_value_array[i]));
            if (rd != exp) {
#ifdef DEBUG_DISPLAY
                printf("[ERR] RD/WR mismatch at addr 0x%08lX: exp=0x%08X rd=0x%08X (rmsk=0x%08X wmsk=0x%08X)\n",
                       addr, exp, rd, read_mask_array[i], write_mask_array[i]);
#endif
                extern int wr_fail_cnt; /* forward ref */
                wr_fail_cnt++;
            }
        }
    }
}

int def_fail_cnt = 0;
int wr_fail_cnt = 0;

/*
 * test_case
 * Purpose: Entry point that runs default-value and write/read checks, then finishes per acceptance criteria.
 */
void test_case(void)
{
    def_fail_cnt = 0;
    wr_fail_cnt = 0;
    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
}
