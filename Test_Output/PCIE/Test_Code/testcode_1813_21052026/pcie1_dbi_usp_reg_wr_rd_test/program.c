// Author - AI Force 1.3.2. Date 21-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose: Perform default/reset value checks per Meta Steps
 */
static void chk_rst_val(void)
{
    unsigned int i;
    unsigned long int addr;
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rst_val()\n");
#endif
    for (i = 0U; i < CNT; ++i) {
        addr = addr_array[i];
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Skip read: index=%u addr=0x%08lx (read_mask==0)\n", i, addr);
#endif
            continue;
        }
        if ((addr == mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG) ||
            (addr == mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS) ||
            (addr == mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF)) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Skip default check for index=%u addr=0x%08lx (exception)\n", i, addr);
#endif
            continue;
        }
        {
            int data_rd = read_reg(addr);
            if (data_rd != default_value_array[i]) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG][DEF_MISMATCH] idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, default_value_array[i]);
#endif
            }
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose: Perform masked write then read/verify per Meta Steps
 */
static void chk_rd_wr(int *p_wr_fail_cnt)
{
    unsigned int i;
    unsigned int p;
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rd_wr()\n");
#endif
    for (p = 0U; p < (sizeof(chk_val)/sizeof(chk_val[0])); ++p) {
        int data_wr = chk_val[p];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern pass %u: data_wr=0x%08x\n", p, data_wr);
#endif
        /* Write phase */
        for (i = 0U; i < CNT; ++i) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skip write idx=%u (skip_array==1)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Skip write idx=%u (write_mask==0)\n", i);
#endif
                continue;
            }
            write_reg(addr_array[i], data_wr);
        }
        /* Read/verify phase */
        for (i = 0U; i < CNT; ++i) {
            if (skip_array[i] == 1) {
                continue;
            }
            if ((write_mask_array[i] == 0) || (read_mask_array[i] == 0)) {
                continue;
            }
            {
                int data_rd = read_reg(addr_array[i]);
                int wr_n = (write_mask_array[i] ^ 0xffffffff);
                int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                               (wr_n & read_mask_array[i] & default_value_array[i]));
                if (data_rd != exp_val) {
                    (*p_wr_fail_cnt)++;
#ifdef DEBUG_DISPLAY
                    printf("[DEBUG][WR_MISMATCH] idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x wmask=0x%08x rmask=0x%08x\n",
                           i, addr_array[i], data_rd, exp_val, write_mask_array[i], read_mask_array[i]);
#endif
                }
            }
        }
    }
}

/*
 * Function: test_case
 * Purpose: Entry point. Execute phases and finish with pass/fail.
 */
int test_case(void)
{
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Start test_case: pcie1_dbi_usp_reg_wr_rd_test\n");
#endif
    chk_rst_val();
    chk_rd_wr(&wr_fail_cnt);

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1);
    } else {
        finish(0);
    }
    return 0; /* Unreachable if finish() terminates */
}
