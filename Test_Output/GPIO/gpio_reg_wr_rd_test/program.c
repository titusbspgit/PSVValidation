// Author - AI Force 1.3.2. Date 05-05-2026
// (EMBENGG-SYSAPPS)

/* gpio_reg_wr_rd_test: 
   Description: Checks default values of GPIO-related registers and verifies masked write/read behavior across multiple registers using defined patterns.
   Acceptance Criteria:
   - In chk_rst_val: (data_rd & 0xfffffffe) must equal default_value_array[i]; else def_fail_cnt++.
   - In chk_rd_wr: data_rd (masked) must equal exp_val; else wr_fail_cnt++.
   - Final: finish(0) if both counters zero; otherwise finish(1).
*/

#include "test_define.c"

/* Banner: Functions implement the Test Steps Procedure AS-IS.
   Utilities assumed from test_common.h: read_reg, write_reg, wait_on, finish. */

static unsigned int def_fail_cnt = 0;
static unsigned int wr_fail_cnt  = 0;

/* Function: chk_rst_val
   Purpose: Verify reset/default values across all impacted registers.
   Method: For each register i in [0..CNT-1], if skip_rst_array[i] == 1 or read_mask_array[i] == 0, skip.
           Otherwise read and compare (data_rd & 0xfffffffe) to default_value_array[i]. */
static void chk_rst_val(void)
{
    for (unsigned int i = 0; i < CNT; ++i) {
        unsigned long addr = addr_array[i];
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("[RST] Skipping index %u (%s) per skip_rst_array\n", i, (i < CNT ? impacted_registers[i] : "NA"));
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
            printf("[RST] Skipping index %u (%s) due to read_mask==0\n", i, (i < CNT ? impacted_registers[i] : "NA"));
#endif
            continue;
        }
        unsigned int data_rd = (unsigned int)read_reg(addr);
        unsigned int data = (data_rd & 0xFFFFFFFEU);
        unsigned int exp  = default_value_array[i];
        if (data != exp) {
            def_fail_cnt++;
            printf("RST : Failed Default value mismatch Addr :0x%lx Expected : 0x%x\tRead_data : 0x%x\tDATA : 0x%x\n",
                   addr, exp, data_rd, data);
        } else {
#ifdef DEBUG_DISPLAY
            printf("[RST] OK Addr:0x%lx Exp:0x%x Read:0x%x\n", addr, exp, data_rd);
#endif
        }
    }
}

/* Function: chk_rd_wr
   Purpose: Perform masked write/read verification across all impacted registers using patterns.
   Method: For each pattern in chk_val[], write masked value to each non-skipped, writeable register.
           Then read back with read_mask and compare against expected value using default preservation for non-writeable bits. */
static void chk_rd_wr(void)
{
    for (unsigned int j = 0; j < 6U; ++j) {
        unsigned int data_wr = chk_val[j];
        /* Write phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
                printf("[WR ] Skip index %u (%s) per skip_array\n", i, (i < CNT ? impacted_registers[i] : "NA"));
#endif
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
#ifdef DEBUG_DISPLAY
                printf("[WR ] Skip index %u (%s) due to write_mask==0\n", i, (i < CNT ? impacted_registers[i] : "NA"));
#endif
                continue;
            }
            unsigned int wr_val = (data_wr & write_mask_array[i]);
            write_reg(addr, wr_val);
#ifdef DEBUG_DISPLAY
            printf("[WR ] Addr:0x%lx DataWr:0x%x Mask:0x%x\n", addr, wr_val, write_mask_array[i]);
#endif
        }
        /* Read/compare phase */
        for (unsigned int i = 0; i < CNT; ++i) {
            unsigned long addr = addr_array[i];
            if (skip_array[i] == 1U) {
                continue;
            }
            if (write_mask_array[i] == 0x00000000U) {
                continue;
            }
            if (read_mask_array[i] == 0x00000000U) {
                continue;
            }
            unsigned int data_rd = ((unsigned int)read_reg(addr) & read_mask_array[i]);
            unsigned int wr_n    = (write_mask_array[i] ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n    & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
                printf("Read_write : Failed : Write Read mismatch For Address %lx, Expected value=0x%x\tRead value=0x%x\n",
                       addr, exp_val, data_rd);
            } else {
#ifdef DEBUG_DISPLAY
                printf("[RD ] OK Addr:0x%lx Exp:0x%x Rd:0x%x\n", addr, exp_val, data_rd);
#endif
            }
        }
    }
}

/* Function: test_case
   Purpose: Orchestrate reset verification and read/write verification and apply pass/fail per acceptance criteria. */
static void test_case(void)
{
    /* 4.1) Call chk_rst_val() */
    chk_rst_val();
    /* 4.2) Call chk_rd_wr() */
    chk_rd_wr();
    /* 4.3) If (def_fail_cnt > 0 || wr_fail_cnt > 0): finish(1) else finish(0). */
    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1);
    } else {
        finish(0);
    }
}

/* Entry */
int main(void)
{
    test_case();
    return 0;
}
