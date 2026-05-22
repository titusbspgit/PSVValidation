// Author - AI Force 1.3.2. Date 22-05-2026
// (EMBENGG-SYSAPPS)

#include "test_define.c"

/*
 * Function: chk_rst_val
 * Purpose : Validate default/reset values for readable registers.
 */
static void chk_rst_val(void)
{
    unsigned long addr = 0U;          // Register address
    unsigned int data_rd = 0U;        // Read value
    int i = 0;                        // Loop index

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rst_val()\n");
#endif

    for (i = 0; i < CNT; i++) {
        addr = addr_array[i];

        // Skip if read mask is zero
        if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: i=%d addr=0x%08lx SKIP (read mask 0)\n", i, addr);
#endif
            continue;
        }

        // Skip default comparison for specific register as per Meta Steps
        if (addr == mizar_PCIE0_SII_PHY_RST_CONTROL) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: i=%d addr=0x%08lx SKIP (PHY_RST_CONTROL)\n", i, addr);
#endif
            continue;
        }

        // Read and compare against default value
        data_rd = read_reg(addr);
        if (data_rd != (unsigned int)default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: MISMATCH i=%d addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data_rd, (unsigned int)default_value_array[i]);
#endif
            extern int def_fail_cnt; // Declared below
            def_fail_cnt++;
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] chk_rst_val: PASS i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_rd);
#endif
        }
    }
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform write/readback validation using specified patterns.
 */
static void chk_rd_wr(void)
{
    unsigned long addr = 0U;
    unsigned int data_rd = 0U;
    unsigned int data_wr = 0U;
    unsigned int wr_n = 0U;
    unsigned int exp_val = 0U;
    int i = 0;
    int j = 0;

    // Patterns as per Meta Steps
    int chk_val[6] = {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000};

#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Enter chk_rd_wr()\n");
#endif

    for (j = 0; j < 6; j++) {
        data_wr = (unsigned int)chk_val[j];
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Pattern %d: 0x%08x\n", j, data_wr);
#endif
        // Write phase
        for (i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write: i=%d SKIP (skip_array)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Write: i=%d SKIP (write mask 0)\n", i);
#endif
                continue;
            }
            addr = addr_array[i];
            write_reg(addr, data_wr);
#ifdef DEBUG_DISPLAY
            printf("[DEBUG] Write: i=%d addr=0x%08lx val=0x%08x\n", i, addr, data_wr);
#endif
        }

        // Read/verify phase
        for (i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : i=%d SKIP (skip_array)\n", i);
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : i=%d SKIP (write mask 0)\n", i);
#endif
                continue;
            }
            if (read_mask_array[i] == 0x00000000) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] Read : i=%d SKIP (read mask 0)\n", i);
#endif
                continue;
            }

            addr = addr_array[i];
            data_rd = read_reg(addr);
            wr_n = (unsigned int)(write_mask_array[i] ^ 0xffffffffU);
            exp_val = (unsigned int)((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                     (wr_n & read_mask_array[i] & default_value_array[i]));

            if (data_rd != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RDWR MISMATCH i=%d addr=0x%08lx rd=0x%08x exp=0x%08x wm=0x%08x rm=0x%08x def=0x%08x\n",
                       i, addr, data_rd, exp_val, (unsigned int)write_mask_array[i], (unsigned int)read_mask_array[i], (unsigned int)default_value_array[i]);
#endif
            extern int wr_fail_cnt; // Declared below
            wr_fail_cnt++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("[DEBUG] RDWR PASS i=%d addr=0x%08lx rd=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
}

/*
 * Function: soft_reset_chk
 * Purpose : Perform soft reset sequence (Not invoked per Meta).
 */
static void soft_reset_chk(void)
{
    unsigned int save = 0U;
    save = read_reg(SOFT_RST_REG_ADDRESS);
    write_reg(SOFT_RST_REG_ADDRESS, SOFT_RST_REG_DATA);
    wait_on(1000U);
    write_reg(SOFT_RST_REG_ADDRESS, save);
    wait_on(1000U);
}

/* Global error counters as per Acceptance Criteria */
int def_fail_cnt = 0;
int wr_fail_cnt = 0;

/*
 * Entry Point: test_case
 */
int test_case(void)
{
#ifdef DEBUG_DISPLAY
    printf("[DEBUG] Start test_case()\n");
#endif

    def_fail_cnt = 0;
    wr_fail_cnt = 0;

    // Step 1: Reset value checks
    chk_rst_val();

    // Step 2: Read/Write checks
    chk_rd_wr();

    // Step 3: Determine result
    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Test FAIL def_fail_cnt=%d wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1); // FAIL
    } else {
#ifdef DEBUG_DISPLAY
        printf("[DEBUG] Test PASS\n");
#endif
        finish(0); // PASS
    }
}
