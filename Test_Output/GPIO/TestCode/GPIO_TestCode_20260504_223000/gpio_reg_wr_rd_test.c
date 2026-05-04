// Author - AI Force 1.3.2. Date 04-05-2026
// (EMBENGG-SYSAPPS)

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <test_define.c>
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// gpio_reg_wr_rd_test
// Description (from Hidden_Test_Description):
// Program exercises two phases: (1) Default value check for all entries in addr_array[]
// subject to skip_rst_array[] and read_mask_array[]; (2) Write/read check using six patterns
// for all entries in addr_array[] subject to skip_array[], write_mask_array[], and read_mask_array[].
// In chk_rst_val(): for i=0..CNT-1, if skip_rst_array[i]==1 then continue; if read_mask_array[i]==0x00000000 then continue;
// data_rd=read_reg(addr_array[i]); data=(data_rd & 0xfffffffe); compare data == default_value_array[i]; on mismatch, def_fail_cnt++.
// In chk_rd_wr(): for each data pattern in chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}, write phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; write_reg(addr_array[i], (data_wr & write_mask_array[i])).
// Read/verify phase: for i, if skip_array[i]==1 continue; if write_mask_array[i]==0x0 continue; if read_mask_array[i]==0x0 continue; data_rd = (read_reg(addr_array[i]) & read_mask_array[i]);
// wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd != exp_val) wr_fail_cnt++.
// At end of test_case(): finish(1) if (def_fail_cnt>0 || wr_fail_cnt>0) else finish(0).

#define CNT 49

// Impacted register address list (macros provided by headers)
static const unsigned long addr_array[CNT] = {
    MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11,
    MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15,
    MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19,
    MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23,
    MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27,
    MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31,
    MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35,
    MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39,
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2,
    MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4,
    MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2,
    MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4
};

// Default/reset values per register (as macros from gpio_def.h)
static const unsigned int default_value_array[CNT] = {
    GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL, GPIO_GP0_GPIO_11_DEFAULT_VAL,
    GPIO_GP0_GPIO_12_DEFAULT_VAL, GPIO_GP0_GPIO_13_DEFAULT_VAL, GPIO_GP0_GPIO_14_DEFAULT_VAL, GPIO_GP0_GPIO_15_DEFAULT_VAL,
    GPIO_GP0_GPIO_16_DEFAULT_VAL, GPIO_GP0_GPIO_17_DEFAULT_VAL, GPIO_GP0_GPIO_18_DEFAULT_VAL, GPIO_GP0_GPIO_19_DEFAULT_VAL,
    GPIO_GP0_GPIO_20_DEFAULT_VAL, GPIO_GP0_GPIO_21_DEFAULT_VAL, GPIO_GP0_GPIO_22_DEFAULT_VAL, GPIO_GP0_GPIO_23_DEFAULT_VAL,
    GPIO_GP0_GPIO_24_DEFAULT_VAL, GPIO_GP0_GPIO_25_DEFAULT_VAL, GPIO_GP0_GPIO_26_DEFAULT_VAL, GPIO_GP0_GPIO_27_DEFAULT_VAL,
    GPIO_GP0_GPIO_28_DEFAULT_VAL, GPIO_GP0_GPIO_29_DEFAULT_VAL, GPIO_GP0_GPIO_30_DEFAULT_VAL, GPIO_GP0_GPIO_31_DEFAULT_VAL,
    GPIO_GP0_GPIO_32_DEFAULT_VAL, GPIO_GP0_GPIO_33_DEFAULT_VAL, GPIO_GP0_GPIO_34_DEFAULT_VAL, GPIO_GP0_GPIO_35_DEFAULT_VAL,
    GPIO_GP0_GPIO_36_DEFAULT_VAL, GPIO_GP0_GPIO_37_DEFAULT_VAL, GPIO_GP0_GPIO_38_DEFAULT_VAL, GPIO_GP0_GPIO_39_DEFAULT_VAL,
    GPIO_GPIO_INTR_RAW_STCLR1_DEFAULT_VAL,
    GPIO_GP0_INTR1_INTR_EN1_DEFAULT_VAL, GPIO_GP0_INTR1_INTR_STS1_DEFAULT_VAL,
    GPIO_GP0_INTR2_INTR_EN1_DEFAULT_VAL, GPIO_GP0_INTR2_INTR_STS1_DEFAULT_VAL,
    GPIO_GPIO_IO_CTRL_GROUP1_DEFAULT_VAL, GPIO_GPIO_IO_CTRL_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_IO_CTRL_GROUP3_DEFAULT_VAL, GPIO_GPIO_IO_CTRL_GROUP4_DEFAULT_VAL,
    GPIO_GPIO_DOUT_GROUP1_DEFAULT_VAL, GPIO_GPIO_DOUT_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_DOUT_GROUP3_DEFAULT_VAL, GPIO_GPIO_DOUT_GROUP4_DEFAULT_VAL,
    GPIO_GPIO_DIN_GROUP1_DEFAULT_VAL, GPIO_GPIO_DIN_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_DIN_GROUP3_DEFAULT_VAL, GPIO_GPIO_DIN_GROUP4_DEFAULT_VAL
};

