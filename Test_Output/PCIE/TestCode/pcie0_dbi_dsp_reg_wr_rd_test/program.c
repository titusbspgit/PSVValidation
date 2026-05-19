// Author - AI Force 1.3.2. Date 19-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose : Perform default/reset value checks on readable registers.
 * Notes   : Skips entries with read_mask_array[i] == 0 and the three
 *           explicitly excluded addresses per Meta Steps.
 */
static void chk_rst_val(void)
{
    unsigned int i;
    for (i = 0; i < CNT; i++) {
        unsigned int addr = addr_array[i];
        if (read_mask_array[i] == 0U) {
            continue; /* Not readable or spec not found; skip */
        }
        if ((addr == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG) ||
            (addr == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS) ||
            (addr == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF)) {
            continue; /* Explicitly excluded per test procedure */
        }
        unsigned int data_rd = read_reg(addr);
#ifdef DEBUG_DISPLAY
        printf("[CHK_DEF] idx=%u addr=0x%08X rd=0x%08X exp=0x%08X\n", i, addr, data_rd, default_value_array[i]);
#endif
        if (data_rd != default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[CHK_DEF][FAIL] idx=%u addr=0x%08X got=0x%08X exp=0x%08X\n", i, addr, data_rd, default_value_array[i]);
#endif
            extern int def_fail_cnt; /* ensure single definition below */
            def_fail_cnt++;
        } else {
#ifdef DEBUG_DISPLAY
            printf("[CHK_DEF][PASS] idx=%u addr=0x%08X value matches default.\n", i, addr);
#endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform write/readback checks with specified patterns on
 *           writable bits only, validating expected read values.
 */
static void chk_rd_wr(void)
{
    static const unsigned int chk_val[6] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
    };
    unsigned int j, i;

    for (j = 0U; j < 6U; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[WRCHK] pattern[%u]=0x%08X\n", j, data_wr);
#endif
        /* Write phase */
        for (i = 0U; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1U) {
                continue; /* Skip flagged entries */
            }
            if (write_mask_array[i] == 0U) {
                continue; /* No writable bits */
            }
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[WR] idx=%u addr=0x%08X data=0x%08X\n", i, addr, data_wr);
#endif
        }

        /* Read/verify phase */
        for (i = 0U; i < CNT; i++) {
            unsigned int addr = addr_array[i];
            if (skip_array[i] == 1U) {
                continue; /* Skip flagged entries */
            }
            if (write_mask_array[i] == 0U) {
                continue; /* Not part of write test */
            }
            if (read_mask_array[i] == 0U) {
                continue; /* Not readable / not validated */
            }
            unsigned int data_rd = read_reg(addr);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n    & read_mask_array[i] & default_value_array[i]));
#ifdef DEBUG_DISPLAY
            printf("[RD] idx=%u addr=0x%08X rd=0x%08X exp=0x%08X wr_m=0x%08X rd_m=0x%08X\n",
                   i, addr, data_rd, exp_val, write_mask_array[i], read_mask_array[i]);
#endif
            if (data_rd != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[WRCHK][FAIL] idx=%u addr=0x%08X got=0x%08X exp=0x%08X\n",
                       i, addr, data_rd, exp_val);
#endif
                extern int wr_fail_cnt; /* ensure single definition below */
                wr_fail_cnt++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("[WRCHK][PASS] idx=%u addr=0x%08X value matches expected.\n", i, addr);
#endif
            }
        }
    }
}

/* Global error counters */
int def_fail_cnt = 0;
int wr_fail_cnt = 0;

/*
 * Function: test_case
 * Purpose : Execute test sequence: default check then write/read check,
 *           evaluate acceptance criteria and terminate with finish().
 */
void test_case(void)
{
    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
}
