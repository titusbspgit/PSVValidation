// Author - AI Force 1.3.2. Date 26-04-2026
// (EMBENGG-SYSAPPS)

/* Include only test_define.c as mandated */
#include "test_define.c"

/*
 * Test: gpio_reg_wr_rd_test
 * Description (from metadata):
 * program.c runs test_case() which calls chk_rst_val() then chk_rd_wr().
 * chk_rst_val() loops i=0..CNT-1 over addr_array[]; if skip_rst_array[i]==1 continue; if read_mask_array[i]==0 continue;
 * data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); compare data==default_value_array[i].
 * chk_rd_wr() iterates j over 6 patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000);
 * for each i=0..CNT-1: if skip_array[i]==1 continue; if write_mask_array[i]==0 continue; write_reg(addr,(data_wr & write_mask_array[i]));
 * then readback phase: if skip_array[i]==1 or write_mask_array[i]==0 or read_mask_array[i]==0 continue;
 * data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i]^0xffffffff);
 * exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); compare data_rd==exp_val.
 * At end: if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).
 */

/* Bannered function prototypes */
static int chk_rst_val(void);
static int chk_rd_wr(void);

/*
 * Function: chk_rst_val
 * Purpose : Validate default/reset values for all impacted registers using read_mask and expected defaults.
 */
static int chk_rst_val(void)
{
    int def_fail_cnt = 0;
    for (unsigned int i = 0; i < CNT; ++i) {
        const unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1u) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping reset-check idx=%u due to skip_rst_array\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0u) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping reset-check idx=%u due to read_mask==0\n", i);
#endif
            continue;
        }
        if (!is_addr_impacted(addr)) {
#ifdef DEBUG_DISPLAY
            printf("[DBG] Skipping reset-check idx=%u because addr not in impacted list (0x%08lx)\n", i, addr);
#endif
            continue; /* Use only impacted registers */
        }
        unsigned int data_rd = read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEu); /* mask out bit0 as per description */
        unsigned int exp  = default_value_array[i];
        if (data != exp) {
            ++def_fail_cnt;
#ifdef DEBUG_DISPLAY
            printf("[ERR] Default mismatch idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x\n", i, addr, data, exp);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("[OK ] Default match idx=%u addr=0x%08lx val=0x%08x\n", i, addr, data);
#endif
        }
    }
    return def_fail_cnt;
}

/*
 * Function: chk_rd_wr
 * Purpose : Perform write/readback tests for all impacted registers using provided patterns and masks.
 */
static int chk_rd_wr(void)
{
    int wr_fail_cnt = 0;
    for (unsigned int j = 0; j < (sizeof(chk_val)/sizeof(chk_val[0])); ++j) {
        unsigned int data_wr = chk_val[j];
        /* Write phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            const unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u) continue;
            if (write_mask_array[i] == 0u) continue;
            if (!is_addr_impacted(addr)) continue; /* Use only impacted registers */
            unsigned int wrv = (data_wr & write_mask_array[i]);
            write_reg(addr, wrv);
#ifdef DEBUG_DISPLAY
            printf("[WR ] idx=%u addr=0x%08lx data_wr=0x%08x (mask=0x%08x)\n", i, addr, wrv, write_mask_array[i]);
#endif
        }
        /* Readback/compare phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            const unsigned long addr = addr_array[i];
            if (skip_array[i] == 1u) continue;
            if (write_mask_array[i] == 0u) continue;
            if (read_mask_array[i] == 0u) continue;
            if (!is_addr_impacted(addr)) continue; /* Use only impacted registers */

            unsigned int data_rd = (read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                ++wr_fail_cnt;
#ifdef DEBUG_DISPLAY
                printf("[ERR] R/W mismatch idx=%u addr=0x%08lx rd=0x%08x exp=0x%08x pat=0x%08x rm=0x%08x wm=0x%08x\n",
                       i, addr, data_rd, exp_val, data_wr, read_mask_array[i], write_mask_array[i]);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("[OK ] R/W match idx=%u addr=0x%08lx rd=0x%08x\n", i, addr, data_rd);
#endif
            }
        }
    }
    return wr_fail_cnt;
}

/*
 * Function: test_case
 * Purpose : Entry point that sequences default value checks followed by R/W verification, and reports pass/fail.
 */
void test_case(void)
{
    int def_fail = chk_rst_val();
    int wr_fail  = chk_rd_wr();

#ifdef DEBUG_DISPLAY
    printf("[SUM] def_fail=%d wr_fail=%d\n", def_fail, wr_fail);
#endif

    if (def_fail > 0 || wr_fail > 0) {
        finish(1); /* FAIL */
    } else {
        finish(0); /* PASS */
    }
}
