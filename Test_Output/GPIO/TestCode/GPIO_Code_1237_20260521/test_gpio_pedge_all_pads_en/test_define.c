#ifndef TEST_GPIO_PEDGE_ALL_PADS_EN_TEST_DEFINE_C
#define TEST_GPIO_PEDGE_ALL_PADS_EN_TEST_DEFINE_C

/* Meta Headers (unchanged) */
#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Meta Macros (unchanged) */
#define CNT 49

/* Impacted Registers array (from Meta Impacted Registers) */
/* Note: Using macros as provided by headers; no inference of addresses. */
const unsigned int impacted_registers[] = {
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_LSS_SYSREG_RAW_STCR1,
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1
};
const unsigned int impacted_registers_count = (sizeof(impacted_registers)/sizeof(impacted_registers[0]));

/* Skip Registers array (from Meta Arrays) */
unsigned int skip_array[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#endif /* TEST_GPIO_PEDGE_ALL_PADS_EN_TEST_DEFINE_C */
