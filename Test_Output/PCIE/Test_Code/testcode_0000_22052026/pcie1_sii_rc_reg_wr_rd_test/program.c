// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/* --------------------------------------------------------------------------
 * Function: static void chk_rst_val(void)
 * Purpose : Verify default reset values for readable registers. Skip
 *           default-value comparison for mizar_PCIE1_SII_PHY_RST_CONTROL.
 * -------------------------------------------------------------------------- */
static int def_fail_cnt = 0;
static int wr_fail_cnt  = 0;

static void chk_rst_val(void)
{
    unsigned int i;
    const unsigned int arr_len = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));

    for (i = 0; i < arr_len; i++) {
        unsigned long addr = addr_array[i];
        unsigned int rmask = read_mask_array[i];

        if (rmask == 0x00000000u) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: skip idx=%u (read_mask=0)\n", i);
#endif
            continue; // unreadable
        }

        if (addr == mizar_PCIE1_SII_PHY_RST_CONTROL) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] chk_rst_val: skip default compare idx=%u (PHY_RST_CONTROL)\n", i);
#endif
            continue; // special-case skip per Meta Steps
        }

        unsigned int data_rd = read_reg(addr);
        unsigned int def_exp = (unsigned int)default_value_array[i];
        if (data_rd != def_exp) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[ERR] Default mismatch idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x\n",
                   i, addr, data_rd, def_exp);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Default match idx=%u addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
}

/* --------------------------------------------------------------------------
 * Function: static void chk_rd_wr(void)
 * Purpose : Perform masked write-readback across all registers using
 *           fixed patterns, honoring skip/write/read masks.
 * -------------------------------------------------------------------------- */
static void chk_rd_wr(void)
{
    static const unsigned int chk_val[6] = {
        0xffffffffu, 0xaaaaaaaau, 0x55555555u, 0x00000000u, 0xA5A5A5A5u, 0xffff0000u
    };

    const unsigned int arr_len = (unsigned int)(sizeof(addr_array)/sizeof(addr_array[0]));

    for (unsigned int j = 0; j < 6u; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DBG] Pattern %u: data_wr=0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (unsigned int i = 0; i < arr_len; i++) {
            unsigned long addr = addr_array[i];
            unsigned int wmask = (unsigned int)write_mask_array[i];
            unsigned int skip  = (unsigned int)skip_array[i];

            if ((skip == 1u) || (wmask == 0x00000000u)) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Write skip idx=%u (skip=%u wmask=0x%08x)\n", i, skip, wmask);
#endif
                continue;
            }

            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DBG] W idx=%u addr=0x%08lx data=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (unsigned int i = 0; i < arr_len; i++) {
            unsigned long addr = addr_array[i];
            unsigned int rmask = (unsigned int)read_mask_array[i];
            unsigned int wmask = (unsigned int)write_mask_array[i];
            unsigned int skip  = (unsigned int)skip_array[i];

            if ((skip == 1u) || (wmask == 0x00000000u) || (rmask == 0x00000000u)) {
#ifdef DEBUG_DISPLAY
                printf("[DBG] Read skip idx=%u (skip=%u wmask=0x%08x rmask=0x%08x)\n", i, skip, wmask, rmask);
#endif
                continue;
            }

            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (wmask ^ 0xffffffffu);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & (unsigned int)default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[ERR] WR mismatch idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x (wmask=0x%08x rmask=0x%08x)\n",
                       i, addr, data_rd, exp_val, wmask, rmask);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DBG] WR match idx=%u addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}

/* --------------------------------------------------------------------------
 * Function: static void soft_reset_chk(void)
 * Purpose : Demonstration of soft reset write/restore per Meta (not invoked).
 * -------------------------------------------------------------------------- */
static void soft_reset_chk(void)
{
    unsigned int save = read_reg(SOFT_RST_REG_ADDRESS);
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(1000);
    write_reg(SOFT_RST_REG_ADDRESS, save);
    wait_on(1000);
}

/* --------------------------------------------------------------------------
 * Function: int test_case(void)
 * Purpose : Entry point; run checks and finish with PASS/FAIL per criteria.
 * -------------------------------------------------------------------------- */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] Starting test_case: pcie1_sii_rc_reg_wr_rd_test\n");
#endif

    chk_rst_val();
    chk_rd_wr();

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
#ifdef DEBUG_DISPLAY
        printf("[INFO] TEST FAIL: def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
        return 1;
    }

#ifdef DEBUG_DISPLAY
    printf("[INFO] TEST PASS\n");
#endif
    finish(0);
    return 0;
}
