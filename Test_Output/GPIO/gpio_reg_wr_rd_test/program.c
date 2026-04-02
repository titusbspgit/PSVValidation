#include "test_define.c"

/*
 * gpio_reg_wr_rd_test
 * Description: Default value check and write/read verification across defined GPIO registers
 * using mask arrays and skip lists. Two phases are executed:
 *  1) DEFAULT VALUE CHECK (chk_rst_val)
 *  2) WRITE & READ CHECK (chk_rd_wr)
 */

static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; i++) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1u)
            continue; /* Skip default check for this address */
        if (read_mask_array[i] == 0x00000000u)
            continue; /* Address not readable */

        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); /* Modify mask as per procedure */
        if (data == default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[DEF-OK] Addr=0x%08lX Exp=0x%08X Rd=0x%08X Data=0x%08X\n", addr, default_value_array[i], data_rd, data);
#endif
        } else {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEF-ERR] Addr=0x%08lX Exp=0x%08X Rd=0x%08X Data=0x%08X\n", addr, default_value_array[i], data_rd, data);
#endif
        }
    }
}

static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < (sizeof(chk_val)/sizeof(chk_val[0])); j++) {
        unsigned int data_wr = chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[PATTERN] 0x%08X\n", data_wr);
#endif
        /* Write phase */
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u)
                continue; /* Skip write for this address */
            if (write_mask_array[i] == 0x00000000u)
                continue; /* Not writable */
            write_reg(addr, (data_wr & write_mask_array[i]));
        }

        /* Read/verify phase */
        for (unsigned int i = 0; i < CNT; i++) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u)
                continue; /* Skip read for this address */
            if (write_mask_array[i] == 0x00000000u)
                continue; /* Not writable; skip */
            if (read_mask_array[i] == 0x00000000u)
                continue; /* Not readable; skip */

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n = ~write_mask_array[i];
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (data_rd == exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[WRRD-OK] Addr=0x%08lX Exp=0x%08X Rd=0x%08X\n", addr, exp_val, data_rd);
#endif
            } else {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[WRRD-ERR] Addr=0x%08lX Exp=0x%08X Rd=0x%08X\n", addr, exp_val, data_rd);
#endif
            }
        }
    }
}

void test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[TEST] gpio_reg_wr_rd_test: START\n");
#endif

    def_fail_cnt = 0;
    wr_fail_cnt  = 0;

    chk_rst_val();
    chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[RESULT] def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0u) || (wr_fail_cnt > 0u)) {
#ifdef DEBUG_DISPLAY
        printf("[TEST] gpio_reg_wr_rd_test: FAIL\n");
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[TEST] gpio_reg_wr_rd_test: PASS\n");
#endif
        finish(0);
    }
}
