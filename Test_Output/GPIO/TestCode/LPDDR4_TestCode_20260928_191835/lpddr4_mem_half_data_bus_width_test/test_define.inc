// Author - AI Force 2.3. 28-Sep-2026 19:18 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_half_data_bus_width_test
 * Purpose:   Validates LPDDR4 memory access in half data bus width mode.
 *            Reads fixed addresses and verifies against expected patterns
 *            0x3333333333333333 and 0x2222222222222222.
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
#define LPDDR4_READ_OFFSET_0        0x100UL
#define LPDDR4_READ_OFFSET_1        0x108UL
#define LPDDR4_EXPECTED_DATA_0      0x3333333333333333ULL
#define LPDDR4_EXPECTED_DATA_1      0x2222222222222222ULL

/* --------------------------------------------------------------------------
 * Arrays
 * Note: No arrays were specified in the Meta TestPlan JSON.
 * -------------------------------------------------------------------------- */
