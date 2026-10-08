// Author - AI Force 2.3. 08-Jun-2025 01:52 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Purpose   : Supporting definitions for MIPI CSI-2 DPHY lane configuration
 *             and CSI-2 data transfer validation testcase.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

/* FV Template framework headers */
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan JSON */
#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2       0xE6002000U
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2       0xE6001000U
#endif

#ifndef LS_LE_EN
#define LS_LE_EN                        0x1U
#endif

#ifndef TOTAL_FRAME
#define TOTAL_FRAME                     1U
#endif

#ifndef VC_ID
#define VC_ID                           0x0U
#endif

#ifndef VRES
#define VRES                            16U
#endif

#ifndef HRES
#define HRES                            320U
#endif

#ifndef DATA_TYPE
#define DATA_TYPE                       0x2BU
#endif

/* DMA register offset macros */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET      0x20U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x24U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET     0x2CU
#endif

/* Arrays: NA */
