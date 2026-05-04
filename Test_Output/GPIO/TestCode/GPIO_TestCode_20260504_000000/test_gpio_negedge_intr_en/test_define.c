#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

#define CNT 49

/* Impacted registers as provided by metadata (for reference/debug). */
static const char* impacted_regs[] = {
    "MIZAR_LSS_SYSREG_INTR_EN1",
    "MIZAR_GPIO_GP0_GPIO_8",
    "MIZAR_GPIO_GPIO_INTR_RAW_STCLR1",
    "MIZAR_GPIO_GP0_INTR1_INTR_EN1",
    "MIZAR_GPIO_GP0_INTR1_INTR_STS1",
    "MIZAR_LSS_SYSREG_RAW_STCR1"
};
