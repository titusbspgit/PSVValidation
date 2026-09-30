// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Validates the MIPI CSI2 internal test pattern generator (PPI PG)
 *   functionality including pattern generator configuration, DMA data path
 *   verification, PHY stop state polling, and interrupt masking.
 */

#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros field: NA - no testcase-specific macros provided */

/* Arrays field: NA - no testcase-specific arrays provided */
