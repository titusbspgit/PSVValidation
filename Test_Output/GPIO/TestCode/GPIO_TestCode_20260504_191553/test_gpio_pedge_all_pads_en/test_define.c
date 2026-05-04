#ifndef TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_C
#define TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_C

// Headers (unchanged from context)
#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c>  // self-include guarded
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Defines (unchanged from context)
#define CNT 49

// Optional skip array (from context); used for conditional per-pin operations
static const unsigned int skip_array[CNT] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0
};

#endif // TEST_GPIO_PEDGE_ALL_PADS_EN_DEFINE_C
