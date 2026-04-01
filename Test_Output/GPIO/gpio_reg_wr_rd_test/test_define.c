#ifndef TEST_DEFINE_C
#define TEST_DEFINE_C

// Note: All headers and defines are placed here unchanged and before any other code.
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_define.c"  // kept as-is from extracted includes; guarded by include-guard
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// Extracted defines (unchanged)
#define SOFT_RST_REG_ADDRESS 0x00000000
#define SOFT_RST_REG_DATA 0x00000000
#define CNT 49

// -----------------------------------------------------------------------------
// Arrays expected by the testcase (must come from Impacted_Register_Spec_RAG)
// IMPORTANT: The following arrays are intentionally zero-initialized placeholders.
// Do NOT rely on them. They must be populated strictly from Impacted_Register_Spec_RAG.
// A runtime guard in program.c will fail the test if these remain placeholders.
// -----------------------------------------------------------------------------
const unsigned int addr_array[CNT]           = {0};
const unsigned int read_mask_array[CNT]      = {0};
const unsigned int write_mask_array[CNT]     = {0};
const unsigned int default_value_array[CNT]  = {0};

// -----------------------------------------------------------------------------
// Skip arrays (verbatim from testcase context)
// comment: //80,94,98,9c,a0,a4,a8,ac,b0...SKIPPING VRRW registers
// -----------------------------------------------------------------------------
const unsigned int skip_array[49]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
};

const unsigned int skip_rst_array[49]={
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,
};

#endif // TEST_DEFINE_C
