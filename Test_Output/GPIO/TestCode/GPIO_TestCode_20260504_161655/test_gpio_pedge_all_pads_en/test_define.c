#include <stdio.h>
#include <stdint.h>
#include "test_common.h"
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

#define CNT 49
#ifndef MIZAR_PAD_CTRL_GPIO
#define MIZAR_PAD_CTRL_GPIO 0xA0243ffc
#endif
#ifndef LSS_SYSREG_INTR_EN1_GPIO0_INTR
#define LSS_SYSREG_INTR_EN1_GPIO0_INTR 0x00000001
#endif
#ifndef LSS_SYSREG_INTR_EN1_GPIO1_INTR
#define LSS_SYSREG_INTR_EN1_GPIO1_INTR 0x00000002
#endif
#ifndef LSS_SYSREG_RAW_STCR1_GPIO0_INTR
#define LSS_SYSREG_RAW_STCR1_GPIO0_INTR 0x00000001
#endif
#ifndef LSS_SYSREG_RAW_STCR1_GPIO1_INTR
#define LSS_SYSREG_RAW_STCR1_GPIO1_INTR 0x00000002
#endif
#ifndef GPIO0_IRQ_ID
#define GPIO0_IRQ_ID 87
#endif
#ifndef GPIO1_IRQ_ID
#define GPIO1_IRQ_ID 88
#endif

/* Impacted registers for test_gpio_pedge_all_pads_en (from Hidden_Impacted_Registers plus approved pad control) */
static const uint32_t impacted_registers[] = {
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_LSS_SYSREG_RAW_STCR1,
    MIZAR_PAD_CTRL_GPIO
};

/* No skipped registers specified */
static const uint32_t skip_registers[] = { };
