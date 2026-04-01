#ifndef TEST_DEFINE_C
#define TEST_DEFINE_C

// All header includes and defines are placed here unchanged and before any other code.
#include <lss_sysreg.h>
#include <stdio.h>
#include <test_define.c> // kept as-is from testcase context; guarded by include-guard
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Extracted defines (unchanged)
#define CNT 49

// Skip registers/symbols (from testcase context)
const int skip_array[49] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#endif // TEST_DEFINE_C
