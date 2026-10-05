// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Supporting definitions for MIPI CSI-2 D-PHY Lane Configuration Test.
 *          Validates receiver operation across all D-PHY lane configurations
 *          (4 lanes down to 1 lane) with DMA-based control and data packet transfers.
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
#ifndef GDMA_CSI2_DATA_DEST_ADDR2
#define GDMA_CSI2_DATA_DEST_ADDR2       0xE6002000U
#endif

#ifndef GDMA_CTRL_DATA_DEST_ADDR2
#define GDMA_CTRL_DATA_DEST_ADDR2       0xE6001000U
#endif

#ifndef LS_LE_EN
#define LS_LE_EN                        1U
#endif

#ifndef TOTAL_FRAME
#define TOTAL_FRAME                     1U
#endif

#ifndef VC_ID
#define VC_ID                           3U
#endif

#ifndef VRES
#define VRES                            16U
#endif

#ifndef HRES
#define HRES                            320U
#endif

#ifndef DATA_TYPE
#define DATA_TYPE                       0x24U
#endif

/* DMA register offsets */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET      0x020U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x028U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET     0x02CU
#endif

/* PHY stop state expected value */
#ifndef PHY_STOPSTATE_EXPECTED
#define PHY_STOPSTATE_EXPECTED          0x1000fU
#endif

/* Short packet threshold for long packet detection */
#ifndef SHORT_PKT_THRESHOLD
#define SHORT_PKT_THRESHOLD             0x0fU
#endif
