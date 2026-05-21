#include <stdio.h>
#include <lss_sysreg.h>
"test_define.c"
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

#define CNT 49

/* Impacted Registers (from Meta Impacted Registers) */
const unsigned long int GPIO_BASE_PAD_REG = MIZAR_GPIO_GP0_GPIO_8;             /* Base register for GPIO_8 */
const unsigned long int REG_GPIO_INTR_EN1 = MIZAR_GPIO_GP0_INTR1_INTR_EN1;     /* Per-pad interrupt enable register */
const unsigned long int REG_GPIO_INTR_STS1 = MIZAR_GPIO_GP0_INTR1_INTR_STS1;   /* Per-pad interrupt status register */
const unsigned long int REG_GPIO_INTR_RAW_STCLR1 = MIZAR_GPIO_GPIO_INTR_RAW_STCLR1; /* Per-pad raw interrupt clear register */
const unsigned long int REG_SYSREG_INTR_EN1 = MIZAR_LSS_SYSREG_INTR_EN1;       /* System-level interrupt enable register */
const unsigned long int REG_SYSREG_RAW_STCR1 = MIZAR_LSS_SYSREG_RAW_STCR1;     /* System-level raw status clear register */

/* Skip Registers array (no skip entries provided in Meta Arrays) */
const unsigned int skip_registers[1] = { 0u };
