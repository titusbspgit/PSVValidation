#ifndef TEST_GPIO_NEGEDGE_INTR_EN_TEST_DEFINE_C
#define TEST_GPIO_NEGEDGE_INTR_EN_TEST_DEFINE_C

/* Meta Headers (unchanged) */
#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Meta Macros (unchanged) */
#define CNT 49

/* RAG-mapped register arrays from Impacted Registers */
/* GPIO pad registers: MIZAR_GPIO_GP0_GPIO_8 + i*4 for i=0..31 */
static const unsigned int gpio_pad_addr[32] = {
    (MIZAR_GPIO_GP0_GPIO_8 + 0x00U), (MIZAR_GPIO_GP0_GPIO_8 + 0x04U), (MIZAR_GPIO_GP0_GPIO_8 + 0x08U), (MIZAR_GPIO_GP0_GPIO_8 + 0x0CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x10U), (MIZAR_GPIO_GP0_GPIO_8 + 0x14U), (MIZAR_GPIO_GP0_GPIO_8 + 0x18U), (MIZAR_GPIO_GP0_GPIO_8 + 0x1CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x20U), (MIZAR_GPIO_GP0_GPIO_8 + 0x24U), (MIZAR_GPIO_GP0_GPIO_8 + 0x28U), (MIZAR_GPIO_GP0_GPIO_8 + 0x2CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x30U), (MIZAR_GPIO_GP0_GPIO_8 + 0x34U), (MIZAR_GPIO_GP0_GPIO_8 + 0x38U), (MIZAR_GPIO_GP0_GPIO_8 + 0x3CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x40U), (MIZAR_GPIO_GP0_GPIO_8 + 0x44U), (MIZAR_GPIO_GP0_GPIO_8 + 0x48U), (MIZAR_GPIO_GP0_GPIO_8 + 0x4CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x50U), (MIZAR_GPIO_GP0_GPIO_8 + 0x54U), (MIZAR_GPIO_GP0_GPIO_8 + 0x58U), (MIZAR_GPIO_GP0_GPIO_8 + 0x5CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x60U), (MIZAR_GPIO_GP0_GPIO_8 + 0x64U), (MIZAR_GPIO_GP0_GPIO_8 + 0x68U), (MIZAR_GPIO_GP0_GPIO_8 + 0x6CU),
    (MIZAR_GPIO_GP0_GPIO_8 + 0x70U), (MIZAR_GPIO_GP0_GPIO_8 + 0x74U), (MIZAR_GPIO_GP0_GPIO_8 + 0x78U), (MIZAR_GPIO_GP0_GPIO_8 + 0x7CU)
};

/* Control registers: index mapping maintained for readability */
enum { IDX_RAW_STCLR1 = 0, IDX_INTR_EN1 = 1, IDX_INTR_STS1 = 2 };
static const unsigned int gpio_ctrl_regs[3] = {
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,  /* gpio_intr_raw_stclr1 */
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,    /* intr1_intr_en1 */
    MIZAR_GPIO_GP0_INTR1_INTR_STS1    /* intr1_intr_sts1 */
};

/* System registers: index mapping maintained for readability */
enum { IDX_SYS_INTR_EN1 = 0, IDX_SYS_RAW_STCR1 = 1 };
static const unsigned int sysreg_regs[2] = {
    MIZAR_LSS_SYSREG_INTR_EN1,        /* system interrupt enable */
    MIZAR_LSS_SYSREG_RAW_STCR1        /* system raw status clear */
};

/* External pad drive/data register address from Meta Steps */
static const unsigned int PAD_DATA_REG = 0xA0243ffcU;

/* Shared globals for ISR <-> main coordination */
volatile int int_pend = 0;            /* 1 => waiting for interrupt; ISR will clear to 0 */
volatile unsigned int g_isr_i = 0;    /* current GPIO bit index for ISR context */
volatile unsigned int isr_local_wr = 0; /* cached (1U << g_isr_i) for debug */
volatile int test_err = 0;            /* accumulated errors */

/* Skip arrays from Meta Arrays (skip-related only) */
static const unsigned int skip_array[20] = {
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U
};

#endif /* TEST_GPIO_NEGEDGE_INTR_EN_TEST_DEFINE_C */
