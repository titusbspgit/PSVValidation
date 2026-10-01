// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "lpddr4_mem_dm_test.h"
#include "test_define.inc"

static unsigned int err0;
static unsigned long int exp_data_array[50];

#define PHASE1_COUNT     10U
#define PHASE1_STRIDE    0x1000UL
#define PHASE2_COUNT     32U
#define PHASE2_STRIDE    0x20000000UL
