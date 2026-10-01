// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case : mipi_csi2_dphy_lanes_test
 * Purpose   : Supporting macros, headers, and testcase constants for
 *             MIPI CSI-2 D-PHY lane configuration validation.
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

#ifndef LS_LE_EN
#define LS_LE_EN                        0x1U
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

/* --------------------------------------------------------------------------
 * DMA Register Offsets
 * -------------------------------------------------------------------------- */

#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET      0x020U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET     0x028U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET     0x02CU
#endif

#ifndef MIPI_CSI2_DMA_DEBUG_OFFSET
#define MIPI_CSI2_DMA_DEBUG_OFFSET      0x028U
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

#ifndef MIPI_CSI2_CH0_PC
#define MIPI_CSI2_CH0_PC                0xE6000000UL
#endif

#ifndef MIPI_CSI2_CH1_PC
#define MIPI_CSI2_CH1_PC                0xE6000500UL
#endif

#ifndef MIPI_CSI2_TRIGGER_ADDR
#define MIPI_CSI2_TRIGGER_ADDR          0xa0243ffcUL
#endif

#ifndef MIPI_CSI2_POLL_TIMEOUT
#define MIPI_CSI2_POLL_TIMEOUT          100000U
#endif

/* --------------------------------------------------------------------------
 * Arrays: NA (none supplied in Meta TestPlan JSON)
 * -------------------------------------------------------------------------- */
