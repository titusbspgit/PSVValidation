// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_dphy_idi_test
 * Purpose: Supporting definitions for MIPI CSI-2 DPHY IDI data transfer
 *          validation testcase across all 16 virtual channel IDs.
 *          Contains headers, macros, register addresses, and constants.
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

/* Hardcoded addresses from impacted registers */
#define MIPI_CSI2_SEQ_TRIGGER_ADDR      0xa0243ffcUL
#define MIPI_CSI2_CTRL_DATA_ADDR        0xE6001000UL
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

/* Total virtual channel IDs (0 through 15) */
#define IDI_TOTAL_VCIDS                  16U

/* Packets per virtual channel */
#define IDI_PACKETS_PER_VCID             129U

/* Arrays: NA (no arrays provided in Meta TestPlan JSON) */
