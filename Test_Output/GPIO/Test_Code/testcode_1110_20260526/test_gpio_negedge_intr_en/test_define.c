// test_define.c: Auto-generated definitions for test_gpio_negedge_intr_en

// Headers: Meta provided none; include standard and platform headers required by template
#include <stdio.h>
#include <stdint.h>
#include "test_common.h"
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Macros: None provided in Meta for this testcase

// Impacted registers and arrays (derived from Meta description + RAG mapping)
// Use RAG-provided macro for GPIO_8 as base and enumerate +4 per pin

// External pad output register used to drive edges per Meta description
#define PAD_OUT_REG_ADDR (0xA0243ffcUL)

// Per-pin control register addresses for GP0 GPIO[8..39]
const uintptr_t gp0_pin_reg_addr[32] = {
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x00U), // 8
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x04U), // 9
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x08U), // 10
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x0CU), // 11
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x10U), // 12
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x14U), // 13
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x18U), // 14
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x1CU), // 15
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x20U), // 16
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x24U), // 17
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x28U), // 18
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x2CU), // 19
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x30U), // 20
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x34U), // 21
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x38U), // 22
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x3CU), // 23
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x40U), // 24
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x44U), // 25
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x48U), // 26
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x4CU), // 27
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x50U), // 28
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x54U), // 29
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x58U), // 30
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x5CU), // 31
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x60U), // 32
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x64U), // 33
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x68U), // 34
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x6CU), // 35
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x70U), // 36
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x74U), // 37
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x78U), // 38
    (mizar_GPIO_GP0_GPIO_8_REG  + 0x7CU)  // 39
};

// Group control/status registers per RAG mapping
const uintptr_t gp0_rawstcr1_reg     = mizar_GPIO_GP0_RAWSTCR1_REG;
const uintptr_t gp0_intr_en1_reg     = mizar_GPIO_GP0_INTR1_INTR_EN1_REG;
const uintptr_t gp0_intr_sts1_reg    = mizar_GPIO_GP0_INTR1_INTR_STS1_REG;

// Skip array: Meta Arrays = NA; provide zeroed skip array to indicate no skips specified
const unsigned int skip_array[32] = {
    0U,0U,0U,0U, 0U,0U,0U,0U, 0U,0U,0U,0U, 0U,0U,0U,0U,
    0U,0U,0U,0U, 0U,0U,0U,0U, 0U,0U,0U,0U, 0U,0U,0U,0U
};
