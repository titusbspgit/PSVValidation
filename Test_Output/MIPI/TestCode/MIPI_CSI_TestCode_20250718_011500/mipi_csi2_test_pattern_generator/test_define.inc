// Author - AI Force 2.3. 18-Jul-2025 01:15 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Purpose   : Supporting definitions for MIPI CSI-2 test pattern generator
 *             testcase including headers, macros, and testcase constants.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan */
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000

/* Timeout constant for polling loops */
#define MIPI_CSI2_POLL_TIMEOUT       100000

/* DV wait_on() replacement constants */
#define MIPI_CSI2_WAIT_PG_COMPLETE   1000
#define MIPI_CSI2_WAIT_SETTLE        100
