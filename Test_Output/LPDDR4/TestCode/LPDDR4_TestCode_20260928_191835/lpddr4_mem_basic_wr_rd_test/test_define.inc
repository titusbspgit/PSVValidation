// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_basic_wr_rd_test
 * Purpose: Verifies basic LPDDR4 memory write and read operations using
 * 64-bit data patterns across two memory ports with Phase 1
 * (port0/port1, stride 0x1000, 10 locations) and Phase 2
 * (port1, stride 0x20000000, 32 locations).
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* Phase 1 configuration */
#define LPDDR4_PHASE1_STRIDE 0x1000UL
#define LPDDR4_PHASE1_LOCATIONS 10U

/* Phase 2 configuration */
#define LPDDR4_PHASE2_STRIDE 0x20000000UL
#define LPDDR4_PHASE2_LOCATIONS 32U

/* Total expected checks: Phase 1 = 10 port0 + 10 port1 = 20, Phase 2 = 32 */
#define LPDDR4_TOTAL_CHECKS (LPDDR4_PHASE1_LOCATIONS * 2U + LPDDR4_PHASE2_LOCATIONS)

/* Macros: NA - no additional testcase-specific macros were supplied */

/* Arrays */
// exp_data_array stores expected 64-bit data for write/read-back comparison.
static unsigned long exp_data_array[50];
