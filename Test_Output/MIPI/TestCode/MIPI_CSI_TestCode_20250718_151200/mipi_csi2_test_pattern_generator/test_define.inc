// Author - AI Force 2.3. 18-Jul-2025 15:12 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose:  Supporting definitions for MIPI CSI-2 test pattern generator
 *           testcase.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* --------------------------------------------------------------------------
 * Macros from Meta TestPlan JSON
 * -------------------------------------------------------------------------- */
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000
#define VC_ID                       3
