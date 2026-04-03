#include "test_define.c"

/*
  Test: dmac_reg_wr_rd_test
  Objective:
    - Verify DMA controller register reset/default values.
    - For six data patterns, write to writable registers and verify masked readback.
    - Use masks and defaults from arrays; skip per skip_array and 0 read/write masks.

  Procedure (from CSV):
    - chk_rst_val(): For i=0..CNT-1, if (read_mask_array[i] != 0) read addr_array[i] and compare with default_value_array[i].
    - chk_rd_wr(): For each data pattern in chk_val:
        * Write phase: For i, if (skip_array[i]==1 || write_mask_array[i]==0) continue; else write_reg(addr, data_wr).
        * Read/verify phase: For i, if (skip_array[i]==1 || write_mask_array[i]==0 || read_mask_array[i]==0) continue; else:
            wr_n = (write_mask_array[i] ^ 0xffffffff);
            exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                       (wr_n     & read_mask_array[i] & default_value_array[i]));
            Compare read_reg(addr) with exp_val.
    - Finalize: finish(0) on pass; finish(1) on any failure.
*/

static int def_fail_cnt = 0;
static int wr_fail_cnt  = 0;

static void chk_rst_val(void) {
#ifdef DEBUG_DISPLAY
    printf("[INFO] chk_rst_val: Checking %d registers for default values\n", CNT);
#endif
    for (int i = 0; i < CNT; ++i) {
        unsigned long addr = addr_array[i];
        unsigned int rmask = (unsigned int)read_mask_array[i];

        if (rmask == 0x00000000u) {
#ifdef DEBUG_DISPLAY
            printf("[SKIP] i=%d addr=0x%08lx read_mask=0x%08x (no readable bits)\n", i, addr, rmask);
#endif
            continue;
        }

        unsigned int data_rd = read_reg(addr);
        unsigned int exp     = (unsigned int)default_value_array[i];

        if (data_rd != exp) {
            ++def_fail_cnt;
            printf("[DEF-FAIL] i=%d addr=0x%08lx exp=0x%08x rd=0x%08x\n", i, addr, exp, data_rd);
        }
#ifdef DEBUG_DISPLAY
        else {
            printf("[DEF-PASS] i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
        }
#endif
    }
}

static void chk_rd_wr(void) {
#ifdef DEBUG_DISPLAY
    printf("[INFO] chk_rd_wr: Patterns=%zu, Registers=%d\n", sizeof(chk_val)/sizeof(chk_val[0]), CNT);
#endif

    for (unsigned int j = 0; j < (sizeof(chk_val)/sizeof(chk_val[0])); ++j) {
        unsigned int data_wr = (unsigned int)chk_val[j];

#ifdef DEBUG_DISPLAY
        printf("[INFO] Pattern %u: data_wr=0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[SKIP-WR] i=%d (skip_array)\n", i);
#endif
                continue;
            }
            unsigned long addr  = addr_array[i];
            unsigned int wmask = (unsigned int)write_mask_array[i];

            if (wmask == 0x00000000u) {
#ifdef DEBUG_DISPLAY
                printf("[SKIP-WR] i=%d addr=0x%08lx write_mask=0x%08x (not writable)\n", i, addr, wmask);
#endif
                continue;
            }

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[WRITE] i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[SKIP-RD] i=%d (skip_array)\n", i);
#endif
                continue;
            }

            unsigned long addr  = addr_array[i];
            unsigned int rmask = (unsigned int)read_mask_array[i];
            unsigned int wmask = (unsigned int)write_mask_array[i];

            if (wmask == 0x00000000u) {
#ifdef DEBUG_DISPLAY
                printf("[SKIP-RD] i=%d addr=0x%08lx write_mask=0x%08x (not writable, skip verify)\n", i, addr, wmask);
#endif
                continue;
            }
            if (rmask == 0x00000000u) {
#ifdef DEBUG_DISPLAY
                printf("[SKIP-RD] i=%d addr=0x%08lx read_mask=0x%08x (not readable)\n", i, addr, rmask);
#endif
                continue;
            }

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n    = (wmask ^ 0xffffffffu);
            unsigned int defv    = (unsigned int)default_value_array[i];
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & defv));

            if (data_rd != exp_val) {
                ++wr_fail_cnt;
                printf("[WR-FAIL] pat=%u i=%d addr=0x%08lx exp=0x%08x rd=0x%08x rmask=0x%08x wmask=0x%08x def=0x%08x\n",
                       j, i, addr, exp_val, data_rd, rmask, wmask, defv);
            }
#ifdef DEBUG_DISPLAY
            else {
                printf("[WR-PASS] pat=%u i=%d addr=0x%08lx val=0x%08x\n", j, i, addr, data_rd);
            }
#endif
        }
    }
}

void test_case(void) {
#ifdef DEBUG_DISPLAY
    printf("[START] dmac_reg_wr_rd_test\n");
#endif

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        printf("[RESULT] FAIL: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
        return;
    }

    printf("[RESULT] PASS\n");
    finish(0);
    return;
}
