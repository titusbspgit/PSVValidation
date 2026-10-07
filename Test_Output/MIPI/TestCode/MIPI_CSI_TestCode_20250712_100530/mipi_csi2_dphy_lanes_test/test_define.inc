// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: mipi_csi2_dphy_lanes_test
 * Purpose: Validates MIPI CSI-2 D-PHY lane configuration by iterating through
 *   lane counts from 4 lanes down to 1 lane, enabling interrupts, configuring
 *   virtual channels, initializing D-PHY, and performing DMA-based control and
 *   data packet transfers for each lane configuration.
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
#define GDMA_CSI2_DATA_DEST_ADDR2    0xE6040080U
#define GDMA_CTRL_DATA_DEST_ADDR2   0xE6040000U
#define LS_LE_EN                     1U
#define TOTAL_FRAME                  1U
#define VC_ID                        3U

/* MANUAL_REVIEW: VRES, HRES, DATA_TYPE are conditionally compiled in DV source. */
/* Provide target-specific values below. */
#ifndef VRES
#define VRES                         16U
#endif

#ifndef HRES
#define HRES                         320U
#endif

#ifndef DATA_TYPE
#define DATA_TYPE                    0x24U
#endif

/* GDMA register base address */
#define GDMA_REG_BASE                0xE6A00000UL

/* DMA register offsets */
/* MANUAL_REVIEW: MIPI_CSI2_DMA_INTEN_OFFSET, MIPI_CSI2_DMA_INTMIS_OFFSET, */
/* MIPI_CSI2_DMA_INTCLR_OFFSET are expected from mipi_csi2.h or test_common.h. */
/* If not provided by those headers, define them here. */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET   0x20U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x24U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET  0x2CU
#endif

/* Virtual channel wrap register value for GDMA0_PATH default */
/* MANUAL_REVIEW: Adjust for target GDMA path (GDMA3/2/1/0_PATH). */
/* For GDMA0_PATH: vcid_csi2_wrap_reg = VC_ID shifted into bits [3:0] */
#ifndef VCID_CSI2_WRAP_REG
#define VCID_CSI2_WRAP_REG           (VC_ID)
#endif

/* Timeout values for polling loops */
#ifndef PHY_STOPSTATE_TIMEOUT
#define PHY_STOPSTATE_TIMEOUT        100000U
#endif

#ifndef DMA_POLL_TIMEOUT
#define DMA_POLL_TIMEOUT             100000U
#endif
