#ifndef TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_H
#define TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_H

/* Headers (from context, unchanged) */
#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Defines (from context, unchanged) */
#define CNT 49

/* Impacted registers list (subset provided) */
const unsigned long int impacted_registers[] = {
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_LSS_SYSREG_RAW_STCR1
};

#endif /* TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_H */
