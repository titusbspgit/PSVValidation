// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Validates MIPI CSI2 D-PHY lane configuration by iterating through
 *   different lane counts, configuring registers, polling PHY stop state,
 *   reading interrupt status, and masking all CSI2 host interrupt sources.
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