// Read/Write masks per register (as macros from gpio_def.h)
static const unsigned int read_mask_array[CNT] = {
    GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK, GPIO_GP0_GPIO_11_READ_MASK,
    GPIO_GP0_GPIO_12_READ_MASK, GPIO_GP0_GPIO_13_READ_MASK, GPIO_GP0_GPIO_14_READ_MASK, GPIO_GP0_GPIO_15_READ_MASK,
    GPIO_GP0_GPIO_16_READ_MASK, GPIO_GP0_GPIO_17_READ_MASK, GPIO_GP0_GPIO_18_READ_MASK, GPIO_GP0_GPIO_19_READ_MASK,
    GPIO_GP0_GPIO_20_READ_MASK, GPIO_GP0_GPIO_21_READ_MASK, GPIO_GP0_GPIO_22_READ_MASK, GPIO_GP0_GPIO_23_READ_MASK,
    GPIO_GP0_GPIO_24_READ_MASK, GPIO_GP0_GPIO_25_READ_MASK, GPIO_GP0_GPIO_26_READ_MASK, GPIO_GP0_GPIO_27_READ_MASK,
    GPIO_GP0_GPIO_28_READ_MASK, GPIO_GP0_GPIO_29_READ_MASK, GPIO_GP0_GPIO_30_READ_MASK, GPIO_GP0_GPIO_31_READ_MASK,
    GPIO_GP0_GPIO_32_READ_MASK, GPIO_GP0_GPIO_33_READ_MASK, GPIO_GP0_GPIO_34_READ_MASK, GPIO_GP0_GPIO_35_READ_MASK,
    GPIO_GP0_GPIO_36_READ_MASK, GPIO_GP0_GPIO_37_READ_MASK, GPIO_GP0_GPIO_38_READ_MASK, GPIO_GP0_GPIO_39_READ_MASK,
    GPIO_GPIO_INTR_RAW_STCLR1_READ_MASK,
    GPIO_GP0_INTR1_INTR_EN1_READ_MASK, GPIO_GP0_INTR1_INTR_STS1_READ_MASK,
    GPIO_GP0_INTR2_INTR_EN1_READ_MASK, GPIO_GP0_INTR2_INTR_STS1_READ_MASK,
    GPIO_GPIO_IO_CTRL_GROUP1_READ_MASK, GPIO_GPIO_IO_CTRL_GROUP2_READ_MASK,
    GPIO_GPIO_IO_CTRL_GROUP3_READ_MASK, GPIO_GPIO_IO_CTRL_GROUP4_READ_MASK,
    GPIO_GPIO_DOUT_GROUP1_READ_MASK, GPIO_GPIO_DOUT_GROUP2_READ_MASK,
    GPIO_GPIO_DOUT_GROUP3_READ_MASK, GPIO_GPIO_DOUT_GROUP4_READ_MASK,
    GPIO_GPIO_DIN_GROUP1_READ_MASK, GPIO_GPIO_DIN_GROUP2_READ_MASK,
    GPIO_GPIO_DIN_GROUP3_READ_MASK, GPIO_GPIO_DIN_GROUP4_READ_MASK
};

static const unsigned int write_mask_array[CNT] = {
    GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK, GPIO_GP0_GPIO_11_WRITE_MASK,
    GPIO_GP0_GPIO_12_WRITE_MASK, GPIO_GP0_GPIO_13_WRITE_MASK, GPIO_GP0_GPIO_14_WRITE_MASK, GPIO_GP0_GPIO_15_WRITE_MASK,
    GPIO_GP0_GPIO_16_WRITE_MASK, GPIO_GP0_GPIO_17_WRITE_MASK, GPIO_GP0_GPIO_18_WRITE_MASK, GPIO_GP0_GPIO_19_WRITE_MASK,
    GPIO_GP0_GPIO_20_WRITE_MASK, GPIO_GP0_GPIO_21_WRITE_MASK, GPIO_GP0_GPIO_22_WRITE_MASK, GPIO_GP0_GPIO_23_WRITE_MASK,
    GPIO_GP0_GPIO_24_WRITE_MASK, GPIO_GP0_GPIO_25_WRITE_MASK, GPIO_GP0_GPIO_26_WRITE_MASK, GPIO_GP0_GPIO_27_WRITE_MASK,
    GPIO_GP0_GPIO_28_WRITE_MASK, GPIO_GP0_GPIO_29_WRITE_MASK, GPIO_GP0_GPIO_30_WRITE_MASK, GPIO_GP0_GPIO_31_WRITE_MASK,
    GPIO_GP0_GPIO_32_WRITE_MASK, GPIO_GP0_GPIO_33_WRITE_MASK, GPIO_GP0_GPIO_34_WRITE_MASK, GPIO_GP0_GPIO_35_WRITE_MASK,
    GPIO_GP0_GPIO_36_WRITE_MASK, GPIO_GP0_GPIO_37_WRITE_MASK, GPIO_GP0_GPIO_38_WRITE_MASK, GPIO_GP0_GPIO_39_WRITE_MASK,
    GPIO_GPIO_INTR_RAW_STCLR1_WRITE_MASK,
    GPIO_GP0_INTR1_INTR_EN1_WRITE_MASK, GPIO_GP0_INTR1_INTR_STS1_WRITE_MASK,
    GPIO_GP0_INTR2_INTR_EN1_WRITE_MASK, GPIO_GP0_INTR2_INTR_STS1_WRITE_MASK,
    GPIO_GPIO_IO_CTRL_GROUP1_WRITE_MASK, GPIO_GPIO_IO_CTRL_GROUP2_WRITE_MASK,
    GPIO_GPIO_IO_CTRL_GROUP3_WRITE_MASK, GPIO_GPIO_IO_CTRL_GROUP4_WRITE_MASK,
    GPIO_GPIO_DOUT_GROUP1_WRITE_MASK, GPIO_GPIO_DOUT_GROUP2_WRITE_MASK,
    GPIO_GPIO_DOUT_GROUP3_WRITE_MASK, GPIO_GPIO_DOUT_GROUP4_WRITE_MASK,
    GPIO_GPIO_DIN_GROUP1_WRITE_MASK, GPIO_GPIO_DIN_GROUP2_WRITE_MASK,
    GPIO_GPIO_DIN_GROUP3_WRITE_MASK, GPIO_GPIO_DIN_GROUP4_WRITE_MASK
};

