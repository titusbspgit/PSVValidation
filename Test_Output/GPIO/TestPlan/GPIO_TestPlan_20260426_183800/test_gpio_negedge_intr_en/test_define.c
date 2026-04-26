#ifndef TEST_DEFINE_TEST_GPIO_NEGEDGE_INTR_EN_C
#define TEST_DEFINE_TEST_GPIO_NEGEDGE_INTR_EN_C

/* Headers from context (unchanged) */
#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Macros from context (unchanged) */
#define CNT 49

/* Impacted registers list (AS-IS from Hidden_Impacted_Registers) */
const unsigned long impacted_reg_addr[] = {
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11,
    MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15,
    MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19,
    MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23,
    MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27,
    MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31,
    MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35,
    MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2,
    MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4,
    MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2,
    MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4
};
const unsigned int impacted_reg_addr_count = sizeof(impacted_reg_addr)/sizeof(impacted_reg_addr[0]);

/* Optional addr list (49 entries) – used for per-GPIO config loops */
const unsigned long gp_addr_array[32] = {
    MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11,
    MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15,
    MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19,
    MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23,
    MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27,
    MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31,
    MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35,
    MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39
};

/* Skip array – not provided explicitly; default to none skipped */
const unsigned int skip_array[49] = {0};

/* Helper to ensure only impacted registers are accessed */
static inline int is_addr_impacted(unsigned long addr)
{
    for (unsigned int k = 0; k < impacted_reg_addr_count; ++k) {
        if (impacted_reg_addr[k] == addr) return 1;
    }
    return 0;
}

static inline unsigned int read_if_impacted(unsigned long addr)
{
    if (is_addr_impacted(addr)) return read_reg(addr);
    return 0u;
}

static inline void write_if_impacted(unsigned long addr, unsigned int val)
{
    if (is_addr_impacted(addr)) write_reg(addr, val);
}

#endif /* TEST_DEFINE_TEST_GPIO_NEGEDGE_INTR_EN_C */
