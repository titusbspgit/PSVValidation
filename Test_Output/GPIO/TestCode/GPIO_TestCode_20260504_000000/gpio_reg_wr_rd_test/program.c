// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

/* High-level Description (from metadata, AS-IS):
   program.c calls chk_rst_val() then chk_rd_wr(). chk_rst_val(): For i=0..CNT-1, addr=addr_array[i]; if skip_rst_array[i]==1 → continue; if read_mask_array[i]==0 → continue; data_rd=read_reg(addr); data=(data_rd & 0xFFFFFFFE); if data==default_value_array[i] → PASS else def_fail_cnt++ and print failure. chk_rd_wr(): Define chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; For each pattern j: data_wr=chk_val[j]; Writing phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; else write_reg(addr, (data_wr & write_mask_array[i])). Reading phase: for i=0..CNT-1, addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0 → continue; if read_mask_array[i]==0 → continue; else data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xFFFFFFFF); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd==exp_val → PASS else wr_fail_cnt++ and print failure. At end of test_case(), if def_fail_cnt>0 or wr_fail_cnt>0 finish(1) else finish(0).
*/

#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose : Validate reset values for readable registers while respecting skip_rst_array and read masks.
 */
static int chk_rst_val(void) {
    int def_fail_cnt = 0;
    for (unsigned i = 0; i < CNT; ++i) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[RST] Skipping index %u (skip_rst_array=1)\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[RST] Skipping index %u (read_mask=0)\n", i);
#endif
            continue;
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU); /* Per spec mask bit0 */
        if (data != default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[RST][FAIL] idx=%u addr=0x%08lX exp=0x%08X got=0x%08X raw=0x%08X\n",
                   i, addr, default_value_array[i], data, data_rd);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[RST][PASS] idx=%u addr=0x%08lX val=0x%08X\n", i, addr, data);
#endif
        }
    }
    return def_fail_cnt;
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform masked write/readback across registers, verifying expected value calculation as per spec.
 */
static int chk_rd_wr(void) {
    int wr_fail_cnt = 0;
    const unsigned int chk_val[6] = {0xFFFFFFFFU, 0xAAAAAAA AU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U};

    for (unsigned j = 0; j < 6U; ++j) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[WR] Pattern %u: 0x%08X\n", j, data_wr);
#endif
        /* Writing phase */
        for (unsigned i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[WR] Skip idx=%u (skip_array=1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[WR] Skip idx=%u (write_mask=0)\n", i);
#endif
                continue;
            }
            write_reg(addr, (data_wr & write_mask_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[WR] idx=%u addr=0x%08lX w=0x%08X mask=0x%08X\n", i, addr, (data_wr & write_mask_array[i]), write_mask_array[i]);
#endif
        }
        /* Read/verify phase */
        for (unsigned i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) continue;
            if (write_mask_array[i] == 0x00000000U) continue;
            if (read_mask_array[i] == 0x00000000U) continue;

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[RD][FAIL] idx=%u addr=0x%08lX exp=0x%08X got=0x%08X rm=0x%08X wm=0x%08X\n",
                       i, addr, exp_val, data_rd, read_mask_array[i], write_mask_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[RD][PASS] idx=%u addr=0x%08lX val=0x%08X\n", i, addr, data_rd);
#endif
            }
        }
    }
    return wr_fail_cnt;
}

/*
 * Function: test_case
 * Purpose : Execute reset check followed by write/readback check and report final PASS/FAIL strictly per acceptance criteria.
 */
void test_case(void) {
    int def_fail_cnt = chk_rst_val();
    int wr_fail_cnt = chk_rd_wr();
#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1);
    } else {
        finish(0);
    }
}
