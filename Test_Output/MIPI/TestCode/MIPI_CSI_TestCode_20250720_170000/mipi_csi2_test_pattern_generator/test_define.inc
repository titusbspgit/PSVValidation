// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Supporting definitions for MIPI CSI-2 internal test pattern
 *          generator validation testcase. Contains headers, macros,
 *          register addresses, and testcase constants.
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

/* Macros from Meta TestPlan JSON */
// MANUAL_REVIEW: The following macros were listed in the Meta TestPlan JSON
// without explicit values. Define them per the platform/IP configuration.
#ifndef GDMA_CSI2_DATA_DEST_ADDR2
// #define GDMA_CSI2_DATA_DEST_ADDR2  /* MANUAL_REVIEW: value not provided */
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
// #define GDMA_CTRL_DATA_DEST_ADDR2  /* MANUAL_REVIEW: value not provided */
#endif

#ifndef VC_ID
// #define VC_ID  /* MANUAL_REVIEW: value not provided */
#endif

/* GDMA path selection (compile-time configurable) */
// MANUAL_REVIEW: Define one of GDMA0_PATH, GDMA1_PATH, GDMA2_PATH, GDMA3_PATH
// at build time to select the GDMA path.

/* GDMA register base */
#define MIPI_CSI2_GDMA_REG_BASE         0xE6A00000UL

/* DMA register offsets */
// MANUAL_REVIEW: MIPI_CSI2_DMA_INTMIS_OFFSET and MIPI_CSI2_DMA_INTCLR_OFFSET
// values are not explicitly provided in the Meta TestPlan JSON. Define per
// the platform DMA register map.
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
// #define MIPI_CSI2_DMA_INTMIS_OFFSET  /* MANUAL_REVIEW: offset not provided */
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
// #define MIPI_CSI2_DMA_INTCLR_OFFSET  /* MANUAL_REVIEW: offset not provided */
#endif

/* PHY stop state expected value */
#define MIPI_CSI2_PHY_STOPSTATE_EXPECTED 0x1000fU

/* Polling timeout count (FV Template style) */
#define MIPI_CSI2_POLL_TIMEOUT           100000U

/* Test pattern generator frame parameters: 320x16, 24bpp */
#define TPG_HRES        320U
#define TPG_VRES        16U
#define TPG_BPP         24U

/* Test pattern generator configuration value */
// MANUAL_REVIEW: TPG_CONFIG_VAL is not explicitly provided in the Meta
// TestPlan JSON. Define per the CSI-2 host PPI_PG_CONFIG register spec.
#ifndef TPG_CONFIG_VAL
// #define TPG_CONFIG_VAL  /* MANUAL_REVIEW: value not provided */
#endif

/* Wait cycles for pattern generator enable/disable */
// MANUAL_REVIEW: Wait period duration is not specified. Adjust as needed.
#define TPG_WAIT_CYCLES  10000U

/* Arrays: NA (no arrays provided in Meta TestPlan JSON) */
