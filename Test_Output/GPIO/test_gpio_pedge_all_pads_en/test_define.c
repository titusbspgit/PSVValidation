#include <lss_sysreg.h>
#include <stdio.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Defines (from testcase context)
#define CNT 49

// High-level test constants (from CSV Memory Start/End Offsets and procedure)
#define GPIO_DRIVE_REG   (0xA0243FFC)   // External pad drive register (from CSV offsets)
#define GPIO_PINS        (32u)

// GPIO per-pin configuration values (from CSV procedure)
#define GPIO_POSEDGE_EN_VAL  (0x00020000u)  // bit17=1: enable positive-edge
#define GPIO_RAW_CLEAR_VAL   (0x00010000u)  // bit16=1: per-pin raw clear (W1C)

// Impacted register addresses (preserve macro names from headers)
static const unsigned long REG_LSS_INTR_EN1   = MIZAR_LSS_SYSREG_INTR_EN1;
static const unsigned long REG_LSS_RAW_STCR1  = MIZAR_LSS_SYSREG_RAW_STCR1;
static const unsigned long REG_GPIO_PIN_BASE  = MIZAR_GPIO_GP0_GPIO_8; // Use + (i*4)
static const unsigned long REG_GPIO_INTR1_EN1 = MIZAR_GPIO_GP0_INTR1_INTR_EN1;
static const unsigned long REG_GPIO_INTR1_STS1= MIZAR_GPIO_GP0_INTR1_INTR_STS1;

static const unsigned long REG_GPIO_IO_CTRL_GRP[4] = {
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP4
};

// Optional skip array (no skips specified for this testcase)
static const int skip_array[GPIO_PINS] = { 
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0
};