// Skip arrays as specified in metadata/context
static const unsigned int skip_array[CNT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // GPIO_8..GPIO_39
    1, // GPIO_INTR_RAW_STCLR1 (skip during generic WR phase)
    0,0, // INTR1_EN1, INTR1_STS1
    0,0, // INTR2_EN1, INTR2_STS1
    1,1,1,1, // IO_CTRL_GROUP1..4 (skip writes in WR test)
    1,1,1,1, // DOUT_GROUP1..4 (skip writes in WR test)
    1,0,0,0  // DIN_GROUP1..4 (mostly RO; keep indices consistent)
};

static const unsigned int skip_rst_array[CNT] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // GPIO_8..GPIO_39
    0, // GPIO_INTR_RAW_STCLR1
    0,0, // INTR1_EN1, INTR1_STS1
    0,0, // INTR2_EN1, INTR2_STS1
    1,1,1,1, // IO_CTRL_GROUP1..4 (may be platform dependent)
    1,1,1,1, // DOUT_GROUP1..4
    1,1,1,1  // DIN_GROUP1..4
};

static unsigned int chk_val[6] = {
    0xFFFFFFFFu, 0xAAAAAAA Au, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
};

// Function: main
// Purpose: Execute default value verification and masked write/read validation for GPIO registers
int main(void)
{
    unsigned int def_fail_cnt = 0;
    unsigned int wr_fail_cnt = 0;

    // Phase 1: Default value check
    for (int i = 0; i < CNT; i++) {
        if (skip_rst_array[i]) continue;                    // Skip reset check if marked
        if (read_mask_array[i] == 0x00000000u) continue;    // Nothing readable; skip

        unsigned int data_rd = read_reg(addr_array[i]);     // Read register i
        unsigned int data = (data_rd & 0xFFFFFFFEu);        // Mask as per requirement
        if (data != default_value_array[i]) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("[DEF-CHK][IDX=%d][ADDR=0x%08lx] exp=0x%08x, rd=0x%08x\n",
                   i, addr_array[i], default_value_array[i], data);
#endif
        }
    }

    // Phase 2: Write/Read masked checks for each pattern
    for (int p = 0; p < 6; p++) {
        unsigned int data_wr = chk_val[p];

        // Write phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) continue;                    // Skip write if marked
            if (write_mask_array[i] == 0x00000000u) continue; // No write bits; skip
            unsigned int wval = (data_wr & write_mask_array[i]);
            write_reg(addr_array[i], wval);
#ifdef DEBUG_DISPLAY
            printf("[WR][P=%d][IDX=%d][ADDR=0x%08lx] w=0x%08x\n", p, i, addr_array[i], wval);
#endif
        }

        // Read/verify phase
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) continue;                      // Skip verify if marked
            if (write_mask_array[i] == 0x00000000u) continue; // Wasn't written; skip
            if (read_mask_array[i] == 0x00000000u) continue;  // Not readable; skip

            unsigned int data_rd = (read_reg(addr_array[i]) & read_mask_array[i]);
            unsigned int wr_n = (write_mask_array[i] ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) |
                                    (wr_n & read_mask_array[i] & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("[RD-CHK][P=%d][IDX=%d][ADDR=0x%08lx] exp=0x%08x rd=0x%08x rm=0x%08x wm=0x%08x\n",
                       p, i, addr_array[i], exp_val, data_rd, read_mask_array[i], write_mask_array[i]);
#endif
            }
        }
    }

    // Pass/Fail termination per acceptance criteria
    if ((def_fail_cnt > 0u) || (wr_fail_cnt > 0u)) {
        finish(1); // FAIL
    } else {
        finish(0); // PASS
    }
    return 0;
}
