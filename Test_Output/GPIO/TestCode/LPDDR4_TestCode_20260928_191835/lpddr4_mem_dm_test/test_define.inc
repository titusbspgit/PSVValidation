// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_dm_test
 * Purpose:   Validates LPDDR4 memory data mask (DM) functionality by
 *            performing write and read-back verification across two
 *            memory access patterns (Phase 1 conditional, Phase 2 unconditional).
 */

#include "test.h"

/* --------------------------------------------------------------------------
 * Meta TestPlan Headers
 * -------------------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* --------------------------------------------------------------------------
 * Macros
 * Note: No additional macros were specified in the Meta TestPlan JSON.
 * -------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------
 * Testcase Constants
 * -------------------------------------------------------------------------- */
#define LPDDR4_PHASE1_STRIDE        0x1000UL
#define LPDDR4_PHASE1_LOCATIONS     10U
#define LPDDR4_PHASE2_STRIDE        0x20000000UL
#define LPDDR4_PHASE2_LOCATIONS     32U

/* --------------------------------------------------------------------------
 * Arrays
 * -------------------------------------------------------------------------- */
static uint64_t exp_data_array[50];
