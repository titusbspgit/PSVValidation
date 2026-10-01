// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_half_data_bus_width_test.h"
#include "test_define.inc"

static unsigned int err0;

#define EXPECTED_PATTERN_0x100  0x3333333333333333UL
#define EXPECTED_PATTERN_0x108  0x2222222222222222UL
#define OFFSET_0x100  0x100UL
#define OFFSET_0x108  0x108UL
