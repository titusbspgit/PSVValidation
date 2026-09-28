// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_dm_test
 * Purpose: Validates LPDDR4 memory data mask (DM) functionality by performing
 * write and read-back verification across two memory access patterns.
 * Configures DM_EN=1, DBI_EN=0, bus_width=0 (Full Bus), performs
 * lpddr4_training(), then executes Phase 1 (conditional) and Phase 2
 * (unconditional) write/read checks with random 64-bit data.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* Macros: NA - no testcase-specific macros provided */

/* Arrays */
static uint64_t exp_data_array[50];
