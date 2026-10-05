// Author - AI Force 2.3. 01-Oct-2026 13:31 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Defines headers, macros, and supporting data for the MIPI CSI-2
 *          PPI Pattern Generator testcase.
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
#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2    0xE6002000U
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2    0xE6001000U
#endif

/* DMA register offsets */
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x28U
#endif

/* Polling timeout constant */
#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT       100000U
#endif

/* PHY stop state expected value */
#define MIPI_CSI2_PHY_STOPSTATE_EXPECTED  0x1000fU

/* Arrays: NA (no arrays supplied in Meta TestPlan) */
