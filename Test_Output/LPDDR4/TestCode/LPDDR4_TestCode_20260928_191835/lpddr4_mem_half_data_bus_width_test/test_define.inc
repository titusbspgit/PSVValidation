// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_half_data_bus_width_test
 * Purpose: Validates LPDDR4 memory access in half data bus width mode.
 * Configures bus_width=1 (Half Bus), DBI_EN=0, DM_EN=0, performs
 * lpddr4_training(), training_done(), wait_on(1000), then reads
 * fixed addresses and validates expected data patterns
 * 0x3333333333333333 and 0x2222222222222222 across two phases.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* Macros: NA - no testcase-specific macros provided */

/* Arrays: NA - no testcase-specific arrays provided */
