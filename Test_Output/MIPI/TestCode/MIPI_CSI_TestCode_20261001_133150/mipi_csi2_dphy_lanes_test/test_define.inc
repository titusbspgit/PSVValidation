// Author - AI Force 2.3. 01-Oct-2026 13:31 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Defines headers, macros, and supporting data for the MIPI CSI-2
 *          D-PHY lane configuration testcase.
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

#ifndef LS_LE_EN
#define LS_LE_EN                     0U
#endif

#ifndef TOTAL_FRAME
#define TOTAL_FRAME                  1U
#endif

#ifndef VC_ID
#define VC_ID                        3U
#endif

#ifndef VRES
#define VRES                         16U
#endif

#ifndef HRES
#define HRES                         320U
#endif

#ifndef DATA_TYPE
#define DATA_TYPE                    0x24U
#endif

/* DMA register offsets */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET   0x20U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x28U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET  0x2CU
#endif

/* Polling timeout constant */
#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT       100000U
#endif

/* PHY stop state expected value */
#define MIPI_CSI2_PHY_STOPSTATE_EXPECTED  0x1000fU

/* Arrays: NA (no arrays supplied in Meta TestPlan) */
