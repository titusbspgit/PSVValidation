// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: lpddr4_mem_quarter_data_bus_width_test
 * Purpose: Verifies LPDDR4 memory read-back correctness when operating in
 * quarter data bus width mode. Validates read-back at offsets 0x100
 * and 0x108 against expected patterns 0x3333333333333333 and
 * 0x2222222222222222 for port0 (conditional) and port1 (unconditional).
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

/* Read offsets for quarter bus width verification */
#define LPDDR4_OFFSET_0 0x100UL
#define LPDDR4_OFFSET_1 0x108UL

/* Expected data patterns for quarter bus width read-back */
#define LPDDR4_EXPECTED_DATA0 0x3333333333333333ULL
#define LPDDR4_EXPECTED_DATA1 0x2222222222222222ULL

/* Macros: NA - no additional testcase-specific macros were supplied */

/* Arrays: NA - no testcase-specific arrays were supplied */
