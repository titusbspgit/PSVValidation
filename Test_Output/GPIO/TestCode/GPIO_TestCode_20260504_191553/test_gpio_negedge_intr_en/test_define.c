#ifndef TEST_GPIO_NEGEDGE_INTR_EN_DEFINE_C
#define TEST_GPIO_NEGEDGE_INTR_EN_DEFINE_C

// Headers (unchanged from context)
#include <stdio.h>
#include <lss_sysreg.h>
#include "test_define.c"  // self-include guarded
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Defines (unchanged from context)
#define CNT 49

// Optional skip array (from context); used to conditionally skip indices if ever required
static const unsigned int skip_array[CNT] = {
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0
};

// Helper compile-time selection; exactly one of GPIO0 or GPIO1 should be defined by build system.
// No defaults are forced here to keep behavior deterministic per build configuration.

#endif // TEST_GPIO_NEGEDGE_INTR_EN_DEFINE_C
