/*
 * test_define.c — Context and definitions for test_gpio_negedge_intr_en
 * NOTE: Headers and macros are reproduced as-is from the testcase context.
 */

/* Headers (unchanged from context) */
#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Macros (unchanged from context) */
#define CNT 49

/*
 * Arrays
 * - impacted_registers: strictly the registers listed in Hidden_Impacted_Registers
 * - skip_registers: from context skip list (empty => count = 0)
 */
static const unsigned long impacted_registers[] = {
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_LSS_SYSREG_RAW_STCR1
};

/* Empty skip-list for this testcase (no registers to skip) */
static const unsigned long skip_registers[1] = { 0UL };

/* Counts for convenience */
static const unsigned int impacted_registers_count = (unsigned int)(sizeof(impacted_registers)/sizeof(impacted_registers[0]));
static const unsigned int skip_registers_count     = 0U;
