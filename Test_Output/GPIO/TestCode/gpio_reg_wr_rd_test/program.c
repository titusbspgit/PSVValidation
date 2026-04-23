// Author - AI Force 1.3.2. Date 23-04-2026
// (EMBENGG-SYSAPPS)

/*
  Testcase: gpio_reg_wr_rd_test
  High-level description (from metadata):
  - Two phases: chk_rst_val() to verify defaults and chk_rd_wr() to verify masked
    write/read using arrays from test_define.c (addr_array, default_value_array,
    read_mask_array, write_mask_array, skip_array, skip_rst_array).
  - Fail counts (def_fail_cnt, wr_fail_cnt) accumulate and finish(0/1) indicates pass/fail.
  Constraint: LSS_SYSREG intentionally ignored.
*/

#include "test_define.c"

/* Banner comment for function: test_case
   Purpose: Entry point that runs reset-value checks and masked write/read checks,
            then reports pass/fail via finish(). */
static int chk_rst_val(void);
static int chk_rd_wr(void);

void test_case(void)
{
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] Start\n");
#endif

    def_fail_cnt = chk_rst_val();
    wr_fail_cnt  = chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] def_fail_cnt=%d, wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
}

/* Banner comment for function: chk_rst_val
   Purpose: For each addressed register, verify the reset value matches expected
            default considering the known masking pattern. */
static int chk_rst_val(void)
{
    int i;
    int def_fail_cnt = 0;

    for (i = 0; i < CNT; i++) {
        unsigned int rm = read_mask_array[i];
        if (skip_rst_array[i] == 1) {
#ifdef DEBUG_DISPLAY
            printf("[RST] skip_rst idx=%d\n", i);
#endif
            continue;
        }
        if (rm == 0) {
#ifdef DEBUG_DISPLAY
            printf("[RST] read_mask==0 idx=%d\n", i);
#endif
            continue; /* nothing to compare */
        }
        unsigned int data_rd = read_reg(addr_array[i]);
        unsigned int data = (data_rd & 0xFFFFFFFEu); /* as per metadata */
        if (data != default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[RST][FAIL] idx=%d addr=0x%08lX rd=0x%08X exp=0x%08X\n",
                   i, addr_array[i], data, default_value_array[i]);
#endif
        }
    }

    return def_fail_cnt;
}

/* Banner comment for function: chk_rd_wr
   Purpose: Perform masked write/read verification using defined test patterns.
*/
static int chk_rd_wr(void)
{
    int i, j;
    int wr_fail_cnt = 0;
    const unsigned int chk_val[6] = {
        0xFFFFFFFFu, 0xAAAAAAAau, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    for (j = 0; j < 6; j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[WR] pattern[%d]=0x%08X\n", j, data_wr);
#endif
        /* Write phase */
        for (i = 0; i < CNT; i++) {
            unsigned int wm = write_mask_array[i];
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[WR] skip idx=%d\n", i);
#endif
                continue;
            }
            if (wm == 0u) {
                continue; /* non-writable: e.g., DIN groups */
            }
            write_reg(addr_array[i], (data_wr & wm));
        }

        /* Read/compare phase */
        for (i = 0; i < CNT; i++) {
            unsigned int rm = read_mask_array[i];
            unsigned int wm = write_mask_array[i];
            unsigned int wr_n = (wm ^ 0xFFFFFFFFu);

            if (skip_array[i] == 1) {
                continue;
            }
            if ((wm == 0u) || (rm == 0u)) {
                continue; /* cannot validate */
            }

            unsigned int data_rd = (read_reg(addr_array[i]) & rm);
            unsigned int exp_val = ((data_wr & rm & wm) | (wr_n & rm & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[RD][FAIL] idx=%d addr=0x%08lX rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X\n",
                       i, addr_array[i], data_rd, exp_val, rm, wm);
#endif
            }
        }
    }

    return wr_fail_cnt;
}
