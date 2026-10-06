// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: mipi_csi2_test_pattern_generator
 * Purpose: Defines headers, macros, and supporting data for MIPI CSI-2
 *          internal test pattern generator testcase.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "mipi_csi2.h"

/* FV framework headers */
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

/* DMA register offset macros */
#ifndef MIPI_CSI2_DMA_INTEN_OFFSET
#define MIPI_CSI2_DMA_INTEN_OFFSET   0x20U
#endif

#ifndef MIPI_CSI2_DMA_INTMIS_OFFSET
#define MIPI_CSI2_DMA_INTMIS_OFFSET  0x24U
#endif

#ifndef MIPI_CSI2_DMA_INTCLR_OFFSET
#define MIPI_CSI2_DMA_INTCLR_OFFSET  0x2CU
#endif

/* Timeout values for polling loops */
#ifndef PHY_STOPSTATE_TIMEOUT
#define PHY_STOPSTATE_TIMEOUT        100000U
#endif

#ifndef DMA_POLL_TIMEOUT
#define DMA_POLL_TIMEOUT             100000U
#endif
