#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Defines (from context, unchanged) */
#define CNT 49

/*
 * Impacted registers array (from Hidden_Impacted_Registers)
 * Note: Order preserved as provided by metadata.
 */
const unsigned long int impacted_regs[] = {
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,
    MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
    MIZAR_LSS_SYSREG_INTR_EN1,
    MIZAR_LSS_SYSREG_RAW_STCR1
};
const unsigned int impacted_regs_count = 6U;

/*
 * Skip registers (from context skip_registers). None specified => empty.
 */
const unsigned long int skip_regs[] = { };
const unsigned int skip_regs_count = 0U;

/*
 * External platform I/O and test harness APIs.
 * Using generic types to avoid additional headers.
 */
extern void write_reg(unsigned int addr, unsigned int val);
extern unsigned int read_reg(unsigned int addr);
extern void wait_on(unsigned int cycles);
extern void finish(unsigned int status);
