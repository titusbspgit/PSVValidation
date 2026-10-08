// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case    : mipi_csi2_dphy_lanes_test
 * Purpose      : Validates MIPI CSI-2 D-PHY lane configuration by iterating
 *                through all lane counts (4 down to 1), enabling interrupt masks,
 *                configuring virtual channel and control data registers,
 *                initializing D-PHY, polling PHY stop state, and performing
 *                DMA-based control and image data transfers with interrupt-driven
 *                completion polling.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Macros from Meta TestPlan JSON */
#define GDMA_CSI2_DATA_DEST_ADDR2   0xE6040080
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000
#define LS_LE_EN                    1
#define TOTAL_FRAME                 1
#define VC_ID                       3

#ifdef GDMA0_FULL_MEM
#define VRES                        1080
#define HRES                        1920
#else
#define VRES                        3
#define HRES                        64
#endif

#define DATA_TYPE                   CSI2_RGB888

/* DMA Interrupt Offsets */
#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET 0x10
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET 0x14
#endif

#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET  0x18
#endif

/* CSI-2 Address Defines */
#define CSI_CTRL_DATA_ADDR          0xE6001000
#define CSI_DATA_DEST_ADDR          0xE6002000
#define CSI_CTRL_SRC_ADDR           0x8000
#define CSI_DATA_SRC_ADDR           0x0000
#define CSI2_SEQ_TRIGGER_ADDR       0xa0243ffc

/* PHY Stop State Expected Value */
#define PHY_STOPSTATE_EXPECTED      0x1000f

/* Timeout count for polling loops */
#ifndef MIPI_CSI2_TIMEOUT_COUNT
#define MIPI_CSI2_TIMEOUT_COUNT     100000
#endif
