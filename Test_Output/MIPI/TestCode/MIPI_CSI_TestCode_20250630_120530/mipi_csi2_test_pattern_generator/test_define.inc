// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case : mipi_csi2_test_pattern_generator
 * Purpose   : Supporting macros, headers, and testcase constants for
 *             MIPI CSI-2 PPI Pattern Generator validation.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

/* --------------------------------------------------------------------------
 * Macros from Meta TestPlan JSON
 * -------------------------------------------------------------------------- */

#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2       0xE6002000UL
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2       0xE6001000UL
#endif

/* --------------------------------------------------------------------------
 * DMA Register Offsets
 * -------------------------------------------------------------------------- */

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x028U
#endif

/* --------------------------------------------------------------------------
 * Testcase Constants
 * -------------------------------------------------------------------------- */

#ifndef PHY_STOPSTATE_EXPECTED
#define PHY_STOPSTATE_EXPECTED          0x1000fU
#endif

#ifndef MIPI_CSI2_GDMA_REG_BASE
#define MIPI_CSI2_GDMA_REG_BASE         0xE6A00000UL
#endif

#ifndef MIPI_CSI2_DMA_CH0_PC
#define MIPI_CSI2_DMA_CH0_PC            0xE6000000UL
#endif

#ifndef MIPI_CSI2_FRAC_DIV_OFFSET
#define MIPI_CSI2_FRAC_DIV_OFFSET       0xf4U
#endif

#ifndef MIPI_CSI2_DATA_TRNSFR_SIZE
#define MIPI_CSI2_DATA_TRNSFR_SIZE      15360U
#endif

#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT          100000U
#endif

/* --------------------------------------------------------------------------
 * Arrays: NA (none supplied in Meta TestPlan JSON)
 * -------------------------------------------------------------------------- */
