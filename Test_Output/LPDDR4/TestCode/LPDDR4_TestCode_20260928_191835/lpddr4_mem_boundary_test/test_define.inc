// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_boundary_test
 * Purpose: Verifies LPDDR4 memory data integrity at boundary address ranges
 *          up to 2GB and 16GB limits with Phase 1 (port0/port1, boundary
 *          val_2g=0x80000000) and Phase 2 (port1, boundary
 *          val_16g=0x400000000).
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* Step 5: Boundary values */
#define LPDDR4_VAL_2G 0x0000000080000000ULL
#define LPDDR4_VAL_16G 0x0000000400000000ULL

/* Macros: NA - no additional testcase-specific macros were supplied */

/* Arrays */
// exp_data_array stores expected 64-bit data for write/read-back comparison.
// Size 150 as specified in the Meta TestPlan JSON.
static unsigned long exp_data_array[150];
